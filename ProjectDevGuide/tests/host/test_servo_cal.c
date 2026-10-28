/* host test of App/servo_cal_test/ (the bench limit screen), built by run.sh with its own flags:
 * APP_USE_SERVO_CAL_TEST 1, MOTION 1, MODES 0, LCD 0, INPUT 1 (the input getters are faked here).
 * checks that the screen only moves joints through motion/ (speed limit, table limits, J2/J5 coupling)
 * and that a printed pose keeps the servo_cal.c min/max, never a temporary coupled bound. */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "main.h"
#include "tim.h"
#include "common/app_events.h"
#include "common/app_fault.h"
#include "motion/motion.h"
#include "motion/servo.h"
#include "servo_cal_test/servo_cal_test.h"

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
static float g_joy_y;
static int   g_joy_centred = 1;
float input_joy_y(void) { return g_joy_y; }
bool  input_joy_centered(void) { return g_joy_centred != 0; }

/* ---------------------------------------------------------------- helpers */
static int fails, passes;
#define CHECK(cond, msg) do { if (cond) { passes++; } else { fails++; printf("  FAIL: %s (line %d)\n", msg, __LINE__); } } while (0)
#define BOOT_MS (MOTION_BOOT_STAGGER_MS * ARM_N_JOINTS + 100u)

static float cur(int j) { joint_vec_t v; motion_get_current(&v); return v.deg[j]; }
static float tgt(int j) { joint_vec_t v; motion_get_target(&v); return v.deg[j]; }
static void run_ms(uint32_t ms) { for (uint32_t t = 0; t < ms; t += 20) { g_tick += 20; motion_tick(); servo_cal_test_step(); } }
static void post(evt_type_t e, int32_t v) { app_event_post(e, 0, v, 0, 0); servo_cal_test_step(); }
static void click(int n) { post(EVT_ENC_STEP, n); }
static void press(void)   { post(EVT_ENC_PRESS, 0); }
static void select_joint(int j) { while (servo_cal_test_selected() != j) press(); }

static char g_out[8192];
static void capture_step(void)           /* one step with stdout (the LOG output) captured into g_out */
{
  fflush(stdout);
  FILE *f = tmpfile();
  int saved = dup(1);
  dup2(fileno(f), 1);
  servo_cal_test_step();
  fflush(stdout);
  dup2(saved, 1);
  close(saved);
  rewind(f);
  size_t n = fread(g_out, 1, sizeof g_out - 1, f);
  g_out[n] = 0;
  fclose(f);
}

int main(void)
{
  app_events_init();
  motion_init();
  servo_cal_test_init();

  printf("== start-up runs through motion (one joint at a time), knob ignored until then\n");
  click(5);
  run_ms(20);
  CHECK(tgt(J1_BASE) == 85.0f, "a click during start-up is ignored");
  CHECK(htim4.ccr[3] != 0 && htim3.ccr[0] == 0, "first tick: only the gripper has its home pulse");
  run_ms(BOOT_MS);
  CHECK(motion_started(), "all six joints started");
  CHECK(strcmp(servo_cal_test_status(), "Home. Turn to test limits") == 0, "screen says home");
  CHECK(cur(J1_BASE) == 85.0f && cur(J2_SHOULDER) == 125.0f && cur(J3_ELBOW) == 145.0f &&
        cur(J4_WRIST_ROLL) == 90.0f && cur(J5_WRIST_PITCH) == 25.0f && cur(J6_GRIPPER) == 40.0f, "every joint at home");

  printf("== one click = 1 deg, through motion's speed limit\n");
  CHECK(servo_cal_test_step_deg() == 1 && servo_cal_test_selected() == J1_BASE, "starts at 1 deg/click on J1");
  click(1);
  CHECK(tgt(J1_BASE) == 86.0f, "one click: target +1 deg");
  run_ms(20);
  CHECK(cur(J1_BASE) > 85.0f && cur(J1_BASE) < 85.3f, "after one 20 ms tick it has moved only a little (speed limit)");
  run_ms(500);
  CHECK(cur(J1_BASE) == 86.0f && htim3.ccr[0] == servo_deg_to_us(J1_BASE, 86.0f), "reaches 86 deg, pulse from servo_cal");

  printf("== joystick press toggles 1 <-> 5 deg\n");
  post(EVT_JOY_PRESS, 0);
  CHECK(servo_cal_test_step_deg() == 5, "press: 5 deg/click");
  click(2);
  CHECK(tgt(J1_BASE) == 96.0f, "two clicks at 5: +10 deg");
  post(EVT_JOY_PRESS, 0);
  CHECK(servo_cal_test_step_deg() == 1, "press again: back to 1 deg/click");
  run_ms(2000);

  printf("== table limit: J6 stops at its 60 deg max\n");
  select_joint(J6_GRIPPER);
  post(EVT_JOY_PRESS, 0);                              /* 5 deg */
  click(4);
  CHECK(tgt(J6_GRIPPER) == 60.0f, "40 + 20 = 60 accepted");
  click(1);
  CHECK(tgt(J6_GRIPPER) == 60.0f, "65 clamped to 60");
  CHECK(strstr(servo_cal_test_status(), "MAX 60") != NULL, "screen says MAX 60");
  run_ms(3000);
  CHECK(cur(J6_GRIPPER) == 60.0f, "gripper reaches 60, not beyond");

  printf("== J2/J5 coupling comes from motion: J2 stops at 90 while J5 is low\n");
  select_joint(J2_SHOULDER);
  click(-10);                                          /* 125 - 50 = 75 wanted */
  CHECK(tgt(J2_SHOULDER) == 90.0f, "J2 75 clamped to 90 (J5 at 25)");
  CHECK(strncmp(servo_cal_test_status(), "J2 MIN 90", 9) == 0, "screen names the coupling");
  run_ms(5000);
  CHECK(cur(J2_SHOULDER) == 90.0f, "J2 stops at 90");

  printf("== printed pose keeps the table min/max\n");
  g_joy_y = -1.0f; g_joy_centred = 0;
  capture_step();
  CHECK(strstr(g_out, "POSE #1") != NULL, "joystick down prints POSE #1");
  CHECK(strstr(g_out, "{ \"J2 SHOULDER\", 500, 1430, 2440, 50.0f, 180.0f, 90.0f },") != NULL,
        "J2 row: table min 50 (not the coupled 90), home = current 90");
  CHECK(strstr(g_out, "{ \"J5 WRIST PITCH\", 640, 1580, 2470, 25.0f, 140.0f, 25.0f },") != NULL,
        "J5 row: table min 25");
  CHECK(strstr(g_out, "{ \"J6 GRIPPER\", 500, 1380, 2300, 0.0f, 60.0f, 60.0f },") != NULL, "J6 row");
  capture_step();
  CHECK(strstr(g_out, "POSE #2") == NULL, "held down: no second print");
  g_joy_y = 0.0f; g_joy_centred = 1;
  servo_cal_test_step();
  g_joy_y = -1.0f; g_joy_centred = 0;
  capture_step();
  CHECK(strstr(g_out, "POSE #2") != NULL, "back to centre, down again: POSE #2");

  /* last: a fatal fault latches until reset */
  printf("== fatal fault after start-up: FAULT shown, knob ignored\n");
  g_joy_y = 0.0f; g_joy_centred = 1;
  select_joint(J1_BASE);
  run_ms(3000);
  float t1 = tgt(J1_BASE);
  app_fault_raise(FAULT_INIT, true);
  servo_cal_test_step();
  CHECK(strncmp(servo_cal_test_status(), "FAULT 2 - RESET BOARD", 21) == 0, "status names the fault");
  click(3);
  CHECK(tgt(J1_BASE) == t1, "a click after the fault does not change the target");
  CHECK(strncmp(servo_cal_test_status(), "FAULT", 5) == 0, "and does not overwrite the FAULT status");
  press();
  CHECK(servo_cal_test_selected() == J1_BASE, "a press after the fault does not change the joint");
  g_joy_y = -1.0f; g_joy_centred = 0;
  capture_step();
  CHECK(strstr(g_out, "POSE #") == NULL, "joystick down does not print");
  g_joy_y = 0.0f; g_joy_centred = 1;
  run_ms(1000);
  CHECK(strncmp(servo_cal_test_status(), "FAULT", 5) == 0, "FAULT stays on screen");

  printf("== fatal fault before start-up ends: FAULT, not \"Going home...\"\n");
  motion_init();                                       /* a new start-up while the fault is still latched */
  run_ms(BOOT_MS);
  CHECK(!motion_started(), "motion does not start under a fatal fault");
  CHECK(strncmp(servo_cal_test_status(), "FAULT", 5) == 0, "screen shows FAULT, not Going home");

  printf("\n%d passed, %d failed\n", passes, fails);
  return fails ? 1 : 0;
}
