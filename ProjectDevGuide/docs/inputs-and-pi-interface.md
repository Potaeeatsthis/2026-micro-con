# Inputs and Pi link: who sends what

One page for the whole team: what each input component gives the STM32, in which form, and how your
module reads it. Pins: [`core-config.md`](core-config.md) §3. How modules talk (events / getters / motion API):
[`TEAM_GUIDE.md`](TEAM_GUIDE.md) §3. Measured numbers and the bench test: [`input-and-pilink-test.md`](input-and-pilink-test.md).

```text
 KY-023 joystick ──ADC (PA3, PC0)──┐
 KY-040 encoder ──TIM1 (PE9, PE11)─┤                       ┌─ events ─> modes (the only event consumer)
 buttons ENC/JOY ──EXTI────────────┴──> input/  ───────────┤
                                                           └─ getters: input_joy_x/y(), input_joy_centered() ...
 D435 ──USB──> Pi (detect + depth + transform) ──USART6 text──> pi_link/ ──> events EVT_PI_*
```

Nothing in `input/` or `pi_link/` moves the arm. The active mode reads these values and calls the motion API.

## 1. What each component sends

| Component | Raw signal | What your code gets | Units / range | Measured |
|---|---|---|---|---|
| **KY-023 joystick X / Y** | two pots, ADC 1 kHz | `input_joy_x()`, `input_joy_y()` (getters, read every loop) | float −1…+1, 0 inside the 8 % dead zone; **+ = right / up** (away from the operator) | raw 0–4095, centre ≈ 2072 / 2066, noise 10–28 counts |
| | | `input_joy_centered()` | bool | |
| | | `input_raw_x()`, `input_raw_y()` (debug / calibration screen) | 0–4095 counts | |
| **Joystick button** (SW) | EXTI, debounced 20–40 ms | `EVT_JOY_PRESS`, **only when the stick is centred** | one event per press | bounces ~4 edges, debounce absorbs it |
| **KY-040 encoder** | quadrature, TIM1 | `EVT_ENC_STEP`, `v[0]` = detents turned | int, **+ = clockwise** | 4 counts per detent; very fast spins can lose half a detent |
| **Encoder button** | EXTI, debounced | `EVT_ENC_PRESS` | one event per press | clean |
| **LCD mode card** (touch) | `lcd.c` touch zone | `EVT_TOUCH_MODE_TAB`, `arg` = `arm_mode_t` | one event per tap | replaces the MODE button (removed 27 Oct 2026) |
| **Depth camera via Pi** | text line on USART6 | `EVT_PI_OBJECT` / `EVT_PI_NOTFOUND` / `EVT_PI_TIMEOUT` (see §2) | mm | round trip 4–14 ms |

Only presses become events (releases are tracked but not posted). The arm has no MODE button,
Hold-to-Enable or e-stop, no limit switches and no servo-rail sense ([`core-config.md`](core-config.md) v1.4);
those pins are no longer in the `.ioc`.

## 2. Depth camera → Pi → STM32

**The STM32 asks, the Pi answers.** The Pi never sends anything on its own.

| Step | Who | What |
|---|---|---|
| 1 | STM32 (a mode) | `pi_link_find(OBJ_RED_BOX)` → sends `FIND red_box\n` |
| 2 | Pi (`pi_bridge.py`) | looks up the latest camera detection of that colour (camera runs continuously, 15 fps) |
| 3a | Pi | seen within the last 1 s with confidence ≥ 40 → `OBJ red_box 182 -45 30\n` |
| 3b | Pi | not seen / unclear → `NOTFOUND red_box\n` |
| 4 | STM32 (`pi_link`) | parses the line, posts one event (below) |

| Event | `arg` | `v[0..2]` | Meaning |
|---|---|---|---|
| `EVT_PI_OBJECT` | `object_id_t` (colour) | x, y, z in mm | block found |
| `EVT_PI_NOTFOUND` | `object_id_t` | – | Pi answered: not visible |
| `EVT_PI_TIMEOUT` | – | – | no answer within `APP_PI_TIMEOUT_MS` (3000 ms now): Pi not running or wires off |

What the numbers mean:

- **Colour** = the name: `red_box`, `green_box`, `blue_box`, `black_box` (`OBJ_RED_BOX` … in `object_id_t`,
  `OBJ_BOX_COUNT` of them). `hand` (`OBJ_HAND`) comes after the boxes, see below.
- **x y z** = integer mm, **robot-base frame**: right-handed, +X forward, +Y left, +Z up. The intended
  origin is on the base axis at the floor, but the floor-to-base-bottom (stand) offset is not measured,
  so the floor-origin transform is not confirmed. The CAD conversion below maps to the **base-bottom
  (mounting/table) plane**, with `z_B = 0` there; floor height is `z_floor = z_B + s` for the unknown
  stand offset `s`.
  CAD **direction vectors** map as `x = -X_cad`, `y = Z_cad`, `z = Y_cad` (CAD is Y-up). For **points**,
  use `x = -X_cad`, `y = Z_cad - 18.18`, `z_B = Y_cad + 30.5` mm, i.e. `p_B = R * (p_cad - o)` with
  `R * (X,Y,Z) = (-X,Z,Y)` and `o = (0, -30.5, 18.18)` mm in CAD coordinates. This is the
  **base-bottom-plane origin**, not a floor origin; do not claim a measured floor transform without
  measuring the stand. The workbook records measured floor-to-J2 = `94 mm` and CAD base-bottom-to-J2 =
  `90.22 mm` (`Real Arm Check` C6 is `CHECK`, 3.78 mm > 2 mm tolerance); the 3.78 mm difference is not
  an independently measured stand thickness. The reported point is the **centre of the block's top face**.
- At the straight-up zero pose the positive joint axes (right-hand rule) are **J1 +Z** base yaw,
  **J2 +Y** shoulder pitch, **J3 +Y** elbow pitch, **J4 +Z** wrist roll, **J5 +Y** wrist pitch.
  J6 is the gripper and is not part of positioning IK. These are chosen model signs, not measured servo
  or PWM directions; the definitions live in `App/kinematics/kinematics.h` and `arm_geometry.h`.
- The Pi does detection, depth and the camera → base transform. **The STM32 does IK and the grasp offset**
  (where to close the gripper below the top face).
- One request at a time. A reply for a request that is no longer pending is ignored and logged.
  `pi_link_cancel()` forgets the pending request (mode change, stop).
- Bad lines never become events; they are logged with a reason (`pi: bad line (...)`).

**Hand.** The bridge also answers `FIND hand`; voice mode sends it after `hand` + `yes` (`voice-task.md`):

| STM32 sends | Pi answers | Meaning |
|---|---|---|
| `FIND hand` | `OBJ hand 210 15 96` | palm centre seen within the last 1 s |
| | `NOTFOUND hand` | no hand, palm depth unusable, or hand detection off |

- The point is the **palm centre on the palm surface** the camera sees: the mean of the wrist and the four finger
  bases, each with its own depth. Same frame and units as the blocks (camera-frame mm until `calib_T.npy` exists).
- A hand **moves**: the answer is where it was at that moment. Ask again just before using it.
- One hand only (the first MediaPipe finds). Detection runs up to 5 times a second (≈ 120 ms per run on the Pi 4).
- MediaPipe runs in a **separate worker process** with its own Python (`~/hand-env`), so a crash there can never
  stop the block answers. The newest MediaPipe (1.1.1) dies with `Illegal instruction` on the Pi 4; use 0.10.18.
- STM32 side: `OBJ_HAND` in `object_id_t` (after the boxes; `OBJ_BOX_COUNT` = boxes only) and the name `"hand"`
  in `pi_proto.c`. Code that means "boxes" loops over `OBJ_BOX_COUNT`, not `OBJ_COUNT`.
- `APP_INPUT_TEST_CYCLE` (bench) cycles over `OBJ_COUNT`, so it now also sends `FIND hand`.

**Current status (2026-10-24):** colours re-taught (all four found, confidence 70–87). The camera → base
calibration (`calib_T.npy`) is not done yet, so the bridge runs with `--camera-frame` and the x y z are
**camera-frame** mm. As soon as `calib_T.npy` appears in `~/arm`, the running bridge switches to robot-base mm
by itself, with no change on the STM32. Colour detection quality belongs to the Pi / camera owner.

## 3. Running the Pi side (for the camera owner)

The bridge is meant to run all the time: install it once as a service and it starts at boot, restarts after a
crash (or when the camera disappears) and answers every `FIND`.

```sh
# once, on the Pi (pi_bridge.py and pi_bridge.service copied to ~/arm/tools/ from ProjectDevGuide/tools/pi/)
sudo cp ~/arm/tools/pi_bridge.service /etc/systemd/system/
sudo systemctl daemon-reload
sudo systemctl enable --now pi_bridge
```

| I want to | Command |
|---|---|
| see what it does, live | `journalctl -u pi_bridge -f` |
| run `--snapshot`, `--learn-color`, `--print-cam` (they need the camera) | `sudo systemctl stop pi_bridge`, run it, then `sudo systemctl start pi_bridge` |
| use a new calibration | `--solve-calib points.csv` as usual: the bridge reloads `calib_T.npy` on the next `FIND` |
| use re-taught colours | nothing: `color_refs.json` is reloaded within 2 s (re-teaching itself needs the service stopped, see above) |
| change `pi_vision_uart.py` or `pi_bridge.py` | `sudo systemctl restart pi_bridge` |
| run it by hand instead | `~/arm-env/bin/python ~/arm/tools/pi_bridge.py --camera-frame` (service stopped) |
| check the hand detection | service stopped, `... pi_bridge.py --hand-snapshot`: writes `~/arm/hand.jpg` (landmarks, palm points, XYZ) |
| see everything the bridge detects | service stopped, `... pi_bridge.py --snapshot`: writes `~/arm/snapshot.jpg` (blocks + hand on the colour image, depth beside it) and prints each position |
| run without hand detection | add `--no-hand` (FIND hand gets NOTFOUND, blocks unchanged) |
| run without station detection | add `--no-station` (FIND station gets NOTFOUND, blocks unchanged) |
| set up hand detection (once) | `~/.local/bin/uv venv ~/hand-env --python 3.12` then `~/.local/bin/uv pip install --python ~/hand-env/bin/python "mediapipe==0.10.18"`; `~/arm-env` is not touched |

What the log shows (`journalctl -u pi_bridge -f`):

```text
calib: now sending CAMERA-frame mm (no calib_T.npy, --camera-frame)
Colours: red learned x2, green learned x3, blue learned x3, black learned x2
open /dev/serial0 115200 8N1, answering FIND. Ctrl+C to stop.
status: frame CAMERA (no calib_T.npy, --camera-frame) | 13.9 fps | blue c74 95 4 288 0.1s | green c76 103 -102 419 0.1s | red c86 162 -74 376 0.1s | black c74 74 -56 360 0.1s | hand - (no hand) | FIND 5 OBJ 5 NOTFOUND 0 ignored 0
RX FIND red_box
TX OBJ red_box 163 -74 376 [CAMERA] (10 ms after RX)
```

- `status` every 10 s: which frame is sent, camera fps, and per colour the confidence `c`, x y z (in the frame
  being sent) and how old the detection is. `(not sent)` = too old or confidence below 40, a `FIND` would get
  `NOTFOUND`.
- `hand ...` at the end of `status`: `hand c95 x y z 0.1s` (position being sent), `hand - (no hand)` /
  `hand - (depth on 2 of 5 palm points)` (why not), or `hand off (...)` / `hand stopped (...)` (no `~/hand-env`,
  worker crashed or stuck: it is restarted every 30 s; `hand off (--no-hand)`). The
  `c` is MediaPipe's left/right confidence, usually above 90: a weak filter. The depth check decides.
- `calib: ...` whenever `calib_T.npy` appears, changes or disappears (the matrix is printed). A broken file is
  reported and not used. Once a calibration has been used, a missing or broken file never falls back to
  camera frame: every `FIND` gets `NOTFOUND` until it is fixed.
- `camera: no frame for 5 s` = camera unplugged or stuck; if the camera thread dies the bridge exits and
  systemd restarts it.
- `pi_vision_uart.py ... no longer has X` = the detector code was changed and the bridge needs updating. The
  bridge uses `BlockCamera`, `BlockDetector`, `load_refs`, `best_per_color`, `COLOR_NAMES`, `CALIB_FILE`,
  `REFS_FILE` and the detection fields `conf` and `xyz_cam_mm`.
- On the STM32 side the same exchange appears in the debug log as `pi: OBJ red_box 163 -74 376 mm, rtt 4 ms`.

The service passes `--camera-frame`. Once the team relies on calibration, remove it from
`pi_bridge.service` so a missing `calib_T.npy` stops the bridge instead of sending camera-frame mm.

## 4. Where each part lives

| Part | File | Owner |
|---|---|---|
| Joystick, encoder, buttons | `RoboticArmMain/App/input/input.c` / `.h` | input member |
| Pi link, protocol parser | `RoboticArmMain/App/pi_link/pi_link.c`, `pi_proto.c` | Pi member |
| Pi bridge (text protocol, uses the detector) + service | `ProjectDevGuide/tools/pi/pi_bridge.py`, `pi_bridge.service` → `~/arm/tools/` on the Pi | Pi member |
| Hand detection (MediaPipe worker, palm XYZ) | `ProjectDevGuide/tools/pi/pi_hand.py` → `~/arm/tools/`; MediaPipe 0.10.18 in `~/hand-env` (not in `~/arm-env`) | Pi member |
| Detector, colours, calibration | `~/arm/pi_vision_uart.py`, `color_refs.json`, `calib_T.npy` (on the Pi) | camera member |
| Event ids | `RoboticArmMain/App/common/app_events.h` | core |

## 5. Station

The bridge also answers `FIND station`:

| STM32 sends | Pi answers | Meaning |
|---|---|---|
| `FIND station` | `OBJ station -10 -50 298` | the free **drop spot** on the station top (top centre if free) |
| | `NOTFOUND station` | no station in view, partly out of view, or no free spot |

- Found by **depth only**: a flat top of the right size at the station's height above the table (default: the
  200 x 100 x 45 mm kraft box; `--station-size L W H` for another). Blocks lying on it are answered by the normal
  `FIND <colour>_box`.
- The STM32 adds the release height, as for `hand`. Same frame and units as the blocks.
- `--snapshot` (pi_vision_uart and pi_bridge) draws the station and its drop "+"; `--no-station` turns it off.
- Teach block colours **on the station** too (`--learn-color` with the block on it saves `red@station`).
- STM32 side (27 Oct 2026): `OBJ_STATION` in `object_id_t` (after `OBJ_HAND`, outside `OBJ_BOX_COUNT`), the name
  `"station"` in `pi_proto.c`; voice mode sends `FIND station` after `station` + `yes`.
- Tested in simulation and on the real camera with the box (27 Oct 2026: found, 4 blocks on it with the right
  colour, drop spot shown). The full STM32 -> Pi `FIND station` round trip is not tested on the board yet.
  How it works and test results: [`station-detection.md`](station-detection.md).
