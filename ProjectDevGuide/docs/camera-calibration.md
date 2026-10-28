# Camera -> robot calibration (`calib_T.npy`)

How positions seen by the D435 camera become robot coordinates, how to calibrate, how to check the result and what
to do when it is wrong. Enough for a teammate to redo it alone.

| Part | Where |
|---|---|
| Measure points, solve | `ProjectDevGuide/tools/pi/pi_vision_uart.py` (`--add-point`, `--solve-calib`) -> `~/arm/` on the Pi |
| Use the result | `ProjectDevGuide/tools/pi/pi_bridge.py` (service `pi_bridge`), loads `~/arm/calib_T.npy` by itself |
| Simulation test (no camera) | `ProjectDevGuide/tools/pi/test_calib_sim.py` |
| Protocol / FIND replies | [`inputs-and-pi-interface.md`](inputs-and-pi-interface.md) |
| Cartesian mode, IK guards | [`kinematics.md`](kinematics.md) |

**Status (28 Oct 2026):** calibrated and in use. RMS 8.9 mm. A block in the middle was reported at X 273 Y 22 Z 14
and the gripper, sent there in Cartesian mode, landed exactly on it. The right side (Y < -100) is 12-15 mm off
because the base joint J1 turns about 14 deg too little (section 7). Voice cannot use the positions yet: voice IK is
still blocked (`ARM_GEOMETRY_MEASURED 0`, [`kinematics.md`](kinematics.md)); Cartesian mode can.

---

## 1. What it is

The camera measures each block's **top-face centre** in camera millimetres. `calib_T.npy` is a 4 x 4 matrix
(rotation + offset, camera metres -> robot metres) that turns that into the robot frame:

- origin on the **base rotation axis**, at the **mounting surface** (the table);
- **+X forward**, **+Y left**, **+Z up**, millimetres.

It is found from point pairs: the same block top measured by the camera **and** known in robot coordinates (Kabsch /
SVD, "eye-to-hand"). No STM32 change is needed: once the file exists, the bridge answers every `FIND` (blocks,
`hand`, `station`) in robot millimetres.

**`calib_T.npy` belongs to one camera position and one robot position. If the camera, its stand or the robot base
moves even a little, calibrate again.**

## 2. Before you start

- [ ] **Cartesian mode works** on the STM32 (joystick X/Y, knob Z, X/Y/Z shown on the LCD Cartesian page). If the LCD
      shows `ALIGN J4 88`, align J4 in a manual mode first ([`kinematics.md`](kinematics.md)).
- [ ] **Camera fixed** (tape / clamp) where it sees the whole work area. It does not need to see the robot.
- [ ] **Robot base fixed.** Nothing may move from now on.
- [ ] **Block colours taught** (table and station) and a `--snapshot` with one block finds it without "(estimated)".
- [ ] **One red or blue block** for the points. Not black: black tops have no depth, so their position is estimated.
- [ ] Pi reachable: `ssh potae@10.10.10.123` (password), project in `~/arm`, venv `~/arm-env`.

Layout used on 28 Oct 2026: camera on boxes about 36 cm high, about 45 deg down, facing the robot; the station
about 20 cm in front of the camera, robot base about 20-25 cm behind the station (arm reach about 35 cm); the
station is off to one side so it hides no blocks. The camera sees the table from about 16 to 80 cm in front of it.

## 3. Copy the code to the Pi

On the **PC**, in PowerShell (the destination is always the **last** argument of `scp`; Windows PowerShell 5.1 has
no `&&`, so run one command per line):

```text
scp C:\microconSpace\2026-micro-con\ProjectDevGuide\tools\pi\pi_vision_uart.py potae@10.10.10.123:~/arm/
scp C:\microconSpace\2026-micro-con\ProjectDevGuide\tools\pi\pi_bridge.py potae@10.10.10.123:~/arm/tools/
```

On the **Pi** (prompt `potae@potae-pi4:~/arm $`), check:

```text
python pi_vision_uart.py --help | grep -E "add-point|solve-calib"
```

`scp` runs on the PC, not inside the SSH session (on the Pi it cannot see `C:\...`).

## 4. Measure the points

### 4.1 Start fresh (Pi)

```text
sudo systemctl stop pi_bridge          # the camera can only be used by one program
cd ~/arm && rm -f points.csv calib_T.npy
```

### 4.2 One point

**On the robot, Cartesian mode, gripper open:**

1. Move with the joystick / knob to a spot at about **Z 60**. **Write down the X and Y shown on the LCD.**
2. Put the block on the table, roughly under the gripper.
3. Lower to about **Z 8** (Z 5-12 is fine): the open fingers now stand on both sides of the block.
4. **Centre the block by hand** between the fingertips: look from the side (between the fingers) and from the front
   (along the fingers). Closing the gripper is not needed and may squeeze a clay block.
5. Raise back to about Z 60 without touching the block, then **park the arm out of the camera view**.
6. Only this one block may be in view.

**On the Pi:**

```text
python pi_vision_uart.py --add-point X Y Z
```

- **X Y** = the values from the LCD.
- **Z = the height of the block's TOP**, not where the gripper was: **15** on the table, **60** on the station
  (45 mm station + 15 mm block). Use your block's real height if it is not 15 mm.

It watches the block for about 3 s (median of 40 frames) and appends one line `xc,yc,zc,xb,yb,zb` (camera mm, robot
mm) to `points.csv`:

```text
Point 2 saved: camera 120.2 -108.2 351.0 mm -> robot 248 119 15 mm (jitter 1.0 mm, 40 frames, red block on the table)
Now 2 point(s) in points.csv. Aim for 6-8 spread out, then: python pi_vision_uart.py --solve-calib points.csv
```

Check: **jitter below 5 mm**, the right surface (`on the table` / `on the station`), no `WARNING` line.

### 4.3 Where to put the points

- **At least 6 on the table**, spread **left / right** and **near / far** over the area where blocks will be used,
  about 10 cm or more apart, not in one straight line.
- **2-3 on the station top**, spread along the box, at least 3 cm from its edges (Z = 60). Different heights make the
  fit much better.
- Any X Y that the arm reaches and the camera sees is fine; type exactly what you commanded.
- The calibration is best **inside** the area the points cover. Run 1 only covered X 200-300: add points closer to
  the robot (X 120-180) next time.

```text
      far-left  o            o far-right
                      o  middle               + 2-3 points on the station
      near-left o            o near-right
```

### 4.4 Fix or delete a point (Pi)

```text
cat -n points.csv                                   # line 1 is the header, point 1 is line 2
sed -i '$d' points.csv                              # delete the last point
sed -i '3d' points.csv                              # delete line 3 (= point 2)
sed -i 's/,250.0,12.0,10.0$/,250.0,12.0,15.0/' points.csv   # fix a wrong Z without measuring again
```

If the camera or the robot moved: `rm -f points.csv calib_T.npy` and start again.

## 5. Solve (Pi)

```text
python pi_vision_uart.py --solve-calib points.csv
```

It saves `calib_T.npy` and prints:

| Line | Meaning | Good |
|---|---|---|
| `point N: robot X Y Z  error E mm` | how far each point is from the fit | all similar, < 5 mm |
| `<- much worse than the others ...` | one point placed or typed wrong | delete it (4.4), solve again |
| `RMS error` | overall accuracy | **< 5 mm** good; 5-10 usable; > 10 redo |
| `Camera at X Y Z ..., looks N deg downward, towards H deg` | where the fit puts the camera | matches a tape measure (height ~ camera height, distance ~ camera-to-base) |
| `Note: ...` | too few heights / too small an area | add station points / spread the points |

Then:

```text
cp calib_T.npy calib_T.backup.npy
sudo systemctl start pi_bridge
journalctl -u pi_bridge -n 20          # look for: calib: now sending robot-base-frame mm
```

## 6. Check the result

1. Put a red block somewhere new, park the arm out of view.
2. Read its robot position: `journalctl -u pi_bridge -f` (status line every 10 s: `red c.. X Y Z ... s`), or stop
   the bridge and run `python pi_vision_uart.py --snapshot` (positions printed in robot mm now).
3. In Cartesian mode go to that X Y at Z 60, then down to the reported Z.
4. The open fingers should be around the block, centred. With 25 x 18 mm blocks and a 40 mm opening there is about
   7 mm of margin each side, so the total error should stay below ~7 mm.

## 7. Result, 28 Oct 2026 (run 1)

| Point | Robot X Y Z (mm) | Surface | Error (mm) |
|---|---|---|---|
| 1 | 259 12 15 | table | 7.4 |
| 2 | 248 119 15 | table | 7.1 |
| 3 | 201 -156 15 | table | 12.5 |
| 4 | 288 20 15 | table | 6.1 |
| 5 | 224 -140 15 | table | 14.6 |
| 6 | 266 104 15 | table | 9.1 |
| 7 | 300 26 60 | station | 3.4 |
| 8 | 276 -33 60 | station | 4.8 |
| 9 | 291 94 60 | station | 9.6 |

- RMS **8.9 mm**. Jitter of each point 0.3-1.0 mm, so the camera side is steady.
- Camera solved at X 435, Y 17, Z 340 mm, 76 deg downward, facing back towards the robot (estimated by eye: 36 cm,
  45 deg; the camera is probably steeper than it looks).
- Check: block in the middle reported X 273 Y 22 Z 14, gripper landed exactly on it.

**The remaining error is on the arm side.** It grows to the right; no single point is bad. Ruler check of the IK
without the camera: commanded X 224 Y -140 Z 15 -> the real tip was at about X 240 Y -80, i.e. the base J1 turned
about 14 deg less than commanded (reach about 11 mm short). Because each calibration point pairs the camera with
the **commanded** position, part of that error cancels (the camera is mapped into the arm's own coordinates); the
non-rigid part stays.

**Until J1 is fixed:** use the middle, the left and the station; avoid the right side (Y < -100).

## 8. When the arm misses: camera side or arm side?

Check each side on its own:

- **Camera side:** put a block at a ruler-measured robot X Y, read the reported position (`--snapshot` or the bridge
  log). Wrong here = calibration (redo points, more spread, fix Z values).
- **Arm side:** send the gripper to X Y Z in Cartesian mode **without the camera** and measure where the tip really
  is with a ruler from the base axis.

| Arm error pattern | Likely cause |
|---|---|
| The same offset everywhere | calibration offset, or a joint zero (e.g. J1 base zero angle) |
| Grows with distance / with base angle | link lengths, servo scaling (us per degree), IK factor |
| Random, a few mm | servo backlash / gear play |

Fix the arm first, then delete `points.csv` and redo the points.

## 9. Troubleshooting

| Message / problem | Cause | Fix |
|---|---|---|
| `Saw exactly ONE block in only 0 frames (... [2, 2, 2 ...])` | two blocks (or a block-like object) in view | remove it; check with `--snapshot` |
| `... in only 7 frames (... [0, 0, 1, 0 ...])` | the block is hard to see: image edge, next to the station / an object, poor depth | move the spot towards the middle, 3 cm from anything |
| `WARNING: no depth on this block` | black block | use red or blue |
| `WARNING: jitter above 5 mm` | block or camera moving, poor depth | delete the point, measure again |
| `WARNING: the block is on the station, so Z should be ...` / `on the table, but Z is high` | Z typed as the gripper height | fix the line (4.4) |
| Point saved `on the table` while the block is on the station | the station was not found | `--snapshot`, read the `not the station at ...` reason; keep the arm 6 cm away from it |
| A point much worse than the others | placed or typed wrong | delete it, solve again |
| Bridge log still says `frame CAMERA (no calib_T.npy ...)` | file missing or bridge not restarted | `ls ~/arm/calib_T.npy`, `sudo systemctl restart pi_bridge` |
| Everything off after a while | camera or robot moved | recalibrate |
| `scp` asks to trust `10.10.10.123` from inside the Pi | `scp` was typed in the SSH session | answer `no`, run it in a PC PowerShell window |
| `The token '&&' is not a valid statement separator` | Windows PowerShell 5.1 | one command per line |

## 10. Still open

- **J1 (IK / motion owner):** measure the real tip at X 250 Y 0 Z 15 and X 250 Y 120 Z 15. Middle also turned
  ~14 deg -> J1 zero angle; middle right but the sides short -> J1 scaling (servo calibration of `MG996R-1` or IK).
  Then redo all points (aim < 5 mm) and update section 7.
- More points nearer the robot (X 120-180).
- Voice IK still blocked (`ARM_GEOMETRY_MEASURED 0`): until it is enabled, only Cartesian mode can use the robot
  positions.
- Bridge frame rate dropped to 3-6 fps with station + hand detection on the Pi 4 (was about 14): fine for `FIND`
  (answers from the last second); watch it.
