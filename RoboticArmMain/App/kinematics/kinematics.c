/* kinematics.c - skeleton. replace the two TODO bodies, keep the signatures */
#include "kinematics/kinematics.h"
#include "kinematics/arm_geometry.h"

void kin_init(void)
{
  /* TODO(ik): precompute anything constant (squared link lengths, reach limits) */
}

kin_status_t kin_ik(const cart_pose_t *pose, const joint_vec_t *seed, joint_vec_t *out)
{
  (void)pose;
  (void)seed;
  (void)out;
#if !ARM_GEOMETRY_MEASURED
  return KIN_NOT_READY;
#else
  /* TODO(ik): base yaw from atan2(y, x), then 2-link planar solution for shoulder/elbow,
   * wrist pitch (J5) from the requested tool pitch; check every joint against servo_cal limits.
   * J4 wrist roll is not in this planar model; hold it at its reference angle (see arm_geometry.h). */
  return KIN_OUT_OF_REACH;
#endif
}

kin_status_t kin_fk(const joint_vec_t *joints, cart_pose_t *out)
{
  (void)joints;
  (void)out;
#if !ARM_GEOMETRY_MEASURED
  return KIN_NOT_READY;
#else
  /* TODO(ik): forward chain of the same model as kin_ik; ik(fk(q)) == q is the unit test */
  return KIN_NOT_READY;
#endif
}

const char *kin_status_name(kin_status_t s)
{
  switch (s) {
    case KIN_OK:           return "OK";
    case KIN_OUT_OF_REACH: return "OUT OF REACH";
    case KIN_JOINT_LIMIT:  return "JOINT LIMIT";
    case KIN_NOT_READY:    return "IK NOT READY";
    default:               return "?";
  }
}
