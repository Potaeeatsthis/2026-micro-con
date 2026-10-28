# Station detection (Pi vision)

How the Pi finds the station, and the blocks lying on it, with the D435. Code:
[`tools/pi/pi_vision_uart.py`](../tools/pi/pi_vision_uart.py) (`BlockDetector`), a copy of `~/arm/pi_vision_uart.py`
on the Pi; tests: [`tools/pi/test_station_sim.py`](../tools/pi/test_station_sim.py). Link to the STM32:
[`inputs-and-pi-interface.md`](inputs-and-pi-interface.md).

**Status (27 Oct 2026):** built and passing every test **in simulation**. The first real snapshot did **not**
find the station: 47% of the depth was missing (laser already ON: 74% holes off, 67% on), and the top came back as
5 separate pieces with a 4-5 mm spread. Version 2 (section 3, steps 2a-2c) joins the pieces and handles a hidden
end; it passes a simulated copy of that snapshot. **Real camera with the tray, 27 Oct, after raising the camera and tilting it
more toward the work area (nothing else changed): station found** (conf 78, top measured 82 mm, drop spot between
the two blocks), and both blocks on it found with surface "station" (red conf 55 with built-in colours). The low,
shallow camera position was the cause of the missing depth. The STM32 side is done (section 7); the round trip
on the board is not tested yet.

**Real camera, kraft box (27 Oct):** found, conf 83, top 45 mm (exact), size 191 x 95 mm (box specified as
200 x 100: the top's real size is to be measured, 9 mm short is close to the 10 mm trust limit), yaw -2 deg, 23%
of the depth missing (tray: 47%). All 4 blocks on it found with the right colour after teaching the station
colours (blue 80, red 70, black 63, green 50; red and green tops had no depth, positions estimated) and the drop
spot correctly reported FULL. With 3 blocks on it the drop "+" was drawn on the free end (conf 89). **Open:** in the
same layout one snapshot found it and the next did not: the measured length sits at the 10 mm trust limit. Measure
the real top and set it as `STATION_SIZE_MM` (the bridge's 1 s hold hides single-frame misses, snapshots do not).
Later the same day (`pi_bridge.py --snapshot`, blocks + hand): station found, conf 82, **199 x 96 mm** (no longer at
the limit), 2 blocks on it, drop spot, green on the table and the hand (c78, 26 of 26 frames) all at once.

**Known limit: a hand (or arm part) within about 6 cm of the station, at about its height, hides it.** The depth
search joins the two into one shape that is too big, so the station is NOT found; it is never reported in the wrong
place (test `hand_beside_station`). A rebuild that tried to leave the hand out was tried and dropped: with patchy
depth it placed the station 80-100 mm off. Ask `FIND station` with hands and the arm away from it.

## 1. The station

- **Kraft cardboard box, 200 mm long x 100 mm wide x 45 mm tall** (default since 27 Oct; `--station-size L W H`
  for another one). Matte, finely textured top (good for depth), flat to the edge, brown (good contrast with blue,
  green and black blocks; teach `red@station`). Place the 200 mm side left-right: the camera sees widest that way.
  Put some weight inside so it does not slide. The first station was a pale grey-blue plastic tray, 210 x 160 x 75 mm:
  it worked, but the blue block blended in and the camera had to be held high to see all of it.
- A good station: light or neutral matte top (not blue / green / red / black), 45-100 mm tall (clearly taller than
  the 15-20 mm blocks), big enough for a few drop spots, flat to the edge, and not palm-sized (about 90 x 70 mm).
- Two uses: the arm **drops** blocks on top of it, or **picks** blocks lying on it.
- Blocks: clay, about 25 mm wide, 15-20 mm tall, red / green / blue / black.
- Camera: about 25 cm high, tilted 45 deg down, 30-45 cm from the work area. USB 2: depth 480x270 aligned onto
  colour 424x240, so detection runs at 424x240.

## 2. Why depth, not colour

A photo test (26 Oct) showed the station is hard by colour: almost grey (saturation about 75), the **same hue as
the blue block**, and a fixed colour range lost it when the lighting changed. By depth it is easy: a big flat
top clearly above the table (45 mm for the box), with a known length and width. Blocks are 15-20 mm tall and a hand is neither flat nor
rectangular, so nothing else in the scene looks like that. No colour, no ML, no ArUco marker (markers stay the
fallback if the real depth is too poor).

## 3. How it works

1. **Table plane.** Unchanged RANSAC fit, plus one safety check: if a plane parallel to the biggest one lies one
   station height (+-30%) **farther** from the camera and holds at least 10% of the points, the biggest plane was
   the station top, and the lower one is used as the table.
2. **Station.** Pixels between 0.8 x H and H + 12 mm above the table (H = station height), opened, split into
   groups (at least 300 px). For each group:
   keep the pixels within 8 mm of its median height, reject it if that top is not flat (robust spread > 6 mm),
   fit a rectangle on the table plane (in mm) and accept it if length and width are within 25% of the station's size and
   the shape fills at least 75% of the rectangle. A group touching the image edge is rejected: the station is
   partly out of view and its centre would be wrong.
   - 2a. **Pieces.** Depth holes split a plain top into pieces, so pieces up to 35 px apart are joined into one
     group (only to decide what belongs together; only real top pixels are measured).
   - 2b. **Edges by percentiles.** The rectangle's direction comes from the outline's convex hull, but its edges
     come from the 0.5 / 99.5 percentiles of all top points: the hull's extreme points follow depth noise outwards.
   - 2c. **Hidden end.** A side more than 10 mm too short would give a wrong centre. If at least 60% of it is seen,
     one end is clearly a real edge (12 mm past it the table or the station's side shows) and the other end is
     clearly a depth hole, that side is rebuilt from the real edge with the known length (confidence x 0.85).
     Only one side may be rebuilt, and the other must then fit within 10 mm. Otherwise the station is **not
     reported** ("partly hidden by depth holes ... its centre would be wrong"), never reported in the wrong place.
3. **From the rectangle:** top centre, 4 top corners, top height, yaw, a footprint mask (top + base corners) and an
   inner mask (top, 8 mm inside the edge).
4. **Blocks on two surfaces.** The table search is the old one, skipping the footprint (station pixels next to a
   block count neither as table nor as "something taller"). The station search runs the same block finder with
   the station top as the "table": heights above the top, only inside the inner mask, colours relative to the
   station top's own median colour. Each block has `surface` = `"table"` or `"station"`.
5. **Colours per surface.** Colour is judged against the surface under the block, so the station has its own
   samples: `--learn-color` with the block **on the station** saves `red@station` in `color_refs.json`. Without
   them the station uses the built-in references (made for a pale surface).
6. **Drop spot.** The centre of the top if it is at least 45 mm from every block on the station, else the nearest
   point of a 5 x 3 grid 35 mm inside the edge that is; `None` if the station is full.

Output: `det.station` = `{centre_cam_m, corners_cam_m, base_corners_cam_m, size_mm, top_mm, yaw, drop_cam_m,
drop_px, n_blocks, conf, flat_mm, top_px}` or `None`. `--snapshot` draws the outline (magenta) and the drop "+",
and prints a `station:` line and why each rejected part was not the station. `--no-station` turns it all off.

## 4. Link to the STM32 (Pi side done)

`pi_bridge.py` answers `FIND station` with `OBJ station x y z` = the **drop spot** on the top surface, or
`NOTFOUND station` (no station, too old, confidence below `--min-conf`, or no free spot). The STM32 adds the
release height itself, as for `hand`. Blocks lying on the station are answered by the normal `FIND red_box`
(their z is the station top + block height). The status line ends with `station c.. drop x y z (n blocks)`.

**Station age (28 Oct 2026, board test):** while the arm holds a block above the table to place it, the arm
hides the station from the camera, and with the 1 s age every `FIND station` was a `NOTFOUND` although the
station had been seen at 90 %+ a moment before. The station is now answered from the last good sighting up
to `--station-max-age` seconds (default 30); blocks keep the 1 s `--max-age`. Limit: a block placed while the
station was hidden is only taken into account for the next drop spot once the camera sees the top again.

`best_per_color` still keeps **one block per colour**: with a red block on the table and one on the station,
`FIND red_box` answers the more confident one. Changing that needs a protocol decision (e.g. `red_box@station`).

## 5. Test results (simulation, 27 Oct 2026)

`python test_station_sim.py` renders a table, the station and blocks as the camera would see them (424x240,
depth noise, about 11% depth holes, black blocks with no depth) and checks. The rows down to "Speed" were
measured with the tray (210 x 160 x 75); the rows after it are marked.

| Test | Result |
|---|---|
| Station at yaw +15 and -35 deg | found; size within 3 mm, top within 0.1 mm, centre within 1 mm |
| 3 blocks on the station (incl. black, no depth) + 3 on the table | all found, right colour and surface, 2-7 mm error |
| Empty station / block in the middle | drop = centre / drop moves to 54 mm from the nearest block |
| Camera close, station = 30% of the pixels | table plane is still the real table |
| Bumpy depth, hand-sized box, arm-like bar at station height | no false station |
| Table-only scenes in 4 lightings | identical blocks with and without the station code |
| Patchy depth like the real snapshot (56% missing in blobs, 4 mm noise), 3 scenes | found; centre within 2 mm |
| 20 random poses, normal depth / patchy depth | found 20/20 / 18/20 (the 2 misses rejected, not misplaced); centre max 2.1 / 5.8 mm |
| Same poses with a hand-sized box + arm bar instead of the station | no false station (0/40) |
| Speed (this PC) | 12.7 ms per frame without station, 23.2 ms with |
| **Box 200 x 100 x 45**, 60 random poses each: normal / patchy depth | found 60/60 / 58/60; centre max 1.5 / 5.0 mm; 0/120 false stations |
| Tray 210 x 160 x 75 (`--station-size`), same 60 poses | found 60/60 / 60/60; centre max 1.9 / 4.7 mm; 0/120 false stations |
| Box full (3 blocks along the 200 mm top) | all 3 found; no drop spot (FULL), as expected: 45 mm clearance |

The tests follow the detector's station size (`STATION_SIZE_MM`), and the hand / arm-bar false-station checks are
made at the station's own height. On the 100 mm-wide box about **3 blocks** fit in a row before it is FULL.

With patchy depth the **block** positions (unchanged block code) are 4-14 mm off instead of 2-7 mm: the noisier
depth costs accuracy there too.

Not covered: the team's own earlier tests (lighting suite, bumpy depth, hand tests) were not available here; the
simulation has equivalents. Built-in colours give low confidence for the blue block on the station (45) and a
black block on the table (39-52): teach the colours on the real camera.

## 6. Placing the camera

The detector uses the colour camera's view, about 69 deg wide x 42 deg tall, so place the station's long side
left-right. A camera 25 cm high tilted 45 deg sees the box's 45 mm-high top from about 9 to 46 cm in front of it on
the table (the tray's 75 mm top: 8 to 39 cm). A station touching the image edge is rejected as "partly out of view".
On the real camera a low, shallow camera also gave poor depth up close (~50% missing); raising it and tilting it
more toward the work area fixed that (27 Oct). The final camera needs a fixed mount anyway (`calib_T.npy` is only
valid while it does not move): measure the height and angle that work and build the mount for them.
`tools/pi/ir_check.py` saves the infrared view with the projector off and on, to check that the projector works
when the depth looks poor (service stopped, `python ir_check.py`).

## 7. STM32 side (done 27 Oct 2026, with Supanut)

| File | Change |
|---|---|
| `App/common/app_types.h` | `OBJ_STATION` after `OBJ_HAND` (not a box, outside `OBJ_BOX_COUNT`; `OBJ_COUNT` is now 6) |
| `App/pi_link/pi_proto.c` | name `"station"` in `OBJ_NAMES` |
| `App/modes/mode_voice.c` | `station` + `yes` sends `FIND station` (like `hand`) instead of the fixed `DEST_STATION` pose; the reply's drop spot is placed with the gripper opening `STATION_RELEASE_MM` (25 mm since 28 Oct 2026: held block centre above the top; 12 bumped the station) above it; `NOTFOUND` shows `STATION NOT FOUND` and asks again |
| `ProjectDevGuide/tests/host/` | `OBJ station` / `NOTFOUND station` parsing; dry run walks `station` end to end (FIND, NOTFOUND, a hand reply ignored, place above the drop spot, never down to it) |

Bench check without voice: `APP_INPUT_TEST 1` + `APP_INPUT_TEST_CYCLE 1` in `app_config_local.h` now also sends
`FIND station` (every object in turn).

## 8. Still open

- Run version 2 on the real camera. Why is ~50% of the depth missing with the laser on? Worth trying: camera
  higher / farther, less light on the scene, `--laser 150` (the dots may glare up close).
- Speed on the Pi 4 (the bridge status line shows fps).
- Partly visible station: today it is rejected, not guessed.
- Drop-spot clearance (45 mm) checked against the real gripper.
- Camera -> robot calibration (`calib_T.npy`): until then all positions are camera-frame mm.

## 9. Rules for changing the detector

- Setup: Pi 4 + RealSense D435, Python 3.12. `tools/pi/pi_vision_uart.py` is the repo copy of
  `~/arm/pi_vision_uart.py` on the Pi: change both together.
- Do not change how blocks on the **table** are detected unless a test shows it is needed.
- It must keep working on USB 2 (depth 480x270) and on the Pi 4 CPU.
- Test in simulation first (`tools/pi/test_station_sim.py`), then on the real camera.
- Do not try the "grow / leave the hand out" rebuild again (section 1, known limit): with patchy depth it placed
  the station 80-100 mm off. A missed station (NOTFOUND) is better than a wrong centre.
- `scp` needs the destination as its **last** argument: a missing one once overwrote `color_refs.json`.
- STM32 files outside `App/modes/` and `App/pi_link/` (e.g. `App/common/`) belong to the core owner: ask first
  (`AGENTS.md`). `OBJ_STATION` was agreed with Supanut on 27 Oct 2026.
- When a decision or test result changes, update this file (it is the task's notes; there is no other copy).
