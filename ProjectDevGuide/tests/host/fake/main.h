/* host fake of main.h + the bits of HAL/CMSIS the App code uses */
#ifndef FAKE_MAIN_H
#define FAKE_MAIN_H
#include <stdint.h>
#include <stddef.h>
typedef enum { HAL_OK = 0, HAL_ERROR } HAL_StatusTypeDef;
typedef enum { GPIO_PIN_RESET = 0, GPIO_PIN_SET } GPIO_PinState;
typedef struct { int id; } GPIO_TypeDef;
extern GPIO_TypeDef fGPIOB, fGPIOC, fGPIOE;
#define GPIOB (&fGPIOB)
#define GPIOC (&fGPIOC)
#define GPIOE (&fGPIOE)
#define GPIO_PIN_0  0x0001u
#define GPIO_PIN_2  0x0004u
#define GPIO_PIN_7  0x0080u
#define GPIO_PIN_8  0x0100u
#define GPIO_PIN_10 0x0400u
#define GPIO_PIN_11 0x0800u
#define GPIO_PIN_12 0x1000u
#define GPIO_PIN_14 0x4000u
#define GPIO_PIN_15 0x8000u
#define BTN_ENABLE_Pin GPIO_PIN_12
#define BTN_ENABLE_GPIO_Port GPIOE
#define ESTOP_NO_Pin GPIO_PIN_7
#define ESTOP_NO_GPIO_Port GPIOE
#define BTN_MODE_Pin GPIO_PIN_10
#define BTN_MODE_GPIO_Port GPIOE
#define ENC_SW_Pin GPIO_PIN_14
#define ENC_SW_GPIO_Port GPIOE
#define JOY_SW_Pin GPIO_PIN_15
#define JOY_SW_GPIO_Port GPIOE
#define LIM1_Pin GPIO_PIN_8
#define LIM1_GPIO_Port GPIOE
#define LIM2_Pin GPIO_PIN_11
#define LIM2_GPIO_Port GPIOB
#define LIM3_Pin GPIO_PIN_0
#define LIM3_GPIO_Port GPIOE
#define LIM4_Pin GPIO_PIN_2
#define LIM4_GPIO_Port GPIOE
#define LD1_Pin GPIO_PIN_0
#define LD1_GPIO_Port GPIOB
#define LD2_Pin GPIO_PIN_7
#define LD2_GPIO_Port GPIOB
#define LD3_Pin GPIO_PIN_14
#define LD3_GPIO_Port GPIOB
GPIO_PinState HAL_GPIO_ReadPin(GPIO_TypeDef *p, uint16_t pin);
void HAL_GPIO_WritePin(GPIO_TypeDef *p, uint16_t pin, GPIO_PinState s);
void HAL_GPIO_TogglePin(GPIO_TypeDef *p, uint16_t pin);
uint32_t HAL_GetTick(void);
static inline uint32_t __get_PRIMASK(void) { return 0; }
static inline void __disable_irq(void) {}
static inline void __set_PRIMASK(uint32_t p) { (void)p; }
/* timers */
typedef struct { int dummy; } TIM_TypeDef;
extern TIM_TypeDef fTIM3, fTIM4, fTIM6;
#define TIM6 (&fTIM6)
typedef struct { TIM_TypeDef *Instance; uint32_t ccr[5]; } TIM_HandleTypeDef;
#define TIM_CHANNEL_1 0u
#define TIM_CHANNEL_2 1u
#define TIM_CHANNEL_3 2u
#define TIM_CHANNEL_4 3u
#define __HAL_TIM_SET_COMPARE(h, ch, v) ((h)->ccr[(ch)] = (v))
HAL_StatusTypeDef HAL_TIM_PWM_Start(TIM_HandleTypeDef *h, uint32_t ch);
/* uart */
typedef struct { int dummy; } USART_TypeDef;
typedef struct { USART_TypeDef *Instance; } UART_HandleTypeDef;
HAL_StatusTypeDef HAL_UARTEx_ReceiveToIdle_DMA(UART_HandleTypeDef *h, uint8_t *p, uint16_t n);
HAL_StatusTypeDef HAL_UART_Transmit_DMA(UART_HandleTypeDef *h, const uint8_t *p, uint16_t n);
HAL_StatusTypeDef HAL_UART_AbortReceive(UART_HandleTypeDef *h);
#endif
