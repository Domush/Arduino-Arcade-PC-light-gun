
/*
 *  Project     Arduino Arcade Light Gun
 *  @author     Edward Webber
 *  @link       github.com/domush/arduino-arcade-light-gun
 *  @license    GPLv3 - Copyright (c) 2021 Edward Webber
 */

#include <Mouse.h>
#include <Keyboard.h>
#include <Wire.h>
// #include <MPU6050.h>
// #include <MPU6050_tockn.h>
#include <MPU6050_light.h>
#include <MD_REncoder.h>

// -------- User Settings --------
// Tuning Options
// const long encoderUpdateRate = 5;    // In milliseconds
const long mpuUpdateRate = 5;   // In milliseconds

// multiplier converts gun movement into appropriate mouse movement
int mpuMovementMultiplierX = 50, mpuMovementMultiplierY = 48;

// Debug Flags (uncomment to dd)
#define DEBUG   // Enable to use any prints
#ifdef DEBUG
  #define DEBUG_BUTTONS
// #define DEBUG_GYRO
// #define DEBUG_ENCODER
#endif

// Pin Definitions (do not change)
const uint8_t buttonTriggerPin = 14;
const uint8_t buttonAltPin     = 16;
const uint8_t buttonReloadPin  = 10;
const uint8_t buttonEncoderPin = 7;
const uint8_t encoderDTPin     = 6;
const uint8_t encoderCLKPin    = 5;

// Global Variables
long timestamp;
long mpuLastUpdate;
long encoderLastUpdate;
char currentAdjustmentAxis = 'X';

boolean activeTrigger = false;
boolean activeAlt     = false;
boolean activeReload  = false;
boolean activeEncoder = false;

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
// encoder
MD_REncoder encoder = MD_REncoder(encoderCLKPin, encoderDTPin);

// MPU6050 gryo
MPU6050 mpu = MPU6050(Wire);

void setup() {
#ifdef DEBUG
  Serial.begin(115200);
#endif
  while (!Serial)
    ;   // Wait for connection

  pinMode(buttonTriggerPin, INPUT_PULLUP);
  pinMode(buttonAltPin, INPUT_PULLUP);
  pinMode(buttonReloadPin, INPUT_PULLUP);
  pinMode(buttonEncoderPin, INPUT_PULLUP);

  if (digitalRead(buttonTriggerPin) == 0) {
    failsafe();   // Just in-case something goes wrong
  }
  Mouse.begin();
  Keyboard.begin();
  encoder.begin();
  Wire.begin();
  mpu.begin();
  // mpu.calcGyroOffsets(true);
  mpu.calcOffsets();   // gyro and accelero
}

void loop() {
  timestamp = millis();
  if (timestamp >= mpuLastUpdate + mpuUpdateRate) {
    mpuLastUpdate = timestamp;
    ProcessMPU();
  }
  // if (timestamp >= encoderLastUpdate + encoderUpdateRate) {
  // encoderLastUpdate = timestamp;
  ProcessEncoder();
  // }

  ProcessButtons();
}

void ProcessButtons() {
  boolean pressedTrigger = !digitalRead(buttonTriggerPin);
  boolean pressedAlt     = !digitalRead(buttonAltPin);
  boolean pressedReload  = !digitalRead(buttonReloadPin);
  boolean pressedEncoder = !digitalRead(buttonEncoderPin);

  if (pressedTrigger) {
    if (!activeTrigger && activeEncoder) {
      // if encode and trigger buttons both pressed, calibrate gyro
      // mpu.calcGyroOffsets(true);
      mpu.calcOffsets();   // gyro and accelero

    } else if (!activeTrigger) {
#ifdef DEBUG_BUTTONS
      DEBUG_PRINTLN("Pressing left mouse button");
#endif
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

  if (pressedAlt) {
    if (!activeAlt) {
#ifdef DEBUG_BUTTONS
      DEBUG_PRINTLN("Pressing right mouse button");
#endif
      Mouse.press(MOUSE_RIGHT);
      activeAlt = true;
    }
  } else if (!pressedAlt && activeAlt) {
#ifdef DEBUG_BUTTONS
    DEBUG_PRINTLN("Releasing right mouse button");
#endif
    Mouse.release(MOUSE_RIGHT);
    activeAlt = false;
  }

  if (pressedReload) {
    if (!activeReload) {
#ifdef DEBUG_BUTTONS
      DEBUG_PRINTLN("Switching axis");
      // DEBUG_PRINTLN("Pressing R");
#endif
      if (currentAdjustmentAxis == 'X') {
        currentAdjustmentAxis = 'Y';
      } else {
        currentAdjustmentAxis = 'X';
      }
      // Keyboard.press('r');
      activeReload = true;
    }
  } else if (!pressedReload && activeReload) {
#ifdef DEBUG_BUTTONS
    DEBUG_PRINTLN("Switching axis");
    // DEBUG_PRINTLN("Releasing R");
#endif
    // Keyboard.release('r');
    activeReload = false;
  }

  if (pressedEncoder) {
    if (!activeEncoder) {
#ifdef DEBUG_BUTTONS
      DEBUG_PRINTLN("Encoder button pressed");
#endif
      activeEncoder = true;
    }
  } else if (!pressedEncoder && activeEncoder) {
#ifdef DEBUG_BUTTONS
    DEBUG_PRINTLN("Encoder button released");
#endif
    activeEncoder = false;
  }
}

void ProcessEncoder() {
  uint8_t encoderPos = encoder.read();
  if (encoderPos) {
#ifdef DEBUG_ENCODER
    DEBUG_PRINT(encoderPos == DIR_CW ? "\n+1" : "\n-1");
  #if ENABLE_SPEED
    DEBUG_PRINT(" Rate: ");
    DEBUG_PRINTLN(encoder.speed());
  #else
    DEBUG_PRINTLN("");
  #endif
#endif
    if (encoderPos == DIR_CCW) {
#ifdef DEBUG
      DEBUG_PRINT("Reducing ");
      DEBUG_PRINT(currentAdjustmentAxis);
      DEBUG_PRINT("-axis sensitivity to ");
      DEBUG_PRINTLN(currentAdjustmentAxis == 'X' ? mpuMovementMultiplierX - 1 : mpuMovementMultiplierY - 1);
      // DEBUG_PRINTLN("Scrolling down");
#endif
      if (currentAdjustmentAxis == 'X') {
        mpuMovementMultiplierX -= 1;
      } else {
        mpuMovementMultiplierY -= 1;
      }
      // Mouse.move(0, 0, -1);
      // Keyboard.press(KEY_DOWN_ARROW);
      // Keyboard.release(KEY_DOWN_ARROW);
    } else if (encoderPos == DIR_CW) {
#ifdef DEBUG
      DEBUG_PRINT("Increasing ");
      DEBUG_PRINT(currentAdjustmentAxis);
      DEBUG_PRINT("-axis sensitivity to ");
      DEBUG_PRINTLN(currentAdjustmentAxis == 'X' ? mpuMovementMultiplierX + 1 : mpuMovementMultiplierY + 1);
      // DEBUG_PRINTLN("Scrolling up");
#endif
      if (currentAdjustmentAxis == 'X') {
        mpuMovementMultiplierX += 1;
      } else {
        mpuMovementMultiplierY += 1;
      }
      // Mouse.move(0, 0, 1);
      // Keyboard.press(KEY_UP_ARROW);
      // Keyboard.release(KEY_UP_ARROW);
    }
  }
}

void ProcessMPU() {
  static float mpuLastAngleX = 0, mpuLastAngleY = 0, mpuLastAngleZ = 0;
  static float mpuAngleX = 0, mpuAngleY = 0, mpuAngleZ = 0;
  mpu.update();
  // fetch axis and convert to gun-specific axes
  mpuAngleX = -mpu.getAngleZ();   // horizontal axis
  mpuAngleY = mpu.getAngleX();    // vertical axis
  mpuAngleZ = -mpu.getAngleY();   // lean
#ifdef DEBUG_GYRO
  DEBUG_PRINT("Gryo Hori:");
  DEBUG_PRINT(mpuAngleX);
  DEBUG_PRINT(" Vert:");
  DEBUG_PRINT(mpuAngleY);
  DEBUG_PRINT(" Lean:");
  DEBUG_PRINTLN(mpuAngleZ);
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
  DEBUG_PRINT("Moving mouse X:");
  DEBUG_PRINT(mouseMoveX);
  DEBUG_PRINT(" and Y:");
  DEBUG_PRINT(mouseMoveY);
  DEBUG_PRINT(" -- Leaning:");
  if (mpuValidZ) {
    if (mpuAngleZ > 0) {
      DEBUG_PRINTLN("right");
    } else {
      DEBUG_PRINTLN("left");
    }
  } else {
    DEBUG_PRINTLN("none");
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

void failsafe() {
  DEBUG_PRINTLN("Failsafe triggered. Code will not execute");
}
