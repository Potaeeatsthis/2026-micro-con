#!/usr/bin/env python3
"""Line handling of ProjectDevGuide/tools/pi/pi_bridge.py, no camera or serial port needed.
    python3 ProjectDevGuide/tests/host/test_pi_bridge.py
"""
import os
import sys
import unittest

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


if __name__ == "__main__":
    unittest.main(verbosity=1)
