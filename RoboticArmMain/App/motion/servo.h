/* servo.h - the only code that writes the six PWM compare registers
 *
 * owner: motion member.
 * nobody outside motion/ calls servo_*; modes set targets through motion.h.
 *
 * TIM3/TIM4 run at 1 us per count with a 20 ms period, so CCR = pulse width in us.
 *   J1 PC6 TIM3_CH1   J2 PC7 TIM3_CH2   J3 PD12 TIM4_CH1
 *   J4 PD13 TIM4_CH2  J5 PD14 TIM4_CH3  J6 PD15 TIM4_CH4
 */
#ifndef APP_SERVO_H
#define APP_SERVO_H

#include <stdint.h>
#include <stdbool.h>
#include "common/app_types.h"

/* per-servo calibration: two straight lines 0..90 and 90..180 deg (guide section 13) */
typedef struct {
  const char *name;
  uint16_t us_0;       /* pulse at 0 deg */
  uint16_t us_90;      /* pulse at 90 deg */
  uint16_t us_180;     /* pulse at 180 deg */
  float    min_deg;    /* safe software limits for this joint */
  float    max_deg;
  float    home_deg;   /* pose loaded before the first pulse is sent */
} servo_cal_t;

void               servo_init(void);                 /* starts the 6 channels with CCR = 0 (no pulse) */
void               servo_write_deg(int joint, float deg);
void               servo_write_us(int joint, uint16_t us);
void               servo_all_off(void);              /* CCR = 0: no pulse, servos go limp */
bool               servo_outputs_on(void);
uint16_t           servo_deg_to_us(int joint, float deg);
const servo_cal_t *servo_cal(int joint);

#endif /* APP_SERVO_H */
