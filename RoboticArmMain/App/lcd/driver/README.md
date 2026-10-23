# Lab08 ILI9341 driver goes here

Copy from `D:\Microcon69\ClassLab\Lab08`:
`ILI9341_STM32_Driver.c/.h`, `ILI9341_GFX.c/.h`, `ILI9341_Touchscreen.c/.h`, `5x5_font.h`.

Then:
- rename `CS_Pin` / `DC_Pin` / `RST_Pin` (and their `_GPIO_Port`) to `LCD_CS` / `LCD_DC` / `LCD_RST`
- change the touch port defines from `GPIOE` to the new labels `T_CLK`, `T_CS`, `T_MOSI`, `T_MISO`, `T_IRQ`
- make sure the driver uses `hspi5`
- fill in `../lcd_port.c`; nothing else in `App/` includes the driver directly
