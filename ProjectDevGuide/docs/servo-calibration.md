# Servo calibration — measured values and install checklist

This is the canonical record of the servo measurements made on the bench. It is
documentation only: **the console and the firmware do not read this file**, and
a reset still leaves the console RAM records empty. Keep your saved CSV (`table`
output) outside the board. There is no point-data import: do not replay unloaded
measurement pulses on the assembled arm just to restore the records.

The bench tools are separate from the HAL firmware:

- `code/servo-calibration/` (Arduino Uno, one servo on pin 9) and
  `code/stm32-servos/` (STM32 NUCLEO-F767ZI, one output armed at a time) are the
  calibration bench.
- `RoboticArmMain/` is the HAL firmware. Its `App/motion/servo_cal.c` table is
  not automatically filled from this document.

## Safety warning

**The installed calibration is not done.** In `App/motion/servo_cal.c` every
joint still uses `min_deg = 0`, `max_deg = 180` and `home_deg = 90`. Those are
**placeholders, not calibrated safe limits and not an installed home pose**.
The values below are unloaded bench measurements only.

Do not move the assembled arm from the current firmware table. Support the arm,
keep a hardware power disconnect within reach, and keep the servo signal wires
disconnected while bringing up new motion code. `off`/`select` detach the
signal only; they do not cut servo power and do not hold the arm.

## Confirmed unit-to-joint mapping and bench pins

The mapping below is confirmed. A bench unit label identifies one physical
servo; a joint is a position in the arm. They are recorded separately even
though the current mapping is one unit per joint.

| Joint | Servo (user label) | Bench unit | Bench console label | STM32 pin | Timer channel |
|---|---|---|---|---|---|
| J1 base | MG996R#1 | 1 | `MG996R-1` | PC6 (CN7-1, D16) | TIM3_CH1 |
| J2 shoulder | MG996R#2 | 2 | `MG996R-2` | PC7 (CN7-11, D21) | TIM3_CH2 |
| J3 elbow | MG996R#3 | 3 | `MG996R-3` | PD12 (CN10-21, D29) | TIM4_CH1 |
| J4 wrist roll | SG92R#1 | 4 | `SG92R-1` | PD13 (CN10-19, D28) | TIM4_CH2 |
| J5 wrist pitch | SG92R#3 | 5 | `SG92R-3` | PD14 (CN7-16, D10) | TIM4_CH3 |
| J6 gripper | SG90 | 6 | `SG90` | PD15 (CN7-18, D9) | TIM4_CH4 |

The Arduino Uno bench still uses one servo on **pin 9** at a time. On the STM32
bench, use the physical NUCLEO pin names above, not Arduino pin numbers. The
authoritative pin/timer map remains [`core-config.md`](core-config.md) §3–§4.1.

## Canonical unloaded measurements

Triples are `pulse @ measured 0°`, `pulse @ measured 90°`, `pulse @ measured
180°`, in microseconds. Each active unit was tested **individually and
unloaded** over the 500..2500 µs range. These are unloaded bench results, not
installed safe limits.

| Bench unit | Label | Joint | 0° (µs) | 90° (µs) | 180° (µs) | Reference (µs at measured 90°) | Unloaded tested range (µs) |
|---|---|---|---|---|---|---|---|
| 1 | `MG996R-1` | J1 | 500 | 1460 | 2430 | 1460 | 500..2500 |
| 2 | `MG996R-2` | J2 | 500 | 1430 | 2440 | 1430 | 500..2500 |
| 3 | `MG996R-3` | J3 | 540 | 1530 | 2500 | 1530 | 500..2500 |
| 4 | `SG92R-1` | J4 | 500 | 1440 | 2370 | 1440 | 500..2500 |
| 5 | `SG92R-3` | J5 | 640 | 1580 | 2470 | 1580 | 500..2500 |
| 6 | `SG90` | J6 | 580 | 1570 | 2500 | 1570 | 500..2500 |

**Reference pulse = the pulse measured at 90°**, not the arithmetic midpoint of
the 0°/180° pulses. For example, J1's reference is 1460 µs; the midpoint of 500
and 2430 would be 1465 µs.

**Values on the tool limit may be clipped.** The bench tool could not send less than 500 µs or more than
2500 µs. These points landed exactly on that limit, so the true pulse for that angle may lie slightly beyond it:
0° of `MG996R-1`, `MG996R-2` and `SG92R-1` (500 µs), and 180° of `MG996R-3` and `SG90` (2500 µs). The horn
was seated one spline tooth (about 14° on the MG996R, 17° on the SG92R) closest to 90° at 1500 µs, so a
different tooth moves all three points together.

**Repeatability (three runs, 23 Oct 2026).** The span from 0° to 180° stayed within 30 µs between runs for every
unit measured three times (`MG996R-1` 1950 / 1950 / 1930 µs, `SG92R-1` 1900 / 1900 / 1870 µs). The absolute
pulses moved by up to about 150 µs between runs because the horn sat on a different tooth. Runs 1 and 2 used
50 µs steps (about ±2.5°); the table above is run 3, fine-tuned with 10 µs steps.

The 500..2500 µs column is the unloaded range each unit was tested over. It is
the value to enter as the console's unloaded `limits` if it matches what you
physically checked; it is not an installed range.

### Retired unit (kept for history)

`SG92R-2` was replaced because it was unreliable. Keep the old row for the
record; do not use it for the arm or for a new calibration.

| Bench unit | Label | Joint | 0° (µs) | 90° (µs) | 180° (µs) | Reference (µs at measured 90°) | Unloaded tested range (µs) | Note |
|---|---|---|---|---|---|---|---|---|
| — | `SG92R-2` (retired) | — | 680 | 1550 | 2390 | 1550 | 500..2500 | Historical run 2; replaced as unreliable |

## STM32 bench test (24 Oct 2026)

All six servos moved from `RoboticArmMain` with the signals wired **direct** (STM32 pin → 330 Ω →
servo signal, common GND); the SN74HC244N + diode stage did not work (see
[`core-config.md`](core-config.md) §11). The test code (all joints to 90°, then the voice word
"pick" swings J1..J6 one at a time) is on branch `test/servo-power-test`. It is a bench tool only and
is never merged: it writes PWM outside `motion/`. Turn it on there in `app_config_local.h` with
`APP_USE_SERVO_TEST 1`, `APP_USE_MOTION 0`, `APP_USE_MODES 0`.

## Signal check (when a servo does not move)

Measure before changing code. A multimeter on DC shows the **average** of the PWM signal,
≈ high level × pulse / 20000 µs. Example for a joint held at 90° (J1, 1460 µs):

| Point (to common GND) | Expected | Means if wrong |
|---|---|---|
| STM32 pin (e.g. PC6) | 3.3 V × 1460 / 20000 ≈ **0.24 V** (measured 0.23 V on 24 Oct) | 0 V: channel not started or CCR = 0; 3.3 V: pin stuck high |
| Buffer VCC (pin 20) / GND (pin 10) | 5 V / 0 V | buffer not powered |
| Buffer /OE (pins 1 and 19) | **0 V** (tied to GND) | floating /OE: outputs undefined |
| Buffer output for that channel | 5 V × 1460 / 20000 ≈ **0.37 V** | input OK but output wrong: wiring or chip |
| Servo signal wire at the plug | same as the line feeding it | broken wire or wrong row on the breadboard |
| Servo red / brown | rail voltage (6 V) / 0 V | servo not powered |

The averages are calculated, not measured, except the PC6 reading.

## Bench power lessons (23 Oct 2026)

These happened during the unloaded measurements. They are why the servo rail must never come from a board's
5 V pin or from laptop USB.

| What was done | What happened |
|---|---|
| MG996R powered from the NUCLEO **VIN** pin | Clicks, no motion: VIN is an **input** (7–12 V into the board), not a supply |
| MG996R and SG92R from the NUCLEO **5V** pin (ST-LINK USB) | Clicks; the rail sagged to ≈ 4.6 V on a meter and the board **reset** mid-test (startup log printed again) |
| MG996R from the Arduino Nano 5V (USB) | Worked for slow moves, but a full-range jump **froze the Nano** (no reply until a DTR reset), the USB port locked (`Device Descriptor Request Failed`, error 43) and once the **laptop shut down** (Windows Kernel-Power 41) |
| 12 V battery into a buck converter without a fuse | The buck's own input capacitor **burst** at connection; most likely the input polarity was reversed. Treat that buck as damaged |

Rules that follow:

- Power the servos only from the separate 6 V buck rail (or 4 × AA ≈ 6 V for a single bench servo), with the
  ground shared with the board. Never from VIN, 5V, 3V3 or a USB port.
- Before the buck: **fuse** on the battery +, and a reverse-polarity diode or a polarity check with a meter.
  Set the buck output (6.0 V, CC limit ≈ 3 A for one MG996R) with nothing connected, then connect the servo.
- Do not jump a servo across its full range in one step on the bench; ramp it. The full-range jump is the
  biggest current spike.
- A brown-out can **hang** an AVR board without resetting it: if a servo suddenly stops responding, reset the
  board before suspecting the servo.

## Installed calibration status

Everything below is still **to be measured after assembly**:

- installed minimum and maximum pulse per joint;
- installed joint zero pulse;
- installed home pose/pulse;
- joint direction sign;
- mechanical clearance and self-collision limits.

| Joint | Unit label | Installed min (µs) | Installed max (µs) | Installed zero (µs) | Installed home (µs) | Direction sign |
|---|---|---|---|---|---|---|
| J1 | `MG996R-1` | | | | | |
| J2 | `MG996R-2` | | | | | |
| J3 | `MG996R-3` | | | | | |
| J4 | `SG92R-1` | | | | | |
| J5 | `SG92R-3` | | | | | |
| J6 | `SG90` | | | | | |

These cells are deliberately blank. Do not fill them by inference.

Do not infer installed values from the unloaded table, from the unit label, or
from the old IK sketches. The installed zero and home are physical poses, and
the installed range can be narrower than the unloaded tested range.

## After-assembly checklist

- [ ] Assemble and support the arm before applying servo power.
- [ ] Confirm on the real arm that each joint is driven by the unit in the
      confirmed mapping table above; label the cables.
- [ ] Keep the other joints supported while testing one installed joint.
- [ ] With the output detached, use `select N`, then `limits 500 2500` to
      restore this unit's confirmed unloaded bounds. This does not move it.
      Keep the unloaded measurement points in the saved table; do not command
      those endpoints on the installed arm just to restore RAM records.
- [ ] Set `window <min> <max>` to a conservative interval based on the actual
      assembly clearance. Choose a starting pulse that matches the supported
      pose, then `arm <pulse>`; if that pose is not known, disconnect the linkage
      and establish alignment first. No universal starting pulse is safe.
- [ ] Use `pulse` steps of at most **10 µs** while armed. (`arm` can still jump
      to its first pulse; support the arm and keep the range conservative.)
- [ ] Find the narrower installed safe range with power on only for that joint,
      watching clearance, binding, buzz and current. Do not run a full sweep.
- [ ] Use `off`, then save the installed range with `installed <min> <max>`.
- [ ] Re-arm at a known conservative pulse inside both the installed bounds and
      test window. Move slowly to the physical joint zero, then save it with
      `zero` while armed. Only expand the test window within verified clearances.
- [ ] Measure and record the installed home pose separately from joint zero.
- [ ] Confirm each joint's positive direction with a small, slow move and record
      its sign.
- [ ] Save `table` output before reset and keep the CSV outside the board.
- [ ] Only after these values are measured, have the motion owner replace the
      `0 / 180 / 90` placeholders in `App/motion/servo_cal.c`.
- [ ] Verify the updated firmware with servo signal wires disconnected before
      moving the assembled arm under load.

## Bench console notes

- `select <1..6>` detaches the signal and selects the bench unit. Switching
  units resets the test window to 1400..1600 µs; it does not move anything.
- `arm <pulse>` attaches the output and can jump immediately to the requested
  pulse.
- `pulse <pulse>` moves while armed, at most 10 µs per command; smaller steps
  are allowed.
- `reference` stores the current pulse as the unit reference. Use the pulse at
  measured 90°.
- `limits` is the unloaded safe range you establish by testing. `installed` is
  the narrower range after fitting. `zero` is the installed joint zero.
- `table` prints the summary and points as CSV. Records live in RAM and are lost
  on reset or power loss; there is no CSV import.

See [`code/servo-calibration/README.md`](../../code/servo-calibration/README.md)
for the full command behaviour and
[`code/stm32-servos/README.md`](../../code/stm32-servos/README.md) for the STM32
bench wiring.

## Related documentation

- [`core-config.md`](core-config.md) — pins, timers, PWM settings.
- [`TEAM_GUIDE.md`](TEAM_GUIDE.md) — firmware ownership and safety rules.
- [`study/robotic-arm-kinematics-todo.md`](../../study/robotic-arm-kinematics-todo.md)
  — assembly and geometry/IK workflow.
