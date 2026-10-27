# RoboticArmMain in STM32CubeIDE

The CubeIDE project settings are stored in `.cproject`, which is committed, so a clone is ready to build.
Section 1 is all a teammate needs. Sections 2–3 describe how `App/` is wired in; repeat them only if a
regenerate ever loses a setting (CI fails the build if one is missing).

## 1. Open, build, run (everyone)

1. Use a CubeIDE (2.2.0) workspace **outside** the repo, e.g. `D:\...\ide_workspace`.
2. File → Import → General → **Existing Projects into Workspace** → root directory
   `2026-micro-con\RoboticArmMain` → **untick "Copy projects into workspace"** → Finish.
3. Do **not** open `RoboticArmMain.ioc` or generate code (only the `.ioc` owner does that).
4. Select **RoboticArmMain** in Project Explorer → hammer. Expected: 0 errors; the only warnings are
   `"DEBUG" redefined` from the KISS FFT files (harmless).
5. **Run As → STM32 C/C++ Application** flashes and starts the program.
   **Debug As** flashes and **stops at `main()`**: nothing runs (no LED, no log) until you press Resume (F8).
   Either way the program stays in flash and runs alone after RESET or power-up.
6. `git status` must stay clean after building (build output is gitignored).

## 2. How `App/` is wired in (core owner; already done)

Project → Properties → C/C++ General → **Paths and Symbols**:

1. **Source Location:** `App` is a source folder in **both** configurations. Pick Debug, then Release, in the
   Configuration list; for each: *Add Folder…* → `App` → OK.
2. **Includes** → language **GNU C**, Configuration **[All configurations]**: the entry is typed as **`App`**
   (not `../App`), "Is a workspace path" unticked.
   - this dialog adds `../` itself (it shows `Core/Inc` for `../Core/Inc`), so `App` is stored in `.cproject`
     as `../App` and the compile line gets `-I../App`. Typing `../App` there gives `-I../../App` and every
     `App/` include fails with "No such file or directory".
   - this is the **only** include path App needs: every include is written from the App root,
     e.g. `#include "motion/motion.h"`.
3. Apply and Close → Project → Clean → Build.

## 3. Generated files that hold our code (core owner)

- `Core/Src/main.c`: `#include "app.h"`, `app_init()` in USER CODE 2, `app_loop()` in USER CODE 3
  (template: `main_USER_CODE.c` next to this file; USER CODE 3 also holds the `}` of `while(1)`).
- `X-CUBE-AI/App/app_x-cube-ai.c`: the 4 `kws_model_*` accessors, `ai_boostrap` in USER CODE 5, **USER CODE 6
  empty** (template: `app_x-cube-ai_USER_CODE.c` next to this file).
- `STM32F767ZITX_FLASH.ld`: `RAM` 496K + `RAM_DMA` 16K at 0x2007C000 + the `.dma_buffer` section.
- The model is `Models/kws.onnx` (exp04). A new model = copy its `kws.onnx` **and** `kws_classes.h`
  (into `App/voice/`) together, regenerate, rebuild. `voice_init()` refuses to start if the network outputs and
  `N_CLASSES` disagree.

After every regenerate, check these three files and section 2, build, run on the board, then open a PR.

## 4. Boot log (USART3 = ST-LINK COM port, 921600 8N1)

```text
=== RoboticArmMain 0.1.0 ===
flags: motion 1 input 1 ik 1 voice 1 pi 1 lcd 1 modes 1 | limits 0 rail 0
safety: init, rail sense off (APP_USE_RAIL_SENSE 0), limit switches off (APP_USE_LIMIT_SWITCHES 0)
motion: init, home pose loaded, pulses start when safety allows
input: init
voice: listening, 23 classes, threshold 75%, min snr 12 dB, gain 15 dB
pi: link on USART6, timeout 3000 ms
lcd: init
modes: -> VOICE
app: running
fault: code 7 (1 total)
input: joystick centre 2048 / 2048
```

- `fault: code 7` without a Pi connected: USART6 RX (PG9) floats and the UART sees noise once at boot. Harmless.
- `ERROR dma_buf: ...` → the linker edit or the MPU region is missing. Motion is blocked until fixed.
- `__io_putchar` defined twice → delete the copy outside `App/common/app_log.c`.
- `HAL_..._Callback` defined twice → delete the copy outside `App/app_callbacks.c`.
- LD1 blinks twice per second = main loop alive; LD3 on = fatal fault.
- A spoken command prints `>>> word NN% (chX, snr NN dB)`.
