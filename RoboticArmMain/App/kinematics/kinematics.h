/* kinematics.h - cartesian <-> joint angles
 *
 * owner: ik member.
 * pure math: no HAL, no globals besides constants, so it can be unit-tested on a pc
 * (compile kinematics.c with gcc and call it from a small main()).
 * runs in the main loop only, never in an isr.
 *
 * coordinates: every cart_pose_t is in the robot-base frame, in mm and degrees:
 *   right-handed; origin at the base axis on the mounting/base-bottom plane;
 *   +X forward, +Y left, +Z up (the frame the pi reports).
 *   CAD direction vectors map as (-X_cad, Z_cad, Y_cad) (CAD is Y-up).
 *   for points, first subtract the robot origin expressed in CAD coordinates:
 *     p_robot = R * (p_cad - origin_cad), R * (X,Y,Z) = (-X,Z,Y).
 *   origin_cad = (0, -30.5, 18.18) mm maps the workbook CAD to this
 *   mounting-plane frame. The floor is lower by an unmeasured stand offset;
 *   measured floor->J2 = 94 mm, CAD base-bottom->J2 = 90.22 mm. Do not treat
 *   this mounting-plane transform as a confirmed floor transform.
 *   at the straight-up zero pose the positive joint axes (right-hand rule) are
 *   J1 +Z base yaw, J2 +Y shoulder pitch, J3 +Y elbow pitch,
 *   J4 +Z wrist roll, J5 +Y wrist pitch. J6 is the gripper, not in positioning ik.
 *   these are chosen model signs, not measured servo or PWM directions.
 */
#ifndef APP_KINEMATICS_H
#define APP_KINEMATICS_H

#include "common/app_types.h"

typedef enum {
  KIN_OK = 0,
  KIN_OUT_OF_REACH,         /* show OUT OF REACH, keep the last reachable target */
  KIN_JOINT_LIMIT,          /* a solution exists but breaks a joint limit */
  KIN_NOT_READY             /* geometry not measured yet (ARM_GEOMETRY_MEASURED 0) */
} kin_status_t;

void         kin_init(void);

/* pose -> joints. seed = current joints, used to pick the closest of several solutions.
 * out is written only when KIN_OK. out->deg[J6_GRIPPER] is copied from seed */
kin_status_t kin_ik(const cart_pose_t *pose, const joint_vec_t *seed, joint_vec_t *out);

/* joints -> pose (used to start cartesian mode from where the arm is now) */
kin_status_t kin_fk(const joint_vec_t *joints, cart_pose_t *out);

const char  *kin_status_name(kin_status_t s);

#endif /* APP_KINEMATICS_H */
