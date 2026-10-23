#pragma once

#include <Arduino.h>
#include <HardwareTimer.h>

// STM32 NUCLEO-F767ZI bench backend.
//
// Each unit ID has its own fixed output so the console can be used without
// rewiring as units are selected. These are bench connection IDs, not a claim
// that the unit belongs to a particular arm joint.
//
//   unit 1 -> PC6  TIM3_CH1
//   unit 2 -> PC7  TIM3_CH2
//   unit 3 -> PD12 TIM4_CH1
//   unit 4 -> PD13 TIM4_CH2
//   unit 5 -> PD14 TIM4_CH3
//   unit 6 -> PD15 TIM4_CH4
//
// Both timers count in 1 us steps with a 20 ms period (50 Hz). A compare value
// is therefore the pulse width in microseconds. Only one channel is ever
// started, and a detach stops the channel and drives the pin low.

static const uint32_t SERVO_BACKEND_PERIOD_US = 20000;

struct ServoBackendOutput {
  PinName pinName;
  pin_size_t digitalPin;
  HardwareTimer *timer;
  uint32_t channel;
  const char *name;
};

static HardwareTimer servoBackendTimer3;
static HardwareTimer servoBackendTimer4;

static const ServoBackendOutput SERVO_BACKEND_OUTPUTS[6] = {
  {PC_6,  PC6,  &servoBackendTimer3, 1, "PC6 (TIM3_CH1)"},
  {PC_7,  PC7,  &servoBackendTimer3, 2, "PC7 (TIM3_CH2)"},
  {PD_12, PD12, &servoBackendTimer4, 1, "PD12 (TIM4_CH1)"},
  {PD_13, PD13, &servoBackendTimer4, 2, "PD13 (TIM4_CH2)"},
  {PD_14, PD14, &servoBackendTimer4, 3, "PD14 (TIM4_CH3)"},
  {PD_15, PD15, &servoBackendTimer4, 4, "PD15 (TIM4_CH4)"},
};

static int8_t servoBackendAttached = -1;
static bool servoBackendStarted = false;
static bool servoBackendReady = false;

static inline const ServoBackendOutput &servoBackendOutput(uint8_t index) {
  return SERVO_BACKEND_OUTPUTS[index];
}

static inline void servoBackendDriveLow(const ServoBackendOutput &output) {
  pinMode(output.digitalPin, OUTPUT);
  digitalWrite(output.digitalPin, LOW);
}

static inline void servoBackendSetupTimer(HardwareTimer &timer,
                                          TIM_TypeDef *instance) {
  timer.setup(instance);

  // Make one timer tick equal one microsecond, then 20000 ticks equal 20 ms.
  uint32_t ticksPerUs = timer.getTimerClkFreq() / 1000000U;
  if (ticksPerUs == 0U) ticksPerUs = 1U;
  timer.setPrescaleFactor(ticksPerUs);
  timer.setOverflow(SERVO_BACKEND_PERIOD_US, TICK_FORMAT);
}

static inline void servoBackendBegin() {
  // Startup order: pins low first, then configure timers. Neither timer is
  // started and no channel is configured, so no PWM leaves the board.
  for (uint8_t i = 0; i < 6; ++i) {
    servoBackendDriveLow(servoBackendOutput(i));
  }

  servoBackendSetupTimer(servoBackendTimer3, TIM3);
  servoBackendSetupTimer(servoBackendTimer4, TIM4);

  servoBackendAttached = -1;
  servoBackendStarted = false;
  servoBackendReady = true;
}

static inline const char *servoBackendBoardName() {
  return "NUCLEO-F767ZI";
}

static inline const char *servoBackendPinSummary() {
  return "1=PC6/TIM3_CH1 2=PC7/TIM3_CH2 3=PD12/TIM4_CH1 "
         "4=PD13/TIM4_CH2 5=PD14/TIM4_CH3 6=PD15/TIM4_CH4";
}

static inline const char *servoBackendPinName(uint8_t index) {
  return servoBackendOutput(index).name;
}

// The STM32 output path starts the timer with the target compare value already
// loaded, so it needs no interrupt masking.
static inline void servoBackendEnterCritical() {}
static inline void servoBackendExitCritical() {}

static inline void servoBackendDetach(uint8_t index);

static inline void servoBackendAttach(uint8_t index, int minUs, int maxUs) {
  (void)minUs;
  (void)maxUs;
  if (!servoBackendReady || index >= 6) return;

  // Defensive: the console arms only one unit at a time, and the backend keeps
  // the same rule if it is called directly.
  if (servoBackendAttached >= 0) {
    servoBackendDetach((uint8_t)servoBackendAttached);
  }

  const ServoBackendOutput &output = servoBackendOutput(index);
  servoBackendDriveLow(output);
  servoBackendAttached = (int8_t)index;
  servoBackendStarted = false;
}

static inline void servoBackendWrite(uint8_t index, int pulseUs) {
  if (!servoBackendReady || index >= 6 || servoBackendAttached != (int8_t)index) {
    return;
  }

  const ServoBackendOutput &output = servoBackendOutput(index);
  if (!servoBackendStarted) {
    // Configure the channel and load the first compare value before starting
    // the timer. The first pulse is therefore the requested pulse, never a
    // default or stale width.
    output.timer->setMode(output.channel, TIMER_OUTPUT_COMPARE_PWM1,
                          output.pinName);
    output.timer->setCaptureCompare(output.channel, (uint32_t)pulseUs,
                                    TICK_COMPARE_FORMAT);
    output.timer->setCount(0);
    output.timer->resumeChannel(output.channel);
    servoBackendStarted = true;
  } else {
    output.timer->setCaptureCompare(output.channel, (uint32_t)pulseUs,
                                    TICK_COMPARE_FORMAT);
  }
}

static inline void servoBackendDetach(uint8_t index) {
  if (!servoBackendReady || index >= 6) return;

  const ServoBackendOutput &output = servoBackendOutput(index);
  output.timer->pauseChannel(output.channel);
  if (servoBackendAttached == (int8_t)index) {
    if (servoBackendStarted) output.timer->pause();
    servoBackendAttached = -1;
    servoBackendStarted = false;
  }
  output.timer->setCaptureCompare(output.channel, 0, TICK_COMPARE_FORMAT);
  servoBackendDriveLow(output);
}
