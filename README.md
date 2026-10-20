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

The **Raspberry Pi 4** handles voice, the RGB-D camera and inverse kinematics (calculating joint angles from a target position). It sends movement requests to the **STM32 NUCLEO-F767ZI** over USB serial. The STM32 checks limits, creates smooth motion and drives six servos through an SN74AHCT244N signal buffer. An ILI9341 LCD shows the mode and system status. A hardware emergency stop disconnects servo power.

## Project files

- [Final build specification](study/robotic-arm-final-build-spec.md): equipment, wiring, build order and safety checks.
- [System guide](study/robotic-arm-guide.md): mode controls and STM32 responsibilities.
- [Code and bench tests](code/README.md): the Arduino servo test sketches.

**Current code status:** the repository contains single-servo Arduino tests. The complete Raspberry Pi and STM32 arm controller is still planned.
