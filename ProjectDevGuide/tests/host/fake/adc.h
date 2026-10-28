/* host fake of the CubeMX adc.h, for input.c */
#ifndef FAKE_ADC_H
#define FAKE_ADC_H
#include "main.h"
typedef struct { int dummy; } ADC_HandleTypeDef;
extern ADC_HandleTypeDef hadc1;
HAL_StatusTypeDef HAL_ADC_Start_DMA(ADC_HandleTypeDef *h, uint32_t *p, uint32_t n);
#endif
