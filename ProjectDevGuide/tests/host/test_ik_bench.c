/* host test of App/ik_bench_test/ (one supervised IK move), built by run.sh with its own flags:
 * APP_USE_IK_BENCH_TEST 1, MOTION 1, MODES 0, LCD 0, INPUT 1, KINEMATICS 1, and ARM_GEOMETRY_MEASURED
 * left at its firmware default 0. checks that kin_ik() stays off, that the screen moves only through
 * motion_set_target_exact() to the one IK solution, that a press stops it, and that it goes home.
 * the knob goes through the real input.c (pin sampled in input_tick, event posted by input_step), so
 * arming is tested on the press EDGE time, including a late main loop and a knob held across start-up. */
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "main.h"
#include "tim.h"
#include "common/app_events.h"
#include "common/app_fault.h"
#include "kinematics/arm_geometry.h"
#include "kinematics/kinematics.h"
#include "motion/motion.h"
#include "motion/servo.h"
#include "ik_bench_test/ik_bench_test.h"
#include "input/input.h"
#include "adc.h"

/* ---------------------------------------------------------------- fakes */
GPIO_TypeDef fGPIOB = {1}, fGPIOC = {2}, fGPIOE = {4};
TIM_TypeDef fTIM3, fTIM4, fTIM6;
TIM_HandleTypeDef htim3 = { &fTIM3, {0} }, htim4 = { &fTIM4, {0} }, htim6 = { &fTIM6, {0} };
static uint32_t g_tick = 1000;
uint32_t HAL_GetTick(void) { return g_tick; }
static int g_knob_down;                                /* KY-040 switch, active low */
GPIO_PinState HAL_GPIO_ReadPin(GPIO_TypeDef *p, uint16_t pin)
{
  return (p == ENC_SW_GPIO_Port && pin == ENC_SW_Pin && g_knob_down) ? GPIO_PIN_RESET : GPIO_PIN_SET;
}
void HAL_GPIO_WritePin(GPIO_TypeDef *p, uint16_t pin, GPIO_PinState s) { (void)p; (void)pin; (void)s; }
void HAL_GPIO_TogglePin(GPIO_TypeDef *p, uint16_t pin) { (void)p; (void)pin; }
HAL_StatusTypeDef HAL_TIM_PWM_Start(TIM_HandleTypeDef *h, uint32_t ch) { (void)h; (void)ch; return HAL_OK; }
TIM_HandleTypeDef htim1 = { &fTIM3, {0} }, htim2 = { &fTIM3, {0} };
uint32_t fake_tim_counter(TIM_HandleTypeDef *h) { (void)h; return 0u; }   /* encoder not turned */
ADC_HandleTypeDef hadc1;
HAL_StatusTypeDef HAL_TIM_Encoder_Start(TIM_HandleTypeDef *h, uint32_t ch) { (void)h; (void)ch; return HAL_OK; }
HAL_StatusTypeDef HAL_TIM_Base_Start(TIM_HandleTypeDef *h) { (void)h; return HAL_OK; }
HAL_StatusTypeDef HAL_ADC_Start_DMA(ADC_HandleTypeDef *h, uint32_t *p, uint32_t n) { (void)h; (void)p; (void)n; return HAL_OK; }

/* ---------------------------------------------------------------- helpers */
static int fails, passes;
#define CHECK(cond, msg) do { if (cond) { passes++; } else { fails++; printf("  FAIL: %s (line %d)\n", msg, __LINE__); } } while (0)
#define BOOT_MS (MOTION_BOOT_STAGGER_MS * ARM_N_JOINTS + 100u)

static float cur(int j) { joint_vec_t v; motion_get_current(&v); return v.deg[j]; }
static float tgt(int j) { joint_vec_t v; motion_get_target(&v); return v.deg[j]; }
/* one 20 ms control tick (TIM6 isr: input_tick, motion_tick), then optionally one main-loop pass */
static void isr_tick(void) { g_tick += 20; input_tick(); motion_tick(); }
static void loop_pass(void) { input_step(); ik_bench_test_step(); }
static void run_ms(uint32_t ms) { for (uint32_t t = 0; t < ms; t += 20) { isr_tick(); loop_pass(); } }
static void isr_only_ms(uint32_t ms) { for (uint32_t t = 0; t < ms; t += 20) isr_tick(); }   /* main loop stuck */
static void knob_click(void) { g_knob_down = 1; run_ms(80); g_knob_down = 0; run_ms(80); }   /* through input.c */
static void post(evt_type_t e) { app_event_post(e, 0, 0, 0, 0); ik_bench_test_step(); }
static int at(const joint_vec_t *v)
{
  for (int j = 0; j < ARM_N_JOINTS; j++) if (fabsf(cur(j) - v->deg[j]) > 0.01f) return 0;
  return 1;
}

int main(void)
{
  joint_vec_t home, sol;
  for (int j = 0; j < ARM_N_JOINTS; j++) home.deg[j] = servo_cal(j)->home_deg;
  app_events_init();
  motion_init();
  input_init();
  kin_init();
  ik_bench_test_init();

  printf("== IK stays off for everything except this screen\n");
  {
    cart_pose_t p = { 250.0f, 13.0f, 120.0f, 150.0f };
    joint_vec_t out = home;
    CHECK(ARM_GEOMETRY_MEASURED == 0 && !kin_ik_enabled(), "activation guard unchanged (0)");
    CHECK(kin_ik(&p, &home, &out) == KIN_NOT_READY, "kin_ik() still refuses: cartesian / voice stay blocked");
    CHECK(kin_ik_bench(&p, &home, &out) == KIN_OK, "kin_ik_bench() solves the bench target");
  }

  printf("== start-up: home first, presses ignored, a knob held across start-up is not GO\n");
  g_knob_down = 1;                                     /* held from power-on */
  run_ms(20);
  CHECK(ik_bench_test_state() == IKB_BOOT, "still booting");
  while (!motion_started()) {                          /* tick until the last joint gets its home pulse */
    isr_tick();
    if (!motion_started()) loop_pass();
  }
  app_event_post(EVT_ENC_PRESS, 0, 0, 0, 0);           /* captured on the final start-up tick, still queued */
  loop_pass();                                         /* the pass that arms */
  CHECK(ik_bench_test_state() == IKB_READY, "a press queued at the end of start-up is not GO");
  run_ms(100);
  CHECK(ik_bench_test_state() == IKB_READY && tgt(J3_ELBOW) == home.deg[J3_ELBOW], "nothing moves");
  post(EVT_ENC_PRESS);                                 /* 100 ms after READY: inside the arming time */
  CHECK(ik_bench_test_state() == IKB_READY, "a press right after arming is ignored too");
  run_ms(1500);
  CHECK(ik_bench_test_state() == IKB_READY && at(&home), "knob still held 1.6 s after READY: no new edge, no GO");
  g_knob_down = 0;                                     /* released: still no GO */
  run_ms(200);
  CHECK(ik_bench_test_state() == IKB_READY && at(&home), "releasing it is not GO either");
  CHECK(motion_started() && at(&home), "every joint at home");
  CHECK(ik_bench_test_state() == IKB_READY, "ready after start-up");
  CHECK(strcmp(ik_bench_test_status(), "KNOB PRESS = GO") == 0, "screen asks for a press");
  CHECK(motion_get_speed() == SPEED_FINE, "fine speed");
  ik_bench_test_solution(&sol);
  CHECK(fabsf(sol.deg[J1_BASE] - 85.8f) < 0.2f && fabsf(sol.deg[J2_SHOULDER] - 125.9f) < 0.2f &&
        fabsf(sol.deg[J3_ELBOW] - 157.5f) < 0.2f && fabsf(sol.deg[J4_WRIST_ROLL] - 88.0f) < 0.2f &&
        fabsf(sol.deg[J5_WRIST_PITCH] - 34.6f) < 0.2f, "IK solution 85.8 125.9 157.5 88 34.6");
  CHECK(sol.deg[J6_GRIPPER] == home.deg[J6_GRIPPER], "the gripper is not part of the move");

  printf("== a new press = go, fine speed, arrives at the solution\n");
  g_knob_down = 1;
  run_ms(60);                                          /* debounced edge seen, event posted and handled */
  g_knob_down = 0;
  CHECK(ik_bench_test_state() == IKB_MOVING, "a new knob press (through input.c) = GO");
  CHECK(fabsf(tgt(J3_ELBOW) - sol.deg[J3_ELBOW]) < 0.001f, "target = the IK solution");
  run_ms(200);
  CHECK(cur(J3_ELBOW) > home.deg[J3_ELBOW] && cur(J3_ELBOW) <= home.deg[J3_ELBOW] + 2.7f, "J3 moves at most 10 deg/s");
  run_ms(3000);
  CHECK(ik_bench_test_state() == IKB_AT_TARGET, "arrived");
  CHECK(at(&sol), "the arm is at the solution");
  {
    joint_vec_t now;
    cart_pose_t est;
    motion_get_current(&now);
    CHECK(kin_fk(&now, &est) == KIN_OK && fabsf(est.x_mm - 250.0f) < 0.5f && fabsf(est.y_mm - 13.0f) < 0.5f &&
          fabsf(est.z_mm - 120.0f) < 0.5f, "FK of the reached pose = the target (round trip)");
  }
  CHECK(cur(J6_GRIPPER) == home.deg[J6_GRIPPER], "gripper untouched");

  printf("== press = home, then ready again\n");
  post(EVT_JOY_PRESS);
  CHECK(ik_bench_test_state() == IKB_AT_TARGET, "a stick press does not start the way home");
  post(EVT_ENC_PRESS);
  CHECK(ik_bench_test_state() == IKB_RETURNING, "going home");
  for (int i = 0; i < 400 && ik_bench_test_state() != IKB_READY; i++) run_ms(20);
  CHECK(at(&home), "back at home");
  CHECK(ik_bench_test_state() == IKB_READY, "ready again");

  printf("== an early press handled late (main loop stuck past the window) is not GO\n");
  isr_only_ms(100);                                    /* READY just appeared; the main loop stops running */
  g_knob_down = 1;
  isr_only_ms(80);                                     /* press edge about 140 ms after READY */
  g_knob_down = 0;
  isr_only_ms(600);                                    /* still not handled, now past IK_BENCH_ARM_MS */
  run_ms(100);                                         /* the loop runs again: the old press is posted now */
  CHECK(ik_bench_test_state() == IKB_READY && at(&home), "judged by its edge time: ignored, nothing moves");
  knob_click();
  CHECK(ik_bench_test_state() == IKB_MOVING, "a new press after that = GO");

  printf("== any press while moving stops; press again = home\n");
  run_ms(240);
  post(EVT_JOY_PRESS);
  CHECK(ik_bench_test_state() == IKB_STOPPED, "stick press stops it");
  {
    float j3 = cur(J3_ELBOW);
    CHECK(tgt(J3_ELBOW) == j3, "target = where it is (held)");
    run_ms(1000);
    CHECK(cur(J3_ELBOW) == j3 && j3 > home.deg[J3_ELBOW] && j3 < sol.deg[J3_ELBOW], "stays stopped half way");
  }
  post(EVT_ENC_PRESS);
  CHECK(ik_bench_test_state() == IKB_RETURNING, "press = home");
  run_ms(200);
  post(EVT_ENC_PRESS);
  CHECK(ik_bench_test_state() == IKB_STOPPED, "the way home can be stopped too");
  post(EVT_ENC_PRESS);
  run_ms(3000);
  CHECK(at(&home) && ik_bench_test_state() == IKB_READY, "home and ready");

  printf("== motion would clamp: nothing moves (exact target)\n");
  {
    joint_vec_t bad = home;
    bad.deg[J2_SHOULDER] = 60.0f;                       /* below 90 with J5 at 25: coupled */
    CHECK(motion_set_target_exact(&bad) == MOTION_CLAMPED, "exact setter reports the clamp");
    CHECK(tgt(J2_SHOULDER) == home.deg[J2_SHOULDER], "and leaves the target unchanged");
    run_ms(200);
    CHECK(at(&home), "nothing moved");
  }

  /* last: a fatal fault latches until reset */
  printf("== fatal fault: FAULT shown, presses ignored\n");
  app_fault_raise(FAULT_INIT, true);
  run_ms(20);
  CHECK(ik_bench_test_state() == IKB_FAULT, "fault state");
  post(EVT_ENC_PRESS);
  run_ms(200);
  CHECK(ik_bench_test_state() == IKB_FAULT && at(&home), "a press after the fault does nothing");

  printf("\n%d passed, %d failed\n", passes, fails);
  return fails ? 1 : 0;
}
