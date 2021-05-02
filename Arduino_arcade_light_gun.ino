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
int mpuMovementMultiplierX = 50, mpuMovementMultiplierY = 48;

// Debug Flags (uncomment to display comments via serial connection)
#define DEBUG   // Enabling will wait for serial connection before activating
#ifdef DEBUG
  #define DEBUG_BUTTONS   // Show button presses (and what they do)
  // #define DEBUG_GYRO // Show mouse movements (very spammy, use with caution)
  #define DEBUG_ENCODER    // Show encoder movements
  #define DEBUG_JOYSTICK   // Show joystick movements
#endif

// Pin Definitions (change with care)
const uint8_t buttonTriggerPin  = 14;
const uint8_t buttonAltPin      = 16;
const uint8_t buttonReloadPin   = 10;
const uint8_t encoderDTPin      = 6;
const uint8_t encoderCLKPin     = 5;
const uint8_t buttonEncoderPin  = 7;
const uint8_t joystickXPin      = A0;
const uint8_t joystickYPin      = A1;
const uint8_t buttonJoystickPin = 15;

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
// #include <MPU6050.h>
// #include <MPU6050_tockn.h>
#include <Adafruit_SSD1306.h>
#include <splash.h>
// #include <OakOLED.h>

// Global Variables
long timestamp;
long mpuLastUpdate;
long debugLastUpdate;
char currentAdjustmentAxis = 'X';

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
#define SCREEN_HEIGHT 64    // OLED display height, in pixels

#define OLED_RESET     -1     // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x78   ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
// Adafruit_SSD1306 display(128, 64, &Wire, -1);
#define LOGO_HEIGHT 16
#define LOGO_WIDTH  16
static const unsigned char PROGMEM logo_bmp[] =
    {B00000000, B11000000,
     B00000001, B11000000,
     B00000001, B11000000,
     B00000011, B11100000,
     B11110011, B11100000,
     B11111110, B11111000,
     B01111110, B11111111,
     B00110011, B10011111,
     B00011111, B11111100,
     B00001101, B01110000,
     B00011011, B10100000,
     B00111111, B11100000,
     B00111111, B11110000,
     B01111100, B11110000,
     B01110000, B01110000,
     B00000000, B00110000};

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
  boolean pressedTrigger  = !digitalRead(buttonTriggerPin);
  boolean pressedAlt      = !digitalRead(buttonAltPin);
  boolean pressedReload   = !digitalRead(buttonReloadPin);
  boolean pressedJoystick = !digitalRead(buttonJoystickPin);
  boolean pressedEncoder  = !digitalRead(buttonEncoderPin);

  if (pressedTrigger) {
    if (!activeTrigger) {
#ifdef DEBUG_BUTTONS
      DEBUG_PRINTLN("Pressing left mouse button");
#endif
      // Display what's happening
      displayConfig(true, 3);
      display.println("BANG!");
      display.display();
      //  End display
      Mouse.press();
      activeTrigger = true;
    }
  } else if (!pressedTrigger && activeTrigger) {
#ifdef DEBUG_BUTTONS
    DEBUG_PRINTLN("Releasing left mouse button");
#endif
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
      display.println("Adjusting aim");
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
    displayConfig();
    display.println(currentAdjustmentAxis == 'X' ? "Adjusting X axis" : "Adjusting Y axis");
    display.display();
    //  End display
    activeEncoder = false;
  }

  if (pressedAlt) {
    if (!activeAlt && activeEncoder) {
      // If *both* the encode and alt buttons are pressed simultaneously, re-calibrate the gyro
      // mpu.calcGyroOffsets(true);
      mpu.calcOffsets();   // gyro and accelerometer calibration
    } else if (!activeAlt) {
#ifdef DEBUG_BUTTONS
      DEBUG_PRINTLN("Pressing middle mouse button");
      // Display what's happening
      displayConfig(true, 2);
      display.println("Middle click");
      display.display();
      //  End display
#endif
      Mouse.press(MOUSE_MIDDLE);
      activeAlt = true;
    }
  } else if (!pressedAlt && activeAlt) {
#ifdef DEBUG_BUTTONS
    DEBUG_PRINTLN("Releasing middle mouse button");
#endif
    Mouse.release(MOUSE_MIDDLE);
    activeAlt = false;
  }

  if (pressedReload) {
    if (!activeReload) {
#ifdef DEBUG_BUTTONS
      DEBUG_PRINTLN("Pressing R");
#endif
      // Display what's happening
      displayConfig(true, 2);
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
    Keyboard.release('r');
    activeReload = false;
  }

  if (pressedJoystick) {
    if (!activeJoystick) {
#ifdef DEBUG_BUTTONS
      DEBUG_PRINTLN("Pressing right mouse button");
      // Display what's happening
      displayConfig(true, 2);
      display.println("Right click");
      display.display();
      //  End display
#endif
      Mouse.click(MOUSE_RIGHT);
      activeJoystick = true;
    }
  } else if (!pressedJoystick && activeJoystick) {
#ifdef DEBUG_BUTTONS
    DEBUG_PRINTLN("Releasing right mouse button");
#endif
    Mouse.release(MOUSE_RIGHT);
    activeJoystick = false;
  }
}

// Handle joystick movements
void ProcessJoystick() {
  int joystickXValue = analogRead(joystickXPin);
  int joystickYValue = analogRead(joystickYPin);

  // Process X values (strafing movement)
  if (joystickXValue > 700) {
    if (!activeJoystickXplus) {
#ifdef DEBUG_JOYSTICK
      DEBUG_PRINTLN("Moving right");
      // Display what's happening
      displayConfig(true, 2);
      display.println("Moving right");
      display.display();
      //  End display
#else
      Keyboard.release('A');
      Keyboard.press('D');
#endif
      activeJoystickXminus = false;
      activeJoystickXplus  = true;
    }
  } else if (joystickXValue < 300) {
    if (!activeJoystickXminus) {
#ifdef DEBUG_JOYSTICK
      DEBUG_PRINTLN("Moving left");
      // Display what's happening
      displayConfig(true, 2);
      display.println("Moving left");
      display.display();
      //  End display
#else
      Keyboard.release('D');
      Keyboard.press('A');
#endif
      activeJoystickXplus  = false;
      activeJoystickXminus = true;
    }
  } else if (joystickXValue < 600 && joystickXValue > 400) {
    if (activeJoystickXminus || activeJoystickXplus) {
#ifdef DEBUG_JOYSTICK
      DEBUG_PRINTLN("Stopping X movement");
#else
      Keyboard.release('D');
      Keyboard.release('A');
#endif
      activeJoystickXplus  = false;
      activeJoystickXminus = false;
    }
  }

  // Process Y values (forward/reverse movement)
  if (joystickYValue > 700) {
    if (!activeJoystickYplus) {
#ifdef DEBUG_JOYSTICK
      DEBUG_PRINTLN("Moving forward");
      // Display what's happening
      displayConfig(true, 2);
      display.println("Moving forward");
      display.display();
      //  End display
#else
      Keyboard.release('S');
      Keyboard.press('W');
#endif
      activeJoystickYminus = false;
      activeJoystickYplus  = true;
    }
  } else if (joystickYValue < 300) {
    if (!activeJoystickYminus) {
#ifdef DEBUG_JOYSTICK
      DEBUG_PRINTLN("Moving backwards");
      // Display what's happening
      displayConfig(true, 2);
      display.println("Moving backwards");
      display.display();
      //  End display
#else
      Keyboard.release('W');
      Keyboard.press('S');
#endif
      activeJoystickYplus  = false;
      activeJoystickYminus = true;
    }
  } else if (joystickYValue < 600 && joystickYValue > 400) {
    if (activeJoystickYminus || activeJoystickYplus) {
#ifdef DEBUG_JOYSTICK
      DEBUG_PRINTLN("Stopping Y movement");
#else
      Keyboard.release('W');
      Keyboard.release('S');
#endif
      activeJoystickYplus  = false;
      activeJoystickYminus = false;
    }
  }
}

// Handle encoder turns
void ProcessEncoder() {
  uint8_t encoderPos = encoder.read();
  if (encoderPos) {
    if (encoderPos == DIR_CCW) {
#ifdef DEBUG_ENCODER
      DEBUG_PRINT("Reducing ");
      DEBUG_PRINT(currentAdjustmentAxis);
      DEBUG_PRINT("-axis sensitivity to ");
      DEBUG_PRINTLN(currentAdjustmentAxis == 'X' ? mpuMovementMultiplierX - 1 : mpuMovementMultiplierY - 1);
      // Display what's happening
      displayConfig();
      display.print(currentAdjustmentAxis == 'X' ? "X axis mult: " : "Y axis mult: ");
      display.println(currentAdjustmentAxis == 'X' ? mpuMovementMultiplierX - 1 : mpuMovementMultiplierY - 1);
      display.display();
      //  End display
      // DEBUG_PRINTLN("Scrolling down");
#endif
      if (currentAdjustmentAxis == 'X') {
        mpuMovementMultiplierX -= 1;
      } else {
        mpuMovementMultiplierY -= 1;
      }
      // Mouse.move(0, 0, -1);
    } else if (encoderPos == DIR_CW) {
#ifdef DEBUG_ENCODER
      DEBUG_PRINT("Increasing ");
      DEBUG_PRINT(currentAdjustmentAxis);
      DEBUG_PRINT("-axis sensitivity to ");
      DEBUG_PRINTLN(currentAdjustmentAxis == 'X' ? mpuMovementMultiplierX + 1 : mpuMovementMultiplierY + 1);
      // Display what's happening
      displayConfig();
      display.print(currentAdjustmentAxis == 'X' ? "X axis mult: " : "Y axis mult: ");
      display.println(currentAdjustmentAxis == 'X' ? mpuMovementMultiplierX + 1 : mpuMovementMultiplierY + 1);
      display.display();
      //  End display
      // DEBUG_PRINTLN("Scrolling up");
#endif
      if (currentAdjustmentAxis == 'X') {
        mpuMovementMultiplierX += 1;
      } else {
        mpuMovementMultiplierY += 1;
      }
      // Mouse.move(0, 0, 1);
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
  mpuAngleY = mpu.getAngleX();    // vertical axis
  mpuAngleZ = -mpu.getAngleY();   // lean
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
  // ignore if horizontal movement too shallow (small twitches) or too steep (no longer aiming at screen)
  // if (mpuAngleX < .1 && mpuAngleX > -.1 /* || mpuAngleX > 60 || mpuAngleX < -60 */) {
  // mpuAngleX = 0;
  // mpuValidX = false;
  // }
  // ignore if vertical movement too shallow (small twitches) or too steep (no longer aiming at screen)
  // if (mpuAngleY < .1 && mpuAngleY > -.1 /* || mpuAngleY > 45 || mpuAngleY < -45 */) {
  // mpuAngleY = 0;
  // mpuValidY = false;
  // }
  // ignore if lean angle too shallow or too steep
  if (mpuAngleZ < 40 && mpuAngleZ > -40 /* || mpuAngleZ > 85 || mpuAngleZ < -85 */) {
    // mpuAngleZ = 0;
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
// mpuLastAngleZ = mpuAngleZ;
#ifdef DEBUG_GYRO
  if (timestamp >= debugLastUpdate + 2000) {   // only show debug info every 2 secs to avoid serial spam
    DEBUG_PRINT("Moving mouse X:");
    DEBUG_PRINT(mouseMoveX);
    DEBUG_PRINT(" and Y:");
    DEBUG_PRINT(mouseMoveY);
    DEBUG_PRINT(" -- Leaning:");
    // Display what's happening
    displayConfig(true);
    display.println("Moving mouse X:");
    display.println(mouseMoveX);
    display.println(" and Y:");
    display.println(mouseMoveY);
    display.println(" -- Leaning:");
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
  if (mouseMoveX != 0 || mouseMoveY != 0) {
    Mouse.move(mouseMoveX, mouseMoveY, 0);
  }
  if (mpuValidZ) {
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
