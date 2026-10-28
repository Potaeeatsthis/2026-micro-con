/* kinematics.h - cartesian <-> joint angles
 *
 * Pure math, main loop only, never commands motion.
 * Poses are in the robot-base frame (mm, deg): origin at the J1 axis on the floor, +X forward,
 * +Y left, +Z up. Joint angles are servo degrees 0..180; q = sign * (servo - zero) (arm_geometry.h).
 *
 * KIN_OK            out is written
 * KIN_OUT_OF_REACH  bad input or no geometric solution, out unchanged
 * KIN_JOINT_LIMIT   a solution breaks a servo limit or the J2/J5 coupling, out unchanged
 * KIN_NOT_READY     the caller's IK guard is off, or FK can't give a pitch
 *
 * pitch_deg is the tool direction in the vertical plane, exact only when J4 is at its reference.
 * IK keeps J4 at its reference, checks both elbow branches and returns the one closest to seed.
 * J6 is copied from seed.
 */
#ifndef APP_KINEMATICS_H
#define APP_KINEMATICS_H

#include "common/app_types.h"
#include "common/app_config.h"
#include <stdbool.h>

typedef enum {
  KIN_OK = 0,
  KIN_OUT_OF_REACH,         /* keep the last reachable target */
  KIN_JOINT_LIMIT,
  KIN_NOT_READY
} kin_status_t;

/* why kin_ik_cartesian_diag() refused: the closest failed branch and the joint / bound it broke.
 * joint / branch = -1 when not used */
typedef enum {
  KIN_DIAG_NONE = 0,
  KIN_DIAG_INVALID,
  KIN_DIAG_GEOMETRY,
  KIN_DIAG_JOINT_MIN,
  KIN_DIAG_JOINT_MAX,
  KIN_DIAG_COUPLING,
  KIN_DIAG_NOT_READY
} kin_reason_t;

typedef struct {
  kin_status_t status;
  kin_reason_t reason;
  int          joint;       /* 0..5 = J1..J6 */
  int          branch;
  float        limit_deg;
  float        value_deg;
} kin_diag_t;

void         kin_init(void);

/* pose -> joints, seed = current joints. guarded by ARM_GEOMETRY_MEASURED; the modes use the two below */
kin_status_t kin_ik(const cart_pose_t *pose, const joint_vec_t *seed, joint_vec_t *out);

/* cartesian mode IK (ARM_CARTESIAN_IK_ENABLED) */
kin_status_t kin_ik_cartesian(const cart_pose_t *pose, const joint_vec_t *seed, joint_vec_t *out);

/* same, plus why it refused */
kin_status_t kin_ik_cartesian_diag(const cart_pose_t *pose, const joint_vec_t *seed, joint_vec_t *out,
                                   kin_diag_t *diag);

/* voice mode IK (ARM_VOICE_IK_ENABLED) */
kin_status_t kin_ik_voice(const cart_pose_t *pose, const joint_vec_t *seed, joint_vec_t *out);

/* joints -> pose, an estimate from the commanded angles */
kin_status_t kin_fk(const joint_vec_t *joints, cart_pose_t *out);

const char  *kin_status_name(kin_status_t s);

#if APP_USE_IK_BENCH_TEST
/* IK bench build only, no guard */
kin_status_t kin_ik_bench(const cart_pose_t *pose, const joint_vec_t *seed, joint_vec_t *out);
#endif

bool         kin_fk_enabled(void);
bool         kin_ik_enabled(void);
bool         kin_ik_voice_enabled(void);
bool         kin_ik_cartesian_enabled(void);

#endif /* APP_KINEMATICS_H */
