/*
 *  Project     Arduino Arcade Light Gun
 *  @author     Edward Webber
 *  @link       github.com/domush/arduino-arcade-light-gun
 *  @license    GPLv3 - Copyright (c) 2021 Edward Webber
 */

#include <MPU6050_tockn.h>
#include <Wire.h>
#include <Mouse.h>
#include <Keyboard.h>
#include <MD_REncoder.h>

// -------- User Settings --------
// Tuning Options
const long encoderUpdateRate = 100;   // In milliseconds
const long mpuUpdateRate     = 5;     // In milliseconds

// Debug Flags (uncomment to dd)
#define DEBUG   // Enable to use any prints
// #define DEBUG_ACCEL
// #define DEBUG_GYRO
// #define DEBUG_AIMING

// Pin Definitions (do not change)
const uint8_t buttonTriggerPin = 14;
const uint8_t buttonAltPin     = 16;
const uint8_t buttonReloadPin  = 10;
const uint8_t buttonEncoderPin = 5;
const uint8_t encoderDTPin     = 6;
const uint8_t encoderCLKPin    = 7;

// Global Variables
long timestamp;
long mpuLastUpdate;
long encoderLastUpdate;

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
  while (!Serial)
    ;   // Wait for connection
#endif

  pinMode(buttonTriggerPin, INPUT_PULLUP);
  pinMode(buttonAltPin, INPUT_PULLUP);
  pinMode(buttonReloadPin, INPUT_PULLUP);
  pinMode(buttonEncoderPin, INPUT_PULLUP);

  if (digitalRead(buttonTriggerPin) == 0) {
    failsafe();   // Just in-case something goes wrong
  }
  encoder.begin();
  Mouse.begin();
  Wire.begin();
  mpu.begin();
  mpu.calcGyroOffsets(true);
}

void loop() {
  timestamp = millis();
  if (timestamp >= mpuLastUpdate + mpuUpdateRate) {
    mpuLastUpdate = timestamp;
    ProcessMPU();
  }
  if (timestamp >= encoderLastUpdate + encoderUpdateRate) {
    encoderLastUpdate = timestamp;
    ProcessEncoder();
  }

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
      mpu.calcGyroOffsets(true);
    } else if (!activeTrigger) {
      Mouse.press();
      activeTrigger = true;
    }
  } else if (!pressedTrigger && activeTrigger) {
    Mouse.release();
    activeTrigger = false;
  }

  if (pressedAlt) {
    if (!activeAlt) {
      Mouse.press(MOUSE_RIGHT);
      activeAlt = true;
    }
  } else if (!pressedAlt && activeAlt) {
    Mouse.release(MOUSE_RIGHT);
    activeAlt = false;
  }

  if (pressedReload) {
    if (!activeReload) {
      Keyboard.press('r');
      activeReload = true;
    }
  } else if (!pressedReload && activeReload) {
    Keyboard.release('r');
    activeReload = false;
  }

  if (pressedEncoder) {
    if (!activeEncoder) {
      activeEncoder = true;
    }
  } else if (!pressedEncoder && activeEncoder) {
    activeEncoder = false;
  }
}

void ProcessEncoder() {
  uint8_t encoderPos = encoder.read();
  if (encoderPos) {
    DEBUG_PRINT(encoderPos == DIR_CW ? "\n+1" : "\n-1");
#if ENABLE_SPEED
    DEBUG_PRINT(" Rate: ");
    DEBUG_PRINTLN(encoder.speed());
#else
    DEBUG_PRINTLN("");
#endif
    if (encoderPos == DIR_CW) {
      Keyboard.press(KEY_DOWN_ARROW);
      Keyboard.release(KEY_DOWN_ARROW);
    } else if (encoderPos == DIR_CW) {
      Keyboard.press(KEY_UP_ARROW);
      Keyboard.release(KEY_UP_ARROW);
    }
  }
}

void ProcessMPU() {
  static float mpuLastAngleX = 0, mpuLastAngleY = 0, mpuLastAngleZ = 0;
  static float mpuAngleX = 0, mpuAngleY = 0, mpuAngleZ = 0;
  mpu.update();
  mpuAngleX = mpu.getAngleX();
  mpuAngleY = mpu.getAngleY();
  mpuAngleZ = mpu.getAngleZ();
  DEBUG_PRINT("angleX : ");
  DEBUG_PRINT(mpuAngleX);
  DEBUG_PRINT("\tangleY : ");
  DEBUG_PRINT(mpuAngleY);
  DEBUG_PRINT("\tangleZ : ");
  DEBUG_PRINTLN(mpuAngleZ);
  if (mpuAngleX < 2 || mpuAngleX > 50) {
    mpuAngleX == 0;
  }
  if (mpuAngleZ < 2 || mpuAngleZ > 50) {
    mpuAngleZ == 0;
  }
  int mouseMoveX = mpuAngleX * 3;
  int mouseMoveZ = mpuAngleZ * 3;
  Mouse.move(mouseMoveX, mouseMoveZ, 0);
}

void failsafe() {
  DEBUG_PRINTLN(F("Failsafe triggered. Code will not execute"));
}
