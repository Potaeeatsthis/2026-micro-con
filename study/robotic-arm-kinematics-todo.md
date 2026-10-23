# Robotic Arm Kinematics — To-Do

Offline kinematics plan for the assembled Omartronics arm. Work through the
steps in order. Do not skip the measurement and validation steps.

## 1. Fixed facts and scope

- [ ] Confirm the arm is the assembled, unmodified Omartronics arm with five
      positioning rotary joints plus one gripper:
      - J1: base rotation
      - J2: shoulder
      - J3: elbow
      - J4: wrist pitch
      - J5: wrist roll
      - J6: gripper (not part of the XYZ positioning chain)
- [ ] Confirm the motors are already calibrated. **Do not recalibrate them.**
- [ ] Use millimetres (mm) for all lengths and degrees for all angles.
- [ ] Record the chosen zero pose: the **bent pose photo**, not the straight
      upright pose in the dimension drawing. Link this photo here:
      `[zero pose photo path: __________]`
- [ ] Record the TCP: a fixed, marked point on the gripper body.
      `[TCP description / photo: __________]`
- [ ] Agree the initial goal: position **XYZ only**, while J4 and J5 are held
      at fixed **commanded reference angles**.
      - This is not mechanical locking.
      - This does not keep the tool horizontal.
- [ ] Treat every pose as either **measured physically** or **commanded**.
      Unless position feedback is available, a commanded pose is not proof
      of the real pose.

## 2. Startup safety

- [ ] Do not force any powered joint to move by hand.
- [ ] Support the arm before powering motors, so it cannot fall.
- [ ] Keep the power stop reachable at all times.
- [ ] Do not assume the startup pose is safe. Check the physical pose first.
- [ ] Test new motion slowly, with a small range and the arm clear of people
      and objects.

## 3. Record servo zero commands and signs

For each joint, record the command that produces the chosen zero pose and the
direction sign. Leave values blank until measured. Do not guess.

| Joint | Motor channel | Zero command | Sign (+/-) | Command format | Notes |
|-------|---------------|--------------|------------|----------------|-------|
| J1    |               |              |            |                |       |
| J2    |               |              |            |                |       |
| J3    |               |              |            |                |       |
| J4    |               |              |            |                |       |
| J5    |               |              |            |                |       |
| J6    |               |              |            |                |       |

- [ ] Confirm each sign by a small, slow commanded move.
- [ ] Define positive joint rotation using the right-hand rule about its
      marked axis. Use +1 if increasing the servo angle follows that direction,
      and -1 if it moves in the opposite direction.
- [ ] Record whether commands are degrees or another unit. The model uses
      degrees; firmware converts them to the driver format.

## 4. Offline angle model (illustrative example)

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
- [ ] Replace the illustrative constants only after Section 3 is complete.

Inputs must be calibrated output-shaft angles in degrees, not raw PWM values.
This example does not enforce physical joint limits or command motion.

## 5. Measure the physical geometry

Use a square and plumb line. Define the origin on the base axis at the
mounting plane, with +X forward, +Y left, +Z up. Record signed projections.

Example format (hypothetical): `(160, -20, 230)` mm.

- [ ] Mark the origin and perpendicular X/Y lines on paper under the arm.
- [ ] Project the marked TCP vertically onto that paper with a square or plumb line.
- [ ] Measure the signed X and Y distances to the projected point.
- [ ] Measure Z vertically from the mounting plane to the TCP. If the paper is
      below that plane, subtract the plane's height above the paper.
- [ ] For example, 160 mm forward, 20 mm right, and 230 mm above the origin
      gives `(160, -20, 230)` mm. These are example values, not this arm's dimensions.

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

## 6. Forward kinematics (FK) validation

Build FK from the measured values only. Compare model output with physically
measured TCP positions at several poses.

| Pose | J1 | J2 | J3 | J4 | J5 | Measured XYZ | Model XYZ | Error | Notes |
|------|----|----|----|----|----|--------------|-----------|-------|-------|
| 1    |    |    |    |    |    |              |           |       |       |
| 2    |    |    |    |    |    |              |           |       |       |
| 3    |    |    |    |    |    |              |           |       |       |
| 4    |    |    |    |    |    |              |           |       |       |

- [ ] Test the zero pose, small individual J1/J2/J3 changes, and a combined pose,
      all comfortably inside known safe limits.
- [ ] Record the error in mm and investigate any large error before IK.
- [ ] Keep J4 and J5 at the chosen reference angles during XYZ tests.
- [ ] Keep sending those fixed commands; do not mechanically lock the joints.
      Move only J1-J3 and leave J6 unchanged. Keep wrist/TCP offsets in the model.

## 7. Inverse kinematics (IK) and motion limits

- [ ] Implement IK for XYZ with J4/J5 fixed at the reference angles.
- [ ] Reject unreachable targets and report them; do not clamp silently.
- [ ] Record joint limits and test them in the model.
- [ ] Run every IK solution through FK and compare it with the requested XYZ.
- [ ] Check the whole path for self-collision, table contact, and obstacles,
      not only the final position. Small steps alone do not guarantee safety.
- [ ] Plan paths with small steps and a slow speed.
- [ ] Test with the arm supported and the stop within reach.
- [ ] Compare commanded XYZ with measured XYZ after every test move.

| Joint | Min (deg) | Max (deg) | Source | Notes |
|-------|-----------|-----------|--------|-------|
| J1    |           |           |        |       |
| J2    |           |           |        |       |
| J3    |           |           |        |       |
| J4    |           |           |        |       |
| J5    |           |           |        |       |

## 8. Hardware and firmware information to fill in

These fields are needed before firmware work starts. Leave blank if unknown.

| Item | Value | Notes |
|------|-------|-------|
| Board model | | |
| Driver model | | |
| Number of channels | | |
| Channel-to-joint map | | |
| Calibrated command format | | |
| Command units | | |
| Power / stop method | | |

- [ ] Confirm the above with the real hardware before writing firmware.
- [ ] Keep the offline model separate from hardware-driving code.
