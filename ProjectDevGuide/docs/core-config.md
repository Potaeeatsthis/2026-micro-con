# Robotic Arm — Core STM32 Configuration (v1.4)

**Board:** NUCLEO-F767ZI (STM32F767ZIT6, LQFP144)
**Tools (everyone, same versions):** STM32CubeMX 6.18.0 (standalone), STM32CubeIDE 2.2.0, FW_F7 V1.17.4, X-CUBE-AI 10.2.1
**Project:** `RoboticArmMain/` in the team repo `2026-micro-con` (public, owner `Potaeeatsthis`), on `master` since
23 Oct 2026 (PR #2 `b75e68f`; docs + CI hardening PR #3 `dca1b23`)
**`.ioc` owner:** Supanut (only he edits and regenerates the `.ioc`)
**Status (23 Oct 2026):** the `.ioc` is built, audited, generated, built in CubeIDE (0 errors), run on the board
and guarded by CI. This file describes that `.ioc`. Open items (§12) do not affect the `.ioc`.

This file is the single source of truth for pins, clocks, timers, DMA and interrupt
priorities. If code and this file disagree, fix one of them in the same commit.
Its main copy is `ProjectDevGuide/docs/core-config.md` in the repo; change it there, through a PR.

**v1.2 (23 Oct 2026) vs v1.1:** CRC on (X-CUBE-AI); HSE startup timeout 5000 ms; final power only through the
ST-LINK USB (§11); heap and stack 0x2000 each; the SRAM2 MPU region is **region 1** (region 0 = CubeMX's
background region); CubeMX pin traps and setting order (§13); CubeIDE project setup (§14); CI (§15).

**Update 24 Oct 2026 (no `.ioc` change):** servo signals are wired **direct** for now, the SN74HC244N stage failed
(§11); notes from the six-servo bench test (§4.1); voice-word timing for modules that use voice events (§9).

**v1.3 (27 Oct 2026, no `.ioc` change yet):** the arm has **no E-stop, no MODE button and no Hold-to-Enable
button**. Inputs are the KY-040 encoder, KY-023 joystick, LCD touch, 2 mics and the Pi link (USART6); outputs
the 6 servos and the LCD. The mode is chosen on the LCD.

**v1.4 (27 Oct 2026, `.ioc` regenerated):** every pin with nothing wired to it is out of the `.ioc` and the code:
PE7 / PE10 / PE12 (removed buttons), the four limit-switch inputs PE8 / PB11 / PE0 / PE2, and the servo-rail
sense PC3 (ADC1 IN13). ADC1 now converts 2 channels (joystick X / Y); EXTI0, EXTI2 and EXTI9_5 are off.
`safety/` is only the fatal-fault gate; joint limits stay in software (`motion/`, `servo_cal.c`).

---

## 1. What changed from `robotic-arm-guide.md`

| Topic | Old guide | Now |
|---|---|---|
| Voice | INMP441 + speech recognition on the Pi | **2 × INMP441 on STM32 SAI1**, keyword-spotting CNN (X-CUBE-AI) and the voice state machine on the STM32 |
| Inverse kinematics | Pi | **STM32** (so Cartesian Mode works without the Pi) |
| Pi job | Voice, vision, IK, task planning | **Camera only**: object colour, position and depth |
| Pi link | USB UART | **Pi GPIO UART → STM32 USART6** (USART3 / ST-Link USB stays free for debugging) |
| Servo PWM | TIM3 CH1–4 + TIM4 CH1–2 | **TIM3 CH1–2 + TIM4 CH1–4** (TIM3 CH3/CH4 pins are the LCD CS/DC pins) |
| LCD | ILI9341, no touch | ILI9341 **with touch**: mode tabs, jog-distance slider (cm), jog queue |
| Clock | – | 216 MHz from the ST-Link 8 MHz (HSE bypass), needed for an exact 16 kHz audio rate |
| Software | – | Bare-metal: main loop + interrupts (no RTOS), §9 |

Unchanged: four modes, KY-023 joystick + KY-040 encoder, SN74AHCT244N buffer
(planned; servos wired direct until it is fitted, §11), power architecture, shared motion pipeline (every mode only sets targets).

Dropped (v1.3, 27 Oct 2026): MODE button (the LCD mode cards replace it), Hold-to-Enable button, and the
1NO+1NC latching E-stop with its servo-power relay. Nothing cuts servo power in hardware now (§11).
Dropped (v1.4, 27 Oct 2026): the optional limit switches and the servo-rail sense (no room on the arm; nothing
was fitted).

---

## 2. Clock

| Setting | Value |
|---|---|
| HSE | **BYPASS Clock Source**, 8 MHz from the ST-Link MCO on PH0 |
| HSE startup timeout | **5000 ms** (`HSE_STARTUP_TIMEOUT`). The ST-LINK starts its MCO clock late when there is no PC; the HAL default gives up too early and the board hangs in `Error_Handler` |
| Main PLL | source HSE, /M 4 (2 MHz), ×N 216, /P 2 → **SYSCLK 216 MHz**; /Q 9 (48 MHz, unused) |
| AHB / APB1 / APB2 | /1 → 216 MHz; /4 → **54 MHz** (timers 108 MHz); /2 → **108 MHz** (timers 216 MHz) |
| PLLSAI | ×N 213, /Q 13, /DIVQ 4 → SAI1 clock 8.192 MHz → 16,000.6 Hz frame rate (verified on the board) |
| Power | Voltage scale 1 + over-drive, flash latency 7 WS (CubeMX sets these for 216 MHz) |
| SYS | Debug: Serial Wire. Timebase: SysTick |

Do **not** use HSI: it measured 1.4 % fast and breaks the audio sample rate.

---

## 3. Pin map

Avoided on purpose: Ethernet pins (PA1, PA2, PA7, PB13, PC1, PC4, PC5, PG11, PG13),
USB (PA8–PA12, PG6, PG7), SWD (PA13, PA14, PB3), HSE (PH0, PH1), LSE (PC14, PC15).

| Feature | Signal | STM32 pin | Header (Arduino name) | CubeMX mode |
|---|---|---|---|---|
| **Servos** | J1 base | PC6 | CN7-1 (D16) | TIM3_CH1 |
| | J2 shoulder | PC7 | CN7-11 (D21) | TIM3_CH2 |
| | J3 elbow | PD12 | CN10-21 (D29) | TIM4_CH1 |
| | J4 wrist roll | PD13 | CN10-19 (D28) | TIM4_CH2 |
| | J5 wrist pitch | PD14 | CN7-16 (D10) | TIM4_CH3 |
| | J6 gripper | PD15 | CN7-18 (D9) | TIM4_CH4 |
| **Microphones** (INMP441 ×2) | WS / FS | PE4 | CN9-16 (D57) | SAI1_FS_A |
| | SCK | PE5 | CN9-18 (D58) | SAI1_SCK_A |
| | SD | PE6 | CN9-20 (D59) | SAI1_SD_A, **internal pull-down** |
| **LCD** (ILI9341, SPI5) | SCK | PF7 | CN9-26 (D62) | SPI5_SCK |
| | MISO (SDO) | PF8 | CN9-24 (D61) | SPI5_MISO |
| | MOSI (SDI) | PF9 | CN9-28 (D63) | SPI5_MOSI |
| | CS | PC8 | CN8-2 (D43) | GPIO out, label `LCD_CS` |
| | DC | PC9 | CN8-4 (D44) | GPIO out, label `LCD_DC` |
| | RESET | PC10 | CN8-6 (D45) | GPIO out, label `LCD_RST` |
| | LED (backlight) | – | 3.3 V | always on, no pin |
| **Touch** (bit-banged) | T_CLK | PC11 | CN8-8 (D46) | GPIO out, label `T_CLK` |
| | T_CS | PC12 | CN8-10 (D47) | GPIO out, label `T_CS` |
| | T_DIN (MOSI) | PD2 | CN8-12 (D48) | GPIO out, label `T_MOSI` |
| | T_DO (MISO) | PG2 | CN8-14 (D49) | GPIO in, label `T_MISO` |
| | T_IRQ | PG3 | CN8-16 (D50) | GPIO in, label `T_IRQ` (polled) |
| **KY-023 joystick** (3.3 V) | VRx | PA3 | CN9-1 (A0) | ADC1_IN3 |
| | VRy | PC0 | CN9-3 (A1) | ADC1_IN10 |
| | SW (Fine/Fast speed) | PE15 | CN10-30 (D37) | GPIO_EXTI15, pull-up |
| **KY-040 encoder** (3.3 V) | CLK | PE9 | CN10-4 (D6) | TIM1_CH1 (encoder) |
| | DT | PE11 | CN10-6 (D5) | TIM1_CH2 (encoder) |
| | SW | PE14 | CN10-28 (D38) | GPIO_EXTI14, pull-up |
| **Pi link** | STM32 TX → Pi GPIO15 (RXD, pin 10) | PG14 | CN10-14 (D1) | USART6_TX |
| | STM32 RX ← Pi GPIO14 (TXD, pin 8) | PG9 | CN10-16 (D0) | USART6_RX |
| **Debug** | ST-Link virtual COM port | PD8 / PD9 | (USB) | USART3_TX / RX |
| **On-board** | LD1 green / LD2 blue / LD3 red | PB0 / PB7 / PB14 | – | GPIO out, labels `LD1` / `LD2` / `LD3` |
| | User button (blue) | PC13 | – | GPIO in, label `USER_BTN` (not EXTI) |

**EXTI lines are numbered by pin number, not port**, so each number may be used once in the
whole project. Used: 14, 15. Free for later: 0–13 (avoid 13 if the user button ever becomes an
interrupt).

**Removed in v1.4 (27 Oct 2026), now unassigned in the `.ioc`:** PE7, PE10, PE12 (E-stop, MODE, Hold-to-Enable),
PE8, PB11, PE0, PE2 (limit switches) and PC3 (servo-rail sense). Nothing is wired to them; an unassigned pin
stays in its reset state (an input nobody reads), which is harmless. ("Set all free pins as analog" in Project
Manager stays off: it would reconfigure every free pin on the board.) Manual modes (Single Joint, Joint
Group, Cartesian) move whenever no fatal fault is latched, with no Enable button.

**Joystick press (KY-023 SW) = Fine/Fast speed toggle in every manual mode**, accepted only while
the stick is near its centre (so an accidental press while pushing the stick does nothing).
The knob press (KY-040 SW) keeps the mode-specific actions from `robotic-arm-guide.md`.

The encoder and joystick buttons wire **pin → button → GND**; the internal pull-up makes "pressed = 0".

**Mode selection = LCD touch.** A mode card posts `EVT_TOUCH_MODE_TAB` (arg = `arm_mode_t`); `modes.c`
switches when motion is idle and the stick is centred.

**PG9 (USART6 RX) floats while no Pi is connected.** The UART then sees noise at boot and the log shows
one `fault: code 7` (`FAULT_UART_PI`); `pi_link` restarts its receiver and carries on. Harmless. If it
should go away before the Pi is wired, set PG9 to pull-up in CubeMX (GPIO Settings of USART6).

---

## 4. Peripheral settings

### 4.1 Servo PWM — TIM3 (CH1, CH2) and TIM4 (CH1–CH4)
| Setting | Value |
|---|---|
| Clock | internal, 108 MHz |
| Prescaler / Period | **107 / 19999** → 1 µs per count, 20 ms (50 Hz) |
| Mode | PWM Generation CHx, PWM mode 1, polarity high |
| Pulse (initial) | **0** (no pulse until the firmware has loaded its home pose; the current 90° home is a placeholder, not an installed calibration) |
| Auto-reload preload / output-compare preload | Enable / Enable (a new pulse takes effect at the next 20 ms frame) |

`CCR = pulse in µs`. 500–2500 µs fits. Measured **unloaded** pulses at
0°/90°/180° are J1 500/1460/2430, J2 500/1430/2440, J3 540/1530/2500,
J4 500/1440/2370, J5 640/1580/2470, J6 580/1570/2500. Installed limits, zero
and home are still unmeasured, so the firmware's `0/180/90` values are
placeholders and not safe installed limits. Calibration per servo lives in code
(`App/motion/servo_cal.c`), not in CubeMX; the canonical table is
[`servo-calibration.md`](servo-calibration.md).

**Bench test, 24 Oct 2026** (all six servos, direct wiring, test code on branch `test/servo-power-test`,
never merged): the timers and pins above work on all six channels. Lessons for `motion/`:

- **Write the CCRs only from the TIM6 tick, never from the main loop.** The main loop is blocked
  ≈ 43 ms by every CNN run (§9), so a ramp stepped from the loop skips frames and the servos jerk
  and lag behind the log.
- **Small steps disappear in the servo deadband.** Reported deadbands: SG90/SG92R about 10 µs,
  MG996R about 5 µs (datasheets / seller pages, not measured by us). With ≈ 10.7 µs per degree
  (J1: 500 → 2430 µs over 180°), `APP_SPEED_FINE_DEG_S` 10 °/s gives 0.2° ≈ 2 µs per 20 ms frame,
  below the SG deadband, so an SG joint moves in jumps. The test ran at 60 °/s (≈ 13 µs per frame)
  and was smooth. Choose the fine speed with this in mind.

### 4.2 KY-040 — TIM1 encoder mode
Combined channels: Encoder Mode, **Encoder Mode TI1 and TI2**, prescaler 0, period 65535,
IC1/IC2 polarity rising, **input filter 15** (debounce). One detent ≈ 4 counts.

### 4.3 Joystick — ADC1 triggered by TIM2
| Setting | Value |
|---|---|
| TIM2 | internal clock, prescaler 107, period 999 → **1 kHz**, TRGO = Update Event, no interrupt |
| ADC1 | 12-bit, clock PCLK2 /4 (27 MHz), **scan on**, continuous off, **DMA continuous requests on**, EOC at end of sequence |
| Trigger | external, **Timer 2 Trigger Out event**, rising edge |
| Ranks | **Number of conversion 2**: 1 = IN3 (PA3, joy X), 2 = IN10 (PC0, joy Y) — both **480 cycles** |
| DMA | DMA2 Stream 0, circular, half-word, priority low; buffer 2 × 20 samples (half-transfer every 10 ms) |

### 4.4 Microphones — SAI1 Block A
Master receive, **master clock output disabled**, I2S standard, **32-bit data, 2 slots, stereo, 16 kHz**.
DMA2 Stream 1, circular, word/word, priority high, 640-word buffer (10 ms per half).

### 4.5 LCD — SPI5
Full-duplex master, 8 bit, MSB first, **prescaler 2 → 54 Mbit/s** (Lab08 ran 50 Mbit/s;
use 4 → 27 Mbit/s if the picture glitches), CPOL low, CPHA 1 edge, NSS software, CRC off.
CS/DC/RST and the touch outputs: GPIO speed Very High, start level high for CS/T_CS/RST.

### 4.6 UARTs
| | USART6 — Pi | USART3 — debug (ST-Link USB) |
|---|---|---|
| Baud | **115200 8N1** | **921600 8N1, oversampling 8** (matches the voice PC tools) |
| RX | DMA2 Stream 2, circular, + idle-line (`HAL_UARTEx_ReceiveToIdle_DMA`) | not used |
| TX | DMA2 Stream 7, normal | DMA1 Stream 3, normal |
| Interrupt | USART6 global on | USART3 global on |

The Pi-link code uses one macro `#define PI_UART huart6`, so moving it to USB (`huart3`) is one line.

Pi side: `enable_uart=1` and `dtoverlay=disable-bt` in `config.txt`, remove the serial console
from `cmdline.txt`, then use `/dev/serial0`. Both sides are 3.3 V: connect TX→RX, RX←TX, GND–GND.

### 4.7 Control tick — TIM6
Prescaler 10799, period **199** → **50 Hz**, TIM6 global interrupt on. Runs the control tick (§9).

### 4.8 X-CUBE-AI 10.2.1 and CRC
- Model `Models/kws.onnx` (exp04, copied from `voice_command_research/final_model/exp04_tcresnet-w16d3/onnx_out/`;
  **relative path inside the project**), network name `network`, float32, compression none, **Application Template**.
- Analyze result: MACC 1,483,140; flash 248,192 B; RAM 40,452 B. One inference ≈ 43.2 ms on the board.
- **CRC is enabled** (Computing → CRC → Activated): the X-CUBE-AI runtime needs it.
- USART3's "Do Not Generate Function Call" stays **unticked** after adding X-CUBE-AI.
- The firmware keeps its own copy of the model: a new research release changes nothing until the `.ioc`
  owner copies the new `kws.onnx` **and** its `kws_classes.h` together, regenerates and rebuilds.

### 4.9 Removed from Lab08
AM2320 / I2C1, RNG, Lab08's TIM3 backlight PWM, Lab08's ADC pot.

---

## 5. DMA summary

| Stream | Request | Mode | Priority |
|---|---|---|---|
| DMA2 Stream 0 | ADC1 | circular | low |
| DMA2 Stream 1 | SAI1_A RX | circular | high |
| DMA2 Stream 2 | USART6_RX | circular | low |
| DMA2 Stream 7 | USART6_TX | normal | low |
| DMA1 Stream 3 | USART3_TX | normal | low |
| (free) DMA2 Stream 4 | SPI5_TX later, if LCD drawing must get faster | normal | – |

---

## 6. Interrupt priorities (NVIC priority group 4, lower number = more urgent)

| Priority | Interrupt | Why |
|---|---|---|
| 0 | DMA2 Stream 1 (SAI RX) | Audio must never be late. Callback only DC-filters and copies (≈ few µs) |
| 1 | EXTI15_10 (encoder SW, joystick SW) | Button edges (stats only; the press is decided in the TIM6 tick) |
| 2 | TIM6 (control tick) | Smooth servo motion even while the CNN or LCD is busy |
| 3 | USART6 + DMA2 Streams 2/7 | Pi messages |
| 4 | DMA2 Stream 0 (ADC) | Joystick |
| 5 | USART3 + DMA1 Stream 3 | Debug only |
| 15 | SysTick | Default |

Rules for every interrupt handler: short, no `HAL_Delay`, no LCD drawing, no `printf`.

---

## 7. Cortex-M7, cache and memory

| Setting | Value |
|---|---|
| ICache / DCache | **enabled** (the CNN needs the cache) |
| MPU control | "Background region privileged accesses only + MPU disabled during hard fault, NMI and FAULTMASK" |
| MPU region 0 | CubeMX's default background region (base 0x0, 4 GB, no access, sub-region disable 0x87). Leave it |
| **MPU region 1** | base **0x2007C000 (SRAM2)**, size **16 KB**, TEX level 1, full access, instruction access disable, shareable **enable**, cacheable **disable**, bufferable **disable** |
| Stack / heap (Project Manager) | **0x2000 / 0x2000** |

Every DMA buffer goes into this non-cacheable SRAM2 region, so nobody has to think about
cache cleaning/invalidating:

```c
// App/common/dma_buf.h
#define DMA_BUF __attribute__((section(".dma_buffer"), aligned(32)))
// usage:  static uint16_t adc_buf[60] DMA_BUF;
```

Linker script (`STM32F767ZITX_FLASH.ld`, not `_RAM.ld`), edited once after the first generate:

```ld
MEMORY {
  RAM     (xrw) : ORIGIN = 0x20000000, LENGTH = 496K
  RAM_DMA (xrw) : ORIGIN = 0x2007C000, LENGTH = 16K   /* SRAM2, non-cacheable via MPU region 1 */
  FLASH   (rx)  : ORIGIN = 0x08000000, LENGTH = 2048K
}
/* inside SECTIONS, before the "Remove information from the compiler libraries" block: */
.dma_buffer (NOLOAD) :
{
  . = ALIGN(32);
  *(.dma_buffer)
  *(.dma_buffer*)
  . = ALIGN(32);
} >RAM_DMA
```

Three checks keep this honest: `dma_buf_check()` at boot (motion blocked and `ERROR dma_buf` logged if the
section or the MPU region is wrong), the CI build (fails if `.dma_buffer` is not at 0x2007C000 or exceeds
16 KB), and a look at the `.ld` after every regenerate.

Size today (CubeIDE 2.2.0, Debug): text 324,300 B, data 7,980 B, bss 127,712 B — far below 2 MB / 512 KB.
DMA buffers use 0xF00 B of the 16 KB (measured before v1.4; the ADC buffer shrank by one channel since).

---

## 8. Project Manager settings

- Project name `RoboticArmMain`, toolchain STM32CubeIDE, firmware package FW_F7 V1.17.4
- "Generate peripheral initialization as a pair of .c/.h files" **on**
- "Keep User Code when re-generating" **on**, "Delete previously generated files" **on**
- Copy only the necessary library files; `Drivers/` and `Middlewares/` are committed so a fresh clone builds
  without CubeMX or X-CUBE-AI installed
- Register callbacks: off (we use the weak HAL callbacks, §10)

---

## 9. Software model: bare-metal

No RTOS. One `while(1)` loop plus interrupts. Anything that needs exact timing runs in an
interrupt; anything slow runs in the loop.

| Runs in | Job | Time |
|---|---|---|
| SAI DMA interrupt (every 10 ms) | DC filter, push 160 samples per mic into the voice ring | µs |
| TIM6 interrupt (every 20 ms) | `input_tick` → `motion_tick` (debounce, fatal-fault check, trajectory step, write 6 CCRs) | < 0.5 ms target |
| EXTI | count edges (encoder / joystick buttons) | µs |
| USART6 idle-line callback | mark a Pi message as ready | µs |
| Main loop (`app_loop`) | voice features + CNN every 100 ms (≈ 43 ms), Pi message parsing, mode logic + voice task state machine, IK, LCD updates | – |

**The one main-loop rule:** no single job may block for more than **~100 ms**. The voice ring
holds 256 ms of audio; if the loop is away longer, audio is lost (`FAULT_VOICE_OVERRUN`).
So the LCD redraws only the parts that changed, never the whole screen in every loop.

**Voice-word timing (for modules that react to `EVT_VOICE_WORD`).** Settings in `App/voice/voice.c`:

| Setting | Value | Effect |
|---|---|---|
| `VOICE_STRIDE_FRAMES` | 10 (CNN every 100 ms) | the loop is busy ≈ 43 ms of every 100 ms |
| `VOICE_AVG` | 3 | a word must win several runs in a row |
| `VOICE_THRESHOLD` | 0.75 | averaged probability needed |
| `VOICE_MIN_SNR_DB` | 12 dB | quieter sound is ignored |
| `VOICE_REFRACTORY_MS` | 1500 ms | no second word within 1.5 s |

So a word event arrives a few CNN runs after the word, and two words closer than 1.5 s give one
event. In voice mode a word the current task state does not accept is dropped in `voice.c` and starts no
refractory time ([`voice-task.md`](voice-task.md) §2). Treat voice as "do this next", never as a timing signal (e.g. do not cut a servo pulse on a
word; finish or stop the move in `motion/`).

---

## 10. Code layout and team rules

```text
2026-micro-con/
├── .github/
│   ├── workflows/firmware-ci.yml   CI (§15)
│   └── pull_request_template.md
├── AGENTS.md                       short rules for AI assistants (read first)
├── ProjectDevGuide/                everything for developing the firmware that is NOT firmware
│   ├── docs/                       core-config.md (this file), TEAM_GUIDE.md, integration/, module_template/
│   ├── tests/host/                 host behaviour tests (gcc on a PC)
│   └── ci/                         build_firmware.py, check_rules.py (used by CI)
└── RoboticArmMain/                 ONLY the CubeIDE project (this folder is what gets built and submitted)
    ├── RoboticArmMain.ioc          only the .ioc owner edits this
    ├── Core/ Drivers/ Middlewares/ X-CUBE-AI/   generated; main.c only calls App/
    ├── STM32F767ZITX_FLASH.ld      with the .dma_buffer edit (§7)
    ├── Models/kws.onnx
    ├── App/
    │   ├── app.c/.h                app_init(), app_loop(), app_control_tick()
    │   ├── app_callbacks.c         THE only place that defines HAL_*Callback functions
    │   ├── common/                 app_config.h (feature flags), events, faults, log, timing, dma_buf
    │   ├── safety/                 fatal-fault gate for motion (always on)
    │   ├── motion/                 servo PWM, calibration, trajectory (TIM6 tick)
    │   ├── kinematics/             IK / FK, arm_geometry.h
    │   ├── input/                  joystick (ADC), encoder (TIM1), encoder / joystick buttons (EXTI)
    │   ├── voice/                  kws_features, kws_stream, KISS FFT, kws_classes.h
    │   ├── pi_link/                USART6 protocol
    │   ├── lcd/                    status screen + touch, lcd_port.c → Lab08 ILI9341 driver
    │   └── modes/                  mode state machine, one file per mode
    └── .gitignore
```

1. `main.c` contains only `#include "app.h"`, `app_init()` in USER CODE 2 and `app_loop()` in USER CODE 3
   (USER CODE 3 also holds the closing `}` of `while(1)`).
2. Each HAL callback is defined once, in `App/app_callbacks.c`, and routes by instance
   (`if (htim->Instance == TIM6) app_control_tick();`). Two definitions = link error.
3. `__io_putchar` (printf → USART3) is defined once, in `App/common/app_log.c`.
4. `X-CUBE-AI/App/app_x-cube-ai.c` USER CODE holds only the 4 `kws_model_*` accessors and `ai_boostrap`;
   **USER CODE 6 stays empty** (the template's default body is a loop that never returns).
5. Need a pin or peripheral change? Ask the `.ioc` owner; never regenerate on your own branch.
6. Feature flags in `App/common/app_config.h` (`APP_USE_VOICE`, `APP_USE_LCD`, `APP_USE_PI`…); personal
   overrides in the gitignored `App/common/app_config_local.h`.
7. `.gitignore` (in `RoboticArmMain/`): `Debug/`, `Release/`, `.settings/`, `*.launch`, `.metadata/`,
   `MXTmpFiles/`, `.ai/`, `App/common/app_config_local.h`.
8. Every change reaches `master` through a PR with CI green (§15).

---

## 11. Hardware notes

- KY-023 and KY-040 are powered from **3.3 V** (not 5 V), so their outputs are safe for the STM32.
- KY-040 CLK/DT have 10 kΩ pull-ups on the module; its SW pin uses the STM32 internal pull-up.
- Servo signals go through the SN74AHCT244N (5.1 V logic rail); servo current never goes
  through the Nucleo or the buffer.
- **Current wiring (24 Oct 2026): no buffer.** Each STM32 PWM pin → **330 Ω** in series → servo
  signal wire, with one common GND (servo buck, Nucleo). All six servos move this way (bench test,
  §4.1). The servo reads the pulse **width**, so the 3.3 V level and the unloaded calibration from
  the 5 V Uno bench both still apply. The 330 Ω limits the current into the STM32 pin if the servo
  side ever drives the line; a 10 kΩ pull-down per signal is optional (it only stops twitching
  while the STM32 is in reset).
- **Do not use the SN74HC244N (no "T") with a diode on VCC.** That stage was built and failed: no
  or erratic output. An HC input needs VIH ≥ 3.15 V at VCC 4.5 V (TI SCLS130D), too close to a
  3.3 V pin, which is why a diode was added to lower VCC. Measured on the bench: STM32 pin correct
  (PC6 0.23 V DC average for a 1460 µs pulse; 3.3 V × 1460 / 20000 = 0.24 V expected), but /OE
  pins 1 and 19 floating (0.8 V and 1.1 V, not tied to GND), pin 20 (VCC) at a few mV in one
  check, output at −0.7 V. The chip may be damaged; do not reuse it. The **HCT/AHCT** versions
  have TTL inputs (VIH 2.0 V) and need no diode. When the SN74HCT244N arrives: pin 20 straight to 5 V, pin 10 GND, pins 1 and
  19 to GND, unused inputs (e.g. 15, 17) to GND, then check one channel stage by stage
  ([`servo-calibration.md`](servo-calibration.md), "Signal check").
- Power order: Nucleo on **before** the servo rail; servo rail off **before** the Nucleo.
- **Final robot with no laptop: power the Nucleo from the Pi's USB port into the ST-LINK USB connector.**
  Not through E5V: the 8 MHz HSE clock comes from the ST-LINK's MCO, and without the ST-LINK running
  there is no clock (the board stops in `Error_Handler` after the 5 s HSE timeout). The USB link also lets
  the Pi reflash the board.
- **No E-stop (v1.3).** Nothing cuts servo power in hardware. Stops are: letting go of the joystick (a jog
  not refreshed for 150 ms stops by itself) and switching the servo supply off. Keep a hand on the servo supply switch while the arm moves.
- First run of new motion code: **servo signal wires disconnected**.

---

## 12. Open items

Decided 20 Oct 2026: LCD = the 2.8″ ILI9341 board from Lab08 (with touch); KY-023 press = Fine/Fast.
27 Oct 2026 (v1.4): no limit switches and no servo-rail sense; joint limits are enforced in software.

| # | Item | Needed for |
|---|---|---|
| 1 | Arm geometry for IK (joint order, link lengths, zero pose) | `kinematics/` only — not the `.ioc` |
| 2 | Pi message format details; Pi sends object XYZ in the **robot base frame**? | `pi_link/` only — not the `.ioc` |
| 3 | Installed servo calibration: safe min/max, joint zero and home (unit-to-joint mapping confirmed J1-J6 = units 1-6) | `motion/` only — not the `.ioc` |
| 4 | Optional PG9 pull-up (removes the boot-time `fault: code 7` without a Pi) | `.ioc`, owner's choice |
| 5 | **Manual modes without Hold-to-Enable.** Decided 27 Oct 2026 (Supanut): no Hold-to-Enable; manual modes move whenever safety allows. Single Joint / Joint Group keep the 150 ms jog timeout; Cartesian moves only while the stick is pushed, and an encoder turn / knob press moves Z / the gripper at once | **done 27 Oct 2026** on `dev` (`motion/`, `modes/`, `safety/`) |
| 6 | Remove the E-stop and MODE code paths (`EVT_ESTOP_*`, `EVT_BTN_MODE`, `SAFETY_ESTOP`) | **done 27 Oct 2026** on `dev`; the event numbers stay, marked unused |
| 7 | Drop PE7 / PE10 / PE12 from the `.ioc` | **done 27 Oct 2026** (v1.4 regenerate) |
| 8 | Drop the limit switches (PE8 / PB11 / PE0 / PE2) and the servo-rail sense (PC3, ADC1 rank 3) from the `.ioc` and the code (`safety/`, `input/`, `app_config.h`, restart latch). Decided 27 Oct 2026 (Supanut) | **done 27 Oct 2026** (v1.4 regenerate); `EVT_RAIL_*` / `EVT_LIMIT_HIT` numbers stay, marked unused |

---

## 13. CubeMX build checklist (for the `.ioc` owner)

How `RoboticArmMain.ioc` was built (standalone CubeMX 6.18.0), and how to rebuild it if ever needed.
Work top to bottom; each step refers to the section with the values.

**Pin traps:** when a peripheral mode is set from the left-hand peripheral list, CubeMX assigns its
**default** pins, which collide with our map: USART6 → PC6/PC7 (servos), USART3 → PB10/PB11 (ours is PD8/PD9),
TIM1 → PA8/PA9 (USB), TIM4 → PB6–PB9. Always **click the pin in the Pinout view first** and pick the
function there, then set the mode. Afterwards check every pin against §3.

1. **New project:** MCU/MPU Selector → `STM32F767ZITx` (not the Board Selector: it pre-assigns Ethernet/USB
   pins we don't want). Answer **No** to "initialize all peripherals with their default mode".
   Save as `RoboticArmMain` inside `2026-micro-con\`.
2. **System Core → RCC:** HSE = BYPASS Clock Source; HSE startup timeout 5000 (§2). **SYS:** Debug = Serial Wire, timebase SysTick.
3. **Multimedia → SAI1:** set Block A to **Master** first (§4.4) — the PLLSAI /Q divider is only editable after that.
4. **Clock Configuration tab:** §2. Check SYSCLK 216 MHz, APB1 54, APB2 108, SAI1 clock 8.192 MHz.
5. **Pins** (§3, mind the pin traps): click each pin, pick its function, right-click → *Enter User Label*.
   Labels: `LCD_CS`, `LCD_DC`, `LCD_RST`, `T_CLK`, `T_CS`, `T_MOSI`, `T_MISO`, `T_IRQ`, `ENC_SW`, `JOY_SW`,
   `LD1`, `LD2`, `LD3`, `USER_BTN`.
   TIM3 may show an ETR pin conflict warning: ignore it, ETR is not used.
6. **System Core → GPIO:** outputs CS / T_CS / RST start **High**, speed Very High for LCD + touch outputs;
   `ENC_SW`, `JOY_SW`: **pull-up**, EXTI **falling**. SAI pin PE6: **pull-down**.
7. **Timers:** TIM3, TIM4 (§4.1), TIM1 (§4.2), TIM2 (§4.3), TIM6 (§4.7 — period **199**).
8. **Analog → ADC1:** §4.3 (add the DMA request in its DMA Settings tab).
9. **SAI1 DMA:** §4.4.
10. **Connectivity → SPI5** (§4.5), **USART6** and **USART3** (§4.6, + DMA requests).
11. **Computing → CRC:** Activated (§4.8).
12. **DMA:** compare all streams with §5.
13. **NVIC:** priority group 4 bits; priorities from §6 (check DMA2 Stream 7 = **3**); tick TIM6 and EXTI15_10.
14. **CORTEX_M7:** §7 (caches, MPU region 1; leave region 0 as CubeMX made it).
15. **X-CUBE-AI:** Software Packs → Select Components → X-CUBE-AI 10.2.1 (Core + Application
    Template) → add network `network` from `Models/kws.onnx` (copy the file into the project folder
    first) → Analyze. Then re-check USART3 is still generated (§4.8).
16. **Project Manager:** stack/heap §7, code generator options §8.
17. **Generate code.** Edit the linker script once (§7). Paste the USER CODE (§10 rules 1 and 4;
    templates in `ProjectDevGuide/docs/integration/`).
18. Search the `.ioc` for `D:\` or `C:\` in model paths — an absolute model path breaks teammates' builds.

**After any later regenerate:** the `.ld` edit is still there, `App` is still a source folder and include path
(§14), the USER CODE blocks are intact — then build, run on the board, PR. CI checks most of this (§15).

---

## 14. CubeIDE project (2.2.0)

- **Workspace outside the repo** (e.g. `D:\Microcon69\ProjectRoboticArm\ide_workspace`), so `.metadata/`
  never lands in git.
- **Import:** File → Import → General → Existing Projects into Workspace → root
  `2026-micro-con\RoboticArmMain` → **untick "Copy projects into workspace"** → Finish.
  Teammates do only this; they never open the `.ioc` or generate.
- **`App/` wiring (stored in `.cproject`, already committed):** Properties → C/C++ General → Paths and Symbols:
  - Source Location: `App` is a source folder in **both Debug and Release** (pick each in the Configuration list).
  - Includes → GNU C: the entry is typed as **`App`** — this dialog adds `../` itself (it shows `Core/Inc` for
    `../Core/Inc`), so it is stored as `../App` and the compiler gets `-I../App`. Typing `../App` there stores
    `../../App` and every `App/` include fails with "No such file or directory".
- **Build:** select RoboticArmMain in Project Explorer → hammer (Ctrl+B may build another project in the
  workspace). Expected: 0 errors;
  warnings only `"DEBUG" redefined` from the KISS FFT files (harmless).
- **Run vs Debug:** **Run As → STM32 C/C++ Application** flashes and starts the program. **Debug As** flashes and
  **stops at `main()`** until Resume (F8) — nothing runs, no LED, no log, until you press it. After either, the
  program stays in flash and runs alone after RESET or power-up.
- **Boot log:** USART3 via the ST-LINK COM port, 921600 8N1. Expected log and fault meanings:
  `ProjectDevGuide/docs/integration/CUBEIDE_SETUP.md`. LD1 blinks twice per second = main loop alive; LD3 on = fatal fault.

---

## 15. CI (`firmware-ci`)

`.github/workflows/firmware-ci.yml` runs on every PR into `master` and every push to `master` (about a minute).
Checks:

| Check | What it proves |
|---|---|
| `build (Debug)`, `build (Release)` | `ProjectDevGuide/ci/build_firmware.py` compiles and links RoboticArmMain with the source folders, defines, include paths, libraries and linker script **read from `.cproject`** (Ubuntu `arm-none-eabi-gcc`). Fails on any error, flash/RAM overflow, `.dma_buffer` not at 0x2007C000 or over 16 KB, and any warning in `App/` (third-party code exempt: KISS FFT, `App/lcd/driver/`) |
| `host-tests` | `ProjectDevGuide/tests/host/run.sh`: safety / motion / modes / voice-mode / pi_link / screen-logic behaviour on a PC (74 checks + the voice-mode and voice-hand dry runs). The real `lcd_port.c` (ILI9341 driver, HAL) is replaced by `ProjectDevGuide/tests/host/fake/lcd_port_fake.c` |
| `repo-rules` | `ProjectDevGuide/ci/check_rules.py`: `RoboticArmMain/` holds only the project (no build output, IDE state, `app_config_local.h`, `.md` notes, `docs/` or `tests/`); no file over 3 MB; `.cproject` source folders, `../App`, AI library and linker script in both configurations; `main.c` and `app_x-cube-ai.c` USER CODE as in §10; USER CODE markers paired; callbacks and `__io_putchar` in one place; the `.ioc` model file committed |

Run the same checks locally from the repo root before pushing:

```text
python ProjectDevGuide/ci/build_firmware.py       (needs arm-none-eabi-gcc on PATH)
sh ProjectDevGuide/tests/host/run.sh              (needs gcc; on Windows use Git Bash)
python ProjectDevGuide/ci/check_rules.py
```

CI's compiler is not CubeIDE's, so its size numbers differ a little (Debug text 328,444 B in CI vs 324,300 B in
CubeIDE); only pass/fail matters.

**Hang protection (PR #3):** every job has a time limit (builds 15 min, the others 10 min). The compiler install
uses 30 s network timeouts and up to 3 attempts, because Ubuntu package mirrors sometimes stall (this hung two
runs on 23 Oct). If the install still fails, the log says `could not install arm-none-eabi-gcc`: re-run the job.

A ruleset on `master` that requires these 4 checks (set by the repo owner) turns CI from a warning into a gate.
Delete merged PR branches on GitHub ("Delete branch" on the merged PR) and locally (`git branch -d`).
