// STM32 entry point. The calibration console and its command/record behaviour
// live in the shared Arduino sketch; the selected backend supplies the six
// hardware PWM outputs. The Arduino core calls the setup()/loop() defined there.
#include "../../servo-calibration/servo-calibration.ino"
