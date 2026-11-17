import React, { useState, useEffect } from 'react';
import {
  Crosshair,
  Zap,
  Cpu,
  Terminal,
  Gamepad2,
  Sliders,
  Settings,
  Layers,
  Check,
  Copy,
  Code2,
  ExternalLink,
  Shield,
  Activity,
  RotateCw,
  Monitor,
  Flame,
  HelpCircle,
  FolderTree,
  ChevronDown,
  Sparkles,
  ArrowRight,
  Eye,
  Radio,
  Maximize2
} from 'lucide-react';

// Pinout mapping data
const PINOUT_DATA = [
  { pin: 'Pin 10', component: 'Trigger Microswitch', type: 'Digital Input (Pullup)', action: 'Left Mouse Button', purpose: 'Primary Weapon Firing' },
  { pin: 'Pin 16', component: 'Alt Fire Button', type: 'Digital Input (Pullup)', action: 'Middle Mouse Button', purpose: 'Secondary Fire / Grenade / Special' },
  { pin: 'Pin 14', component: 'Reload Button', type: 'Digital Input (Pullup)', action: 'Key "r"', purpose: 'Reload Magazine' },
  { pin: 'Pin 5 (CLK)', component: 'Rotary Encoder (Clock)', type: 'Digital Interrupt', action: 'Encoder Step Counter', purpose: 'Tune sensitivity on selected axis' },
  { pin: 'Pin 6 (DT)', component: 'Rotary Encoder (Data)', type: 'Digital Input', action: 'Direction Detection', purpose: 'Detect clockwise / counter-clockwise' },
  { pin: 'Pin 7 (SW)', component: 'Encoder Pushbutton', type: 'Digital Input (Pullup)', action: 'Short Press / Long Press', purpose: 'Short press: Switch X/Y Axis | Long press: Aim Hold (freeze)' },
  { pin: 'Pin A0', component: 'Thumbstick VRx', type: 'Analog Input', action: 'Keys "a" & "d"', purpose: 'Horizontal player movement (Strafing)' },
  { pin: 'Pin A1', component: 'Thumbstick VRy', type: 'Analog Input', action: 'Keys "w" & "s"', purpose: 'Vertical player movement (Forward / Back)' },
  { pin: 'Pin 15', component: 'Thumbstick Pushbutton', type: 'Digital Input (Pullup)', action: 'Right Mouse Button', purpose: 'Aim Down Sights (ADS) / Zoom Scope' },
  { pin: 'Pin 2 (SDA)', component: 'MPU6050 & OLED SDA', type: 'I2C Data Bus', action: '400 kHz Fast I2C', purpose: 'Reads 6-DOF gyro/accelerometer & drives OLED' },
  { pin: 'Pin 3 (SCL)', component: 'MPU6050 & OLED SCL', type: 'I2C Clock Bus', action: '400 kHz Fast I2C', purpose: 'Synchronous clock for sensor & display' },
  { pin: 'Pin 9', component: 'Gun Enable Switch', type: 'Digital Input (Pullup)', action: 'Master Hardware Toggle', purpose: 'Instantly disable/enable gyro mouse output' },
  { pin: 'Pin 8', component: 'Scroll Enable Switch', type: 'Digital Input (Pullup)', action: 'Wheel Mode Toggle', purpose: 'Converts gyro tilt into mouse wheel scroll' }
];

// Supported Arcade Emulators
const EMULATORS = [
  {
    name: 'MAME',
    genre: 'Classic Arcade Golden Age',
    games: ['Time Crisis', 'CarnEvil', 'Area 51', 'Point Blank 1 & 2', 'Terminator 2', 'Maximum Force'],
    configTip: 'Set Lightgun device to "mouse" and enable "offscreen reload" in mame.ini. The gun sends standard HID mouse packets directly.'
  },
  {
    name: 'Sega Model 2 Emulator',
    genre: '90s Sega 3D Arcade Titans',
    games: ['The House of the Dead', 'Virtua Cop', 'Virtua Cop 2', 'Behind Enemy Lines', 'Gunblade NY'],
    configTip: 'In emulator settings, select "Mouse" under Raw Input. In-game calibration screen allows pixel-perfect crosshair convergence.'
  },
  {
    name: 'Supermodel (Sega Model 3)',
    genre: 'Late 90s High-End Arcade',
    games: ['The Lost World: Jurassic Park', 'Star Wars Trilogy Arcade', 'Ocean Hunter'],
    configTip: 'Configure Supermodel.ini with InputGunX = MOUSE_XAXIS and InputGunY = MOUSE_YAXIS. Use Trigger (Left Click) and Reload (r).'
  },
  {
    name: 'TeknoParrot',
    genre: 'Modern High-Definition Arcade',
    games: ['House of the Dead 4', 'House of the Dead: Scarlet Dawn', 'Time Crisis 5', 'Transformers Human Alliance', 'Golden Guns'],
    configTip: 'Enable RawInput mode in Game Settings. Map Left Button to Trigger and Thumbstick to WASD for free roaming movement.'
  },
  {
    name: 'Flycast / Demul (Sega Naomi & Hikaru)',
    genre: 'Sega Dreamcast / Naomi Arcade',
    games: ['The Maze of the Kings', 'Lupin the Third: The Shooting', 'Confidential Mission', 'Death Crimson OX'],
    configTip: 'Assign Lightgun port to Mouse 1. Gun tilt left/right can be mapped to dodge buttons in Confidential Mission.'
  },
  {
    name: 'DuckStation & PCSX2',
    genre: 'PlayStation 1 & 2 Classics',
    games: ['Time Crisis: Project Titan', 'Resident Evil: Survivor', 'Dino Stalker', 'Time Crisis 2 & 3', 'Vampire Night'],
    configTip: 'Enable GunCon / GunCon 2 emulation in controller settings. Map cursor to mouse and use the reload key for pedal duck/reload.'
  }
];

export default function App() {
  const [copiedKey, setCopiedKey] = useState(null);

  // Virtual Light Gun State
  const [multiplierX, setMultiplierX] = useState(64);
  const [multiplierY, setMultiplierY] = useState(53);
  const [selectedAxis, setSelectedAxis] = useState('X');
  const [ammo, setAmmo] = useState(6);
  const [isFiring, setIsFiring] = useState(false);
  const [isReloading, setIsReloading] = useState(false);
  const [aimHold, setAimHold] = useState(false);
  const [tiltLean, setTiltLean] = useState(null); // 'left' | 'right' | null
  const [thumbstickDir, setThumbstickDir] = useState(null);
  const [gunEnabled, setGunEnabled] = useState(true);
  const [lastAction, setLastAction] = useState('System Initialized. MPU6050 Ready.');
  const [selectedPin, setSelectedPin] = useState(PINOUT_DATA[0]);
  const [faqOpenIndex, setFaqOpenIndex] = useState(null);

  const handleCopy = (text, key) => {
    navigator.clipboard.writeText(text);
    setCopiedKey(key);
    setTimeout(() => setCopiedKey(null), 2000);
  };

  const handleTrigger = () => {
    if (!gunEnabled) return;
    if (ammo > 0) {
      setAmmo(prev => prev - 1);
      setIsFiring(true);
      setLastAction('Trigger Pressed: Left Mouse Click (Primary Fire)');
      setTimeout(() => setIsFiring(false), 120);
    } else {
      setLastAction('Click! Out of Ammo. Press Reload (Key "r").');
    }
  };

  const handleReload = () => {
    setIsReloading(true);
    setLastAction('Reloading: Key "r" sent to PC.');
    setTimeout(() => {
      setAmmo(6);
      setIsReloading(false);
      setLastAction('Reload Complete! 6 Shells Loaded.');
    }, 400);
  };

  const handleEncoderStep = (direction) => {
    if (selectedAxis === 'X') {
      const next = Math.max(10, Math.min(200, multiplierX + direction * 2));
      setMultiplierX(next);
      setLastAction(`Rotary Encoder Turned: X-Axis Sensitivity &rarr; ${next}`);
    } else {
      const next = Math.max(10, Math.min(200, multiplierY + direction * 2));
      setMultiplierY(next);
      setLastAction(`Rotary Encoder Turned: Y-Axis Sensitivity &rarr; ${next}`);
    }
  };

  const toggleEncoderAxis = () => {
    const nextAxis = selectedAxis === 'X' ? 'Y' : 'X';
    setSelectedAxis(nextAxis);
    setLastAction(`Encoder Button Clicked: Active Tuning Axis switched to ${nextAxis}`);
  };

  return (
    <div className="min-h-screen flex flex-col bg-slate-950 text-slate-100 font-sans">
      {/* Top Banner */}
      <div className="bg-gradient-to-r from-red-950/80 via-slate-900 to-amber-950/80 border-b border-red-500/20 px-4 py-2 text-xs text-slate-300 text-center flex items-center justify-center gap-2">
        <Sparkles className="w-3.5 h-3.5 text-red-400 animate-pulse" />
        <span>
          Play retro arcade light gun classics on modern LCD/OLED monitors with zero CRT requirements.
        </span>
        <a
          href="https://github.com/Domush/Arduino-Arcade-PC-light-gun"
          target="_blank"
          rel="noreferrer"
          className="text-red-400 font-medium hover:underline inline-flex items-center gap-1"
        >
          View Source <ArrowRight className="w-3 h-3" />
        </a>
      </div>

      {/* Main Header */}
      <header className="sticky top-0 z-50 backdrop-blur-md bg-slate-950/85 border-b border-slate-800">
        <div className="max-w-6xl mx-auto px-4 h-16 flex items-center justify-between">
          {/* Logo & Brand */}
          <div className="flex items-center gap-3">
            <div className="relative flex items-center justify-center w-10 h-10 rounded-xl bg-gradient-to-br from-red-500/20 to-orange-500/10 border border-red-500/30 shadow-inner">
              <Crosshair className="w-5 h-5 text-red-500 animate-laser" />
              <span className="absolute -top-1 -right-1 flex h-2.5 w-2.5">
                <span className="animate-ping absolute inline-flex h-full w-full rounded-full bg-red-400 opacity-75"></span>
                <span className="relative inline-flex rounded-full h-2.5 w-2.5 bg-red-500"></span>
              </span>
            </div>
            <div className="flex-shrink-0">
              <div className="flex items-center gap-2">
                <span className="font-bold tracking-tight text-white text-base sm:text-lg whitespace-nowrap">
                  Arcade Light Gun
                </span>
                <span className="bg-red-500/10 text-red-400 border border-red-500/20 px-2 py-0.5 rounded-full text-xs font-mono whitespace-nowrap flex-shrink-0">
                  Arduino &bull; MPU6050
                </span>
              </div>
              <p className="text-xs text-slate-400 hidden sm:block whitespace-nowrap">
                For Modern PC &amp; Retro Arcade Emulators
              </p>
            </div>
          </div>

          {/* Navigation Links */}
          <nav className="hidden md:flex items-center gap-4 lg:gap-6 text-sm font-medium text-slate-300">
            <a
              href="#overview"
              className="hover:text-red-400 transition-colors whitespace-nowrap"
            >
              Overview
            </a>
            <a
              href="#hud"
              className="hover:text-red-400 transition-colors whitespace-nowrap"
            >
              Virtual Gun HUD
            </a>
            <a
              href="#wiring"
              className="hover:text-red-400 transition-colors whitespace-nowrap"
            >
              Pinout &amp; Wiring
            </a>
            <a
              href="#emulators"
              className="hover:text-red-400 transition-colors whitespace-nowrap"
            >
              Emulator Setup
            </a>
            <a
              href="#build"
              className="hover:text-red-400 transition-colors whitespace-nowrap"
            >
              Build Guide
            </a>
          </nav>

          {/* GitHub CTA */}
          <div className="flex items-center gap-3">
            <a
              href="https://github.com/Domush/Arduino-Arcade-PC-light-gun"
              target="_blank"
              rel="noreferrer"
              className="flex items-center gap-1.5 px-3 py-1.5 rounded-lg border border-slate-700 bg-slate-900/80 text-xs font-medium text-slate-200 hover:border-slate-600 transition-colors"
              title="GitHub Repository"
            >
              <Code2 className="w-3.5 h-3.5 text-slate-400" />
              GitHub
            </a>
          </div>
        </div>
      </header>

      {/* Main Content Body */}
      <main className="flex-1">
        {/* HERO SECTION */}
        <section id="overview" className="relative pt-16 pb-20 px-4 overflow-hidden">
          <div className="max-w-4xl mx-auto text-center space-y-6">
            <div className="inline-flex items-center gap-2 px-3 py-1.5 rounded-full bg-slate-900/90 border border-slate-700/80 text-xs text-slate-300 shadow-sm">
              <span className="w-2 h-2 rounded-full bg-red-500 animate-pulse"></span>
              <span>No CRT Required &bull; Works on Any Modern Flat-Screen Display</span>
              <span className="text-slate-600">|</span>
              <span className="text-red-400 font-mono">100% Native USB HID</span>
            </div>

            <h1 className="text-4xl sm:text-5xl lg:text-6xl font-extrabold tracking-tight text-white leading-tight">
              Play All the Arcade Shooters You Grew Up{' '}
              <span className="bg-gradient-to-r from-red-500 via-orange-500 to-amber-400 bg-clip-text text-transparent">
                Dumping Quarters Into
              </span>
            </h1>

            <p className="text-base sm:text-lg text-slate-300 max-w-2xl mx-auto leading-relaxed">
              They don&apos;t make light guns for modern LCDs. This open-source DIY light gun uses an Arduino Pro Micro and MPU6050 6-DOF gyro tracking to bring arcade precision to modern PC displays, MAME, Model 2, and TeknoParrot with zero sensor bars.
            </p>

            {/* Feature Highlights Grid */}
            <div className="grid grid-cols-2 sm:grid-cols-4 gap-3 max-w-3xl mx-auto text-left pt-2">
              <div className="p-3 rounded-xl bg-slate-900/60 border border-slate-800 space-y-1">
                <div className="flex items-center gap-1.5 text-red-400 font-semibold text-xs">
                  <Crosshair className="w-3.5 h-3.5" />
                  <span>MPU6050 6-DOF</span>
                </div>
                <p className="text-[11px] text-slate-400">
                  Sub-5ms gyro motion tracking with automatic drift compensation.
                </p>
              </div>

              <div className="p-3 rounded-xl bg-slate-900/60 border border-slate-800 space-y-1">
                <div className="flex items-center gap-1.5 text-orange-400 font-semibold text-xs">
                  <Sliders className="w-3.5 h-3.5" />
                  <span>On-The-Fly Rotary</span>
                </div>
                <p className="text-[11px] text-slate-400">
                  Live per-axis sensitivity adjustment stored permanently in EEPROM.
                </p>
              </div>

              <div className="p-3 rounded-xl bg-slate-900/60 border border-slate-800 space-y-1">
                <div className="flex items-center gap-1.5 text-cyan-400 font-semibold text-xs">
                  <Gamepad2 className="w-3.5 h-3.5" />
                  <span>Mini-Joystick</span>
                </div>
                <p className="text-[11px] text-slate-400">
                  Built-in thumbstick for WASD movement and Right-Click ADS zoom.
                </p>
              </div>

              <div className="p-3 rounded-xl bg-slate-900/60 border border-slate-800 space-y-1">
                <div className="flex items-center gap-1.5 text-amber-400 font-semibold text-xs">
                  <RotateCw className="w-3.5 h-3.5" />
                  <span>Physical Tilt-to-Lean</span>
                </div>
                <p className="text-[11px] text-slate-400">
                  Tilting the gun left/right presses PgUp/PgDn for tactical cover games.
                </p>
              </div>
            </div>

            {/* Action Buttons */}
            <div className="flex flex-wrap items-center justify-center gap-3 pt-4">
              <a
                href="#hud"
                className="bg-red-600 hover:bg-red-500 text-white font-semibold text-sm px-5 py-2.5 rounded-lg flex items-center gap-2 transition-all shadow-md shadow-red-500/20"
              >
                <Crosshair className="w-4 h-4" />
                Launch Virtual Gun HUD
              </a>
              <a
                href="#wiring"
                className="bg-slate-900 hover:bg-slate-800 text-slate-200 border border-slate-700 font-medium text-sm px-5 py-2.5 rounded-lg flex items-center gap-2 transition-all"
              >
                <Cpu className="w-4 h-4 text-slate-400" />
                Wiring &amp; Pinout
              </a>
            </div>

            {/* Git Clone Box */}
            <div className="max-w-xl mx-auto pt-4">
              <div className="bg-slate-900/90 border border-slate-800 rounded-xl p-3 flex items-center justify-between text-xs font-mono text-slate-300">
                <div className="flex items-center gap-2 overflow-x-auto whitespace-nowrap">
                  <span className="text-red-400 select-none">$</span>
                  <span>git clone https://github.com/Domush/Arduino-Arcade-PC-light-gun.git</span>
                </div>
                <button
                  onClick={() => handleCopy('git clone https://github.com/Domush/Arduino-Arcade-PC-light-gun.git', 'clone')}
                  className="ml-3 p-1.5 rounded hover:bg-slate-800 text-slate-400 hover:text-white transition-colors"
                  title="Copy command"
                >
                  {copiedKey === 'clone' ? (
                    <Check className="w-4 h-4 text-red-400" />
                  ) : (
                    <Copy className="w-4 h-4" />
                  )}
                </button>
              </div>
            </div>
          </div>
        </section>

        {/* INTERACTIVE VIRTUAL LIGHT GUN HUD */}
        <section id="hud" className="py-16 px-4 bg-slate-900/40 border-y border-slate-800/80">
          <div className="max-w-5xl mx-auto space-y-8">
            <div className="text-center space-y-3">
              <div className="inline-flex items-center gap-2 px-3 py-1 rounded-full bg-red-500/10 border border-red-500/20 text-xs font-mono text-red-400">
                <TargetIcon className="w-3.5 h-3.5" />
                <span>Real-Time Firmware Emulation</span>
              </div>
              <h2 className="text-3xl sm:text-4xl font-bold tracking-tight text-white">
                Interactive Virtual Gun HUD
              </h2>
              <p className="text-slate-400 text-sm max-w-2xl mx-auto">
                Test the exact button behaviors, rotary encoder tuning, aim freeze, and SSD1306 OLED feedback implemented in <code className="text-red-400 font-mono">Arduino_arcade_light_gun.ino</code>.
              </p>
            </div>

            {/* HUD Simulator Workspace */}
            <div className="grid grid-cols-1 lg:grid-cols-12 gap-6 bg-slate-950/80 border border-slate-800 rounded-2xl p-4 sm:p-6 shadow-2xl">
              {/* Left Column: Virtual Arcade Target Range & Screen */}
              <div className="lg:col-span-7 space-y-4">
                <div className="relative aspect-video bg-gradient-to-b from-slate-900 to-slate-950 rounded-xl border border-slate-800 overflow-hidden flex flex-col items-center justify-between p-4 select-none">
                  {/* Top HUD Data */}
                  <div className="w-full flex items-center justify-between text-xs font-mono">
                    <div className="flex items-center gap-2">
                      <span className={`w-2 h-2 rounded-full ${gunEnabled ? 'bg-emerald-400 animate-pulse' : 'bg-red-500'}`}></span>
                      <span className="text-slate-400">STATUS: {gunEnabled ? 'GUN ARMED' : 'DISABLED'}</span>
                    </div>
                    <div className="flex items-center gap-3">
                      <span className="text-amber-400">AMMO: {ammo} / 6</span>
                      <span className={aimHold ? 'text-cyan-400 font-bold' : 'text-slate-600'}>
                        [AIM HOLD {aimHold ? 'ON' : 'OFF'}]
                      </span>
                    </div>
                  </div>

                  {/* Target Crosshair / Reticle */}
                  <div
                    onClick={handleTrigger}
                    className="relative cursor-crosshair group flex items-center justify-center p-8"
                  >
                    <div className={`relative flex items-center justify-center w-24 h-24 rounded-full border-2 transition-all duration-75 ${
                      isFiring
                        ? 'scale-125 border-yellow-400 bg-yellow-500/20 shadow-lg shadow-yellow-500/50'
                        : isReloading
                        ? 'border-blue-400 animate-spin'
                        : 'border-red-500/70 group-hover:border-red-400 group-hover:scale-105'
                    }`}>
                      {/* Crosshair ticks */}
                      <div className="absolute w-full h-[1px] bg-red-500/60"></div>
                      <div className="absolute h-full w-[1px] bg-red-500/60"></div>
                      <div className="w-2.5 h-2.5 rounded-full bg-red-500/80"></div>
                    </div>
                    {isFiring && (
                      <div className="absolute -top-3 text-xs font-mono text-yellow-300 font-bold animate-ping">
                        BANG!
                      </div>
                    )}
                  </div>

                  {/* Bottom HUD Log */}
                  <div className="w-full bg-slate-950/90 border border-slate-800/80 rounded px-3 py-1.5 flex items-center justify-between text-[11px] font-mono text-slate-400">
                    <span className="text-red-400 font-semibold select-none">&gt;&gt;</span>
                    <span className="truncate max-w-[85%] text-slate-300">{lastAction}</span>
                    <span className="text-slate-500">USB HID</span>
                  </div>
                </div>

                {/* Gun Hardware Controls Bar */}
                <div className="grid grid-cols-2 sm:grid-cols-4 gap-2 pt-2">
                  <button
                    onClick={handleTrigger}
                    className="py-2.5 px-3 rounded-lg bg-red-600 hover:bg-red-500 text-white font-bold text-xs flex items-center justify-center gap-1.5 shadow-md shadow-red-600/20 active:scale-95 transition-all"
                  >
                    <Crosshair className="w-3.5 h-3.5" />
                    Trigger (Fire)
                  </button>

                  <button
                    onClick={handleReload}
                    className="py-2.5 px-3 rounded-lg bg-slate-900 hover:bg-slate-800 border border-slate-700 text-slate-200 font-semibold text-xs flex items-center justify-center gap-1.5 active:scale-95 transition-all"
                  >
                    <RotateCw className={`w-3.5 h-3.5 text-blue-400 ${isReloading ? 'animate-spin' : ''}`} />
                    Reload (Key &apos;r&apos;)
                  </button>

                  <button
                    onClick={() => {
                      setAimHold(!aimHold);
                      setLastAction(`Aim Hold Toggled: ${!aimHold ? 'Crosshair Frozen (Adjust Gun Aim)' : 'Resumed Tracking'}`);
                    }}
                    className={`py-2.5 px-3 rounded-lg border font-semibold text-xs flex items-center justify-center gap-1.5 active:scale-95 transition-all ${
                      aimHold
                        ? 'bg-cyan-500/20 border-cyan-500/50 text-cyan-300'
                        : 'bg-slate-900 border-slate-700 text-slate-300 hover:bg-slate-800'
                    }`}
                  >
                    <Eye className="w-3.5 h-3.5" />
                    Aim Hold ({aimHold ? 'ON' : 'OFF'})
                  </button>

                  <button
                    onClick={() => {
                      setGunEnabled(!gunEnabled);
                      setLastAction(`Hardware Switch: Gun ${!gunEnabled ? 'ENABLED' : 'DISABLED'}`);
                    }}
                    className={`py-2.5 px-3 rounded-lg border font-semibold text-xs flex items-center justify-center gap-1.5 active:scale-95 transition-all ${
                      gunEnabled
                        ? 'bg-emerald-500/20 border-emerald-500/40 text-emerald-300'
                        : 'bg-red-950 border-red-800 text-red-400'
                    }`}
                  >
                    <Radio className="w-3.5 h-3.5" />
                    Gun Power
                  </button>
                </div>
              </div>

              {/* Right Column: Simulated SSD1306 OLED Display & Rotary Encoder */}
              <div className="lg:col-span-5 bg-slate-900/70 border border-slate-800 rounded-xl p-4 flex flex-col justify-between space-y-4">
                <div className="space-y-3">
                  <div className="flex items-center justify-between">
                    <span className="text-xs font-semibold text-slate-300 flex items-center gap-1.5">
                      <Monitor className="w-4 h-4 text-cyan-400" />
                      Simulated 128x32 OLED Display
                    </span>
                    <span className="text-[10px] font-mono text-slate-500">I2C 0x3C</span>
                  </div>

                  {/* OLED Physical Screen Simulation */}
                  <div className="bg-black border-2 border-slate-800 rounded-lg p-3 font-mono text-cyan-400 text-xs shadow-inner space-y-1 select-none">
                    <div className="flex items-center justify-between border-b border-cyan-900/60 pb-1">
                      <span className="font-bold tracking-widest">ARCADE GUN</span>
                      <span className="text-[10px] text-cyan-500">v1.2</span>
                    </div>
                    <div className="flex items-center justify-between pt-1">
                      <div className="space-y-0.5">
                        <div className={selectedAxis === 'X' ? 'bg-cyan-950 px-1 text-white font-bold' : ''}>
                          SENS X: {multiplierX}
                        </div>
                        <div className={selectedAxis === 'Y' ? 'bg-cyan-950 px-1 text-white font-bold' : ''}>
                          SENS Y: {multiplierY}
                        </div>
                      </div>
                      <div className="text-right text-[11px] text-cyan-300">
                        <div>AXIS: [{selectedAxis}]</div>
                        <div className="text-[10px] text-cyan-600">EEPROM OK</div>
                      </div>
                    </div>
                  </div>

                  {/* Rotary Encoder Knob Simulation */}
                  <div className="bg-slate-950 p-3.5 rounded-xl border border-slate-800 space-y-2.5">
                    <div className="flex items-center justify-between text-xs">
                      <span className="text-slate-300 font-medium">Rotary Encoder (KY-040)</span>
                      <button
                        onClick={toggleEncoderAxis}
                        className="px-2 py-0.5 rounded bg-slate-800 hover:bg-slate-700 text-[11px] text-amber-400 font-mono transition-colors"
                      >
                        Press Knob: [{selectedAxis}] Axis
                      </button>
                    </div>

                    <div className="flex items-center justify-between gap-2">
                      <button
                        onClick={() => handleEncoderStep(-1)}
                        className="flex-1 py-1.5 rounded bg-slate-900 hover:bg-slate-800 border border-slate-700 text-xs font-mono text-slate-200 active:scale-95 transition-all"
                      >
                        &minus; Turn Left
                      </button>
                      <div className="w-8 h-8 rounded-full bg-slate-800 border border-slate-700 flex items-center justify-center text-xs font-mono font-bold text-cyan-400">
                        {selectedAxis}
                      </div>
                      <button
                        onClick={() => handleEncoderStep(1)}
                        className="flex-1 py-1.5 rounded bg-slate-900 hover:bg-slate-800 border border-slate-700 text-xs font-mono text-slate-200 active:scale-95 transition-all"
                      >
                        + Turn Right
                      </button>
                    </div>
                    <p className="text-[10px] text-slate-500">
                      Encoder button cycles between X and Y sensitivity. Multipliers are auto-saved to EEPROM after 2 seconds of inactivity.
                    </p>
                  </div>

                  {/* Tilt to Lean Simulation */}
                  <div className="bg-slate-950 p-3 rounded-xl border border-slate-800 space-y-2">
                    <div className="flex items-center justify-between text-xs">
                      <span className="text-slate-300 font-medium">Gyro Tilt-To-Lean</span>
                      <span className="text-[10px] font-mono text-slate-500">MPU6050 Accelerometer</span>
                    </div>
                    <div className="grid grid-cols-2 gap-2 text-xs font-mono">
                      <button
                        onMouseDown={() => {
                          setTiltLean('left');
                          setLastAction('Gun Tilted Left: Sent Key PgUp (Lean Left in Cover)');
                        }}
                        onMouseUp={() => setTiltLean(null)}
                        className={`py-1.5 rounded border transition-all ${
                          tiltLean === 'left'
                            ? 'bg-amber-500/20 border-amber-500/50 text-amber-300'
                            : 'bg-slate-900 border-slate-800 text-slate-400 hover:text-white'
                        }`}
                      >
                        &larr; Tilt Left (PgUp)
                      </button>
                      <button
                        onMouseDown={() => {
                          setTiltLean('right');
                          setLastAction('Gun Tilted Right: Sent Key PgDn (Lean Right in Cover)');
                        }}
                        onMouseUp={() => setTiltLean(null)}
                        className={`py-1.5 rounded border transition-all ${
                          tiltLean === 'right'
                            ? 'bg-amber-500/20 border-amber-500/50 text-amber-300'
                            : 'bg-slate-900 border-slate-800 text-slate-400 hover:text-white'
                        }`}
                      >
                        Tilt Right (PgDn) &rarr;
                      </button>
                    </div>
                  </div>
                </div>

                <div className="text-[10px] text-slate-500 font-mono text-center">
                  Firmware Loop: 5ms MPU Refresh (200Hz Tracking)
                </div>
              </div>
            </div>
          </div>
        </section>

        {/* PINOUT & INTERACTIVE WIRING MATRIX */}
        <section id="wiring" className="py-16 px-4">
          <div className="max-w-5xl mx-auto space-y-8">
            <div className="text-center space-y-3">
              <div className="inline-flex items-center gap-2 px-3 py-1 rounded-full bg-cyan-500/10 border border-cyan-500/20 text-xs font-mono text-cyan-400">
                <Cpu className="w-3.5 h-3.5" />
                <span>ATmega32U4 Hardware Pinout</span>
              </div>
              <h2 className="text-3xl sm:text-4xl font-bold tracking-tight text-white">
                Arduino Pro Micro Wiring &amp; Pin Assignments
              </h2>
              <p className="text-slate-400 text-sm max-w-2xl mx-auto">
                All optional controls (thumbstick, rotary encoder, switches) are completely modular. If you don&apos;t connect a component, the firmware simply ignores it without code changes.
              </p>
            </div>

            {/* Interactive Wiring Table */}
            <div className="bg-slate-900/80 border border-slate-800 rounded-2xl overflow-hidden shadow-xl">
              <div className="overflow-x-auto">
                <table className="w-full text-left text-xs text-slate-300">
                  <thead className="bg-slate-950 text-slate-400 font-mono border-b border-slate-800 uppercase text-[11px]">
                    <tr>
                      <th className="py-3 px-4">Pin Assignment</th>
                      <th className="py-3 px-4">Connected Component</th>
                      <th className="py-3 px-4">Input Type</th>
                      <th className="py-3 px-4">PC Emulation (HID)</th>
                      <th className="py-3 px-4">Shooter Function</th>
                    </tr>
                  </thead>
                  <tbody className="divide-y divide-slate-800/60 font-mono">
                    {PINOUT_DATA.map((row, idx) => (
                      <tr
                        key={idx}
                        onClick={() => setSelectedPin(row)}
                        className={`hover:bg-slate-800/50 cursor-pointer transition-colors ${
                          selectedPin.pin === row.pin ? 'bg-red-500/10 text-white' : ''
                        }`}
                      >
                        <td className="py-3 px-4 font-bold text-red-400 whitespace-nowrap">
                          {row.pin}
                        </td>
                        <td className="py-3 px-4 font-sans font-medium text-slate-200 whitespace-nowrap">
                          {row.component}
                        </td>
                        <td className="py-3 px-4 text-slate-400 whitespace-nowrap">
                          {row.type}
                        </td>
                        <td className="py-3 px-4 text-cyan-300 font-bold whitespace-nowrap">
                          {row.action}
                        </td>
                        <td className="py-3 px-4 text-slate-300 font-sans">
                          {row.purpose}
                        </td>
                      </tr>
                    ))}
                  </tbody>
                </table>
              </div>
            </div>

            {/* Selected Pin Details Callout */}
            <div className="bg-slate-950 border border-slate-800 rounded-xl p-4 flex flex-col sm:flex-row items-start sm:items-center justify-between gap-3 text-xs">
              <div className="space-y-1">
                <div className="flex items-center gap-2">
                  <span className="font-bold text-red-400 font-mono">{selectedPin.pin}:</span>
                  <span className="text-white font-semibold">{selectedPin.component}</span>
                  <span className="text-slate-500 font-mono">({selectedPin.type})</span>
                </div>
                <p className="text-slate-400">
                  Emulates <strong className="text-cyan-300 font-mono">{selectedPin.action}</strong> for {selectedPin.purpose}. Internal pull-ups are enabled in firmware; connect one side to the pin and the other directly to Ground (GND).
                </p>
              </div>
              <span className="text-[10px] font-mono px-2 py-1 rounded bg-slate-900 border border-slate-800 text-slate-400 flex-shrink-0">
                Active in Firmware
              </span>
            </div>
          </div>
        </section>

        {/* EMULATOR SETUP & COMPATIBILITY MATRIX */}
        <section id="emulators" className="py-16 px-4 bg-slate-900/30 border-y border-slate-800">
          <div className="max-w-5xl mx-auto space-y-8">
            <div className="text-center space-y-3">
              <div className="inline-flex items-center gap-2 px-3 py-1 rounded-full bg-amber-500/10 border border-amber-500/20 text-xs font-mono text-amber-400">
                <Gamepad2 className="w-3.5 h-3.5" />
                <span>Universal PC Compatibility</span>
              </div>
              <h2 className="text-3xl sm:text-4xl font-bold tracking-tight text-white">
                Emulator Configuration &amp; Supported Games
              </h2>
              <p className="text-slate-400 text-sm max-w-2xl mx-auto">
                Because the gun appears to Windows and Linux as a standard USB Mouse and Keyboard, it works natively across every major retro arcade emulator with zero third-party drivers.
              </p>
            </div>

            <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-4">
              {EMULATORS.map((emu, i) => (
                <div
                  key={i}
                  className="bg-slate-950 border border-slate-800 rounded-xl p-5 space-y-3 hover:border-red-500/40 transition-colors flex flex-col justify-between"
                >
                  <div className="space-y-2">
                    <div className="flex items-center justify-between">
                      <h3 className="font-bold text-white text-base">{emu.name}</h3>
                      <span className="text-[10px] font-mono px-2 py-0.5 rounded bg-slate-900 text-amber-400 border border-slate-800">
                        {emu.genre}
                      </span>
                    </div>

                    <div className="space-y-1">
                      <div className="text-[11px] text-slate-400 font-semibold">Featured Classics:</div>
                      <div className="flex flex-wrap gap-1">
                        {emu.games.map((g, gi) => (
                          <span
                            key={gi}
                            className="text-[10px] font-mono px-1.5 py-0.5 rounded bg-slate-900/80 border border-slate-800 text-slate-300"
                          >
                            {g}
                          </span>
                        ))}
                      </div>
                    </div>
                  </div>

                  <div className="pt-2 border-t border-slate-800/80 text-xs text-slate-400 leading-relaxed">
                    <strong className="text-slate-300 font-medium">Config:</strong> {emu.configTip}
                  </div>
                </div>
              ))}
            </div>
          </div>
        </section>

        {/* STEP-BY-STEP BUILD & CALIBRATION GUIDE */}
        <section id="build" className="py-16 px-4">
          <div className="max-w-4xl mx-auto space-y-8">
            <div className="text-center space-y-3">
              <div className="inline-flex items-center gap-2 px-3 py-1 rounded-full bg-red-500/10 border border-red-500/20 text-xs font-mono text-red-400">
                <Terminal className="w-3.5 h-3.5" />
                <span>Maker Walkthrough</span>
              </div>
              <h2 className="text-3xl sm:text-4xl font-bold tracking-tight text-white">
                Build &amp; Flashing Guide
              </h2>
              <p className="text-slate-400 text-sm max-w-2xl mx-auto">
                Assemble the hardware, flash the sketch using the Arduino IDE or PlatformIO, and calibrate your gyroscope offsets.
              </p>
            </div>

            <div className="space-y-4">
              {/* Step 1 */}
              <div className="bg-slate-900/70 border border-slate-800 rounded-xl p-5 space-y-2">
                <div className="flex items-center gap-2 text-red-400 font-bold text-sm">
                  <span className="w-6 h-6 rounded-full bg-red-500/20 flex items-center justify-center text-xs">1</span>
                  <span>Gather Hardware Components</span>
                </div>
                <p className="text-xs text-slate-300 leading-relaxed">
                  You need an <strong>Arduino Pro Micro (5V / 16MHz ATmega32U4)</strong> and an <strong>MPU6050 6-DOF IMU module</strong>. Optional enhancements include a KY-040 rotary encoder, an analog 2-axis thumbstick, an I2C 128x32/128x64 SSD1306 OLED display, and push buttons / microswitches for Trigger and Reload.
                </p>
              </div>

              {/* Step 2 */}
              <div className="bg-slate-900/70 border border-slate-800 rounded-xl p-5 space-y-3">
                <div className="flex items-center gap-2 text-red-400 font-bold text-sm">
                  <span className="w-6 h-6 rounded-full bg-red-500/20 flex items-center justify-center text-xs">2</span>
                  <span>Install Required Arduino Libraries</span>
                </div>
                <p className="text-xs text-slate-300">
                  Open Arduino IDE or PlatformIO and install the following standard libraries from the Library Manager:
                </p>
                <div className="bg-slate-950 p-3 rounded-lg border border-slate-800 font-mono text-xs text-cyan-300 space-y-1">
                  <div>&bull; Wire, SPI, Mouse, Keyboard (Built-in)</div>
                  <div>&bull; MPU6050_light by rfetick</div>
                  <div>&bull; MD_REncoder by majicDesigns</div>
                  <div>&bull; Adafruit SSD1306 &amp; Adafruit GFX Library</div>
                </div>
              </div>

              {/* Step 3 */}
              <div className="bg-slate-900/70 border border-slate-800 rounded-xl p-5 space-y-3">
                <div className="flex items-center gap-2 text-red-400 font-bold text-sm">
                  <span className="w-6 h-6 rounded-full bg-red-500/20 flex items-center justify-center text-xs">3</span>
                  <span>Flash the Firmware</span>
                </div>
                <p className="text-xs text-slate-300">
                  Clone the repository, open <code className="text-red-400 font-mono">Arduino_arcade_light_gun.ino</code>, select <strong>Arduino Leonardo</strong> or <strong>Arduino Micro</strong> as your board, and hit Upload:
                </p>
                <div className="bg-slate-950 p-3 rounded-lg border border-slate-800 font-mono text-xs flex items-center justify-between text-slate-300">
                  <code>git clone https://github.com/Domush/Arduino-Arcade-PC-light-gun.git</code>
                  <button
                    onClick={() => handleCopy('git clone https://github.com/Domush/Arduino-Arcade-PC-light-gun.git', 'clone-guide')}
                    className="p-1 hover:text-white text-slate-400"
                  >
                    {copiedKey === 'clone-guide' ? <Check className="w-4 h-4 text-red-400" /> : <Copy className="w-4 h-4" />}
                  </button>
                </div>
              </div>

              {/* Step 4 */}
              <div className="bg-slate-900/70 border border-slate-800 rounded-xl p-5 space-y-2">
                <div className="flex items-center gap-2 text-red-400 font-bold text-sm">
                  <span className="w-6 h-6 rounded-full bg-red-500/20 flex items-center justify-center text-xs">4</span>
                  <span>Gyro Calibration &amp; Sensitivity Tuning</span>
                </div>
                <p className="text-xs text-slate-300 leading-relaxed">
                  Upon powering on, keep the gun stationary for 1 second while the MPU6050 calculates initial zero-rate offsets. If crosshair drift occurs during a play session, press <strong>Encoder + Alt Button</strong> simultaneously to recalculate offsets instantly without rebooting. Turn the rotary knob to tune sensitivity for your screen size.
                </p>
              </div>
            </div>

            {/* Troubleshooting FAQ Accordion */}
            <div className="space-y-3 pt-6">
              <h3 className="text-base font-bold text-white flex items-center gap-2">
                <HelpCircle className="w-4 h-4 text-amber-400" />
                <span>Frequently Asked Questions</span>
              </h3>

              {[
                {
                  q: 'Why not use traditional light gun sensors (CRT diodes)?',
                  a: 'Original arcade light guns detected the instantaneous cathode ray electron beam scanning lines of glass CRT televisions. Modern LCD, LED, and OLED displays draw entire frames simultaneously, making classic CRT light guns physically impossible to use on modern flat screens without gyro or camera tracking.'
                },
                {
                  q: 'How does this compare to Wiimote or IR sensor bars?',
                  a: 'IR sensor bar setups (like the Sinden or Mayflash DolphinBar) require line-of-sight tracking and can lose tracking if you step out of bounds or point off-screen. Gyro-based tracking requires zero sensor bars, works at any distance or angle from the monitor, and has zero optical occlusion issues.'
                },
                {
                  q: 'Can I use this as an Air Mouse / Hover Mouse for my PC desktop?',
                  a: 'Yes! The code works right out of the box as an air mouse with zero code changes. You can navigate Windows or macOS, browse websites, and click desktop icons just by pointing the gun.'
                },
                {
                  q: 'What if I only want the trigger and no joystick or encoder?',
                  a: 'All optional components are purely plug-and-play. If you leave pins 5, 6, 7, A0, A1 disconnected, the firmware cleanly ignores them and defaults to the hardcoded movement multipliers (X: 64, Y: 53).'
                }
              ].map((faq, i) => (
                <div
                  key={i}
                  className="bg-slate-900/60 border border-slate-800 rounded-xl overflow-hidden"
                >
                  <button
                    onClick={() => setFaqOpenIndex(faqOpenIndex === i ? null : i)}
                    className="w-full px-4 py-3 text-left text-xs sm:text-sm font-medium text-slate-200 flex items-center justify-between hover:bg-slate-850"
                  >
                    <span>{faq.q}</span>
                    <ChevronDown
                      className={`w-4 h-4 text-slate-400 transform transition-transform ${
                        faqOpenIndex === i ? 'rotate-180' : ''
                      }`}
                    />
                  </button>
                  {faqOpenIndex === i && (
                    <div className="px-4 pb-3 pt-1 text-xs text-slate-400 leading-relaxed border-t border-slate-800/60">
                      {faq.a}
                    </div>
                  )}
                </div>
              ))}
            </div>
          </div>
        </section>
      </main>

      {/* Footer */}
      <footer className="border-t border-slate-800/80 bg-slate-950 py-10 px-4 text-xs text-slate-500">
        <div className="max-w-6xl mx-auto flex flex-col sm:flex-row items-center justify-between gap-4">
          <div className="flex items-center gap-2">
            <Crosshair className="w-4 h-4 text-red-500" />
            <span className="font-semibold text-slate-300">Arduino Arcade PC Light Gun</span>
            <span>&bull;</span>
            <span>By Edward Webber &bull; GPLv3 Non-Commercial</span>
          </div>

          <div className="flex items-center gap-4">
            <a
              href="https://github.com/Domush/Arduino-Arcade-PC-light-gun"
              target="_blank"
              rel="noreferrer"
              className="text-slate-400 hover:text-white transition-colors"
            >
              GitHub Repository
            </a>
            <span>&bull;</span>
            <span className="text-slate-400">Zero CRT Required</span>
          </div>
        </div>
      </footer>
    </div>
  );
}

function TargetIcon(props) {
  return (
    <svg
      {...props}
      xmlns="http://www.w3.org/2000/svg"
      viewBox="0 0 24 24"
      fill="none"
      stroke="currentColor"
      strokeWidth="2"
      strokeLinecap="round"
      strokeLinejoin="round"
    >
      <circle cx="12" cy="12" r="10" />
      <circle cx="12" cy="12" r="6" />
      <circle cx="12" cy="12" r="2" />
    </svg>
  );
}
