/*
 *  Project     Arduino Arcade PC Light Gun
 *  @author     Edward Webber
 *  @link       https://github.com/Domush/Arduino-Arcade-PC-light-gun
 *  @license    GPLv3 non-commercial - Copyright (c) 2021 Edward Webber
 */

// ===============
// User Settings
// ===============

// Tuning Options
const long mpuUpdateRate = 5;   // In milliseconds

// Multiplier converts gun movement into appropriate mouse movement
int mpuMovementMultiplierX = 64, mpuMovementMultiplierY = 53;

// Debug Flags (uncomment to display comments via serial connection)
// #define DEBUG   // Enabling will wait for serial connection before activating
#ifdef DEBUG
  #define DEBUG_BUTTONS   // Show button presses (and what they do)
  // #define DEBUG_GYRO // Show mouse movements (very spammy, use with caution)
  #define DEBUG_ENCODER    // Show encoder movements
  #define DEBUG_JOYSTICK   // Show joystick movements
#endif

// Pin Definitions (change with care)
const uint8_t buttonTriggerPin      = 10;
const uint8_t buttonAltPin          = 16;
const uint8_t buttonReloadPin       = 14;
const uint8_t encoderDTPin          = 6;
const uint8_t encoderCLKPin         = 5;
const uint8_t buttonEncoderPin      = 7;
const uint8_t joystickXPin          = A0;
const uint8_t joystickYPin          = A1;
const uint8_t buttonJoystickPin     = 15;
const uint8_t switchGunEnablePin    = 9;
const uint8_t switchScrollTogglePin = 8;

// ========================
// DO NOT EDIT BELOW HERE
// ========================
#include <SPI.h>
#include <Wire.h>
#include <Mouse.h>
#include <Keyboard.h>
#include <Adafruit_GFX.h>
#include <MPU6050_light.h>
#include <MD_REncoder.h>
#include <Adafruit_SSD1306.h>
#include <splash.h>

// Global Variables
long timestamp;
long mpuLastUpdate;
long debugLastUpdate;
char currentAdjustmentAxis = 'X';

boolean activeScroll         = false;
boolean activeGun            = false;
boolean activeTrigger        = false;
boolean activeAlt            = false;
boolean activeReload         = false;
boolean activeEncoder        = false;
boolean activeJoystick       = false;
boolean activeJoystickXplus  = false;
boolean activeJoystickXminus = false;
boolean activeJoystickYplus  = false;
boolean activeJoystickYminus = false;

#ifdef DEBUG
  #define DEBUG_PRINT(x) \
    do {                 \
      Serial.print(x);   \
    } while (0)
  #define DEBUG_PRINTLN(x) \
    do {                   \
      Serial.println(x);   \
    } while (0)
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTLN(x)
#endif

// Instantiate Objects
MD_REncoder encoder = MD_REncoder(encoderCLKPin, encoderDTPin);   // Encoder
MPU6050 mpu         = MPU6050(Wire);                              // MPU6050 gryo
// OakOLED oled;
// Adafruit_SSD1306 oled;
#define SCREEN_WIDTH  128   // OLED display width, in pixels
#define SCREEN_HEIGHT 32    // OLED display height, in pixels

#define OLED_RESET     -1     // Reset pin # (or -1 if sharinrg Arduino reset pin)
#define SCREEN_ADDRESS 0x3C   ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void displayConfig(bool displayClear = false, int displayTextSize = 1) {
  display.setTextSize(displayTextSize);
  display.setTextColor(1);
  display.setRotation(0);
  if (displayClear) {
    display.clearDisplay();
    display.setCursor(0, 0);
  }
}

void setup() {
#ifdef DEBUG
  Serial.begin(115200);
  while (!Serial)
    ;   // Wait for connection
#endif

  pinMode(switchGunEnablePin, INPUT_PULLUP);
  pinMode(switchScrollTogglePin, INPUT_PULLUP);
  pinMode(buttonTriggerPin, INPUT_PULLUP);
  pinMode(buttonAltPin, INPUT_PULLUP);
  pinMode(buttonReloadPin, INPUT_PULLUP);
  pinMode(buttonEncoderPin, INPUT_PULLUP);
  pinMode(buttonJoystickPin, INPUT_PULLUP);
  pinMode(joystickXPin, INPUT);
  pinMode(joystickYPin, INPUT);

  if (digitalRead(buttonTriggerPin) == 0) {
    failsafe();   // In case something goes wrong, stop everything and display serial message
  }

  Mouse.begin();
  Keyboard.begin();
  encoder.begin();
  Wire.begin();
  mpu.begin();
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {   // Address 0x3C for 128x64
    DEBUG_PRINTLN("SSD1306 initialization failed");
    failsafe();
  }
  // mpu.calcGyroOffsets(true);
  mpu.calcOffsets();   // Calibrate gyro and accelerometer
}

void loop() {
  timestamp = millis();
  if (timestamp >= mpuLastUpdate + mpuUpdateRate) {
    mpuLastUpdate = timestamp;
    ProcessMPU();
  }
  ProcessEncoder();
  ProcessJoystick();
  ProcessButtons();
}

// Handle button presses from all sources
void ProcessButtons() {
  boolean pressedGunEnable    = !digitalRead(switchGunEnablePin);
  boolean pressedScrollToggle = !digitalRead(switchScrollTogglePin);
  boolean pressedTrigger      = !digitalRead(buttonTriggerPin);
  boolean pressedAlt          = !digitalRead(buttonAltPin);
  boolean pressedReload       = !digitalRead(buttonReloadPin);
  boolean pressedJoystick     = !digitalRead(buttonJoystickPin);
  boolean pressedEncoder      = !digitalRead(buttonEncoderPin);

  if (pressedGunEnable) {
    if (!activeGun) {
#ifdef DEBUG_BUTTONS
      DEBUG_PRINTLN("Enabling Gun Controls");
#endif
      // Display what's happening
      displayConfig(true, 2);
      display.setCursor(15, 0);
      display.println("Powering\n    up!");
      display.display();
      //  End display
      activeGun = true;
    }
  } else if (!pressedGunEnable && activeGun) {
#ifdef DEBUG_BUTTONS
    DEBUG_PRINTLN("Disabling Gun Controls");
#endif
    // Display what's happening
    displayConfig(true, 2);
    display.setCursor(15, 0);
    display.println("Gun\n disabled :(");
    display.display();
    //  End display
    activeGun = false;
  }

  if (pressedScrollToggle) {
    if (!activeScroll) {
#ifdef DEBUG_BUTTONS
      DEBUG_PRINTLN("Rotary scrolling mode");
#endif
      // Display what's happening
      displayConfig(true, 2);
      display.setCursor(10, 0);
      display.println("Scrolling\n  mode");
      display.display();
      //  End display
      activeScroll = true;
    }
  } else if (!pressedScrollToggle && activeScroll) {
#ifdef DEBUG_BUTTONS
    DEBUG_PRINTLN("Axis sensitivity adjustment mode");
#endif
    // Display what's happening
    displayConfig(true, 2);
    display.setCursor(0, 0);
    display.println("X/Y adjust\n  mode");
    display.display();
    //  End display
    activeScroll = false;
  }

  if (pressedTrigger && activeGun) {
    if (!activeTrigger) {
#ifdef DEBUG_BUTTONS
      DEBUG_PRINTLN("Pressing left mouse button");
#endif
      // Display what's happening
      displayConfig(true, 3);
      display.setCursor(random(0, 40), random(0, 10));
      const char* buttonTriggerWords[] = {"BANG!", " POW!", "BOOM!", "BLAM!", "WHIZ!"};
      display.println(buttonTriggerWords[random(0, 4)]);
      display.display();
      //  End display
      Mouse.press();
      activeTrigger = true;
    }
  } else if (!pressedTrigger && activeTrigger) {
#ifdef DEBUG_BUTTONS
    DEBUG_PRINTLN("Releasing left mouse button");
#endif
    displayConfig(true);
    display.display();
    Mouse.release();
    activeTrigger = false;
  }

  if (pressedEncoder) {
    if (!activeEncoder) {
#ifdef DEBUG_BUTTONS
      DEBUG_PRINTLN("Encoder button pressed (Hold for aim adjust)");
#endif
      // Display what's happening
      displayConfig(true, 2);
      display.setCursor(5, 0);
      display.println("Adjusting\n    aim");
      display.display();
      //  End display
      activeEncoder = true;
    }
  } else if (!pressedEncoder && activeEncoder) {
#ifdef DEBUG_BUTTONS
    DEBUG_PRINT("Encoder button released");
    DEBUG_PRINT(" - Switching sensitivity adjustment to ");
    DEBUG_PRINTLN(currentAdjustmentAxis == 'X' ? "Y axis" : "X axis");
#endif
    if (currentAdjustmentAxis == 'X') {
      currentAdjustmentAxis = 'Y';
    } else {
      currentAdjustmentAxis = 'X';
    }
    // Display what's happening
    displayConfig(true);
    display.println(currentAdjustmentAxis == 'X' ? "  Adjusting X axis" : "  Adjusting Y axis");
    display.display();
    //  End display
    activeEncoder = false;
  }

  if (pressedAlt && activeGun) {
    if (!activeAlt && activeEncoder) {
      // If *both* the encode and alt buttons are pressed simultaneously, re-calibrate the gyro
      // mpu.calcGyroOffsets(true);
#ifdef DEBUG_BUTTONS
      DEBUG_PRINTLN("Re-calculating gyro offsets");
#endif
      // Display what's happening
      displayConfig(true, 2);
      display.setCursor(5, 0);
      display.println("Initialize\n   gyro");
      display.display();
      //  End display
      mpu.calcOffsets();   // gyro and accelerometer calibration
    } else if (!activeAlt) {
#ifdef DEBUG_BUTTONS
      DEBUG_PRINTLN("Pressing middle mouse button");
#endif
      // Display what's happening
      displayConfig(true, 2);
      display.setCursor(15, 0);
      display.println("Middle\n   click");
      display.display();
      //  End display
      Mouse.press(MOUSE_MIDDLE);
      activeAlt = true;
    }
  } else if (!pressedAlt && activeAlt) {
#ifdef DEBUG_BUTTONS
    DEBUG_PRINTLN("Releasing middle mouse button");
#endif
    displayConfig(true);
    display.display();
    Mouse.release(MOUSE_MIDDLE);
    activeAlt = false;
  }

  if (pressedReload && activeGun) {
    if (!activeReload) {
#ifdef DEBUG_BUTTONS
      DEBUG_PRINTLN("Pressing R");
#endif
      // Display what's happening
      displayConfig(true, 2);
      display.setCursor(10, 5);
      display.println("Reloading");
      display.display();
      //  End display
      Keyboard.press('r');
      activeReload = true;
    }
  } else if (!pressedReload && activeReload) {
#ifdef DEBUG_BUTTONS
    DEBUG_PRINTLN("Releasing R");
#endif
    displayConfig(true);
    display.display();
    Keyboard.release('r');
    activeReload = false;
  }

  if (pressedJoystick && activeGun) {
    if (!activeJoystick) {
#ifdef DEBUG_BUTTONS
      DEBUG_PRINTLN("Pressing right mouse button");
#endif
      // Display what's happening
      displayConfig(true, 2);
      display.setCursor(15, 0);
      display.println("Right\n   click");
      display.display();
      //  End display
      Mouse.click(MOUSE_RIGHT);
      activeJoystick = true;
    }
  } else if (!pressedJoystick && activeJoystick) {
#ifdef DEBUG_BUTTONS
    DEBUG_PRINTLN("Releasing right mouse button");
#endif
    displayConfig(true);
    display.display();
    Mouse.release(MOUSE_RIGHT);
    activeJoystick = false;
  }
}

// Handle joystick movements
void ProcessJoystick() {
  int joystickXValue = analogRead(joystickXPin);
  int joystickYValue = analogRead(joystickYPin);

  // Process X values (strafing movement)
  if (joystickXValue > 700 && activeGun) {
    if (!activeJoystickXplus) {
#ifdef DEBUG_JOYSTICK
      DEBUG_PRINTLN("Moving right");
#else
      // Display what's happening
      displayConfig(true, 2);
      display.setCursor(15, 0);
      display.println("Moving\n  right");
      display.display();
      //  End display
      Keyboard.release('a');
      Keyboard.press('d');
#endif
      activeJoystickXminus = false;
      activeJoystickXplus  = true;
    }
  } else if (joystickXValue < 300 && activeGun) {
    if (!activeJoystickXminus) {
#ifdef DEBUG_JOYSTICK
      DEBUG_PRINTLN("Moving left");
#else
      // Display what's happening
      displayConfig(true, 2);
      display.setCursor(15, 0);
      display.println("Moving\n  left");
      display.display();
      //  End display
      Keyboard.release('d');
      Keyboard.press('a');
#endif
      activeJoystickXplus  = false;
      activeJoystickXminus = true;
    }
  } else if (joystickXValue < 600 && joystickXValue > 400) {
    if (activeJoystickXminus || activeJoystickXplus) {
#ifdef DEBUG_JOYSTICK
      DEBUG_PRINTLN("Stopping X movement");
#else
      displayConfig(true);
      display.display();
      Keyboard.release('d');
      Keyboard.release('a');
#endif
      activeJoystickXplus  = false;
      activeJoystickXminus = false;
    }
  }

  // Process Y values (forward/reverse movement)
  if (joystickYValue > 700 && activeGun) {
    if (!activeJoystickYplus) {
#ifdef DEBUG_JOYSTICK
      DEBUG_PRINTLN("Moving forward");
#else
      // Display what's happening
      displayConfig(true, 2);
      display.setCursor(20, 5);
      display.println("Onward!");
      display.display();
      //  End display
      Keyboard.release('s');
      Keyboard.press('w');
#endif
      activeJoystickYminus = false;
      activeJoystickYplus  = true;
    }
  } else if (joystickYValue < 300 && activeGun) {
    if (!activeJoystickYminus) {
#ifdef DEBUG_JOYSTICK
      DEBUG_PRINTLN("Moving backwards");
#else
      // Display what's happening
      displayConfig(true, 2);
      display.setCursor(15, 5);
      display.println("Retreat!");
      display.display();
      //  End display
      Keyboard.release('w');
      Keyboard.press('s');
#endif
      activeJoystickYplus  = false;
      activeJoystickYminus = true;
    }
  } else if (joystickYValue < 600 && joystickYValue > 400) {
    if (activeJoystickYminus || activeJoystickYplus) {
#ifdef DEBUG_JOYSTICK
      DEBUG_PRINTLN("Stopping Y movement");
#else
      displayConfig(true);
      display.display();
      Keyboard.release('w');
      Keyboard.release('s');
#endif
      activeJoystickYplus  = false;
      activeJoystickYminus = false;
    }
  }
}

// Handle encoder turns
void ProcessEncoder() {
  uint8_t encoderPos = encoder.read();
  if (encoderPos && activeGun) {
    if (encoderPos == DIR_CCW) {
#ifdef DEBUG_ENCODER
      if (activeScroll) {
        DEBUG_PRINTLN("Scrolling down");
      } else {
        DEBUG_PRINT("Reducing ");
        DEBUG_PRINT(currentAdjustmentAxis);
        DEBUG_PRINT("-axis sensitivity to ");
        DEBUG_PRINTLN(currentAdjustmentAxis == 'X' ? mpuMovementMultiplierX - 1 : mpuMovementMultiplierY - 1);
      }
#endif
      if (activeScroll) {
        // Display what's happening
        displayConfig(true, 2);
        display.println("Scrolling\n   down");
        display.display();
        //  End display
        Mouse.move(0, 0, -1);
      } else {
        // Display what's happening
        displayConfig(true);
        display.println(currentAdjustmentAxis == 'X' ? "  Adjusting X axis" : "  Adjusting Y axis");
        display.println("--------------------");
        display.print(currentAdjustmentAxis == 'X' ? "X axis multiplier: " : "Y axis multiplier: ");
        display.println(currentAdjustmentAxis == 'X' ? mpuMovementMultiplierX - 1 : mpuMovementMultiplierY - 1);
        display.display();
        //  End display
        if (currentAdjustmentAxis == 'X') {
          mpuMovementMultiplierX -= 1;
        } else {
          mpuMovementMultiplierY -= 1;
        }
      }
    } else if (encoderPos == DIR_CW) {
#ifdef DEBUG_ENCODER
      if (activeScroll) {
        DEBUG_PRINTLN("Scrolling down");
      } else {
        DEBUG_PRINT("Increasing ");
        DEBUG_PRINT(currentAdjustmentAxis);
        DEBUG_PRINT("-axis sensitivity to ");
        DEBUG_PRINTLN(currentAdjustmentAxis == 'X' ? mpuMovementMultiplierX + 1 : mpuMovementMultiplierY + 1);
      }
#endif
      if (activeScroll) {
        // Display what's happening
        displayConfig(true, 2);
        display.println("Scrolling\n   up");
        display.display();
        //  End display
        Mouse.move(0, 0, 1);
      } else {
        // Display what's happening
        displayConfig(true);
        display.println(currentAdjustmentAxis == 'X' ? "  Adjusting X axis" : "  Adjusting Y axis");
        display.println("--------------------");
        display.print(currentAdjustmentAxis == 'X' ? "X axis multiplier: " : "Y axis multiplier: ");
        display.println(currentAdjustmentAxis == 'X' ? mpuMovementMultiplierX + 1 : mpuMovementMultiplierY + 1);
        display.display();
        //  End display
        if (currentAdjustmentAxis == 'X') {
          mpuMovementMultiplierX += 1;
        } else {
          mpuMovementMultiplierY += 1;
        }
      }
    }
  }
}

// Handle gyro movements (gun aiming)
void ProcessMPU() {
  static float mpuLastAngleX = 0, mpuLastAngleY = 0 /*, mpuLastAngleZ = 0*/;
  static float mpuAngleX = 0, mpuAngleY = 0, mpuAngleZ = 0;
  mpu.update();
  // fetch axis and convert to gun-specific axes
  mpuAngleX = -mpu.getAngleZ();   // horizontal axis
  mpuAngleY = mpu.getAngleY();    // vertical axis
  mpuAngleZ = mpu.getAngleX();    // lean
#ifdef DEBUG_GYRO
  if (timestamp >= debugLastUpdate + 2000) {   // only show debug info every 2 secs to avoid serial spam
    DEBUG_PRINT("Gryo Hori:");
    DEBUG_PRINT(mpuAngleX);
    DEBUG_PRINT(" Vert:");
    DEBUG_PRINT(mpuAngleY);
    DEBUG_PRINT(" Lean:");
    DEBUG_PRINTLN(mpuAngleZ);
  }
#endif
  bool mpuValidX = true, mpuValidY = true, mpuValidZ = true;

  // ignore if lean angle too shallow (ignore twitching) or too steep (assume gun laying down)
  if ((mpuAngleZ < 40 && mpuAngleZ > -40) || mpuAngleZ > 75 || mpuAngleZ < -75) {
    mpuValidZ = false;
  }

  int mouseMoveX = 0;
  if (mpuValidX) {
    if (!activeEncoder) mouseMoveX = (mpuAngleX - mpuLastAngleX) * mpuMovementMultiplierX;
    mpuLastAngleX = mpuAngleX;
  }
  int mouseMoveY = 0;
  if (mpuValidY) {
    if (!activeEncoder) mouseMoveY = (mpuAngleY - mpuLastAngleY) * mpuMovementMultiplierY;
    mpuLastAngleY = mpuAngleY;
  }

#ifdef DEBUG_GYRO
  if (timestamp >= debugLastUpdate + 2000) {   // only show debug info every 2 secs to avoid serial spam
    DEBUG_PRINT("Moving mouse X:");
    DEBUG_PRINT(mouseMoveX);
    DEBUG_PRINT(" and Y:");
    DEBUG_PRINT(mouseMoveY);
    DEBUG_PRINT(" -- Leaning:");
    // Display what's happening
    displayConfig(true);
    display.print("Moving mouse \nX:");
    display.print(mouseMoveX);
    display.print(" and Y:");
    display.print(mouseMoveY);
    display.println("\nLeaning:");
    //  End display
    if (mpuValidZ) {
      if (mpuAngleZ > 0) {
        DEBUG_PRINTLN("right");
        display.println("right");
      } else {
        DEBUG_PRINTLN("left");
        display.println("left");
      }
    } else {
      DEBUG_PRINTLN("none");
      display.println("none");
    }
    display.display();
    debugLastUpdate = timestamp;
  }
#else
  if ((mouseMoveX != 0 || mouseMoveY != 0) && activeGun) {
    Mouse.move(mouseMoveX, mouseMoveY, 0);
  }
  if (mpuValidZ && activeGun) {
    if (mpuAngleZ > 0) {
      Keyboard.press(KEY_PAGE_DOWN);
    } else {
      Keyboard.press(KEY_PAGE_UP);
    }
  } else {
    Keyboard.release(KEY_PAGE_DOWN);
    Keyboard.release(KEY_PAGE_UP);
  }
#endif
}

// Abort message upon initialization failure
void failsafe() {
  DEBUG_PRINTLN("Failsafe triggered. Code will not execute");
}
