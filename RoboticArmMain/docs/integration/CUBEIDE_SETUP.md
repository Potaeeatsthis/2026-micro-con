# Adding `App/` to the CubeIDE project (core owner, once)

Done once by Supanut after the first `Generate Code`. Everything here is saved in `.cproject`,
which is committed, so teammates get it automatically on import.

## 1. Copy the files in

```text
RoboticArmMain/
├── App/                     ← the whole App/ folder from this package
├── docs/                    ← TEAM_GUIDE.md, core-config.md, robotic-arm-guide.md, integration/, module_template/
├── .gitignore               ← from this package
└── (generated: Core/ Drivers/ Middlewares/ X-CUBE-AI/ RoboticArmMain.ioc ...)
```

Voice files (the package already has `App/voice/kws_classes.h` from the exp04 release):
- **before the X-CUBE-AI step:** `voice_command_research/final_model/exp04_tcresnet-w16d3/onnx_out/kws.onnx`
  → `RoboticArmMain/Models/kws.onnx`. The firmware keeps its own copy so a new research release
  cannot change the firmware until you deliberately copy it and regenerate.
- **after generating:** from the live project (`RA\realtime_detect\model_realtime_code`) into `App/voice/`:
  `kws_features.c/.h`, `kws_stream.c/.h`, `kws_feature_tables.h`, `kiss_fft.c/.h`, `_kiss_fft_guts.h`
  (unchanged since the 22-class model). Their own `#include "kws_features.h"` lines work unchanged.
- **a new model later:** copy its `kws.onnx` and `kws_c/kws_classes.h` together, regenerate, rebuild.
  `voice_init()` refuses to start if the network outputs and `N_CLASSES` disagree.

## 2. Make `App/` a source folder

Project → Properties → C/C++ General → **Paths and Symbols**:

1. **Source Location** tab → *Add Folder…* → `App` → OK.
2. **Includes** tab → language **GNU C** → *Add…* → type **`App`** (not `../App`)
   - tick **Add to all configurations**; leave "Is a workspace path" unticked
   - this dialog adds `../` itself (it shows `Core/Inc` for `../Core/Inc`), so `App` is stored
     in `.cproject` as `../App` and the compile line gets `-I../App`. Typing `../App` here gives
     `-I../../App` and every `App/` include fails with "No such file or directory".
   - this is the **only** include path App needs: every include is written from the App root,
     e.g. `#include "motion/motion.h"`.
3. Apply and Close → Project → Clean → Build.

After every later regenerate, check once that `App` is still a source folder and `App` (stored as `../App`) is
still an include path. CubeMX normally keeps user-added paths; if one is gone, re-add it. CI fails the
build if it is missing, so it cannot slip into `master` unnoticed.

## 3. Paste the user code

- `docs/integration/main_USER_CODE.c` → `Core/Src/main.c` (3 lines)
- `docs/integration/app_x-cube-ai_USER_CODE.c` → `X-CUBE-AI/App/app_x-cube-ai.c` (accessors, `ai_boostrap`, **empty** USER CODE 6)

## 4. One-time checks on the board (USART3, 921600 baud)

Expected boot log:

```text
=== RoboticArmMain 0.1.0 ===
flags: motion 1 input 1 ik 1 voice 1 pi 1 lcd 1 modes 1 | limits 0 rail 0
safety: init, estop released, rail sense off (APP_USE_RAIL_SENSE 0), limit switches off (APP_USE_LIMIT_SWITCHES 0)
motion: init, home pose loaded, pulses start when safety allows
input: init
voice: listening, 23 classes, threshold 75%, min snr 12 dB, gain 15 dB
pi: link on USART6, timeout 3000 ms
lcd: init
modes: -> VOICE
app: running
input: joystick centre 2041 / 2052
```

- `ERROR dma_buf: ...` → the linker edit or the MPU region is missing (core-config §7). Motion is blocked until fixed.
- `__io_putchar` defined twice → delete the copy outside `App/common/app_log.c`.
- `HAL_..._Callback` defined twice → delete the copy outside `App/app_callbacks.c`.
- LD1 must blink twice per second (main loop alive).
