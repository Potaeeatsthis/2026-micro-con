/* kinematics.h - cartesian <-> joint angles
 *
 * owner: IK member.
 * pure math: no HAL, no PWM, no globals besides constants. Uses the motion
 * calibration getters only for installed servo limits and coupling constants.
 * Runs in the main loop only, never in an ISR. It never commands motion.
 *
 * Coordinates: every cart_pose_t is in the robot-base frame, mm and degrees:
 *   right-handed; origin at the J1 axis on the floor; +X forward, +Y left,
 *   +Z up. The frame and floor transform are in arm_geometry.h.
 *
 * joint_vec_t is the servo frame, 0..180 deg. Model angle conversion uses the
 * candidate zero/sign table in arm_geometry.h:
 *   q = sign * (servo - zero)
 *
 * Status and output rules:
 *   KIN_OK            out is written.
 *   KIN_OUT_OF_REACH  null/non-finite/invalid input, or no geometric solution.
 *                     out is unchanged.
 *   KIN_JOINT_LIMIT   a geometric solution exists, but a servo limit, the J2/J5
 *                     coupling, or the seed J6 range fails. out is unchanged.
 *   KIN_NOT_READY     IK: activation guard off (ARM_GEOMETRY_MEASURED 0).
 *                     FK: disabled (ARM_FK_ENABLED 0), or it cannot express
 *                     pitch from a degenerate approach projection.
 *                     out is unchanged.
 *
 * FK returns a full 5-joint position (including roll q4). cart_pose_t has only
 * one orientation scalar, so pitch_deg is the projected approach direction in
 * the yaw-removed X-Z plane:
 *   atan2(approach_x, approach_z)
 * It is exact only when q4 = 0. With roll it is a projection, not a full tool
 * orientation. If that projection is below ~1e-4, FK returns KIN_NOT_READY
 * rather than inventing a pitch.
 *
 * IK is the workbook planar closed form with q4 fixed at 0 (servo reference).
 * It uses only the positive in-plane radial branch; other radial/roll solutions
 * are not enumerated. Both elbow branches are checked and the valid one closest
 * to seed is returned. Periodic solutions are normalized to the installed servo
 * range (0..180 deg) and accepted only if they map to the closest seed. The J6
 * gripper angle is copied from seed and must be inside its calibration range.
 */
#ifndef APP_KINEMATICS_H
#define APP_KINEMATICS_H

#include "common/app_types.h"
#include "common/app_config.h"
#include <stdbool.h>

typedef enum {
  KIN_OK = 0,
  KIN_OUT_OF_REACH,         /* no geometric solution; keep the last reachable target */
  KIN_JOINT_LIMIT,          /* a solution exists but breaks a servo limit / coupling */
  KIN_NOT_READY             /* geometry disabled, or FK pitch projection degenerate */
} kin_status_t;

void         kin_init(void);

/* pose -> joints. seed = current joints, used to pick the closest elbow branch.
 * out is written only when KIN_OK. out->deg[J6_GRIPPER] is copied from seed. */
kin_status_t kin_ik(const cart_pose_t *pose, const joint_vec_t *seed, joint_vec_t *out);

/* joints -> pose (used to start cartesian mode from where the arm is now).
 * all six input angles must be finite; FK does not enforce servo software
 * limits, so the CAD/math test vectors can be checked outside the installed
 * ranges. This is a mathematical estimate, not a command validation. */
kin_status_t kin_fk(const joint_vec_t *joints, cart_pose_t *out);

const char  *kin_status_name(kin_status_t s);

#if APP_USE_IK_BENCH_TEST
/* bench test only (App/ik_bench_test/, local config): the same solver as kin_ik() but without the
 * ARM_GEOMETRY_MEASURED guard. kin_ik() itself stays KIN_NOT_READY in that build. never call it from
 * a mode: it does not exist in a normal build */
kin_status_t kin_ik_bench(const cart_pose_t *pose, const joint_vec_t *seed, joint_vec_t *out);
#endif

bool         kin_fk_enabled(void);   /* ARM_FK_ENABLED: estimated X/Y/Z available */
bool         kin_ik_enabled(void);   /* ARM_GEOMETRY_MEASURED: IK may produce targets */

#endif /* APP_KINEMATICS_H */
