/* input.c
 *
 * button debounce: sampled in the 50 Hz control tick; a press counts after 2 equal
 * samples (20-40 ms). sampling in a timer isr means a press is never missed while the
 * main loop is busy with the CNN or the LCD. the EXTI lines on these pins are kept
 * (course requirement + quick edge stats) but the decision is made here.
 *
 * TODO(input): check the joystick axis directions and the encoder direction on the
 *              real panel and flip JOY_X_SIGN / JOY_Y_SIGN / ENC_SIGN if needed.
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
#define JOY_Y_SIGN        1.0f
#define ENC_SIGN          1
#define ENC_COUNTS_PER_DETENT 4

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
static volatile uint32_t s_exti_edges;

/* encoder */
static int16_t s_enc_last;
static int32_t s_enc_accum;

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
  for (uint32_t i = 0; i < ADC_HALF_SAMPLES; i++) {
    for (uint32_t c = 0; c < ADC_CH; c++) sum[c] += p[i * ADC_CH + c];
  }
  for (uint32_t c = 0; c < ADC_CH; c++) s_raw[c] = (uint16_t)(sum[c] / ADC_HALF_SAMPLES);
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
    } else if (h == 0x00u) {
      s_btn_state[b] = false;
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

void input_step(void)
{
  calibrate_center_once();

  uint32_t p = __get_PRIMASK();
  __disable_irq();
  uint8_t flags = s_btn_press_flags;
  s_btn_press_flags = 0u;
  __set_PRIMASK(p);

  if (flags & (1u << BTN_MODE_I)) (void)app_event_post0(EVT_BTN_MODE);
  if (flags & (1u << BTN_ENC_I))  (void)app_event_post0(EVT_ENC_PRESS);
  if ((flags & (1u << BTN_JOY_I)) && input_joy_centered()) (void)app_event_post0(EVT_JOY_PRESS);

  int16_t cnt = (int16_t)__HAL_TIM_GET_COUNTER(&htim1);
  s_enc_accum += (int16_t)(cnt - s_enc_last);
  s_enc_last = cnt;
  int32_t detents = s_enc_accum / ENC_COUNTS_PER_DETENT;
  if (detents != 0) {
    s_enc_accum -= detents * ENC_COUNTS_PER_DETENT;
    (void)app_event_post(EVT_ENC_STEP, 0, detents * ENC_SIGN, 0, 0);
  }
}
