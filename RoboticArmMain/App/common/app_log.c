/* app_log.c - printf retarget. syscalls.c (generated) calls __io_putchar for every char.
 * if the linker reports __io_putchar defined twice, delete the other copy
 * (old projects put one in main.c or app_x-cube-ai.c) - this is the only one */
#include "common/app_log.h"
#include "main.h"
#include "usart.h"

int __io_putchar(int ch)
{
  uint8_t c = (uint8_t)ch;
  HAL_UART_Transmit(&huart3, &c, 1, 10);
  return ch;
}

void app_log_init(void)
{
  /* unbuffered: lines appear immediately, nothing is lost on a hard fault */
  setvbuf(stdout, NULL, _IONBF, 0);
}
