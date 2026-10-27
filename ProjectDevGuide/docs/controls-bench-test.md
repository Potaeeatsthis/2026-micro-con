# Controls bench test: MODE, Hold-to-Enable, LCD (no servos)

> **Obsolete since 27 Oct 2026.** The MODE and Hold-to-Enable buttons and the e-stop were removed from the
> hardware ([`core-config.md`](core-config.md) v1.3); the mode is chosen on the LCD. Kept as the record of the
> 24 Oct 2026 test. Do not wire PE7, PE10 or PE12.

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
evt: ENABLE held (safety)           Hold-to-Enable pressed / released (read from safety_enable_held())
evt: ENABLE released (safety)
screen: SINGLE JOINT FINE | Joint: J1 BASE | Target: 90 deg | Pulse: 1460 us | Enable: RELEASED
```
`screen:` = what the LCD would show (mode, fine / fast, the mode's 4 detail lines), printed when it changes,
at most 4 times a second. It shows the modes' reactions (selected joint, speed, jog targets) while the LCD
driver is not connected.


Mode order (MODE button cycles): **VOICE → CARTESIAN → JOINT GROUP → SINGLE JOINT → VOICE**. The firmware
boots in VOICE. A switch only happens when motion is idle, Hold-to-Enable is released and the stick is centred.

## 5. Procedure and results

| # | Do | Expected log | Result |
|---|---|---|---|
| 1 | Boot, hands off | `safety: init, estop released, ...`, `modes: -> VOICE`, `input: joystick centre ~2073 / ~2067` | pass 2026-10-24 (centre 2073 / 2067) |
| 2 | Press MODE 4 times, a second apart | 4 × `evt: BTN_MODE`, `modes: -> CARTESIAN`, `-> JOINT GROUP`, `-> SINGLE JOINT`, `-> VOICE` | pass 2026-10-24: 4 presses, 4 switches, each within 1 ms of the event |
| 3 | Hold the stick right, press MODE, wait 3 s, release the stick | `evt: BTN_MODE` at once, `modes: -> ...` only **after** the stick is released | pass 2026-10-24: press 23:39:56, switch 23:40:07 after release; a second press while waiting was ignored (one switch) |
| 4 | Hold Hold-to-Enable, press MODE, wait 3 s, release Enable | `evt: BTN_MODE` at once, `modes: -> ...` only **after** Enable is released | pass 2026-10-25 (both buttons wired): waits while Enable held, also while Enable + stick held (switches only after both released), jog freezes on MODE |
| 5 | In SINGLE JOINT: press the joystick (centred) twice | 2 × `evt: JOY_PRESS`, `screen:` FINE → FAST → FINE | pass 2026-10-25 |
| 6 | In SINGLE JOINT: 3 encoder clicks each way, press the knob | `evt: ENC_STEP` ±1 × 6, `screen:` Joint changes, `evt: ENC_PRESS` | pass 2026-10-25: J1 → J6 → J1 both ways (12 + 6 clicks), each joint keeps its target; JOINT GROUP: Group 1 → 2 → 3 → 1 |
| 7 | SINGLE JOINT / JOINT GROUP: push the stick without Enable, then with Enable held, then release Enable | `Target` moves only while Enable is held, stops at release | pass 2026-10-25: FINE ≈ 10 deg/s, FAST ≈ 45 deg/s, J1 stops at 0 deg (616 us with the calibration of that day; since the master merge J1 0 deg = 500 us), no movement without Enable |

Write the results here (or in the PR) with the date, the commit and the local flags.
2026-10-24 run: commit 6f49100 + `APP_INPUT_LOG_EVENTS`, local flags `APP_USE_VOICE 0`, `APP_INPUT_LOG_EVENTS 1`;
MODE = 12 × 12 mm 4-pin tactile button on diagonal pins (PE10 ↔ GND), no Hold-to-Enable yet, no servos, no LCD.
MODE presses while a switch is waiting do not queue a second switch.
Extra MODE checks (user, live): hold 3 s = one event, 5 fast taps = 5, very short tap, wire wiggle = nothing,
MODE held through RESET = harmless: all pass. Hold-to-Enable alone (the same button moved to PE12): no line at
boot, one `held` / `released` pair per press, 10 s hold = one line, 5 taps = 5 pairs, wiggle = nothing: all pass.
2026-10-25 (one button moved between PE10 and PE12 without a reset): steps 5–7 pass. CARTESIAN shows
`IK NOT READY` (arm geometry `TODO(ik)`). Step 4 passed later the same night with both buttons wired. Real motion needs servos.

## 6. Next: servos

Not part of this test. Notes for whoever connects them:
- First run of new motion code: **servo signal wires disconnected**, then one servo, then all six.
- Wiring, power order and the buffer plan: [`core-config.md`](core-config.md) §11 (each PWM pin → 330 Ω → servo
  signal, common GND, Nucleo on before the servo rail; HCT244 buffer later, not the HC244).
- Calibration values: [`servo-calibration.md`](servo-calibration.md) (motion owner). Installed limits and home are
  not measured yet, so the 0 / 180 / 90 deg values are placeholders.
- FINE 10 deg/s is ≈ 2 µs per 20 ms frame, below the SG90/SG92R deadband (`core-config.md` §4.1): an SG joint
  may move in small jumps in FINE. Watch for it in the first servo test.
