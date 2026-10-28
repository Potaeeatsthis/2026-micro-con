/* test_cartesian.c - host checks for the input-enabled Cartesian path
 *
 * owner: modes / IK.
 * test_app.c is built with APP_USE_INPUT=0, so it cannot exercise the joystick XY path. This test
 * calls the real MODE_CARTESIAN_OPS enter/step/on_event directly with a fake joystick, real
 * motion / servo / calibration / kinematics and the real gripper code. lcd_show_message is a stub.
 *
 * Checks: XY changes only the expected axis in one 20 ms step; centring the stick issues no new
 * request (an already queued target is not cancelled); a refusal holds the current pose and the next
 * XY request rebases on that held pose, not on the cancelled request; voice kin_ik() stays blocked;
 * entering/moving Cartesian does not touch the gripper. No HAL, no board.
 */
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "main.h"
#include "tim.h"
#include "common/app_events.h"
#include "common/app_fault.h"
#include "safety/safety.h"
#include "motion/motion.h"
#include "motion/servo.h"
#include "kinematics/kinematics.h"
#include "kinematics/arm_geometry.h"
#include "modes/mode_ops.h"
#include "lcd/lcd.h"

/* ---------------------------------------------------------------- fakes */
GPIO_TypeDef fGPIOB = {1}, fGPIOC = {2}, fGPIOE = {4};
TIM_TypeDef fTIM3, fTIM4, fTIM6;
TIM_HandleTypeDef htim3 = { &fTIM3, {0} }, htim4 = { &fTIM4, {0} }, htim6 = { &fTIM6, {0} };
static uint32_t g_tick = 1000;
uint32_t HAL_GetTick(void) { return g_tick; }
GPIO_PinState HAL_GPIO_ReadPin(GPIO_TypeDef *p, uint16_t pin) { (void)p; (void)pin; return GPIO_PIN_SET; }
void HAL_GPIO_WritePin(GPIO_TypeDef *p, uint16_t pin, GPIO_PinState s) { (void)p; (void)pin; (void)s; }
void HAL_GPIO_TogglePin(GPIO_TypeDef *p, uint16_t pin) { (void)p; (void)pin; }
HAL_StatusTypeDef HAL_TIM_PWM_Start(TIM_HandleTypeDef *h, uint32_t ch) { (void)h; (void)ch; return HAL_OK; }

static float g_joy_x, g_joy_y;
float input_joy_x(void) { return g_joy_x; }
float input_joy_y(void) { return g_joy_y; }

static char g_msg[32];
void lcd_show_message(const char *msg, uint32_t duration_ms)
{
  (void)duration_ms;
  snprintf(g_msg, sizeof g_msg, "%s", msg);
}

/* ---------------------------------------------------------------- helpers */
static int fails, passes;
#define CHECK(cond, msg) do { if (cond) { passes++; } else { fails++; printf("  FAIL: %s (line %d)\n", (msg), __LINE__); } } while (0)
#define BOOT_MS (MOTION_BOOT_STAGGER_MS * ARM_N_JOINTS + 100u)

static void run_motion_ms(uint32_t ms)
{
  for (uint32_t t = 0; t < ms; t += 20) { g_tick += 20; motion_tick(); }
}
static void control_tick(void)
{
  g_tick += 20;
  motion_tick();
  MODE_CARTESIAN_OPS.step();
}
static void target_fk(cart_pose_t *p)
{
  joint_vec_t j;
  motion_get_target(&j);
  CHECK(kin_fk(&j, p) == KIN_OK, "target FK");
}
static void current_fk(cart_pose_t *p)
{
  joint_vec_t j;
  motion_get_current(&j);
  CHECK(kin_fk(&j, p) == KIN_OK, "current FK");
}
static int same_joints(const joint_vec_t *a, const joint_vec_t *b)
{
  for (int j = 0; j < ARM_N_JOINTS; j++) if (fabsf(a->deg[j] - b->deg[j]) > 0.001f) return 0;
  return 1;
}

int main(void)
{
  const joint_vec_t setup = { .deg = { 85.0f, 125.0f, 157.0f, 88.0f, 35.0f, 40.0f } };
  joint_vec_t cur, tgt;
  cart_pose_t p0, p1, held, ahead, retry;
  app_event_t e;

  app_events_init();
  motion_init();
  kin_init();
  run_motion_ms(BOOT_MS);

  printf("== setup: near-bench clear pose, Cartesian ready, voice still blocked\n");
  CHECK(motion_set_target_exact(&setup) == MOTION_OK, "setup pose accepted");
  run_motion_ms(4000);
  motion_get_current(&cur);
  CHECK(fabsf(cur.deg[J3_ELBOW] - setup.deg[J3_ELBOW]) < 0.01f, "setup pose reached");
  MODE_CARTESIAN_OPS.enter();
  CHECK(kin_ik_cartesian_enabled() && !kin_ik_enabled(), "Cartesian on, global kin_ik off");
  motion_get_target(&tgt);
  CHECK(tgt.deg[J6_GRIPPER] == setup.deg[J6_GRIPPER], "entry leaves the gripper alone");
  target_fk(&p0);
  {
    joint_vec_t seed = setup, out;
    cart_pose_t target = p0;
    target.x_mm += 1.0f;
    CHECK(kin_ik(&target, &seed, &out) == KIN_NOT_READY, "global kin_ik() still refuses");
  }

  printf("== XY: one 20 ms step moves only the commanded axis\n");
  g_joy_x = 0.0f; g_joy_y = 1.0f;
  control_tick();
  target_fk(&p1);
  CHECK(p1.x_mm > p0.x_mm + 0.2f && fabsf(p1.y_mm - p0.y_mm) < 0.2f,
        "stick up moves forward (+X), not Y");
  p0 = p1;

  g_joy_x = 1.0f; g_joy_y = 0.0f;
  control_tick();
  target_fk(&p1);
  CHECK(p1.y_mm < p0.y_mm - 0.2f && fabsf(p1.x_mm - p0.x_mm) < 0.2f,
        "stick right moves to the arm's right (-Y), not X");
  p0 = p1;

  printf("== centre: no new request; an already queued target is not cancelled\n");
  g_joy_x = 0.0f; g_joy_y = 0.0f;
  motion_get_target(&tgt);
  joint_vec_t before = tgt;
  control_tick();
  motion_get_target(&tgt);
  CHECK(same_joints(&before, &tgt), "centred stick sets no new request");
  run_motion_ms(200);
  control_tick();                                      /* still centred: refresh dt, issue nothing */
  motion_get_target(&tgt);
  CHECK(same_joints(&before, &tgt), "the queued target still holds while the arm moves");

  printf("== refusal: no hold, the last accepted target stays and the next request continues from it\n");
  {
    joint_vec_t accepted;
    e.type = EVT_ENC_STEP; e.arg = 0; e.v[0] = -1; e.v[1] = 0; e.v[2] = 0;
    MODE_CARTESIAN_OPS.on_event(&e);
    MODE_CARTESIAN_OPS.on_event(&e);                   /* two accepted requests ahead of the arm */
    target_fk(&ahead);
    motion_get_target(&accepted);
    e.v[0] = 100;                                      /* 500 mm: unreachable */
    MODE_CARTESIAN_OPS.on_event(&e);
    CHECK(strcmp(g_msg, "OUT OF REACH") == 0, "the extreme request is refused");
    motion_get_target(&tgt);
    CHECK(same_joints(&accepted, &tgt), "refusal keeps the last accepted target (no hold, no jerk)");
    g_joy_x = 0.0f; g_joy_y = 0.5f;                    /* half up = +X */
    control_tick();                                    /* the arm still trails the target: nothing new */
    motion_get_target(&tgt);
    CHECK(same_joints(&accepted, &tgt), "while the joints trail the target, the stick adds nothing");
    g_joy_y = 0.0f;
    run_motion_ms(3000);                               /* the arm finishes the accepted target */
    current_fk(&held);
    CHECK(fabsf(held.z_mm - ahead.z_mm) < 0.5f, "the arm reaches the last accepted pose (Z -10)");
    g_joy_y = 0.5f;
    control_tick();
    target_fk(&retry);
    CHECK(fabsf(retry.z_mm - ahead.z_mm) < 0.5f && retry.x_mm > ahead.x_mm + 0.02f,
          "the next request continues +X from the accepted pose");
    g_joy_y = 0.0f;
    CHECK(tgt.deg[J6_GRIPPER] == setup.deg[J6_GRIPPER], "no automatic gripper command on approach");
  }

  printf("== smooth joystick: the target never runs far ahead, no joint asked to go faster than its limit\n");
  {
    float worst_lead = 0.0f, worst_step = 0.0f;
    joint_vec_t prev;
    run_motion_ms(1000);
    control_tick();                                    /* stick centred: only refreshes the step's clock */
    motion_get_target(&prev);
    g_joy_x = 0.0f; g_joy_y = 1.0f;                    /* full forward, FINE: 20 mm/s requested */
    for (int i = 0; i < 25; i++) {                     /* 0.5 s */
      control_tick();
      motion_get_current(&cur);
      motion_get_target(&tgt);
      for (int j = 0; j < J6_GRIPPER; j++) {
        float lead = fabsf(tgt.deg[j] - cur.deg[j]), st = fabsf(tgt.deg[j] - prev.deg[j]);
        if (lead > worst_lead) worst_lead = lead;
        if (st > worst_step) worst_step = st;
      }
      prev = tgt;
    }
    g_joy_y = 0.0f;
    CHECK(worst_step <= 10.0f * 0.02f + 0.01f, "each request moves no joint more than 10 deg/s * 20 ms");
    CHECK(worst_lead <= 10.0f * 0.15f + 10.0f * 0.02f + 0.01f, "the target leads the joints by <= ~1.7 deg");
    cart_pose_t at_release, stopped;
    current_fk(&at_release);
    run_motion_ms(1000);
    current_fk(&stopped);
    CHECK(fabsf(stopped.x_mm - at_release.x_mm) < 3.0f, "after the stick is let go the arm stops within 3 mm");
  }

  printf("== blocked (J4 far from the reference): a control attempt shows why, nothing moves\n");
  {
    char align[24];
    snprintf(align, sizeof align, "ALIGN J4 %d", (int)ARM_J4_ZERO_DEG);
    g_joy_x = 0.0f; g_joy_y = 0.0f;
    MODE_CARTESIAN_OPS.exit();
    motion_set_joint_target(J4_WRIST_ROLL, 120.0f);
    run_motion_ms(4000);
    MODE_CARTESIAN_OPS.enter();
    motion_get_target(&before);
    g_msg[0] = '\0';
    e.type = EVT_ENC_STEP; e.v[0] = -1;
    MODE_CARTESIAN_OPS.on_event(&e);
    CHECK(strcmp(g_msg, align) == 0, "knob turn while blocked: shows ALIGN J4");
    g_msg[0] = '\0';
    g_joy_y = 0.5f;
    control_tick();
    g_joy_y = 0.0f;
    CHECK(strcmp(g_msg, align) == 0, "stick push while blocked: shows ALIGN J4");
    motion_get_target(&tgt);
    CHECK(same_joints(&before, &tgt), "and nothing moves");
    MODE_CARTESIAN_OPS.exit();
  }

  printf("== joint groups: stick axes (operator view) and directions, set on the arm 28 Oct 2026\n");
  {
    /* {joint on X (right), joint on Y (up)} and the direction a push right / up moves it */
    static const int   AX[3][2] = { { J1_BASE, J2_SHOULDER }, { J4_WRIST_ROLL, J3_ELBOW }, { J6_GRIPPER, J5_WRIST_PITCH } };
    static const float SG[3][2] = { { -1.0f, 1.0f }, { -1.0f, -1.0f }, { -1.0f, 1.0f } };
    joint_vec_t t0, t1;
    MODE_JOINT_GROUP_OPS.enter();
    run_motion_ms(200);
    for (int g = 0; g < 3; g++) {
      if (g > 0) {
        app_event_t sel = { .type = EVT_ENC_STEP, .v = { 1, 0, 0 } };
        MODE_JOINT_GROUP_OPS.on_event(&sel);              /* next group */
        run_motion_ms(100);
      }
      for (int a = 0; a < 2; a++) {
        motion_get_target(&t0);
        g_joy_x = (a == 0) ? 1.0f : 0.0f; g_joy_y = (a == 1) ? 1.0f : 0.0f;
        MODE_JOINT_GROUP_OPS.step();
        run_motion_ms(100);
        motion_get_target(&t1);
        g_joy_x = 0.0f; g_joy_y = 0.0f;
        int other = AX[g][1 - a];
        float d = t1.deg[AX[g][a]] - t0.deg[AX[g][a]];
        char m[72];
        snprintf(m, sizeof m, "group %d stick %s: J%d %s, J%d still", g + 1, a ? "up" : "right", AX[g][a] + 1,
                 SG[g][a] > 0 ? "up" : "down", other + 1);
        CHECK((SG[g][a] > 0 ? d > 0.5f : d < -0.5f) && fabsf(t1.deg[other] - t0.deg[other]) < 0.01f, m);
        MODE_JOINT_GROUP_OPS.step();                    /* centred: stop the jog */
        run_motion_ms(200);
      }
    }
    MODE_JOINT_GROUP_OPS.exit();
  }

  printf("\n%d passed, %d failed\n", passes, fails);
  return fails ? 1 : 0;
}
