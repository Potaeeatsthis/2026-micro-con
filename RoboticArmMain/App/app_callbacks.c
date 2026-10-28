/* app_callbacks.c - the only file that defines HAL_*Callback functions (CI checks this)
 *
 * Route by instance to one function of the owning module, no logic here. Everything called
 * from here runs in an isr: keep it short (no HAL_Delay, printf or LCD).
 * Keep SysTick as the HAL timebase, otherwise CubeMX generates a second
 * HAL_TIM_PeriodElapsedCallback in main.c.
 */
#include "main.h"
#include "app.h"
#include "common/app_config.h"
#include "common/app_fault.h"
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
    app_control_tick();                           /* 50 Hz: input sample, motion */
  }
}

/* ---------------------------------------------------------------- gpio exti */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
#if APP_USE_INPUT
  input_on_exti(GPIO_Pin);                        /* ENC_SW, JOY_SW */
#endif
}

/* ---------------------------------------------------------------- adc1 (joystick) */
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
 * add new callbacks here (e.g. SPI5 tx done if the LCD moves to DMA) */
