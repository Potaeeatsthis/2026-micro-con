#!/usr/bin/env python3
"""Offline check of the candidate servo-angle <-> model-angle mapping.

Owner: IK / mechanical member.

The reference pose is POSE43 on the installed arm: servo J1..J5 =
85, 140, 85, 75, 100 deg, with J6 = 0. The candidate zeros equal that pose and
the candidate signs come from the small 1 deg direction checks recorded in
ProjectDevGuide/docs/servo-calibration.md.

This is not production FK/IK and not a hardware command. It only checks the
documented zero/sign round trip, excluding the J6 gripper. The per-joint
installed ranges below are finite sample bounds; the J2/J5 coupled limits and
collision safety are not certified here.

Run: python3 ProjectDevGuide/tests/host/test_kinematic_reference.py
"""
import unittest

JOINTS = ("J1", "J2", "J3", "J4", "J5")
ZERO = dict(zip(JOINTS, (85.0, 140.0, 85.0, 75.0, 100.0)))
SIGN = dict(zip(JOINTS, (1.0, -1.0, 1.0, 1.0, -1.0)))
HOME = dict(zip(JOINTS, (85.0, 125.0, 145.0, 90.0, 25.0)))
REFERENCE = dict(zip(JOINTS, (85.0, 140.0, 85.0, 75.0, 100.0)))
INSTALLED = {  # table min/max from servo_cal.c, used only as sample bounds
    "J1": (0.0, 180.0),
    "J2": (50.0, 180.0),
    "J3": (45.0, 180.0),
    "J4": (0.0, 180.0),
    "J5": (25.0, 140.0),
}


def joint_from_servo(joint, servo):
    return SIGN[joint] * (servo - ZERO[joint])


def servo_from_joint(joint, q):
    return ZERO[joint] + SIGN[joint] * q


class KinematicReference(unittest.TestCase):
    def test_reference_pose_and_all_zeros(self):
        for joint in JOINTS:
            self.assertAlmostEqual(joint_from_servo(joint, REFERENCE[joint]), 0.0)
            self.assertAlmostEqual(joint_from_servo(joint, ZERO[joint]), 0.0)

    def test_positive_one_degree_follows_candidate_signs(self):
        observed = (1.0, -1.0, 1.0, 1.0, -1.0)
        for joint, direction in zip(JOINTS, observed):
            self.assertAlmostEqual(joint_from_servo(joint, REFERENCE[joint] + 1.0), direction)
        for joint in JOINTS:
            self.assertAlmostEqual(joint_from_servo(joint, ZERO[joint] + SIGN[joint]), 1.0)
            self.assertAlmostEqual(servo_from_joint(joint, 1.0), ZERO[joint] + SIGN[joint])

    def test_inverse_roundtrip_inside_installed_ranges(self):
        for joint in JOINTS:
            lo, hi = INSTALLED[joint]
            samples = (
                lo,
                (lo + ZERO[joint]) / 2.0,
                ZERO[joint],
                (ZERO[joint] + hi) / 2.0,
                hi,
            )
            for servo in samples:
                self.assertGreaterEqual(servo, lo)
                self.assertLessEqual(servo, hi)
                q = joint_from_servo(joint, servo)
                self.assertAlmostEqual(servo_from_joint(joint, q), servo)

    def test_unchanged_home_maps_to_expected_angles(self):
        expected = (0.0, 15.0, 60.0, 15.0, 75.0)
        for joint, angle in zip(JOINTS, expected):
            self.assertAlmostEqual(joint_from_servo(joint, HOME[joint]), angle)


if __name__ == "__main__":
    unittest.main()
