# Voice task: states, word filter, dry-run bench test

The Auto/Voice mode task state machine (`RoboticArmMain/App/modes/mode_voice.c`) and how to test it on the
board before the arm exists. Words come from `App/voice/` as `EVT_VOICE_WORD`; the Pi link
([`inputs-and-pi-interface.md`](inputs-and-pi-interface.md)) gives the box position. Voice-word timing
(threshold, refractory): [`core-config.md`](core-config.md) §9.

## 1. States and the words each one accepts

Each word is spoken on its own. **`stop` is accepted in every state.** Any other word not listed for the
current state is ignored (section 2). Nothing moves without a `yes`, and waiting states have no timeout:
they wait for a valid word or `stop` for as long as the user needs (decided 25 Oct 2026).

| State | Meaning | Words accepted → result | `stop` |
|---|---|---|---|
| S0 IDLE | waiting for a start word | `move` / `pick` / `grab` → S1; `home` → S11 | nothing |
| S1 WHICH BOX? | waiting for a box word | `blue box` / `green box` / `red box` / `black box` → `FIND` to the Pi. While the reply is pending only `stop`. `OBJ` → S2; `NOTFOUND` → stay in S1 (say another box); no reply in 3 s → S0 | → S0 |
| S2 CONFIRM BOX | shows x y z | `yes` → S3 (the position is fixed from here); `no` → S1 | → S0 |
| S3 GRASPING | moving: open, above, down, close, up | only `stop` | → S10 |
| S4 HOLDING | object in the gripper | `hand` / `station` / `base` / `place` / `drop` → S7; `forward` / `backward` / `left` / `right` → S6 (first item queued); `home` → `RELEASE FIRST` | nothing |
| S6 JOG QUEUE | building the queue (max 10 items, each keeps the slider step) | jog words → add; `undo` → remove the last; `yes` → S8 RUN QUEUE; `no` → clear, S4 | clear, S4 |
| S7 CONFIRM PLACE | shows the destination | `yes` → S8 PLACING (for `hand` / `station`: first `FIND hand` / `FIND station`, waits with only `stop`; reply → S8, `NOTFOUND` → asks again, Pi timeout → S4); `no` → back to S4 or S10 (where it came from) | back to S4 or S10 |
| S8 RUN QUEUE | moving through the queue | only `stop` | → S10 |
| S8 PLACING | moving: above, down, open, up (hand: opens `HAND_RELEASE_MM` above the palm, never at it; station: opens `STATION_RELEASE_MM` above the drop spot on its top) | only `stop` | → S10 |
| S9 GOING HOME | moving (from J2 < 90 deg: shoulder up to 90 first, then home; done only when every joint is at home) | only `stop` | → S10 |
| S10 STOPPED | holds still; a closed gripper stays closed | `place` / `drop` → S7; `home` → S11 (opens the gripper first) | nothing |
| S11 CONFIRM HOME | "go home?" | `yes` → S9; `no` → back to S0 or S10 | back to S0 or S10 |

Changes from the system guide §11.3: S5 is merged into S7 (S7 shows the destination); S11 is new, so a
misheard `home` (e.g. "foam") can never move the arm or open the gripper without `yes`; `place` from S10
goes through S7. The arm never releases a held object on its own. The gripper state follows the gripper
step when it is commanded, so a `stop` right after "open" in a place script knows the gripper is empty.

`hand` (26 Oct 2026) has no fixed pose: the hand moves, so `yes` in S7 asks the Pi (`FIND hand`) for the palm
centre at that moment. The answer is read once and kept for the whole place script (the arm does not follow a
hand that moves during the place). The gripper opens `HAND_RELEASE_MM` (30 mm placeholder, team decision) above
the palm and never goes down to it.

Other events: a fatal fault stops a moving state (→ S10) and cancel a Pi
request; a mode change holds the motion and cancels the task (back in voice mode: S0, or S4 if holding).
Touch: STOP = `stop`, UNDO = `undo` in S6, the slider sets the jog step.

## 2. Word filter

Every main-loop pass `mode_voice` tells `voice/` which words the current state accepts
(`voice_set_accept_mask()`, one bit per class id). A detected word outside that set:

- is logged once as `--- yes 78% ignored (not wanted now)`,
- posts no event and does not light LD2,
- **does not start the 1.5 s refractory**, so a stray word cannot block the real word that follows.

Outside voice mode every word passes, as before. `mode_voice` still checks each word itself, so the
filter is a second line of defence.

## 3. Dry-run bench test (before the arm exists)

Two flags, only ever in `RoboticArmMain/App/common/app_config_local.h` (gitignored):

```c
#pragma once
#define APP_VOICE_DRY_RUN  1   /* each move step is logged and counts as done after 1 s; no ik, nothing moves */
#define APP_VOICE_FAKE_PI  0   /* 1 = a box word gets an instant fake OBJ 182 -45 30 (no pi needed) */
```

- `APP_VOICE_DRY_RUN`: script steps print `dry-run: step 2/5 move to x y z mm` instead of calling IK and
  motion, and finish after `APP_VOICE_DRY_RUN_STEP_MS` (1000 ms, time to say `stop`). Uncalibrated
  destinations use their placeholder pose `0 0 0`.
- `APP_VOICE_FAKE_PI` needs `APP_VOICE_DRY_RUN` (the build refuses otherwise), so a made-up position can
  never move the real arm.
- Boot line: `voice-mode: DRY RUN (moves are logged, nothing moves)` (`, FAKE PI` when on).

Setup: mics on SAI1 as usual; Pi wired as in [`input-and-pilink-test.md`](input-and-pilink-test.md) §2
(3 wires, Pi on its own adapter, `pi_bridge` running); ST-LINK USB to the PC; Tera Term 921600 8N1;
**servo signal wires disconnected** (motion still sends the home pulse). Say each word on its own,
about 2 s apart.

## 4. Results (25 Oct 2026, board, real Pi + D435)

Branch `core/voice-task` on `f799ee2`, local flags `APP_VOICE_DRY_RUN 1`, `APP_VOICE_FAKE_PI 0`; Pi side
`pi_bridge.py` with camera-frame mm (no `calib_T.npy` yet, so z is camera depth, not height).

| Checked | Result |
|---|---|
| Pi link | `OBJ` for blue, red and green boxes (e.g. `pi: OBJ green_box 3 -77 310 mm, rtt 6 ms`); `NOTFOUND` when the box was removed, state stayed S1; round trip 6–11 ms; no bad lines |
| Full flow | S0 → S1 → S2 → S3 (5 steps) → S4 → S6 (3 jog words) → S8 RUN QUEUE → S4 → S7 (`station`) → S8 PLACING → S9 → S0 |
| Jog queue | `backward`, `forward`, `right` gave −20 x, +20 x, −20 y (20 mm step) from the held pose, in the order spoken |
| Filter | `no` in S0 and `backward` in S1 ignored, and the next word was still heard; `home` in S4 heard, state stayed S4 |
| Stop / S10 / S11 | `stop` during PLACING → S10; `drop` → S7 → `yes` → place → S9 → S0; `home` in S0 → S11 CONFIRM HOME |
| Recognition | every accepted word 75–99 %, SNR 15–28 dB, no wrong word in either run |

Not run on the board (pure state logic; the Pi side was the point of this test): S11 `yes`/`no`,
`undo`, `no` in S2, `stop` during a grasp, the Pi timeout (measured in PR #8: ≈ 3009 ms). The host tests
cover `home` through S11 from S0 and S10, `stop` while moving, the Pi timeout and (dry-run build) `hand` with
the live palm position: FIND hand, NOTFOUND, timeout, stop, release height; full host tests for
every state are still to do. The two fixes
made after the run (gripper state per step, log order) were build-checked only.

## 5. IK and the LCD (28 Oct 2026)

- **IK on for voice**: `kin_ik_voice()`, own guard `ARM_VOICE_IK_ENABLED 1` (`arm_geometry.h`, `kinematics.md`).
  A real `yes` in S2 now grasps. Grasp / place search the pitch 180 -> 110; every pose is solved before step 1,
  with a raise step (wrist / shoulder up first) where the J2/J5 coupling needs it, and a 30 mm approach when
  50 mm does not fit (`kinematics.md`). After closing, the box is lifted 20 mm higher than the approach
  (`HOLD_EXTRA_MM`, 28 Oct 2026: carried clear of the table and station); the station release is 25 mm
  (held block centre above the top; 12 let the fingers bump the station);
  a box no pitch reaches (e.g. 182 -45 30) shows the IK status and stays in S2, nothing moves. A target motion
  would clamp is a failed step (`MOVE FAILED` -> S10). The grasp point is the box top minus `GRASP_BELOW_TOP_MM`.
  Camera positions come from `calib_T.npy` ([`camera-calibration.md`](camera-calibration.md)); J1 is ~14 deg
  short to the right (not fixed): keep boxes in the middle / left (Y > -100).
- **LCD**: tap the Voice card. The VOICE page shows "Pick Color" = the colour said in S1, a tick on each colour
  the Pi found (`OBJ` sets it, `NOTFOUND` clears it), and a pop-up:

  | State | Pop-up |
  |---|---|
  | S3 grasping, S8 jog queue / placing | PROCESSING |
  | S9 going home after a place | COMPLETE |
  | S9 going home (asked with `home`) | PROCESSING |
  | S10 stopped | STOPPED holding / STOPPED empty |
  | waiting states (S0, S1, S2, S4, S6, S7, S11) | none |

  Right of the Back button (where Proceed was) two small lines: the task state and its detail in white (e.g.
  `S2 CONFIRM BOX  260 20 15 mm`: check the position before saying `yes`), and the latest message in yellow
  for a few seconds (`OUT OF REACH`, `JOINT LIMIT`, `BOX NOT FOUND`, `PI NOT RESPONDING` ...).
  Top right of the voice page (in the header): the **jog step slider**, 0.00-2.00 cm in 0.05 cm steps (default
  2.00 cm = the old 20 mm). Tap or drag; the next jog words (`forward` ...) use it (`EVT_TOUCH_STEP_CM`, 0.1 mm).
  Right of the slider: **FINE / FAST** (white / yellow), the speed every voice move uses (joystick press toggles).
  In S6 the **jog queue pop-up** covers the camera panel only: `[forward 1.23, right 1.11]` (cm, in the order
  spoken) with `say YES / UNDO / NO`. It is not an overlay, so the slider and Back keep working under it.
  A tap hides the pop-up (the task goes on; use `stop` to stop). The old touch flow (Proceed -> confirm ->
  DESTINATION / CUSTOM pages) is off (`VOICE_TOUCH_FLOW 0` in `lcd.c`): words drive the task.

## 6. Still open

- ~~`base` pose~~ set 28 Oct 2026: X 220, Y 120, opens at Z 12 (held block centre). "place here" still lowers a
  fixed 50 mm instead of to the table.
  `station` is no longer a fixed pose (27 Oct 2026): `yes` sends `FIND station` and places at the Pi's drop spot
  ([`station-detection.md`](station-detection.md)). `STATION_RELEASE_MM` = 12 mm (held block centre above the
  station top: half the 15 mm block + ~4.5 mm, 28 Oct 2026).
- ~~Jog directions~~ 28 Oct 2026: forward = +X, left = +Y, for someone behind the arm. Queue travel limit
  (300 mm placeholder) still to check.
- ~~Camera → robot-base calibration~~ done 28 Oct 2026 (`camera-calibration.md`, RMS 8.9 mm; J1 short to the right).
- `HAND_RELEASE_MM` (how far above the palm the gripper opens) and a safety-owner OK before the arm ever moves
  towards a person's hand. There is no hardware e-stop planned.
