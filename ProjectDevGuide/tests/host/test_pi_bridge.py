#!/usr/bin/env python3
"""Line handling, transform and file reloading of ProjectDevGuide/tools/pi/pi_bridge.py,
no camera or serial port needed. Calibration-file tests need numpy (skipped without it):
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

POS = {"red_box": (182.4, -45.6, 30.0), "blue_box": (-0.5, 0.49, 12.5)}


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

    def test_ignored(self):
        for line in ("", "FIND", "FIND red", "FIND red_box extra", "find red_box", "OBJ red_box 1 2 3",
                     "NOTFOUND red_box", "garbage !! 123"):
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


if __name__ == "__main__":
    unittest.main(verbosity=1)
