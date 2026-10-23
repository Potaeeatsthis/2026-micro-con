# ProjectDevGuide — everything for developing the firmware that is not firmware

`../RoboticArmMain/` holds only the STM32CubeIDE project (what gets built and flashed).
This folder holds the material around it:

| Folder | What is in it | Who uses it |
|---|---|---|
| `docs/TEAM_GUIDE.md` | who owns which folder, the 10 rules, how modules talk, feature flags, PR checklist | every team member — read once |
| `docs/core-config.md` | pins, clocks, timers, DMA, interrupts, memory, CubeMX checklist, CubeIDE setup, CI | anyone touching hardware or the `.ioc` |
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
