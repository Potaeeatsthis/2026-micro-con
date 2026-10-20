# Robotic Arm Code

This repository currently contains **Arduino bench tests for one servo at a time**. The planned four-mode controller, including STM32 voice recognition and inverse kinematics, is described in [the system guide](../study/robotic-arm-guide.md); its firmware is not in this repository yet.

| Sketch | What it tests |
|---|---|
| [`test-sg92r.ino`](test-sg92r/test-sg92r.ino) | Basic one-link inverse kinematics (IK): convert three X/Y targets into servo angles. |
| [`test-sg92r-cali-deg-to-pwm.ino`](test-sg92r-cali-deg-to-pwm/test-sg92r-cali-deg-to-pwm.ino) | Send a value through Serial Monitor and inspect the resulting servo pulse for calibration. |
| [`test-sg92r-ik.ino`](test-sg92r-ik/test-sg92r-ik.ino) | Repeat the one-link IK test using measured angle-to-pulse values. |
| [`test-mg996r.ino`](test-mg996r/test-mg996r.ino) | Test **continuous-rotation** servo direction and speed. This sketch does not position a standard MG996R by angle. |

## Run a test

1. Open one `.ino` file in Arduino IDE and select your Arduino board and port.
2. Connect that servo's signal wire to **pin 9**. Use a suitable external servo supply and connect its ground to the Arduino ground. Keep the arm or servo horn clear.
3. Upload the sketch. Open Serial Monitor at **115200 baud** to see the output. For the calibration sketch, type a value and press Enter.

Test only one sketch and one servo at a time. The pulse values in the IK sketches are test values; calibrate each real servo before attaching it to the arm.
