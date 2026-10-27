/* arm_geometry.h - CAD arm model for fk / ik
 *
 * owner: IK / mechanical member.
 *
 * Frame: robot base, right-handed, +X forward, +Y left, +Z up, mm.
 * The robot origin is the J1 axis on the floor. The workbook CAD frame has its
 * plate bottom at Fusion Y = -34.5 and the J1 axis at Fusion X = 0, Z = 18.18,
 * so a CAD point maps as:
 *   p_robot = (-X_cad, Z_cad - 18.18, Y_cad + 34.5 + foot_height)
 * with foot_height = 0 (not measured). This is the CAD candidate floor model,
 * not a claim that the installed arm has been fully measured.
 *
 * Geometry below is the single source of truth, copied from
 * Robotic_Arm_Kinematics_Workbook.xlsx (Model_Params, Joint_Axes, Inputs):
 *   S      = (23.15, 15.25, 94.22) mm
 *   v_SE   = (-0.01, 0, 113.12) mm
 *   v_EW   = (0, 0, 126.40) mm
 *   t      = (-12.99, -5.73, 104.76) mm
 *   TCP0   = (10.15, 9.52, 438.5) mm
 * The 0.01 mm shoulder lean and the non-zero tool X/Y are part of the model.
 * Do not replace these with the older hybrid 94/114/127/103.5 values.
 *
 * TCP provenance: X/Y are the workbook CAD-derived/imposed reference; Z is
 * derived from the measured 446 mm block top minus 15/2 mm. It is not an
 * independently measured tool centre.
 *
 * Servo zero/sign are candidate values from the workbook, not measured or
 * proven by joint axes. J5 is explicitly provisional in the workbook. Retest
 * on the real arm before trusting them for hardware.
 *
 * ARM_GEOMETRY_MEASURED is an activation guard, not a measurement claim.
 * 0 = both public entry points return KIN_NOT_READY. Host tests may override
 * it with -DARM_GEOMETRY_MEASURED=1; firmware must stay 0 until the owner
 * explicitly activates IK/FK.
 */
#ifndef APP_ARM_GEOMETRY_H
#define APP_ARM_GEOMETRY_H

#ifndef ARM_GEOMETRY_MEASURED
#define ARM_GEOMETRY_MEASURED 0
#endif

/* Shoulder axis point S in the robot frame. */
#define ARM_SHOULDER_X_MM    23.15f
#define ARM_SHOULDER_Y_MM    15.25f
#define ARM_SHOULDER_Z_MM    94.22f

/* Upper arm vector S -> E at the zero pose. */
#define ARM_UPPER_X_MM       (-0.01f)
#define ARM_UPPER_Y_MM       0.0f
#define ARM_UPPER_Z_MM       113.12f

/* Forearm vector E -> W at the zero pose (W = J4 roll / J5 pitch centre). */
#define ARM_FOREARM_X_MM     0.0f
#define ARM_FOREARM_Y_MM     0.0f
#define ARM_FOREARM_Z_MM     126.40f

/* Tool vector W -> TCP at the zero pose. */
#define ARM_TOOL_X_MM        (-12.99f)
#define ARM_TOOL_Y_MM        (-5.73f)
#define ARM_TOOL_Z_MM        104.76f

/* Servo angle (deg) at which each joint is at its kinematic zero; sign = +1 or -1.
 * These are candidate mappings from the workbook, not measured servo directions.
 * J4 is wrist roll, proximal to J5 pitch. IK fixes q4 = 0 at its reference. */
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
