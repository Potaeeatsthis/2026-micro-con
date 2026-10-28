/* arm_geometry.h - arm model for FK / IK
 *
 * Robot frame: origin at the J1 axis on the floor, +X forward, +Y left, +Z up, mm.
 * Values come from Robotic_Arm_Kinematics_Workbook.xlsx (CAD). CAD point -> robot frame:
 *   p_robot = (-X_cad, Z_cad - 18.18, Y_cad + 34.5)
 * TCP z = the measured 446 mm block top - 7.5. The servo zero / sign values are from the workbook,
 * not measured (J5 is provisional).
 *
 * Guards: ARM_FK_ENABLED (LCD X/Y/Z), ARM_CARTESIAN_IK_ENABLED, ARM_VOICE_IK_ENABLED, and
 * ARM_GEOMETRY_MEASURED for the global kin_ik() (off, nothing uses it). The IK bench build
 * turns all normal IK callers off.
 */
#ifndef APP_ARM_GEOMETRY_H
#define APP_ARM_GEOMETRY_H

#ifndef ARM_FK_ENABLED
#define ARM_FK_ENABLED 1
#endif
#ifndef ARM_CARTESIAN_IK_ENABLED
#define ARM_CARTESIAN_IK_ENABLED 1       /* don't change without the owner */
#endif
#ifndef ARM_VOICE_IK_ENABLED
#define ARM_VOICE_IK_ENABLED 1           /* don't change without the owner */
#endif
#ifndef ARM_GEOMETRY_MEASURED
#define ARM_GEOMETRY_MEASURED 0          /* don't change without the owner */
#endif

/* shoulder axis point S */
#define ARM_SHOULDER_X_MM    23.15f
#define ARM_SHOULDER_Y_MM    15.25f
#define ARM_SHOULDER_Z_MM    94.22f

/* upper arm S -> E at zero pose */
#define ARM_UPPER_X_MM       (-0.01f)
#define ARM_UPPER_Y_MM       0.0f
#define ARM_UPPER_Z_MM       113.12f

/* forearm E -> W at zero pose (W = wrist centre) */
#define ARM_FOREARM_X_MM     0.0f
#define ARM_FOREARM_Y_MM     0.0f
#define ARM_FOREARM_Z_MM     126.40f

/* tool W -> TCP at zero pose */
#define ARM_TOOL_X_MM        (-12.99f)
#define ARM_TOOL_Y_MM        (-5.73f)
#define ARM_TOOL_Z_MM        104.76f

/* servo angle at each joint's zero, and its sign (workbook values, not measured) */
#define ARM_J1_ZERO_DEG      85.0f
#define ARM_J2_ZERO_DEG      145.0f
#define ARM_J3_ZERO_DEG      88.0f
#define ARM_J4_ZERO_DEG      88.0f
#define ARM_J5_ZERO_DEG      96.0f
#define ARM_J1_SIGN          1.0f
#define ARM_J2_SIGN          (-1.0f)
#define ARM_J3_SIGN          1.0f
#define ARM_J4_SIGN          1.0f
#define ARM_J5_SIGN          (-1.0f)

#endif /* APP_ARM_GEOMETRY_H */
