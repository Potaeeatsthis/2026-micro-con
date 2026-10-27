#!/usr/bin/env python3
"""Offline position reference for the five positioning joints.

Owner: IK / mechanical member.

This is an explicit hybrid model, not exact CAD and not production FK/IK.
Measured lengths are used for the shoulder height (94 mm), upper arm (114 mm),
forearm (127 mm), jaw-tip distance (107 mm) and the object-centre setback
(3.5 mm -> 103.5 mm along the tool). CAD-only values are the shoulder forward
offset (23.15 mm), the wrist roll line lateral offset (15.25 mm), the object
offset in the J5 frame (-13, -5.73 mm robot) and the object-centre geometry.

The composition is:
  Rz(q1) * T(23.15, 0, 94) * Ry(q2) * T(0, 0, 114) * Ry(q3) *
  T(0, 15.25, 127) * Rz(q4) * Ry(q5) * T(-13, -5.73, 103.5)

It treats the three pitch axes as parallel +Y on the shared forward line
x = 23.15 and collapses the J4 roll / J5 pitch points to the end of the
forearm. That is valid for position: the roll axis is parallel to the forearm,
so its height along the line does not change the x/y offset it rotates, and a
pitch about Y leaves the lateral y offset unchanged. It does not model
orientation, the J6 gripper, collisions or servo limits.

CAD places the shoulder at X = 23.15 and the wrist axes at X = 23.14. The model
folds that 0.01 mm difference into the 23.15 line, so the zero object is
x = 10.15 mm instead of the 10.14 mm CAD-derived reference (a 0.01 mm
simplification, not a measured accuracy claim). The mean jaw-tip height is not used to invent individual
tip coordinates.

Angles are model angles q1..q5 in degrees, right-handed. +Y pitch maps +Z
toward +X. This file never commands hardware.

CLI: python3 ProjectDevGuide/tests/host/arm_fk_reference.py Q1 Q2 Q3 Q4 Q5
"""
import math
import sys

SHOULDER = (23.15, 0.0, 94.0)
UPPER_ARM = 114.0
FOREARM = 127.0
ROLL_LATERAL = 15.25
TOOL = (-13.0, -5.73, 103.5)


def _rot_z(angle_rad):
    c, s = math.cos(angle_rad), math.sin(angle_rad)
    return ((c, -s, 0.0), (s, c, 0.0), (0.0, 0.0, 1.0))


def _rot_y(angle_rad):
    c, s = math.cos(angle_rad), math.sin(angle_rad)
    return ((c, 0.0, s), (0.0, 1.0, 0.0), (-s, 0.0, c))


def _mat_mul(a, b):
    return tuple(
        tuple(sum(a[row][k] * b[k][col] for k in range(3)) for col in range(3))
        for row in range(3)
    )


def _mat_vec(matrix, vector):
    return tuple(
        sum(matrix[row][k] * vector[k] for k in range(3)) for row in range(3)
    )


def _add(a, b):
    return tuple(a[i] + b[i] for i in range(3))


def object_position(q1, q2, q3, q4, q5):
    """Return the object-centre position in mm for model angles in degrees."""
    rotation = _rot_z(math.radians(q1))
    position = _mat_vec(rotation, SHOULDER)

    rotation = _mat_mul(rotation, _rot_y(math.radians(q2)))
    position = _add(position, _mat_vec(rotation, (0.0, 0.0, UPPER_ARM)))

    rotation = _mat_mul(rotation, _rot_y(math.radians(q3)))
    position = _add(position, _mat_vec(rotation, (0.0, ROLL_LATERAL, FOREARM)))

    rotation = _mat_mul(rotation, _rot_z(math.radians(q4)))
    rotation = _mat_mul(rotation, _rot_y(math.radians(q5)))
    position = _add(position, _mat_vec(rotation, TOOL))
    return position


def parse_angles(argv):
    """Parse exactly five finite angles in degrees, or raise ValueError."""
    if len(argv) != 5:
        raise ValueError("expected five joint angles in degrees")
    try:
        angles = [float(value) for value in argv]
    except ValueError as exc:
        raise ValueError("angles must be numbers") from exc
    if not all(math.isfinite(angle) for angle in angles):
        raise ValueError("angles must be finite")
    return angles


def main(argv=None):
    argv = sys.argv[1:] if argv is None else argv
    try:
        angles = parse_angles(argv)
    except ValueError as exc:
        print(f"error: {exc}", file=sys.stderr)
        print(__doc__.strip().splitlines()[-1], file=sys.stderr)
        return 2

    x, y, z = object_position(*angles)
    print("model angles q1..q5 (deg): " + " ".join(f"{angle:.3f}" for angle in angles))
    print(f"object xyz (mm): x={x:.3f} y={y:.3f} z={z:.3f}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
