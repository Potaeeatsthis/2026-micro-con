#!/usr/bin/env python3
"""
pi_vision_uart.py - RealSense D435 block detector -> STM32 NUCLEO-F767ZI over UART

How it finds blocks (lighting-independent)
  1. Fit the TABLE as a flat plane from the depth image (RANSAC, refreshed every second).
  2. A block is anything 8-35 mm above that plane and 12-40 mm across.
     Black blocks that return no depth are found as dark "holes" sitting on the table,
     and their position is computed from the table plane + 20 mm.
  3. Colour (red / green / blue / black) is judged RELATIVE to the table right next to
     the block, so dim, yellow or blue-tinted lighting cancels out.
  4. At a new venue, run --learn once (about 30 s) so it learns that room's colours.
  5. The STATION (a 200 x 100 x 45 mm box, --station-size) is found by depth only: a flat top between
     0.8 x its height and its height + 12 mm above the table, with the right length and width. Blocks lying on it are found the same way, with
     the station top as their "table" (surface = "station"; colours taught on it are saved as
     "red@station" ...). det.station holds its centre, corners, top height, yaw and a free drop spot.

Wire protocol (little-endian)
  0xAA 0x55 | type u8 | seq u8 | len u8 | payload[len] | crc16 u16
  crc16 = CRC-16/CCITT-FALSE (poly 0x1021, init 0xFFFF) over type..payload

  type 0x01 PING            STM32 -> Pi   payload: none
  type 0x02 PONG            Pi -> STM32   payload: none
  type 0x10 REQUEST_TARGET  STM32 -> Pi   payload: color_id u8  (0 red, 1 green, 2 blue, 3 black)
  type 0x11 TARGET          Pi -> STM32   payload (11 bytes):
        color_id u8, found u8 (0/1), x_mm i16, y_mm i16, z_mm i16,
        confidence u8 (0-100), age_ms u16
  type 0x7F NACK            Pi -> STM32   payload: rejected type u8
  Replies reuse the request's seq so the STM32 can match them.
  x, y, z = centre of the block's TOP face, robot-base frame. Grasp centre = z - 10 mm.

Pi setup: see SETUP_PI.md (64-bit OS + Python 3.12 venv).

Usage
  python pi_vision_uart.py --no-uart --show          # camera only, preview window
  python pi_vision_uart.py --learn                   # learn colours at this venue (needs a screen)
  python pi_vision_uart.py --show                    # camera + UART to STM32
  python pi_vision_uart.py --print-cam --no-uart     # print camera-frame XYZ for calibration
  python pi_vision_uart.py --solve-calib points.csv  # build calib_T.npy from measured points

Calibration (camera -> robot base)
  Put a block at 6-8 spread-out spots (some raised on a known-height box).
  For each spot, write the --print-cam XYZ and the ruler-measured XYZ of the
  block's TOP-FACE CENTRE from the robot base origin, one line per spot (mm):
      xc,yc,zc,xb,yb,zb
  Then run --solve-calib points.csv. For 2 cm blocks aim for RMS error < 5 mm.
"""

import argparse
import json
import os
import struct
import sys
import time

import cv2
import numpy as np

try:
    import serial
except ImportError:  # only needed when the UART is used
    serial = None

try:
    import pyrealsense2 as rs
except ImportError:  # allows --solve-calib and unit tests without the SDK
    rs = None

# ---------------------------------------------------------------- protocol --
SOF = b"\xAA\x55"
MSG_PING = 0x01
MSG_PONG = 0x02
MSG_REQ_TARGET = 0x10
MSG_TARGET = 0x11
MSG_NACK = 0x7F
MAX_PAYLOAD = 64
TARGET_FMT = "<BBhhhBH"  # 11 bytes, must match pi_link.c

# ------------------------------------------------------------ detector cfg --
COLOR_NAMES = {0: "red", 1: "green", 2: "blue", 3: "black"}
COLOR_KEYS = {ord("r"): "red", ord("g"): "green", ord("b"): "blue", ord("k"): "black"}
DRAW_BGR = {"red": (0, 0, 255), "green": (0, 200, 0), "blue": (255, 80, 0), "black": (60, 60, 60)}

# Block size: set with --block-size / --block-height (mm), or edit here.
BLOCK_SIZE_MM = 25.0             # width of the block seen from above
BLOCK_HEIGHT_MM = 15.0           # height of the block
SALIENCY_MIN = 0.45              # how different from the table a pixel must look to be a block
HEIGHT_RANGE_MM = TOP_MIN_MM = SIZE_RANGE_MM = None  # derived in configure_block()


def laser_arg(v):
    return "auto" if str(v).lower() == "auto" else int(v)


def configure_block(size_mm=None, height_mm=None):
    global BLOCK_SIZE_MM, BLOCK_HEIGHT_MM, HEIGHT_RANGE_MM, TOP_MIN_MM, SIZE_RANGE_MM
    if size_mm:
        BLOCK_SIZE_MM = float(size_mm)
    if height_mm:
        BLOCK_HEIGHT_MM = float(height_mm)
    HEIGHT_RANGE_MM = (max(5.0, 0.35 * BLOCK_HEIGHT_MM), BLOCK_HEIGHT_MM + 15.0)  # "raised" band
    TOP_MIN_MM = 0.6 * BLOCK_HEIGHT_MM                                         # top face
    SIZE_RANGE_MM = (0.5 * BLOCK_SIZE_MM, 1.9 * BLOCK_SIZE_MM)                 # incl. sides/shadow


configure_block()


def configure_station(size_mm=None, enabled=None):
    global STATION_ENABLED
    if size_mm:
        STATION_SIZE_MM[:] = [float(v) for v in size_mm]
    if enabled is not None:
        STATION_ENABLED = bool(enabled)


TABLE_TOL_MM = 5.0              # pixels within this of the plane count as table
MIN_TABLE_AROUND = 0.5           # >= 50% of the ring around a block must be table
DEPTH_MIN_M, DEPTH_MAX_M = 0.15, 1.2
PLANE_REFIT_FRAMES = 30          # refit the table plane about once per second
MAX_COLOR_DIST = 1.2             # further than this from every reference = unknown colour
MAX_AGE_MS = 500                 # older detections are reported as found=0

CALIB_FILE = "calib_T.npy"
REFS_FILE = "color_refs.json"

# Station: a flat-topped tray the arm drops blocks on or picks blocks from. Found by DEPTH only: a big flat
# top STATION_SIZE_MM[2] above the table with the right length and width. Set with --station-size.
STATION_ENABLED = True           # --no-station turns it off
STATION_SIZE_MM = [200.0, 100.0, 45.0]  # length, width, height (kraft box, 27 Oct; was the 210x160x75 tray)
STATION_SIZE_TOL = 0.25          # length and width within 25%
STATION_BAND = (0.8, 12.0)       # top pixels: height in [0.8 * H, H + 12 mm]
STATION_MIN_PX = 300             # smallest top component worth looking at
STATION_FLAT_MM = 8.0            # pixels within this of the top's median height belong to the top
STATION_MAX_ROUGH_MM = 6.0       # top must be flat: robust spread of those heights (real D435: 4-5 mm)
STATION_JOIN_PX = 35             # top pieces split by depth holes up to this far apart are one top
STATION_MIN_SEEN = 0.6           # a side hidden by a depth hole is rebuilt only if >= 60% of it is seen
STATION_TRUST_MM = 10.0          # a side this much too short is hidden: rebuild it or do not report
STATION_MIN_FILL = 0.75          # convex hull of the top / fitted rectangle (rejects hands, bars)
STATION_INSET_MM = 8.0           # blocks on the station are searched this far inside the edge
STATION_PLANE_FRAC = 0.10        # a parallel plane H below the biggest one, with >= 10% of points = table
DROP_INSET_MM = 35.0             # drop spot grid: 5 x 3 points this far inside the edge
DROP_CLEAR_MM = 45.0             # drop spot must be this far from every block already on the station
DROP_GRID = (5, 3)

# Built-in colour references, used until --learn has been run at the venue.
# Feature = [r, g, b chromaticity of (block / table), log(block brightness / table brightness)]
# Values assume a light, roughly neutral table.
DEFAULT_REFS = {
    "red":   [[0.68, 0.16, 0.16, -0.9]],
    "green": [[0.17, 0.60, 0.23, -0.9]],
    "blue":  [[0.13, 0.25, 0.62, -0.9]],
    "black": [[0.33, 0.33, 0.34, -1.9]],
}
FEATURE_WEIGHTS = np.array([3.0, 3.0, 3.0, 1.0])  # chromaticity matters more than brightness


def crc16_ccitt(data: bytes, crc: int = 0xFFFF) -> int:
    for b in data:
        crc ^= b << 8
        for _ in range(8):
            crc = ((crc << 1) ^ 0x1021) & 0xFFFF if crc & 0x8000 else (crc << 1) & 0xFFFF
    return crc


def build_frame(msg_type: int, seq: int, payload: bytes = b"") -> bytes:
    body = struct.pack("<BBB", msg_type, seq & 0xFF, len(payload)) + payload
    return SOF + body + struct.pack("<H", crc16_ccitt(body))


class FrameParser:
    """Byte-stream -> list of (type, seq, payload). Resyncs on bad CRC."""

    def __init__(self):
        self.buf = bytearray()
        self.bad_crc = 0

    def feed(self, data: bytes):
        self.buf += data
        frames = []
        while True:
            i = self.buf.find(SOF)
            if i < 0:
                del self.buf[:-1]  # keep a trailing 0xAA in case 0x55 follows
                break
            del self.buf[:i]
            if len(self.buf) < 5:
                break
            length = self.buf[4]
            if length > MAX_PAYLOAD:
                del self.buf[:2]
                continue
            total = 5 + length + 2
            if len(self.buf) < total:
                break
            body = bytes(self.buf[2:5 + length])
            (crc_rx,) = struct.unpack_from("<H", self.buf, 5 + length)
            if crc16_ccitt(body) == crc_rx:
                frames.append((body[0], body[1], body[3:]))
                del self.buf[:total]
            else:
                self.bad_crc += 1
                del self.buf[:2]
        return frames


def clamp_i16(v: float) -> int:
    return max(-32768, min(32767, int(round(v))))


def target_payload(cid: int, latest: dict) -> bytes:
    r = latest.get(cid)
    if cid not in COLOR_NAMES or r is None:
        return struct.pack(TARGET_FMT, cid, 0, 0, 0, 0, 0, 0xFFFF)
    age_ms = int((time.monotonic() - r["t"]) * 1000)
    found = 1 if age_ms <= MAX_AGE_MS else 0
    x, y, z = r["xyz_base_mm"]
    return struct.pack(TARGET_FMT, cid, found, clamp_i16(x), clamp_i16(y), clamp_i16(z),
                       r["conf"], min(age_ms, 0xFFFF))


# ------------------------------------------------------------- calibration --
def solve_rigid_transform(A: np.ndarray, B: np.ndarray) -> np.ndarray:
    """Kabsch: 4x4 T with B ~= R @ A + t. A, B are (N,3) in the same units."""
    ca, cb = A.mean(axis=0), B.mean(axis=0)
    H = (A - ca).T @ (B - cb)
    U, _, Vt = np.linalg.svd(H)
    R = Vt.T @ U.T
    if np.linalg.det(R) < 0:  # reflection fix
        Vt[-1] *= -1
        R = Vt.T @ U.T
    T = np.eye(4)
    T[:3, :3] = R
    T[:3, 3] = cb - R @ ca
    return T


def run_solve_calib(csv_path: str):
    pts = np.loadtxt(csv_path, delimiter=",", comments="#") / 1000.0  # mm -> m
    if pts.ndim != 2 or pts.shape[0] < 4 or pts.shape[1] != 6:
        sys.exit("Need >= 4 rows of xc,yc,zc,xb,yb,zb (mm)")
    A, B = pts[:, :3], pts[:, 3:]
    T = solve_rigid_transform(A, B)
    pred = (T[:3, :3] @ A.T).T + T[:3, 3]
    err_mm = np.linalg.norm(pred - B, axis=1) * 1000
    np.save(CALIB_FILE, T)
    print(f"Saved {CALIB_FILE}\n{np.array2string(T, precision=4)}")
    print("Per-point error (mm):", np.round(err_mm, 1))
    print(f"RMS error: {np.sqrt(np.mean(err_mm ** 2)):.1f} mm")


# ------------------------------------------------------------------ camera --
def setup_depth_sensor(ds, laser=None):
    """'High Density' preset + projector. laser: None/"auto" = full power, 0 = off, 1-360 = power."""
    lp = None if laser in (None, "auto") else int(laser)
    def preset():
        r = ds.get_option_range(rs.option.visual_preset)
        for i in range(int(r.min), int(r.max) + 1):
            if ds.get_option_value_description(rs.option.visual_preset, i) == "High Density":
                ds.set_option(rs.option.visual_preset, i)
                return

    steps = [  # each step separately: on USB 2 one can fail with "resource busy"
        (rs.option.visual_preset, preset),
        (rs.option.emitter_enabled, lambda: ds.set_option(
            rs.option.emitter_enabled, 0 if lp == 0 else 1)),
        (rs.option.laser_power, lambda: ds.set_option(
            rs.option.laser_power, ds.get_option_range(rs.option.laser_power).max
            if lp is None else float(lp))),
    ]
    for opt, fn in steps:
        if not ds.supports(opt):
            continue
        for attempt in range(3):
            try:
                fn()
                break
            except RuntimeError:
                time.sleep(0.2)


class BlockCamera:
    """Colour + depth, with the COLOUR image aligned onto the DEPTH image.

    Depth resolution is picked from the USB link:
      USB 3: depth 640x480, colour 640x480 @ 30 fps
      USB 2: depth 480x270, colour 424x240 @ 15 fps (falls back to 6 fps). Tested on a D435
             with a USB 2 cable: 640x480 depth gave no frames / 85% holes, 480x270 worked.
    Depth is aligned onto the colour image, so every colour pixel keeps its colour even where
    depth is missing (needed to find black blocks). Tiny alignment gaps are filled."""

    def __init__(self, fps=None, laser="auto"):
        if rs is None:
            sys.exit("pyrealsense2 not installed - see SETUP_PI.md")
        devs = rs.context().query_devices()
        if len(devs) == 0:
            sys.exit("No RealSense camera found. Check the cable and run: lsusb | grep 8086")
        dev = devs[0]
        self.usb = (dev.get_info(rs.camera_info.usb_type_descriptor)
                    if dev.supports(rs.camera_info.usb_type_descriptor) else "?")
        usb3 = self.usb.startswith("3")
        self.laser = laser
        setup_depth_sensor(dev.first_depth_sensor(), laser)  # before streaming: most reliable on USB 2
        self.depth_res = (640, 480) if usb3 else (480, 270)
        self.color_res = (640, 480) if usb3 else (424, 240)
        tries = [fps] if fps else ([30, 15] if usb3 else [15, 6])
        last_err = None
        for f in tries:
            try:
                self._start(f)
                self.fps = f
                break
            except RuntimeError as e:
                last_err = e
                try:
                    self.pipe.stop()
                except Exception:
                    pass
        else:
            sys.exit(f"Camera would not stream ({last_err}). Try another cable or USB port.")
        (w, h), (cw, ch) = self.depth_res, self.color_res
        print(f"Camera: USB {self.usb}, depth {w}x{h}, colour {cw}x{ch} @ {self.fps} fps")
        if self.laser == "auto":
            self._auto_laser()

    def _auto_laser(self):
        """Try the projector OFF and ON (full power) and keep whichever leaves fewer depth holes.
        Shiny / varnished tables: OFF usually wins (the dots glare). Plain matte tables: ON wins
        (no texture of their own). Needs the camera pointed at the work area when it starts."""
        try:
            ds = self.pipe.get_active_profile().get_device().first_depth_sensor()
            if not ds.supports(rs.option.laser_power):
                return
            top = int(ds.get_option_range(rs.option.laser_power).max)
            holes = {}
            for power in (0, top):
                setup_depth_sensor(ds, power)
                for _ in range(10):  # let the change take effect
                    self.pipe.wait_for_frames(3000)
                frames = [np.asanyarray(self.pipe.wait_for_frames(3000).get_depth_frame().get_data())
                          for _ in range(5)]
                holes[power] = float(np.mean([np.mean(f == 0) for f in frames]))
            best = 0 if holes[0] + 0.03 < holes[top] else top  # prefer ON unless OFF is clearly better
            setup_depth_sensor(ds, best)
            self.laser = best
            print(f"Laser auto: off {holes[0]:.0%} holes, on {holes[top]:.0%} holes -> "
                  f"{'OFF' if best == 0 else 'ON'}")
        except RuntimeError as e:
            print(f"Laser auto-select skipped ({e}); using full power")

    def _start(self, fps):
        self.pipe = rs.pipeline()
        cfg = rs.config()
        cfg.enable_stream(rs.stream.depth, *self.depth_res, rs.format.z16, fps)
        cfg.enable_stream(rs.stream.color, *self.color_res, rs.format.bgr8, fps)
        prof = self.pipe.start(cfg)
        ds = prof.get_device().first_depth_sensor()
        self.depth_scale = ds.get_depth_scale()
        setup_depth_sensor(ds, self.laser)  # again: some options only stick once streaming
        self.align = rs.align(rs.stream.color)  # depth pixels now match colour pixels
        self.intr = prof.get_stream(rs.stream.color).as_video_stream_profile().get_intrinsics()
        self.k3 = np.ones((3, 3), np.uint8)
        for _ in range(15):  # let auto-exposure settle; raises RuntimeError if no frames
            self.pipe.wait_for_frames(3000)

    def grab(self):
        try:
            frames = self.align.process(self.pipe.wait_for_frames(1000))
        except RuntimeError:
            return None, None  # camera timeout: detections age out -> found=0
        d, c = frames.get_depth_frame(), frames.get_color_frame()
        if not d or not c:
            return None, None
        depth = np.asanyarray(d.get_data())
        # aligning lower-res depth onto colour leaves 1-pixel gaps: fill them from neighbours
        # (real holes, like a black block, are much bigger and stay empty)
        depth = np.where(depth == 0, cv2.dilate(depth, self.k3), depth)
        return np.asanyarray(c.get_data()), depth

    def stop(self):
        self.pipe.stop()


# ----------------------------------------------------------------- geometry --
def fit_plane(pts, iters=150, thresh=0.004, rng=np.random.default_rng(0)):
    """RANSAC plane through (N,3) points in metres. Returns (n, d, inlier_ratio).
    Sign is chosen so n.p + d > 0 on the camera's side, i.e. it is height above table."""
    if len(pts) < 50:
        return None
    best, best_cnt = None, 0
    for _ in range(iters):
        s = pts[rng.choice(len(pts), 3, replace=False)]
        n = np.cross(s[1] - s[0], s[2] - s[0])
        norm = np.linalg.norm(n)
        if norm < 1e-9:
            continue
        n = n / norm
        d = -n @ s[0]
        cnt = np.count_nonzero(np.abs(pts @ n + d) < thresh)
        if cnt > best_cnt:
            best, best_cnt = (n, d), cnt
    if best is None:
        return None
    n, d = best
    inl = pts[np.abs(pts @ n + d) < thresh]
    c = inl.mean(axis=0)
    n = np.linalg.svd(inl - c)[2][2]  # least-squares refinement
    d = -n @ c
    if d < 0:
        n, d = -n, -d
    return n, d, best_cnt / len(pts)


def ray_to_height(ray, n, d, h_m):
    """Point on the ray through a pixel that is h_m above the table plane."""
    denom = n @ ray
    if abs(denom) < 1e-6:
        return None
    t = (h_m - d) / denom
    return ray * t if t > 0 else None


def estimate_top_centre(sil_rays, ray_c, n, d):
    """Top-face centre of a block with NO depth (e.g. black), from its silhouette.
    Each silhouette pixel is projected onto the plane 20 mm above the table. Top-face pixels
    land on the true 2 cm square; side-face pixels land nearer the camera. So the FAR edge
    of the projected silhouette is the top face's far edge, and the centre is 10 mm closer."""
    h_m = BLOCK_HEIGHT_MM / 1000.0
    denom = sil_rays @ n
    ok = np.abs(denom) > 1e-6
    t = (h_m - d) / denom[ok]
    pts = sil_rays[ok] * t[:, None]
    pts = pts[t > 0]
    if len(pts) < 5:
        return ray_to_height(ray_c, n, d, h_m)
    away = ray_c - (ray_c @ n) * n  # horizontal direction from camera to block
    if np.linalg.norm(away) < 1e-6:  # camera straight above: no side faces visible
        return np.median(pts, axis=0)
    away /= np.linalg.norm(away)
    side = np.cross(n, away)
    along = np.percentile(pts @ away, 95) - BLOCK_SIZE_MM / 2000.0
    lateral = np.median(pts @ side)
    q = pts[0]
    return q + (along - q @ away) * away + (lateral - q @ side) * side


# ------------------------------------------------------------------- colour --
def color_feature(block_rgb, table_rgb):
    """Block colour relative to the nearby table: cancels brightness and colour cast."""
    rel = np.maximum(block_rgb, 1.0) / np.maximum(table_rgb, 1.0)
    chroma = rel / rel.sum()
    return np.array([chroma[0], chroma[1], chroma[2], np.log(rel.mean())])


def load_refs():
    """Colour samples. Plain keys ("red") = blocks on the table; "red@station" = blocks on the station top
    (colour is judged relative to the surface under the block, so each surface has its own samples)."""
    refs = {k: [list(v) for v in vals] for k, vals in DEFAULT_REFS.items()}
    if os.path.exists(REFS_FILE):
        with open(REFS_FILE) as f:
            learned = {k: v for k, v in json.load(f).items() if v}
        refs.update(learned)  # colours not learned yet keep the built-in reference
        print("Colours: " + ", ".join(f"{k} {'learned x' + str(len(learned[k])) if k in learned else 'built-in'}"
                                      for k in COLOR_NAMES.values()))
        on_station = [f"{k} x{len(learned[k + '@station'])}" for k in COLOR_NAMES.values() if k + "@station" in learned]
        if on_station:
            print("Colours on the station: " + ", ".join(on_station) + " (others: built-in)")
    else:
        print(f"No {REFS_FILE} yet - using built-in colours. Learn them at the venue for best results.")
    return refs


def surface_refs(refs, surface):
    """The samples to classify with on one surface, keyed by plain colour name."""
    if surface == "station":  # the built-in references assume a pale surface, which the station top is
        return {k: refs.get(k + "@station") or DEFAULT_REFS[k] for k in COLOR_NAMES.values()}
    return {k: refs[k] for k in COLOR_NAMES.values() if refs.get(k)}


def refs_key(name, surface):
    return name if surface == "table" else f"{name}@{surface}"


def classify(feature, refs):
    """Nearest learned sample. Returns (name, confidence 0-1) or (None, 0)."""
    best = {}
    for name, samples in refs.items():
        if samples:
            dist = np.min(np.linalg.norm((np.array(samples) - feature) * FEATURE_WEIGHTS, axis=1))
            best[name] = dist
    if not best:
        return None, 0.0
    ranked = sorted(best.items(), key=lambda kv: kv[1])
    name, d1 = ranked[0]
    if d1 > MAX_COLOR_DIST:
        return None, 0.0
    d2 = ranked[1][1] if len(ranked) > 1 else MAX_COLOR_DIST * 2
    return name, float(np.clip((d2 - d1) / (d2 + 1e-6), 0.0, 1.0))  # margin to runner-up


# ----------------------------------------------------------------- detector --
class BlockDetector:
    def __init__(self, intr, depth_scale, refs):
        self.fx, self.fy, self.ppx, self.ppy = intr.fx, intr.fy, intr.ppx, intr.ppy
        self.scale = depth_scale
        self.refs = refs
        self.plane = None
        self.plane_note = ""
        self.frame_i = 0
        self.rays = None
        self.k3 = np.ones((3, 3), np.uint8)
        self.k5 = np.ones((5, 5), np.uint8)
        self.station_enabled = STATION_ENABLED
        self.station = None
        self.station_rejects = []

    def _rays(self, h, w):
        if self.rays is None or self.rays.shape[:2] != (h, w):
            u, v = np.meshgrid(np.arange(w, dtype=np.float32), np.arange(h, dtype=np.float32))
            self.rays = np.dstack([(u - self.ppx) / self.fx, (v - self.ppy) / self.fy,
                                   np.ones_like(u)])
        return self.rays

    def _update_plane(self, z, rays):
        sub = (slice(None, None, 4), slice(None, None, 4))
        zs, rs_ = z[sub], rays[sub]
        ok = (zs > DEPTH_MIN_M) & (zs < DEPTH_MAX_M)
        pts = rs_[ok] * zs[ok][:, None]
        if len(pts) > 4000:
            pts = pts[np.random.default_rng(self.frame_i).choice(len(pts), 4000, replace=False)]
        res = fit_plane(pts)
        if res is None:
            return
        below = self._plane_below(pts, res) if self.station_enabled else None
        if below is not None:  # the biggest flat thing was the station top: use the table under it
            self.plane = (below[0].astype(np.float32), float(below[1]))
            self.plane_note = "biggest plane was the station top; table = the plane below it"
        elif res[2] > 0.3:  # table must be >= 30% of the view
            self.plane = (res[0].astype(np.float32), float(res[1]))
            self.plane_note = ""

    @staticmethod
    def _plane_below(pts, res):
        """A parallel plane about one station height (+-30%) FARTHER from the camera, holding >= 10% of the
        points, means the biggest plane was the station top and this one is the table."""
        n, d, _ = res
        h = (pts @ n + d) * 1000.0
        H = STATION_SIZE_MM[2]
        sel = (h < -0.7 * H) & (h > -1.3 * H)
        if sel.mean() < STATION_PLANE_FRAC:
            return None
        low = fit_plane(pts[sel])
        if low is None or abs(low[0] @ n) < 0.98:
            return None
        return low

    # ------------------------------------------------------------------ station --
    def _project(self, P):
        """Camera-frame points (N,3) in m -> pixel coordinates (N,2) float."""
        return np.stack([self.fx * P[:, 0] / P[:, 2] + self.ppx, self.fy * P[:, 1] / P[:, 2] + self.ppy], axis=1)

    def _find_station(self, z, rays, valid, height_mm, n, d):
        """Returns (station dict or None, footprint mask, inner mask). Depth only: the top is a flat patch
        STATION_SIZE_MM[2] above the table, about STATION_SIZE_MM[0] x [1] when fitted with a rectangle."""
        self.station_rejects = []
        L, Wd, Hh = STATION_SIZE_MM
        h, w = z.shape
        with np.errstate(invalid="ignore"):
            band = valid & (height_mm >= STATION_BAND[0] * Hh) & (height_mm <= Hh + STATION_BAND[1])
        band = cv2.morphologyEx(band.astype(np.uint8), cv2.MORPH_OPEN, self.k3)
        # depth holes split a plain top into pieces: join pieces that are close together, then use only
        # the real top pixels of each joined group (the closing only decides which pieces belong together)
        k = cv2.getStructuringElement(cv2.MORPH_ELLIPSE, (STATION_JOIN_PX, STATION_JOIN_PX))
        joined = cv2.morphologyEx(band, cv2.MORPH_CLOSE, k)
        ncomp, labels, stats, _ = cv2.connectedComponentsWithStats(joined, connectivity=8)
        band = band > 0

        # orthonormal axes on the table plane: e1 = the camera's "right", e2 = away from the camera
        e1 = np.array([1.0, 0.0, 0.0]) - n[0] * n
        e1 /= np.linalg.norm(e1)
        e2 = np.cross(n, e1)
        best = None
        for i in range(1, ncomp):
            x0, y0, bw, bh, area = stats[i]
            if area < STATION_MIN_PX:
                continue
            sel = (labels == i) & band
            if sel.sum() < STATION_MIN_PX:
                continue
            # bounding box of the REAL top pixels: the closing can stretch a group up to the image border
            sy_, sx_ = np.nonzero(sel)
            x0, y0 = int(sx_.min()), int(sy_.min())
            bw, bh = int(sx_.max()) - x0 + 1, int(sy_.max()) - y0 + 1
            rect_px = (x0, y0, bw, bh)
            top_mm = float(np.nanmedian(height_mm[sel]))
            with np.errstate(invalid="ignore"):
                keep = sel & (np.abs(height_mm - top_mm) < STATION_FLAT_MM)
            hk = height_mm[keep]
            rough = 1.4826 * float(np.median(np.abs(hk - np.median(hk))))  # robust spread (MAD)
            if rough > STATION_MAX_ROUGH_MM:
                self.station_rejects.append((rect_px, f"top not flat (spread {rough:.1f} mm)"))
                continue
            # only the outline of the top can be a corner of its convex hull: use just those pixels
            cnts, _ = cv2.findContours(keep.astype(np.uint8), cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_NONE)
            edge = np.vstack([c_[:, 0, :] for c_ in cnts])  # (x, y)
            P = rays[edge[:, 1], edge[:, 0]] * z[edge[:, 1], edge[:, 0]][:, None]
            uv = (np.stack([P @ e1, P @ e2], axis=1) * 1000.0).astype(np.float32)  # mm on the table plane
            hull = cv2.convexHull(uv)
            rect = cv2.minAreaRect(hull)  # direction of the sides (same as from all points, much faster)
            fill = cv2.contourArea(hull) / max(1.0, rect[1][0] * rect[1][1])
            # the hull's extreme points follow depth noise outwards: measure the extent with percentiles
            # of ALL top points along the rectangle's sides instead (corrected for the 0.5 % cut each end)
            ang = np.radians(rect[2])
            a1, a2 = np.array([np.cos(ang), np.sin(ang)]), np.array([-np.sin(ang), np.cos(ang)])
            ys_, xs_ = np.nonzero(keep)
            if len(ys_) > 4000:
                pick = np.random.default_rng(0).choice(len(ys_), 4000, replace=False)
                ys_, xs_ = ys_[pick], xs_[pick]
            Pa = rays[ys_, xs_] * z[ys_, xs_][:, None]
            uva = np.stack([Pa @ e1, Pa @ e2], axis=1) * 1000.0
            lo1, hi1 = np.percentile(uva @ a1, [0.5, 99.5])
            lo2, hi2 = np.percentile(uva @ a2, [0.5, 99.5])
            pad1, pad2 = 0.005 * (hi1 - lo1) / 0.99, 0.005 * (hi2 - lo2) / 0.99
            lo1, hi1, lo2, hi2 = lo1 - pad1, hi1 + pad1, lo2 - pad2, hi2 + pad2
            # a depth hole can hide one END of the top. Too short a side means a wrong centre, so a side
            # must be within STATION_TRUST_MM of its real length, or be rebuilt from an edge that IS seen
            # (the other end clearly hidden by a hole); otherwise the station is not reported at all
            top_now = float(np.median(hk))
            long1 = (hi1 - lo1) >= (hi2 - lo2)
            hidden, rebuilt, short = 0, None, []
            for ax_, ot_, lims, olims, nom in ((a1, a2, [lo1, hi1], (lo2, hi2), L if long1 else Wd),
                                               (a2, a1, [lo2, hi2], (lo1, hi1), Wd if long1 else L)):
                ext = lims[1] - lims[0]
                if ext >= nom - STATION_TRUST_MM:
                    continue  # complete
                ends = ([self._edge_seen(ax_, ot_, lims[0], -1, olims, e1, e2, n, d, top_now, height_mm),
                         self._edge_seen(ax_, ot_, lims[1], +1, olims, e1, e2, n, d, top_now, height_mm)]
                        if ext >= STATION_MIN_SEEN * nom else [])
                if ends == ["edge", "hidden"]:
                    lims[1] = lims[0] + nom
                    hidden, rebuilt = hidden + 1, nom
                elif ends == ["hidden", "edge"]:
                    lims[0] = lims[1] - nom
                    hidden, rebuilt = hidden + 1, nom
                else:
                    short.append(f"{ext:.0f} of {nom:.0f} mm")
                if ax_ is a1:
                    lo1, hi1 = lims
                else:
                    lo2, hi2 = lims
            rw, rh = hi1 - lo1, hi2 - lo2
            if hidden:  # a rebuilt side is a guess: the other side and the shape must fit tightly
                other_nom = Wd if rebuilt == L else L
                other = (rh if long1 else rw) if rebuilt == L else (rw if long1 else rh)
                if hidden > 1 or abs(other - other_nom) > STATION_TRUST_MM or fill < 0.85:
                    short.append("a rebuilt side does not fit")
            if short and max(abs(max(rw, rh) - L) / L, abs(min(rw, rh) - Wd) / Wd) <= STATION_SIZE_TOL:
                self.station_rejects.append(((int(x0), int(y0), int(bw), int(bh)),
                                             "partly hidden by depth holes (" + ", ".join(short)
                                             + " seen): its centre would be wrong"))
                continue
            c_ = a1 * (lo1 + hi1) / 2 + a2 * (lo2 + hi2) / 2
            rect = ((float(c_[0]), float(c_[1])), (float(rw), float(rh)), rect[2])
            Lm, Wm = max(rw, rh), min(rw, rh)
            eL, eW = abs(Lm - L) / L, abs(Wm - Wd) / Wd
            at_edge = x0 <= 1 or y0 <= 1 or x0 + bw >= w - 1 or y0 + bh >= h - 1
            why = None
            if eL > STATION_SIZE_TOL or eW > STATION_SIZE_TOL:
                why = (f"wrong size ({Lm:.0f} x {Wm:.0f} mm, need {L:.0f} x {Wd:.0f} +-{STATION_SIZE_TOL:.0%})"
                       + (" - partly out of view?" if at_edge else ""))
            elif fill < STATION_MIN_FILL:
                why = f"not rectangular (fills {fill:.0%} of its rectangle)"
            elif at_edge:
                why = "touches the image edge: partly out of view, position would be wrong"
            if why:
                self.station_rejects.append((rect_px, why))
                continue
            score = 1.0 - (eL + eW) / (2 * STATION_SIZE_TOL)
            if best is None or score > best["score"]:
                best = dict(score=score, rect=rect, top_mm=float(np.median(hk)), fill=fill, rough=rough,
                            hidden=hidden)
        if best is None:
            return None, None, None

        (cx, cy), (rw, rh), _ = best["rect"]
        box = cv2.boxPoints(best["rect"])  # 4 corners in mm on the plane
        s01, s12 = box[1] - box[0], box[2] - box[1]
        ax = s01 if np.linalg.norm(s01) >= np.linalg.norm(s12) else s12
        ax = ax / np.linalg.norm(ax)  # long axis on the plane
        if ax[0] < 0:
            ax = -ax
        ay = np.array([-ax[1], ax[0]])
        top_m = best["top_mm"] / 1000.0

        def to3d(a_mm, b_mm, hgt_m):  # plane coords (mm) + height above table (m) -> camera frame (m)
            return (np.outer(np.asarray(a_mm) / 1000.0, e1) + np.outer(np.asarray(b_mm) / 1000.0, e2)
                    + (hgt_m - d) * n)

        centre = to3d([cx], [cy], top_m)[0]
        top3 = to3d(box[:, 0], box[:, 1], top_m)
        base3 = to3d(box[:, 0], box[:, 1], 0.0)
        hl, hw_ = L / 2 - STATION_INSET_MM, Wd / 2 - STATION_INSET_MM
        inner_mm = np.array([np.array([cx, cy]) + sx * hl * ax + sy * hw_ * ay
                             for sx, sy in ((-1, -1), (1, -1), (1, 1), (-1, 1))])
        inner3 = to3d(inner_mm[:, 0], inner_mm[:, 1], top_m)

        footprint = np.zeros((h, w), np.uint8)
        hull = cv2.convexHull(np.round(self._project(np.vstack([top3, base3]))).astype(np.int32))
        cv2.fillConvexPoly(footprint, hull, 1)
        footprint = cv2.dilate(footprint, self.k5, iterations=2) > 0
        inner = np.zeros((h, w), np.uint8)
        cv2.fillConvexPoly(inner, np.round(self._project(inner3)).astype(np.int32), 1)
        inner = inner > 0

        size_score = max(0.0, best["score"])
        station = {
            "centre_cam_m": centre, "corners_cam_m": top3, "base_corners_cam_m": base3,
            "size_mm": (float(max(rw, rh)), float(min(rw, rh))), "top_mm": best["top_mm"],
            "yaw": float(np.degrees(np.arctan2(ax[1], ax[0]))), "drop_cam_m": None, "drop_px": None,
            "n_blocks": 0, "conf": int(100 * (0.5 + 0.5 * size_score) * min(1.0, best["fill"] / 0.95)
                                       * 0.85 ** best["hidden"]),
            "flat_mm": best["rough"], "hidden_ends": best["hidden"],  # sides rebuilt from the known size
            "top_px": np.round(self._project(top3)).astype(np.int32),
            "_frame": (np.array([cx, cy]), ax, ay, e1, e2, n, d, top_m),  # for the drop spot
        }
        return station, footprint, inner

    def _edge_seen(self, axis, other, end_mm, sign, olims, e1, e2, n, d, top_mm, height_mm):
        """Look 12 mm past this end of the top, at top height. "edge": the table or the station's side shows
        there (lower than the top), so this is the real edge. "hidden": mostly no depth there, so the top may go
        on under a depth hole. "unclear": anything else."""
        h, w = height_mm.shape
        t = np.linspace(olims[0] + 10.0, olims[1] - 10.0, 7)
        q = np.outer(np.full(7, end_mm + sign * 12.0), axis) + np.outer(t, other)  # mm on the plane
        P = np.outer(q[:, 0] / 1000.0, e1) + np.outer(q[:, 1] / 1000.0, e2) + (top_mm / 1000.0 - d) * n
        px = np.round(self._project(P)).astype(int)
        ok = (px[:, 0] >= 0) & (px[:, 0] < w) & (px[:, 1] >= 0) & (px[:, 1] < h)
        hv = height_mm[px[ok, 1], px[ok, 0]]
        with np.errstate(invalid="ignore"):
            lower = int(np.sum(hv < top_mm - STATION_FLAT_MM))  # table, or the station's side wall
            same = int(np.sum(np.abs(hv - top_mm) < STATION_FLAT_MM))
        holes = int(np.sum(~np.isfinite(hv))) + int(np.sum(~ok))
        if lower >= 3 and same == 0:
            return "edge"
        if holes >= 4 and lower <= 1:
            return "hidden"
        return "unclear"

    def _drop_spot(self, station, st_blocks):
        """Centre of the top if it is free, else the nearest free point of a 5 x 3 grid; None if full."""
        c, ax, ay, e1, e2, n, d, top_m = station["_frame"]
        held = []
        for b in st_blocks:
            p = b["xyz_cam_m"] * 1000.0
            q = np.array([p @ e1, p @ e2]) - c
            held.append(np.array([q @ ax, q @ ay]))
        L, Wd, _ = STATION_SIZE_MM
        gx = np.linspace(-(L / 2 - DROP_INSET_MM), L / 2 - DROP_INSET_MM, DROP_GRID[0])
        gy = np.linspace(-(Wd / 2 - DROP_INSET_MM), Wd / 2 - DROP_INSET_MM, DROP_GRID[1])
        cands = [np.zeros(2)] + sorted((np.array([x, y]) for x in gx for y in gy), key=np.linalg.norm)
        for p in cands:
            if all(np.linalg.norm(p - q) >= DROP_CLEAR_MM for q in held):
                xy = (c + p[0] * ax + p[1] * ay) / 1000.0
                drop = xy[0] * e1 + xy[1] * e2 + (top_m - d) * n
                return drop, tuple(np.round(self._project(drop[None]))[0].astype(int))
        return None, None

    # ------------------------------------------------------------------- blocks --
    def detect(self, color_bgr, depth_raw, force_plane=False):
        """Returns a list of dicts, one per block found (any colour), each with "surface" = "table" or
        "station". The station (or None) is left in self.station."""
        h, w = depth_raw.shape
        rays = self._rays(h, w)
        z = depth_raw.astype(np.float32) * self.scale
        if self.plane is None or force_plane or self.frame_i % PLANE_REFIT_FRAMES == 0:
            self._update_plane(z, rays)
        self.frame_i += 1
        self.station = None
        if self.plane is None:
            return []
        n, d = self.plane

        if getattr(self, "_rn_plane", None) is not self.plane:  # cache rays.n per plane
            self._rn, self._rn_plane = (rays @ n).astype(np.float32), self.plane
        valid = (z > DEPTH_MIN_M) & (z < DEPTH_MAX_M)
        height_mm = np.where(valid, (z * self._rn + d) * 1000.0, np.nan)
        rgb = color_bgr[..., ::-1].astype(np.float32)
        gray = rgb.mean(axis=2)
        # depth sees wider than colour: the aligned colour image has black (0,0,0) borders
        has_col = color_bgr.any(axis=2)
        self.last_height_mm = height_mm  # for --snapshot debug picture

        footprint = inner = None
        if self.station_enabled:
            self.station, footprint, inner = self._find_station(z, rays, valid, height_mm, n, d)

        found = self._find_blocks(color_bgr, rgb, gray, has_col, z, rays, valid, height_mm, n, d,
                                  region=None if footprint is None else ~footprint, exclude=footprint,
                                  surface="table")
        self.last_saliency = self._sal  # table saliency for --snapshot
        rejects = self.rejects
        if self.station is not None and not inner.any():  # top too small in the image to search: no blocks
            self.station["drop_cam_m"], self.station["drop_px"] = self._drop_spot(self.station, [])
        elif self.station is not None:
            top_m = self.station["top_mm"] / 1000.0
            # the station top is the "table" for blocks lying on it
            st_px = inner & valid & has_col
            st_rgb = np.median(rgb[st_px], axis=0) if st_px.any() else None
            # blocks on the station can only be inside its top: search just that part of the image
            ys_, xs_ = np.nonzero(inner)
            pad = 10
            y0, y1 = max(0, ys_.min() - pad), min(h, ys_.max() + pad + 1)
            x0, x1 = max(0, xs_.min() - pad), min(w, xs_.max() + pad + 1)
            c = (slice(y0, y1), slice(x0, x1))
            with np.errstate(invalid="ignore"):
                h_st = height_mm[c] - self.station["top_mm"]
            st = self._find_blocks(color_bgr[c], rgb[c], gray[c], has_col[c], z[c], rays[c], valid[c], h_st,
                                   n, d - top_m, region=inner[c], exclude=None, surface="station", bg_rgb=st_rgb)
            for b in st:  # crop -> full-image pixel coordinates
                bx, by, bw, bh = b["rect"]
                b["rect"], b["px"] = (bx + x0, by + y0, bw, bh), (b["px"][0] + x0, b["px"][1] + y0)
            rejects += [((rx + x0, ry + y0, rw, rh), why) for (rx, ry, rw, rh), why in self.rejects]
            found += st
            self.station["n_blocks"] = len(st)
            self.station["drop_cam_m"], self.station["drop_px"] = self._drop_spot(self.station, st)
        self.rejects = rejects
        return found

    def _find_blocks(self, color_bgr, rgb, gray, has_col, z, rays, valid, height_mm, n, d,
                     region=None, exclude=None, surface="table", bg_rgb=None):
        """Blocks lying on one flat surface. height_mm = height above THAT surface, (n, d) = its plane.
        region: only look here (None = everywhere). exclude: pixels ignored in the ring around a block
        (the station, when searching the table). bg_rgb: surface colour for the saliency (None = local
        median blur, as on the table)."""
        h, w = height_mm.shape
        with np.errstate(invalid="ignore"):
            table = valid & (np.abs(height_mm) < TABLE_TOL_MM)
            raised = valid & (height_mm > HEIGHT_RANGE_MM[0]) & (height_mm < HEIGHT_RANGE_MM[1])
            top = valid & (height_mm > TOP_MIN_MM)

        table &= has_col
        raised &= has_col
        if region is not None:
            table &= region
            raised &= region
        tsub = table[::4, ::4]  # subsample: a median over the whole table is slow on a Pi
        table_rgb_global = np.median(rgb[::4, ::4][tsub], axis=0) if tsub.any() else np.full(3, 128.0)
        dark_hole = (~valid) & has_col & (gray < 0.45 * table_rgb_global.mean())  # black block, no depth

        self.rejects = []                # (rect, reason) for --snapshot debug output
        # COLOUR saliency: each pixel vs. the local table colour around it. Finds blocks that
        # depth cannot see (small / low / seen at a shallow angle); lighting cancels out.
        if bg_rgb is None:
            small = cv2.resize(color_bgr, (max(8, w // 4), max(8, h // 4)), interpolation=cv2.INTER_AREA)
            bg = cv2.resize(cv2.medianBlur(small, 21), (w, h), interpolation=cv2.INTER_LINEAR)[..., ::-1]
        else:  # station: against its own top colour (a median blur mixes in the wood near its edges)
            bg = np.asarray(bg_rgb, np.float32)
        rel = (rgb + 4.0) / (bg.astype(np.float32) + 4.0)
        chroma = rel / rel.sum(axis=2, keepdims=True)
        sal = 3.0 * np.linalg.norm(chroma - 1.0 / 3.0, axis=2) \
            + 0.6 * np.maximum(0.0, -np.log(rel.mean(axis=2)))  # darker than table counts, glare doesn't
        salient = has_col & (sal > SALIENCY_MIN)
        self._sal = sal

        mask = raised | dark_hole | salient
        if region is not None:
            mask &= region
        mask = cv2.morphologyEx(mask.astype(np.uint8), cv2.MORPH_OPEN, self.k3)
        mask = cv2.morphologyEx(mask, cv2.MORPH_CLOSE, self.k5)
        ncomp, labels, stats, cents = cv2.connectedComponentsWithStats(mask, connectivity=8)
        refs = surface_refs(self.refs, surface)

        found = []
        for i in range(1, ncomp):
            x0, y0, bw, bh, area = stats[i]
            rect = (int(x0), int(y0), int(bw), int(bh))
            if area < 30:
                if area >= 10:
                    self.rejects.append((rect, f"too few pixels ({area})"))
                continue
            pad = 8
            ys, xs = slice(max(0, y0 - pad), min(h, y0 + bh + pad)), slice(max(0, x0 - pad), min(w, x0 + bw + pad))
            comp = (labels[ys, xs] == i).astype(np.uint8)

            # must be sitting on the table: the ring around it should mostly be table
            ring = (cv2.dilate(comp, self.k5, iterations=2) > 0) & (comp == 0)
            if exclude is not None:  # the station beside a table block is neither table nor "taller"
                ring &= ~exclude[ys, xs]
            ring_valid = ring & valid[ys, xs]  # ignore depth holes in the ring (shiny tables)
            if ring_valid.sum() < 8 or table[ys, xs][ring_valid].mean() < MIN_TABLE_AROUND:
                frac = table[ys, xs][ring_valid].mean() if ring_valid.sum() else 0.0
                self.rejects.append((rect, f"not surrounded by {surface} ({frac:.0%} of ring is {surface})"))
                continue
            # reject the side wall of something TALLER (arm, hand, box): it touches high pixels
            with np.errstate(invalid="ignore"):
                if (height_mm[ys, xs][ring] > HEIGHT_RANGE_MM[1]).mean() > 0.05:
                    self.rejects.append((rect, "next to something taller than 35 mm"))
                    continue

            # a bump in the depth with NO colour difference is depth noise (plain / shiny tables),
            # unless it is clearly as tall as a block
            comp_b = comp > 0
            sal_mean = float(np.mean(sal[ys, xs][comp_b]))
            with np.errstate(invalid="ignore"):
                hv = height_mm[ys, xs][comp_b]
                h_med = float(np.nanmedian(hv)) if np.isfinite(hv).any() else float("nan")
            if sal_mean < SALIENCY_MIN and not (h_med >= 0.6 * BLOCK_HEIGHT_MM):
                self.rejects.append((rect, f"depth bump without colour ({h_med:.0f} mm high) - noise"))
                continue

            cu, cv_ = cents[i]
            ray_c = rays[int(round(cv_)), int(round(cu))]
            comp_b = comp > 0
            top_px = comp_b & top[ys, xs]
            if top_px.sum() >= 5:  # normal case: real depth on the top face
                pts = rays[ys, xs][top_px] * z[ys, xs][top_px][:, None]
                xyz = np.median(pts, axis=0)
                depth_ok = True
            else:  # no depth (black block): estimate from the surface plane
                xyz = estimate_top_centre(rays[ys, xs][comp_b], ray_c, n, d)
                depth_ok = False
                if xyz is None:
                    continue
            size_mm = np.sqrt(area) * xyz[2] / self.fx * 1000.0
            if not (SIZE_RANGE_MM[0] <= size_mm <= SIZE_RANGE_MM[1]):
                self.rejects.append((rect, f"wrong size ({size_mm:.0f} mm, need "
                                           f"{SIZE_RANGE_MM[0]:.0f}-{SIZE_RANGE_MM[1]:.0f})"))
                continue

            # colour: inner pixels of the block vs. the table ring around it
            inner = cv2.erode(comp, self.k3) > 0
            if inner.sum() < 5:
                inner = comp_b
            patch = rgb[ys, xs]
            table_ring = ring & table[ys, xs]
            table_rgb = np.median(patch[table_ring], axis=0) if table_ring.sum() >= 10 else table_rgb_global
            feat = color_feature(np.median(patch[inner], axis=0), table_rgb)
            name, color_conf = classify(feat, refs)

            ref = 1.15 * BLOCK_SIZE_MM  # apparent size incl. some side face
            size_score = 1.0 - min(1.0, abs(size_mm - ref) / ref)
            conf = int(100 * color_conf * (0.6 + 0.4 * size_score) * (1.0 if depth_ok else 0.8))
            found.append({
                "name": name, "conf": conf, "feature": feat, "xyz_cam_m": xyz,
                "px": (int(cu), int(cv_)), "rect": (int(x0), int(y0), int(bw), int(bh)),
                "size_mm": float(size_mm), "depth_ok": depth_ok, "sal": sal_mean, "surface": surface,
            })
        return found


def best_per_color(blocks, T, now):
    """Keep the most confident block of each colour, in robot-base mm."""
    out = {}
    for b in blocks:
        if b["name"] is None:
            continue
        cid = next(k for k, v in COLOR_NAMES.items() if v == b["name"])
        if cid in out and out[cid]["conf"] >= b["conf"]:
            continue
        xyz = b["xyz_cam_m"]
        out[cid] = dict(b, t=now, xyz_cam_mm=xyz * 1000.0,
                        xyz_base_mm=(T[:3, :3] @ xyz + T[:3, 3]) * 1000.0)
    return out


def enlarge(img):
    """480x270 is small on screen: show it twice as big."""
    return cv2.resize(img, None, fx=2, fy=2, interpolation=cv2.INTER_NEAREST) if img.shape[1] < 600 else img


def draw(img, blocks, T, status=""):
    for b in blocks:
        x, y, w, h = b["rect"]
        name = b["name"] or "?"
        col = DRAW_BGR.get(name, (255, 255, 255))
        bx, by, bz = (T[:3, :3] @ b["xyz_cam_m"] + T[:3, 3]) * 1000.0
        cv2.rectangle(img, (x - 2, y - 2), (x + w + 2, y + h + 2), (255, 255, 255), 3)
        cv2.rectangle(img, (x - 2, y - 2), (x + w + 2, y + h + 2), col, 1)
        label = (f"{name} c{b['conf']} {bx:.0f},{by:.0f},{bz:.0f}" + ("" if b["depth_ok"] else " (est)")
                 + (" ST" if b.get("surface") == "station" else ""))
        cv2.putText(img, label, (x, max(15, y - 8)), cv2.FONT_HERSHEY_SIMPLEX, 0.45, (0, 0, 0), 3)
        cv2.putText(img, label, (x, max(15, y - 8)), cv2.FONT_HERSHEY_SIMPLEX, 0.45, (255, 255, 255), 1)
    if status:
        cv2.putText(img, status, (8, img.shape[0] - 10), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 0, 0), 3)
        cv2.putText(img, status, (8, img.shape[0] - 10), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 255, 255), 1)
    return img


def draw_station(img, det, T=None):
    """Station outline (magenta) and its drop spot (+)."""
    s = getattr(det, "station", None)
    if s is None:
        return img
    cv2.polylines(img, [s["top_px"].reshape(-1, 1, 2)], True, (255, 255, 255), 3)
    cv2.polylines(img, [s["top_px"].reshape(-1, 1, 2)], True, (255, 0, 255), 1)
    if s["drop_px"] is not None:
        u, v = s["drop_px"]
        for col, th in (((255, 255, 255), 3), ((255, 0, 255), 1)):
            cv2.line(img, (u - 6, v), (u + 6, v), col, th)
            cv2.line(img, (u, v - 6), (u, v + 6), col, th)
    x, y = s["top_px"].min(axis=0)
    label = f"station c{s['conf']} top {s['top_mm']:.0f}mm" + ("" if s["drop_px"] is not None else " FULL")
    cv2.putText(img, label, (int(x), max(15, int(y) - 6)), cv2.FONT_HERSHEY_SIMPLEX, 0.45, (0, 0, 0), 3)
    cv2.putText(img, label, (int(x), max(15, int(y) - 6)), cv2.FONT_HERSHEY_SIMPLEX, 0.45, (255, 0, 255), 1)
    return img


def station_line(det, T):
    """One text line for the station, in the frame being sent (T = camera -> base)."""
    s = getattr(det, "station", None)
    if s is None:  # the biggest rejected candidate is the one most likely to be the station
        rej = sorted(getattr(det, "station_rejects", []), key=lambda r: -r[0][2] * r[0][3])
        return "station: none" + (f" ({rej[0][1]})" if rej else "")
    to_mm = lambda p: (T[:3, :3] @ p + T[:3, 3]) * 1000.0  # noqa: E731
    cx, cy, cz = to_mm(s["centre_cam_m"])
    drop = ("full" if s["drop_cam_m"] is None else "drop %.0f %.0f %.0f mm" % tuple(to_mm(s["drop_cam_m"])))
    return (f"station: conf {s['conf']}  size {s['size_mm'][0]:.0f} x {s['size_mm'][1]:.0f} mm  "
            f"top {s['top_mm']:.0f} mm  yaw {s['yaw']:.0f} deg  centre {cx:.0f} {cy:.0f} {cz:.0f} mm  "
            f"{s['n_blocks']} block(s) on it  {drop}")


# ------------------------------------------------------------------- learn --
def run_learn(cam, det, T):
    """Place ONE block, press its letter. Repeat for each colour (2-3 spots each is better)."""
    refs = {name: [] for name in COLOR_NAMES.values()}
    if os.path.exists(REFS_FILE):
        with open(REFS_FILE) as f:
            refs.update(json.load(f))
    print("LEARN MODE: put ONE block in view, press r / g / b / k (black) to record it.\n"
          "Record each colour 2-3 times in different spots. s = save, x = clear all, q = quit.")
    while True:
        color, depth = cam.grab()
        if color is None:
            continue
        blocks = det.detect(color, depth)
        counts = " ".join(f"{k[0]}:{len(v)}" for k, v in refs.items())
        cv2.imshow("learn", enlarge(draw(color.copy(), blocks, T,
                                 f"{len(blocks)} block(s) | samples {counts} | r g b k s x q")))
        key = cv2.waitKey(1) & 0xFF
        if key in COLOR_KEYS:
            if len(blocks) != 1:
                print(f"Need exactly ONE block in view (seeing {len(blocks)}).")
                continue
            name = refs_key(COLOR_KEYS[key], blocks[0].get("surface", "table"))
            refs.setdefault(name, []).append([round(float(v), 4) for v in blocks[0]["feature"]])
            det.refs = {k: v for k, v in refs.items() if v} or det.refs
            print(f"Recorded {name} ({len(refs[name])} samples): {refs[name][-1]}")
        elif key == ord("x"):
            refs = {name: [] for name in COLOR_NAMES.values()}
            print("Cleared all samples.")
        elif key == ord("s"):
            missing = [k for k in COLOR_NAMES.values() if not refs.get(k)]
            with open(REFS_FILE, "w") as f:
                json.dump(refs, f, indent=1)
            print(f"Saved {REFS_FILE}." + (f" WARNING: no samples for {missing}" if missing else ""))
        elif key == ord("q"):
            break
    cv2.destroyAllWindows()


def run_learn_color(cam, det, name):
    """No-screen learning (works over SSH): put ONE block of this colour in view and run
    python pi_vision_uart.py --learn-color red   (green / blue / black)
    Run it 2-3 times per colour with the block in different spots.
    A block lying on the station is saved as "<colour>@station" (its colour is judged against the
    station top, not the table), so teach the station colours with the block on the station."""
    if name not in COLOR_NAMES.values():
        sys.exit(f"Colour must be one of: {', '.join(COLOR_NAMES.values())}")
    feats, seen, surfaces = [], [], []
    for _ in range(40):
        color, depth = cam.grab()
        if color is None:
            continue
        blocks = det.detect(color, depth)
        seen.append(len(blocks))
        if len(blocks) == 1:
            feats.append(blocks[0]["feature"])
            surfaces.append(blocks[0].get("surface", "table"))
        elif len(blocks) > 1:  # several candidates: take the clearly most colourful one
            ranked = sorted(blocks, key=lambda b: b["sal"], reverse=True)
            if ranked[0]["sal"] >= 1.5 * ranked[1]["sal"]:
                feats.append(ranked[0]["feature"])
                surfaces.append(ranked[0].get("surface", "table"))
        if len(feats) >= 10:
            break
    if len(feats) < 5:
        sys.exit(f"Could not see exactly ONE block (blocks seen per frame: {seen[-10:]}). "
                 "Put one block 0.3-0.45 m from the camera on the table and try again. "
                 "Run --snapshot to see what the camera detects.")
    surface = max(set(surfaces), key=surfaces.count)
    key = refs_key(name, surface)
    refs = {n: [] for n in COLOR_NAMES.values()}
    if os.path.exists(REFS_FILE):
        with open(REFS_FILE) as f:
            refs.update(json.load(f))
    refs.setdefault(key, []).append([round(float(v), 4) for v in np.median(feats, axis=0)])
    with open(REFS_FILE, "w") as f:
        json.dump(refs, f, indent=1)
    print(f"Learned {key} (block on the {surface}, now {len(refs[key])} sample(s)). Saved {REFS_FILE}.")
    missing = [n for n in COLOR_NAMES.values() if not refs.get(n)]
    if missing:
        print(f"Still to learn: {', '.join(missing)}")


def run_snapshot(cam, det, T, path="blocks.jpg"):
    """No-screen check (works over SSH): saves blocks.jpg = labelled colour | depth."""
    blocks, color, depth = [], None, None
    for _ in range(20):  # a few frames so exposure and the table plane settle
        c, d = cam.grab()
        if c is not None:
            color, depth = c, d
            blocks = det.detect(color, depth)
    if color is None:
        sys.exit("No frames from the camera.")
    status = f"{len(blocks)} block(s)" + ("" if det.plane is not None else " | NO TABLE PLANE")
    left = draw_station(draw(color.copy(), blocks, T, status), det, T)
    for (x, y, w, h), _ in getattr(det, "rejects", []):  # rejected candidates: thin grey boxes
        cv2.rectangle(left, (x - 1, y - 1), (x + w + 1, y + h + 1), (160, 160, 160), 1)
    dvis = cv2.applyColorMap(cv2.convertScaleAbs(depth, alpha=255.0 / 1500), cv2.COLORMAP_JET)
    dvis[depth == 0] = 0
    panels = [left, cv2.resize(dvis, (left.shape[1], left.shape[0]), interpolation=cv2.INTER_NEAREST)]
    hm = getattr(det, "last_height_mm", None)
    if hm is not None:  # height above table: black = no depth, dark blue = table, red = 40 mm+
        hv = cv2.applyColorMap(np.clip(np.nan_to_num(hm, nan=0) * 255 / 40, 0, 255).astype(np.uint8),
                               cv2.COLORMAP_JET)
        hv[np.isnan(hm)] = 0
        cv2.putText(hv, "height 0-40mm", (5, 15), cv2.FONT_HERSHEY_SIMPLEX, 0.45, (255, 255, 255), 1)
        panels.append(hv)
    sal = getattr(det, "last_saliency", None)
    if sal is not None:  # how different from the table each pixel looks: blocks should be red/yellow
        sv = cv2.applyColorMap(np.clip(sal * 255 / 1.5, 0, 255).astype(np.uint8), cv2.COLORMAP_JET)
        cv2.putText(sv, "colour vs table", (5, 15), cv2.FONT_HERSHEY_SIMPLEX, 0.45, (255, 255, 255), 1)
        panels.append(sv)
    cv2.imwrite(path, enlarge(np.hstack(panels)))
    if det.plane is not None and hm is not None:
        with np.errstate(invalid="ignore"):
            print(f"Table: {np.mean(np.abs(hm) < TABLE_TOL_MM):.0%} of pixels on the table plane, "
                  f"{np.mean(hm > HEIGHT_RANGE_MM[0]):.0%} raised above 8 mm, "
                  f"{np.mean(np.isnan(hm)):.0%} no depth")
    for (x, y, w, h), why in getattr(det, "rejects", []):
        print(f"  rejected at x={x} y={y} ({w}x{h} px): {why}")
    if det.plane_note:
        print(f"Table plane: {det.plane_note}")
    if det.station_enabled:
        for (x, y, w, h), why in det.station_rejects:
            print(f"  not the station at x={x} y={y} ({w}x{h} px): {why}")
        print(station_line(det, T))
    print(status if det.plane is not None else
          "NO TABLE PLANE found: aim the camera so the table fills at least a third of the view.")
    for b in blocks:
        bx, by, bz = (T[:3, :3] @ b["xyz_cam_m"] + T[:3, 3]) * 1000.0
        print(f"  {(b['name'] or '?'):>6}  conf {b['conf']:3d}  size {b['size_mm']:4.0f} mm  "
              f"xyz {bx:6.0f} {by:6.0f} {bz:6.0f} mm  on {b.get('surface', 'table')}"
              + ("" if b["depth_ok"] else "  (estimated)"))
    print(f"Saved {path}")


# -------------------------------------------------------------------- main --
def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--port", default="/dev/serial0")
    ap.add_argument("--baud", type=int, default=115200)
    ap.add_argument("--block-size", type=float, default=None, help=f"block width in mm (default {BLOCK_SIZE_MM:.0f})")
    ap.add_argument("--block-height", type=float, default=None, help=f"block height in mm (default {BLOCK_HEIGHT_MM:.0f})")
    ap.add_argument("--laser", type=laser_arg, default="auto",
                    help="projector: auto (default, picks on/off by itself), 0 = off, 1-360 = power")
    ap.add_argument("--fps", type=int, default=None,
                    help="override frame rate (default: 30 on USB 3, 15 then 6 on USB 2)")
    ap.add_argument("--show", action="store_true", help="preview window")
    ap.add_argument("--learn", action="store_true", help="learn block colours (needs a screen)")
    ap.add_argument("--learn-color", metavar="NAME", help="learn one colour over SSH: red/green/blue/black")
    ap.add_argument("--snapshot", action="store_true", help="save blocks.jpg with detections (no screen needed)")
    ap.add_argument("--print-cam", action="store_true", help="print camera XYZ (mm) for calibration")
    ap.add_argument("--solve-calib", metavar="CSV", help="solve calib_T.npy from point pairs")
    ap.add_argument("--no-uart", action="store_true", help="camera only, no STM32 connected")
    ap.add_argument("--station-size", type=float, nargs=3, metavar=("L", "W", "H"), default=None,
                    help="station length, width, height in mm (default %.0f %.0f %.0f)" % tuple(STATION_SIZE_MM))
    ap.add_argument("--no-station", action="store_true", help="do not look for the station")
    args = ap.parse_args()

    if args.solve_calib:
        run_solve_calib(args.solve_calib)
        return
    configure_block(args.block_size, args.block_height)
    configure_station(args.station_size, enabled=not args.no_station)
    print(f"Block: {BLOCK_SIZE_MM:.0f} mm wide, {BLOCK_HEIGHT_MM:.0f} mm tall")
    print("Station: " + ("off (--no-station)" if not STATION_ENABLED else
                         "%.0f x %.0f mm, %.0f mm tall" % tuple(STATION_SIZE_MM)))

    if os.path.exists(CALIB_FILE):
        T = np.load(CALIB_FILE)
    else:
        T = np.eye(4)
        print(f"WARNING: {CALIB_FILE} not found - using CAMERA-frame XYZ, not robot-base XYZ")

    cam = BlockCamera(fps=args.fps, laser=args.laser)
    det = BlockDetector(cam.intr, cam.depth_scale, load_refs())

    if args.learn or args.learn_color or args.snapshot:
        try:
            if args.learn:
                run_learn(cam, det, T)
            elif args.learn_color:
                run_learn_color(cam, det, args.learn_color.lower())
            else:
                run_snapshot(cam, det, T)
        finally:
            cam.stop()
        return

    ser = None
    if not args.no_uart:
        if serial is None:
            sys.exit("pyserial not installed: uv pip install pyserial")
        ser = serial.Serial(args.port, args.baud, timeout=0)
    parser = FrameParser()
    latest = {}
    last_print = 0.0
    print("Running camera only (no UART)." if ser is None
          else f"Running on {args.port} @ {args.baud}.", "Ctrl+C to stop.")

    try:
        while True:
            color, depth = cam.grab()
            if color is not None:
                blocks = det.detect(color, depth)
                latest.update(best_per_color(blocks, T, time.monotonic()))
                if args.print_cam and time.monotonic() - last_print > 0.5:
                    last_print = time.monotonic()
                    for b in blocks:
                        xc, yc, zc = b["xyz_cam_m"] * 1000.0
                        print(f"{(b['name'] or '?'):>6}: cam {xc:7.1f},{yc:7.1f},{zc:7.1f} mm"
                              f"  size {b['size_mm']:.0f} mm  conf {b['conf']}")
                if args.show:
                    status = "table OK" if det.plane is not None else "NO TABLE PLANE - aim camera at table"
                    cv2.imshow("blocks", enlarge(draw(color.copy(), blocks, T, status)))
                    if cv2.waitKey(1) & 0xFF == ord("q"):
                        break

            if ser is None:
                continue
            for msg_type, seq, payload in parser.feed(ser.read(ser.in_waiting or 0)):
                if msg_type == MSG_PING:
                    ser.write(build_frame(MSG_PONG, seq))
                elif msg_type == MSG_REQ_TARGET and len(payload) == 1:
                    ser.write(build_frame(MSG_TARGET, seq, target_payload(payload[0], latest)))
                else:
                    ser.write(build_frame(MSG_NACK, seq, bytes([msg_type])))
    except KeyboardInterrupt:
        pass
    finally:
        cam.stop()
        if ser is not None:
            ser.close()
        if args.show:
            cv2.destroyAllWindows()


if __name__ == "__main__":
    main()
