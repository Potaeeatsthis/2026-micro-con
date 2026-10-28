# Kinematics (FK / IK)

Owner: IK / mechanical member. Code: `RoboticArmMain/App/kinematics/`.
This is math only. It never writes PWM, never calls `motion_set_target()`, and never
runs in an ISR.

## Activation state

FK and the two IK entry points have separate guards in `arm_geometry.h` (28 Oct 2026):

| Guard | Default | Effect |
|---|---|---|
| `ARM_FK_ENABLED` | **1** | `kin_fk()` works. The manual-mode LCD pages show X/Y/Z labelled **"XYZ ESTIMATED, mm"**: FK of the *commanded* angles (no joint sensors) with candidate zeros, so an estimate, not a measurement. FK never commands motion. |
| `ARM_CARTESIAN_IK_ENABLED` | **1** | Cartesian mode may call `kin_ik_cartesian()`. The owner authorised this after the supervised bench move. The mode is ready, the joystick/knob move it, and the gripper is manual. It does **not** unlock voice. |
| `ARM_GEOMETRY_MEASURED` | **0** | The global/voice IK activation guard. `kin_ik()` returns `KIN_NOT_READY` immediately, before any input check, so Voice cannot start an IK move. |

`kin_fk_enabled()` / `kin_ik_enabled()` / `kin_ik_cartesian_enabled()` report the guards at run
time. The global guard can be overridden only for a host test with `-DARM_GEOMETRY_MEASURED=1`;
firmware keeps global/voice IK fail-closed until the owner explicitly changes the default.
`ARM_GEOMETRY_MEASURED` is an activation guard, not a claim that the arm is measured.

The IK bench build (`APP_USE_IK_BENCH_TEST`) is special: it compiles the solver for
`kin_ik_bench()` only. `kin_ik()` and `kin_ik_cartesian()` both refuse in that build, even if
`ARM_CARTESIAN_IK_ENABLED` is 1, so the bench screen cannot activate a normal caller.

Cartesian was activated after the supervised bench move. Global/voice IK is still pending:
voice pose moves stay blocked until the owner activates global IK and the board check is repeated
with the servo signal wires disconnected first.

## IK callers (28 Oct 2026: Cartesian on, global/voice off)

**Cartesian vs voice.** Cartesian mode calls `kin_ik_cartesian()`; Voice still calls `kin_ik()` and
therefore stays blocked. The two functions share the same solver and status rules; only the activation
guard differs.

**Cartesian entry and roll.** The mode never sets a target or moves the arm on entry. It forces
`SPEED_FINE` (the operator can still toggle FAST), reads the current pose with FK, and keeps that FK
pitch for every Cartesian move. IK fixes J4 at `ARM_J4_ZERO_DEG` (88 deg), so the first **accepted**
move from the installed home (J4 90) rolls J4 by the bench-tested 2 deg; the other joints make the
small endpoint corrections needed to keep the requested X/Y/Z/pitch. Between moves, motion is
joint-interpolated, not a straight Cartesian line. This is not a full orientation command:
`cart_pose_t` carries only the projected pitch. If J4 is more than 2.1 deg from the reference at entry
(`CART_MAX_ROLL_DEG`: the tested 2 deg plus 0.1 deg margin), the mode stays blocked and shows
`ALIGN J4 <reference>`. Align J4 in a manual mode and re-enter; there is no automatic alignment.
The block reason (`ALIGN J4 88`, or the IK status) is shown on the LCD Cartesian page, in yellow on the
line under the X/Y/Z rows, when the page opens and again on every knob turn / press or stick push
while blocked; refused moves (`JOINT LIMIT`, `OUT OF REACH`) show there too (28 Oct 2026). J4 is the
Y axis of joint group 2, so using group 2 is the usual way to end up blocked.

**Cartesian controls.** The joystick moves X/Y at 20 mm/s in FINE and 60 mm/s in FAST: stick up = forward
(+X), stick right = the arm's right (-Y), for an operator behind the arm (28 Oct 2026; mode entry
resets to FINE). One encoder detent moves Z by 5 mm. The encoder press toggles the gripper; J6 can be
commanded to 0 deg (closed) and there is no deadman. Centring the stick issues no new request, but it
does **not** cancel an already queued target: the arm continues to the last accepted target. Exit calls
`motion_hold()`. There is no automatic collision checker and no software stop or emergency-stop
guarantee; switch the servo supply off for a hard stop. Keep the workspace clear and be ready to leave
the mode.

**Failure handling.** IK or exact-motion failure calls `motion_hold()`, so an older pending target
cannot keep moving the arm. `s_pose` stays the last **accepted requested** pose for the LCD, but both
the encoder and XY control increments after a failure rebase on the FK of the held **current** pose.
Otherwise a reverse input could resume a cancelled target that was still ahead of the arm. If that
held-pose FK fails, the mode blocks and shows the FK status until it is re-entered.

**Pitch.** `pitch_deg` is the tool approach in the arm's vertical plane: **0 = gripper pointing up,
180 = straight down**, smaller = tilted toward the reach. Home is about 148. Voice used 0 (up) for every
grasp and place, so IK refused every table target. Now grasp and place **search the pitch**:
180 first, then 10 deg less at a time down to 120 (`GRASP_PITCH_*` in `mode_voice.c`). The first pitch
for which **every pose of the script** solves (above, grasp / release, lift) is used for the whole
script. The same pitch is requested at each waypoint; pitch may change during travel. The poses are solved in order, each from the
previous solution, so the J2/J5 check is the one motion will see. If no pitch fits, the LCD shows the
IK status and nothing moves (the gripper does not open). The jog queue keeps the pitch it is holding.

**Endpoint orientation only.** The pitch holds AT each waypoint (above, grasp, lift). Between waypoints
motion moves every joint on its own at its speed limit (no Cartesian interpolation, joints do not arrive
together), so the tool path is not straight and the pitch changes during the move. The descent onto the
box is therefore **not** guaranteed vertical or orientation-preserving.

**Grasp height.** The Pi reports the **centre of the box's top face**; the TCP is the centre of the
**held** box (TCP z = block top - 15 / 2). Voice grasps at the top minus `GRASP_BELOW_TOP_MM` (7.5 mm,
the documented 15 mm block; check on the real gripper). `HAND_RELEASE_MM` / `STATION_RELEASE_MM` are
also TCP heights (held box centre) above the palm / station top.

With the candidate geometry and the installed limits the reachable band is narrow. Reachable Z (mm) at
Y = 0, from home (host scan, test only):

| X | p180 | p170 | p160 | p150 | p140 |
|---|---|---|---|---|---|
| 200 | -50..52 | 76..84 | 108..114 | 134..146 | 158..178 |
| 220 | -32..40 | -44..78 | 76..112 | 112..146 | 140..180 |
| 240 | -6..22 | -26..68 | -34..106 | 76..142 | 114..178 |
| 260 | - | 2..52 | -16..96 | -22..136 | 72..172 |
| 280 | - | - | 16..80 | 0..122 | -4..162 |

A grasp needs the grasp Z **and** grasp Z + `APPROACH_MM` (50) at one pitch: e.g. a box whose top is at
240, 0, 30 (grasp Z 22.5) uses pitch 160; the pi protocol example 182, -45, 30 is refused at every pitch. From home, Cartesian can move
down but not up (J5 is at its 25 deg minimum).

**Clamped / refused targets.** IK callers now use `motion_set_target_exact()`: all or nothing. If motion
would clamp any joint (table limit or J2/J5 coupling), the target is **not** changed, so the arm never
starts toward a different pose than planned.

| Caller | On `MOTION_CLAMPED` / `MOTION_REFUSED` |
|---|---|
| Cartesian | `motion_hold()`, the pose stays the last accepted request, LCD `JOINT LIMIT` / `MOTION REFUSED` |
| Voice script step | `motion_hold()`, LCD `JOINT LIMIT`, the step fails -> `MOVE FAILED` -> S10 (stop safely) |
| Voice home / raise | same (`HOME BLOCKED` for home) |

## IK bench test (one supervised move, global/voice guard still 0)

`App/ik_bench_test/` (`APP_USE_IK_BENCH_TEST`, off by default, never on `master`) solves **one** fixed
target with `kin_ik_bench()` (the same solver without the guard) and moves there only on a knob press.
`kin_ik()` still returns `KIN_NOT_READY` in that build. The Cartesian IK API is also disabled there
(`kin_ik_cartesian_enabled()` false, `kin_ik_cartesian()` refuses), so the bench screen is the only
solver caller.

Default target **X 250, Y 13, Z 120 mm, pitch 150** (27 mm below home). IK gives
J1..J5 = 85.8, 125.9, 157.5, 88.0, 34.6 deg; the biggest change from home is J3, 12.5 deg. The gripper
never moves. A solution that moves any joint more than 30 deg (`IK_BENCH_MAX_DELTA_DEG`) is refused.

`app_config_local.h` for the bench:

```c
#define APP_USE_IK_BENCH_TEST  1
#define APP_USE_SERVO_CAL_TEST 0
#define APP_USE_MOTION         1
#define APP_USE_MODES          0
#define APP_USE_LCD            0
#define APP_USE_INPUT          1
#define APP_USE_KINEMATICS     1
#define APP_USE_VOICE          0
#define APP_USE_PI             0
#define APP_SPEED_FINE_DEG_S   5.0f   /* slower than the normal 10 deg/s */
```

Procedure:

1. Clear the space around the arm (about 30 mm below the home tool point and the path to it).
   Have the servo power switch in reach.
2. Power on. Motion sends home one joint at a time. The screen shows the target, the IK angles,
   `NOW` (commanded), `EST` (FK of now), `MAX MOVE` and `KNOB PRESS = GO`. Check the IK angles above.
   GO needs a **new** knob press: its debounced edge must come at least 0.5 s (`IK_BENCH_ARM_MS`) after
   `KNOB PRESS = GO` appears. This is judged by the edge time recorded in the input tick, not by when the
   main loop handles the event, so a late-handled early press never counts. A knob held through start-up
   counts only after it is released and pressed again.
3. Knob press: the arm moves at fine speed. **Any press (knob, or stick press with the stick centred)
   stops it** (`STOPPED`); the servo power switch is the hard stop.
4. Arrived (`MEASURE. PRESS = HOME`): `EST` and `ERR` are the FK estimate of the reached pose and its
   difference from the target, also printed on USART3. Measure the real tool point (robot frame: origin
   on the floor under the J1 axis, +X forward, +Y left, +Z up) and write down the difference.
5. Knob press: back home, then ready again.

Pass: the move is smooth, no joint hits a limit, and the measured tool point is close to the target.
A large error means the candidate zeros / signs (`arm_geometry.h`) need a fix before global/voice
IK is activated.

**Board check (FK only):** at J1..J5 = 85, 145, 88, 88, 96 deg (the candidate zeros) the LCD should
show about **10, 9, 438** (X/Y/Z, whole mm; FK gives 10.15, 9.52, 438.50). The values must follow slow
manual moves. Voice IK moves must stay blocked; Cartesian is active in normal firmware and must obey
the entry roll guard above.

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
kin_status_t kin_ik_cartesian(const cart_pose_t *pose, const joint_vec_t *seed, joint_vec_t *out);

bool kin_fk_enabled(void);
bool kin_ik_enabled(void);              /* global/voice guard */
bool kin_ik_cartesian_enabled(void);    /* Cartesian guard; false in a bench build */
```

`joint_vec_t.deg[]` is **servo degrees 0..180**, not model angles. FK converts with
`q = sign * (servo - zero)`. FK accepts any finite angles; it does not enforce the
servo software limits, so the CAD test vectors can be checked outside the installed
ranges. IK enforces the installed limits.

Outputs are written only on `KIN_OK`. Every failure preserves `out`. Each IK entry
point checks its own activation guard before looking at the inputs.

| Status | Meaning |
|---|---|
| `KIN_OK` | output written |
| `KIN_OUT_OF_REACH` | null/non-finite/invalid input, or no geometric solution |
| `KIN_JOINT_LIMIT` | a geometric solution exists, but a servo limit, the J2/J5 coupling, or seed J6 fails |
| `KIN_NOT_READY` | the caller's guard is off (global/voice or Cartesian, including any bench build), or FK cannot express a pitch from the approach projection |

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

Cartesian mode uses `kin_ik_cartesian()`; Voice still uses `kin_ik()` and stays fail-closed. Both
share the same solver. The solver is the workbook planar closed form with roll fixed at `q4 = 0`
(servo J4 reference 88 deg):

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

`test_ik_bench.c` runs the bench screen with the global guard at 0 (`kin_ik()` refused, `kin_ik_bench()`
solves). It also checks that the bench build keeps the Cartesian API off even though
`ARM_CARTESIAN_IK_ENABLED` is 1, and that a refusal preserves `out`. The rest of the bench test uses the
real `input.c` path: a knob held across start-up, a press queued on the last start-up tick, and a press
100 ms after READY that is handled only after the window are all not GO; a new press is. Go / stop /
home, exact target.

The default `test_app.c` build checks that Cartesian is ready while voice/global `kin_ik()` still
refuses; entering Cartesian sets no target/motion and forces FINE; Z -5 mm at home is accepted and
finishes J4 at the reference; Z +5 mm is refused, holds the previous target and does not advance the
accepted pose; a Z move leaves J6 alone and an explicit encoder press still toggles the gripper. It
also seeds J4 at 120 deg, checks the `ALIGN J4 <reference>` block with no knob/gripper target change,
then aligns J4 and re-enters. `test_app.c` is also built with `-DARM_GEOMETRY_MEASURED=1 -DAPP_TEST_IK=1`
for the full IK callers (pitch search picks 160 for a box top at 240, 0, 30; 182, -45, 30 is refused
before anything moves; Cartesian keeps its last accepted pose at a limit).

`test_cartesian.c` is built with `APP_USE_INPUT=1` and calls the real `MODE_CARTESIAN_OPS` with a fake
joystick. It checks that X+ / Y+ change only the expected axis in one 20 ms step, that centring issues
no new request, that a refusal holds the current pose and the next XY retry rebases on it (not on the
cancelled request), that voice `kin_ik()` stays blocked, and that entry/XY never command the gripper.

`ProjectDevGuide/tests/host/test_kinematics.c` is compiled three times by `run.sh`:

```text
default                                  Cartesian on, global/voice off: Cartesian round trip, kin_ik NOT_READY
-DARM_CARTESIAN_IK_ENABLED=0             both normal IK APIs NOT_READY, unchanged outputs; FK still on
-DARM_GEOMETRY_MEASURED=1 -DARM_CARTESIAN_IK_ENABLED=0 -lm
                                         the math, limits and error paths
```

The math build checks the nine workbook `Test_Vectors` FK cases, full-roll FK
pitch projection, degenerate approach refusal, safe IK round trips, both elbow
branches with seed selection, periodic pitch, J2/J5 coupling, servo limits, J6,
null/non-finite inputs, output preservation, and `out == seed`.

Run from the repo root:

```text
sh ProjectDevGuide/tests/host/run.sh
python3 ProjectDevGuide/tests/host/test_kinematic_reference.py
python3 ProjectDevGuide/tests/host/test_kinematics_workbook.py
python ProjectDevGuide/ci/build_firmware.py
python ProjectDevGuide/ci/check_rules.py
```

No board or servo test is performed by these host checks. Cartesian is already active; turning the
global/voice guard on is a separate owner decision and must be followed by a hardware test with the
servo signal wires disconnected first.

For the first supervised Cartesian move, start at the unchanged installed home, clear the workspace,
keep the servo power switch in reach, and begin with a single encoder click Z -5 mm (the verified
home move). The firmware home pose is not changed by this activation.
