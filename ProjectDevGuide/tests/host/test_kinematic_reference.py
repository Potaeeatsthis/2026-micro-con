#!/usr/bin/env python3
"""Offline check of the candidate servo-angle <-> model-angle mapping.

Owner: IK / mechanical member.

The original alignment pose was POSE43 on the installed arm: servo J1..J5 =
85, 140, 85, 75, 100 deg, with J6 = 0. Those historical values are kept as
recorded, and the small 1 deg direction checks were made at that pose. The
active candidate reference is now J1..J5 = 85, 145, 88, 88, 96 deg: the user
reported J2 = 145 as vertical and supplied J3 = 88, J4 = 88 and J5 = 96 as the
revised candidate references, so those are the candidate zeros used by the
mapping. J3 = 88, J4 = 88 and J5 = 96 are user-supplied references; exact CAD
and physical verification are not established. The signs come from the 1 deg
checks recorded in ProjectDevGuide/docs/servo-calibration.md (the J2 check was
140 -> 141, not re-dated to 145; the J4 and J5 checks were made with J4 at 75,
so the J5 sign still needs a recheck at the active 88). With the corrected
gripper gear, the closed jaws just touch without binding and both closed
jaw-tip heights are 445 mm from the floor/underplate. At the J1..J5 reference
(candidate zeros) a held 15 x 25 x 15 mm block (15 mm vertical, top face assumed level)
had its top at 446 mm, giving an object-centre height of 438.5 mm. That matches
the model z within ruler precision only; x/y and the full FK are not validated.
The loaded J6 holding angle was not specified. The earlier 453/425 mm readings
were separate object-top attempts before the gear fix, not jaw tips, and are
superseded.

This is not production FK/IK and not a hardware command. It checks the
documented zero/sign round trip (excluding the J6 gripper) and the offline
hybrid position reference in arm_fk_reference.py. The per-joint installed
ranges below are finite sample bounds; the J2/J5 coupled limits, orientation,
collision safety and the real jaw-tip coordinates are not certified here.

Run: python3 ProjectDevGuide/tests/host/test_kinematic_reference.py
"""
import math
import unittest

from arm_fk_reference import (
    ROLL_LATERAL,
    SHOULDER,
    object_position,
    parse_angles,
)

JOINTS = ("J1", "J2", "J3", "J4", "J5")
ZERO = dict(zip(JOINTS, (85.0, 145.0, 88.0, 88.0, 96.0)))
SIGN = dict(zip(JOINTS, (1.0, -1.0, 1.0, 1.0, -1.0)))
HOME = dict(zip(JOINTS, (85.0, 125.0, 145.0, 90.0, 25.0)))
REFERENCE = dict(zip(JOINTS, (85.0, 145.0, 88.0, 88.0, 96.0)))
HISTORICAL_POSE43 = dict(zip(JOINTS, (85.0, 140.0, 85.0, 75.0, 100.0)))
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
            # The 1 deg checks were made at the historical POSE43 positions, so
            # J2, J3 and J4 start at their old positions. The J5 check used
            # J4 = 75; its sign still needs a recheck at the active J4 = 88 roll.
            start = HISTORICAL_POSE43[joint]
            delta = joint_from_servo(joint, start + 1.0) - joint_from_servo(joint, start)
            self.assertAlmostEqual(delta, direction)
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
        expected = (0.0, 20.0, 57.0, 2.0, 71.0)
        for joint, angle in zip(JOINTS, expected):
            self.assertAlmostEqual(joint_from_servo(joint, HOME[joint]), angle)


class ForwardKinematicsReference(unittest.TestCase):
    def test_zero_pose_object_position(self):
        # 10.15 is the model's shared-line value; the CAD-derived reference is
        # 10.14, the 0.01 mm shoulder (23.15) vs wrist (23.14) CAD difference.
        x, y, z = object_position(0.0, 0.0, 0.0, 0.0, 0.0)
        self.assertAlmostEqual(x, 10.15, places=9)
        self.assertAlmostEqual(y, 15.25 - 5.73, places=9)
        self.assertAlmostEqual(z, 94.0 + 114.0 + 127.0 + 103.5, places=9)

    def test_positive_increments_follow_model_signs(self):
        base = object_position(0.0, 0.0, 0.0, 0.0, 0.0)
        base_yaw = object_position(1.0, 0.0, 0.0, 0.0, 0.0)
        self.assertGreater(base_yaw[1], base[1])
        self.assertLess(base_yaw[0], base[0])

        for angles in ((0, 1, 0, 0, 0), (0, 0, 1, 0, 0), (0, 0, 0, 0, 1)):
            moved = object_position(*angles)
            self.assertGreater(moved[0], base[0])
            self.assertAlmostEqual(moved[1], base[1])

        rolled = object_position(0.0, 0.0, 0.0, 1.0, 0.0)
        self.assertGreater(rolled[0], base[0])
        self.assertLess(rolled[1], base[1])
        self.assertAlmostEqual(rolled[2], base[2])

    def test_nonzero_roll_rotates_about_the_roll_line(self):
        base = object_position(0.0, 0.0, 0.0, 0.0, 0.0)
        rolled = object_position(0.0, 0.0, 0.0, 90.0, 0.0)
        axis_x, axis_y = SHOULDER[0], ROLL_LATERAL
        self.assertAlmostEqual(
            math.hypot(base[0] - axis_x, base[1] - axis_y),
            math.hypot(rolled[0] - axis_x, rolled[1] - axis_y),
            places=9,
        )
        self.assertAlmostEqual(rolled[2], base[2], places=9)
        self.assertAlmostEqual(rolled[0], 23.15 + 5.73, places=9)
        self.assertAlmostEqual(rolled[1], 15.25 - 13.0, places=9)

    def test_known_90_degree_base_rotation(self):
        x, y, z = object_position(90.0, 0.0, 0.0, 0.0, 0.0)
        self.assertAlmostEqual(x, -9.52, places=9)
        self.assertAlmostEqual(y, 10.15, places=9)
        self.assertAlmostEqual(z, 438.5, places=9)

    def test_q4_zero_matches_independent_planar_model(self):
        a, h, l1, l2, lt = 23.15, 94.0, 114.0, 127.0, 103.5
        tx, ty, d = -13.0, -5.73, 15.25
        for q1, q2, q3, q5 in (
            (0.0, 0.0, 0.0, 0.0),
            (30.0, 20.0, 30.0, -25.0),
            (10.0, -10.0, 45.0, 10.0),
            (0.0, 30.0, 30.0, -60.0),
        ):
            r2, r3 = math.radians(q2), math.radians(q3)
            phi = math.radians(q2 + q3 + q5)
            u = (
                a
                + l1 * math.sin(r2)
                + l2 * math.sin(r2 + r3)
                + tx * math.cos(phi)
                + lt * math.sin(phi)
            )
            v = (
                h
                + l1 * math.cos(r2)
                + l2 * math.cos(r2 + r3)
                - tx * math.sin(phi)
                + lt * math.cos(phi)
            )
            lateral = d + ty
            r1 = math.radians(q1)
            expected = (
                u * math.cos(r1) - lateral * math.sin(r1),
                u * math.sin(r1) + lateral * math.cos(r1),
                v,
            )
            actual = object_position(q1, q2, q3, 0.0, q5)
            for got, want in zip(actual, expected):
                self.assertAlmostEqual(got, want, places=9)

    def test_cli_requires_five_finite_angles(self):
        self.assertEqual(
            parse_angles(["1", "2.5", "-3", "4", "5"]),
            [1.0, 2.5, -3.0, 4.0, 5.0],
        )
        for bad in (
            [],
            ["1", "2", "3", "4"],
            ["1", "2", "3", "4", "5", "6"],
            ["1", "2", "3", "4", "nan"],
            ["1", "2", "3", "4", "inf"],
            ["1", "2", "3", "4", "x"],
        ):
            with self.assertRaises(ValueError):
                parse_angles(bad)


if __name__ == "__main__":
    unittest.main()
