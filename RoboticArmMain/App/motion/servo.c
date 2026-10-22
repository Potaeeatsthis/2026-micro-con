/* servo.c */
#include "motion/servo.h"
#include "common/app_util.h"
#include "main.h"
#include "tim.h"

extern const servo_cal_t SERVO_CAL[ARM_N_JOINTS];

typedef struct {
  TIM_HandleTypeDef *htim;
  uint32_t           channel;
} servo_hw_t;

static const servo_hw_t HW[ARM_N_JOINTS] = {
  { &htim3, TIM_CHANNEL_1 },   /* J1 PC6  */
  { &htim3, TIM_CHANNEL_2 },   /* J2 PC7  */
  { &htim4, TIM_CHANNEL_1 },   /* J3 PD12 */
  { &htim4, TIM_CHANNEL_2 },   /* J4 PD13 */
  { &htim4, TIM_CHANNEL_3 },   /* J5 PD14 */
  { &htim4, TIM_CHANNEL_4 },   /* J6 PD15 */
};

#define SERVO_US_MIN 500u      /* hard clamp, whatever the table says */
#define SERVO_US_MAX 2500u

static volatile bool s_on;

void servo_init(void)
{
  for (int j = 0; j < ARM_N_JOINTS; j++) {
    __HAL_TIM_SET_COMPARE(HW[j].htim, HW[j].channel, 0u);
    (void)HAL_TIM_PWM_Start(HW[j].htim, HW[j].channel);
  }
  s_on = false;
}

const servo_cal_t *servo_cal(int joint)
{
  if (joint < 0 || joint >= ARM_N_JOINTS) return &SERVO_CAL[0];
  return &SERVO_CAL[joint];
}

uint16_t servo_deg_to_us(int joint, float deg)
{
  const servo_cal_t *c = servo_cal(joint);
  deg = clampf(deg, 0.0f, 180.0f);
  float us;
  if (deg <= 90.0f) {
    us = (float)c->us_0 + ((float)c->us_90 - (float)c->us_0) * (deg / 90.0f);
  } else {
    us = (float)c->us_90 + ((float)c->us_180 - (float)c->us_90) * ((deg - 90.0f) / 90.0f);
  }
  return (uint16_t)clampf(us + 0.5f, (float)SERVO_US_MIN, (float)SERVO_US_MAX);
}

void servo_write_us(int joint, uint16_t us)
{
  if (joint < 0 || joint >= ARM_N_JOINTS) return;
  if (us != 0u) {
    if (us < SERVO_US_MIN) us = SERVO_US_MIN;
    if (us > SERVO_US_MAX) us = SERVO_US_MAX;
    s_on = true;
  }
  __HAL_TIM_SET_COMPARE(HW[joint].htim, HW[joint].channel, us);
}

void servo_write_deg(int joint, float deg)
{
  servo_write_us(joint, servo_deg_to_us(joint, deg));
}

void servo_all_off(void)
{
  for (int j = 0; j < ARM_N_JOINTS; j++) {
    __HAL_TIM_SET_COMPARE(HW[j].htim, HW[j].channel, 0u);
  }
  s_on = false;
}

bool servo_outputs_on(void) { return s_on; }
