/* kinematics.c - workbook CAD forward/inverse kinematics
 *
 * owner: IK member.
 * Pure float math, no HAL and no PWM. It reads the motion calibration table
 * (servo_cal) and the shared J2/J5 coupling constants; it never calls motion.
 * Runs in the main loop only.
 *
 * FK uses the full 5-joint workbook model, including wrist roll q4:
 *   p = Rz(q1) * [ S + Ry(q2)*v_SE
 *                      + Ry(q2+q3)*( v_EW + Rz(q4)*Ry(q5)*t ) ]
 * IK uses the workbook planar closed form with q4 = 0, because cart_pose_t has
 * only one orientation scalar. See kinematics.h for the pitch projection and
 * branch/status rules.
 */
#include "kinematics/kinematics.h"
#include "kinematics/arm_geometry.h"
#include "common/app_config.h"
#include "motion/motion.h"
#include "motion/servo.h"
#include <math.h>
#include <stddef.h>

#define KIN_PI_F            3.14159265358979323846f
#define KIN_DEG2RAD         (KIN_PI_F / 180.0f)
#define KIN_RAD2DEG         (180.0f / KIN_PI_F)
#define KIN_D_TOL           1e-5f
#define KIN_R_TOL_MM        1e-4f
#define KIN_APPROACH_EPS    1e-4f

/* the IK solver is compiled when IK is activated, or for the bench test only (kin_ik_bench). the bench
 * build does NOT activate kin_ik(): cartesian and voice keep getting KIN_NOT_READY */
#if ARM_GEOMETRY_MEASURED || APP_USE_IK_BENCH_TEST
#define KIN_IK_SOLVER 1
#else
#define KIN_IK_SOLVER 0
#endif

/* FK-only helpers under ARM_FK_ENABLED, shared ones under either guard, IK-only ones under
 * KIN_IK_SOLVER, so no build leaves an unused function */
#if ARM_FK_ENABLED
typedef struct { float m[3][3]; } kin_mat3_t;
typedef struct { float x, y, z; } kin_vec3_t;

static kin_mat3_t kin_rot_z(float a)
{
  float c = cosf(a), s = sinf(a);
  kin_mat3_t r;
  r.m[0][0] = c;  r.m[0][1] = -s; r.m[0][2] = 0.0f;
  r.m[1][0] = s;  r.m[1][1] = c;  r.m[1][2] = 0.0f;
  r.m[2][0] = 0.0f; r.m[2][1] = 0.0f; r.m[2][2] = 1.0f;
  return r;
}

static kin_mat3_t kin_rot_y(float a)
{
  float c = cosf(a), s = sinf(a);
  kin_mat3_t r;
  r.m[0][0] = c;  r.m[0][1] = 0.0f; r.m[0][2] = s;
  r.m[1][0] = 0.0f; r.m[1][1] = 1.0f; r.m[1][2] = 0.0f;
  r.m[2][0] = -s; r.m[2][1] = 0.0f; r.m[2][2] = c;
  return r;
}

static kin_mat3_t kin_mat3_mul(kin_mat3_t a, kin_mat3_t b)
{
  kin_mat3_t r;
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      r.m[i][j] = a.m[i][0] * b.m[0][j] + a.m[i][1] * b.m[1][j] + a.m[i][2] * b.m[2][j];
    }
  }
  return r;
}

static kin_vec3_t kin_mat3_vec(kin_mat3_t a, kin_vec3_t v)
{
  kin_vec3_t r;
  r.x = a.m[0][0] * v.x + a.m[0][1] * v.y + a.m[0][2] * v.z;
  r.y = a.m[1][0] * v.x + a.m[1][1] * v.y + a.m[1][2] * v.z;
  r.z = a.m[2][0] * v.x + a.m[2][1] * v.y + a.m[2][2] * v.z;
  return r;
}

static kin_vec3_t kin_vec_add(kin_vec3_t a, kin_vec3_t b)
{
  kin_vec3_t r = { a.x + b.x, a.y + b.y, a.z + b.z };
  return r;
}

#endif /* ARM_FK_ENABLED */

#if ARM_FK_ENABLED || KIN_IK_SOLVER
static bool kin_joints_finite(const joint_vec_t *j)
{
  for (int i = 0; i < ARM_N_JOINTS; i++) {
    if (!isfinite((double)j->deg[i])) return false;
  }
  return true;
}

#endif /* ARM_FK_ENABLED || KIN_IK_SOLVER */

#if KIN_IK_SOLVER
static bool kin_pose_finite(const cart_pose_t *p)
{
  return isfinite((double)p->x_mm) && isfinite((double)p->y_mm) &&
         isfinite((double)p->z_mm) && isfinite((double)p->pitch_deg);
}

/* A joint angle is periodic. Normalize to [0,360), then accept the equivalent
 * representative inside the installed servo range that is closest to seed.
 * The installed ranges are at most 180 deg wide, so this yields at most one
 * candidate; the loop keeps the rule explicit. */
static bool kin_servo_candidate(int joint, float raw_deg, float seed_deg, float *out)
{
  const servo_cal_t *cal = servo_cal(joint);
  if (!isfinite((double)raw_deg) || !isfinite((double)seed_deg)) return false;
  float base = fmodf(raw_deg, 360.0f);
  if (base < 0.0f) base += 360.0f;

  bool found = false;
  float best = 0.0f, best_d = 0.0f;
  for (int k = -1; k <= 1; k++) {
    float c = base + 360.0f * (float)k;
    if (!isfinite((double)c)) continue;
    if (c < cal->min_deg || c > cal->max_deg) continue;
    float d = fabsf(c - seed_deg);
    if (!found || d < best_d) {
      found = true;
      best = c;
      best_d = d;
    }
  }
  if (found) *out = best;
  return found;
}

/* Same endpoint rule as motion.c's joint_min_deg(), evaluated for the full
 * target and the current seed so IK does not hand motion a target it will
 * clamp. */
static bool kin_coupling_ok(const joint_vec_t *seed, const float cand[5])
{
  const float s2 = seed->deg[J2_SHOULDER];
  const float s5 = seed->deg[J5_WRIST_PITCH];
  const float t2 = cand[J2_SHOULDER];
  const float t5 = cand[J5_WRIST_PITCH];

  if (t2 < MOTION_J2_COUPLED_DEG &&
      (t5 < MOTION_J5_WRIST_UP_DEG || s5 < MOTION_J5_WRIST_UP_DEG)) return false;
  if (t5 < MOTION_J5_WRIST_UP_DEG &&
      (t2 < MOTION_J2_COUPLED_DEG || s2 < MOTION_J2_COUPLED_DEG)) return false;
  return true;
}

static float kin_candidate_distance(const float cand[5], const joint_vec_t *seed)
{
  float d = 0.0f;
  for (int i = 0; i < 5; i++) {
    float e = cand[i] - seed->deg[i];
    d += e * e;
  }
  return d;
}
#endif /* KIN_IK_SOLVER */

void kin_init(void)
{
  /* All geometry is compile-time; nothing to precompute. */
}

#if KIN_IK_SOLVER
static kin_status_t ik_solve(const cart_pose_t *pose, const joint_vec_t *seed, joint_vec_t *out)
{
  if (pose == NULL || seed == NULL || out == NULL) return KIN_OUT_OF_REACH;

  joint_vec_t seed_local = *seed;
  if (!kin_pose_finite(pose) || !kin_joints_finite(&seed_local)) return KIN_OUT_OF_REACH;

  const servo_cal_t *j6cal = servo_cal(J6_GRIPPER);
  if (seed_local.deg[J6_GRIPPER] < j6cal->min_deg ||
      seed_local.deg[J6_GRIPPER] > j6cal->max_deg) return KIN_JOINT_LIMIT;

  const float l2 = sqrtf(ARM_UPPER_X_MM * ARM_UPPER_X_MM + ARM_UPPER_Z_MM * ARM_UPPER_Z_MM);
  const float l3 = sqrtf(ARM_FOREARM_X_MM * ARM_FOREARM_X_MM +
                         ARM_FOREARM_Z_MM * ARM_FOREARM_Z_MM);
  const float delta = atan2f(ARM_UPPER_X_MM, ARM_UPPER_Z_MM);
  const float y_t = ARM_SHOULDER_Y_MM + ARM_TOOL_Y_MM;

  const float x = pose->x_mm;
  const float y = pose->y_mm;
  const float z = pose->z_mm;
  const float r = sqrtf(x * x + y * y);
  if (!isfinite((double)r)) return KIN_OUT_OF_REACH;

  if (r < fabsf(y_t) - KIN_R_TOL_MM) return KIN_OUT_OF_REACH;
  if (r <= KIN_R_TOL_MM && fabsf(y_t) <= KIN_R_TOL_MM) return KIN_OUT_OF_REACH;

  float radial_sq = r * r - y_t * y_t;
  if (radial_sq < 0.0f) radial_sq = 0.0f;
  const float xp = sqrtf(radial_sq);

  /* Canonicalize any finite pitch before sin/cos; large finite values are valid. */
  const float phi_deg = fmodf(pose->pitch_deg, 360.0f);
  const float phi = phi_deg * KIN_DEG2RAD;

  const float ox = ARM_TOOL_X_MM * cosf(phi) + ARM_TOOL_Z_MM * sinf(phi);
  const float oz = -ARM_TOOL_X_MM * sinf(phi) + ARM_TOOL_Z_MM * cosf(phi);
  const float dx = xp - ox - ARM_SHOULDER_X_MM;
  const float dz = z - oz - ARM_SHOULDER_Z_MM;

  float dcos = (dx * dx + dz * dz - l2 * l2 - l3 * l3) / (2.0f * l2 * l3);
  if (!isfinite((double)dcos)) return KIN_OUT_OF_REACH;
  if (dcos < -1.0f - KIN_D_TOL || dcos > 1.0f + KIN_D_TOL) return KIN_OUT_OF_REACH;
  if (dcos < -1.0f) dcos = -1.0f;
  if (dcos > 1.0f) dcos = 1.0f;

  const float q1 = atan2f(y, x) - atan2f(y_t, xp);
  const float gamma_base = acosf(dcos);
  const float theta_base = atan2f(dx, dz);

  bool have = false;
  float best_dist = 0.0f;
  float best[5] = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };

  for (int b = 0; b < 2; b++) {
    const float branch = (b == 0) ? 1.0f : -1.0f;
    const float gamma = branch * gamma_base;
    const float theta = theta_base - atan2f(l3 * sinf(gamma), l2 + l3 * cosf(gamma));
    const float q2 = theta - delta;
    const float q3 = gamma + delta;
    const float q5 = phi - q2 - q3;

    const float raw[5] = {
      ARM_J1_ZERO_DEG + ARM_J1_SIGN * (q1 * KIN_RAD2DEG),
      ARM_J2_ZERO_DEG + ARM_J2_SIGN * (q2 * KIN_RAD2DEG),
      ARM_J3_ZERO_DEG + ARM_J3_SIGN * (q3 * KIN_RAD2DEG),
      ARM_J4_ZERO_DEG,
      ARM_J5_ZERO_DEG + ARM_J5_SIGN * (q5 * KIN_RAD2DEG)
    };
    float cand[5];
    bool ok = true;
    for (int i = 0; i < 5; i++) {
      if (!kin_servo_candidate(i, raw[i], seed_local.deg[i], &cand[i])) {
        ok = false;
        break;
      }
    }
    if (!ok || !kin_coupling_ok(&seed_local, cand)) continue;

    float dist = kin_candidate_distance(cand, &seed_local);
    if (!have || dist < best_dist) {
      have = true;
      best_dist = dist;
      for (int i = 0; i < 5; i++) best[i] = cand[i];
    }
  }

  if (!have) return KIN_JOINT_LIMIT;

  joint_vec_t result;
  for (int i = 0; i < 5; i++) result.deg[i] = best[i];
  result.deg[J6_GRIPPER] = seed_local.deg[J6_GRIPPER];
  *out = result;
  return KIN_OK;
}
#endif /* KIN_IK_SOLVER */

kin_status_t kin_ik(const cart_pose_t *pose, const joint_vec_t *seed, joint_vec_t *out)
{
#if !ARM_GEOMETRY_MEASURED
  (void)pose;
  (void)seed;
  (void)out;
  return KIN_NOT_READY;
#else
  return ik_solve(pose, seed, out);
#endif
}

#if APP_USE_IK_BENCH_TEST
kin_status_t kin_ik_bench(const cart_pose_t *pose, const joint_vec_t *seed, joint_vec_t *out)
{
  return ik_solve(pose, seed, out);
}
#endif

kin_status_t kin_fk(const joint_vec_t *joints, cart_pose_t *out)
{
#if !ARM_FK_ENABLED
  (void)joints;
  (void)out;
  return KIN_NOT_READY;
#else
  if (joints == NULL || out == NULL) return KIN_OUT_OF_REACH;

  joint_vec_t j = *joints;
  if (!kin_joints_finite(&j)) return KIN_OUT_OF_REACH;

  const float q1 = ARM_J1_SIGN * (j.deg[J1_BASE] - ARM_J1_ZERO_DEG) * KIN_DEG2RAD;
  const float q2 = ARM_J2_SIGN * (j.deg[J2_SHOULDER] - ARM_J2_ZERO_DEG) * KIN_DEG2RAD;
  const float q3 = ARM_J3_SIGN * (j.deg[J3_ELBOW] - ARM_J3_ZERO_DEG) * KIN_DEG2RAD;
  const float q4 = ARM_J4_SIGN * (j.deg[J4_WRIST_ROLL] - ARM_J4_ZERO_DEG) * KIN_DEG2RAD;
  const float q5 = ARM_J5_SIGN * (j.deg[J5_WRIST_PITCH] - ARM_J5_ZERO_DEG) * KIN_DEG2RAD;

  const kin_vec3_t shoulder = { ARM_SHOULDER_X_MM, ARM_SHOULDER_Y_MM, ARM_SHOULDER_Z_MM };
  const kin_vec3_t v_se = { ARM_UPPER_X_MM, ARM_UPPER_Y_MM, ARM_UPPER_Z_MM };
  const kin_vec3_t v_ew = { ARM_FOREARM_X_MM, ARM_FOREARM_Y_MM, ARM_FOREARM_Z_MM };
  const kin_vec3_t tool = { ARM_TOOL_X_MM, ARM_TOOL_Y_MM, ARM_TOOL_Z_MM };

  const kin_mat3_t ry2 = kin_rot_y(q2);
  const kin_mat3_t ry23 = kin_mat3_mul(ry2, kin_rot_y(q3));
  const kin_mat3_t wrist = kin_mat3_mul(kin_rot_z(q4), kin_rot_y(q5));
  const kin_mat3_t rz1 = kin_rot_z(q1);

  kin_vec3_t p = kin_vec_add(shoulder, kin_mat3_vec(ry2, v_se));
  kin_vec3_t forearm = kin_vec_add(v_ew, kin_mat3_vec(wrist, tool));
  p = kin_vec_add(p, kin_mat3_vec(ry23, forearm));
  p = kin_mat3_vec(rz1, p);

  const kin_mat3_t rot = kin_mat3_mul(rz1, kin_mat3_mul(ry23, wrist));
  const kin_vec3_t approach = { rot.m[0][2], rot.m[1][2], rot.m[2][2] };

  /* cart_pose_t carries one orientation scalar. Report the approach projected
   * into the yaw-removed X-Z plane; with roll this is not a full orientation. */
  const float px = cosf(q1) * approach.x + sinf(q1) * approach.y;
  const float pz = approach.z;
  if (sqrtf(px * px + pz * pz) < KIN_APPROACH_EPS) return KIN_NOT_READY;

  cart_pose_t result;
  result.x_mm = p.x;
  result.y_mm = p.y;
  result.z_mm = p.z;
  result.pitch_deg = atan2f(px, pz) * KIN_RAD2DEG;
  *out = result;
  return KIN_OK;
#endif
}

bool kin_fk_enabled(void) { return ARM_FK_ENABLED != 0; }
bool kin_ik_enabled(void) { return ARM_GEOMETRY_MEASURED != 0; }

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
