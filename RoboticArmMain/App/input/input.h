/* input.h - KY-023 joystick + servo-rail adc, KY-040 encoder, MODE / encoder / joystick buttons
 *
 * owner: input member.
 * outputs:
 *   - events (EVT_BTN_MODE, EVT_ENC_STEP, EVT_ENC_PRESS, EVT_JOY_PRESS) for one-shot actions
 *   - getters (input_joy_x/y) for continuous values, read by the active mode every loop
 * hold-to-enable and e-stop are NOT here, they belong to safety/. this module never moves
 * anything: modes read the getters / events and set motion targets.
 * buttons: debounced press and release are tracked; only presses become events.
 * APP_INPUT_TEST (local config only): 10 Hz bench log of raw / averaged / dead-zone joystick
 * values, encoder counts and button press/release, see input.c.
 *
 * hardware: ADC1 ranks IN3 (PA3 joy X), IN10 (PC0 joy Y), IN13 (PC3 rail), triggered
 * by TIM2 at 1 kHz, DMA circular. TIM1 encoder mode on PE9/PE11, 4 counts per detent.
 */
#ifndef APP_INPUT_H
#define APP_INPUT_H

#include <stdint.h>
#include <stdbool.h>

void     input_init(void);                 /* starts TIM2, ADC1 DMA, TIM1 encoder */
void     input_tick(void);                 /* TIM6 isr: samples + debounces the buttons */
void     input_step(void);                 /* main loop: posts button and encoder events */
void     input_on_adc(int half);           /* ADC dma isr: 0 = first half ready, 1 = second */
void     input_on_exti(uint16_t gpio_pin); /* EXTI isr: MODE / ENC_SW / JOY_SW edges (stats only) */

float    input_joy_x(void);                /* -1..+1 after dead zone, 0 = centred */
float    input_joy_y(void);
bool     input_joy_centered(void);
uint32_t input_rail_mv(void);              /* servo rail in mV (divider already removed) */

/* raw values for the calibration screen / debugging */
uint16_t input_raw_x(void);
uint16_t input_raw_y(void);
uint16_t input_raw_rail(void);

#endif /* APP_INPUT_H */
