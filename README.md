# Arduino Arcade PC Light Gun

[![Website](https://img.shields.io/badge/Website-Interactive%20Guide%20%26%20Wiring-red?style=flat-square)](https://domush.github.io/Arduino-Arcade-PC-light-gun/)
[![Hardware](https://img.shields.io/badge/Hardware-Arduino%20Pro%20Micro-00979D?style=flat-square&logo=arduino)](https://www.arduino.cc/)
[![Sensor](https://img.shields.io/badge/IMU-MPU6050%20Gyro%20%2B%20Accel-blue?style=flat-square)](https://invensense.tdk.com/products/motion-tracking/6-axis/mpu-6050/)

🌐 **Live Interactive Website, Wiring Diagrams & Calibration Guide:** [domush.github.io/Arduino-Arcade-PC-light-gun](https://domush.github.io/Arduino-Arcade-PC-light-gun/)

## Arcade light gun, for the modern PC and retro arcade game emulators

They don't make light guns that work with LCD screens, so I wrote this to emulate the old arcade light guns for using with a modern PC. 

## Play all of those arcade shooters you grew up dumping quarters into!

### Requirements:
- an Arduino pro micro
- a Gyroscope
- an Accelerometer

### Optionals:
- Rotary encoder
- Analog thumbstick
- Switch
- Two push buttons

### Features:
- Primary firing
- Secondary firing
- Scope aim
- Reload
- Character leaning _(by tilting the gun)_
- Adjustable sensitivity on each axis
- Movement _(using a mini-joystick/thumbstick)_
- Aim hold _(press to adjust the gun without moving the crosshair)_
- On/off switch

**The code is complete and should work for anyone. Take careful note of the pin assignments and adjust accordingly.**

There is mini-joystick support for movement and zoom (aiming down sights). I feel like it would be limiting if you have to use a secondary controller/keyboard for movement, but it's your call if you want to use it.

### All options are just that, _optional_. If you don't add a joystick or encoder or buttons they'll just be ignored. No need to disable them in the code.

This code can easily be used as a simple hover-mouse if you wish, with _zero required code changes_.

### Assignments:
- **Trigger** : Left mouse
- **Alt** : Middle mouse
- **Reload** : Presses 'r' key
- **Encoder** : increase/decrease gyro sensitivity on a per-axis basis (press encoder button to switch axes)
  - **Encoder + Alt** : Re-calibrate gyro
  - **Encoder button** :
    - (**press**) Switch between X and Y gyro sensitivity adjustment
    - (**hold**) Hold cursor - used to adjust gun aim
- **Joystick** : WASD keys
  - **Joystick button** : Right mouse
- **Left/Right tilt** : Presses PgUp/PgDn keys (designed to be used for leaning in games which support it)

My code is on Github: https://github.com/Domush/Arduino-Arcade-PC-light-gun

# For non-commercial use ONLY.
If you wish to use this code for commercial purposes, you must contact me for permission.
