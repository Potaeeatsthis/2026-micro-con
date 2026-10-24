# Robotic Arm Kinematics — To-Do

Offline kinematics plan for the Omartronics arm. Calibration bench code now runs
on the STM32 NUCLEO-F767ZI through PlatformIO and the Arduino core, without
CubeMX or STM32CubeIDE. The earlier Arduino Uno version is still available.
The six active servos have measured **unloaded** 0/90/180 pulses, but they are
**not installed-calibrated**. The canonical measured table is
[`ProjectDevGuide/docs/servo-calibration.md`](../ProjectDevGuide/docs/servo-calibration.md);
it is documentation only and is not auto-loaded by the console or firmware.
Work in this order:

1. Calibrate all six servo units unloaded. (Measurement done; see the canonical
   table above. Keep the saved CSV if console RAM was lost; do not replay
   unloaded endpoints on the assembled arm to recreate records.)
2. Assemble the arm and map units to joints (mapping confirmed; verify on the
   real arm).
3. Set narrower installed limits and the installed joint zero, then the home
   pose. These are still **not measured**.
4. Measure geometry, then build and validate FK and IK.

Do not skip physical measurement or validation. A commanded pose is not a
measured pose.

## 0. Current code and software status

Legacy Uno bench setup: **Arduino Uno**, servo signal on **pin 9**,
Serial Monitor at **115200 baud**, line ending **Newline** (or Both NL & CR),
one physical servo at a time, external regulated servo supply, common ground.
These are bench tools, not the full arm controller. The merged controller
skeleton is in [`RoboticArmMain/`](../RoboticArmMain/): app loop, safety,
motion/calibration, modes, voice, Pi link and LCD framework exist, but IK/FK and
geometry are still placeholders and the firmware still uses uncalibrated
installed limits/home. For your STM32, follow
[`code/stm32-servos/README.md`](../code/stm32-servos/README.md): the commands
below are shared, but the STM32 uses its own physical pins, not Uno pin 9.

| STM32 unit | Bench label | Output pin |
|---|---|---|
| 1 | MG996R-1 | PC6 |
| 2 | MG996R-2 | PC7 |
| 3 | MG996R-3 | PD12 |
| 4 | SG92R-1 | PD13 |
| 5 | SG92R-3 | PD14 |
| 6 | SG90 | PD15 |

- [ ] Install the PlatformIO extension in VS Code and open `code/stm32-servos`.
- [ ] Build the `nucleo_f767zi` environment, then connect the ST-LINK USB port.
- [ ] With servo power disconnected, upload and open the 115200-baud monitor.
- [ ] Follow the STM32 README's buffer wiring, input pulldowns, and supply checks.
- [ ] Test one positional servo at a time with the shared calibration commands.

The Uno setup/upload instructions below are alternatives only: skip those
board-specific steps if using STM32. Calibration and measurement steps apply
to both. No CubeMX or CubeIDE is needed for this STM32 bench project.

| File | Purpose / status |
|------|------------------|
| [`servo-calibration.ino`](../code/servo-calibration/servo-calibration.ino) | **Current calibration tool.** Six unit records, one physical servo at a time; nothing attached at startup; no auto-sweep. |
| [`README.md`](../code/servo-calibration/README.md) | Exact commands, guard behaviour, example session and RAM warning. |
| [`test_host.py`](../code/servo-calibration/test_host.py) | Host-only compile/behaviour test; no Arduino needed. |
| [`test-sg92r.ino`](../code/test-sg92r/test-sg92r.ino) | Basic one-link IK bench test. Its numbers are test values, not calibration. |
| [`test-sg92r-ik.ino`](../code/test-sg92r-ik/test-sg92r-ik.ino) | One-link IK using the fixed angle-to-pulse values in that sketch. They are not calibration for the new servos. |
| [`test-mg996r.ino`](../code/test-mg996r/test-mg996r.ino) | **Continuous-rotation** direction/speed test. It does not position a standard MG996R by angle. |

- [x] The calibration console, README, host test and three bench sketches exist
      in this repository.
- [x] The calibration console is the current bench tool for the six units.
- [x] Unloaded 0/90/180 pulses are measured for all six active units and are
      recorded in the canonical table. Installed limits/zero/home are not.
- [x] The HAL controller skeleton is merged in `RoboticArmMain/`; IK/FK and
      geometry remain incomplete (`ARM_GEOMETRY_MEASURED 0`).
- [x] Host tests passed (all eight scenarios):
      `python3 code/servo-calibration/test_host.py`
- [x] Arduino Uno compilation passed (software only; not uploaded).
- [ ] Compile and upload `servo-calibration.ino` to the Arduino Uno and open
      the Serial Monitor as described above.
- [ ] Every physical assembly, installed calibration, measurement and motion
      task below.

Host tests and Arduino compile checks are **software-only**. They do not prove
servo type, voltage, wiring, direction, mechanical limits, measured angles or
physical safety.

## 1. Scope and fixed facts

- [ ] Confirm the arm is the Omartronics model with five positioning
      rotary joints plus one gripper:
      - J1: base rotation
      - J2: shoulder
      - J3: elbow
      - J4: wrist pitch
      - J5: wrist roll
      - J6: gripper (not part of the XYZ positioning chain)
- [ ] Confirm all six servos are **positional**. The set is three MG996R plus
      `SG92R-1`, `SG92R-3` and `SG90`; they have unloaded measurements but no
      installed calibration.
      - A continuous-rotation MG996R has no angle control. The calibration
        console does not support it, and `test-mg996r.ino` only tests direction
        and speed.
      - If any unit is continuous-rotation, stop and replace it or change the
        design before calibration.
- [ ] Use these fixed unit IDs:
      - Units **1..3**: MG996R-1, MG996R-2, MG996R-3
      - Unit **4**: SG92R-1
      - Unit **5**: SG92R-3
      - Unit **6**: SG90
      These identify physical units only. They are **not** joint assignments by
      themselves; the current confirmed mapping is recorded in Section 4 and in
      the canonical table.
- [ ] Do not infer installed limits or home from a unit label. Verify the
      confirmed unit-to-joint map on the assembled arm and record it in
      Section 4.
- [ ] Use millimetres (mm) for lengths and degrees for model angles. The bench
      console commands pulses in microseconds (us).
- [ ] Use the known **bent reference pose** as the model zero pose, not the
      straight upright pose in the dimension drawing.
      `[bent reference pose photo / description: __________]`
- [ ] Record the TCP: a fixed, marked point on the gripper body.
      `[TCP description / photo: __________]`
- [ ] Agree the initial goal: position **XYZ only**, while J4 and J5 are held
      at fixed **commanded reference angles**.
      - This is not mechanical locking.
      - This does not keep the tool horizontal.
- [ ] Treat every pose as either **measured physically** or **commanded**.
      Without position feedback, a commanded pose is not proof of the real pose.

## 2. Bench safety and voltage checks

- [ ] Use an external regulated servo supply. Do not power servos from the
      Arduino 5 V pin. Connect servo ground to Arduino ground.
- [ ] **Verify the exact SG92R voltage rating.** Do not assume a blanket 6 V is
      acceptable, especially for continuous operation. Start conservatively and
      check temperature, buzz and strain.
- [ ] Verify the MG996R type. A standard MG996R is positional; a
      continuous-rotation version is speed-controlled and cannot be calibrated
      by angle.
- [ ] Keep the signal wire on pin 9 and keep one physical servo connected at a
      time for calibration.
- [ ] Support the servo or arm before `arm`. The first `arm` can jump to the
      requested pulse. No safe startup pose is assumed.
- [ ] Keep a hardware power disconnect within reach. Detaching the signal is
      not an emergency stop.
- [ ] `select` and `off` only detach the signal. They do not cut servo power
      and do not hold the arm; the servo can drop or move unpredictably when the
      signal stops. Support the arm before using either command.
- [ ] Do not force any powered joint by hand.
- [ ] Test new motion slowly, with a small range and the arm clear of people and
      objects.
- [ ] Do not run an automatic endpoint sweep. Do not force a servo to 0 or 180
      degrees to find its range.

## 3. Calibration first (before assembly)

Calibrate every unit unloaded before fitting it to the arm. The console stores
records for all six units, but only one physical servo is connected and moved
at a time.

### 3.1 Start the console

- [ ] Open `servo-calibration.ino` in Arduino IDE. Select the Arduino Uno and
      its port.
- [ ] Set the Serial Monitor to **115200 baud** and **Newline**
      (or Both NL & CR). The sketch expects one full line per command.
- [ ] Confirm the startup message says nothing is attached or moved. Type
      `help` to print the command list.
- [ ] Understand the memory limit before recording: all records are in RAM.
      A reset, opening or reopening the Serial Monitor (the Uno may auto-reset
      on DTR), or powering off the Arduino loses all six records. `table` only
      **prints** CSV; copy or save that text before any of those events.
- [ ] Run the host test if desired:
      `python3 code/servo-calibration/test_host.py`
      This is software verification only.

The console has no position feedback. `record` stores the angle **you measured**
at the current pulse; it does not measure the servo.

### 3.2 Procedure for each unit 1..6

1. Support the servo. Send `select N`, where N is 1..6. This detaches the
   signal, selects the unit, and resets the test window to 1400..1600 us when
   switching units. It does not command movement.
2. Connect only that unit. Apply servo power.
3. While the signal is still detached, widen `window <min> <max>` only if the
   first test pulse or a needed point is outside the current guard. Keep it
   inside 500..2500 us. This is a deliberate guard change, not a safety claim.
4. Send `arm <pulse_us>` inside the active guard. This attaches pin 9 and jumps
   directly to the requested pulse; it is not ramped and does not obey the
   10 us step limit. Choose a conservative first pulse; 1500 us is a common
   example, not a guaranteed safe centre for these servos.
5. Send `pulse <pulse_us>` for small changes while armed. Each command may
   change the pulse by at most 10 us; smaller steps are allowed. Wait for the
   servo to settle.
6. Measure the physical output angle with a protractor or angle gauge. Send
   `record <measured_deg>`. This stores the measured angle at the current pulse,
   not the commanded angle or a guessed value. Up to 9 points per unit; a
   duplicate pulse is rejected.
7. Repeat at spread points (at least three recommended) to check direction and
   approximate linearity. Do not sweep to the endpoints.
8. Move to the pulse where the measured output angle is 90 deg, then send
   `reference` to store that pulse as the unit reference. This is the reference
   pulse, not the arithmetic midpoint of the 0/180 pulses.
   This is not the assembled arm's bent zero pose; that is defined in Section 4.
9. Support the servo, send `off`, then send `limits <min> <max>` for the
   unloaded safe range. `limits` is accepted only while detached.
10. Send `table` and save the CSV output.
11. Support the servo, then cut **servo** power before swapping units. Keep the
    Arduino powered to retain RAM, or save `table` first. Repeat for the next
    unit.

Stop here before assembly. Do **not** set `installed` or `zero` until Section 4.

### 3.3 Guard, step and limit rules

- The default test `window` is **1400..1600 us**. It is only a guard to reduce
  the chance of an accidental large command; it is **not** a claim that the
  servo is safe over that range.
- `window <min> <max>` is accepted only while detached, must be inside
  500..2500 us, and must have min < max. Widening it only permits farther
  commands; it does not make them safe. Selecting a **different** unit resets
  the window to 1400..1600 us, so a widened window is not silently reused.
  Reselecting the same unit keeps the current window.
- `arm <pulse_us>` can jump immediately to the requested pulse. Every later
  `pulse <pulse_us>` is limited to a 10 us change per command; smaller steps are
  allowed.
- Every `arm` and `pulse` is checked against the intersection of the test
  window, unloaded limits and installed limits that have been set.
- `limits` is the unloaded safe range you establish by testing. `installed` is
  the narrower range after fitting the unit to the arm. Installed limits must be
  a subset of unloaded limits.
- Set unloaded `limits` first, then installed `installed`, then `zero`.
  `zero` is rejected until installed limits exist and the current pulse is
  inside them.
- Unloaded `limits` must contain the saved reference and any installed range.
  Installed limits must contain a saved joint zero; the narrower installed range
  **may exclude** the unloaded reference. Both ranges must contain a saved zero.
- Changing unloaded `limits` does not delete or rewrite calibration points. The
  sketch warns if stored points fall outside the new unloaded limits and keeps
  them as raw measurements.
- Use the exact commands from the README. Malformed lines, extra tokens,
  non-finite angles and overlong lines are rejected without moving the servo.

### 3.4 Example session

The command syntax below is real. **Every number is a made-up example, not a
physical fact about these servos.** Do not paste this as an automatic sequence.
Wait for each movement, measure the angle, then type `record`. Only enter limits
after unloaded testing. The measured unloaded values are in the canonical table;
they are not auto-loaded.

```text
help
select 1
arm 1500
record 90.0
reference
pulse 1510
record 93.0
pulse 1505
record 92.0
off
limits 1400 1600
table
```

In this example, `MG996R-1` ends with a reference at 1500 us (the pulse where
90 deg was measured) and unloaded limits 1400..1600 us, only if that interval
was physically checked and leaves adequate margin. Every `pulse` step is at
most 10 us. All recorded angles are examples only.

After installation, the narrower limits, zero and home are recorded separately:

```text
select 1
limits 1400 1600
window 1480 1520
arm 1500
pulse 1510
pulse 1505
off
installed 1480 1520
arm 1500
zero
table
```

Again, all numbers are examples. `installed` must be inside the unloaded range,
and `zero` must be inside the installed range. Measure the installed home pose
separately; do not infer it from the reference or from these example numbers.

### 3.5 Per-unit calibration records

Leave installed limits, zero and home blank until Section 4. Units are physical
IDs, not joints. The canonical table already records the confirmed unloaded
reference and test envelope for each active unit; it is not auto-loaded. Keep
historical points in your saved CSV and restore only unloaded `limits` while
detached for installed testing. Use the local table below for your session.

| Unit | Label | Positional confirmed? | Reference pulse (us) | Reference measured angle (deg) | Unloaded limits (us) | Installed limits (us) | Zero pulse (us) | Notes |
|------|-------|-----------------------|---------------------|--------------------------------|----------------------|-----------------------|-----------------|-------|
| 1 | MG996R-1 | [ ] yes [ ] no | | | | | | |
| 2 | MG996R-2 | [ ] yes [ ] no | | | | | | |
| 3 | MG996R-3 | [ ] yes [ ] no | | | | | | |
| 4 | SG92R-1 | [ ] yes [ ] no | | | | | | |
| 5 | SG92R-3 | [ ] yes [ ] no | | | | | | |
| 6 | SG90 | [ ] yes [ ] no | | | | | | |

Measured angle-pulse pairs. Enter only values you measured, in the form
`angle deg @ pulse us`.

| Unit | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 |
|------|---|---|---|---|---|---|---|---|---|
| 1 | | | | | | | | | |
| 2 | | | | | | | | | |
| 3 | | | | | | | | | |
| 4 | | | | | | | | | |
| 5 | | | | | | | | | |
| 6 | | | | | | | | | |

- [ ] Unit 1 MG996R-1: positional verified; reference saved; at least three
      measured pairs; unloaded limits saved; CSV saved.
- [ ] Unit 2 MG996R-2: same checks.
- [ ] Unit 3 MG996R-3: same checks.
- [ ] Unit 4 SG92R-1: same checks.
- [ ] Unit 5 SG92R-3: same checks.
- [ ] Unit 6 SG90: same checks.

### 3.6 Finish calibration before assembly

- [ ] All six units are confirmed positional.
- [ ] Each unit has a saved unloaded reference pulse and measured shaft angle.
- [ ] Each unit has measured angle-pulse pairs, not only commanded values.
- [ ] Each unit has an unloaded safe min/max range.
- [ ] `table` output has been copied or saved outside the Arduino.
- [ ] Each physical unit is labelled with its unit ID.
- [ ] No unit's values were copied from another unit or from an old sketch.
- [ ] Installed limits and zero are still blank; they belong after assembly.

## 4. Assembly, unit-to-joint mapping, installed limits

- [ ] Mount the units and support the arm before applying servo power.
- [ ] Verify on the assembled arm that each joint is driven by the unit in the
      confirmed mapping below; label the cables. Do not infer installed limits,
      zero or home from the unit label or from the old sketches.

| Joint | Unit ID (1..6) | Unit label | Installed limits (us) | Installed zero pulse (us) | Installed home pulse (us) | Sign (+/-) | Notes |
|-------|----------------|------------|-----------------------|---------------------------|---------------------------|------------|-------|
| J1 | 1 | MG996R-1 | | | | | |
| J2 | 2 | MG996R-2 | | | | | |
| J3 | 3 | MG996R-3 | | | | | |
| J4 | 4 | SG92R-1 | | | | | |
| J5 | 5 | SG92R-3 | | | | | |
| J6 | 6 | SG90 | | | | | |

For each installed unit, find the narrower installed range and set the installed
zero. Use small steps, support the arm and keep the power disconnect reachable.
Align the horn/linkage at a suitable known shaft reference before tightening it;
do not force the powered shaft. Support all other joints during one-servo tests.

If the controller reset since calibration, its records are empty. Use your saved
CSV to re-enter `limits <min> <max>` for each selected unit before setting
`installed`. The console has no CSV import: keep the saved angle-pulse pairs
outside the Arduino and retain both earlier and later exports.

1. `select N` for the unit.
2. While detached, set `window <min> <max>` to the unloaded range you recorded
   (or a narrower guard that contains the test). The default 1400..1600 us may
   not cover the unloaded range. This is still only a guard; the first `arm` can
   jump anywhere inside it.
3. Support the arm, connect that servo, apply servo power, and `arm <pulse_us>`
   near the expected zero with a conservative value. Use `pulse` changes of at
   most 10 us only.
4. Check clearance, linked-joint movement, buzz and strain. Find a narrower
   installed min/max. Do not run a full sweep.
5. Support the arm, send `off`, then send `installed <min> <max>`. It must be
   inside the unloaded limits.
6. `arm <zero_pulse_us>` inside the installed range, verify the physical zero
   pose, then send `zero`.
7. Move to the measured installed home pose and record its pulse separately
   from the joint zero.
8. `table` and save the updated CSV.

- [ ] `installed` is a subset of the unloaded limits, normally narrower.
- [ ] `zero` was accepted only after installed limits existed.
- [ ] The installed zero pulse may differ from the unloaded reference pulse;
      use the physical pose, not the old number.
- [ ] The installed home pulse was measured on the assembled arm, not inferred
      from 90 deg or from the unloaded reference.
- [ ] The installed range still leaves clearance at both ends under load.
- [ ] Each joint sign is confirmed by a small, slow commanded move.

Define positive joint rotation with the right-hand rule about its marked axis.
Use +1 if increasing the servo angle follows that direction, and -1 if it moves
opposite. The model uses degrees; the firmware converts calibrated degrees to
timer pulses.

## 5. Offline angle model (illustrative example)

This example is **offline only**. It does not drive hardware and does not send
PWM. The numbers are illustrative, not real calibrated values.

```python
"""Calibrated servo degrees to zero-relative model degrees. No hardware."""
import math


def model_angle(servo_deg, zero_deg, sign):
    values = (servo_deg, zero_deg)
    if not all(math.isfinite(v) for v in values):
        raise ValueError("all inputs must be finite")
    if sign not in (-1, 1):
        raise ValueError("sign must be -1 or +1")
    return sign * (servo_deg - zero_deg)


if __name__ == "__main__":
    # ILLUSTRATIVE ONLY: not real servo zero or calibration values.
    ZERO_DEG = 90.0
    assert model_angle(90.0, ZERO_DEG, +1) == 0.0
    assert model_angle(100.0, ZERO_DEG, +1) == 10.0
    assert model_angle(100.0, ZERO_DEG, -1) == -10.0

    for bad in (float("nan"), float("inf")):
        try:
            model_angle(bad, ZERO_DEG, +1)
            raise AssertionError("non-finite input was accepted")
        except ValueError:
            pass
    try:
        model_angle(0.0, ZERO_DEG, 0)
        raise AssertionError("invalid sign was accepted")
    except ValueError:
        pass
    print("Angle conversion checks passed.")
```

- [ ] Copy the Python block into `zero_angle_example.py`, run
      `python3 zero_angle_example.py`, and confirm the checks pass.
- [ ] Replace the illustrative constants only after Section 3 and Section 4 are
      complete.
- [ ] Use calibrated output-shaft angles in degrees, not raw pulse values.
- [ ] Keep this model offline; it does not enforce physical joint limits or
      command motion.

The per-unit measured pairs are the source for calibration. If a later fit is
needed, derive it from those measured points; do not reuse numbers from the old
IK sketches.

## 6. Measure the physical geometry

Use a square and plumb line. Define the origin on the base axis at the mounting
plane, with +X forward, +Y left, +Z up. Record signed projections.

Example format (hypothetical): `(160, -20, 230)` mm.

- [ ] Mark the origin and perpendicular X/Y lines on paper under the arm.
- [ ] Project the marked TCP vertically onto that paper with a square or plumb
      line.
- [ ] Measure the signed X and Y distances to the projected point.
- [ ] Measure Z vertically from the mounting plane to the TCP. If the paper is
      below that plane, subtract the plane's height above the paper.
- [ ] For example, 160 mm forward, 20 mm right and 230 mm above the origin gives
      `(160, -20, 230)` mm. These are example values, not this arm's dimensions.

Record full 3D offsets and axis directions, not only plastic-part lengths.
Measure the reference bends; do not assume 90 degrees from the perspective photo.

| Item | How measured | Value (mm) | Notes |
|------|--------------|------------|-------|
| Base origin to J2 axis | | | |
| J2 to J3 link length | | | |
| J3 to J4 link length | | | |
| J4 to J5 offset | | | |
| J5 to TCP offset | | | |
| Base height / mounting plane | | | |

| Axis / frame | Direction | Measured check | Notes |
|--------------|-----------|----------------|-------|
| +X | forward | | |
| +Y | left | | |
| +Z | up | | |
| J1 zero direction | | | |
| J2 zero direction | | | |
| J3 zero direction | | | |
| J4/J5 reference | | | |

| TCP measurement | Value | Notes |
|-----------------|-------|-------|
| TCP point description | | |
| TCP offset from J5 | | |
| Date / person | | |

- [ ] Do not write a guessed matrix. Fill the tables from measurements first.

## 7. Forward kinematics (FK) validation

Build FK from the measured values only. Compare model output with physically
measured TCP positions at several poses.

| Pose | J1 | J2 | J3 | J4 | J5 | Measured XYZ | Model XYZ | Error | Notes |
|------|----|----|----|----|----|--------------|-----------|-------|-------|
| 1 | | | | | | | | | |
| 2 | | | | | | | | | |
| 3 | | | | | | | | | |
| 4 | | | | | | | | | |

- [ ] Test the zero pose, small individual J1/J2/J3 changes and a combined pose,
      all comfortably inside known safe limits.
- [ ] Record the error in mm and investigate any large error before IK.
- [ ] Keep J4 and J5 at the chosen reference angles during XYZ tests.
- [ ] Keep sending those fixed commands; do not mechanically lock the joints.
      Move only J1-J3 and leave J6 unchanged. Keep wrist/TCP offsets in the
      model.

## 8. Inverse kinematics (IK) and motion limits

- [ ] Implement IK for XYZ with J4/J5 fixed at the reference angles.
- [ ] Reject unreachable targets and report them; do not clamp silently.
- [ ] Derive joint limits from the installed safe pulse ranges and the
      calibrated angle conversion. Do not guess them.
- [ ] Run every IK solution through FK and compare it with the requested XYZ.
- [ ] Check the whole path for self-collision, table contact and obstacles, not
      only the final position. Small steps alone do not guarantee safety.
- [ ] Plan paths with small steps and a slow speed.
- [ ] Test with the arm supported and the stop within reach.
- [ ] Compare commanded XYZ with measured XYZ after every test move.

| Joint | Min (deg) | Max (deg) | Source (installed pulse range) | Notes |
|-------|-----------|-----------|--------------------------------|-------|
| J1 | | | | |
| J2 | | | | |
| J3 | | | | |
| J4 | | | | |
| J5 | | | | |

## 9. Full arm controller (merged skeleton, still incomplete)

The Uno sketches and STM32 calibration project are bench tools, separate from
the HAL firmware. [`RoboticArmMain/`](../RoboticArmMain/) now contains the merged
controller skeleton: app loop, safety, motion/calibration, modes, voice, Pi link
and LCD framework. It is **not a validated complete controller**: IK/FK and arm
geometry are placeholders (`ARM_GEOMETRY_MEASURED 0`, so `kin_ik()`/`kin_fk()`
return `KIN_NOT_READY`), and the motion table still uses uncalibrated
`min/max/home` placeholders.

The hardware plan for the merged controller is:

- **STM32 NUCLEO-F767ZI** generating six **direct hardware PWM** signals,
  not the Arduino Servo library.
- **SN74AHCT244N DIP-20** buffer converting the six 3.3 V logic signals to 5 V
  logic. VCC = 5 V, common ground, `/1OE` and `/2OE` held LOW, 100 nF decoupling.
- Separate regulated servo supply. Servo power must not pass through the STM32
  or the SN74AHCT244N. All devices share one signal ground.
- Verify the complete pin/timer map against the board documentation, system
  guide, and STM32 Arduino core pin definitions before wiring.

- [ ] Convert measured installed min/max and home pulses to the existing
      servo-angle coordinate before updating the degree fields in
      `App/motion/servo_cal.c`. Store the measured joint zero and sign in the
      records for the later joint-coordinate implementation; the current table
      has no zero/sign fields. The current `0/180/90` values are placeholders,
      not safe installed limits. Do not put microseconds into degree fields.
- [ ] Enforce the measured installed limits in the firmware, not only in the
      bench console.
- [ ] Implement and validate IK/FK against measured geometry; remove the
      placeholders only after physical checks.
- [ ] Verify the STM32 pin/timer map and buffer wiring on the real board before
      connecting servos.
- [ ] The 115200-baud Newline console remains the Uno/STM32 calibration
      interface. The controller uses calibrated values internally, not this
      text protocol.

See [the system guide](robotic-arm-guide.md) and
[the build spec](robotic-arm-final-build-spec.md) for the planned wiring, and
[the canonical calibration table](../ProjectDevGuide/docs/servo-calibration.md)
for the measured unloaded values. The merged skeleton exists, but IK/geometry
and installed calibration are still open; do not treat it as tested.

## 10. Hardware and firmware information to fill in

| Item | Current bench | Planned final | Notes |
|------|---------------|---------------|-------|
| Board model | Arduino Uno or STM32 NUCLEO-F767ZI | STM32 NUCLEO-F767ZI | Choose the matching bench project |
| Servo signal | Uno pin 9 or STM32 pins listed above | TIM3/TIM4 via SN74AHCT244N | Bench build verified; final peripheral integration still needs checking |
| Driver model | Uno Servo library or STM32 HardwareTimer | Direct hardware PWM + SN74AHCT244N | Buffer carries signals only |
| Number of channels | 1 at a time | 6 | One unit at a time on the bench |
| Channel-to-joint map | J1..J6 = units 1..6 (confirmed) | Same map; installed calibration blank | Verify on the assembled arm; fill installed values after Section 4 |
| Calibrated command format | `arm`/`pulse` in us | Calibrated angle/pulse plus limits | Final firmware converts |
| Command units | us on the bench | degrees in the model | Do not mix raw PWM with model angles |
| Power / stop method | External supply + disconnect | Same, plus hardware E-stop | Detaching signal is not a power cut |

- [ ] Confirm the above with the real hardware before writing firmware.
- [ ] Keep the offline model separate from hardware-driving code.
