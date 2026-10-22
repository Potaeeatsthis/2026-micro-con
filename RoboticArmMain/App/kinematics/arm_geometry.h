/* arm_geometry.h - measured arm dimensions for ik / fk
 *
 * owner: ik / mechanical member.
 * TODO(ik): measure on the real arm (guide section 22, open item 1):
 *   - joint order and rotation direction of each servo
 *   - link lengths between joint axes, in mm
 *   - the servo angle that corresponds to each joint's zero pose
 * all values below are PLACEHOLDERS so the code compiles; ik refuses to run
 * until ARM_GEOMETRY_MEASURED is 1.
 */
#ifndef APP_ARM_GEOMETRY_H
#define APP_ARM_GEOMETRY_H

#define ARM_GEOMETRY_MEASURED 0

#define ARM_BASE_HEIGHT_MM   0.0f   /* table to shoulder axis */
#define ARM_UPPER_ARM_MM     0.0f   /* shoulder axis to elbow axis */
#define ARM_FOREARM_MM       0.0f   /* elbow axis to wrist pitch axis */
#define ARM_TOOL_MM          0.0f   /* wrist pitch axis to gripper tip */

/* servo angle (deg) at which each joint is at its kinematic zero; sign = +1 or -1 */
#define ARM_J1_ZERO_DEG 90.0f
#define ARM_J2_ZERO_DEG 90.0f
#define ARM_J3_ZERO_DEG 90.0f
#define ARM_J4_ZERO_DEG 90.0f
#define ARM_J1_SIGN     1.0f
#define ARM_J2_SIGN     1.0f
#define ARM_J3_SIGN     1.0f
#define ARM_J4_SIGN     1.0f

#endif /* APP_ARM_GEOMETRY_H */
