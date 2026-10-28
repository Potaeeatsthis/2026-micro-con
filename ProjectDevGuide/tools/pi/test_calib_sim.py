#!/usr/bin/env python3
"""
test_calib_sim.py - camera -> robot calibration (--add-point + --solve-calib) on synthetic frames

A made-up robot base sits on the table (origin on its axis, +X forward, +Y left, +Z up). For each calibration
spot a block is rendered with its top-face centre exactly at robot X Y Z (as if the arm had put it there),
run_add_point() measures it, and run_solve_calib() must recover the transform: small RMS, the right camera
position, and new blocks mapped to the right robot XYZ. No camera needed:
    python test_calib_sim.py
"""
import contextlib
import io
import math
import os
import sys
import tempfile

import numpy as np

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import pi_vision_uart as pv  # noqa: E402
import test_station_sim as T  # noqa: E402

FAILS = []


def check(cond, msg):
    print(("  ok    " if cond else "  FAIL  ") + msg)
    if not cond:
        FAILS.append(msg)


# the robot base in the table/world frame of test_station_sim: 180 mm right of the camera's view line, 120 mm
# in front of the camera, turned so its +X points 20 deg left of the camera's "forward"
BASE_O = np.array([180.0, 120.0, 0.0])
BASE_YAW = math.radians(110.0)
RZ = np.array([[math.cos(BASE_YAW), -math.sin(BASE_YAW), 0], [math.sin(BASE_YAW), math.cos(BASE_YAW), 0], [0, 0, 1]])
BLOCK_H = 15.0


def robot_to_world(p):
    return BASE_O + RZ @ np.asarray(p, float)


def world_to_robot(w):
    return RZ.T @ (np.asarray(w, float) - BASE_O)


class Cam:
    def __init__(self, col, dep):
        self.col, self.dep = col, dep

    def grab(self):
        return self.col, self.dep


def scene_for(robot_xyz, seed, name="red"):
    """Station + one block whose top-face centre is at robot_xyz."""
    w = robot_to_world(robot_xyz)
    st = T.station_box(-60, 270, 5)
    z0 = w[2] - BLOCK_H
    blk = T.Box(w[0], w[1], 25, 18, BLOCK_H, 20, T.BLOCK_RGB[name], z0=z0, name=name)
    return T.render([st, blk], T.camera_pose(), seed=seed)


def main():
    pose = T.camera_pose()
    st_top = T.STATION[2]
    # spots inside the camera view (world mm), turned into robot X Y Z: 6 on the floor (Z = block height)
    # spread near / far / left / right + 3 on the station top (Z = station + block height)
    floor_w = [(110, 170), (170, 210), (130, 285), (165, 330), (20, 160), (-110, 165)]
    floor = [tuple(world_to_robot([x, y, BLOCK_H]).round(1)) for x, y in floor_w]
    st_w = [np.array([-110.0, 265.0]), np.array([-60.0, 255.0]), np.array([-10.0, 280.0])]
    station = [tuple(world_to_robot([x, y, st_top + BLOCK_H]).round(1)) for x, y in st_w]
    spots = floor + station
    old = os.getcwd()
    with tempfile.TemporaryDirectory() as tmp:
        os.chdir(tmp)
        try:
            print("== --add-point for each spot")
            for i, p in enumerate(spots):
                col, dep = scene_for(p, seed=40 + i)
                det = T.new_detector()
                out = io.StringIO()
                with contextlib.redirect_stdout(out):
                    pv.run_add_point(Cam(col, dep), det, p)
                line = [ln for ln in out.getvalue().splitlines() if ln.startswith("Point")][0]
                print("  " + line)
                check("WARNING" not in out.getvalue(), f"spot {i + 1}: no warnings")
            print("== --solve-calib")
            out = io.StringIO()
            with contextlib.redirect_stdout(out):
                pv.run_solve_calib("points.csv")
            print("\n".join("  " + ln for ln in out.getvalue().splitlines()))
            Tm = np.load(pv.CALIB_FILE)
        finally:
            os.chdir(old)

    text = out.getvalue()
    rms = float(text.split("RMS error:")[1].split("mm")[0])
    check(rms < 5.0, f"RMS {rms:.1f} mm (< 5)")
    R, C = pose
    cam_robot = world_to_robot(C)
    got = Tm[:3, 3] * 1000.0
    check(np.linalg.norm(got - cam_robot) < 15, f"camera position {got.round(0)} vs true {cam_robot.round(0)} mm")
    print("== new blocks, not used for the calibration")
    for k, p in enumerate([tuple(world_to_robot([x, y, BLOCK_H]).round(1)) for x, y in ((70, 215), (-150, 185))]):
        col, dep = scene_for(p, seed=90 + k, name="green")
        det = T.new_detector()
        bl = T.run(det, col, dep)
        b = [b for b in bl if b["surface"] == "table"][0]
        pr = (Tm[:3, :3] @ b["xyz_cam_m"] + Tm[:3, 3]) * 1000.0
        err = float(np.linalg.norm(pr - np.array(p)))
        check(err < 8, f"block at robot {p}: reported {pr.round(0)} ({err:.1f} mm off, < 8)")
    print(f"\n{'ALL PASSED' if not FAILS else f'{len(FAILS)} FAILED'}")
    sys.exit(1 if FAILS else 0)


if __name__ == "__main__":
    main()
