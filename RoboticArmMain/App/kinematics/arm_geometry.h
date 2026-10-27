/* arm_geometry.h - arm dimensions and joint axes for ik / fk
 *
 * owner: ik / mechanical member.
 *
 * frame (defined in kinematics.h): robot-base, right-handed, +X forward,
 * +Y left, +Z up; CAD vectors map as (-X_cad, Z_cad, Y_cad).
 * points require subtracting the CAD base-bottom origin (0, -30.5, 18.18) mm
 * before that rotation. The floor is an unmeasured stand offset below that
 * plane: measured floor->J2 = 94 mm, CAD base-bottom->J2 = 90.22 mm, so the
 * floor-origin transform is not confirmed.
 * zero pose: straight up. positive joint axes at that pose (right-hand rule):
 *   J1 +Z base yaw, J2 +Y shoulder pitch, J3 +Y elbow pitch,
 *   J4 +Z wrist roll, J5 +Y wrist pitch. J6 (gripper) is not in positioning ik.
 * these are chosen mathematical signs, not measured servo or PWM directions.
 *
 * TODO(ik): measure on the real arm (guide section 22, open item 1):
 *   - link lengths between joint axes, in mm
 *   - the servo angle that corresponds to each joint's zero pose
 *   - the servo direction sign (the model axes above do not prove it)
 * all values below are PLACEHOLDERS so the code compiles; ik refuses to run
 * until ARM_GEOMETRY_MEASURED is 1.
 */
#ifndef APP_ARM_GEOMETRY_H
#define APP_ARM_GEOMETRY_H

#define ARM_GEOMETRY_MEASURED 0

#define ARM_BASE_HEIGHT_MM   0.0f   /* table to shoulder axis */
#define ARM_UPPER_ARM_MM     0.0f   /* shoulder axis to elbow axis */
#define ARM_FOREARM_MM       0.0f   /* elbow axis to J5 wrist pitch axis */
#define ARM_TOOL_MM          0.0f   /* J5 wrist pitch axis to gripper tip */

/* servo angle (deg) at which each joint is at its kinematic zero; sign = +1 or -1.
 * placeholders: +1 does not mean the installed servo turns the model-positive way.
 * J4 is wrist roll, proximal to the J5 wrist pitch axis. The planar model below
 * ignores J4 roll: assume J4 stays at its reference angle. If roll is non-zero,
 * it rotates the pitch axis with it, so the tool is not limited to the planar
 * pitch plane; add the roll term before commanding a non-zero J4 angle. */
#define ARM_J1_ZERO_DEG 90.0f
#define ARM_J2_ZERO_DEG 90.0f
#define ARM_J3_ZERO_DEG 90.0f
#define ARM_J5_ZERO_DEG 90.0f
#define ARM_J1_SIGN     1.0f
#define ARM_J2_SIGN     1.0f
#define ARM_J3_SIGN     1.0f
#define ARM_J5_SIGN     1.0f

#endif /* APP_ARM_GEOMETRY_H */
