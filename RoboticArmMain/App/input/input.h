/* input.h - KY-023 joystick (ADC1 + DMA, TIM2 1 kHz), KY-040 knob (TIM1 encoder), and their buttons
 *
 * One-shot actions come as events (EVT_ENC_STEP, EVT_ENC_PRESS, EVT_JOY_PRESS); the joystick is
 * read with input_joy_x/y every loop. This module never moves anything.
 */
#ifndef APP_INPUT_H
#define APP_INPUT_H

#include <stdint.h>
#include <stdbool.h>

void     input_init(void);                 /* starts TIM2, ADC1 DMA, TIM1 encoder */
void     input_tick(void);                 /* TIM6 isr: samples + debounces the buttons */
void     input_step(void);                 /* main loop: posts button and encoder events */
void     input_on_adc(int half);           /* ADC dma isr: 0 = first half ready, 1 = second */
void     input_on_exti(uint16_t gpio_pin); /* EXTI isr: button edges (stats only) */

float    input_joy_x(void);                /* -1..+1 after dead zone, 0 = centred */
float    input_joy_y(void);
bool     input_joy_centered(void);

/* raw values for debugging */
uint16_t input_raw_x(void);
uint16_t input_raw_y(void);

#endif /* APP_INPUT_H */
