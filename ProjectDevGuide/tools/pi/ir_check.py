#!/usr/bin/env python3
"""
ir_check.py - is the D435 projector working? (depth-quality check, no STM32 needed)

Saves ir_check.jpg: the left infrared image and the depth (black = no depth) with the projector OFF (top row)
and ON at full power (bottom row), and prints the share of depth holes for each.
A working projector shows thousands of small bright DOTS on the station and the table in the ON row, and
far fewer holes. Stop the bridge service first (it uses the camera):
    sudo systemctl stop pi_bridge
    python ir_check.py
"""
import sys
import time

import cv2
import numpy as np

try:
    import pyrealsense2 as rs
except ImportError:
    sys.exit("pyrealsense2 not installed")

W, H, FPS = 480, 270, 15  # the same depth mode pi_vision_uart.py uses on USB 2


def grab(pipe, n=5):
    for _ in range(10):  # let the emitter change and auto-exposure settle
        pipe.wait_for_frames(3000)
    holes, last = [], None
    for _ in range(n):
        f = pipe.wait_for_frames(3000)
        d, ir = np.asanyarray(f.get_depth_frame().get_data()), np.asanyarray(f.get_infrared_frame(1).get_data())
        holes.append(np.mean(d == 0))
        last = (ir, d)
    return float(np.mean(holes)), last


def main():
    pipe = rs.pipeline()
    cfg = rs.config()
    cfg.enable_stream(rs.stream.depth, W, H, rs.format.z16, FPS)
    cfg.enable_stream(rs.stream.infrared, 1, W, H, rs.format.y8, FPS)
    prof = pipe.start(cfg)
    ds = prof.get_device().first_depth_sensor()
    rows = []
    try:
        for name, on in (("OFF", 0), ("ON", 1)):
            ds.set_option(rs.option.emitter_enabled, on)
            if on and ds.supports(rs.option.laser_power):
                ds.set_option(rs.option.laser_power, ds.get_option_range(rs.option.laser_power).max)
            holes, (ir, d) = grab(pipe)
            exp = ds.get_option(rs.option.exposure) if ds.supports(rs.option.exposure) else float("nan")
            print(f"projector {name:3s}: {holes:.0%} of the depth missing  (IR exposure {exp:.0f} us, "
                  f"IR brightness {ir.mean():.0f}/255, {np.mean(ir >= 250):.1%} of IR pixels saturated)")
            dv = cv2.applyColorMap(cv2.convertScaleAbs(d, alpha=255.0 / 1500), cv2.COLORMAP_JET)
            dv[d == 0] = 0
            irv = cv2.cvtColor(ir, cv2.COLOR_GRAY2BGR)
            for img, label in ((irv, f"IR, projector {name}"), (dv, f"depth, {holes:.0%} holes")):
                cv2.putText(img, label, (6, 18), cv2.FONT_HERSHEY_SIMPLEX, 0.55, (0, 0, 0), 3)
                cv2.putText(img, label, (6, 18), cv2.FONT_HERSHEY_SIMPLEX, 0.55, (255, 255, 255), 1)
            rows.append(np.hstack([irv, dv]))
    finally:
        pipe.stop()
    cv2.imwrite("ir_check.jpg", np.vstack(rows))
    print("Saved ir_check.jpg. Projector ON row: look for many small bright dots on the station and the table.")


if __name__ == "__main__":
    main()
