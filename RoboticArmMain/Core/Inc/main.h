/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f7xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LIM4_Pin GPIO_PIN_2
#define LIM4_GPIO_Port GPIOE
#define LIM4_EXTI_IRQn EXTI2_IRQn
#define USER_BTN_Pin GPIO_PIN_13
#define USER_BTN_GPIO_Port GPIOC
#define LD1_Pin GPIO_PIN_0
#define LD1_GPIO_Port GPIOB
#define ESTOP_NO_Pin GPIO_PIN_7
#define ESTOP_NO_GPIO_Port GPIOE
#define ESTOP_NO_EXTI_IRQn EXTI9_5_IRQn
#define LIM1_Pin GPIO_PIN_8
#define LIM1_GPIO_Port GPIOE
#define LIM1_EXTI_IRQn EXTI9_5_IRQn
#define BTN_MODE_Pin GPIO_PIN_10
#define BTN_MODE_GPIO_Port GPIOE
#define BTN_MODE_EXTI_IRQn EXTI15_10_IRQn
#define BTN_ENABLE_Pin GPIO_PIN_12
#define BTN_ENABLE_GPIO_Port GPIOE
#define BTN_ENABLE_EXTI_IRQn EXTI15_10_IRQn
#define ENC_SW_Pin GPIO_PIN_14
#define ENC_SW_GPIO_Port GPIOE
#define ENC_SW_EXTI_IRQn EXTI15_10_IRQn
#define JOY_SW_Pin GPIO_PIN_15
#define JOY_SW_GPIO_Port GPIOE
#define JOY_SW_EXTI_IRQn EXTI15_10_IRQn
#define LIM2_Pin GPIO_PIN_11
#define LIM2_GPIO_Port GPIOB
#define LIM2_EXTI_IRQn EXTI15_10_IRQn
#define LD3_Pin GPIO_PIN_14
#define LD3_GPIO_Port GPIOB
#define T_MISO_Pin GPIO_PIN_2
#define T_MISO_GPIO_Port GPIOG
#define T_IRQ_Pin GPIO_PIN_3
#define T_IRQ_GPIO_Port GPIOG
#define LCD_CS_Pin GPIO_PIN_8
#define LCD_CS_GPIO_Port GPIOC
#define LCD_DC_Pin GPIO_PIN_9
#define LCD_DC_GPIO_Port GPIOC
#define LCD_RST_Pin GPIO_PIN_10
#define LCD_RST_GPIO_Port GPIOC
#define T_CLK_Pin GPIO_PIN_11
#define T_CLK_GPIO_Port GPIOC
#define T_CS_Pin GPIO_PIN_12
#define T_CS_GPIO_Port GPIOC
#define T_MOSI_Pin GPIO_PIN_2
#define T_MOSI_GPIO_Port GPIOD
#define LD2_Pin GPIO_PIN_7
#define LD2_GPIO_Port GPIOB
#define LIM3_Pin GPIO_PIN_0
#define LIM3_GPIO_Port GPIOE
#define LIM3_EXTI_IRQn EXTI0_IRQn

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
