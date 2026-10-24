# Controls bench test: MODE, Hold-to-Enable, LCD (no servos)

Part C of the input check. Parts A and B are done (2026-10-24, [`input-and-pilink-test.md`](input-and-pilink-test.md)
§7): the joystick, the encoder and their buttons give correct values and exactly one event per press / click.
This part checks that **the modes react** to them, with the MODE and Hold-to-Enable buttons and the LCD
wired. **No servo signal wire is connected** during this test.

Who sends what: [`inputs-and-pi-interface.md`](inputs-and-pi-interface.md). Pins: [`core-config.md`](core-config.md) §3.

## 1. Before you start

**The LCD shows nothing yet.** `App/lcd/lcd_port.c` is still empty stubs (`TODO(lcd)`: the Lab08 ILI9341
driver is not hooked up). The screen logic runs, but nothing reaches the display until the LCD owner
connects the driver. Wire it now if you like; the checks below use the debug log, not the screen.

**No e-stop button.** The team plans to drop it. The firmware needs no change for that: PE7 has a pull-up,
so an unconnected pin reads "released". Without the button, though, nothing cuts servo power in hardware;
the only stops are releasing Hold-to-Enable (software) and pulling the power. Agree on this with the safety
owner before servos move.

## 2. Wiring

Buttons wire **pin → button → GND** (internal pull-up, pressed = 0).

Three different buttons, three different jobs (`core-config.md` §3, decided 20 Oct 2026):

| Button | Pin | Job |
|---|---|---|
| **MODE** (its own push button) | PE10 | next mode |
| **Hold-to-Enable** (its own push button) | PE12 | manual modes move only while it is held |
| Joystick click (KY-023 SW) | PE15 | fine / fast speed, only while the stick is centred |
| Encoder knob click (KY-040 SW) | PE14 | mode-specific action |

The joystick click is **not** the MODE button. Making it one would be a team decision and a core change
(`modes.c`, `core-config.md`), not a wiring choice.

| Part | STM32 pin | Nucleo header |
|---|---|---|
| MODE button | PE10 | CN10-24 (D40) |
| Hold-to-Enable button | PE12 | CN10-26 (D39) |
| KY-023 joystick, KY-040 encoder | as in `input-and-pilink-test.md` §2 (3.3 V, pins pointing left) | |
| LCD (optional now, see above) | SPI5 PF7 / PF8 / PF9, CS PC8, DC PC9, RST PC10; touch PC11 / PC12 / PD2 / PG2 / PG3 | CN8 / CN9 |
| Servo signals | **not connected** | |

## 3. Firmware

`RoboticArmMain/App/common/app_config_local.h` (gitignored, never commit it):

```c
#pragma once
#define APP_USE_VOICE         0   /* optional: quieter log, no mics needed */
#define APP_INPUT_LOG_EVENTS  1   /* one "evt:" line per input event, works with the modes on */
/* APP_INPUT_TEST must NOT be set: it refuses to build with the modes on */
```

Modes, motion, LCD and the Pi link stay on (defaults). Motion starts PWM on all six servo pins after boot
(home pose): that is why no servo may be connected. Build and flash as usual; read the log on the ST-LINK
COM port at 921600 8N1.

## 4. What the log shows

```text
modes: -> CARTESIAN                 a mode switch finished
evt: BTN_MODE                       MODE pressed (input)
evt: JOY_PRESS                      joystick pressed while centred = fine / fast toggle in manual modes
evt: ENC_STEP +1 (sum +3)           encoder click
evt: ENC_PRESS                      encoder knob
```

Not logged: Hold-to-Enable press / release and the fine / fast state (both only on the LCD, once it works).
Hold-to-Enable is checked indirectly below: a mode switch waits while it is held.

Mode order (MODE button cycles): **VOICE → CARTESIAN → JOINT GROUP → SINGLE JOINT → VOICE**. The firmware
boots in VOICE. A switch only happens when motion is idle, Hold-to-Enable is released and the stick is centred.

## 5. Procedure and results

| # | Do | Expected log | Result |
|---|---|---|---|
| 1 | Boot, hands off | `safety: init, estop released, ...`, `modes: -> VOICE`, `input: joystick centre ~2073 / ~2067` | |
| 2 | Press MODE 4 times, a second apart | 4 × `evt: BTN_MODE`, `modes: -> CARTESIAN`, `-> JOINT GROUP`, `-> SINGLE JOINT`, `-> VOICE` | |
| 3 | Hold the stick right, press MODE, wait 3 s, release the stick | `evt: BTN_MODE` at once, `modes: -> ...` only **after** the stick is released | |
| 4 | Hold Hold-to-Enable, press MODE, wait 3 s, release Enable | `evt: BTN_MODE` at once, `modes: -> ...` only **after** Enable is released | |
| 5 | In CARTESIAN: press the joystick (centred) twice | 2 × `evt: JOY_PRESS` (speed toggles fine → fast → fine) | |
| 6 | In SINGLE JOINT: 3 encoder clicks each way, press the knob | `evt: ENC_STEP` ±1 × 6, `evt: ENC_PRESS` | |
| 7 | Hold Enable in CARTESIAN and push the stick | no error / fault lines (motion moves its targets; nothing visible without servos) | |

Write the results here (or in the PR) with the date, the commit and the local flags.

## 6. Next: servos

Not part of this test. Notes for whoever connects them:
- First run of new motion code: **servo signal wires disconnected**, then one servo, then all six.
- Until the SN74AHCT244N buffer arrives the team plans **STM32 pin → series resistor → servo signal**. That
  drives the servo with 3.3 V logic instead of the 5 V the buffer gives (`core-config.md` §11). Check that each
  servo model accepts 3.3 V signals, and that the servo pins are 5 V tolerant in the STM32F767 datasheet
  (a servo may hold its signal line at its own supply voltage). Servo GND and Nucleo GND must be connected;
  servo current never goes through the Nucleo.
- Servo calibration (`ProjectDevGuide/docs/servo-calibration.md` on `master`) belongs to the motion owner.
