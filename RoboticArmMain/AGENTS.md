# AGENTS.md — rules for AI assistants working on RoboticArmMain

Read this first, then `docs/TEAM_GUIDE.md` (how the code is split) and `docs/core-config.md` (pins,
clocks, timers, DMA, interrupts). If anything here conflicts with a request, say so instead of guessing.

## The project in five lines
- STM32F767ZI (NUCLEO-F767ZI), 216 MHz, bare-metal (no RTOS), STM32CubeIDE 2.2.0 + CubeMX 6.18 + X-CUBE-AI 10.2.1.
- 6-DOF robotic arm: servos (TIM3/TIM4 PWM), joystick + encoder + buttons, voice commands (2 mics, CNN on the
  STM32), Raspberry Pi camera link (USART6), ILI9341 touch LCD (SPI5).
- `main.c` only calls `app_init()` and `app_loop()`. All our code is in `App/`, one folder per feature.
- Interrupts do the timing-critical work (TIM6 50 Hz control tick, audio DMA); everything slow runs in the main loop.
- Modules talk through an event queue (`common/app_events.h`), getter functions and the motion API (`motion/motion.h`).

## Where you may write
- **Only in the feature folder you were asked to work on**, e.g. `App/lcd/`, `App/motion/`, `App/input/`.
- Never edit: `RoboticArmMain.ioc`, `Core/`, `Drivers/`, `Middlewares/`, `X-CUBE-AI/`, `*.ld`, `.cproject`,
  `.project`, `App/app.c`, `App/app_callbacks.c`, `App/common/`, `App/safety/`, `.github/`, `tools/`.
  These belong to the core owner. If the task needs a change there (a pin, a peripheral, a new callback, a new
  event type), stop and write down exactly what is needed so the owner can do it.
- Never regenerate code with CubeMX and never "fix" generated files.
- Never edit `App/common/app_config.h` to test. Per-developer overrides go in `App/common/app_config_local.h`
  (gitignored), e.g. `#define APP_USE_VOICE 0`.

## Rules the code must follow
1. **HAL callbacks** (`HAL_*Callback`) exist only in `App/app_callbacks.c`. A module offers a function such as
   `lcd_on_xxx()` and the core owner routes to it. A second definition anywhere = link error / CI failure.
2. **Interrupt context** (callbacks, `*_tick()`): short, no `HAL_Delay`, no `printf`/`LOG`, no LCD drawing,
   no loops over big arrays. Copy data, set a flag, return.
3. **Main loop**: each `*_step()` returns within a few ms; the whole pass must stay under ~100 ms or audio is lost.
   Split long work over several passes. The LCD redraws only what changed.
4. **Servos**: only `App/motion/` writes PWM. Everything else sets targets: `motion_set_target()`,
   `motion_set_joint_target()`, `motion_jog()`, `motion_hold()`.
5. **Safety** is always on and never bypassed. Motion respects `safety_motion_allowed()` (inside motion).
6. **DMA buffers** use `DMA_BUF` from `common/dma_buf.h` (non-cacheable SRAM2, 16 KB total) and get a row in
   `docs/TEAM_GUIDE.md` section 6.
7. **Module state is `static`**; other modules use the functions in your header. Data written in an ISR and read
   in the main loop is `volatile`.
8. **Includes are written from the `App/` root**: `#include "motion/motion.h"`, `#include "common/app_log.h"`.
9. Logging: `LOG(...)` / `LOG_ERR(...)` from `common/app_log.h` (USART3, 921600). No `%f` (newlib-nano):
   print scaled integers.
10. **0 warnings** in `App/` (CI fails on any). Third-party code is exempt only in `App/voice/kiss_fft*` and
    `App/lcd/driver/`.
11. Keep the header comment of every file you touch accurate (owner, what it does, what it does not do).
12. A new module: copy `docs/module_template/`, follow `docs/TEAM_GUIDE.md` section 7.

## Module-specific notes
- **LCD (`App/lcd/`)**: the Lab08 ILI9341 driver goes in `App/lcd/driver/` (see `App/lcd/driver/README.md`),
  only `lcd_port.c` calls it, `lcd.c` holds the screen logic and **pulls** state from getters
  (`modes_describe()`, safety, voice, pi_link). Touch input becomes events. Pins: `docs/core-config.md` §3, §4.5.
  Host tests replace `lcd_port.c` with `tests/host/fake/lcd_port_fake.c`, so keep `lcd.c` free of HAL calls
  except through `lcd_port.h`.
- **Modes (`App/modes/mode_*.c`)**: implement the `mode_ops_t` functions (`enter`, `exit`, `on_event`, `step`,
  `describe`) from `modes/mode_ops.h`. Only `modes_step()` consumes events.
- **Kinematics**: pure math, no HAL, testable on a PC; geometry in `kinematics/arm_geometry.h`.

## Before you say "done"
Run from the repo root (all three are what CI runs on every PR):
```text
python tools/ci/build_firmware.py        # firmware builds, no warnings in App/
sh RoboticArmMain/tests/host/run.sh      # host behaviour tests
python tools/ci/check_rules.py           # project rules
```
Then the human builds in CubeIDE, runs it on the board (servo signal wires disconnected for new motion code),
and opens a PR into `master` using the PR template. Never commit `Debug/`, `.settings/`, `*.launch` or
`app_config_local.h`.
