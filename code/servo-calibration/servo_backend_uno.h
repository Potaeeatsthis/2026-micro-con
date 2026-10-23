#pragma once

#include <Arduino.h>
#include <Servo.h>

// Arduino Uno bench backend: all six unit IDs share pin 9 because only one
// physical servo is connected at a time.
static const uint8_t SERVO_BACKEND_PIN = 9;

static Servo servoBackendServos[6];

static inline void servoBackendBegin() {}

static inline const char *servoBackendBoardName() { return "Arduino Uno"; }

static inline const char *servoBackendPinSummary() {
  return "unit 1..6 -> pin 9 (one servo at a time)";
}

static inline const char *servoBackendPinName(uint8_t) { return "pin 9"; }

// Keep the Servo ISR away from attach + first write. The console calls this
// around exactly that pair so no default or stale pulse is emitted.
static inline void servoBackendEnterCritical() { noInterrupts(); }
static inline void servoBackendExitCritical() { interrupts(); }

static inline void servoBackendAttach(uint8_t index, int minUs, int maxUs) {
  servoBackendServos[index].attach(SERVO_BACKEND_PIN, minUs, maxUs);
}

static inline void servoBackendWrite(uint8_t index, int pulseUs) {
  servoBackendServos[index].writeMicroseconds(pulseUs);
}

static inline void servoBackendDetach(uint8_t index) {
  servoBackendServos[index].detach();
}
