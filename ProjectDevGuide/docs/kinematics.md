# Kinematics (FK / IK)

Owner: IK / mechanical member. Code: `RoboticArmMain/App/kinematics/`.
This is math only. It never writes PWM, never calls `motion_set_target()`, and never
runs in an ISR.

## Activation state

`ARM_GEOMETRY_MEASURED` is currently **0** in `arm_geometry.h`. Both `kin_fk()` and
`kin_ik()` then return `KIN_NOT_READY` immediately, before any input check. The LCD
shows `---`, Cartesian stays not ready, and Voice cannot start an IK move. The guard
can be overridden only for a host test with `-DARM_GEOMETRY_MEASURED=1`; firmware
stays fail-closed until the owner explicitly changes the default.

`ARM_GEOMETRY_MEASURED` is an activation guard, not a claim that the arm is measured.

## Geometry source and provenance

The single source is `Robotic_Arm_Kinematics_Workbook.xlsx`, copied into
`arm_geometry.h`:

```text
S    = (23.15, 15.25, 94.22) mm
v_SE = (-0.01, 0, 113.12) mm
v_EW = (0, 0, 126.40) mm
t    = (-12.99, -5.73, 104.76) mm
TCP0 = (10.15, 9.52, 438.5) mm
servo zero J1..J5 = 85, 145, 88, 88, 96 deg
servo sign J1..J5 = +1, -1, +1, +1, -1
```

Frame: robot base, mm, `+X` forward, `+Y` left, `+Z` up. The robot origin is the
J1 axis on the floor. The CAD floor is Fusion `Y = -34.5`, so
`p_robot = (-X_cad, Z_cad - 18.18, Y_cad + 34.5)` with the foot height assumed 0.

The older `arm-measurement-calculations.md` hybrid (measured `94/114/127/103.5`)
is not the firmware model. The workbook TCP X/Y are CAD-derived/imposed; TCP Z is
derived from the measured 446 mm block top minus 15/2 mm, so it is not an
independently measured tool centre. The zeros and signs are candidate values; J5
is explicitly provisional in the workbook. These are not hardware proof.

## API and status

```c
kin_status_t kin_fk(const joint_vec_t *joints, cart_pose_t *out);
kin_status_t kin_ik(const cart_pose_t *pose, const joint_vec_t *seed, joint_vec_t *out);
```

`joint_vec_t.deg[]` is **servo degrees 0..180**, not model angles. FK converts with
`q = sign * (servo - zero)`. FK accepts any finite angles; it does not enforce the
servo software limits, so the CAD test vectors can be checked outside the installed
ranges. IK enforces the installed limits.

Outputs are written only on `KIN_OK`. Every failure preserves `out`. Null arguments
are handled after the activation guard.

| Status | Meaning |
|---|---|
| `KIN_OK` | output written |
| `KIN_OUT_OF_REACH` | null/non-finite/invalid input, or no geometric solution |
| `KIN_JOINT_LIMIT` | a geometric solution exists, but a servo limit, the J2/J5 coupling, or seed J6 fails |
| `KIN_NOT_READY` | geometry disabled, or FK cannot express a pitch from the approach projection |

## FK

Full 5-joint model, including wrist roll:

```text
p = Rz(q1) * [ S + Ry(q2) * v_SE
                   + Ry(q2+q3) * ( v_EW + Rz(q4) * Ry(q5) * t ) ]
```

`cart_pose_t` has only one orientation scalar, so `pitch_deg` is the approach
projected into the yaw-removed X-Z plane:

```text
pitch = atan2(approach_x, approach_z)
```

It is exact only when `q4 = 0`. With roll it is a projection, not a full tool
orientation; do not present it as one. If the projection is below about `1e-4`,
FK returns `KIN_NOT_READY` and leaves `out` unchanged.

## IK

Workbook planar closed form with roll fixed at `q4 = 0` (servo J4 reference 88 deg):

```text
y_t = S.y + t.y
r   = hypot(x, y)
xp  = sqrt(r^2 - y_t^2)              positive radial branch only

o_x =  t.x*cos(phi) + t.z*sin(phi)
o_z = -t.x*sin(phi) + t.z*cos(phi)
dx  = xp - o_x - S.x
dz  = z  - o_z - S.z

D     = (dx^2 + dz^2 - L2^2 - L3^2) / (2*L2*L3)
gamma = +/- acos(D)                  both elbow branches
q1    = atan2(y, x) - atan2(y_t, xp)
q2    = atan2(dx,dz) - atan2(L3*sin(gamma), L2+L3*cos(gamma)) - delta
q3    = gamma + delta
q5    = phi - q2 - q3
```

`delta = atan2(v_SE.x, v_SE.z) = -0.005065 deg`. `D` is clamped only inside
`1e-5`; a larger overshoot is `KIN_OUT_OF_REACH`.

Branch rules:

- Both elbow branches are evaluated. The valid one closest to all five seed
  angles is returned; ties keep the `+gamma` branch.
- Only the positive `xp` radial branch is implemented. The negative radial
  solution is a different base configuration and is **not** enumerated. This is a
  documented workspace limitation.
- `r < |y_t|` is out of reach. Exact tangency (`r = |y_t|`) is the branch meeting
  point; it is not specially claimed safe.
- Periodic joint solutions are normalized to the installed 0..180 deg range and
  accepted only if the representative is within the servo limits. This keeps
  equivalent `q1 +/- 360`, `q5 +/- 360`, etc. from producing a different arm pose.
- `pitch_deg` is finite and is reduced with `fmod(..., 360)` before use, so large
  finite values are accepted. Non-finite pitch is `KIN_OUT_OF_REACH`.

Limits and coupling:

- Installed min/max come from `servo_cal()` (`App/motion/servo_cal.c`); do not
  duplicate the table.
- The J2/J5 rule uses `MOTION_J2_COUPLED_DEG` and `MOTION_J5_WRIST_UP_DEG` and
  checks both the current seed and the full target, so IK does not hand motion a
  target that `motion_set_target()` would clamp.
- J6 is not a positioning joint. Its seed angle must be inside the J6 calibration
  range; on success it is copied to the output unchanged.
- If no geometric solution exists, the result is `KIN_OUT_OF_REACH`. If geometry
  exists but every candidate fails a limit/coupling, the result is
  `KIN_JOINT_LIMIT`.

## Tests

`ProjectDevGuide/tests/host/test_kinematics.c` is compiled twice by `run.sh`:

```text
default                  guard 0: NOT_READY and unchanged outputs
-DARM_GEOMETRY_MEASURED=1 -lm: the math, limits and error paths
```

The math build checks the nine workbook `Test_Vectors` FK cases, full-roll FK
pitch projection, degenerate approach refusal, safe IK round trips, both elbow
branches with seed selection, periodic pitch, J2/J5 coupling, servo limits, J6,
null/non-finite inputs, output preservation, and `out == seed`.

Run from the repo root:

```text
sh ProjectDevGuide/tests/host/run.sh
python ProjectDevGuide/ci/build_firmware.py
python ProjectDevGuide/ci/check_rules.py
```

No board or servo test is performed by these host checks. Turning the guard on is
a separate owner decision and must be followed by a hardware test with the servo
signal wires disconnected first.
