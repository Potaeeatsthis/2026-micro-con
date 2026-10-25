#!/usr/bin/env python3
"""pi_hand.py - hand detection for pi_bridge.py: palm centre XYZ from the D435 colour + depth frames.

Used by pi_bridge.py to answer `FIND hand` with `OBJ hand <x> <y> <z>` (or `NOTFOUND hand`).
The point is the PALM CENTRE on the palm surface the camera sees: the mean of the wrist and the four
finger-base landmarks (MediaPipe 0, 5, 9, 13, 17), each with its own depth. Units and frame are the
same as the blocks (camera mm here; pi_bridge.py applies calib_T.npy).

Two halves, two processes:
  - worker (`pi_hand.py --worker`, run with ~/hand-env/bin/python): MediaPipe only. Reads RGB frames on
    stdin, writes the 21 hand landmarks as one JSON line per frame on stdout.
  - bridge side (HandSource, inside pi_bridge.py, ~/arm-env): sends the newest colour frame at most
    --hand-hz times a second, turns the palm landmarks + the depth image into XYZ (palm_xyz).
Why a separate process: MediaPipe is native code. On the Pi 4 the newest MediaPipe (1.1.1) dies with
"Illegal instruction", which would kill the whole bridge if it ran inside it. Now a dead or stuck
worker only means `hand off`: FIND hand gets NOTFOUND, the blocks are not affected. MediaPipe and its
numpy / OpenCV live in ~/hand-env, so ~/arm-env (the block detector's venv) is never changed.

Pi setup (once): ~/.local/bin/uv venv ~/hand-env --python 3.12
                 ~/.local/bin/uv pip install --python ~/hand-env/bin/python "mediapipe==0.10.18"
0.10.18 has the classic mp.solutions.hands (no model file). A version without it uses the Tasks API
and the model file --hand-model (hand_landmarker.task).

Score: MediaPipe's handedness confidence (left / right), usually > 90 whenever a hand is found, so it is a
weak filter. The real quality gate is the depth check in palm_xyz().

Debug pictures (the bridge must be stopped, it holds the camera):
    sudo systemctl stop pi_bridge
    ~/arm-env/bin/python ~/arm/tools/pi_bridge.py --hand-snapshot      # hand only: ~/arm/hand.jpg
    ~/arm-env/bin/python ~/arm/tools/pi_bridge.py --snapshot           # blocks + hand + depth: ~/arm/snapshot.jpg
    sudo systemctl start pi_bridge
"""
import json
import os
import struct
import subprocess
import sys
import threading
import time

PALM_IDS = (0, 5, 9, 13, 17)   # wrist + finger bases (MediaPipe hand landmark numbers)
DEPTH_WIN = 2                  # depth = median of the non-zero pixels in a (2*2+1)^2 window
Z_RANGE_MM = (150.0, 1500.0)   # D435 depth that makes sense for a hand in the work area
Z_TOL_MM = 40.0                # palm points further than this from the median depth are dropped
MIN_POINTS = 3                 # palm points with good depth needed for a position
HEADER = struct.Struct("<III")  # frame on the worker's stdin: height, width, 3, then h*w*3 RGB bytes
WORKER_START_S = 60.0          # first MediaPipe start on a Pi 4 is slow
WORKER_RETRY_S = 30.0          # wait before starting a dead worker again


def palm_xyz(points_px, depth, depth_scale, deproject):
    """Palm centre in camera mm from the palm landmark pixels.
    points_px: [(u, v), ...] pixel positions; depth: 2-D array (raw units, 0 = no depth);
    deproject(u, v, z_m) -> (x, y, z) in metres. Returns ((x, y, z) mm or None, short reason)."""
    import numpy as np
    h, w = depth.shape
    pts = []
    for u, v in points_px:
        ui, vi = int(round(u)), int(round(v))
        if not (0 <= ui < w and 0 <= vi < h):
            continue
        win = depth[max(vi - DEPTH_WIN, 0):vi + DEPTH_WIN + 1, max(ui - DEPTH_WIN, 0):ui + DEPTH_WIN + 1]
        good = win[win > 0]
        if good.size < 3:
            continue
        z_m = float(np.median(good)) * depth_scale
        pts.append([c * 1000.0 for c in deproject(u, v, z_m)])
    if len(pts) < MIN_POINTS:
        return None, f"depth on {len(pts)} of {len(points_px)} palm points"
    pts = np.array(pts)
    zmed = np.median(pts[:, 2])
    pts = pts[np.abs(pts[:, 2] - zmed) <= Z_TOL_MM]
    if len(pts) < MIN_POINTS:
        return None, "palm depth too uneven"
    x, y, z = (float(c) for c in pts.mean(axis=0))
    if not Z_RANGE_MM[0] <= z <= Z_RANGE_MM[1]:
        return None, f"z {z:.0f} mm out of range"
    return (x, y, z), f"{len(pts)} pts"


def hand_result(msg, w, h, depth, depth_scale, deproject):
    """Worker reply for one frame -> dict(xyz_cam_mm, score, px, info); xyz_cam_mm None = no position."""
    if not msg.get("lms"):
        return {"xyz_cam_mm": None, "info": "no hand"}
    px = [(x * w, y * h) for x, y in msg["lms"]]
    xyz, info = palm_xyz([px[i] for i in PALM_IDS], depth, depth_scale, deproject)
    return {"xyz_cam_mm": xyz, "score": int(round(msg["score"] * 100)), "px": px, "info": info}


class Worker:
    """The worker process: frame in, landmarks out. Any failure raises; the caller decides what to do."""

    def __init__(self, cmd):
        self.p = subprocess.Popen(cmd, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.DEVNULL)
        self.kind = self._read(WORKER_START_S).get("ready")
        if not self.kind:
            raise RuntimeError("worker did not start")

    def _read(self, timeout_s):
        line = [None]
        t = threading.Thread(target=lambda: line.__setitem__(0, self.p.stdout.readline()), daemon=True)
        t.start()
        t.join(timeout_s)
        if t.is_alive():
            raise RuntimeError(f"worker silent for {timeout_s:.0f} s")
        if not line[0]:
            raise RuntimeError(f"worker exited ({self.p.wait()})")
        msg = json.loads(line[0])
        if "error" in msg:
            raise RuntimeError(msg["error"])
        return msg

    def detect(self, rgb, timeout_s=5.0):
        h, w = rgb.shape[:2]
        try:
            self.p.stdin.write(HEADER.pack(h, w, 3) + rgb.tobytes())
            self.p.stdin.flush()
        except (BrokenPipeError, OSError):
            raise RuntimeError(f"worker exited ({self.p.wait()})") from None
        return self._read(timeout_s)

    def close(self):
        try:
            self.p.stdin.close()
            self.p.wait(timeout=2.0)
        except Exception:  # noqa: BLE001
            self.p.kill()


class HandSource:
    """Bridge side: newest frame -> worker -> palm XYZ, in a background thread."""

    def __init__(self, cmd, depth_scale, deproject, hz, say):
        self.cmd, self.depth_scale, self.deproject, self.period, self.say = cmd, depth_scale, deproject, 1.0 / hz, say
        self.latest, self.frame, self.lock = None, None, threading.Lock()
        self.state, self.runs, self.stop = "starting", 0, False
        self.thread = threading.Thread(target=self._run, daemon=True)
        self.thread.start()

    def offer(self, color, depth, t):
        """Called by the camera thread for every frame. The colour array points into librealsense's
        frame memory, so it is copied (holding the frame could starve the camera); depth is already a copy."""
        color = color.copy()
        with self.lock:
            self.frame = (color, depth, t)

    def _run(self):
        while not self.stop:
            try:
                w = Worker(self.cmd)
            except Exception as e:  # noqa: BLE001 - no worker: hand off, blocks keep working
                self._down(f"off ({str(e)[:80]})")
                continue
            self.state = "on"
            self.say(f"hand: on, {w.kind}, at most {1.0 / self.period:.0f} per s")
            try:
                while not self.stop:
                    t_next = time.monotonic() + self.period
                    with self.lock:
                        f, self.frame = self.frame, None
                    if f is not None:
                        color, depth, t = f
                        msg = w.detect(color[:, :, ::-1].copy())  # BGR -> RGB
                        r = hand_result(msg, color.shape[1], color.shape[0], depth, self.depth_scale, self.deproject)
                        r["t"] = t
                        self.runs += 1
                        with self.lock:
                            self.latest = r
                    time.sleep(max(0.0, t_next - time.monotonic()))
            except Exception as e:  # noqa: BLE001 - a hand failure must be visible but never stop the blocks
                self._down(f"stopped ({str(e)[:80]})")
            finally:
                w.close()

    def _down(self, state):
        if state != self.state:  # log once per new reason, not every retry
            self.say(f"hand: {state}; FIND hand gets NOTFOUND, retrying every {WORKER_RETRY_S:.0f} s")
        self.state = state
        with self.lock:
            self.latest = None
        t_end = time.monotonic() + WORKER_RETRY_S
        while not self.stop and time.monotonic() < t_end:
            time.sleep(0.2)

    def get(self):
        with self.lock:
            return self.latest

    def close(self):
        self.stop = True
        self.thread.join(timeout=3.0)


def worker_cmd(python, model):
    return [os.path.expanduser(python), os.path.abspath(__file__), "--worker", "--model", os.path.expanduser(model)]


def draw(color_bgr, r, text_top=False):
    """Colour image x2 with the landmarks, the palm points and the result written on it
    (bottom left, or top left with text_top so it does not cover another status line)."""
    import cv2
    img = cv2.resize(color_bgr, None, fx=2, fy=2, interpolation=cv2.INTER_NEAREST)
    for i, (u, v) in enumerate(r.get("px") or []):
        cv2.circle(img, (int(u * 2), int(v * 2)), 6 if i in PALM_IDS else 3,
                   (0, 255, 255) if i in PALM_IDS else (255, 255, 255), -1)
    xyz = r.get("xyz_cam_mm")
    text = (f"hand c{r.get('score', 0)} cam {' '.join(str(int(round(c))) for c in xyz)} mm" if xyz
            else f"NO HAND POSITION: {r.get('info', '')}")
    y = 28 if text_top else img.shape[0] - 12
    cv2.putText(img, text, (10, y), cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 0, 0), 4)
    cv2.putText(img, text, (10, y), cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 255, 255), 2)
    return img


# ------------------------------------------------------------------ worker (runs in ~/hand-env) --
def _landmarker(model_path):
    """-> (detect(rgb) -> (lms, score) or None, description)."""
    import mediapipe as mp
    if hasattr(mp, "solutions") and hasattr(mp.solutions, "hands"):
        hands = mp.solutions.hands.Hands(static_image_mode=False, max_num_hands=1, model_complexity=0,
                                         min_detection_confidence=0.5, min_tracking_confidence=0.5)

        def detect(rgb):
            r = hands.process(rgb)
            if not r.multi_hand_landmarks:
                return None
            return ([(p.x, p.y) for p in r.multi_hand_landmarks[0].landmark],
                    r.multi_handedness[0].classification[0].score)
        return detect, f"mediapipe {mp.__version__} solutions.hands"
    from mediapipe.tasks.python import BaseOptions, vision
    if not os.path.exists(model_path):
        raise RuntimeError(f"{model_path} missing (mediapipe {mp.__version__} needs hand_landmarker.task)")
    task = vision.HandLandmarker.create_from_options(vision.HandLandmarkerOptions(
        base_options=BaseOptions(model_asset_path=model_path), running_mode=vision.RunningMode.VIDEO, num_hands=1))
    t0 = time.monotonic()

    def detect(rgb):
        r = task.detect_for_video(mp.Image(image_format=mp.ImageFormat.SRGB, data=rgb),
                                  int((time.monotonic() - t0) * 1000))
        if not r.hand_landmarks:
            return None
        return [(p.x, p.y) for p in r.hand_landmarks[0]], r.handedness[0][0].score
    return detect, f"mediapipe {mp.__version__} tasks ({os.path.basename(model_path)})"


def _read_exact(f, n):
    buf = b""
    while len(buf) < n:
        chunk = f.read(n - len(buf))
        if not chunk:
            return None
        buf += chunk
    return buf


def worker_main(model_path):
    import numpy as np
    out = sys.stdout
    sys.stdout = sys.stderr  # anything MediaPipe prints must not mix with the replies
    try:
        detect, kind = _landmarker(model_path)
    except Exception as e:  # noqa: BLE001
        out.write(json.dumps({"error": f"{e!r}"[:120]}) + "\n")
        out.flush()
        return
    out.write(json.dumps({"ready": kind}) + "\n")
    out.flush()
    inp = sys.stdin.buffer
    while True:
        head = _read_exact(inp, HEADER.size)
        if head is None:
            return  # bridge closed the pipe
        h, w, c = HEADER.unpack(head)
        data = _read_exact(inp, h * w * c)
        if data is None:
            return
        res = detect(np.frombuffer(data, np.uint8).reshape(h, w, c))
        out.write(json.dumps({"lms": res[0], "score": res[1]} if res else {"lms": None}) + "\n")
        out.flush()


if __name__ == "__main__":
    import argparse
    ap = argparse.ArgumentParser(description="hand worker (started by pi_bridge.py)")
    ap.add_argument("--worker", action="store_true", required=True)
    ap.add_argument("--model", default=os.path.expanduser("~/arm/hand_landmarker.task"))
    worker_main(ap.parse_args().model)
