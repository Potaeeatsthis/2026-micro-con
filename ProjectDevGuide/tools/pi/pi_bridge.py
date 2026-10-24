#!/usr/bin/env python3
"""pi_bridge.py - Pi side of the STM32 <-> Pi link (protocol v1, text lines), real camera positions.

Runs on the Raspberry Pi, talks to the STM32 USART6 over /dev/serial0 (115200 8N1) and answers
every FIND with the latest D435 detection of that block.

Protocol v1 (ProjectDevGuide/docs/core-config.md, study/robotic-arm-guide.md section 12):
    STM32 -> Pi:  FIND <name>
    Pi -> STM32:  OBJ <name> <x> <y> <z>      integer mm, robot-base frame, top-face centre
    Pi -> STM32:  NOTFOUND <name>
    names: blue_box, green_box, red_box, black_box
Nothing else is ever sent. A line that is not a valid FIND is logged and ignored.

Camera: BlockCamera / BlockDetector / best_per_color() from ~/arm/pi_vision_uart.py (imported,
not changed), in a background thread. A FIND is answered from the latest detection: OBJ if it is
younger than --max-age and its confidence is at least --min-conf, otherwise NOTFOUND.

Frame: the protocol wants robot-base mm, so the bridge refuses to start without ~/arm/calib_T.npy
(see SETUP_PI.md section 9). --camera-frame sends camera-frame mm instead, for bench tests only.

Run inside ~/arm-env, one instance only: pi_vision_uart.py and pilink_test.py also open the
camera and /dev/serial0, stop them first.
    ~/arm-env/bin/python ~/arm/tools/pi_bridge.py
    ~/arm-env/bin/python ~/arm/tools/pi_bridge.py --camera-frame --duration 60
"""
import argparse
import os
import sys
import threading
import time

NAMES = ("blue_box", "green_box", "red_box", "black_box")
CAMERA_STALE_S = 5.0  # warn when the camera has delivered no frame for this long


def ts():
    t = time.time()
    return time.strftime("%H:%M:%S", time.localtime(t)) + f".{int(t * 1000) % 1000:03d}"


def say(msg):
    print(f"{ts()} {msg}", flush=True)


def colour_of(name):
    """'red_box' -> 'red' (the pipeline's colour name), or None for an unknown name."""
    return name[:-4] if name in NAMES else None


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
    if len(parts) == 2 and parts[0] == "FIND" and parts[1] in NAMES:
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


class CameraSource:
    """Latest detection per colour from pi_vision_uart.py, updated in a background thread."""

    def __init__(self, arm_dir, max_age_s, min_conf, camera_frame):
        arm_dir = os.path.expanduser(arm_dir)
        sys.path.insert(0, arm_dir)
        os.chdir(arm_dir)  # the pipeline loads color_refs.json / calib_T.npy from the cwd
        import numpy as np
        import pi_vision_uart as pv
        self.pv, self.max_age_s, self.min_conf = pv, max_age_s, min_conf
        if os.path.exists(pv.CALIB_FILE):
            self.T, self.key, frame = np.load(pv.CALIB_FILE), "xyz_base_mm", "robot-base"
        elif camera_frame:
            self.T, self.key, frame = np.eye(4), "xyz_cam_mm", "CAMERA (--camera-frame, bench only)"
        else:
            sys.exit(f"{os.path.join(arm_dir, pv.CALIB_FILE)} not found: calibrate first "
                     "(SETUP_PI.md section 9), or use --camera-frame for a bench test")
        say(f"camera: sending {frame}-frame mm, max age {max_age_s} s, min conf {min_conf}")
        self.cam = pv.BlockCamera(fps=None, laser="auto")
        self.det = pv.BlockDetector(self.cam.intr, self.cam.depth_scale, pv.load_refs())
        self.latest, self.lock, self.stop = {}, threading.Lock(), False
        self.last_frame = time.monotonic()
        self.thread = threading.Thread(target=self._run, daemon=True)
        self.thread.start()

    def _run(self):
        try:
            while not self.stop:
                color, depth = self.cam.grab()  # None after a 1 s camera timeout
                if color is None:
                    continue
                self.last_frame = time.monotonic()
                best = self.pv.best_per_color(self.det.detect(color, depth), self.T, self.last_frame)
                with self.lock:
                    self.latest.update(best)
        except Exception as e:  # noqa: BLE001 - any camera failure must be visible, not silent
            say(f"camera: thread stopped: {e!r}")

    def alive(self):
        return self.thread.is_alive()

    def frame_age(self):
        return time.monotonic() - self.last_frame

    def lookup(self, name):
        colour = colour_of(name)
        cid = next((k for k, v in self.pv.COLOR_NAMES.items() if v == colour), None)
        with self.lock:
            r = self.latest.get(cid)
        if r is None or time.monotonic() - r["t"] > self.max_age_s or r["conf"] < self.min_conf:
            return None
        return tuple(r[self.key])

    def close(self):
        self.stop = True
        self.thread.join(timeout=2.0)
        self.cam.stop()


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--port", default="/dev/serial0")
    ap.add_argument("--baud", type=int, default=115200)
    ap.add_argument("--arm-dir", default="~/arm", help="folder with pi_vision_uart.py and calib_T.npy")
    ap.add_argument("--max-age", type=float, default=1.0, help="oldest detection used (s)")
    ap.add_argument("--min-conf", type=int, default=40, help="lowest confidence (0-100) used")
    ap.add_argument("--camera-frame", action="store_true",
                    help="no calib_T.npy: send camera-frame mm (bench tests only)")
    ap.add_argument("--duration", type=float, default=0, help="quit after this many seconds (0 = never)")
    args = ap.parse_args()

    try:
        import serial
    except ImportError:
        sys.exit("pyserial missing: run with ~/arm-env/bin/python")

    src = CameraSource(args.arm_dir, args.max_age, args.min_conf, args.camera_frame)
    ser = serial.Serial(args.port, args.baud, bytesize=8, parity="N", stopbits=1, timeout=0.05)
    ser.reset_input_buffer()
    split = LineSplitter()
    stats = {"find": 0, "obj": 0, "notfound": 0, "ignored": 0}
    say(f"open {args.port} {args.baud} 8N1, answering FIND. Ctrl+C to stop.")

    t_end = time.monotonic() + args.duration if args.duration > 0 else None
    warned_stale = False
    try:
        while t_end is None or time.monotonic() < t_end:
            if not src.alive():
                sys.exit("camera thread stopped: exiting so the failure is not hidden")
            stale = src.frame_age() > CAMERA_STALE_S
            if stale != warned_stale:
                warned_stale = stale
                say(f"camera: no frame for {CAMERA_STALE_S:.0f} s, every FIND gets NOTFOUND"
                    if stale else "camera: frames again")
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
                say(f"TX {reply} ({int((time.monotonic() - t_rx) * 1000)} ms after RX)")
    except KeyboardInterrupt:
        pass
    finally:
        ser.close()
        src.close()
        say(f"done: FIND {stats['find']}, OBJ {stats['obj']}, NOTFOUND {stats['notfound']}, "
            f"ignored {stats['ignored']}, over-long dropped {split.dropped}")


if __name__ == "__main__":
    main()
