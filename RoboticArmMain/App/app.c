/* app.c - start-up order, main-loop order, control-tick order
 *
 * owner: core. a feature never edits this file except at the three EXTENSION POINT
 * lines, and only for a NEW module (see ProjectDevGuide/docs/TEAM_GUIDE.md "adding a module").
 */
#include "app.h"
#include "common/app_config.h"
#include "common/app_events.h"
#include "common/app_fault.h"
#include "common/app_log.h"
#include "common/app_timing.h"
#include "common/dma_buf.h"
#include "safety/safety.h"
#include "main.h"
#include "tim.h"

#if APP_USE_MOTION
#include "motion/motion.h"
#endif
#if APP_USE_INPUT
#include "input/input.h"
#endif
#if APP_USE_KINEMATICS
#include "kinematics/kinematics.h"
#endif
#if APP_USE_VOICE
#include "voice/voice.h"
#endif
#if APP_USE_PI
#include "pi_link/pi_link.h"
#endif
#if APP_USE_LCD
#include "lcd/lcd.h"
#endif
#if APP_USE_MODES
#include "modes/modes.h"
#endif
#if APP_USE_SERVO_TEST
#include "servo_test/servo_test.h"
#endif

#if APP_CONFIG_LOCAL_ACTIVE
#warning "app_config_local.h is active - feature flags differ from master"
#endif

static uint32_t s_loop_max_us;
static uint32_t s_last_overrun_log_ms;
static uint32_t s_last_beat_ms;

void app_init(void)
{
  app_log_init();
  timing_init();
  LOG("\r\n=== RoboticArmMain %s ===\r\n", APP_FW_VERSION);
  LOG("flags: motion %d input %d ik %d voice %d pi %d lcd %d modes %d | limits %d rail %d%s\r\n",
      APP_USE_MOTION, APP_USE_INPUT, APP_USE_KINEMATICS, APP_USE_VOICE, APP_USE_PI, APP_USE_LCD,
      APP_USE_MODES, APP_USE_LIMIT_SWITCHES, APP_USE_RAIL_SENSE,
      APP_CONFIG_LOCAL_ACTIVE ? "  (LOCAL OVERRIDES)" : "");

  app_events_init();
  if (!dma_buf_check()) app_fault_raise(FAULT_DMA_REGION, true);

  safety_init();                                  /* always first: everything below may move */
#if APP_USE_MOTION
  motion_init();
#endif
#if APP_USE_INPUT
  input_init();
#endif
#if APP_USE_KINEMATICS
  kin_init();
#endif
#if APP_USE_VOICE
  voice_init();
#endif
#if APP_USE_PI
  pi_link_init();
#endif
#if APP_USE_LCD
  lcd_init();
#endif
  /* EXTENSION POINT 1: init of a new module goes here, inside its own #if APP_USE_x */
#if APP_USE_SERVO_TEST
  servo_test_init();
#endif
#if APP_USE_MODES
  modes_init();                                   /* last: it starts using every module above */
#endif

  if (HAL_TIM_Base_Start_IT(&htim6) != HAL_OK) {  /* control tick on: safety + motion run from here */
    LOG_ERR("app: TIM6 start failed\r\n");
    app_fault_raise(FAULT_INIT, true);
  }
  LOG("app: running\r\n");
}

void app_control_tick(void)
{
#if APP_USE_INPUT
  input_tick();
#endif
  safety_tick();
#if APP_USE_MOTION
  motion_tick();                                  /* after safety: sees this tick's state */
#endif
  /* EXTENSION POINT 2: a new 50 Hz job goes here only if it MUST be in an isr (< 50 us) */
#if APP_USE_SERVO_TEST
  servo_test_tick();                              /* after safety, like motion_tick */
#endif
}

void app_loop(void)
{
  uint32_t t0 = timing_cycles();

#if APP_USE_INPUT
  input_step();
#endif
  safety_step();
#if APP_USE_VOICE
  voice_step();
#endif
#if APP_USE_PI
  pi_link_step();
#endif
  /* EXTENSION POINT 3: main-loop step of a new module goes here (producers before modes) */
#if APP_USE_SERVO_TEST
  servo_test_step();
#endif
#if APP_USE_MODES
  modes_step();                                   /* the only event consumer */
#else
  app_events_drain();
#endif
#if APP_USE_LCD
  lcd_step();                                     /* last: draws the state everyone just updated */
#endif

  app_fault_report();

  uint32_t now = HAL_GetTick();
  if (now - s_last_beat_ms >= 500u) {             /* LD1 blinking = main loop alive */
    s_last_beat_ms = now;
    HAL_GPIO_TogglePin(LD1_GPIO_Port, LD1_Pin);
  }

  uint32_t us = timing_us_since(t0);
  if (us > s_loop_max_us) s_loop_max_us = us;
  if (us > APP_LOOP_WARN_US && now - s_last_overrun_log_ms > 1000u) {
    s_last_overrun_log_ms = now;
    app_fault_raise(FAULT_LOOP_OVERRUN, false);
    LOG("app: loop pass took %lu us (limit %lu)\r\n", (unsigned long)us, (unsigned long)APP_LOOP_WARN_US);
  }
}

uint32_t app_loop_max_us(void) { return s_loop_max_us; }
