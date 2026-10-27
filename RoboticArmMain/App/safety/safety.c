/* safety.c
 *
 * pins (labels from the .ioc, all pull-up):
 *   LIM1..LIM4       NC to GND:        normal = 0, triggered or broken wire = 1
 * no e-stop and no hold-to-enable since 27 Oct 2026 (PE7 / PE12 not wired, not read).
 *
 * policy:
 *   - a limit switch acts at once (exti + every tick)
 *   - after a rail loss, motion stays blocked until the rail is back
 *     AND safety_confirm_restart() is called (guide section 15.2)
 */
#include "safety/safety.h"
#include "common/app_config.h"
#include "common/app_events.h"
#include "common/app_fault.h"
#include "common/app_log.h"
#include "main.h"
#if APP_USE_RAIL_SENSE
#include "input/input.h"
#endif

/* ---------------------------------------------------------------- limit switch map
 * TODO(mechanical): fill in which joint and direction each switch protects once fitted.
 * joint = -1 means "not assigned": a triggered unassigned switch is reported but blocks nothing */
typedef struct { int8_t joint; int8_t dir; } limit_map_t;
static const limit_map_t LIMIT_MAP[4] = {
  { -1, 0 },   /* LIM1 PE8  */
  { -1, 0 },   /* LIM2 PB11 */
  { -1, 0 },   /* LIM3 PE0  */
  { -1, 0 },   /* LIM4 PE2  */
};

/* ---------------------------------------------------------------- state (written in isr) */
static volatile bool     s_rail_ok = true;    /* true when rail sense is not used */
static volatile uint32_t s_rail_mv;
static volatile bool     s_restart_required;
static volatile uint8_t  s_limit_mask;

/* last values seen by safety_step, to post events on change */
static bool    s_prev_rail_ok = true;
static uint8_t s_prev_limit;

static inline bool pin_low(GPIO_TypeDef *port, uint16_t pin)
{
  return HAL_GPIO_ReadPin(port, pin) == GPIO_PIN_RESET;
}

static uint8_t read_limits(void)
{
#if APP_USE_LIMIT_SWITCHES
  uint8_t m = 0;
  if (!pin_low(LIM1_GPIO_Port, LIM1_Pin)) m |= 1u << 0;
  if (!pin_low(LIM2_GPIO_Port, LIM2_Pin)) m |= 1u << 1;
  if (!pin_low(LIM3_GPIO_Port, LIM3_Pin)) m |= 1u << 2;
  if (!pin_low(LIM4_GPIO_Port, LIM4_Pin)) m |= 1u << 3;
  return m;
#else
  return 0;
#endif
}

void safety_init(void)
{
  s_restart_required = false;
  s_limit_mask = read_limits();
  s_prev_limit = s_limit_mask;
  LOG("safety: init, rail sense %s, limit switches %s\r\n",
      APP_USE_RAIL_SENSE ? "on" : "off (APP_USE_RAIL_SENSE 0)",
      APP_USE_LIMIT_SWITCHES ? "on" : "off (APP_USE_LIMIT_SWITCHES 0)");
}

void safety_tick(void)
{
#if APP_USE_RAIL_SENSE
  uint32_t mv = input_rail_mv();
  s_rail_mv = mv;
  if (s_rail_ok && mv < APP_RAIL_LOST_MV) {
    s_rail_ok = false;
    s_restart_required = true;
  } else if (!s_rail_ok && mv > APP_RAIL_OK_MV) {
    s_rail_ok = true;                            /* restart latch stays set */
  }
#endif

  s_limit_mask = read_limits();
}

void safety_on_exti(uint16_t gpio_pin)
{
#if APP_USE_LIMIT_SWITCHES
  if (gpio_pin == LIM1_Pin || gpio_pin == LIM2_Pin || gpio_pin == LIM3_Pin || gpio_pin == LIM4_Pin) {
    s_limit_mask = read_limits();
  }
#else
  (void)gpio_pin;
#endif
}

safety_state_t safety_state(void)
{
  if (app_fault_is_fatal()) return SAFETY_FATAL_FAULT;
  if (!s_rail_ok)           return SAFETY_RAIL_LOST;
  if (s_restart_required)   return SAFETY_RESTART_REQUIRED;
  return SAFETY_OK;
}

bool safety_motion_allowed(void) { return safety_state() == SAFETY_OK; }

bool safety_limit_blocks(int joint, int dir)
{
  uint8_t m = s_limit_mask;
  for (int i = 0; i < 4; i++) {
    if ((m & (1u << i)) == 0u) continue;
    if (LIMIT_MAP[i].joint == joint && LIMIT_MAP[i].dir == dir) return true;
  }
  return false;
}

void safety_confirm_restart(void)
{
  if (s_rail_ok) {
    s_restart_required = false;
    LOG("safety: restart confirmed\r\n");
  } else {
    LOG("safety: restart refused (servo rail still not ok)\r\n");
  }
}

void safety_get_status(safety_status_t *out)
{
  out->state = safety_state();
  out->rail_sense_used = (APP_USE_RAIL_SENSE != 0);
  out->rail_ok = s_rail_ok;
  out->rail_mv = s_rail_mv;
  out->limit_mask = s_limit_mask;
}

const char *safety_state_name(safety_state_t s)
{
  switch (s) {
    case SAFETY_OK:               return "OK";
    case SAFETY_RAIL_LOST:        return "SERVO POWER OFF";
    case SAFETY_RESTART_REQUIRED: return "CONFIRM RESTART";
    case SAFETY_FATAL_FAULT:      return "FAULT";
    default:                      return "?";
  }
}

void safety_step(void)
{
  bool rail = s_rail_ok;
  uint8_t lim = s_limit_mask;

  if (rail != s_prev_rail_ok) {
    (void)app_event_post0(rail ? EVT_RAIL_BACK : EVT_RAIL_LOST);
    LOG("safety: servo rail %s (%lu mV)\r\n", rail ? "back" : "LOST", (unsigned long)s_rail_mv);
  }
  uint8_t newly = (uint8_t)(lim & (uint8_t)~s_prev_limit);
  for (int i = 0; i < 4; i++) {
    if (newly & (1u << i)) (void)app_event_post(EVT_LIMIT_HIT, (uint16_t)i, 0, 0, 0);
  }

  s_prev_rail_ok = rail;
  s_prev_limit = lim;
}
