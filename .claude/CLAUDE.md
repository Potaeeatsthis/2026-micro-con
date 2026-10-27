@AGENTS.md

# Project memory for Claude Code

## Current task: station detection (Pi vision)
- Code: `ProjectDevGuide/tools/pi/pi_vision_uart.py` (class `BlockDetector`), a copy of `~/arm/pi_vision_uart.py`
  on the Pi (Pi 4 + RealSense D435, Python 3.12). Full plan, decisions and test results:
  `ProjectDevGuide/docs/station-detection.md`. Update that doc when a decision changes.
- Station (default since 27 Oct): kraft cardboard box 200 x 100 x 45 mm, 200 side left-right in the view. The
  arm DROPS blocks on it or PICKS blocks lying on it. Before: grey-blue tray 210 x 160 x 75 (blue block blended in,
  camera had to be held high). Tests follow `STATION_SIZE_MM`. Blocks: clay, ~25 mm wide, 15-20 mm tall, red / green / blue / black.
- Camera: ~25 cm high, tilted 45 deg down, 30-45 cm from the work area, USB 2 (depth 480x270 aligned onto
  colour 424x240).
- Method: depth only (top between 0.8 x H and H + 12 mm above the table + size + flatness). No colour, ML, ArUco.
- Decided 27 Oct after the first real snapshot (~50% depth missing, top in pieces): join pieces, measure edges by
  percentiles, rebuild ONE hidden side only when an edge is clearly seen; a side >10 mm short is never reported
  (a wrong centre is worse than NOTFOUND). Block (table) detection still unchanged.
- 27 Oct, real camera (tray): found once the camera was raised and tilted more toward the work area (low +
  shallow gave ~50% missing depth). Box on the real camera (27 Oct): found, conf 83, top 45 mm, size
  191 x 95 (measure the real top), 4 blocks on it right colour, drop FULL / "+" on the free end. OPEN: found
  and not found in the same layout (length at the 10 mm trust limit) -> set the measured size as the default.
- Hand/arm within ~6 cm of the station at its height hides it (NOTFOUND, never misplaced). Do NOT add a "grow /
  leave the hand out" rebuild again: tried 27 Oct, it misplaced the station by 80-100 mm with patchy depth.
- Output: `det.station` = {centre_cam_m, corners_cam_m, size_mm, top_mm, yaw, drop_cam_m, n_blocks, conf} or
  None. Pi link: `FIND station` -> `OBJ station x y z` (drop spot) / `NOTFOUND station`.

## Rules for this task
- Don't change how blocks on the TABLE are detected unless a test shows it is needed.
- Must keep working on USB 2 (480x270) and on the Pi 4 CPU.
- Test in simulation first (`ProjectDevGuide/tools/pi/test_station_sim.py`), then on the real camera.
- Explain changes in simple words; give exact commands for the Pi (SSH) and Windows PowerShell.
  `scp` always needs a destination as its LAST argument (a missing one once overwrote `color_refs.json`).
- Core files (`App/common/`, safety, app.c ...) belong to Supanut: edit them only when he approves (27 Oct he
  did, for `OBJ_STATION`); otherwise write down what is needed (AGENTS.md).
- STM32 side done 27 Oct: `OBJ_STATION`, `"station"` in pi_proto.c, voice `station`+`yes` -> FIND station,
  `STATION_RELEASE_MM` 20 mm placeholder. Board round trip not tested yet.
