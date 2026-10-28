# ProjectDevGuide — everything for developing the firmware that is not firmware

`../RoboticArmMain/` holds only the STM32CubeIDE project (what gets built and flashed).
This folder holds the material around it:

| Folder | What is in it | Who uses it |
|---|---|---|
| `docs/TEAM_GUIDE.md` | who owns which folder, the 10 rules, how modules talk, feature flags, PR checklist | every team member — read once |
| `docs/core-config.md` | pins, clocks, timers, DMA, interrupts, memory, CubeMX checklist, CubeIDE setup, CI | anyone touching hardware or the `.ioc` |
| `docs/servo-calibration.md` | canonical unloaded servo measurements, confirmed unit-to-joint/pin map, STM32 bench test, signal check, installed-calibration checklist | anyone calibrating or touching motion |
| `docs/station-detection.md` | how the Pi finds the station and its drop spot (depth only), tests, rules for changing the detector | whoever works on the Pi vision or the station target |
| `docs/camera-calibration.md` | camera -> robot calibration (`calib_T.npy`): measuring points with the arm, solving, checking, troubleshooting, 28 Oct result (J1 issue) | whoever calibrates the camera or debugs a missed grab |
| `docs/voice-task.md` | voice task states S0–S11, which words each state accepts, dry-run bench test with the Pi | whoever works on voice mode, IK or the Pi link |
| `docs/integration/` | how to open, build and run the project in CubeIDE; templates of our USER CODE in generated files | everyone (section 1), the `.ioc` owner (the rest) |
| `docs/module_template/` | starting files for a new feature module | whoever adds a module |
| `tests/host/` | behaviour tests of safety / motion / modes / voice task / Pi link / screen logic, built with gcc on a PC | CI and anyone changing those modules |
| `ci/` | `build_firmware.py` (builds like CubeIDE from `.cproject`), `check_rules.py` (project rules) | CI (`.github/workflows/firmware-ci.yml`) |

Run all checks from the repo root:

```text
python ProjectDevGuide/ci/build_firmware.py
sh ProjectDevGuide/tests/host/run.sh
python ProjectDevGuide/ci/check_rules.py
```

AI assistants: start with `../AGENTS.md`.
