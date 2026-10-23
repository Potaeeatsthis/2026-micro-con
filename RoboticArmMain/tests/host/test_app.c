/* host behaviour test (gcc on a pc, NOT built by CubeIDE): fake HAL with controllable
 * pins and clock, then drives the real App/ code through the safety-relevant scenarios.
 * run: sh tests/host/run.sh. if a test fails after your change, the change broke a safety rule. */
/* host behaviour test of the RoboticArmMain App/ skeleton (not part of the deliverable) */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "main.h"
#include "tim.h"
#include "usart.h"
#include "common/app_events.h"
#include "common/app_fault.h"
#include "safety/safety.h"
#include "motion/motion.h"
#include "motion/servo.h"
#include "modes/modes.h"
#include "lcd/lcd.h"

/* pi_link.c included so the test can inject bytes into its private rx ring */
#include "pi_link/pi_link.c"

/* ---------------------------------------------------------------- fake hal */
GPIO_TypeDef fGPIOB = {1}, fGPIOC = {2}, fGPIOE = {4};
TIM_TypeDef fTIM3, fTIM4, fTIM6;
TIM_HandleTypeDef htim3 = { &fTIM3, {0} }, htim4 = { &fTIM4, {0} }, htim6 = { &fTIM6, {0} };
static USART_TypeDef fUSART6;
UART_HandleTypeDef huart6 = { &fUSART6 };

static uint32_t g_tick = 1000;
static int g_pin_low[3][16];          /* [port][pin index] 1 = reads low (pressed) */
static char g_tx[128];

static int port_i(GPIO_TypeDef *p) { return p == GPIOB ? 0 : (p == GPIOC ? 1 : 2); }
static int pin_i(uint16_t pin) { int i = 0; while (pin > 1u) { pin >>= 1; i++; } return i; }
GPIO_PinState HAL_GPIO_ReadPin(GPIO_TypeDef *p, uint16_t pin)
{ return g_pin_low[port_i(p)][pin_i(pin)] ? GPIO_PIN_RESET : GPIO_PIN_SET; }
void HAL_GPIO_WritePin(GPIO_TypeDef *p, uint16_t pin, GPIO_PinState s) { (void)p; (void)pin; (void)s; }
void HAL_GPIO_TogglePin(GPIO_TypeDef *p, uint16_t pin) { (void)p; (void)pin; }
uint32_t HAL_GetTick(void) { return g_tick; }
HAL_StatusTypeDef HAL_TIM_PWM_Start(TIM_HandleTypeDef *h, uint32_t ch) { (void)h; (void)ch; return HAL_OK; }
HAL_StatusTypeDef HAL_UARTEx_ReceiveToIdle_DMA(UART_HandleTypeDef *h, uint8_t *p, uint16_t n) { (void)h; (void)p; (void)n; return HAL_OK; }
HAL_StatusTypeDef HAL_UART_Transmit_DMA(UART_HandleTypeDef *h, const uint8_t *p, uint16_t n)
{ (void)h; memcpy(g_tx, p, n); g_tx[n] = 0; return HAL_OK; }
HAL_StatusTypeDef HAL_UART_AbortReceive(UART_HandleTypeDef *h) { (void)h; return HAL_OK; }

/* voice stubs over the real generated class order (App/voice/kws_classes.h) */
#include "voice/kws_classes.h"
int voice_class_id(const char *n) { for (int i = 0; i < N_CLASSES; i++) if (!strcmp(CLASS_NAMES[i], n)) return i; return -1; }
const char *voice_class_name(int id) { return (id >= 0 && id < N_CLASSES) ? CLASS_NAMES[id] : "?"; }
int voice_last_word(void) { return -1; }

/* ---------------------------------------------------------------- helpers */
static int fails, passes;
#define CHECK(cond, msg) do { if (cond) { passes++; } else { fails++; printf("  FAIL: %s (line %d)\n", msg, __LINE__); } } while (0)

static void set_pin(GPIO_TypeDef *p, uint16_t pin, int low) { g_pin_low[port_i(p)][pin_i(pin)] = low; }
static void loop_pass(void) { safety_step(); pi_link_step(); modes_step(); }
static int g_trace;
static float tgt(int j);
static float cur(int j);
static void run_ms(uint32_t ms)
{
  for (uint32_t t = 0; t < ms; t += 20) {
    float b = tgt(5);
    g_tick += 20; safety_tick(); motion_tick(); loop_pass();
    if (g_trace && tgt(5) != b) printf("  t=%lu J6 target %.6f -> %.6f (cur %.6f)\n", (unsigned long)g_tick, (double)b, (double)tgt(5), (double)cur(5));
  }
}
static float cur(int j) { joint_vec_t v; motion_get_current(&v); return v.deg[j]; }
static float tgt(int j) { joint_vec_t v; motion_get_target(&v); return v.deg[j]; }
static void say(const char *w) { app_event_post(EVT_VOICE_WORD, (uint16_t)voice_class_id(w), 900, 0, 0); loop_pass(); }
static void pi_rx(const char *s)
{
  for (const char *c = s; *c; c++) { s_rx[s_head] = (uint8_t)*c; pi_link_on_rx_event((uint16_t)(s_head + 1u)); }
  loop_pass();
}
static const char *screen_line(int i)
{
  static char lines[LCD_DETAIL_LINES][LCD_COLS];
  memset(lines, 0, sizeof lines);
  modes_describe(lines, LCD_DETAIL_LINES);
  return lines[i];
}
static int count_events_type(uint16_t type)
{
  /* peeks by draining into a temp array and re-posting */
  app_event_t buf[40]; int n = 0, hit = 0;
  while (n < 40 && app_event_pop(&buf[n])) { if (buf[n].type == type) hit++; n++; }
  for (int i = 0; i < n; i++) app_event_post((evt_type_t)buf[i].type, buf[i].arg, buf[i].v[0], buf[i].v[1], buf[i].v[2]);
  return hit;
}

int main(void)
{
  printf("== event queue\n");
  app_events_init();
  for (int i = 0; i < 40; i++) app_event_post(EVT_ENC_STEP, 0, i, 0, 0);
  CHECK(app_events_dropped() == 8, "40 posts into 32 slots drop 8");
  app_event_t e; app_event_pop(&e);
  CHECK(e.v[0] == 0, "fifo order");
  app_events_drain(); app_events_init();

  printf("== servo calibration\n");
  CHECK(servo_deg_to_us(0, 0.0f) == 616, "0 deg -> 616 us");
  CHECK(servo_deg_to_us(0, 90.0f) == 1523, "90 deg -> 1523 us");
  CHECK(servo_deg_to_us(0, 180.0f) == 2400, "180 deg -> 2400 us");
  CHECK(servo_deg_to_us(0, 250.0f) == 2400, "out of range clamps");

  printf("== boot: no pulse until safety allows, then home\n");
  set_pin(ESTOP_NO_GPIO_Port, ESTOP_NO_Pin, 1);          /* boot with e-stop latched */
  safety_init(); motion_init(); pi_link_init(); lcd_init(); modes_init();
  run_ms(100);
  CHECK(htim3.ccr[0] == 0, "e-stop latched at boot: still no pulse");
  set_pin(ESTOP_NO_GPIO_Port, ESTOP_NO_Pin, 0);
  run_ms(100);
  CHECK(safety_state() == SAFETY_RESTART_REQUIRED, "after e-stop release: restart must be confirmed");
  CHECK(htim3.ccr[0] == 0, "still no pulse before confirm");
  app_event_post0(EVT_TOUCH_CONFIRM); run_ms(40);
  CHECK(safety_state() == SAFETY_OK, "confirm touch button clears the latch");
  CHECK(htim3.ccr[0] == 1523, "first pulse = home pose (90 deg)");

  printf("== voice mode: no hold-to-enable needed, never moves on its own\n");
  CHECK(modes_current() == MODE_VOICE, "boots in voice mode");
  run_ms(500);
  CHECK(cur(0) == 90.0f, "idle voice mode does not move");

  printf("== switch to joint group: blocked while enable held\n");
  set_pin(BTN_ENABLE_GPIO_Port, BTN_ENABLE_Pin, 1);
  run_ms(60);
  app_event_post0(EVT_BTN_MODE); run_ms(40);   /* voice -> cartesian */
  CHECK(modes_switch_pending(), "switch waits while enable is held");
  CHECK(strcmp(screen_line(1), "RELEASE ENABLE") == 0, "lcd shows why it waits");
  set_pin(BTN_ENABLE_GPIO_Port, BTN_ENABLE_Pin, 0);
  run_ms(40);
  CHECK(modes_current() == MODE_CARTESIAN, "switched after release");
  app_event_post0(EVT_BTN_MODE); run_ms(60);
  CHECK(modes_current() == MODE_JOINT_GROUP, "next press -> joint group");

  printf("== manual motion needs hold-to-enable\n");
  motion_set_joint_target(J1_BASE, 100.0f);
  run_ms(1000);
  CHECK(cur(0) == 90.0f, "no enable: target frozen, no motion");
  CHECK(tgt(0) == 90.0f, "no enable: target reset to current (nothing jumps later)");

  set_pin(BTN_ENABLE_GPIO_Port, BTN_ENABLE_Pin, 1);
  run_ms(40);
  CHECK(safety_enable_held(), "enable held after 2 stable ticks");
  motion_set_joint_target(J1_BASE, 100.0f);
  run_ms(500);
  CHECK(cur(0) > 94.5f && cur(0) < 95.5f, "fine speed 10 deg/s: ~5 deg in 0.5 s");

  printf("== enable released mid-move: freezes in the same tick\n");
  set_pin(BTN_ENABLE_GPIO_Port, BTN_ENABLE_Pin, 0);
  safety_on_exti(BTN_ENABLE_Pin);                 /* exti fires */
  float at = cur(0);
  run_ms(20);
  CHECK(cur(0) == at, "no further step after release");
  CHECK(tgt(0) == at, "target frozen at the current angle");

  printf("== jog timeout: a stale jog stops by itself\n");
  set_pin(BTN_ENABLE_GPIO_Port, BTN_ENABLE_Pin, 1);
  run_ms(40);
  motion_jog(J2_SHOULDER, 10.0f);                  /* sent once, never refreshed */
  float j2 = cur(1);
  run_ms(1000);
  CHECK(cur(1) - j2 < 2.5f, "jog not refreshed: stops after ~150 ms, not 10 deg");

  printf("== fast speed toggle + arrival event\n");
  app_events_drain();
  motion_toggle_speed();
  motion_set_joint_target(J3_ELBOW, 120.0f);
  for (int i = 0; i < 60; i++) { g_tick += 20; safety_tick(); motion_tick(); }
  CHECK(cur(2) == 120.0f, "fast 45 deg/s reaches 120 within 1.2 s");
  CHECK(count_events_type(EVT_MOTION_DONE) == 1, "exactly one EVT_MOTION_DONE");
  motion_toggle_speed();

  printf("== e-stop during motion\n");
  motion_set_joint_target(J3_ELBOW, 60.0f);
  run_ms(200);
  set_pin(ESTOP_NO_GPIO_Port, ESTOP_NO_Pin, 1);
  safety_on_exti(ESTOP_NO_Pin);
  at = cur(2);
  run_ms(500);
  CHECK(cur(2) == at, "e-stop freezes");
  set_pin(ESTOP_NO_GPIO_Port, ESTOP_NO_Pin, 0);
  run_ms(200);
  CHECK(cur(2) == at && safety_state() == SAFETY_RESTART_REQUIRED, "release alone does not resume");
  safety_confirm_restart();
  run_ms(40);
  CHECK(safety_motion_allowed(), "confirm -> allowed again");
  set_pin(BTN_ENABLE_GPIO_Port, BTN_ENABLE_Pin, 0);
  run_ms(40);

  printf("== voice task state machine + pi link\n");
  modes_request(MODE_VOICE); run_ms(60);
  CHECK(modes_current() == MODE_VOICE, "back in voice mode");
  say("place");
  CHECK(strcmp(screen_line(0), "S0 IDLE") == 0, "word not valid in S0 does nothing");
  say("pick");
  CHECK(strcmp(screen_line(0), "S1 WHICH BOX?") == 0, "pick -> S1");
  say("red box");
  CHECK(strcmp(g_tx, "FIND red_box\n") == 0, "sends FIND red_box");
  pi_link_on_tx_done();
  pi_rx("OBJ blue_box 1 2 3\n");
  CHECK(strcmp(screen_line(0), "S1 WHICH BOX?") == 0, "reply for another box is ignored");
  pi_rx("OBJ red_box 182 -45 30\n");
  CHECK(strcmp(screen_line(0), "S2 CONFIRM BOX") == 0, "OBJ red_box -> S2");
  CHECK(strcmp(screen_line(1), "182 -45 30 mm") == 0, "position shown for confirmation");
  say("yes");
  CHECK(strcmp(screen_line(0), "S2 CONFIRM BOX") == 0, "yes with no ik yet: refuses to move, stays S2");
  say("stop");
  CHECK(strcmp(screen_line(0), "S0 IDLE") == 0, "stop in a waiting state cancels -> S0");

  printf("== pi timeout and late reply\n");
  say("grab"); say("green box"); pi_link_on_tx_done();
  g_tick += APP_PI_TIMEOUT_MS + 100u; loop_pass();
  CHECK(strcmp(screen_line(0), "S0 IDLE") == 0, "pi timeout -> S0");
  pi_rx("OBJ green_box 10 10 10\n");
  CHECK(strcmp(screen_line(0), "S0 IDLE") == 0, "late reply after timeout cannot start a task");
  pi_rx("garbage line here\n");
  pi_rx("OBJ red_box x y z\n");
  CHECK(1, "malformed lines do not crash");

  printf("== scripts: home, stop while moving, recovery from S10\n");
  motion_set_joint_target(J1_BASE, 60.0f); run_ms(20);   /* move away from home first (voice mode: no enable) */
  run_ms(4000);
  CHECK(cur(0) == 60.0f, "voice mode moves without hold-to-enable");
  say("home");
  CHECK(strcmp(screen_line(0), "S9 GOING HOME") == 0, "home from S0 -> moving");
  run_ms(500);
  say("stop");
  CHECK(strcmp(screen_line(0), "S10 STOPPED") == 0, "stop while moving -> S10");
  at = cur(0);
  run_ms(1000);
  CHECK(cur(0) == at, "S10: arm holds");
  say("pick");
  CHECK(strcmp(screen_line(0), "S10 STOPPED") == 0, "S10 ignores start words");

  say("home");
  run_ms(15000);   /* gripper 90->30 at 10 deg/s = 6 s, then 30 deg back home = 3 s */
  CHECK(cur(0) == 90.0f, "home from S10 reaches home");
  CHECK(strcmp(screen_line(0), "S0 IDLE") == 0, "then back to S0");
  CHECK(cur(5) == APP_GRIPPER_OPEN_DEG, "home from S10 opened the gripper");

  printf("\n%d passed, %d failed\n", passes, fails);
  return fails ? 1 : 0;
}
