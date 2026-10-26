#!/usr/bin/env python3
"""Line handling, transform and file reloading of ProjectDevGuide/tools/pi/pi_bridge.py, and the hand
lookup / palm position of pi_hand.py, no camera, serial port or mediapipe needed.
Calibration-file and palm tests need numpy (skipped without it):
    python3 ProjectDevGuide/tests/host/test_pi_bridge.py
    uv run --no-project --with numpy python ProjectDevGuide/tests/host/test_pi_bridge.py
"""
import os
import sys
import tempfile
import time
import unittest

try:
    import numpy as np
except ImportError:
    np = None

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", "tools", "pi"))
import pi_bridge as pb  # noqa: E402
import pi_hand as ph  # noqa: E402

POS = {"red_box": (182.4, -45.6, 30.0), "blue_box": (-0.5, 0.49, 12.5), "hand": (210.2, 15.0, 95.6)}


def lookup(name):
    return POS.get(name)


class HandleLine(unittest.TestCase):
    def test_find_seen(self):
        self.assertEqual(pb.handle_line("FIND red_box", lookup)[0], "OBJ red_box 182 -46 30")

    def test_find_not_seen(self):
        self.assertEqual(pb.handle_line("FIND green_box", lookup)[0], "NOTFOUND green_box")

    def test_rounding_and_negative_zero(self):
        self.assertEqual(pb.handle_line("FIND blue_box", lookup)[0], "OBJ blue_box 0 0 12")

    def test_all_names_answered(self):
        for name in ("blue_box", "green_box", "red_box", "black_box"):
            self.assertIsNotNone(pb.handle_line(f"FIND {name}", lookup)[0])

    def test_glitch_bytes_and_spaces(self):
        self.assertEqual(pb.handle_line("\x00\xffFIND  red_box \r", lookup)[0], "OBJ red_box 182 -46 30")

    def test_find_hand(self):
        self.assertEqual(pb.handle_line("FIND hand", lookup)[0], "OBJ hand 210 15 96")
        self.assertEqual(pb.handle_line("FIND hand", lambda n: None)[0], "NOTFOUND hand")

    def test_box_names_unchanged(self):
        self.assertEqual(pb.NAMES, ("blue_box", "green_box", "red_box", "black_box"))
        self.assertIsNone(pb.colour_of("hand"))

    def test_ignored(self):
        for line in ("", "FIND", "FIND red", "FIND red_box extra", "find red_box", "OBJ red_box 1 2 3",
                     "NOTFOUND red_box", "garbage !! 123", "FIND Hand", "FIND hands", "FIND hand 1"):
            reply, note = pb.handle_line(line, lookup)
            self.assertIsNone(reply, line)
            self.assertIn("ignored", note)

    def test_only_v1_keywords_sent(self):
        for name in pb.NAMES:
            for pos in (None, (1, 2, 3)):
                self.assertIn(pb.format_reply(name, pos).split()[0], ("OBJ", "NOTFOUND"))


class Splitter(unittest.TestCase):
    def test_split_across_reads(self):
        s = pb.LineSplitter()
        self.assertEqual(s.feed(b"FIND re"), [])
        self.assertEqual(s.feed(b"d_box\r\nFIND blue_box\n"), ["FIND red_box", "FIND blue_box"])

    def test_over_long_dropped(self):
        s = pb.LineSplitter()
        self.assertEqual(s.feed(b"A" * 200 + b"\nFIND red_box\n"), ["FIND red_box"])
        self.assertEqual(s.dropped, 1)

    def test_endless_junk_dropped(self):
        s = pb.LineSplitter()
        self.assertEqual(s.feed(b"B" * 100), [])
        self.assertEqual(s.dropped, 1)
        self.assertEqual(s.feed(b"FIND red_box\n"), ["FIND red_box"])

    def test_colour_names(self):
        self.assertEqual(pb.colour_of("black_box"), "black")
        self.assertIsNone(pb.colour_of("pink_box"))


class Transform(unittest.TestCase):
    def test_identity(self):
        T = [[1, 0, 0, 0], [0, 1, 0, 0], [0, 0, 1, 0], [0, 0, 0, 1]]
        self.assertEqual(pb.apply_transform(T, (10.0, -20.0, 300.0)), (10.0, -20.0, 300.0))

    def test_rotation_and_translation_in_metres(self):
        # camera z (forward) -> base x, camera x -> base -y, camera y -> base -z, then +0.1 m in x
        T = [[0, 0, 1, 0.1], [-1, 0, 0, 0], [0, -1, 0, 0.05], [0, 0, 0, 1]]
        x, y, z = pb.apply_transform(T, (20.0, 30.0, 300.0))
        self.assertAlmostEqual(x, 400.0)
        self.assertAlmostEqual(y, -20.0)
        self.assertAlmostEqual(z, 20.0)


class Files(unittest.TestCase):
    def setUp(self):
        self.dir = tempfile.TemporaryDirectory()
        self.path = os.path.join(self.dir.name, "calib_T.npy")

    def tearDown(self):
        self.dir.cleanup()

    def test_filewatch_appear_change_disappear(self):
        seen = []
        w = pb.FileWatch(self.path, seen.append)
        w.check()
        self.assertEqual(seen, [None])           # first check reports "missing"
        w.check()
        self.assertEqual(seen, [None])           # no change, no call
        with open(self.path, "w") as f:
            f.write("a")
        w.check()
        self.assertEqual(seen[-1], self.path)    # appeared
        with open(self.path, "w") as f:
            f.write("abc")                       # size change counts even within one mtime tick
        w.check()
        self.assertEqual(len(seen), 3)
        os.remove(self.path)
        w.check()
        self.assertEqual(seen[-1], None)         # gone

    def test_prime_skips_current_state(self):
        with open(self.path, "w") as f:
            f.write("a")
        seen = []
        w = pb.FileWatch(self.path, seen.append)
        w.prime()
        w.check()
        self.assertEqual(seen, [])

    def test_no_calib_refuse_or_camera(self):
        c = pb.Calibration(self.path, camera_frame_ok=False)
        c.check()
        self.assertIsNone(c.frame)
        self.assertIsNone(c.to_frame((1.0, 2.0, 3.0)))
        c = pb.Calibration(self.path, camera_frame_ok=True)
        c.check()
        self.assertEqual(c.frame, "CAMERA")
        self.assertEqual(c.to_frame((1.0, 2.0, 3.0)), (1.0, 2.0, 3.0))

    @unittest.skipIf(np is None, "numpy not installed")
    def test_calib_appears_then_broken_then_gone(self):
        c = pb.Calibration(self.path, camera_frame_ok=True)
        c.check()
        self.assertEqual(c.frame, "CAMERA")
        T = np.eye(4)
        T[:3, 3] = [0.1, 0.0, -0.02]
        np.save(self.path, T)
        c.check()
        self.assertEqual(c.frame, "robot-base")
        self.assertEqual(tuple(round(v, 6) for v in c.to_frame((0.0, 0.0, 300.0))), (100.0, 0.0, 280.0))
        time.sleep(0.01)
        np.save(self.path, np.zeros((3, 3)))   # wrong shape: must not be used, and no camera fallback
        c.check()
        self.assertIsNone(c.frame)
        self.assertIsNone(c.to_frame((0.0, 0.0, 300.0)))
        os.remove(self.path)
        c.check()
        self.assertIsNone(c.frame)               # still no fallback once calibrated
        np.save(self.path, T)
        c.check()
        self.assertEqual(c.frame, "robot-base")  # fixed file is used again
        os.remove(self.path)
        c = pb.Calibration(self.path, camera_frame_ok=False)
        c.check()
        self.assertIsNone(c.frame)


class FakeHand:
    def __init__(self, r, state="on"):
        self.r, self.state = r, state

    def get(self):
        return self.r


def hand_src(hand):
    """A CameraSource without camera: only what lookup("hand") / hand_status() use."""
    s = pb.CameraSource.__new__(pb.CameraSource)
    s.calib = pb.Calibration("/nonexistent/calib_T.npy", camera_frame_ok=True)
    s.calib.check()
    s.max_age_s, s.hand, s.hand_min_score, s.hand_state = 1.0, hand, 50, "off (--no-hand)"
    return s


class HandLookup(unittest.TestCase):
    def test_hand_off_is_notfound(self):
        s = hand_src(None)
        self.assertIsNone(s.lookup("hand"))
        self.assertEqual(s.hand_status(), "hand off (--no-hand)")

    def test_fresh_hand_sent(self):
        s = hand_src(FakeHand({"xyz_cam_mm": (10.0, 20.0, 400.0), "score": 90, "t": time.monotonic(), "info": ""}))
        self.assertEqual(s.lookup("hand"), (10.0, 20.0, 400.0))
        self.assertTrue(s.hand_status().startswith("hand c90 10 20 400"))

    def test_old_low_score_or_no_position_not_sent(self):
        now = time.monotonic()
        for r in ({"xyz_cam_mm": (1.0, 2.0, 300.0), "score": 90, "t": now - 2.0, "info": ""},
                  {"xyz_cam_mm": (1.0, 2.0, 300.0), "score": 30, "t": now, "info": ""},
                  {"xyz_cam_mm": None, "t": now, "info": "no hand"}):
            self.assertIsNone(hand_src(FakeHand(r)).lookup("hand"))

    def test_detector_failed_is_notfound(self):
        s = hand_src(FakeHand(None, state="off (ModuleNotFoundError)"))
        self.assertIsNone(s.lookup("hand"))
        self.assertIn("off", s.hand_status())


def pinhole(u, v, z):
    """fake intrinsics: fx = fy = 300, centre (212, 120)"""
    return ((u - 212) / 300.0 * z, (v - 120) / 300.0 * z, z)


@unittest.skipIf(np is None, "numpy not installed")
class PalmXYZ(unittest.TestCase):
    PALM = [(200, 100), (220, 100), (210, 110), (200, 120), (220, 120)]

    def depth(self, mm=400):
        return np.full((240, 424), mm, dtype=np.uint16)  # depth_scale 0.001: raw = mm

    def test_flat_palm(self):
        xyz, _ = ph.palm_xyz(self.PALM, self.depth(), 0.001, pinhole)
        self.assertAlmostEqual(xyz[2], 400.0)
        self.assertAlmostEqual(xyz[0], (210 - 212) / 300.0 * 400.0)
        self.assertAlmostEqual(xyz[1], (110 - 120) / 300.0 * 400.0)

    def test_holes_skipped_until_too_few(self):
        d = self.depth()
        d[95:106, 195:226] = 0          # no depth at the two top points
        xyz, info = ph.palm_xyz(self.PALM, d, 0.001, pinhole)
        self.assertIsNotNone(xyz, info)
        d[105:126, 195:226] = 0         # and none at the other three
        self.assertIsNone(ph.palm_xyz(self.PALM, d, 0.001, pinhole)[0])

    def test_one_point_on_the_background_dropped(self):
        d = self.depth()
        d[115:126, 215:226] = 900       # last point sees the floor behind the hand
        xyz, _ = ph.palm_xyz(self.PALM, d, 0.001, pinhole)
        self.assertAlmostEqual(xyz[2], 400.0)

    def test_out_of_range_and_off_image(self):
        self.assertIsNone(ph.palm_xyz(self.PALM, self.depth(100), 0.001, pinhole)[0])
        self.assertIsNone(ph.palm_xyz(self.PALM, self.depth(2000), 0.001, pinhole)[0])
        self.assertIsNone(ph.palm_xyz([(-5, 10), (500, 10), (10, 300)], self.depth(), 0.001, pinhole)[0])


FAKE_WORKER = r"""
import json, os, signal, struct, sys
mode = sys.argv[1]
out = sys.stdout
out.write(json.dumps({"ready": "fake " + mode}) + "\n"); out.flush()
n = 0
while True:
    head = sys.stdin.buffer.read(12)
    if len(head) < 12:
        break
    h, w, c = struct.unpack("<III", head)
    sys.stdin.buffer.read(h * w * c)
    n += 1
    if mode == "sigill" and n == 2:
        os.kill(os.getpid(), signal.SIGILL)   # like MediaPipe 1.1.1 on the Pi 4
    lms = [[0.5, 0.5]] * 21                   # every landmark at the image centre
    out.write(json.dumps({"lms": lms, "score": 0.97}) + "\n"); out.flush()
"""


@unittest.skipIf(np is None, "numpy not installed")
class HandWorker(unittest.TestCase):
    def setUp(self):
        self.dir = tempfile.TemporaryDirectory()
        self.script = os.path.join(self.dir.name, "fake_worker.py")
        with open(self.script, "w") as f:
            f.write(FAKE_WORKER)
        self.logs = []
        self.old_retry, ph.WORKER_RETRY_S = ph.WORKER_RETRY_S, 0.3

    def tearDown(self):
        ph.WORKER_RETRY_S = self.old_retry
        self.dir.cleanup()

    def source(self, cmd):
        return ph.HandSource(cmd, 0.001, pinhole, 20.0, self.logs.append)

    def feed(self, hs, seconds):
        color, depth = np.zeros((240, 424, 3), np.uint8), np.full((240, 424), 500, np.uint16)
        t_end = time.monotonic() + seconds
        while time.monotonic() < t_end:
            hs.offer(color, depth, time.monotonic())
            time.sleep(0.02)

    def test_worker_gives_palm_xyz(self):
        hs = self.source([sys.executable, self.script, "ok"])
        self.feed(hs, 1.5)
        r = hs.get()
        hs.close()
        self.assertEqual(hs.state, "on")
        self.assertEqual(r["score"], 97)
        self.assertAlmostEqual(r["xyz_cam_mm"][2], 500.0)
        self.assertAlmostEqual(r["xyz_cam_mm"][0], 0.0)   # centre pixel 212 = principal point
        s = hand_src(hs)
        self.assertIsNotNone(s.lookup("hand"))

    def test_crashing_worker_only_turns_hand_off(self):
        hs = self.source([sys.executable, self.script, "sigill"])
        self.feed(hs, 1.0)
        state, r = hs.state, hs.get()
        hs.close()
        self.assertTrue(state.startswith("stopped") or state == "on", state)  # may already be restarted
        self.assertTrue(any("worker exited" in m for m in self.logs), self.logs)
        self.assertIsNone(hand_src(FakeHand(None, state)).lookup("hand"))
        self.assertTrue(r is None or r["xyz_cam_mm"] is not None)

    def test_missing_python_is_off(self):
        hs = self.source(["/nonexistent/hand-env/bin/python", self.script, "ok"])
        self.feed(hs, 0.5)
        state = hs.state
        hs.close()
        self.assertTrue(state.startswith("off"), state)
        self.assertEqual(len([m for m in self.logs if "off" in m]), 1)    # logged once, not every retry


if __name__ == "__main__":
    unittest.main(verbosity=1)
