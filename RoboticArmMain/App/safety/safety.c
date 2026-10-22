/* safety.c
 *
 * pins (labels from the .ioc, all pull-up):
 *   ESTOP_NO   PE7   e-stop NO contact: closed (pressed) = 0
 *   BTN_ENABLE PE12  hold-to-enable:   pressed = 0
 *   LIM1..LIM4       NC to GND:        normal = 0, triggered or broken wire = 1
 *
 * policy:
 *   - enable release and e-stop act at once (exti + every tick), enable press needs
 *     2 stable ticks (40 ms) so a bounce cannot start motion
 *   - after the e-stop or a rail loss, motion stays blocked until the rail is back
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
static volatile bool     s_estop;
static volatile bool     s_enable_held;
static volatile uint8_t  s_enable_cnt;        /* consecutive ticks the button read pressed */
static volatile bool     s_rail_ok = true;    /* true when rail sense is not used */
static volatile uint32_t s_rail_mv;
static volatile bool     s_restart_required;
static volatile uint8_t  s_limit_mask;

/* last values seen by safety_step, to post events on change */
static bool    s_prev_estop, s_prev_enable, s_prev_rail_ok = true;
static uint8_t s_prev_limit;

static inline bool pin_low(GPIO_TypeDef *port, uint16_t pin)
{
  return HAL_GPIO_ReadPin(port, pin) == GPIO_PIN_RESET;
}

static void read_fast_inputs(void)
{
  bool estop = pin_low(ESTOP_NO_GPIO_Port, ESTOP_NO_Pin);
  if (estop && !s_estop) s_restart_required = true;
  s_estop = estop;

  if (!pin_low(BTN_ENABLE_GPIO_Port, BTN_ENABLE_Pin)) {
    s_enable_cnt = 0;
    s_enable_held = false;                       /* release: immediate */
  }
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
  s_estop = pin_low(ESTOP_NO_GPIO_Port, ESTOP_NO_Pin);
  s_enable_held = false;
  s_enable_cnt = 0;
  s_restart_required = s_estop;  /* booting with the e-stop latched also needs a confirm after release */
  s_limit_mask = read_limits();
  s_prev_estop = s_estop;
  s_prev_limit = s_limit_mask;
  LOG("safety: init, estop %s, rail sense %s, limit switches %s\r\n",
      s_estop ? "PRESSED" : "released",
      APP_USE_RAIL_SENSE ? "on" : "off (APP_USE_RAIL_SENSE 0)",
      APP_USE_LIMIT_SWITCHES ? "on" : "off (APP_USE_LIMIT_SWITCHES 0)");
}

void safety_tick(void)
{
  read_fast_inputs();

  /* enable press: 2 stable ticks */
  if (pin_low(BTN_ENABLE_GPIO_Port, BTN_ENABLE_Pin)) {
    if (s_enable_cnt < 255u) s_enable_cnt++;
    if (s_enable_cnt >= 2u) s_enable_held = true;
  }

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
  if (gpio_pin == BTN_ENABLE_Pin || gpio_pin == ESTOP_NO_Pin) {
    read_fast_inputs();                          /* motion sees it at its next tick at the latest */
  }
#if APP_USE_LIMIT_SWITCHES
  else if (gpio_pin == LIM1_Pin || gpio_pin == LIM2_Pin || gpio_pin == LIM3_Pin || gpio_pin == LIM4_Pin) {
    s_limit_mask = read_limits();
  }
#endif
}

safety_state_t safety_state(void)
{
  if (app_fault_is_fatal()) return SAFETY_FATAL_FAULT;
  if (s_estop)              return SAFETY_ESTOP;
  if (!s_rail_ok)           return SAFETY_RAIL_LOST;
  if (s_restart_required)   return SAFETY_RESTART_REQUIRED;
  return SAFETY_OK;
}

bool safety_motion_allowed(void) { return safety_state() == SAFETY_OK; }
bool safety_enable_held(void)    { return s_enable_held; }

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
  if (!s_estop && s_rail_ok) {
    s_restart_required = false;
    LOG("safety: restart confirmed\r\n");
  } else {
    LOG("safety: restart refused (estop or rail still not ok)\r\n");
  }
}

void safety_get_status(safety_status_t *out)
{
  out->state = safety_state();
  out->estop_pressed = s_estop;
  out->enable_held = s_enable_held;
  out->rail_sense_used = (APP_USE_RAIL_SENSE != 0);
  out->rail_ok = s_rail_ok;
  out->rail_mv = s_rail_mv;
  out->limit_mask = s_limit_mask;
}

const char *safety_state_name(safety_state_t s)
{
  switch (s) {
    case SAFETY_OK:               return "OK";
    case SAFETY_ESTOP:            return "E-STOP";
    case SAFETY_RAIL_LOST:        return "SERVO POWER OFF";
    case SAFETY_RESTART_REQUIRED: return "CONFIRM RESTART";
    case SAFETY_FATAL_FAULT:      return "FAULT";
    default:                      return "?";
  }
}

void safety_step(void)
{
  bool estop = s_estop, enable = s_enable_held, rail = s_rail_ok;
  uint8_t lim = s_limit_mask;

  if (estop != s_prev_estop) {
    (void)app_event_post0(estop ? EVT_ESTOP_PRESSED : EVT_ESTOP_RELEASED);
    LOG("safety: e-stop %s\r\n", estop ? "PRESSED" : "released");
  }
  if (enable != s_prev_enable) {
    (void)app_event_post0(enable ? EVT_ENABLE_PRESSED : EVT_ENABLE_RELEASED);
  }
  if (rail != s_prev_rail_ok) {
    (void)app_event_post0(rail ? EVT_RAIL_BACK : EVT_RAIL_LOST);
    LOG("safety: servo rail %s (%lu mV)\r\n", rail ? "back" : "LOST", (unsigned long)s_rail_mv);
  }
  uint8_t newly = (uint8_t)(lim & (uint8_t)~s_prev_limit);
  for (int i = 0; i < 4; i++) {
    if (newly & (1u << i)) (void)app_event_post(EVT_LIMIT_HIT, (uint16_t)i, 0, 0, 0);
  }

  s_prev_estop = estop;
  s_prev_enable = enable;
  s_prev_rail_ok = rail;
  s_prev_limit = lim;
}
