/* app_callbacks.c - THE ONLY FILE THAT DEFINES HAL_*Callback FUNCTIONS
 *
 * owner: core.
 * HAL declares each callback as __weak; a second definition anywhere (main.c, a feature
 * file, app_x-cube-ai.c) is a link error or, worse, silently replaces this one.
 * CI fails any PR that defines HAL_*Callback outside this file.
 *
 * rules:
 *   - route by instance and call ONE function of the owning module; no logic here
 *   - every routed function must be isr-safe: short, no HAL_Delay, no printf, no LCD
 *   - a new callback: add it in the EXTENSION POINT section at the bottom, in a PR
 *
 * note: SysTick is the HAL timebase (set in the .ioc), so CubeMX does NOT generate a
 * HAL_TIM_PeriodElapsedCallback in main.c. if someone switches the timebase to a TIM,
 * that generated copy collides with the one below - keep SysTick.
 */
#include "main.h"
#include "app.h"
#include "common/app_config.h"
#include "common/app_fault.h"
#include "safety/safety.h"
#include "tim.h"
#include "adc.h"
#include "sai.h"
#include "usart.h"
#if APP_USE_INPUT
#include "input/input.h"
#endif
#if APP_USE_VOICE
#include "voice/voice.h"
#endif
#if APP_USE_PI
#include "pi_link/pi_link.h"
#endif

/* ---------------------------------------------------------------- timers */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Instance == TIM6) {
    app_control_tick();                           /* 50 Hz: input sample, safety, motion */
  }
}

/* ---------------------------------------------------------------- gpio exti
 * every module checks whether the pin is one of its own, so the order does not matter */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  safety_on_exti(GPIO_Pin);                       /* BTN_ENABLE, ESTOP_NO, LIM1..4 */
#if APP_USE_INPUT
  input_on_exti(GPIO_Pin);                        /* BTN_MODE, ENC_SW, JOY_SW */
#endif
}

/* ---------------------------------------------------------------- adc1 (joystick + rail) */
void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef *hadc)
{
#if APP_USE_INPUT
  if (hadc->Instance == ADC1) input_on_adc(0);
#else
  (void)hadc;
#endif
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
#if APP_USE_INPUT
  if (hadc->Instance == ADC1) input_on_adc(1);
#else
  (void)hadc;
#endif
}

void HAL_ADC_ErrorCallback(ADC_HandleTypeDef *hadc)
{
  if (hadc->Instance == ADC1) app_fault_raise(FAULT_ADC, false);
}

/* ---------------------------------------------------------------- sai1 block A (microphones) */
void HAL_SAI_RxHalfCpltCallback(SAI_HandleTypeDef *hsai)
{
#if APP_USE_VOICE
  if (hsai->Instance == SAI1_Block_A) voice_on_sai(0);
#else
  (void)hsai;
#endif
}

void HAL_SAI_RxCpltCallback(SAI_HandleTypeDef *hsai)
{
#if APP_USE_VOICE
  if (hsai->Instance == SAI1_Block_A) voice_on_sai(1);
#else
  (void)hsai;
#endif
}

void HAL_SAI_ErrorCallback(SAI_HandleTypeDef *hsai)
{
  if (hsai->Instance == SAI1_Block_A) app_fault_raise(FAULT_SAI, false);
}

/* ---------------------------------------------------------------- uarts
 * USART6 = pi link. USART3 = debug printf (blocking, no callbacks needed) */
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
#if APP_USE_PI
  if (huart->Instance == USART6) pi_link_on_rx_event(Size);
#else
  (void)huart;
  (void)Size;
#endif
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
#if APP_USE_PI
  if (huart->Instance == USART6) pi_link_on_tx_done();
#else
  (void)huart;
#endif
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
#if APP_USE_PI
  if (huart->Instance == USART6) pi_link_on_error();
#else
  (void)huart;
#endif
}

/* ---------------------------------------------------------------- EXTENSION POINT
 * reserved, add when the feature needs it (each in a PR, routed like above):
 *   HAL_SPI_TxCpltCallback      SPI5 -> lcd_on_spi_done()      if the LCD driver moves to DMA
 *   HAL_TIM_IC_CaptureCallback  a timer input capture
 *   HAL_UART_RxCpltCallback     only if a uart uses plain (non-idle) dma rx
 */
