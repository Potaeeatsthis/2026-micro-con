# Inputs and Pi link: who sends what

One page for the whole team: what each input component gives the STM32, in which form, and how your
module reads it. Pins: [`core-config.md`](core-config.md) §3. How modules talk (events / getters / motion API):
[`TEAM_GUIDE.md`](TEAM_GUIDE.md) §3. Measured numbers and the bench test: [`input-and-pilink-test.md`](input-and-pilink-test.md).

```text
 KY-023 joystick ──ADC (PA3, PC0)──┐
 KY-040 encoder ──TIM1 (PE9, PE11)─┤                       ┌─ events ─> modes (the only event consumer)
 buttons MODE/ENC/JOY ──EXTI───────┼──> input/  ───────────┤
 servo-rail divider ──ADC (PC3)────┘                       └─ getters: input_joy_x/y(), input_rail_mv() ...
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
| **MODE button** | EXTI, debounced | `EVT_BTN_MODE` | one event per press | |
| **Servo-rail sense** | 6 V rail divider, ADC | `input_rail_mv()` (getter); safety/ turns it into `EVT_RAIL_LOST` / `EVT_RAIL_BACK` | mV | |
| **Depth camera via Pi** | text line on USART6 | `EVT_PI_OBJECT` / `EVT_PI_NOTFOUND` / `EVT_PI_TIMEOUT` (see §2) | mm | round trip 4–14 ms |

Only presses become events (releases are tracked but not posted). Hold-to-enable and e-stop are not
here: they belong to `safety/`.

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

- **Colour** = the name: `red_box`, `green_box`, `blue_box`, `black_box` (`OBJ_RED_BOX` … in `object_id_t`).
- **x y z** = integer mm, **robot-base frame**: origin on the base axis at the mounting plane, +X forward,
  +Y left, +Z up. The point is the **centre of the block's top face**.
- The Pi does detection, depth and the camera → base transform. **The STM32 does IK and the grasp offset**
  (where to close the gripper below the top face).
- One request at a time. A reply for a request that is no longer pending is ignored and logged.
  `pi_link_cancel()` forgets the pending request (mode change, stop).
- Bad lines never become events; they are logged with a reason (`pi: bad line (...)`).

**Current status (2026-10-24):** the camera → base calibration (`calib_T.npy`) is not done yet, so the bench
bridge runs with `--camera-frame` and the x y z are **camera-frame** mm. After calibration the same bridge
sends robot-base mm, with no change on the STM32. Colour detection quality (which colours are found in
which light) belongs to the Pi / camera owner.

## 3. Where each part lives

| Part | File | Owner |
|---|---|---|
| Joystick, encoder, buttons, rail ADC | `RoboticArmMain/App/input/input.c` / `.h` | input member |
| Pi link, protocol parser | `RoboticArmMain/App/pi_link/pi_link.c`, `pi_proto.c` | Pi member |
| Pi bridge (text protocol, uses the detector) | `ProjectDevGuide/tools/pi/pi_bridge.py` → `~/arm/tools/` on the Pi | Pi member |
| Detector, colours, calibration | `~/arm/pi_vision_uart.py`, `color_refs.json`, `calib_T.npy` (on the Pi) | camera member |
| Event ids | `RoboticArmMain/App/common/app_events.h` | core |
