# Robotic Arm Code

This repository currently contains **bench firmware for one servo at a time**. The planned four-mode controller, including STM32 voice recognition and inverse kinematics, is described in [the system guide](../study/robotic-arm-guide.md); its firmware is not in this repository yet.

There are two versions of the calibration bench. The Arduino Uno sketch uses the `Servo` library on pin 9. The STM32 project reuses the same console source and outputs PWM from the NUCLEO-F767ZI hardware timers on physical pins PC6, PC7, and PD12-PD15. Use the version that matches the board you have wired.

| Sketch / project | What it tests |
|---|---|
| [`test-sg92r.ino`](test-sg92r/test-sg92r.ino) | Basic one-link inverse kinematics (IK): convert three X/Y targets into servo angles. |
| [`servo-calibration.ino`](servo-calibration/servo-calibration.ino) | **Arduino Uno** six-unit positional servo calibration console for 3 MG996R + 3 SG92R: record pulses and measured angles, mark unloaded/installed safe limits, and print CSV tables. One physical servo at a time on pin 9; no startup motion. |
| [`stm32-servos/`](stm32-servos/README.md) | **STM32 NUCLEO-F767ZI** PlatformIO build of the same calibration console, using hardware `HardwareTimer` PWM at 50 Hz on PC6/PC7/PD12-PD15. One output armed at a time; no startup PWM or auto-sweep. |
| [`test-sg92r-ik.ino`](test-sg92r-ik/test-sg92r-ik.ino) | Repeat the one-link IK test using measured angle-to-pulse values. |
| [`test-mg996r.ino`](test-mg996r/test-mg996r.ino) | Test **continuous-rotation** servo direction and speed. This sketch does not position a standard MG996R by angle. |

## Run a test

**Arduino Uno sketches:**

1. Open one `.ino` file in Arduino IDE and select your Arduino board and port.
2. Connect that servo's signal wire to **pin 9**. Use a suitable external servo supply and connect its ground to the Arduino ground. Keep the arm or servo horn clear.
3. Upload the sketch. Open Serial Monitor at **115200 baud** to see the output. For the calibration sketch, type `help` and press Enter; it attaches nothing until you explicitly select and arm a servo.

**STM32 calibration console:**

1. Open the [`stm32-servos`](stm32-servos/README.md) project with VS Code PlatformIO, or use the uv PlatformIO CLI commands in that README.
2. Connect the ST-LINK USB cable. Use the **physical NUCLEO-144 pin names** PC6, PC7, PD12, PD13, PD14, and PD15, not Arduino pin 9. The exact unit-to-pin map and the SN74AHCT244N buffer wiring are in the project README.
3. Build, upload over ST-LINK, and open the ST-LINK virtual COM port at **115200 baud**. The shared `help`, `select`, `arm`, `pulse`, `record`, `limits`, `installed`, `zero`, `window`, and `table` commands are unchanged.

Test only one sketch and one servo at a time. The pulse values in the IK sketches are test values; calibrate each real servo before attaching it to the arm.
