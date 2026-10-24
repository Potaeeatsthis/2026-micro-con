#!/usr/bin/env python3
"""pilink_test.py - Pi side of the STM32 <-> Pi link bench test (protocol v1, text lines).

Runs on the Raspberry Pi, talks to the STM32 USART6 over /dev/serial0 (115200 8N1).
Every line received from the STM32 is printed with a timestamp: seeing "RX FIND red_box"
proves STM32 TX (PG14) -> Pi RXD (pin 10) works.

Protocol v1 (ProjectDevGuide/docs/core-config.md, study/robotic-arm-guide.md section 12):
    STM32 -> Pi:  FIND <name>
    Pi -> STM32:  OBJ <name> <x> <y> <z>      integer mm, robot-base frame, top-face centre
    Pi -> STM32:  NOTFOUND <name>
    names: blue_box, green_box, red_box, black_box

Reply mode (what the script answers to the next FIND), changed with a key:
    o   reply OBJ with --xyz (or the RealSense position)       [default]
    n   reply NOTFOUND
    s   stay silent (the STM32 should log a timeout)
Send now:
    g   a garbage line            l   an over-long line (200 x 'A')
    q   quit
Keys work in a terminal (single keypress) and from a pipe (one key per line), e.g.
    (sleep 30; echo n; sleep 10; echo q) | python pilink_test.py

--realsense: the XYZ comes from the camera pipeline in ~/arm/pi_vision_uart.py (imported, not
changed). Run inside ~/arm-env. Until ~/arm/calib_T.npy exists the values are CAMERA-frame mm
(the script says so at start); the team protocol wants robot-base mm.
The pipeline and this script cannot run at the same time (both open the camera and the UART).
"""
import argparse
import os
import select
import sys
import threading
import time

try:
    import serial
except ImportError:
    sys.exit("pyserial missing: run with ~/arm-env/bin/python")

NAMES = ("blue_box", "green_box", "red_box", "black_box")


def ts():
    t = time.time()
    return time.strftime("%H:%M:%S", time.localtime(t)) + f".{int(t * 1000) % 1000:03d}"


def say(msg):
    print(f"{ts()} {msg}", flush=True)


class RealSenseSource:
    """Latest position per colour from pi_vision_uart.py, updated in a background thread."""

    def __init__(self, arm_dir, max_age_s, min_conf):
        arm_dir = os.path.expanduser(arm_dir)
        sys.path.insert(0, arm_dir)
        os.chdir(arm_dir)  # the pipeline loads color_refs.json / calib_T.npy from the cwd
        import numpy as np
        import pi_vision_uart as pv
        self.pv, self.max_age_s, self.min_conf = pv, max_age_s, min_conf
        if os.path.exists(pv.CALIB_FILE):
            self.T, self.key, self.frame = np.load(pv.CALIB_FILE), "xyz_base_mm", "robot-base"
        else:
            self.T, self.key, self.frame = np.eye(4), "xyz_cam_mm", "CAMERA"
        say(f"realsense: sending {self.frame}-frame mm"
            + ("" if self.frame == "robot-base" else f" ({pv.CALIB_FILE} not found)"))
        self.cam = pv.BlockCamera(fps=None, laser="auto")
        self.det = pv.BlockDetector(self.cam.intr, self.cam.depth_scale, pv.load_refs())
        self.latest, self.lock, self.stop = {}, threading.Lock(), False
        threading.Thread(target=self._run, daemon=True).start()

    def _run(self):
        while not self.stop:
            color, depth = self.cam.grab()
            if color is None:
                continue
            blocks = self.det.detect(color, depth)
            best = self.pv.best_per_color(blocks, self.T, time.monotonic())
            with self.lock:
                self.latest.update(best)

    def lookup(self, name):
        """name like 'red_box' -> (x, y, z) int mm, or None if not seen recently / low confidence."""
        colour = name[:-4] if name.endswith("_box") else name
        cid = next((k for k, v in self.pv.COLOR_NAMES.items() if v == colour), None)
        with self.lock:
            r = self.latest.get(cid)
        if r is None or time.monotonic() - r["t"] > self.max_age_s or r["conf"] < self.min_conf:
            return None
        return tuple(int(round(v)) for v in r[self.key])

    def close(self):
        self.stop = True
        time.sleep(0.2)
        self.cam.stop()


class Keys:
    """Single keys from a terminal, or keys from a pipe (whitespace ignored, one per line is fine).
    Reads the raw fd so select() never misses keys sitting in Python's stdin buffer."""

    def __init__(self):
        self.fd = sys.stdin.fileno()
        self.old = None
        if sys.stdin.isatty():
            import termios
            import tty
            self.old = termios.tcgetattr(self.fd)
            tty.setcbreak(self.fd)
        self.pending = []
        self.eof = False

    def get(self, timeout):
        if not self.pending and not self.eof:
            r, _, _ = select.select([self.fd], [], [], timeout)
            if r:
                data = os.read(self.fd, 64)
                if not data:
                    self.eof = True  # pipe closed: keep running until --duration or q
                self.pending += [c for c in data.decode("ascii", "ignore") if not c.isspace()]
        elif not self.pending:
            time.sleep(timeout)
        return self.pending.pop(0) if self.pending else None

    def close(self):
        if self.old is not None:
            import termios
            termios.tcsetattr(self.fd, termios.TCSADRAIN, self.old)


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--port", default="/dev/serial0")
    ap.add_argument("--baud", type=int, default=115200)
    ap.add_argument("--xyz", type=int, nargs=3, default=[182, -45, 30], metavar=("X", "Y", "Z"),
                    help="fake OBJ position in mm (default: the guide's example 182 -45 30)")
    ap.add_argument("--mode", choices="ons", default="o", help="start reply mode: o=OBJ n=NOTFOUND s=silent")
    ap.add_argument("--duration", type=float, default=0, help="quit after this many seconds (0 = until q)")
    ap.add_argument("--realsense", action="store_true", help="XYZ from ~/arm/pi_vision_uart.py")
    ap.add_argument("--arm-dir", default="~/arm", help="folder with pi_vision_uart.py")
    ap.add_argument("--max-age", type=float, default=1.0, help="realsense: oldest detection used (s)")
    ap.add_argument("--min-conf", type=int, default=40, help="realsense: lowest confidence (0-100) used")
    args = ap.parse_args()

    src = RealSenseSource(args.arm_dir, args.max_age, args.min_conf) if args.realsense else None
    ser = serial.Serial(args.port, args.baud, bytesize=8, parity="N", stopbits=1, timeout=0)
    ser.reset_input_buffer()
    keys = Keys()
    mode = args.mode
    mode_name = {"o": "OBJ", "n": "NOTFOUND", "s": "silent"}
    stats = {"rx_lines": 0, "find": 0, "replies": 0, "bad_rx": 0}
    say(f"open {args.port} {args.baud} 8N1, reply mode {mode_name[mode]}, keys o/n/s/g/l/q")

    def send(line):
        ser.write((line + "\n").encode("ascii"))
        ser.flush()
        say(f"TX {line if len(line) <= 60 else line[:60] + '...'} ({len(line) + 1} B)")

    buf = b""
    t_end = time.monotonic() + args.duration if args.duration > 0 else None
    try:
        while t_end is None or time.monotonic() < t_end:
            buf += ser.read(ser.in_waiting or 1)
            while b"\n" in buf:
                raw, buf = buf.split(b"\n", 1)
                t_rx = time.monotonic()
                # drop non-printable bytes (e.g. a glitch byte while the STM32 resets) before matching
                line = "".join(c for c in raw.decode("ascii", "replace") if " " <= c <= "~")
                stats["rx_lines"] += 1
                say(f"RX {line}")
                parts = line.split()
                if len(parts) == 2 and parts[0] == "FIND" and parts[1] in NAMES:
                    stats["find"] += 1
                    name = parts[1]
                    if mode == "s":
                        say(f"(silent: no reply to FIND {name})")
                        continue
                    pos = None
                    if mode == "o":
                        pos = src.lookup(name) if src else tuple(args.xyz)
                    if pos is None:
                        send(f"NOTFOUND {name}")
                    else:
                        send(f"OBJ {name} {pos[0]} {pos[1]} {pos[2]}")
                    stats["replies"] += 1
                    say(f"(reply {int((time.monotonic() - t_rx) * 1000)} ms after RX)")
                else:
                    stats["bad_rx"] += 1
                    say("(not a FIND v1 line)")
            k = keys.get(0.01)
            if k is None:
                continue
            if k in "ons":
                mode = k
                say(f"reply mode -> {mode_name[mode]}")
            elif k == "g":
                send("garbage !! 123")
            elif k == "l":
                send("A" * 200)
            elif k == "q":
                break
    except KeyboardInterrupt:
        pass
    finally:
        keys.close()
        ser.close()
        if src:
            src.close()
        say(f"done: rx lines {stats['rx_lines']}, FIND {stats['find']}, replies {stats['replies']}, "
            f"other rx {stats['bad_rx']}")


if __name__ == "__main__":
    main()
