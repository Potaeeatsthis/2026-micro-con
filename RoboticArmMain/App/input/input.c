/* input.c
 *
 * button debounce: sampled in the 50 Hz control tick; a press counts after 2 equal
 * samples (20-40 ms). sampling in a timer isr means a press is never missed while the
 * main loop is busy with the CNN or the LCD. the EXTI lines on these pins are kept
 * (course requirement + quick edge stats) but the decision is made here.
 *
 * directions measured on the bench 2026-10-24, KY-023 mounted with its pins pointing left:
 * right = PA3 high, up (away from the user) = PC0 low, so JOY_Y_SIGN = -1 makes right and up
 * both positive. KY-040 clockwise counts up, ENC_SIGN = +1. if the module is mounted
 * differently on the final panel, re-measure and change only these signs.
 *
 * APP_INPUT_TEST (app_config_local.h only, bench measurement, needs APP_USE_MODES 0 and
 * APP_USE_MOTION 0): logs at 10 Hz what the stm32 sees, from input_step():
 *   joy: raw = last single conversion, avg = 10-sample mean (what the getters use),
 *        min-max = single samples in the last 100 ms, out = dead-zone output x1000
 *   joy: 2s window min / mean / max / peak-to-peak of single samples, every 2 s
 *   enc: TIM1 counter, raw delta since the last line, direction, total (only when it moved)
 *   btn: debounced press / release of MODE, ENC, JOY
 *   evt: every event input posts (what the modes receive), ENC_STEP with a running sum
 * APP_INPUT_LOG_EVENTS (app_config_local.h only): just the evt: lines, allowed with modes and
 * motion on (bench test of the modes, see ProjectDevGuide/docs/controls-bench-test.md).
 * the isr part is a min/max/sum next to the existing averaging loop (no extra loop).
 */
#include "input/input.h"
#include "common/app_config.h"
#include "common/app_events.h"
#include "common/app_fault.h"
#include "common/app_log.h"
#include "common/app_util.h"
#include "common/dma_buf.h"
#include "main.h"
#include "adc.h"
#include "tim.h"

#define ADC_CH            3u
#define ADC_SAMPLES       20u                    /* per channel in the whole buffer */
#define ADC_HALF_SAMPLES  (ADC_SAMPLES / 2u)     /* 10 per channel per half = 10 ms */
#define ADC_FULL_SCALE    4095u
#define ADC_VREF_MV       3300u

#define JOY_DEADZONE      0.08f                  /* fraction of full deflection */
#define JOY_CENTER_TOL    0.15f                  /* "near centre" for the fine/fast press */
#define JOY_X_SIGN        1.0f
#define JOY_Y_SIGN        -1.0f                 /* up reads low on PC0, see header */
#define ENC_SIGN          1
#define ENC_COUNTS_PER_DETENT 4

#ifndef APP_INPUT_TEST
#define APP_INPUT_TEST 0
#endif
#if APP_INPUT_TEST && (APP_USE_MODES || APP_USE_MOTION)
#error "APP_INPUT_TEST needs APP_USE_MODES 0 and APP_USE_MOTION 0 (no motion during the bench test)"
#endif
#ifndef APP_INPUT_LOG_EVENTS
#define APP_INPUT_LOG_EVENTS 0
#endif
#define INPUT_EVT_LOG (APP_INPUT_TEST || APP_INPUT_LOG_EVENTS)

static uint16_t s_adc_buf[ADC_CH * ADC_SAMPLES] DMA_BUF;  /* [x, y, rail, x, y, rail, ...] */
static volatile uint16_t s_raw[ADC_CH];
static uint16_t s_center_x = 2048u, s_center_y = 2048u;
static bool     s_center_done;
static uint32_t s_init_ms;

/* buttons */
typedef enum { BTN_MODE_I = 0, BTN_ENC_I, BTN_JOY_I, BTN_COUNT } btn_index_t;
typedef struct { GPIO_TypeDef *port; uint16_t pin; } btn_hw_t;
static const btn_hw_t BTN_HW[BTN_COUNT] = {
  { BTN_MODE_GPIO_Port, BTN_MODE_Pin },
  { ENC_SW_GPIO_Port,   ENC_SW_Pin },
  { JOY_SW_GPIO_Port,   JOY_SW_Pin },
};
static volatile uint8_t  s_btn_hist[BTN_COUNT];   /* last samples, bit0 = newest, 1 = pressed */
static volatile bool     s_btn_state[BTN_COUNT];
static volatile uint8_t  s_btn_press_flags;       /* set in isr, cleared in input_step */
static volatile uint8_t  s_btn_release_flags;     /* same, for debounced releases */
static volatile uint32_t s_exti_edges;

/* encoder */
static int16_t s_enc_last;
static int32_t s_enc_accum;

#if APP_INPUT_TEST
/* isr -> main loop, joystick X/Y only. read and reset with interrupts off */
static volatile uint16_t s_t_last[2];
static volatile uint16_t s_t_min[2] = { 0xFFFFu, 0xFFFFu };
static volatile uint16_t s_t_max[2];
static volatile uint32_t s_t_sum[2];
static volatile uint32_t s_t_n;
static void test_step(uint8_t press, uint8_t release, int32_t enc_delta);
#endif

void input_init(void)
{
  s_init_ms = HAL_GetTick();
  if (HAL_TIM_Encoder_Start(&htim1, TIM_CHANNEL_ALL) != HAL_OK) app_fault_raise(FAULT_INIT, false);
  s_enc_last = (int16_t)__HAL_TIM_GET_COUNTER(&htim1);

  if (HAL_ADC_Start_DMA(&hadc1, (uint32_t *)s_adc_buf, ADC_CH * ADC_SAMPLES) != HAL_OK) {
    LOG_ERR("input: adc dma start failed\r\n");
    app_fault_raise(FAULT_INIT, false);
  }
  if (HAL_TIM_Base_Start(&htim2) != HAL_OK) app_fault_raise(FAULT_INIT, false);  /* 1 kHz trigger */
  LOG("input: init\r\n");
}

/* ---------------------------------------------------------------- isr side */
void input_on_adc(int half)
{
  const uint16_t *p = &s_adc_buf[(half ? ADC_HALF_SAMPLES : 0u) * ADC_CH];
  uint32_t sum[ADC_CH] = { 0u, 0u, 0u };
#if APP_INPUT_TEST
  uint16_t mn[2] = { s_t_min[0], s_t_min[1] };
  uint16_t mx[2] = { s_t_max[0], s_t_max[1] };
#endif
  for (uint32_t i = 0; i < ADC_HALF_SAMPLES; i++) {
    for (uint32_t c = 0; c < ADC_CH; c++) {
      uint16_t v = p[i * ADC_CH + c];
      sum[c] += v;
#if APP_INPUT_TEST
      if (c < 2u) {
        if (v < mn[c]) mn[c] = v;
        if (v > mx[c]) mx[c] = v;
      }
#endif
    }
  }
  for (uint32_t c = 0; c < ADC_CH; c++) s_raw[c] = (uint16_t)(sum[c] / ADC_HALF_SAMPLES);
#if APP_INPUT_TEST
  for (uint32_t c = 0; c < 2u; c++) {
    s_t_last[c] = p[(ADC_HALF_SAMPLES - 1u) * ADC_CH + c];
    s_t_min[c] = mn[c];
    s_t_max[c] = mx[c];
    s_t_sum[c] += sum[c];
  }
  s_t_n += ADC_HALF_SAMPLES;
#endif
}

void input_tick(void)
{
  for (int b = 0; b < BTN_COUNT; b++) {
    uint8_t pressed = (HAL_GPIO_ReadPin(BTN_HW[b].port, BTN_HW[b].pin) == GPIO_PIN_RESET) ? 1u : 0u;
    uint8_t h = (uint8_t)(((s_btn_hist[b] << 1) | pressed) & 0x03u);
    s_btn_hist[b] = h;
    if (h == 0x03u && !s_btn_state[b]) {
      s_btn_state[b] = true;
      s_btn_press_flags |= (uint8_t)(1u << b);
    } else if (h == 0x00u && s_btn_state[b]) {
      s_btn_state[b] = false;
      s_btn_release_flags |= (uint8_t)(1u << b);
    }
  }
}

void input_on_exti(uint16_t gpio_pin)
{
  if (gpio_pin == BTN_MODE_Pin || gpio_pin == ENC_SW_Pin || gpio_pin == JOY_SW_Pin) {
    s_exti_edges++;
  }
}

/* ---------------------------------------------------------------- main-loop side */
static float axis(uint16_t raw, uint16_t center, float sign)
{
  float span = (raw >= center) ? (float)(ADC_FULL_SCALE - center) : (float)center;
  if (span < 1.0f) span = 1.0f;
  float v = ((float)raw - (float)center) / span;
  if (absf(v) < JOY_DEADZONE) return 0.0f;
  /* rescale so the output starts at 0 just outside the dead zone */
  v = (v > 0.0f) ? (v - JOY_DEADZONE) / (1.0f - JOY_DEADZONE) : (v + JOY_DEADZONE) / (1.0f - JOY_DEADZONE);
  return clampf(v * sign, -1.0f, 1.0f);
}

float input_joy_x(void) { return axis(s_raw[0], s_center_x, JOY_X_SIGN); }
float input_joy_y(void) { return axis(s_raw[1], s_center_y, JOY_Y_SIGN); }

bool input_joy_centered(void)
{
  float x = ((float)s_raw[0] - (float)s_center_x) / 2048.0f;
  float y = ((float)s_raw[1] - (float)s_center_y) / 2048.0f;
  return absf(x) < JOY_CENTER_TOL && absf(y) < JOY_CENTER_TOL;
}

uint32_t input_rail_mv(void)
{
  uint32_t pin_mv = ((uint32_t)s_raw[2] * ADC_VREF_MV) / ADC_FULL_SCALE;
  return (pin_mv * APP_RAIL_DIV_NUM) / APP_RAIL_DIV_DEN;
}

uint16_t input_raw_x(void)    { return s_raw[0]; }
uint16_t input_raw_y(void)    { return s_raw[1]; }
uint16_t input_raw_rail(void) { return s_raw[2]; }

static void calibrate_center_once(void)
{
  if (s_center_done || ms_since(HAL_GetTick(), s_init_ms) < 300u) return;
  s_center_done = true;
  uint16_t x = s_raw[0], y = s_raw[1];
  /* only trust the reading if the stick looks released */
  if (x > 1748u && x < 2348u && y > 1748u && y < 2348u) {
    s_center_x = x;
    s_center_y = y;
  }
  LOG("input: joystick centre %u / %u\r\n", s_center_x, s_center_y);
}

#if INPUT_EVT_LOG
static int32_t s_t_detents;              /* sum of posted EVT_ENC_STEP v[0] since boot */

static void test_evt(evt_type_t type, const char *name, int32_t v0, bool ok)
{
  if (type == EVT_ENC_STEP) {
    s_t_detents += v0;
    LOG("evt: ENC_STEP %+ld (sum %+ld)%s\r\n", (long)v0, (long)s_t_detents, ok ? "" : " QUEUE FULL, dropped");
  } else {
    LOG("evt: %s%s\r\n", name, ok ? "" : " QUEUE FULL, dropped");
  }
}
#endif

static void post(evt_type_t type, const char *name, int32_t v0)
{
  bool ok = app_event_post(type, 0, v0, 0, 0);
#if INPUT_EVT_LOG
  test_evt(type, name, v0, ok);
#else
  (void)name;
  (void)ok;
#endif
}

void input_step(void)
{
  calibrate_center_once();

  uint32_t p = __get_PRIMASK();
  __disable_irq();
  uint8_t flags = s_btn_press_flags;
  uint8_t released = s_btn_release_flags;
  s_btn_press_flags = 0u;
  s_btn_release_flags = 0u;
  __set_PRIMASK(p);

  if (flags & (1u << BTN_MODE_I)) post(EVT_BTN_MODE, "BTN_MODE", 0);
  if (flags & (1u << BTN_ENC_I))  post(EVT_ENC_PRESS, "ENC_PRESS", 0);
  if ((flags & (1u << BTN_JOY_I)) && input_joy_centered()) post(EVT_JOY_PRESS, "JOY_PRESS", 0);

  int16_t cnt = (int16_t)__HAL_TIM_GET_COUNTER(&htim1);
  int16_t enc_delta = (int16_t)(cnt - s_enc_last);
  s_enc_accum += enc_delta;
  s_enc_last = cnt;
  int32_t detents = s_enc_accum / ENC_COUNTS_PER_DETENT;
  if (detents != 0) {
    s_enc_accum -= detents * ENC_COUNTS_PER_DETENT;
    post(EVT_ENC_STEP, "ENC_STEP", detents * ENC_SIGN);
  }

#if APP_INPUT_TEST
  test_step(flags, released, enc_delta);
#else
  (void)released;
#endif
}

#if APP_INPUT_TEST
/* ---------------------------------------------------------------- bench test logging */
#define TEST_PRINT_MS   100u
#define TEST_WINDOW_MS  2000u

static const char *const BTN_NAME[BTN_COUNT] = { "MODE", "ENC", "JOY" };

static uint32_t s_t_print_ms;
static uint32_t s_t_win_ms;
static uint16_t s_w_min[2] = { 0xFFFFu, 0xFFFFu };
static uint16_t s_w_max[2];
static uint32_t s_w_sum[2];
static uint32_t s_w_n;
static int32_t  s_t_enc_total;           /* raw counts since boot, before ENC_SIGN */
static int32_t  s_t_enc_printed;         /* s_t_enc_total at the last enc line */

static void test_step(uint8_t press, uint8_t release, int32_t enc_delta)
{
  uint32_t now = HAL_GetTick();
  s_t_enc_total += enc_delta;

  for (int b = 0; b < BTN_COUNT; b++) {
    if (press & (1u << b)) {
      LOG("btn: %s press%s, exti edges %lu\r\n", BTN_NAME[b],
          (b == BTN_JOY_I) ? (input_joy_centered() ? " (centred)" : " (NOT centred: no event)") : "",
          (unsigned long)s_exti_edges);
    }
    if (release & (1u << b)) LOG("btn: %s release\r\n", BTN_NAME[b]);
  }

  if (ms_since(now, s_t_print_ms) < TEST_PRINT_MS) return;
  s_t_print_ms = now;

  /* take the isr's 100 ms window and start a new one */
  uint16_t last[2], mn[2], mx[2];
  uint32_t sum[2], n;
  uint32_t pm = __get_PRIMASK();
  __disable_irq();
  for (int c = 0; c < 2; c++) {
    last[c] = s_t_last[c];
    mn[c] = s_t_min[c];
    mx[c] = s_t_max[c];
    sum[c] = s_t_sum[c];
    s_t_min[c] = 0xFFFFu;
    s_t_max[c] = 0u;
    s_t_sum[c] = 0u;
  }
  n = s_t_n;
  s_t_n = 0u;
  __set_PRIMASK(pm);

  if (n == 0u) {
    LOG("joy: no adc samples in %lu ms (ADC1 / TIM2 not running?)\r\n", (unsigned long)TEST_PRINT_MS);
  } else {
    LOG("joy: raw %4u %4u avg %4u %4u min-max %4u-%4u %4u-%4u out %5ld %5ld%s\r\n",
        last[0], last[1], s_raw[0], s_raw[1], mn[0], mx[0], mn[1], mx[1],
        (long)(input_joy_x() * 1000.0f), (long)(input_joy_y() * 1000.0f),
        input_joy_centered() ? " c" : "");
    for (int c = 0; c < 2; c++) {
      if (mn[c] < s_w_min[c]) s_w_min[c] = mn[c];
      if (mx[c] > s_w_max[c]) s_w_max[c] = mx[c];
      s_w_sum[c] += sum[c];
    }
    s_w_n += n;
  }

  int32_t d = s_t_enc_total - s_t_enc_printed;
  if (d != 0) {
    s_t_enc_printed = s_t_enc_total;
    LOG("enc: cnt %5u d %4ld dir %c total %6ld\r\n", (unsigned)(uint16_t)s_enc_last, (long)d,
        (d > 0) ? '+' : '-', (long)s_t_enc_total);
  }

  if (ms_since(now, s_t_win_ms) >= TEST_WINDOW_MS) {
    s_t_win_ms = now;
    if (s_w_n != 0u) {
      LOG("joy: 2s X min %4u mean %4lu max %4u pp %4u | Y min %4u mean %4lu max %4u pp %4u | n %lu\r\n",
          s_w_min[0], (unsigned long)(s_w_sum[0] / s_w_n), s_w_max[0], (unsigned)(s_w_max[0] - s_w_min[0]),
          s_w_min[1], (unsigned long)(s_w_sum[1] / s_w_n), s_w_max[1], (unsigned)(s_w_max[1] - s_w_min[1]),
          (unsigned long)s_w_n);
    }
    for (int c = 0; c < 2; c++) {
      s_w_min[c] = 0xFFFFu;
      s_w_max[c] = 0u;
      s_w_sum[c] = 0u;
    }
    s_w_n = 0u;
  }
}
#endif
