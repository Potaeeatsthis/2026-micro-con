/* test_kinematics.c - host checks for App/kinematics
 *
 * owner: IK / mechanical member.
 * Three builds of the same file:
 *   default                       ARM_GEOMETRY_MEASURED=0, ARM_CARTESIAN_IK_ENABLED=1, ARM_VOICE_IK_ENABLED=1:
 *                                 global kin_ik fail-closed, Cartesian and voice IK available, FK available
 *   -DARM_CARTESIAN_IK_ENABLED=0 -DARM_VOICE_IK_ENABLED=0  every normal IK API fail-closed, FK available
 *   -DARM_GEOMETRY_MEASURED=1 -DARM_CARTESIAN_IK_ENABLED=0 -DARM_VOICE_IK_ENABLED=0
 *                                 the workbook CAD FK/IK math, linked with -lm
 * No HAL, no motion hardware. Run: sh ProjectDevGuide/tests/host/run.sh
 */
#include <math.h>
#include <stdio.h>
#include "kinematics/arm_geometry.h"
#include "kinematics/kinematics.h"
#include "motion/servo.h"

/* Host shim for the real accessor in motion/servo.c. The calibration table
 * itself is the real servo_cal.c object; only the tiny accessor is provided
 * here so this pure-math test does not need the PWM/HAL part of servo.c. */
extern const servo_cal_t SERVO_CAL[ARM_N_JOINTS];
const servo_cal_t *servo_cal(int joint)
{
  if (joint < 0 || joint >= ARM_N_JOINTS) return &SERVO_CAL[0];
  return &SERVO_CAL[joint];
}

static int checks;
static int fails;

#define CHECK(cond, msg) do { \
  checks++; \
  if (!(cond)) { fails++; printf("FAIL: %s (line %d)\n", (msg), __LINE__); } \
} while (0)

#define CHECK_NEAR(got, want, tol, msg) \
  CHECK(fabsf((got) - (want)) <= (tol), (msg))

static void set_sentinel_joint(joint_vec_t *v)
{
  for (int i = 0; i < ARM_N_JOINTS; i++) v->deg[i] = -123.0f - (float)i;
}

static int joint_is_sentinel(const joint_vec_t *v)
{
  for (int i = 0; i < ARM_N_JOINTS; i++) {
    if (v->deg[i] != -123.0f - (float)i) return 0;
  }
  return 1;
}

static void set_sentinel_pose(cart_pose_t *p)
{
  p->x_mm = -999.0f;
  p->y_mm = -998.0f;
  p->z_mm = -997.0f;
  p->pitch_deg = -996.0f;
}

static int pose_is_sentinel(const cart_pose_t *p)
{
  return p->x_mm == -999.0f && p->y_mm == -998.0f &&
         p->z_mm == -997.0f && p->pitch_deg == -996.0f;
}

static int finish(void)
{
  if (fails) {
    printf("== kinematics: FAIL (%d/%d checks)\n", fails, checks);
    return 1;
  }
  printf("== kinematics: %d checks passed\n", checks);
  return 0;
}

#if !ARM_GEOMETRY_MEASURED

int main(void)
{
  const joint_vec_t seed = { .deg = { 85.0f, 125.0f, 145.0f, 90.0f, 25.0f, 30.0f } };
  const cart_pose_t pose = { 100.0f, 20.0f, 300.0f, 180.0f };
  joint_vec_t out;
  cart_pose_t p;

  kin_init();

  set_sentinel_joint(&out);
  CHECK(kin_ik(&pose, &seed, &out) == KIN_NOT_READY, "default IK is fail-closed");
  CHECK(joint_is_sentinel(&out), "default IK leaves out unchanged");

  set_sentinel_joint(&out);
  CHECK(kin_ik(NULL, NULL, &out) == KIN_NOT_READY,
        "default IK checks the guard before null inputs");
  CHECK(joint_is_sentinel(&out), "default IK null call leaves out unchanged");

  CHECK(kin_fk_enabled() && !kin_ik_enabled(), "default: FK available, global kin_ik not activated");
#if ARM_CARTESIAN_IK_ENABLED
  CHECK(kin_ik_cartesian_enabled(), "default: Cartesian IK is activated");
#else
  CHECK(!kin_ik_cartesian_enabled(), "Cartesian IK guard is 0: Cartesian is disabled");
#endif

  /* FK at the candidate reference angles (model q = 0) gives the workbook TCP0 */
  const joint_vec_t ref = { .deg = { ARM_J1_ZERO_DEG, ARM_J2_ZERO_DEG, ARM_J3_ZERO_DEG,
                                     ARM_J4_ZERO_DEG, ARM_J5_ZERO_DEG, 0.0f } };
  set_sentinel_pose(&p);
  CHECK(kin_fk(&ref, &p) == KIN_OK, "default FK works");
  CHECK(fabsf(p.x_mm - 10.15f) < 0.02f && fabsf(p.y_mm - 9.52f) < 0.02f && fabsf(p.z_mm - 438.50f) < 0.02f,
        "reference 85/145/88/88/96 -> estimated 10.15, 9.52, 438.50 mm");

  set_sentinel_pose(&p);
  CHECK(kin_fk(NULL, &p) == KIN_OUT_OF_REACH, "default FK rejects a null input");
  CHECK(pose_is_sentinel(&p), "default FK null call leaves out unchanged");

  /* the same reference pose as an IK request: voice/global IK stays refused, Cartesian may solve */
  cart_pose_t ref_pose;
  set_sentinel_pose(&ref_pose);
  CHECK(kin_fk(&ref, &ref_pose) == KIN_OK, "reference FK for the IK target");
  set_sentinel_joint(&out);
  CHECK(kin_ik(&ref_pose, &ref, &out) == KIN_NOT_READY && joint_is_sentinel(&out),
        "global kin_ik of the FK result is still refused by default");

#if ARM_CARTESIAN_IK_ENABLED
  {
    joint_vec_t cart_out;
    set_sentinel_joint(&cart_out);
    CHECK(kin_ik_cartesian(&ref_pose, &ref, &cart_out) == KIN_OK,
          "Cartesian IK solves the FK reference pose");
    for (int i = 0; i < 5; i++) {
      CHECK_NEAR(cart_out.deg[i], ref.deg[i], 0.1f, "Cartesian round-trip joint angle");
    }
    CHECK_NEAR(cart_out.deg[J6_GRIPPER], ref.deg[J6_GRIPPER], 0.0f,
               "Cartesian IK copies J6 from the seed");
    set_sentinel_joint(&cart_out);
    CHECK(kin_ik_cartesian(NULL, NULL, &cart_out) == KIN_OUT_OF_REACH,
          "Cartesian IK checks arguments once activated");
    CHECK(joint_is_sentinel(&cart_out), "Cartesian null call leaves out unchanged");

    /* Optional diagnostics: same status/out rules, plus the closest failed branch's reason.
     * The target poses below are FK of deliberately out-of-range joint sets. */
    {
      kin_diag_t d;
      cart_pose_t target;

      set_sentinel_joint(&cart_out);
      CHECK(kin_ik_cartesian_diag(&ref_pose, &ref, &cart_out, &d) == KIN_OK,
            "diagnostic wrapper still solves a good pose");
      CHECK(d.status == KIN_OK && d.reason == KIN_DIAG_NONE && d.joint == -1 && d.branch == -1,
            "success clears the diagnostic");

      const joint_vec_t low_j2 = { .deg = { 85.0f, 45.0f, 88.0f, 88.0f, 96.0f, 30.0f } };
      CHECK(kin_fk(&low_j2, &target) == KIN_OK, "low-J2 target FK");
      set_sentinel_joint(&cart_out);
      CHECK(kin_ik_cartesian_diag(&target, &ref, &cart_out, &d) == KIN_JOINT_LIMIT,
            "low-J2 target refused");
      CHECK(joint_is_sentinel(&cart_out), "low-J2 diagnostic leaves out unchanged");
      CHECK(d.reason == KIN_DIAG_JOINT_MIN && d.joint == J2_SHOULDER &&
            fabsf(d.limit_deg - 50.0f) < 0.01f && d.branch >= 0,
            "diagnostic names J2 and its 50 deg minimum");

      const joint_vec_t low_j5 = { .deg = { 85.0f, 125.0f, 145.0f, 88.0f, 16.0f, 30.0f } };
      CHECK(kin_fk(&low_j5, &target) == KIN_OK, "low-J5 target FK");
      set_sentinel_joint(&cart_out);
      CHECK(kin_ik_cartesian_diag(&target, &ref, &cart_out, &d) == KIN_JOINT_LIMIT,
            "low-J5 target refused");
      CHECK(joint_is_sentinel(&cart_out), "low-J5 diagnostic leaves out unchanged");
      CHECK(d.reason == KIN_DIAG_JOINT_MIN && d.joint == J5_WRIST_PITCH &&
            fabsf(d.limit_deg - 25.0f) < 0.01f && d.branch >= 0,
            "diagnostic names J5 and its 25 deg minimum");

      const joint_vec_t low_couple = { .deg = { 85.0f, 65.0f, 88.0f, 88.0f, 76.0f, 30.0f } };
      const joint_vec_t home_seed = { .deg = { 85.0f, 125.0f, 145.0f, 88.0f, 25.0f, 30.0f } };
      CHECK(kin_fk(&low_couple, &target) == KIN_OK, "coupling target FK");
      set_sentinel_joint(&cart_out);
      CHECK(kin_ik_cartesian_diag(&target, &home_seed, &cart_out, &d) == KIN_JOINT_LIMIT,
            "coupling target refused");
      CHECK(joint_is_sentinel(&cart_out), "coupling diagnostic leaves out unchanged");
      CHECK(d.reason == KIN_DIAG_COUPLING && d.joint == J2_SHOULDER &&
            fabsf(d.limit_deg - 90.0f) < 0.01f && d.branch >= 0,
            "diagnostic names the J2 coupling floor when J5 is low");

      const joint_vec_t high_j2 = { .deg = { 85.0f, 125.0f, 88.0f, 88.0f, 40.0f, 30.0f } };
      const joint_vec_t low_j2_seed = { .deg = { 85.0f, 80.0f, 88.0f, 88.0f, 70.0f, 30.0f } };
      CHECK(kin_fk(&high_j2, &target) == KIN_OK, "J5 coupling target FK");
      set_sentinel_joint(&cart_out);
      CHECK(kin_ik_cartesian_diag(&target, &low_j2_seed, &cart_out, &d) == KIN_JOINT_LIMIT,
            "J5 coupling target refused");
      CHECK(d.reason == KIN_DIAG_COUPLING && d.joint == J5_WRIST_PITCH &&
            fabsf(d.limit_deg - 65.0f) < 0.01f && d.branch >= 0,
            "diagnostic names the J5 coupling floor when J2 is low");

      cart_pose_t far = { 1000.0f, 0.0f, 0.0f, 0.0f };
      set_sentinel_joint(&cart_out);
      CHECK(kin_ik_cartesian_diag(&far, &ref, &cart_out, &d) == KIN_OUT_OF_REACH,
            "unreachable target refused by the diagnostic path");
      CHECK(joint_is_sentinel(&cart_out), "geometry diagnostic leaves out unchanged");
      CHECK(d.reason == KIN_DIAG_GEOMETRY && d.joint == -1 && d.branch == -1,
            "geometry refusal says OUT OF REACH, not a joint");

      cart_pose_t bad = ref_pose;
      bad.pitch_deg = NAN;
      set_sentinel_joint(&cart_out);
      CHECK(kin_ik_cartesian_diag(&bad, &ref, &cart_out, &d) == KIN_OUT_OF_REACH,
            "non-finite pitch refused");
      CHECK(joint_is_sentinel(&cart_out), "non-finite pitch leaves out unchanged");
      CHECK(d.reason == KIN_DIAG_INVALID, "non-finite pitch says invalid, not a joint limit");
    }
  }
#else
  {
    joint_vec_t cart_out;
    kin_diag_t d;
    set_sentinel_joint(&cart_out);
    CHECK(kin_ik_cartesian(&ref_pose, &ref, &cart_out) == KIN_NOT_READY,
          "Cartesian IK is fail-closed when its guard is 0");
    CHECK(joint_is_sentinel(&cart_out), "Cartesian guard refusal leaves out unchanged");
    set_sentinel_joint(&cart_out);
    CHECK(kin_ik_cartesian_diag(&ref_pose, &ref, &cart_out, &d) == KIN_NOT_READY,
          "Cartesian diagnostic is fail-closed when its guard is 0");
    CHECK(joint_is_sentinel(&cart_out) && d.reason == KIN_DIAG_NOT_READY,
          "guard refusal reports NOT_READY and leaves out unchanged");
  }
#endif

#if ARM_VOICE_IK_ENABLED
  {
    joint_vec_t v_out;
    CHECK(kin_ik_voice_enabled(), "default: voice IK is activated (its own guard)");
    set_sentinel_joint(&v_out);
    CHECK(kin_ik_voice(&ref_pose, &ref, &v_out) == KIN_OK, "voice IK solves the FK reference pose");
    for (int i = 0; i < 5; i++) CHECK_NEAR(v_out.deg[i], ref.deg[i], 0.1f, "voice round-trip joint angle");
  }
#else
  {
    joint_vec_t v_out;
    CHECK(!kin_ik_voice_enabled(), "voice IK guard is 0: voice is disabled");
    set_sentinel_joint(&v_out);
    CHECK(kin_ik_voice(&ref_pose, &ref, &v_out) == KIN_NOT_READY && joint_is_sentinel(&v_out),
          "voice IK is fail-closed when its guard is 0");
  }
#endif

  return finish();
}

#else /* ARM_GEOMETRY_MEASURED */

static joint_vec_t servo_from_model(const float q[5], float j6)
{
  joint_vec_t s;
  s.deg[J1_BASE]         = ARM_J1_ZERO_DEG + ARM_J1_SIGN * q[0];
  s.deg[J2_SHOULDER]     = ARM_J2_ZERO_DEG + ARM_J2_SIGN * q[1];
  s.deg[J3_ELBOW]        = ARM_J3_ZERO_DEG + ARM_J3_SIGN * q[2];
  s.deg[J4_WRIST_ROLL]   = ARM_J4_ZERO_DEG + ARM_J4_SIGN * q[3];
  s.deg[J5_WRIST_PITCH]  = ARM_J5_ZERO_DEG + ARM_J5_SIGN * q[4];
  s.deg[J6_GRIPPER]      = j6;
  return s;
}

static void check_fk_vector(const float q[5], float x, float y, float z)
{
  joint_vec_t s = servo_from_model(q, 30.0f);
  cart_pose_t p;

  set_sentinel_pose(&p);
  CHECK(kin_fk(&s, &p) == KIN_OK, "CAD FK vector solves");
  if (p.x_mm == -999.0f) return;             /* leave one clear failure per vector */

  CHECK_NEAR(p.x_mm, x, 0.05f, "CAD FK x");
  CHECK_NEAR(p.y_mm, y, 0.05f, "CAD FK y");
  CHECK_NEAR(p.z_mm, z, 0.05f, "CAD FK z");
  if (q[3] == 0.0f) {
    CHECK_NEAR(p.pitch_deg, q[1] + q[2] + q[4], 0.05f, "planar FK pitch");
  } else {
    CHECK(isfinite((double)p.pitch_deg), "rolled FK pitch is finite");
    CHECK(p.pitch_deg >= -180.0f && p.pitch_deg <= 180.0f, "rolled FK pitch is bounded");
  }
}

int main(void)
{
  static const float fk_q[9][5] = {
    {   0.0f,   0.0f,   0.0f,   0.0f,   0.0f },
    { -30.0f,   0.0f,   0.0f,   0.0f,   0.0f },
    {   0.0f, -30.0f,   0.0f,   0.0f,   0.0f },
    {   0.0f,   0.0f,  30.0f,   0.0f,   0.0f },
    {   0.0f,   0.0f,   0.0f, -30.0f,   0.0f },
    {   0.0f,   0.0f,   0.0f,   0.0f, -30.0f },
    {  20.0f,  25.0f, -40.0f,  35.0f,  30.0f },
    { -50.0f,  60.0f,  45.0f, -70.0f, -25.0f },
    { 110.0f, -15.0f,  80.0f, 150.0f,  95.0f }
  };
  static const float fk_p[9][3] = {
    {  10.15f,   9.52f, 438.50f },
    {  13.55f,   3.17f, 438.50f },
    { -160.25f,  9.52f, 385.88f },
    { 127.47f,   9.53f, 414.03f },
    {   9.03f,  16.78f, 438.50f },
    { -40.49f,   9.52f, 417.97f },
    {  34.17f,  48.76f, 422.31f },
    { 266.49f, -214.96f, 118.63f },
    { -94.04f,  45.03f, 338.71f }
  };

  kin_init();

#if ARM_CARTESIAN_IK_ENABLED && !APP_USE_IK_BENCH_TEST
  CHECK(kin_ik_cartesian_enabled(), "global build: Cartesian IK is also enabled");
#else
  CHECK(!kin_ik_cartesian_enabled(), "global-only build: Cartesian IK stays off");
#endif

  for (int i = 0; i < 9; i++) {
    check_fk_vector(fk_q[i], fk_p[i][0], fk_p[i][1], fk_p[i][2]);
  }

  /* FK pitch is a projection, exact only at q4 = 0. */
  {
    const float q0[5] = { 0.0f, 20.0f, 57.0f, 0.0f, -20.0f };
    joint_vec_t s = servo_from_model(q0, 30.0f);
    cart_pose_t p;
    CHECK(kin_fk(&s, &p) == KIN_OK, "safe pose FK");
    CHECK_NEAR(p.pitch_deg, 57.0f, 0.05f, "q4=0 pitch is q2+q3+q5");
  }
  {
    const float qroll[5] = { 0.0f, 0.0f, 0.0f, 45.0f, 45.0f };
    joint_vec_t s = servo_from_model(qroll, 30.0f);
    cart_pose_t p;
    CHECK(kin_fk(&s, &p) == KIN_OK, "roll FK solves");
    CHECK_NEAR(p.pitch_deg, 35.2643897f, 0.05f, "rolled FK reports projected approach");
  }
  {
    const float qdeg[5] = { 0.0f, 0.0f, 0.0f, 90.0f, 90.0f };
    joint_vec_t s = servo_from_model(qdeg, 30.0f);
    cart_pose_t p;
    set_sentinel_pose(&p);
    CHECK(kin_fk(&s, &p) == KIN_NOT_READY, "degenerate approach projection refuses");
    CHECK(pose_is_sentinel(&p), "degenerate FK leaves out unchanged");
  }
  {
    joint_vec_t bad = servo_from_model((const float[5]){ 0, 0, 0, 0, 0 }, 30.0f);
    cart_pose_t p;
    bad.deg[J6_GRIPPER] = NAN;
    set_sentinel_pose(&p);
    CHECK(kin_fk(&bad, &p) == KIN_OUT_OF_REACH, "FK rejects non-finite J6");
    CHECK(pose_is_sentinel(&p), "invalid FK leaves out unchanged");

    set_sentinel_pose(&p);
    CHECK(kin_fk(NULL, &p) == KIN_OUT_OF_REACH, "FK rejects null input");
    CHECK(pose_is_sentinel(&p), "null FK leaves out unchanged");
    CHECK(kin_fk(&bad, NULL) == KIN_OUT_OF_REACH, "FK rejects null output");
  }

  /* Safe round trip and J6 copy. */
  const float safe_q[5] = { 0.0f, 20.0f, 57.0f, 0.0f, -20.0f };
  joint_vec_t safe_seed = servo_from_model(safe_q, 30.0f);
  cart_pose_t safe_pose;
  CHECK(kin_fk(&safe_seed, &safe_pose) == KIN_OK, "safe FK for IK target");

  {
    joint_vec_t out;
    set_sentinel_joint(&out);
    CHECK(kin_ik(&safe_pose, &safe_seed, &out) == KIN_OK, "safe IK round trip");
    for (int i = 0; i < 5; i++) {
      CHECK_NEAR(out.deg[i], safe_seed.deg[i], 0.1f, "round-trip joint angle");
    }
    CHECK_NEAR(out.deg[J6_GRIPPER], 30.0f, 0.0f, "IK copies J6 from seed");
  }

  /* out and seed may be the same object. */
  {
    joint_vec_t alias = safe_seed;
    CHECK(kin_ik(&safe_pose, &alias, &alias) == KIN_OK, "IK supports out == seed");
    CHECK_NEAR(alias.deg[J2_SHOULDER], safe_seed.deg[J2_SHOULDER], 0.1f,
               "aliased IK keeps the seed answer");
    CHECK_NEAR(alias.deg[J6_GRIPPER], 30.0f, 0.0f, "aliased IK keeps J6");
  }

  /* Both elbow branches exist and seed picks the closer one. */
  {
    const float qminus[5] = { 0.0f, 0.0f, -20.0f, 0.0f, 40.0f };
    const float qplus[5]  = { 0.0f, -21.114893f, 19.989870f, 0.0f, 21.125023f };
    joint_vec_t seed_minus = servo_from_model(qminus, 30.0f);
    joint_vec_t seed_plus = servo_from_model(qplus, 30.0f);
    cart_pose_t target;
    CHECK(kin_fk(&seed_minus, &target) == KIN_OK, "branch FK target");

    joint_vec_t out_minus;
    CHECK(kin_ik(&target, &seed_minus, &out_minus) == KIN_OK, "minus branch solves");
    for (int i = 0; i < 5; i++) {
      CHECK_NEAR(out_minus.deg[i], seed_minus.deg[i], 0.2f, "minus branch selected");
    }

    joint_vec_t out_plus;
    CHECK(kin_ik(&target, &seed_plus, &out_plus) == KIN_OK, "plus branch solves");
    for (int i = 0; i < 5; i++) {
      CHECK_NEAR(out_plus.deg[i], seed_plus.deg[i], 0.2f, "plus branch selected");
    }
  }

  /* Periodic pitch: 57, 57 +/- 360 and a large finite value are equivalent. */
  {
    cart_pose_t a = safe_pose, b = safe_pose, c = safe_pose;
    joint_vec_t oa, ob, oc;
    a.pitch_deg = 57.0f;
    b.pitch_deg = 57.0f + 360.0f * 1000.0f;
    c.pitch_deg = 57.0f - 360.0f;
    CHECK(kin_ik(&a, &safe_seed, &oa) == KIN_OK, "canonical pitch solves");
    CHECK(kin_ik(&b, &safe_seed, &ob) == KIN_OK, "huge finite pitch solves");
    CHECK(kin_ik(&c, &safe_seed, &oc) == KIN_OK, "negative equivalent pitch solves");
    for (int i = 0; i < 5; i++) {
      CHECK_NEAR(ob.deg[i], oa.deg[i], 0.05f, "pitch + 360k is equivalent");
      CHECK_NEAR(oc.deg[i], oa.deg[i], 0.05f, "pitch - 360 is equivalent");
    }
  }

  /* Geometric solutions that leave the installed ranges or coupling. */
  {
    const float qlim[5] = { 0.0f, 60.0f, 0.0f, 0.0f, 40.0f };
    joint_vec_t seed_lim = servo_from_model(qlim, 30.0f);
    cart_pose_t target;
    joint_vec_t out;
    CHECK(kin_fk(&seed_lim, &target) == KIN_OK, "limit target FK");
    set_sentinel_joint(&out);
    CHECK(kin_ik(&target, &seed_lim, &out) == KIN_JOINT_LIMIT,
          "IK rejects a solution outside the servo ranges");
    CHECK(joint_is_sentinel(&out), "limit failure leaves out unchanged");
  }
  {
    const float qcoup[5] = { 0.0f, 95.0f, 0.0f, 0.0f, 26.0f };
    const joint_vec_t home = { .deg = { 85.0f, 125.0f, 145.0f, 90.0f, 25.0f, 30.0f } };
    joint_vec_t seed_coup = servo_from_model(qcoup, 30.0f);
    cart_pose_t target;
    joint_vec_t out;
    CHECK(kin_fk(&seed_coup, &target) == KIN_OK, "coupling target FK");
    set_sentinel_joint(&out);
    CHECK(kin_ik(&target, &home, &out) == KIN_JOINT_LIMIT,
          "IK rejects a target that the current J5 would make motion clamp");
    CHECK(joint_is_sentinel(&out), "coupling failure leaves out unchanged");
  }

  /* J6 seed must be in its own installed range; IK copies it on success. */
  {
    joint_vec_t bad_j6 = safe_seed;
    joint_vec_t out;
    bad_j6.deg[J6_GRIPPER] = 61.0f;
    set_sentinel_joint(&out);
    CHECK(kin_ik(&safe_pose, &bad_j6, &out) == KIN_JOINT_LIMIT,
          "IK rejects seed J6 above its range");
    CHECK(joint_is_sentinel(&out), "J6 limit failure leaves out unchanged");

    bad_j6.deg[J6_GRIPPER] = NAN;
    CHECK(kin_ik(&safe_pose, &bad_j6, &out) == KIN_OUT_OF_REACH,
          "IK rejects non-finite seed J6");
  }

  /* Invalid and unreachable inputs keep existing statuses, no new enum. */
  {
    cart_pose_t bad = safe_pose;
    joint_vec_t out;
    set_sentinel_joint(&out);
    CHECK(kin_ik(NULL, &safe_seed, &out) == KIN_OUT_OF_REACH, "null pose");
    CHECK(joint_is_sentinel(&out), "null pose leaves out unchanged");
    CHECK(kin_ik(&safe_pose, NULL, &out) == KIN_OUT_OF_REACH, "null seed");
    CHECK(kin_ik(&safe_pose, &safe_seed, NULL) == KIN_OUT_OF_REACH, "null out");

    bad.x_mm = NAN;
    CHECK(kin_ik(&bad, &safe_seed, &out) == KIN_OUT_OF_REACH, "non-finite pose");
    bad = safe_pose;
    bad.pitch_deg = INFINITY;
    CHECK(kin_ik(&bad, &safe_seed, &out) == KIN_OUT_OF_REACH, "non-finite pitch");

    joint_vec_t bad_seed = safe_seed;
    bad_seed.deg[J1_BASE] = NAN;
    CHECK(kin_ik(&safe_pose, &bad_seed, &out) == KIN_OUT_OF_REACH, "non-finite seed");
  }
  {
    cart_pose_t far = { 1000.0f, 0.0f, 0.0f, 0.0f };
    cart_pose_t huge = { 1.0e30f, 0.0f, 0.0f, 0.0f };
    cart_pose_t near = { 0.0f, 0.0f, 300.0f, 0.0f };
    cart_pose_t tangent = { 9.52f, 0.0f, 300.0f, 0.0f };
    joint_vec_t out;
    set_sentinel_joint(&out);
    CHECK(kin_ik(&far, &safe_seed, &out) == KIN_OUT_OF_REACH, "out-of-reach target");
    CHECK(joint_is_sentinel(&out), "unreachable leaves out unchanged");
    CHECK(kin_ik(&huge, &safe_seed, &out) == KIN_OUT_OF_REACH,
          "huge finite target does not leak a NaN solution");
    CHECK(joint_is_sentinel(&out), "huge finite target leaves out unchanged");
    CHECK(kin_ik(&near, &safe_seed, &out) == KIN_OUT_OF_REACH,
          "target inside the lateral offset is unreachable");
    CHECK(kin_ik(&tangent, &safe_seed, &out) == KIN_JOINT_LIMIT,
          "tangent branch meeting is rejected by the physical servo range");
    CHECK(joint_is_sentinel(&out), "tangent failure leaves out unchanged");
  }

  return finish();
}

#endif /* ARM_GEOMETRY_MEASURED */
