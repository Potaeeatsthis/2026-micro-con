# Autonomous Robotic Arm

A planned six-joint robotic arm that finds a coloured block, picks it up and places it in a target box. This is a third-year microcontroller project.

## Four control modes

| Mode | What you do |
|---|---|
| **Auto/Voice** | Say a fixed command; the camera finds the block and the arm moves it. |
| **Cartesian** | Use the KY-023 joystick for X/Y and the KY-040 knob for Z. |
| **Joint Group** | Use the joystick to move two selected joints. |
| **Single-Joint Service** | Select and test one joint at a time. |

Short presses of **MODE** cycle through these four modes. Manual movement requires the separate **Hold-to-Enable** button.

## How it works

The **Raspberry Pi 4** processes the RGB-D camera and sends object positions to the **STM32 NUCLEO-F767ZI** over a 3.3 V GPIO UART link. The STM32 is planned to run a TC-ResNet voice-command model through two INMP441 microphones, calculate inverse kinematics (joint angles from a target position), and drive six servos through an SN74AHCT244N signal buffer (until it is fitted, the servos are wired direct through 330 Ω; see `ProjectDevGuide/docs/core-config.md` §11). A **2.4-inch ILI9341 LCD** shows status. There is no hardware emergency stop, limit switch or servo-rail sense (`core-config.md` v1.4): switch the servo supply off to stop for sure.

## Project files

- [Final build specification](study/robotic-arm-final-build-spec.md): equipment, wiring, build order and safety checks.
- [System guide](study/robotic-arm-guide.md): mode controls and STM32 responsibilities.
- [Code and bench tests](code/README.md): the servo calibration bench and the merged STM32 firmware skeleton.

**Current code status:** the servo bench tests and the merged STM32 HAL skeleton are in the repository. The skeleton includes the app loop, safety, motion/calibration, modes, voice, Pi link and LCD framework, but IK/FK and arm geometry are still placeholders and the installed servo limits/home are not calibrated. The measured unloaded servo values are in [the canonical calibration table](ProjectDevGuide/docs/servo-calibration.md). The Pi camera program and installed-arm validation are still open.
