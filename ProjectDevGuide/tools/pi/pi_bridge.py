#!/usr/bin/env python3
"""pi_bridge.py - Pi side of the STM32 <-> Pi link (protocol v1, text lines), real camera positions.

Runs on the Raspberry Pi, talks to the STM32 USART6 over /dev/serial0 (115200 8N1) and answers
every FIND with the latest D435 detection of that block. Meant to run all the time (systemd service
pi_bridge.service, see ProjectDevGuide/docs/inputs-and-pi-interface.md).

Protocol v1 (ProjectDevGuide/docs/core-config.md, study/robotic-arm-guide.md section 12):
    STM32 -> Pi:  FIND <name>
    Pi -> STM32:  OBJ <name> <x> <y> <z>      integer mm, robot-base frame, top-face centre
    Pi -> STM32:  NOTFOUND <name>
    names: blue_box, green_box, red_box, black_box, hand
    hand = palm centre on the palm surface (pi_hand.py), same frame and units as the blocks
Nothing else is ever sent. A line that is not a valid FIND is logged and ignored.

Camera: BlockCamera / BlockDetector / load_refs() / best_per_color() from ~/arm/pi_vision_uart.py
(imported, not changed), in a background thread that detects all the time. A FIND is answered from
the latest detection: OBJ if it is younger than --max-age and its confidence is at least --min-conf,
otherwise NOTFOUND.

Hand: pi_hand.py (next to this file) sends the newest colour frame, at most --hand-hz times a second,
to a separate worker process (MediaPipe in ~/hand-env, --hand-python) and turns the palm landmarks +
depth into XYZ. A missing, crashed or stuck worker means "hand off": FIND hand gets NOTFOUND, the
blocks are not affected, and the worker is started again every 30 s. --no-hand: no worker at all.
--hand-snapshot saves ~/arm/hand.jpg to check it; --snapshot saves ~/arm/snapshot.jpg with everything
the bridge would answer (blocks + hand on the colour image | depth).

Files in --arm-dir, re-read while running (no restart needed):
    calib_T.npy       camera -> robot-base transform (--solve-calib). Checked on every FIND.
    color_refs.json   learned colours (--learn-color). Checked every 2 s.
Frame: the protocol wants robot-base mm. Without calib_T.npy the bridge refuses to start, unless
--camera-frame is given: then it sends camera-frame mm until calib_T.npy appears, and switches to
robot-base mm by itself. Once a calibration has been used, a missing or broken calib_T.npy never falls
back to camera frame: every FIND gets NOTFOUND until the file is fixed (the log says so).
Every reply and every status line says which frame was used.

Debug: every FIND and reply is logged, plus a status line every --status seconds (camera fps,
frame, last detection per colour). As a service: journalctl -u pi_bridge -f

One instance only: pi_vision_uart.py (--snapshot, --learn-color, --print-cam) and pilink_test.py
also open the camera and /dev/serial0, stop the bridge first.
    ~/arm-env/bin/python ~/arm/tools/pi_bridge.py                  # needs ~/arm/calib_T.npy
    ~/arm-env/bin/python ~/arm/tools/pi_bridge.py --camera-frame   # until calibrated
"""
import argparse
import os
import sys
import threading
import time

NAMES = ("blue_box", "green_box", "red_box", "black_box")
HAND = "hand"
FIND_NAMES = NAMES + (HAND,)
CAMERA_STALE_S = 5.0  # warn when the camera has delivered no frame for this long
REFS_CHECK_S = 2.0    # how often color_refs.json is checked for changes
PIPELINE_API = ("BlockCamera", "BlockDetector", "load_refs", "best_per_color", "COLOR_NAMES",
                "CALIB_FILE", "REFS_FILE")


def ts():
    t = time.time()
    return time.strftime("%H:%M:%S", time.localtime(t)) + f".{int(t * 1000) % 1000:03d}"


def say(msg):
    print(f"{ts()} {msg}", flush=True)


def colour_of(name):
    """'red_box' -> 'red' (the pipeline's colour name), or None for an unknown name."""
    return name[:-4] if name in NAMES else None


def apply_transform(T, xyz_mm):
    """4x4 rigid transform (nested lists, translation in metres, like calib_T.npy) on a point in mm."""
    p = [v / 1000.0 for v in xyz_mm]
    return tuple((T[i][0] * p[0] + T[i][1] * p[1] + T[i][2] * p[2] + T[i][3]) * 1000.0 for i in range(3))


def format_reply(name, pos):
    """pos = (x, y, z) in mm or None -> the v1 reply line (no newline)."""
    if pos is None:
        return f"NOTFOUND {name}"
    x, y, z = (int(round(v)) for v in pos)
    return f"OBJ {name} {x} {y} {z}"


def handle_line(line, lookup):
    """One received line -> (reply or None, what to log).
    lookup(name) returns (x, y, z) mm or None. Only 'FIND <known name>' gets a reply."""
    text = "".join(c for c in line if " " <= c <= "~")  # drop glitch bytes (e.g. STM32 reset)
    parts = text.split()
    if len(parts) == 2 and parts[0] == "FIND" and parts[1] in FIND_NAMES:
        return format_reply(parts[1], lookup(parts[1])), f"RX {text}"
    return None, f"RX {text!r}: not a FIND v1 line, ignored"


class LineSplitter:
    """Bytes in, complete lines out. Drops a line longer than max_len (and says so)."""

    def __init__(self, max_len=80):
        self.buf, self.max_len, self.dropped = b"", max_len, 0

    def feed(self, data):
        self.buf += data
        lines = []
        while b"\n" in self.buf:
            raw, self.buf = self.buf.split(b"\n", 1)
            raw = raw.rstrip(b"\r")
            if len(raw) > self.max_len:
                self.dropped += 1
            else:
                lines.append(raw.decode("ascii", "replace"))
        if len(self.buf) > self.max_len:  # no newline coming: throw the junk away
            self.buf = b""
            self.dropped += 1
        return lines


class FileWatch:
    """Calls load(path) when the file appears, changes (mtime/size) or disappears (load(None))."""

    def __init__(self, path, load):
        self.path, self.load, self.sig = path, load, "unset"

    def _sig(self):
        try:
            st = os.stat(self.path)
            return st.st_mtime_ns, st.st_size
        except OSError:
            return None

    def prime(self):
        """Take the current state as already loaded."""
        self.sig = self._sig()

    def check(self):
        sig = self._sig()
        if sig != self.sig:
            self.sig = sig
            self.load(self.path if sig is not None else None)


class Calibration:
    """calib_T.npy state. frame = 'robot-base', 'CAMERA' (allowed fallback) or None (refuse).
    CAMERA is only allowed before the first good calibration of this run."""

    def __init__(self, path, camera_frame_ok):
        self.camera_frame_ok, self.T, self.frame, self.info = camera_frame_ok, None, None, ""
        self.had_calib = False
        self.watch = FileWatch(path, self._load)

    def _load(self, path):
        old = self.frame
        self.T = None
        if path is not None:
            try:
                import numpy as np
                T = np.load(path)
                if T.shape != (4, 4) or not np.all(np.isfinite(T)):
                    raise ValueError(f"shape {T.shape}, need a finite 4x4")
                self.T = T.tolist()
                self.info = f"{os.path.basename(path)} {time.strftime('%m-%d %H:%M', time.localtime(os.path.getmtime(path)))}"
            except Exception as e:  # noqa: BLE001 - a broken file must be reported, not crash the bridge
                say(f"calib: cannot use {path}: {e!r}")
        if self.T is not None:
            self.frame, self.had_calib = "robot-base", True
        elif self.had_calib:
            self.frame, self.info = None, "calib_T.npy missing or broken after it was used: fix it"
        else:
            self.frame = "CAMERA" if self.camera_frame_ok else None
            self.info = "no calib_T.npy" + (", --camera-frame" if self.camera_frame_ok else "")
        if self.frame != old or path is not None:
            say(f"calib: now sending {self.frame or 'NOTHING (NOTFOUND to every FIND)'}-frame mm ({self.info})"
                + (f"\ncalib: T = {[[round(v, 4) for v in row] for row in self.T]}" if self.T else ""))

    def check(self):
        self.watch.check()

    def to_frame(self, xyz_cam_mm):
        """camera mm -> (x, y, z) mm in the current frame, or None when no frame may be sent."""
        if self.frame == "robot-base":
            return apply_transform(self.T, xyz_cam_mm)
        if self.frame == "CAMERA":
            return tuple(xyz_cam_mm)
        return None


class CameraSource:
    """Latest detection per colour from pi_vision_uart.py, updated in a background thread."""

    def __init__(self, arm_dir, max_age_s, min_conf, camera_frame, hand=None):
        arm_dir = os.path.expanduser(arm_dir)
        sys.path.insert(0, arm_dir)
        os.chdir(arm_dir)  # the pipeline loads color_refs.json from the cwd
        import numpy as np
        import pi_vision_uart as pv
        missing = [n for n in PIPELINE_API if not hasattr(pv, n)]
        if missing:
            sys.exit(f"{pv.__file__} no longer has {', '.join(missing)}: pi_bridge.py uses them, update it")
        self.pv, self.np, self.max_age_s, self.min_conf = pv, np, max_age_s, min_conf
        self.calib = Calibration(os.path.join(arm_dir, pv.CALIB_FILE), camera_frame)
        self.calib.check()
        if self.calib.frame is None:
            sys.exit(f"{os.path.join(arm_dir, pv.CALIB_FILE)} not found: calibrate first "
                     "(SETUP_PI.md section 9), or use --camera-frame until then")
        say(f"camera: max age {max_age_s} s, min conf {min_conf}")
        self.cam = pv.BlockCamera(fps=None, laser="auto")
        self.det = pv.BlockDetector(self.cam.intr, self.cam.depth_scale, pv.load_refs())
        self.refs_watch = FileWatch(os.path.join(arm_dir, pv.REFS_FILE), self._reload_refs)
        self.refs_watch.prime()  # load_refs() above already read it
        self.latest, self.lock, self.stop = {}, threading.Lock(), False
        self.last_frame, self.frames = time.monotonic(), 0
        self.hand, self.hand_min_score, self.hand_state = None, 0, "off (--no-hand)"
        if hand is not None:
            self.hand_min_score = hand["min_score"]
            try:
                import pi_hand
                intr = self.cam.intr
                deproject = lambda u, v, z: pv.rs.rs2_deproject_pixel_to_point(intr, [u, v], z)  # noqa: E731
                self.hand = pi_hand.HandSource(pi_hand.worker_cmd(hand["python"], hand["model"]),
                                               self.cam.depth_scale, deproject, hand["hz"], say)
            except Exception as e:  # noqa: BLE001 - no hand support must never stop the blocks
                self.hand_state = f"off ({e!r:.80})"
                say(f"hand: {self.hand_state}; FIND hand gets NOTFOUND")
        self.thread = threading.Thread(target=self._run, daemon=True)
        self.thread.start()

    def _reload_refs(self, path):
        self.det.refs = self.pv.load_refs()  # prints "Colours: ..." itself
        say(f"colours: {'reloaded ' + os.path.basename(path) if path else 'file gone, built-in colours'}")

    def _run(self):
        identity = self.np.eye(4)
        next_refs = time.monotonic() + REFS_CHECK_S
        try:
            while not self.stop:
                if time.monotonic() >= next_refs:
                    next_refs = time.monotonic() + REFS_CHECK_S
                    self.refs_watch.check()
                color, depth = self.cam.grab()  # None after a 1 s camera timeout
                if color is None:
                    continue
                self.last_frame = time.monotonic()
                self.frames += 1
                # camera mm only: the current calibration is applied at lookup time
                best = self.pv.best_per_color(self.det.detect(color, depth), identity, self.last_frame)
                with self.lock:
                    self.latest.update(best)
                if self.hand is not None:
                    self.hand.offer(color, depth, self.last_frame)
        except Exception as e:  # noqa: BLE001 - any camera failure must be visible, not silent
            say(f"camera: thread stopped: {e!r}")

    def alive(self):
        return self.thread.is_alive()

    def frame_age(self):
        return time.monotonic() - self.last_frame

    def _entry(self, colour):
        cid = next((k for k, v in self.pv.COLOR_NAMES.items() if v == colour), None)
        with self.lock:
            return self.latest.get(cid)

    def _hand_ok(self, r):
        return (r is not None and r["xyz_cam_mm"] is not None and time.monotonic() - r["t"] <= self.max_age_s
                and r["score"] >= self.hand_min_score)

    def lookup(self, name):
        self.calib.check()
        if name == HAND:
            r = self.hand.get() if self.hand is not None else None
            return self.calib.to_frame(r["xyz_cam_mm"]) if self._hand_ok(r) else None
        r = self._entry(colour_of(name))
        if r is None or time.monotonic() - r["t"] > self.max_age_s or r["conf"] < self.min_conf:
            return None
        return self.calib.to_frame(r["xyz_cam_mm"])

    def status(self, period_s):
        """One line for the log: frame, fps, last detection per colour (in the current frame)."""
        fps = self.frames / period_s
        self.frames = 0
        parts = []
        for name in NAMES:
            r = self._entry(colour_of(name))
            if r is None:
                parts.append(f"{name[:-4]} -")
                continue
            age = time.monotonic() - r["t"]
            pos = self.calib.to_frame(r["xyz_cam_mm"])
            xyz = "?" if pos is None else " ".join(str(int(round(v))) for v in pos)
            ok = "" if age <= self.max_age_s and r["conf"] >= self.min_conf else " (not sent)"
            parts.append(f"{name[:-4]} c{r['conf']} {xyz} {age:.1f}s{ok}")
        parts.append(self.hand_status())
        return f"frame {self.calib.frame or 'NONE'} ({self.calib.info}) | {fps:.1f} fps | " + " | ".join(parts)

    def hand_status(self):
        if self.hand is None or self.hand.state != "on":
            return f"hand {self.hand.state if self.hand is not None else self.hand_state}"
        r = self.hand.get()
        if r is None or r["xyz_cam_mm"] is None:
            return f"hand - ({r['info'] if r else 'no frame yet'})"
        age = time.monotonic() - r["t"]
        pos = self.calib.to_frame(r["xyz_cam_mm"])
        xyz = "?" if pos is None else " ".join(str(int(round(v))) for v in pos)
        return f"hand c{r['score']} {xyz} {age:.1f}s" + ("" if self._hand_ok(r) else " (not sent)")

    def close(self):
        self.stop = True
        self.thread.join(timeout=2.0)
        if self.hand is not None:
            self.hand.close()
        self.cam.stop()


def snapshot(arm_dir, python, model, blocks=False, seconds=5.0):
    """Run the detectors on the live camera for a few seconds and save one picture.
    blocks=False: hand only, best hand frame -> hand.jpg.
    blocks=True: blocks (pi_vision_uart's detector and drawing) + hand + depth -> snapshot.jpg, from the last
    frame with a hand position (or the last frame), after the table plane has settled."""
    import cv2
    import numpy as np
    import pi_hand
    arm_dir = os.path.expanduser(arm_dir)
    sys.path.insert(0, arm_dir)
    os.chdir(arm_dir)
    import pi_vision_uart as pv
    worker = pi_hand.Worker(pi_hand.worker_cmd(python, model))
    cam = pv.BlockCamera(fps=None, laser="auto")
    det = pv.BlockDetector(cam.intr, cam.depth_scale, pv.load_refs()) if blocks else None
    say(f"snapshot: {worker.kind}, {seconds:.0f} s" + (", blocks + hand" if blocks else ", hold a hand open in view"))
    deproject = lambda u, v, z: pv.rs.rs2_deproject_pixel_to_point(cam.intr, [u, v], z)  # noqa: E731
    best, n, found = None, 0, 0
    t_end = time.monotonic() + seconds
    try:
        while time.monotonic() < t_end:
            color, depth = cam.grab()
            if color is None:
                continue
            n += 1
            bl = det.detect(color, depth) if det else []
            msg = worker.detect(color[:, :, ::-1].copy())
            r = pi_hand.hand_result(msg, color.shape[1], color.shape[0], depth, cam.depth_scale, deproject)
            found += r["xyz_cam_mm"] is not None
            has, had = r["xyz_cam_mm"] is not None, best is not None and best[0]["xyz_cam_mm"] is not None
            if blocks:
                better = has or not had  # newest frame with a hand position, else newest frame
            else:
                better = best is None or (has and (not had or r["score"] > best[0]["score"]))
            if better:
                best = (r, color.copy(), depth, bl, det.plane is not None if det else True)
    finally:
        cam.stop()
        worker.close()
    if best is None:
        sys.exit("snapshot: no camera frames")
    r, color, depth, bl, plane = best
    xyz = r["xyz_cam_mm"]
    hand_txt = (f"c{r['score']} camera {' '.join(str(int(round(c))) for c in xyz)} mm" if xyz else r["info"])
    if not blocks:
        path = os.path.join(arm_dir, "hand.jpg")
        cv2.imwrite(path, pi_hand.draw(color, r))
        say(f"hand snapshot: position on {found} of {n} frames; best: {hand_txt} -> {path}")
        return
    status = f"{len(bl)} block(s)" + ("" if plane else " | NO TABLE PLANE")
    left = pi_hand.draw(pv.draw(color.copy(), bl, np.eye(4), status), r, text_top=True)
    dvis = cv2.applyColorMap(cv2.convertScaleAbs(depth, alpha=255.0 / 1500), cv2.COLORMAP_JET)
    dvis[depth == 0] = 0
    path = os.path.join(arm_dir, "snapshot.jpg")
    cv2.imwrite(path, np.hstack([left, cv2.resize(dvis, (left.shape[1], left.shape[0]),
                                                   interpolation=cv2.INTER_NEAREST)]))
    say(f"snapshot: {status} (camera mm)")
    for b in bl:
        x, y, z = b["xyz_cam_m"] * 1000.0
        say(f"  {(b['name'] or '?'):>6}  c{b['conf']:3d}  {x:6.0f} {y:6.0f} {z:6.0f} mm" + ("" if b["depth_ok"] else "  (est)"))
    say(f"  hand    {hand_txt}  (position on {found} of {n} frames)")
    say(f"saved {path}")


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--port", default="/dev/serial0")
    ap.add_argument("--baud", type=int, default=115200)
    ap.add_argument("--arm-dir", default="~/arm", help="folder with pi_vision_uart.py, calib_T.npy, color_refs.json")
    ap.add_argument("--max-age", type=float, default=1.0, help="oldest detection used (s)")
    ap.add_argument("--min-conf", type=int, default=40, help="lowest confidence (0-100) used")
    ap.add_argument("--camera-frame", action="store_true",
                    help="no calib_T.npy yet: send camera-frame mm until it appears")
    ap.add_argument("--status", type=float, default=10.0, help="status line every N seconds (0 = off)")
    ap.add_argument("--duration", type=float, default=0, help="quit after this many seconds (0 = never)")
    ap.add_argument("--no-hand", action="store_true", help="no hand detection (FIND hand gets NOTFOUND)")
    ap.add_argument("--hand-hz", type=float, default=5.0, help="hand detections per second")
    ap.add_argument("--hand-min-score", type=int, default=50, help="lowest hand score (0-100) used")
    ap.add_argument("--hand-python", default="~/hand-env/bin/python", help="Python with MediaPipe for the hand worker")
    ap.add_argument("--hand-model", default="~/arm/hand_landmarker.task",
                    help="MediaPipe model, only for versions without mp.solutions.hands")
    ap.add_argument("--hand-snapshot", action="store_true",
                    help="save ~/arm/hand.jpg with the detected hand and exit (stop the service first)")
    ap.add_argument("--snapshot", action="store_true",
                    help="save ~/arm/snapshot.jpg with blocks + hand + depth and exit (stop the service first)")
    args = ap.parse_args()
    hand_model = os.path.expanduser(args.hand_model)
    if args.hand_snapshot or args.snapshot:
        snapshot(args.arm_dir, args.hand_python, hand_model, blocks=args.snapshot)
        return

    try:
        import serial
    except ImportError:
        sys.exit("pyserial missing: run with ~/arm-env/bin/python")

    hand = None if args.no_hand else {"python": args.hand_python, "model": hand_model, "hz": args.hand_hz,
                                      "min_score": args.hand_min_score}
    src = CameraSource(args.arm_dir, args.max_age, args.min_conf, args.camera_frame, hand)
    ser = serial.Serial(args.port, args.baud, bytesize=8, parity="N", stopbits=1, timeout=0.05)
    ser.reset_input_buffer()
    split = LineSplitter()
    stats = {"find": 0, "obj": 0, "notfound": 0, "ignored": 0}
    say(f"open {args.port} {args.baud} 8N1, answering FIND. Ctrl+C to stop.")

    t_end = time.monotonic() + args.duration if args.duration > 0 else None
    next_status = time.monotonic() + args.status
    warned_stale = False
    try:
        while t_end is None or time.monotonic() < t_end:
            if not src.alive():
                sys.exit("camera thread stopped: exiting so the failure is not hidden (systemd restarts it)")
            stale = src.frame_age() > CAMERA_STALE_S
            if stale != warned_stale:
                warned_stale = stale
                say(f"camera: no frame for {CAMERA_STALE_S:.0f} s, every FIND gets NOTFOUND"
                    if stale else "camera: frames again")
            if args.status > 0 and time.monotonic() >= next_status:
                next_status += args.status
                src.calib.check()
                say(f"status: {src.status(args.status)} | FIND {stats['find']} OBJ {stats['obj']} "
                    f"NOTFOUND {stats['notfound']} ignored {stats['ignored']}")
            for line in split.feed(ser.read(ser.in_waiting or 1)):
                t_rx = time.monotonic()
                reply, note = handle_line(line, src.lookup)
                say(note)
                if reply is None:
                    stats["ignored"] += 1
                    continue
                ser.write((reply + "\n").encode("ascii"))
                ser.flush()
                stats["find"] += 1
                stats["obj" if reply.startswith("OBJ") else "notfound"] += 1
                say(f"TX {reply} [{src.calib.frame or 'no frame'}] ({int((time.monotonic() - t_rx) * 1000)} ms after RX)")
    except KeyboardInterrupt:
        pass
    finally:
        ser.close()
        src.close()
        say(f"done: FIND {stats['find']}, OBJ {stats['obj']}, NOTFOUND {stats['notfound']}, "
            f"ignored {stats['ignored']}, over-long dropped {split.dropped}")


if __name__ == "__main__":
    main()
