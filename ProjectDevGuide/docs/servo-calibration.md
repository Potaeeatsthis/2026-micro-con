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

**Gripper and HOME integration: fixed in code (27 Oct 2026), hardware checks still open.**

- `App/common/app_config.h`: `APP_GRIPPER_CLOSED_DEG` = 0, `APP_GRIPPER_OPEN_DEG` = 60 (the J6 safe
  endpoints; the 35 deg startup home is unchanged). Travel endpoints, not a grip-force calibration.
- `App/modes/gripper.c` (new, used by Cartesian and voice): a gripper command counts only if motion
  accepts it unchanged. A `MOTION_CLAMPED` result is cancelled (`motion_hold_joint(J6)`) and reported as
  failed, so a clamped close is never taken as holding an object (voice: the script stops with
  `GRIPPER LIMIT`; Cartesian: the state stays, `GRIPPER LIMIT` on the LCD). Cartesian reads its open/closed
  state as "nearer the closed setpoint", so a lower angle is closed.
- Voice HOME (`mode_voice.c`): from J2 < 90 deg it first raises J2 to 90 deg (J5 kept at least 65 deg),
  then commands the home pose. A home target that comes back `MOTION_CLAMPED` is refused (`HOME BLOCKED`),
  and S9 reports done only when every joint is at its home angle (`HOME NOT REACHED` otherwise). Ordinary
  wrist commands still never lift the shoulder.
- Host tests: gripper setpoints/direction, clamped command cancelled, HOME from J2 = 50 / J5 = 70 never
  passes J2 < 90 with J5 < 65 and ends exactly at home. With the staging removed the HOME test fails.

Still open before powered automatic operation: build with the ARM toolchain, check the PWM with the servo
signal wires disconnected, then supported, controlled hardware tests. Not covered yet (IK not ready, so not
reachable today): Cartesian moves and voice pose steps still accept `MOTION_CLAMPED` IK targets.

The firmware table in `App/motion/servo_cal.c` now holds the confirmed installed
home poses and conservative per-joint safe limits (27 Oct 2026). The home pose is
a servo-angle pose, **not the kinematic joint zero**; startup sends it after
safety allows, one joint every 500 ms (gripper first, then J1-J5) so the servos do not all draw
start-up current at once. Joint zeros and direction signs are still uncalibrated.

The independent per-joint limits give endpoint clearance only. They do **not**
guarantee that every whole-arm pose or path is collision-free; there is no
automatic collision checking. Keep the arm supported, keep a hardware power
disconnect within reach, and keep the servo signal wires disconnected while
bringing up new motion code. `off`/`select` detach the signal only; they do not
cut servo power and do not hold the arm.

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
| 6 | `SG90` (replacement) | J6 | 500 | 1380 | 2300 | 1380 | not recorded |

J6 was replaced after the original bench run, so the firmware uses the
replacement anchors above rather than the earlier SG90 row (580 / 1570 /
2500 µs). The reported 2490 µs maximum is **not** an angle anchor and **not**
an installed limit; it is not used.

**Reference pulse = the pulse measured at 90°**, not the arithmetic midpoint of
the 0°/180° pulses. For example, J1's reference is 1460 µs; the midpoint of 500
and 2430 would be 1465 µs.

**Values on the tool limit may be clipped.** The bench tool could not send less than 500 µs or more than
2500 µs. These points landed exactly on that limit, so the true pulse for that angle may lie slightly beyond it:
0° of `MG996R-1`, `MG996R-2` and `SG92R-1` (500 µs), and 180° of `MG996R-3` (2500 µs). The original SG90
measurement also touched 2500 µs, but that row is superseded by the replacement J6 anchors above. The horn
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

## Limit test screen (`App/servo_cal_test/`, 27 Oct 2026)

A bench screen for checking the installed home pose and joint limits. **Off by default**
(`APP_USE_SERVO_CAL_TEST 0`); never enable it on `master`. Enable it only in the gitignored
`App/common/app_config_local.h`:

```c
#define APP_USE_SERVO_CAL_TEST 1
#define APP_USE_MOTION         1   /* the screen moves joints only through motion/ */
#define APP_USE_MODES          0
#define APP_USE_LCD            0   /* the screen draws itself through lcd_port */
#define APP_USE_INPUT          1
/* optional on the bench: APP_USE_VOICE 0, APP_USE_PI 0 */
```

**It is not a passive measurement mode.** At boot motion's normal start-up sends every joint to its
`home_deg`, one joint every `MOTION_BOOT_STAGGER_MS` (500 ms), gripper first. Support the arm near
home and keep the servo power switch in reach.

| Control | Action |
|---|---|
| KY-040 press | next joint (J1..J6, wraps) |
| KY-040 turn | selected joint's target ± step per click (clockwise = more); starts at **1 deg** |
| KY-023 press (stick centred) | step 1 ↔ 5 deg |
| KY-023 pushed down, then back to centre | print the current pose on USART3 as `servo_cal.c` rows |

Every move is a `motion_set_joint_target()` call, so motion's speed limit (10 deg/s), the table
min/max and the J2/J5 coupling apply exactly as in the real firmware; the screen never writes PWM
itself. A clamped click shows the reason in red: `J6 GRIPPER at MAX 60`, or for the coupling
`J2 MIN 90: raise J5 to 65` / `J5 MIN 65: J2 below 90`. A `*` after an angle means the joint is still
moving toward its target.

Printed rows use the current pose as `home_deg` and **always the table `min_deg` / `max_deg`** from
`servo_cal.c`, never a temporary coupled bound: copying them changes only the home column. Host test:
`ProjectDevGuide/tests/host/test_servo_cal.c` (run by `run.sh`).

## STM32 bench test (24 Oct 2026)

All six servos moved from `RoboticArmMain` with the signals wired **direct** (STM32 pin → 330 Ω →
servo signal, common GND); the SN74HC244N + diode stage did not work (see
[`core-config.md`](core-config.md) §11). The test code (all joints to 90°, then the voice word
"pick" swings J1..J6 one at a time) is on branch `test/servo-power-test`. It is a bench tool only and
is never merged: it writes PWM outside `motion/`. Turn it on there in `app_config_local.h` with
`APP_USE_SERVO_TEST 1`, `APP_USE_MOTION 0`, `APP_USE_MODES 0`. It is **not** the limit test screen above.

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

## Installed calibration (confirmed 27 Oct 2026)

The firmware table now contains the confirmed installed home and conservative
joint limits. Home is a physical servo-angle pose loaded before the first pulse;
it is **not** the kinematic joint zero. The pulse values below are derived from
the anchors above at those home angles.

| Joint | Unit label | Safe min (°) | Safe max (°) | Home (°) | Home pulse (µs) | Installed zero | Direction sign |
|---|---|---|---|---|---|---|---|
| J1 | `MG996R-1` | 0 | 180 | 85 | 1407 | not measured | not calibrated |
| J2 | `MG996R-2` | 50 | 180 | 125 | 1823 | not measured | not calibrated |
| J3 | `MG996R-3` | 45 | 180 | 145 | 2123 | not measured | not calibrated |
| J4 | `SG92R-1` | 0 | 180 | 90 | 1440 | not measured | not calibrated |
| J5 | `SG92R-3` | 25 | 140 | 25 | 901 | not measured | not calibrated |
| J6 | `SG90` (replacement) | 0 | 60 | 35 | 842 | not measured | not calibrated |

The limits were confirmed independently, one joint at a time, with the rest of
the arm supported. J2's table floor is the absolute **50°**, but that value is
not a confirmed clearance on its own: floor contact was observed around 90–95°
with J5 = 25°. A J2 angle near 50° was observed only with the wrist raised, so
the motion layer raises J2 to **90°** whenever J5 is below 65° (current or
pending). This coupling is still endpoint clearance, not a whole-arm collision
check.

### J2/J5 wrist coupling (motion layer)

`App/motion/motion.c` applies this rule from the **actual commanded pose**
(`motion_get_current()`) plus, for a single-joint request, the pending target of
the other joint:

- J5 at or above **65°** (current or pending) allows J2 down to its absolute
  **50°** table floor.
- Otherwise J2 must stay at **90°** or above.
- While J2 is below **90°** (current or pending), J5 must stay at **65°** or
  above.
- No automatic shoulder lift: a low J5 command is blocked at 65°; J2 is not
  raised merely to make room for it.

The setter applies the current-pose gate **and** validates the combined pending
endpoint under the IRQ lock, so an `MOTION_OK` return means the stored endpoint
is already safe. A full target whose two requested values are individually legal
but whose endpoint is unsafe (for example current 125°/65°, requested 50°/25°) is
deterministically clamped to J2 = 90°, J5 = 25° and returns `MOTION_CLAMPED`; the
tick does not silently change the endpoint later. A single-joint request clamps
only the requested joint and leaves the other pending target untouched. The jog
and TIM6 tick paths still re-clamp from the current pose as the trajectory moves,
and J2 is updated before J5 in the tick so a simultaneous move cannot end with J2
below 90° while J5 is below 65°.

A target that hits a coupled bound is **clamped to that bound and stored
clamped**. It is not kept and replayed later when the other joint moves: after
J5 is raised, the caller must reissue J2 = 50°. This prevents stale latent motion.
Status reports `MOTION_CLAMPED` whenever a requested value was limited, and
`MOTION_OK` otherwise. `MOTION_CLAMPED` is not a success indication for a
multi-step script; voice HOME is staged and checked for that reason (see the safety warning above).

The firmware has **no joint-angle sensors**. "Current" in this rule means the
commanded angle in `s_cur`, not the measured physical pose. A loaded or fast servo
can lag that command; the guard assumes the commanded pose is the safe one to use.
Keep the arm supported and the servo signal wires disconnected when first testing
the assembled arm.

The 0/90/180 pulse anchors are the full-range conversion anchors, not installed
limits. J6's safe range is 0..60° but its anchors still span 0..180°; do not
confuse the two.

### Directional observations (not signs)

| Joint | Observed motion direction |
|---|---|
| J1 | top view: increasing toward 180° = left; toward 0° = right |
| J2 | increasing = up |
| J3 | increasing = down |
| J4 | gripper looking at the shoulder: increasing = left; decreasing = right |
| J5 | increasing = up |
| J6 | increasing = opens |

These are observed directions only. They are **not** calibrated joint signs, and
the joint zeros remain uncalibrated. Do not turn them into IK or servo signs
without a separate calibration. A separate small-angle alignment check gives the
candidate signs below; they are still tentative.

### Candidate kinematic reference and servo-to-joint mapping

The installed-arm alignment check used the control-screen pose `POSE43`:
J1 = 85, J2 = 140, J3 = 85, J4 = 75, J5 = 100, J6 = 0 (degrees). Those raw
values are the **original** historical measurement and stay factual; they are
superseded as the candidate kinematic zero by the revised J2 = 145 and J4 = 87
below.

The active candidate reference is J1..J5 = 85, 145, 85, 87, 100° (J6 = 0): the
user confirmed that J2 = 145° and J4 = 87° are vertical. This is a **candidate
kinematic zero**, not a confirmed joint zero. It was reached with the controls
screen; the start-up home was not changed.

J4 = 87° is an operator-selected roll reference, not a CAD match: J4 roll being
vertical alone does not establish that the joint zero aligns with the CAD
reference, so the J4 value remains pending alignment validation.

Robot frame for this candidate mapping: origin at the J1 axis on the table
underplate, +X forward along the black arrow on the robot, +Y left, +Z up
(right-handed). J6 is the gripper and is excluded from the mapping.

The start-up home pose is unchanged in `servo_cal.c`
(J1..J6 = 85, 125, 145, 90, 25, 35°). Under the revised candidate mapping, the
five positioning joints map to model angles 0, 20, 60, 3, 75°:

| Joint | Home servo (°) | Candidate zero (°) | Candidate sign | Home model q (°) |
|---|---|---|---|---|
| J1 | 85 | 85 | +1 | 0 |
| J2 | 125 | 145 | -1 | 20 |
| J3 | 145 | 85 | +1 | 60 |
| J4 | 90 | 87 | +1 | 3 |
| J5 | 25 | 100 | -1 | 75 |

The candidate mapping is:

```text
q = sign * (servo - zero)
servo = zero + sign * q
```

The signs come from a small 1° servo-angle check at the original POSE43 pose.
Those observations keep their original joint positions: the J2 check was raw
servo 140° → 141° (backward) and is not re-dated to 145°; the J4 check and the
J5 check were both made with J4 at 75°, not the revised 87°:

- J1: increasing the servo angle moved the base counter-clockwise seen from above -> sign +1.
- J2: increasing the servo angle moved the shoulder backward -> sign -1.
- J3: increasing the servo angle moved the elbow forward -> sign +1.
- J4: increasing the servo angle moved the wrist counter-clockwise seen looking from the
  gripper toward the shoulder -> sign +1.
- J5: increasing the servo angle moved the wrist backward, with J4 at 75° -> sign -1.
  This sign still needs rechecking with J4 at the new 87° roll before it is
  treated as validated at the revised alignment.

The candidate values are zeros 85, 145, 85, 87, 100° and signs
+1, -1, +1, +1, -1. They remain **alignment-tentative**: J4 = 87° is an
operator-selected roll reference that has not been matched to the CAD
reference, and the J5 sign still needs rechecking with J4 at 87° (the original
check used J4 = 75°). None of this is production FK/IK or hardware validation.
No firmware, config or servo sign constant was changed by this record.

### Latest unloaded reference check (28 Oct 2026)

The active candidate zeros/signs above and the start-up home are unchanged.
On the installed arm, supported and unloaded, the five positioning joints were
set to exactly J1..J5 = 85, 145, 85, 87, 100° with J6 = 0. The two jaw-tip
heights measured from the floor/underplate were 445 and 440 mm; their mean is
442.5 mm. These are jaw-tip heights, not a held-block or object-centre
measurement. The 5 mm difference between the two tips is unexplained; it is not
turned into individual tip coordinates. No block was held during this check, so
object-centre alignment is **not** validated.

The straight-up measured chain is shoulder 94 + upper arm 114 + forearm 127 +
jaw tip 107 = 442 mm. The mean measured tip is 0.5 mm above that:
`442.5 - 442 = +0.5 mm`. This is one height comparison, not a validation of
either individual tip.

Geometry recorded for the offline reference (mm):

| Quantity | Value | Source |
|---|---|---|
| Floor/underplate -> shoulder | 94 | measured |
| Upper arm | 114 | measured |
| Forearm | 127 | measured |
| Wrist pitch -> jaw tip | 107 | measured |
| Object centre setback from the jaw tip | 3.5 | confirmed |
| Wrist pitch -> object centre along the tool (107 - 3.5) | 103.5 | derived |
| J1 axis -> shoulder forward | 23.15 | CAD |
| J1 axis -> wrist roll line lateral | 15.25 | CAD |
| Object x offset, CAD `+13` mapped to robot | -13 | CAD |
| Roll line -> object sideways (CAD midplane 27.7 - roll axis 33.43) | -5.73 | CAD |
| J1 axis CAD X, Z | 0, 18.18 | CAD |

The current workbook puts the table surface at CAD `Y = -34.5` (base plate
fitted), so its CAD floor-to-J2 is 94.22 mm. The older
[`arm-measurement-calculations.md`](arm-measurement-calculations.md) still uses
the pre-plate `-30.5` / 90.22 mm values. This check uses the measured 94 mm,
not either CAD floor value.

The offline reference `ProjectDevGuide/tests/host/arm_fk_reference.py` is an
explicit **hybrid position model**, not exact CAD and not production FK/IK.
It composes, for model angles q1..q5 in degrees:

```text
Rz(q1) * T(23.15, 0, 94) * Ry(q2) * T(0, 0, 114) * Ry(q3) *
T(0, 15.25, 127) * Rz(q4) * Ry(q5) * T(-13, -5.73, 103.5)
```

It uses the measured link lengths plus the CAD-only forward, lateral and
object offsets. It treats the three pitch axes as parallel `+Y` on the shared
forward line `x = 23.15` and collapses the J4 roll and J5 pitch points to the
end of the forearm. That is defensible for position: the roll axis is parallel
to the forearm, so its height along that line does not change the x/y offset it
rotates, and a pitch about `+Y` leaves the lateral y offset unchanged. It is
not the exact CAD axis layout: orientation, the J6 gripper, collisions and
servo limits are not modelled.

CAD places the shoulder at X = 23.15 but the wrist axes at X = 23.14. The model
folds that 0.01 mm difference into the 23.15 line, so the zero object is
x = 10.15 mm instead of the 10.14 mm CAD/measured target. The 0.01 mm is below
the recorded measurement resolution and is kept as a documented model offset,
not hidden as exact CAD. The mean jaw-tip height is not used to invent
individual tip 3D coordinates.

At q = 0 the model object centre is (10.15, 9.52, 438.5) mm, and the separate
straight-up jaw-tip height is 442 mm. The physical mean tip height was 442.5 mm,
so the observed residual is +0.5 mm. Because the two tip heights differ by
5 mm and no block was held, this remains a height cross-check, not an
object-centre, tool-centre or individual-jaw validation.

The offline check
`ProjectDevGuide/tests/host/test_kinematic_reference.py` exercises the
zero/sign mapping above and the position model in `arm_fk_reference.py`. It
does not command hardware, does not implement IK or orientation, and does not
certify the J2/J5 coupled limits or whole-arm collision safety. Run both with:

```text
python3 ProjectDevGuide/tests/host/test_kinematic_reference.py
python3 ProjectDevGuide/tests/host/arm_fk_reference.py 0 0 0 0 0
```

### Limit scope and collision caveat

The safe limits are independent per-joint endpoint clearances. They do **not**
guarantee collision-free whole-arm poses or paths, and the firmware has no
automatic collision check. Validate each new pose or path on the assembled arm,
supported, with the power disconnect within reach.

Do not infer installed values from the unloaded table, from the unit label, or
from the old IK sketches. The installed range can be narrower than the unloaded
tested range.

## After-assembly checklist

The confirmed limits and home are already in the firmware table. Use this list
to revalidate them and to establish the remaining zeros and signs.

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
      The current home values are already in the table; revalidate them after any
      mechanical change.
- [ ] Confirm each joint's positive direction with a small, slow move and record
      its sign. The directional observations above are not signs.
- [ ] Save `table` output before reset and keep the CSV outside the board.
- [ ] Revalidate the installed limits and home already recorded in
      `App/motion/servo_cal.c` after assembly or any mechanical change. Do not
      infer the remaining joint zeros or signs.
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
