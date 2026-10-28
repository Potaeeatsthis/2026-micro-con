#include "main.h"
extern TIM_HandleTypeDef htim3, htim4, htim6;
/* input.c (encoder on TIM1, ADC trigger on TIM2): only the input test defines these */
extern TIM_HandleTypeDef htim1, htim2;
#define TIM_CHANNEL_ALL 0x3Cu
uint32_t fake_tim_counter(TIM_HandleTypeDef *h);   /* defined by the test that links input.c */
#define __HAL_TIM_GET_COUNTER(h) fake_tim_counter(h)
HAL_StatusTypeDef HAL_TIM_Encoder_Start(TIM_HandleTypeDef *h, uint32_t ch);
HAL_StatusTypeDef HAL_TIM_Base_Start(TIM_HandleTypeDef *h);
