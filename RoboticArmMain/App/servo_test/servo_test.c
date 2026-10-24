/* servo_test.c - see servo_test.h */
#include "servo_test/servo_test.h"
#include "motion/servo.h"
#include "safety/safety.h"
#include "voice/voice.h"
#include "common/app_config.h"
#include "common/app_events.h"
#include "common/app_log.h"
#include "common/app_util.h"

#define TEST_CENTER_DEG   90.0f       /* min/max in servo_cal.c are unmeasured, so stay near 90 */
#define TEST_SPEED_DEG_S  60.0f       /* 1.2 deg per 20 ms frame = about 13 us, above the 10 us sg90 deadband */
#define TEST_DWELL_MS     500u
#define TEST_STAGGER_MS   300u        /* switch-on gap: six servos never jump to 90 at the same moment */
#define NEXT_WORD         "pick"

#define TICK_MS           (1000u / APP_CONTROL_HZ)
#define STEP_DEG          (TEST_SPEED_DEG_S / (float)APP_CONTROL_HZ)
#define DWELL_TICKS       (TEST_DWELL_MS / TICK_MS)
#define STAGGER_TICKS     (TEST_STAGGER_MS / TICK_MS)

static const float SWING_DEG[] = { 60.0f, 120.0f, 90.0f };
#define SWING_LEN  ((int)(sizeof SWING_DEG / sizeof SWING_DEG[0]))

/* isr -> main loop log messages: arg = joint, v[0] = deg, v[1] = us or ms */
#define EVT_TEST_JOINT_ON   ((evt_type_t)(EVT_USER_BASE + 0))
#define EVT_TEST_MOVE       ((evt_type_t)(EVT_USER_BASE + 1))
#define EVT_TEST_REACHED    ((evt_type_t)(EVT_USER_BASE + 2))
#define EVT_TEST_SAFETY_OFF ((evt_type_t)(EVT_USER_BASE + 3))
#define EVT_TEST_ALL_HOLD   ((evt_type_t)(EVT_USER_BASE + 4))
#define EVT_TEST_SWING      ((evt_type_t)(EVT_USER_BASE + 5))

typedef enum {
  PHASE_OFF = 0,     /* no pulses */
  PHASE_POWER_ON,    /* switching J1..J6 on at 90, one per TEST_STAGGER_MS */
  PHASE_HOLD_ALL,    /* all six hold 90, waiting for "pick" */
  PHASE_DWELL,       /* s_joint swings: pause between moves */
  PHASE_MOVE,        /* s_joint swings: moving */
  PHASE_RETURN       /* "pick" heard: s_joint back to 90 */
} phase_t;

/* written in the isr; s_next_requested also by the main loop */
static volatile bool s_next_requested;
static phase_t       s_phase;
static int           s_joint;         /* swinging joint, or next joint to switch on in PHASE_POWER_ON */
static int           s_swing;         /* next index into SWING_DEG */
static float         s_cur_deg;
static float         s_to_deg;
static uint32_t      s_ticks;         /* countdown, or ticks spent on the move */

static int           s_next_word_id;

static void start_swing(int joint)
{
  s_joint = joint;
  s_cur_deg = TEST_CENTER_DEG;        /* every joint is holding 90 when its swing starts */
  s_swing = 0;
  s_phase = PHASE_DWELL;
  s_ticks = DWELL_TICKS;
  (void)app_event_post(EVT_TEST_SWING, (uint16_t)joint, 0, 0, 0);
}

static void start_move(phase_t phase, float to_deg)
{
  s_to_deg = to_deg;
  s_phase = phase;
  s_ticks = 0;
  (void)app_event_post(EVT_TEST_MOVE, (uint16_t)s_joint, (int32_t)to_deg,
                       servo_deg_to_us(s_joint, to_deg), 0);
}

/* one 20 ms step toward s_to_deg; true when it got there */
static bool move_step(void)
{
  float err = s_to_deg - s_cur_deg;
  bool done = absf(err) <= STEP_DEG;
  s_cur_deg = done ? s_to_deg : s_cur_deg + ((err > 0.0f) ? STEP_DEG : -STEP_DEG);
  servo_write_deg(s_joint, s_cur_deg);
  s_ticks++;
  if (done) {
    (void)app_event_post(EVT_TEST_REACHED, (uint16_t)s_joint, (int32_t)s_cur_deg,
                         (int32_t)(s_ticks * TICK_MS), 0);
  }
  return done;
}

void servo_test_tick(void)
{
  if (!safety_motion_allowed()) {
    if (s_phase != PHASE_OFF) {
      servo_all_off();
      s_phase = PHASE_OFF;
      (void)app_event_post0(EVT_TEST_SAFETY_OFF);
    }
    return;
  }

  if (s_next_requested) {
    s_next_requested = false;
    if (s_phase == PHASE_HOLD_ALL) {
      start_swing(0);
    } else if (s_phase == PHASE_DWELL || s_phase == PHASE_MOVE) {
      start_move(PHASE_RETURN, TEST_CENTER_DEG);
    }                                               /* ignored while switching on or returning */
  }

  switch (s_phase) {
    case PHASE_OFF:
      s_joint = 0;
      s_ticks = 0;
      s_phase = PHASE_POWER_ON;
      break;
    case PHASE_POWER_ON:
      if (s_ticks > 0u) {
        s_ticks--;
        break;
      }
      servo_write_deg(s_joint, TEST_CENTER_DEG);
      (void)app_event_post(EVT_TEST_JOINT_ON, (uint16_t)s_joint, (int32_t)TEST_CENTER_DEG,
                           servo_deg_to_us(s_joint, TEST_CENTER_DEG), 0);
      s_joint++;
      s_ticks = STAGGER_TICKS;
      if (s_joint == ARM_N_JOINTS) {
        s_phase = PHASE_HOLD_ALL;
        (void)app_event_post0(EVT_TEST_ALL_HOLD);
      }
      break;
    case PHASE_HOLD_ALL:
      break;
    case PHASE_DWELL:
      if (s_ticks > 0u) s_ticks--;
      if (s_ticks == 0u) {
        start_move(PHASE_MOVE, SWING_DEG[s_swing]);
        s_swing = (s_swing + 1) % SWING_LEN;
      }
      break;
    case PHASE_MOVE:
      if (move_step()) {
        s_phase = PHASE_DWELL;
        s_ticks = DWELL_TICKS;
      }
      break;
    case PHASE_RETURN:
      if (move_step()) {
        if (s_joint == ARM_N_JOINTS - 1) {
          s_phase = PHASE_HOLD_ALL;
          (void)app_event_post0(EVT_TEST_ALL_HOLD);
        } else {
          start_swing(s_joint + 1);
        }
      }
      break;
  }
}

void servo_test_init(void)
{
  servo_init();                       /* CCR = 0 on all six: no pulse until the first tick */
  s_phase = PHASE_OFF;
  s_next_requested = false;
  s_next_word_id = voice_class_id(NEXT_WORD);
  if (s_next_word_id < 0) {
    LOG_ERR("servo_test: the model has no word \"%s\", staying at ALL 90\r\n", NEXT_WORD);
  }
  LOG("servo_test: init, all six to 90 one by one, then say \"%s\" for J1, J2 ...\r\n", NEXT_WORD);
}

void servo_test_step(void)
{
  app_event_t e;
  while (app_event_pop(&e)) {
    int j = (int)e.arg + 1;
    switch (e.type) {
      case EVT_VOICE_WORD:
        if ((int)e.arg == s_next_word_id) {
          s_next_requested = true;
          LOG("servo_test: \"%s\" heard, next stage\r\n", NEXT_WORD);
        }
        break;
      case EVT_TEST_JOINT_ON:
        LOG("servo_test: J%d on at %ld deg (%ld us)\r\n", j, (long)e.v[0], (long)e.v[1]);
        break;
      case EVT_TEST_ALL_HOLD:
        LOG("servo_test: ALL 90 - all six hold 90 deg, say \"%s\" to swing J1\r\n", NEXT_WORD);
        break;
      case EVT_TEST_SWING:
        LOG("servo_test: J%d swings 90/60/120/90, others hold 90\r\n", j);
        break;
      case EVT_TEST_MOVE:
        LOG("servo_test: J%d -> %ld deg (%ld us)\r\n", j, (long)e.v[0], (long)e.v[1]);
        break;
      case EVT_TEST_REACHED:
        LOG("servo_test: J%d at %ld deg (%ld ms)\r\n", j, (long)e.v[0], (long)e.v[1]);
        break;
      case EVT_TEST_SAFETY_OFF:
        LOG("servo_test: safety blocks motion (%s), all pulses off\r\n", safety_state_name(safety_state()));
        break;
      default:
        break;
    }
  }
}
