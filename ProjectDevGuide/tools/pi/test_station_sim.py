#!/usr/bin/env python3
"""
test_station_sim.py - station + block detection tests on synthetic camera frames (no camera needed)

Renders what a D435 on USB 2 sees (424x240, depth aligned onto colour): a wooden table, the station
(pi_vision_uart.STATION_SIZE_MM) and clay blocks, from a camera ~25 cm high tilted 45 deg down. Then runs pi_vision_uart's
BlockDetector on it and checks the results against the true positions.

Run on the PC or the Pi (needs numpy + opencv, not pyrealsense2):
    python test_station_sim.py            # all tests
    python test_station_sim.py -k yaw     # only tests whose name contains "yaw"
    python test_station_sim.py --save     # also write sim_<test>.jpg snapshots
"""

import argparse
import math
import os
import sys
import time
from types import SimpleNamespace

import cv2
import numpy as np

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import pi_vision_uart as pv  # noqa: E402

W, H = 424, 240
INTR = SimpleNamespace(fx=308.0, fy=308.0, ppx=212.0, ppy=120.0)  # D435 colour 424x240 (approx.)
DEPTH_SCALE = 0.001

WOOD = (205, 172, 128)       # RGB
STATION_TOP = (172, 190, 196)
STATION_SIDE = (138, 156, 163)
BLOCK_RGB = {"red": (200, 40, 42), "green": (70, 165, 50), "blue": (45, 150, 215), "black": (32, 32, 34)}
STATION = tuple(pv.STATION_SIZE_MM)  # the tests follow the detector's station size


# ------------------------------------------------------------------ scene --
class Box:
    """Axis box on the table, yawed about the vertical axis. Sizes and centre in mm."""

    def __init__(self, cx, cy, length, width, height, yaw_deg, rgb, side_rgb=None, z0=0.0,
                 no_depth=False, name=None):
        self.c = np.array([cx, cy, z0], float)
        self.half = np.array([length / 2, width / 2, height], float)
        self.yaw = math.radians(yaw_deg)
        self.rgb, self.side_rgb = np.array(rgb, float), np.array(side_rgb or rgb, float) * (0.82 if side_rgb is None else 1)
        self.no_depth, self.name, self.height, self.z0 = no_depth, name, height, z0

    def top_centre(self):
        return self.c + np.array([0, 0, self.height])


def camera_pose(height_mm=250.0, tilt_deg=45.0, back_mm=0.0):
    """World: table z = 0, +x right, +y forward (away), +z up. Camera above (0, -back), looking forward/down.
    Returns (R cam->world, C in mm). Camera axes: x right, y down, z forward."""
    t = math.radians(tilt_deg)
    fwd = np.array([0.0, math.cos(t), -math.sin(t)])
    right = np.array([1.0, 0.0, 0.0])
    down = np.cross(fwd, right)
    R = np.column_stack([right, down, fwd])
    return R, np.array([0.0, -back_mm, height_mm])


def render(boxes, pose, light=1.0, cast=(1.0, 1.0, 1.0), depth_noise_mm=1.5, hole_frac=0.03,
           bumps=None, seed=0, blob_holes=0.0):
    """Ray-trace colour (BGR uint8) and depth (uint16, mm) at 424x240."""
    rng = np.random.default_rng(seed)
    R, C = pose
    u, v = np.meshgrid(np.arange(W, dtype=float), np.arange(H, dtype=float))
    rc = np.dstack([(u - INTR.ppx) / INTR.fx, (v - INTR.ppy) / INTR.fy, np.ones_like(u)])  # z_cam = 1
    dirs = rc @ R.T  # world direction; t along it = camera depth (z_cam)

    t_best = np.full((H, W), np.inf)
    rgb = np.zeros((H, W, 3))
    nodepth = np.zeros((H, W), bool)

    # table plane z = 0 (+ optional smooth bumps in the depth only)
    with np.errstate(divide="ignore", invalid="ignore"):
        t_tab = -C[2] / dirs[..., 2]
    ok = np.isfinite(t_tab) & (t_tab > 0)
    t_best[ok] = t_tab[ok]
    pts = C + dirs * np.where(ok, t_tab, 0)[..., None]
    grain = 1.0 + 0.04 * np.sin(pts[..., 0] * 0.15 + 0.3 * np.sin(pts[..., 1] * 0.05))
    rgb[ok] = np.array(WOOD, float) * grain[ok][:, None]

    for b in boxes:
        cy, sy = math.cos(b.yaw), math.sin(b.yaw)
        Rb = np.array([[cy, -sy, 0], [sy, cy, 0], [0, 0, 1]])  # box -> world
        o = (C - b.c) @ Rb  # camera origin in box frame
        dl = dirs @ Rb
        lo = np.array([-b.half[0], -b.half[1], 0.0])
        hi = np.array([b.half[0], b.half[1], b.half[2]])
        with np.errstate(divide="ignore", invalid="ignore"):
            t1 = (lo - o) / dl
            t2 = (hi - o) / dl
        tmin = np.nanmax(np.minimum(t1, t2), axis=2)
        tmax = np.nanmin(np.maximum(t1, t2), axis=2)
        hit = (tmax >= tmin) & (tmin > 0) & (tmin < t_best)
        if not hit.any():
            continue
        entry_axis = np.argmax(np.minimum(t1, t2), axis=2)  # which slab we entered through
        top = hit & (entry_axis == 2)
        t_best[hit] = tmin[hit]
        rgb[hit] = b.side_rgb
        rgb[top] = b.rgb
        nodepth[hit] = b.no_depth

    z_mm = t_best.copy()
    if bumps:
        for (bx, by, amp, r) in bumps:  # depth-only bumps on the table (noise test)
            pts = C + dirs * np.where(np.isfinite(t_best), t_best, 0)[..., None]
            on_table = np.abs(pts[..., 2]) < 1.0
            g = amp * np.exp(-((pts[..., 0] - bx) ** 2 + (pts[..., 1] - by) ** 2) / (2 * r * r))
            z_mm = np.where(on_table, z_mm - g / np.maximum(1e-6, -dirs[..., 2]), z_mm)
    z_mm = z_mm + rng.normal(0, depth_noise_mm, z_mm.shape) * (z_mm / 400.0) ** 2
    depth = np.where(np.isfinite(z_mm) & ~nodepth, np.round(z_mm), 0)
    holes = rng.random((H, W)) < hole_frac
    holes = cv2.dilate(holes.astype(np.uint8), np.ones((2, 2), np.uint8)) > 0
    depth[holes] = 0
    if blob_holes > 0:  # big patchy holes like the real D435 on USB 2 (27 Oct snapshot: ~50% no depth)
        noise = cv2.resize(rng.random((14, 24)).astype(np.float32), (W, H), interpolation=cv2.INTER_CUBIC)
        noise += 0.35 * cv2.resize(rng.random((34, 60)).astype(np.float32), (W, H), interpolation=cv2.INTER_CUBIC)
        depth[noise < np.quantile(noise, blob_holes)] = 0
    depth = np.clip(depth, 0, 65535).astype(np.uint16)

    shade = 1.0 + rng.normal(0, 0.015, (H, W, 1))
    col = rgb * light * np.array(cast) * shade
    col = np.clip(col + rng.normal(0, 2.0, col.shape), 0, 255).astype(np.uint8)
    return col[..., ::-1].copy(), depth


def world_to_cam_mm(p_world, pose):
    R, C = pose
    return (p_world - C) @ R  # == R.T @ (p - C)


def new_detector():
    pv.fit_plane.__defaults__ = (150, 0.004, np.random.default_rng(0))  # RANSAC uses a shared generator
    det = pv.BlockDetector(INTR, DEPTH_SCALE, {k: [list(v) for v in vals] for k, vals in pv.DEFAULT_REFS.items()})
    return det


def run(det, color, depth, frames=3):
    blocks = []
    for i in range(frames):
        blocks = det.detect(color, depth, force_plane=(i == 0))
    return blocks


def station_box(cx, cy, yaw):
    L, Wd, Hh = STATION
    return Box(cx, cy, L, Wd, Hh, yaw, STATION_TOP, STATION_SIDE, name="station")


def block(cx, cy, name, z0=0.0, yaw=10.0, size=25.0, height=17.0):
    return Box(cx, cy, size, size, height, yaw, BLOCK_RGB[name], z0=z0,
               no_depth=(name == "black"), name=name)


def on_station(st, along, across, name):
    """A block on the station top, `along` / `across` mm from its centre in the station's own axes."""
    c, s_ = math.cos(st.yaw), math.sin(st.yaw)
    return block(st.c[0] + along * c - across * s_, st.c[1] + along * s_ + across * c, name, z0=STATION[2],
                 yaw=math.degrees(st.yaw) + 10)


def hand_and_bar(cx, cy, yaw):
    """Things at station height that are NOT the station: a palm-sized box and a long thin arm-like bar."""
    H = STATION[2]
    return [Box(cx, cy, 90, 70, H, yaw, (210, 160, 140), name="hand"),
            Box(cx + 60, cy + 40, 260, 40, H + 5, -yaw, (90, 90, 95), name="arm")]


# ------------------------------------------------------------------ checks --
FAILS = []


def check(cond, msg):
    print(("  ok    " if cond else "  FAIL  ") + msg)
    if not cond:
        FAILS.append(msg)
    return cond


def match_blocks(found, truth, pose, tol_mm=8.0, label=""):
    """Every true block must be found once with the right colour, within tol_mm (top-face centre)."""
    for b in truth:
        want = world_to_cam_mm(b.top_centre(), pose)
        best, best_d = None, 1e9
        for f in found:
            d = np.linalg.norm(f["xyz_cam_m"] * 1000.0 - want)
            if d < best_d:
                best, best_d = f, d
        surf = "station" if b.z0 > 0 else "table"
        ok = best is not None and best_d <= tol_mm and best["name"] == b.name and best.get("surface", "table") == surf
        got = "none" if best is None else (f"{best['name']} on {best.get('surface', 'table')} c{best['conf']}, "
                                           f"{best_d:.1f} mm" + ("" if best["depth_ok"] else " (est)"))
        check(ok, f"{label}{b.name} block on {surf}: {got}")
    extra = len(found) - len(truth)
    check(extra <= 0, f"{label}no extra detections ({len(found)} found for {len(truth)} blocks)")


def check_station(det, st_box, pose, label=""):
    s = det.station
    if not check(s is not None, f"{label}station found"):
        return
    L, Wd, Hh = STATION
    want_c = world_to_cam_mm(st_box.top_centre(), pose)
    dc = np.linalg.norm(s["centre_cam_m"] * 1000.0 - want_c)
    check(abs(s["size_mm"][0] - L) <= 12 and abs(s["size_mm"][1] - Wd) <= 12,
          f"{label}size {s['size_mm'][0]:.0f} x {s['size_mm'][1]:.0f} mm (want {L:.0f} x {Wd:.0f} +-12)")
    check(abs(s["top_mm"] - Hh) <= 4, f"{label}top {s['top_mm']:.1f} mm (want {Hh:.0f} +-4)")
    check(dc <= 8, f"{label}centre error {dc:.1f} mm (<= 8)")


# ------------------------------------------------------------------- tests --
def t_station_yaw(save):
    for yaw in (15.0, -35.0):
        pose = camera_pose()
        st = station_box(0, 250, yaw)
        col, dep = render([st], pose, seed=int(yaw + 50))
        det = new_detector()
        blocks = run(det, col, dep)
        check_station(det, st, pose, f"yaw {yaw:+.0f}: ")
        check(len(blocks) == 0, f"yaw {yaw:+.0f}: no blocks on an empty scene ({len(blocks)})")
        s = det.station
        if s is not None:
            dd = np.linalg.norm(s["drop_cam_m"] - s["centre_cam_m"]) * 1000
            check(dd < 1.0, f"yaw {yaw:+.0f}: empty station -> drop = centre ({dd:.1f} mm)")
        save_snapshot(save, f"yaw{int(yaw)}", det, col, dep, blocks)


def t_blocks_both_surfaces(save):
    pose = camera_pose()
    st = station_box(-10, 245, 15)
    # the two hardest cases on the station: blue (close to the station colour) and black (no depth)
    on_st = [on_station(st, -60, -10, "blue"), on_station(st, 60, 10, "black")]
    on_tab = [block(-175, 230, "green"), block(160, 190, "red"), block(150, 320, "black")]
    col, dep = render([st] + on_st + on_tab, pose, seed=3)
    det = new_detector()
    blocks = run(det, col, dep)
    check_station(det, st, pose, "mixed: ")
    match_blocks(blocks, on_st + on_tab, pose, label="mixed: ")
    check_drop(det, blocks, "mixed: ")
    save_snapshot(save, "mixed", det, col, dep, blocks)


def t_drop_spot_busy(save):
    pose = camera_pose()
    st = station_box(0, 250, -20)
    # one block right in the middle of the top: the drop spot must move away from it
    on_st = [on_station(st, 0, 0, "green"), on_station(st, 55, 12, "red")]
    col, dep = render([st] + on_st, pose, seed=4)
    det = new_detector()
    blocks = run(det, col, dep)
    check_station(det, st, pose, "busy: ")
    match_blocks(blocks, on_st, pose, label="busy: ")
    check_drop(det, blocks, "busy: ")
    save_snapshot(save, "busy", det, col, dep, blocks)


def t_station_full(save):
    """Blocks every ~65 mm along the top: no spot is 45 mm from all of them -> no drop spot (FULL)."""
    pose = camera_pose()
    st = station_box(0, 250, 5)
    L = STATION[0]
    along = np.linspace(-(L / 2 - 35), L / 2 - 35, 3)
    on_st = [on_station(st, a, 0, c) for a, c in zip(along, ("red", "green", "blue"))]
    col, dep = render([st] + on_st, pose, seed=21)
    det = new_detector()
    blocks = run(det, col, dep)
    check_station(det, st, pose, "full: ")
    match_blocks(blocks, on_st, pose, label="full: ")
    if det.station is not None:
        check(det.station["drop_cam_m"] is None, "full: no drop spot when every spot is within 45 mm of a block")
    save_snapshot(save, "full", det, col, dep, blocks)


def check_drop(det, blocks, label):
    s = det.station
    if s is None:
        return
    st_blocks = [b for b in blocks if b.get("surface") == "station"]
    check(s["n_blocks"] == len(st_blocks), f"{label}n_blocks {s['n_blocks']} == {len(st_blocks)}")
    if s["drop_cam_m"] is None:
        check(False, f"{label}drop spot exists")
        return
    dmin = min([np.linalg.norm(s["drop_cam_m"] - b["xyz_cam_m"]) * 1000 for b in st_blocks] or [999])
    check(dmin >= 40, f"{label}drop spot {dmin:.0f} mm from the nearest block (>= 40)")


def t_camera_close(save):
    """Station fills most of the view: the table plane must still be the real table."""
    pose = camera_pose(height_mm=200, tilt_deg=50)
    st = station_box(0, 205, 5)
    col, dep = render([st], pose, seed=5)
    det = new_detector()
    run(det, col, dep)
    n, d = det.plane
    R, C = pose
    cam_h = (n @ np.zeros(3) + d) * 1000  # height of the camera above the fitted plane
    with np.errstate(invalid="ignore"):
        top_share = np.nanmean(np.abs(det.last_height_mm - STATION[2]) < 10)
    check(abs(cam_h - C[2]) < 10, f"close: table plane is the real table (camera {cam_h:.0f} mm above it, "
                                  f"true {C[2]:.0f}; station top = {top_share:.0%} of pixels)")
    save_snapshot(save, "close", det, col, dep, [])


def t_no_false_station(save):
    """Bumpy depth, a hand-like blob at station height and an arm-like bar: no station."""
    pose = camera_pose()
    H = STATION[2]
    bumps = [(-60, 240, 0.95 * H, 25), (80, 290, 0.9 * H, 30)]  # smooth bumps at station height (not flat)
    hand, bar = hand_and_bar(-40, 230, 30)
    for name, boxes, bmp in (("bumps", [], bumps), ("hand", [hand], None), ("arm bar", [bar], None)):
        col, dep = render(boxes, pose, bumps=bmp, seed=6)
        det = new_detector()
        run(det, col, dep)
        check(det.station is None, f"{name}: no station ({'none' if det.station is None else det.station['size_mm']})")
        save_snapshot(save, name.replace(' ', '_'), det, col, dep, [])


def t_table_unchanged(save):
    """Table-only scenes: block results identical with and without the station code, all lightings."""
    pose = camera_pose()
    blocks_t = [block(-80, 220, "red"), block(-10, 260, "green"), block(60, 220, "blue"), block(110, 290, "black")]
    for light, cast, name in ((1.0, (1, 1, 1), "normal"), (0.55, (1, 1, 1), "dim"),
                              (1.0, (1.12, 1.0, 0.8), "warm"), (1.0, (0.85, 0.95, 1.15), "blue-tint")):
        col, dep = render(blocks_t, pose, light=light, cast=cast, seed=7)
        det_on = new_detector()
        a = run(det_on, col, dep)
        det_off = new_detector()  # fresh RANSAC generator, so both see the same samples
        det_off.station_enabled = False
        b = run(det_off, col, dep)
        same = len(a) == len(b) and all(x["name"] == y["name"] and np.allclose(x["xyz_cam_m"], y["xyz_cam_m"])
                                        for x, y in zip(a, b))
        check(same, f"{name} light: same blocks with and without station code ({len(a)} vs {len(b)})")
        match_blocks(a, blocks_t, pose, label=f"{name} light: ")


def t_timing(save):
    pose = camera_pose()
    st = station_box(-10, 245, 15)
    scene = [st, on_station(st, -50, 0, "red"), block(-175, 230, "green"), block(160, 190, "blue")]
    col, dep = render(scene, pose, seed=8)
    for enabled in (False, True):
        det = new_detector()
        det.station_enabled = enabled
        run(det, col, dep)
        t0 = time.perf_counter()
        for _ in range(20):
            det.detect(col, dep)
        ms = (time.perf_counter() - t0) / 20 * 1000
        print(f"  info  detect() {'with' if enabled else 'without'} station: {ms:.1f} ms per frame on this machine")


def save_snapshot(save, name, det, col, dep, blocks):
    if not save:
        return
    img = pv.draw(col.copy(), blocks, np.eye(4))
    if hasattr(pv, "draw_station"):
        pv.draw_station(img, det)
    hm = det.last_height_mm
    hv = cv2.applyColorMap(np.clip(np.nan_to_num(hm, nan=0) * 255 / 90, 0, 255).astype(np.uint8),
                           cv2.COLORMAP_JET)
    hv[np.isnan(hm)] = 0
    cv2.imwrite(f"sim_{name}.jpg", pv.enlarge(np.hstack([img, hv])))


def t_patchy_depth(save):
    """Real camera, 27 Oct: ~50% of the depth missing in blobs, top split into pieces, noisier depth."""
    pose = camera_pose()
    for seed in (11, 12, 13):
        st = station_box(0, 245, 10 * (seed - 12))
        on_st = [on_station(st, -30, 5, "red")]
        on_tab = [block(-160, 200, "green")]
        col, dep = render([st] + on_st + on_tab, pose, seed=seed, blob_holes=0.5, depth_noise_mm=4.0)
        det = new_detector()
        blocks = run(det, col, dep, frames=1)
        lab = f"patchy #{seed - 10}: "
        print(f"  info  {lab}{np.mean(dep == 0):.0%} of the depth missing")
        check_station(det, st, pose, lab)
        if det.station is not None:
            # block positions come from the unchanged block code; noisier depth costs some accuracy there
            match_blocks(blocks, on_st + on_tab, pose, tol_mm=15.0, label=lab)
        save_snapshot(save, f"patchy{seed - 10}", det, col, dep, blocks)


def t_sweep(save):
    """Random station poses, normal and patchy depth: how often it is found, how far off, false stations."""
    pose = camera_pose()
    for label, kw, need in (("normal", {}, 1.0), ("patchy", dict(blob_holes=0.5, depth_noise_mm=4.0), 0.9)):
        found, errs, false = 0, [], 0
        for seed in range(300, 320):
            rng = np.random.default_rng(seed)
            yaw, cx, cy = rng.uniform(-40, 40), rng.uniform(-30, 30), rng.uniform(235, 260)
            st = station_box(cx, cy, yaw)
            col, dep = render([st], pose, seed=seed, **kw)
            det = new_detector()
            run(det, col, dep, frames=1)
            if det.station is not None:
                found += 1
                errs.append(np.linalg.norm(det.station["centre_cam_m"] * 1000 - world_to_cam_mm(st.top_centre(), pose)))
            others = hand_and_bar(cx, cy, yaw)
            col, dep = render(others, pose, seed=seed, **kw)
            det = new_detector()
            run(det, col, dep, frames=1)
            false += det.station is not None
        check(found >= need * 20, f"{label}: station found {found}/20")
        check(not errs or max(errs) <= 8, f"{label}: centre error max {max(errs or [0]):.1f} mm (<= 8)")
        check(false == 0, f"{label}: no false station from a hand-sized box + arm bar ({false}/20)")


TESTS = [t_station_yaw, t_patchy_depth, t_sweep, t_blocks_both_surfaces, t_drop_spot_busy, t_station_full, t_camera_close, t_no_false_station,
         t_table_unchanged, t_timing]


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("-k", default="", help="only run tests whose name contains this")
    ap.add_argument("--save", action="store_true", help="write sim_<test>.jpg pictures")
    args = ap.parse_args()
    for t in TESTS:
        if args.k in t.__name__:
            print(f"== {t.__name__[2:]}")
            t(args.save)
    print(f"\n{'ALL PASSED' if not FAILS else f'{len(FAILS)} FAILED'}")
    sys.exit(1 if FAILS else 0)


if __name__ == "__main__":
    main()
