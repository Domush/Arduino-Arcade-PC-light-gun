/*
 *  Project     Arduino Arcade Light Gun
 *  @author     Edward Webber
 *  @link       github.com/domush/arduino-arcade-light-gun
 *  @license    GPLv3 - Copyright (c) 2021 Edward Webber
 */

#include <Mouse.h>
#include <Keyboard.h>
#include <Wire.h>
#include <MPU6050_tockn.h>
#include <MD_REncoder.h>

// -------- User Settings --------
// Tuning Options
const long encoderUpdateRate = 100;    // In milliseconds
const long mpuUpdateRate     = 1000;   // In milliseconds

// Debug Flags (uncomment to dd)
#define DEBUG   // Enable to use any prints
#ifdef DEBUG
  #define DEBUG_BUTTONS
  #define DEBUG_GYRO
  #define DEBUG_ENCODER
#endif

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
  Mouse.begin();
  Keyboard.begin();
  encoder.begin();
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
      DEBUG_PRINTLN("Pressing R");
#endif
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
    if (encoderPos == DIR_CW) {
#ifdef DEBUG_ENCODER
      DEBUG_PRINTLN("Pressing down arrow");
#endif
      Keyboard.press(KEY_DOWN_ARROW);
      Keyboard.release(KEY_DOWN_ARROW);
    } else if (encoderPos == DIR_CW) {
#ifdef DEBUG_ENCODER
      DEBUG_PRINTLN("Pressing up arrow");
#endif
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
#ifdef DEBUG_GYRO
  DEBUG_PRINT("Gryo X:");
  DEBUG_PRINT(mpuAngleX);
  DEBUG_PRINT(" Y:");
  DEBUG_PRINT(mpuAngleY);
  DEBUG_PRINT(" Z:");
  DEBUG_PRINTLN(mpuAngleZ);
#endif
  if (mpuAngleX < 2 && mpuAngleX > -2 || mpuAngleX > 50 || mpuAngleX < -50) {
    mpuAngleX == 0;
  }
  if (mpuAngleZ < 2 && mpuAngleZ > -2 || mpuAngleZ > 50 || mpuAngleZ < -50) {
    mpuAngleZ == 0;
  }
  int mouseMoveX = mpuAngleX * 3;
  int mouseMoveZ = mpuAngleZ * 3;
#ifdef DEBUG_GYRO
  DEBUG_PRINT("Moving mouse X:");
  DEBUG_PRINT(mouseMoveX);
  DEBUG_PRINT(" and Y:");
  DEBUG_PRINTLN(mouseMoveZ);
#else
  Mouse.move(mouseMoveX, mouseMoveZ, 0);
#endif
}

void failsafe() {
  DEBUG_PRINTLN("Failsafe triggered. Code will not execute");
}
