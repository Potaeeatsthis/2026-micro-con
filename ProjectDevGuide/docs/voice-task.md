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
| S9 GOING HOME | moving | only `stop` | → S10 |
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

## 5. Still open

- IK/FK (`TODO(ik)`): until it exists, a real (non-dry) `yes` in S2 shows `IK NOT READY` and stays in S2.
- `base` pose (`DEST_CALIBRATED 0`), and "place here" lowers a fixed 50 mm instead of to the table.
  `station` is no longer a fixed pose (27 Oct 2026): `yes` sends `FIND station` and places at the Pi's drop spot
  ([`station-detection.md`](station-detection.md)). `STATION_RELEASE_MM` (20 mm placeholder: the held block's
  height under the gripper + clearance) is to be measured on the real gripper.
- Jog directions in the robot-base frame and the queue travel limit (300 mm placeholder).
- Camera → robot-base calibration (`calib_T.npy`) before any real move.
- `HAND_RELEASE_MM` (how far above the palm the gripper opens) and a safety-owner OK before the arm ever moves
  towards a person's hand. There is no hardware e-stop planned.
