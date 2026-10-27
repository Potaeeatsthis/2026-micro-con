/* host behaviour test (gcc on a pc, NOT built by CubeIDE): fake HAL with controllable
 * pins and clock, then drives the real App/ code through the safety-relevant scenarios.
 * run: sh ProjectDevGuide/tests/host/run.sh. if a test fails after your change, the change broke a safety rule. */
/* host behaviour test of the RoboticArmMain App/ skeleton (not part of the deliverable) */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
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
static uint32_t g_voice_mask = 0xFFFFFFFFu;     /* what mode_voice last told voice/ to accept */
void voice_set_accept_mask(uint32_t m) { g_voice_mask = m; }

/* ---------------------------------------------------------------- helpers */
static int fails, passes;
#define CHECK(cond, msg) do { if (cond) { passes++; } else { fails++; printf("  FAIL: %s (line %d)\n", msg, __LINE__); } } while (0)

static void loop_pass(void) { pi_link_step(); modes_step(); }
static int g_trace;
static float tgt(int j);
static float cur(int j);
static void run_ms(uint32_t ms)
{
  for (uint32_t t = 0; t < ms; t += 20) {
    float b = tgt(5);
    g_tick += 20; motion_tick(); loop_pass();
    if (g_trace && tgt(5) != b) printf("  t=%lu J6 target %.6f -> %.6f (cur %.6f)\n", (unsigned long)g_tick, (double)b, (double)tgt(5), (double)cur(5));
  }
}
static float cur(int j) { joint_vec_t v; motion_get_current(&v); return v.deg[j]; }
static float tgt(int j) { joint_vec_t v; motion_get_target(&v); return v.deg[j]; }
static void run_motion_ms(uint32_t ms)
{
  for (uint32_t t = 0; t < ms; t += 20) { g_tick += 20; motion_tick(); }
}
static motion_result_t set_pair_target(float j2_deg, float j5_deg)
{
  joint_vec_t t;
  motion_get_current(&t);
  t.deg[J2_SHOULDER] = j2_deg;
  t.deg[J5_WRIST_PITCH] = j5_deg;
  return motion_set_target(&t);
}
static void run_jog(int joint, float deg_per_s, uint32_t ms)
{
  for (uint32_t t = 0; t < ms; t += 20) { motion_jog(joint, deg_per_s); g_tick += 20; motion_tick(); }
  for (uint32_t t = 0; t < 200; t += 20) { g_tick += 20; motion_tick(); }  /* timeout + final step */
}
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

static void check_servo_cal(int joint, uint16_t us0, uint16_t us90, uint16_t us180)
{
  char msg[48];
  snprintf(msg, sizeof msg, "J%d 0 deg pulse", joint + 1);
  CHECK(servo_deg_to_us(joint, 0.0f) == us0, msg);
  snprintf(msg, sizeof msg, "J%d 90 deg pulse", joint + 1);
  CHECK(servo_deg_to_us(joint, 90.0f) == us90, msg);
  snprintf(msg, sizeof msg, "J%d 180 deg pulse", joint + 1);
  CHECK(servo_deg_to_us(joint, 180.0f) == us180, msg);
  snprintf(msg, sizeof msg, "J%d 45 deg midpoint pulse", joint + 1);
  CHECK(servo_deg_to_us(joint, 45.0f) == (uint16_t)((us0 + us90) / 2u), msg);
  snprintf(msg, sizeof msg, "J%d 135 deg midpoint pulse", joint + 1);
  CHECK(servo_deg_to_us(joint, 135.0f) == (uint16_t)((us90 + us180) / 2u), msg);
}

static void check_cal_limits_home(int joint, float min_deg, float max_deg,
                                  float home_deg, uint16_t home_us)
{
  char msg[64];
  const servo_cal_t *c = servo_cal(joint);
  snprintf(msg, sizeof msg, "J%d safe min limit", joint + 1);
  CHECK(c->min_deg == min_deg, msg);
  snprintf(msg, sizeof msg, "J%d safe max limit", joint + 1);
  CHECK(c->max_deg == max_deg, msg);
  snprintf(msg, sizeof msg, "J%d home angle", joint + 1);
  CHECK(c->home_deg == home_deg, msg);
  snprintf(msg, sizeof msg, "J%d home pulse", joint + 1);
  CHECK(servo_deg_to_us(joint, c->home_deg) == home_us, msg);
}

static void check_joint_clamp(int joint, float min_deg, float max_deg)
{
  char msg[64];
  snprintf(msg, sizeof msg, "J%d below-min target clamps", joint + 1);
  CHECK(motion_set_joint_target(joint, min_deg - 10.0f) == MOTION_CLAMPED &&
        tgt(joint) == min_deg, msg);
  snprintf(msg, sizeof msg, "J%d above-max target clamps", joint + 1);
  CHECK(motion_set_joint_target(joint, max_deg + 10.0f) == MOTION_CLAMPED &&
        tgt(joint) == max_deg, msg);
  snprintf(msg, sizeof msg, "J%d boundaries are accepted", joint + 1);
  CHECK(motion_set_joint_target(joint, min_deg) == MOTION_OK &&
        tgt(joint) == min_deg &&
        motion_set_joint_target(joint, max_deg) == MOTION_OK &&
        tgt(joint) == max_deg, msg);
}

#if APP_VOICE_DRY_RUN
/* second build (run.sh, -DAPP_VOICE_DRY_RUN=1): ik is not measured yet, so only a dry run can
 * reach S4 HOLDING and walk "hand" and "station" end to end. moves are read back from the dry-run log. */
static FILE *g_log;
static int   g_saved_stdout = -1;
static void log_capture(void)
{
  fflush(stdout);
  g_log = tmpfile();                                  /* can be NULL (e.g. no temp dir on windows) */
  if (!g_log) return;
  g_saved_stdout = dup(1); dup2(fileno(g_log), 1);
}
static int log_has(const char *s)
{
  char line[160]; int hit = 0;
  if (!g_log) return 0;
  fflush(stdout);
  rewind(g_log);
  while (fgets(line, sizeof line, g_log)) if (strstr(line, s)) hit = 1;
  return hit;
}
static void log_release(void)
{
  if (!g_log) return;
  fflush(stdout); dup2(g_saved_stdout, 1); close(g_saved_stdout); fclose(g_log); g_log = NULL;
}
static uint32_t word_bit(const char *w) { return 1u << voice_class_id(w); }

static int voice_hand_dry_run(void)
{
  motion_init(); pi_link_init(); lcd_init(); modes_init();
  run_ms(100);

  printf("== voice hand (dry run): live palm position from the pi\n");
  say("pick"); say("red box"); pi_link_on_tx_done();
  pi_rx("OBJ red_box 182 -45 30\n");
  say("yes");
  run_ms(8000);
  CHECK(strcmp(screen_line(0), "S4 HOLDING") == 0, "dry-run grasp -> S4 holding");

  say("hand");
  CHECK(strcmp(screen_line(0), "S7 CONFIRM PLACE") == 0 && strcmp(screen_line(1), "Place at HAND?") == 0, "hand -> S7 asks");
  g_tx[0] = 0;
  say("yes");
  CHECK(strcmp(g_tx, "FIND hand\n") == 0, "yes sends FIND hand");
  CHECK(strcmp(screen_line(0), "S7 CONFIRM PLACE") == 0 && strcmp(screen_line(1), "Finding hand") == 0, "waits in S7");
  CHECK(!(g_voice_mask & (word_bit("yes") | word_bit("no"))) && (g_voice_mask & word_bit("stop")), "while finding: only stop");
  g_tx[0] = 0;
  say("no");
  say("yes");                                         /* past the filter: on_word must ignore them too */
  CHECK(strcmp(screen_line(0), "S7 CONFIRM PLACE") == 0 && strcmp(screen_line(1), "Finding hand") == 0, "yes / no while finding: still waiting");
  CHECK(g_tx[0] == 0, "no second FIND while finding");
  pi_link_on_tx_done();
  pi_rx("NOTFOUND hand\n");
  CHECK(strcmp(screen_line(0), "S7 CONFIRM PLACE") == 0 && strcmp(screen_line(1), "Place at HAND?") == 0, "NOTFOUND -> asks again");
  CHECK(g_voice_mask & word_bit("yes"), "yes / no accepted again");

  say("yes"); pi_link_on_tx_done();
  g_tick += APP_PI_TIMEOUT_MS + 100u; loop_pass();
  CHECK(strcmp(screen_line(0), "S4 HOLDING") == 0, "pi timeout while finding -> S4, still holding");

  say("hand"); say("yes"); pi_link_on_tx_done();
  say("stop");
  CHECK(strcmp(screen_line(0), "S4 HOLDING") == 0, "stop while finding -> S4");
  pi_rx("OBJ hand 1 2 3\n");
  CHECK(strcmp(screen_line(0), "S4 HOLDING") == 0, "late hand reply after stop does nothing");

  say("hand"); say("yes"); pi_link_on_tx_done();
  pi_rx("OBJ red_box 1 2 3\n");
  CHECK(strcmp(screen_line(1), "Finding hand") == 0, "a box reply does not count as the hand");
  log_capture();
  CHECK(g_log != NULL, "log capture works (tmpfile)");
  pi_rx("OBJ hand 101 -4 279\n");
  int placing = strcmp(screen_line(0), "S8 PLACING") == 0;
  run_ms(8000);
  int above = log_has("move to 101 -4 329 mm"), release = log_has("move to 101 -4 309 mm");
  int open_ = log_has("gripper OPEN"), palm = log_has("move to 101 -4 279 mm");
  log_release();
  CHECK(placing, "OBJ hand -> S8 placing");
  CHECK(above, "first above the palm (+APPROACH_MM)");
  CHECK(release && open_, "opens at palm + HAND_RELEASE_MM");
  CHECK(!palm, "never goes down to the palm itself");
  CHECK(strcmp(screen_line(0), "S0 IDLE") == 0, "then home -> S0");

  printf("== voice station (dry run): drop spot from the pi\n");
  say("pick"); say("red box"); pi_link_on_tx_done();
  pi_rx("OBJ red_box 182 -45 30\n");
  say("yes");
  run_ms(8000);
  CHECK(strcmp(screen_line(0), "S4 HOLDING") == 0, "grasp again -> S4 holding");
  say("station");
  CHECK(strcmp(screen_line(0), "S7 CONFIRM PLACE") == 0 && strcmp(screen_line(1), "Place at STATION?") == 0,
        "station -> S7 asks");
  g_tx[0] = 0;
  say("yes");
  CHECK(strcmp(g_tx, "FIND station\n") == 0, "yes sends FIND station (not the fixed pose)");
  CHECK(strcmp(screen_line(1), "Finding station") == 0, "waits in S7: Finding station");
  CHECK(!(g_voice_mask & (word_bit("yes") | word_bit("no"))) && (g_voice_mask & word_bit("stop")), "while finding: only stop");
  pi_link_on_tx_done();
  pi_rx("NOTFOUND station\n");
  CHECK(strcmp(screen_line(1), "Place at STATION?") == 0, "NOTFOUND (no station / full) -> asks again");
  say("yes"); pi_link_on_tx_done();
  pi_rx("OBJ hand 1 2 3\n");
  CHECK(strcmp(screen_line(1), "Finding station") == 0, "a hand reply does not count as the station");
  log_capture();
  pi_rx("OBJ station 50 60 120\n");
  placing = strcmp(screen_line(0), "S8 PLACING") == 0;
  run_ms(8000);
  above = log_has("move to 50 60 170 mm"); release = log_has("move to 50 60 140 mm");
  open_ = log_has("gripper OPEN"); palm = log_has("move to 50 60 120 mm");
  log_release();
  CHECK(placing, "OBJ station -> S8 placing");
  CHECK(above, "first above the drop spot (+APPROACH_MM)");
  CHECK(release && open_, "opens at drop spot + STATION_RELEASE_MM");
  CHECK(!palm, "never pushes the block down to the station top itself");
  CHECK(strcmp(screen_line(0), "S0 IDLE") == 0, "then home -> S0");

  printf("\n%d passed, %d failed\n", passes, fails);
  return fails ? 1 : 0;
}
#endif

int main(void)
{
#if APP_VOICE_DRY_RUN
  return voice_hand_dry_run();
#endif
  printf("== event queue\n");
  app_events_init();
  for (int i = 0; i < 40; i++) app_event_post(EVT_ENC_STEP, 0, i, 0, 0);
  CHECK(app_events_dropped() == 8, "40 posts into 32 slots drop 8");
  app_event_t e; app_event_pop(&e);
  CHECK(e.v[0] == 0, "fifo order");
  app_events_drain(); app_events_init();

  printf("== servo calibration (measured unloaded endpoints; installed homes/limits)\n");
  check_servo_cal(J1_BASE,         500, 1460, 2430);
  check_servo_cal(J2_SHOULDER,     500, 1430, 2440);
  check_servo_cal(J3_ELBOW,        540, 1530, 2500);
  check_servo_cal(J4_WRIST_ROLL,   500, 1440, 2370);
  check_servo_cal(J5_WRIST_PITCH,  640, 1580, 2470);
  check_servo_cal(J6_GRIPPER,      500, 1380, 2300);
  CHECK(strcmp(servo_cal(J4_WRIST_ROLL)->name, "J4 WRIST ROLL") == 0, "J4 is wrist roll");
  CHECK(strcmp(servo_cal(J5_WRIST_PITCH)->name, "J5 WRIST PITCH") == 0, "J5 is wrist pitch");
  CHECK(servo_deg_to_us(0, 250.0f) == 2430, "out of range clamps");
  CHECK(servo_deg_to_us(J6_GRIPPER, 60.0f) == 1087, "J6 60 deg safe max pulse");
  CHECK(servo_deg_to_us(J6_GRIPPER, 250.0f) == 2300, "J6 clamps to its 180 deg anchor");
  CHECK(servo_deg_to_us(J6_GRIPPER, -10.0f) == 500, "J6 clamps to its 0 deg anchor");

  printf("== installed homes and conservative limits\n");
  check_cal_limits_home(J1_BASE,         0.0f, 180.0f,  85.0f, 1407);
  check_cal_limits_home(J2_SHOULDER,    50.0f, 180.0f, 125.0f, 1823);
  check_cal_limits_home(J3_ELBOW,       45.0f, 180.0f, 145.0f, 2123);
  check_cal_limits_home(J4_WRIST_ROLL,   0.0f, 180.0f,  90.0f, 1440);
  check_cal_limits_home(J5_WRIST_PITCH, 25.0f, 140.0f,  25.0f, 901);
  check_cal_limits_home(J6_GRIPPER,      0.0f,  60.0f,  35.0f, 842);

  printf("== motion clamps to the installed boundaries\n");
  motion_init();
  check_joint_clamp(J1_BASE,         0.0f, 180.0f);
  check_joint_clamp(J2_SHOULDER,    90.0f, 180.0f);
  check_joint_clamp(J3_ELBOW,       45.0f, 180.0f);
  check_joint_clamp(J4_WRIST_ROLL,   0.0f, 180.0f);
  check_joint_clamp(J5_WRIST_PITCH, 25.0f, 140.0f);
  check_joint_clamp(J6_GRIPPER,      0.0f,  60.0f);

  printf("== J2/J5 coupling uses the actual commanded pose, not the target\n");
  motion_init();
  motion_set_speed(SPEED_FAST);
  app_events_drain();
  run_motion_ms(100);
  CHECK(cur(J2_SHOULDER) == 125.0f && cur(J5_WRIST_PITCH) == 25.0f, "coupling starts at the home pose");

  /* J5 current below 65: J2 floor is 90, even when the J5 target is high. */
  CHECK(set_pair_target(50.0f, 25.0f) == MOTION_CLAMPED && tgt(J2_SHOULDER) == 90.0f,
        "J5 current 25: full J2 target 50 clamps to 90");
  {
    joint_vec_t t;
    motion_get_current(&t);
    t.deg[J2_SHOULDER] = 50.0f;
    t.deg[J5_WRIST_PITCH] = 100.0f;
    CHECK(motion_set_target(&t) == MOTION_CLAMPED && tgt(J2_SHOULDER) == 90.0f,
          "J5 target 100 does not unlock J2 while J5 current is 25");
  }
  motion_hold();
  CHECK(cur(J2_SHOULDER) == 125.0f && cur(J5_WRIST_PITCH) == 25.0f,
        "command clamping does not move the arm");

  /* J5 current exactly 65: J2 50 becomes legal. */
  CHECK(set_pair_target(125.0f, 65.0f) == MOTION_OK, "J5 target 65 accepted");
  run_motion_ms(3000);
  CHECK(cur(J5_WRIST_PITCH) == 65.0f, "J5 reaches 65");
  CHECK(set_pair_target(50.0f, 65.0f) == MOTION_OK && tgt(J2_SHOULDER) == 50.0f,
        "J5 current 65 allows J2 50");
  run_motion_ms(5000);
  CHECK(cur(J2_SHOULDER) == 50.0f && cur(J5_WRIST_PITCH) == 65.0f, "J2 reaches 50 with J5 at 65");
  CHECK(motion_set_joint_target(J2_SHOULDER, 40.0f) == MOTION_CLAMPED &&
        tgt(J2_SHOULDER) == 50.0f, "J2 50 is the absolute floor while J5 is high");

  /* J2 current below 90: J5 is blocked at 65; the shoulder is not lifted. */
  CHECK(motion_set_joint_target(J5_WRIST_PITCH, 25.0f) == MOTION_CLAMPED &&
        tgt(J5_WRIST_PITCH) == 65.0f, "J2 current 50: J5 target 25 clamps to 65");
  run_motion_ms(3000);
  CHECK(cur(J2_SHOULDER) == 50.0f && cur(J5_WRIST_PITCH) == 65.0f,
        "blocked J5 leaves J2 at 50 (no automatic shoulder lift)");
  CHECK(tgt(J2_SHOULDER) == 50.0f && tgt(J5_WRIST_PITCH) == 65.0f,
        "blocked target is clamped, not left latent");

  /* J2 current just below 90 still blocks J5 25. */
  CHECK(set_pair_target(89.9f, 65.0f) == MOTION_OK, "J2 target 89.9 accepted with J5 at 65");
  run_motion_ms(3000);
  CHECK(cur(J2_SHOULDER) == 89.9f, "J2 reaches 89.9");
  CHECK(motion_set_joint_target(J5_WRIST_PITCH, 25.0f) == MOTION_CLAMPED &&
        tgt(J5_WRIST_PITCH) == 65.0f, "J2 current 89.9: J5 target 25 clamps to 65");
  motion_hold();

  /* J5 current just below 65 still blocks J2 50. Raise J2 to 90 first, because
   * at J2 = 89.9 the J5 floor is already 65. */
  CHECK(set_pair_target(90.0f, 65.0f) == MOTION_OK, "raise J2 to 90 with J5 at 65");
  run_motion_ms(3000);
  CHECK(cur(J2_SHOULDER) == 90.0f, "J2 reaches 90");
  CHECK(set_pair_target(90.0f, 64.9f) == MOTION_OK, "J5 target 64.9 accepted with J2 at 90");
  run_motion_ms(3000);
  CHECK(cur(J2_SHOULDER) == 90.0f && cur(J5_WRIST_PITCH) == 64.9f, "pose is J2 90 / J5 64.9");
  CHECK(motion_set_joint_target(J2_SHOULDER, 50.0f) == MOTION_CLAMPED &&
        tgt(J2_SHOULDER) == 90.0f, "J5 current 64.9: J2 target 50 clamps to 90");
  motion_hold();

  CHECK(set_pair_target(125.0f, 65.0f) == MOTION_OK, "set concurrent start pose J2 125 / J5 65");
  run_motion_ms(3000);
  CHECK(cur(J2_SHOULDER) == 125.0f && cur(J5_WRIST_PITCH) == 65.0f, "concurrent start pose reached");

  /* Single-joint requests clamp only the requested joint and look at both the
   * current pose and the pending target of the other joint. Test both orders. */
  CHECK(motion_set_joint_target(J5_WRIST_PITCH, 25.0f) == MOTION_OK &&
        tgt(J5_WRIST_PITCH) == 25.0f, "J5 single 25 accepted while J2 pending is 125");
  CHECK(motion_set_joint_target(J2_SHOULDER, 50.0f) == MOTION_CLAMPED &&
        tgt(J2_SHOULDER) == 90.0f && tgt(J5_WRIST_PITCH) == 25.0f,
        "J5 pending 25 blocks J2 50; unrequested J5 target unchanged");
  motion_hold();
  CHECK(motion_set_joint_target(J2_SHOULDER, 50.0f) == MOTION_OK &&
        tgt(J2_SHOULDER) == 50.0f, "J2 single 50 accepted while J5 pending is 65");
  CHECK(motion_set_joint_target(J5_WRIST_PITCH, 25.0f) == MOTION_CLAMPED &&
        tgt(J5_WRIST_PITCH) == 65.0f && tgt(J2_SHOULDER) == 50.0f,
        "J2 pending 50 blocks J5 25; unrequested J2 target unchanged");
  motion_hold();

  /* Both requested values are legal at the current pose, but the endpoint is not.
   * The setter must clamp and report it; the tick must not change it later. */
  CHECK(set_pair_target(50.0f, 25.0f) == MOTION_CLAMPED &&
        tgt(J2_SHOULDER) == 90.0f && tgt(J5_WRIST_PITCH) == 25.0f,
        "full target 50/25 clamps shoulder to 90 and returns CLAMPED");
  int unsafe = 0;
  for (uint32_t t = 0; t < 6000; t += 20) {
    g_tick += 20; motion_tick();
    if (cur(J2_SHOULDER) < 90.0f - 0.01f && cur(J5_WRIST_PITCH) < 65.0f - 0.01f) unsafe = 1;
  }
  CHECK(!unsafe, "simultaneous path never has J2<90 with J5<65");
  CHECK(cur(J2_SHOULDER) == 90.0f && cur(J5_WRIST_PITCH) == 25.0f,
        "blocked shoulder stops at 90 while the wrist reaches 25");
  CHECK(tgt(J2_SHOULDER) == 90.0f && tgt(J5_WRIST_PITCH) == 25.0f,
        "no stale J2 50 target survives the clamp");

  /* Reverse transition: J5 25 is clamped now and must be reissued after J2 reaches 90. */
  CHECK(set_pair_target(90.0f, 140.0f) == MOTION_OK, "raise J5 to 140");
  run_motion_ms(4000);
  CHECK(cur(J5_WRIST_PITCH) == 140.0f, "J5 reaches 140");
  CHECK(set_pair_target(50.0f, 140.0f) == MOTION_OK, "lower J2 to 50 with J5 high");
  run_motion_ms(4000);
  CHECK(cur(J2_SHOULDER) == 50.0f && cur(J5_WRIST_PITCH) == 140.0f, "pose is J2 50 / J5 140");
  CHECK(set_pair_target(90.0f, 25.0f) == MOTION_CLAMPED &&
        tgt(J2_SHOULDER) == 90.0f && tgt(J5_WRIST_PITCH) == 65.0f,
        "while J2 current 50, J5 target 25 clamps to 65");
  run_motion_ms(5000);
  CHECK(cur(J2_SHOULDER) == 90.0f && cur(J5_WRIST_PITCH) == 65.0f,
        "J2 reaches 90; J5 stops at the clamped 65");
  CHECK(tgt(J5_WRIST_PITCH) == 65.0f, "J5 25 is not resumed automatically after J2 reaches 90");
  CHECK(motion_set_joint_target(J5_WRIST_PITCH, 25.0f) == MOTION_OK &&
        tgt(J5_WRIST_PITCH) == 25.0f, "reissue after the pose allows is accepted");
  run_motion_ms(3000);
  CHECK(cur(J5_WRIST_PITCH) == 25.0f, "reissued J5 reaches 25");

  /* Jog crossing: J2 down stops at 90 while J5 is low. */
  CHECK(set_pair_target(125.0f, 25.0f) == MOTION_OK, "raise J2 to 125 for the jog test");
  run_motion_ms(3000);
  CHECK(cur(J2_SHOULDER) == 125.0f && cur(J5_WRIST_PITCH) == 25.0f, "jog start pose reached");
  run_jog(J2_SHOULDER, -45.0f, 3000);
  CHECK(cur(J2_SHOULDER) == 90.0f, "J2 jog stops at 90 while J5 current is 25");
  CHECK(cur(J5_WRIST_PITCH) == 25.0f, "J5 stays at 25 during the shoulder jog");
  CHECK(tgt(J2_SHOULDER) == 90.0f, "jog target is clamped at 90, not left below it");

  /* Jog crossing: J5 down stops at 65 while J2 is low, without lifting J2. */
  CHECK(set_pair_target(90.0f, 140.0f) == MOTION_OK, "raise J5 to 140");
  run_motion_ms(4000);
  CHECK(set_pair_target(50.0f, 140.0f) == MOTION_OK, "lower J2 to 50");
  run_motion_ms(4000);
  CHECK(cur(J2_SHOULDER) == 50.0f && cur(J5_WRIST_PITCH) == 140.0f, "jog start pose reached");
  run_jog(J5_WRIST_PITCH, -45.0f, 3000);
  CHECK(cur(J5_WRIST_PITCH) == 65.0f, "J5 jog stops at 65 while J2 current is 50");
  CHECK(cur(J2_SHOULDER) == 50.0f && tgt(J2_SHOULDER) == 50.0f, "J5 jog does not lift J2");
  CHECK(tgt(J5_WRIST_PITCH) == 65.0f, "jog target is clamped at 65, not left below it");

  /* J2 jog may cross below 90 while J5 is high. */
  CHECK(set_pair_target(125.0f, 70.0f) == MOTION_OK, "raise J2 to 125 and J5 to 70");
  run_motion_ms(4000);
  CHECK(cur(J2_SHOULDER) == 125.0f && cur(J5_WRIST_PITCH) == 70.0f, "jog start pose reached");
  run_jog(J2_SHOULDER, -45.0f, 2500);
  CHECK(cur(J2_SHOULDER) == 50.0f, "J2 jog reaches its 50 floor while J5 is high");
  CHECK(cur(J5_WRIST_PITCH) == 70.0f, "J5 stays high during the shoulder jog");

  motion_set_speed(SPEED_FINE);
  app_events_drain();

  printf("== boot: no pulse until the first tick, then home\n");
  motion_init(); pi_link_init(); lcd_init(); modes_init();
  CHECK(htim3.ccr[0] == 0, "no pulse before the first control tick");
  CHECK(safety_state() == SAFETY_OK, "no e-stop: safety ok at boot");
  run_ms(100);
  CHECK(htim3.ccr[0] == 1407, "J1 first pulse = home 85 deg (1407 us)");
  CHECK(htim3.ccr[1] == 1823, "J2 first pulse = home 125 deg (1823 us)");
  CHECK(htim4.ccr[0] == 2123, "J3 first pulse = home 145 deg (2123 us)");
  CHECK(htim4.ccr[1] == 1440, "J4 first pulse = home 90 deg (1440 us)");
  CHECK(htim4.ccr[2] == 901,  "J5 first pulse = home 25 deg (901 us)");
  CHECK(htim4.ccr[3] == 842,  "J6 first pulse = home 35 deg (842 us)");

  printf("== voice mode: never moves on its own\n");
  CHECK(modes_current() == MODE_VOICE, "boots in voice mode");
  run_ms(500);
  CHECK(cur(0) == 85.0f, "idle voice mode does not move");

  printf("== mode switch from the lcd mode cards\n");
  app_event_post(EVT_TOUCH_MODE_TAB, MODE_CARTESIAN, 0, 0, 0); run_ms(60);
  CHECK(modes_current() == MODE_CARTESIAN, "touch card -> cartesian");
  app_event_post(EVT_TOUCH_MODE_TAB, MODE_JOINT_GROUP, 0, 0, 0); run_ms(60);
  CHECK(modes_current() == MODE_JOINT_GROUP, "touch card -> joint group");

  printf("== manual motion: no hold-to-enable any more\n");
  motion_set_joint_target(J1_BASE, 100.0f);
  run_ms(500);
  CHECK(cur(0) > 89.5f && cur(0) < 90.5f, "fine speed 10 deg/s: ~5 deg in 0.5 s");

  printf("== jog timeout: a stale jog stops by itself\n");
  motion_jog(J2_SHOULDER, 10.0f);                  /* sent once, never refreshed */
  float j2 = cur(1);
  run_ms(1000);
  CHECK(cur(1) - j2 < 2.5f, "jog not refreshed: stops after ~150 ms, not 10 deg");

  printf("== fast speed toggle + arrival event\n");
  app_events_drain();
  motion_toggle_speed();
  motion_set_joint_target(J3_ELBOW, 120.0f);
  for (int i = 0; i < 60; i++) { g_tick += 20; motion_tick(); }
  CHECK(cur(2) == 120.0f, "fast 45 deg/s reaches 120 within 1.2 s");
  CHECK(count_events_type(EVT_MOTION_DONE) == 1, "exactly one EVT_MOTION_DONE");
  motion_toggle_speed();

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
  motion_set_joint_target(J1_BASE, 60.0f); run_ms(20);   /* move away from home first */
  run_ms(4000);
  CHECK(cur(0) == 60.0f, "voice mode moves to a set target");
  say("home");
  CHECK(strcmp(screen_line(0), "S11 CONFIRM HOME") == 0, "home from S0 asks first");
  CHECK(cur(0) == 60.0f, "S11: nothing moves before yes");
  say("yes");
  CHECK(strcmp(screen_line(0), "S9 GOING HOME") == 0, "home + yes -> moving");
  run_ms(500);
  say("stop");
  CHECK(strcmp(screen_line(0), "S10 STOPPED") == 0, "stop while moving -> S10");
  float at = cur(0);
  run_ms(1000);
  CHECK(cur(0) == at, "S10: arm holds");
  say("pick");
  CHECK(strcmp(screen_line(0), "S10 STOPPED") == 0, "S10 ignores start words");

  say("home");
  CHECK(strcmp(screen_line(0), "S11 CONFIRM HOME") == 0, "home from S10 asks first");
  say("no");
  CHECK(strcmp(screen_line(0), "S10 STOPPED") == 0, "no -> back to S10, still holding");
  say("home"); say("yes");
  run_ms(15000);   /* Allow the configured gripper step and arm-home move to finish.
                   * This checks completion, not physical open/close direction. */
  CHECK(cur(0) == 85.0f, "home from S10 reaches home");
  CHECK(strcmp(screen_line(0), "S0 IDLE") == 0, "then back to S0");
  CHECK(cur(5) == APP_GRIPPER_OPEN_DEG, "home from S10 opened the gripper");

  printf("== fatal fault during motion: freezes (last test, latches until reset)\n");
  motion_set_joint_target(J1_BASE, 120.0f);
  run_ms(500);
  app_fault_raise(FAULT_INIT, true);
  at = cur(0);
  run_ms(40);
  CHECK(safety_state() == SAFETY_FATAL_FAULT, "fatal fault -> safety FAULT");
  CHECK(cur(0) == at && at > 85.0f && at < 120.0f, "fault freezes mid-move");
  CHECK(tgt(0) == at, "target reset to the current angle (nothing jumps later)");
  run_ms(1000);
  CHECK(cur(0) == at, "stays frozen");

  printf("\n%d passed, %d failed\n", passes, fails);
  return fails ? 1 : 0;
}
