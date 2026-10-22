/* kinematics.h - cartesian <-> joint angles
 *
 * owner: ik member.
 * pure math: no HAL, no globals besides constants, so it can be unit-tested on a pc
 * (compile kinematics.c with gcc and call it from a small main()).
 * runs in the main loop only, never in an isr.
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
