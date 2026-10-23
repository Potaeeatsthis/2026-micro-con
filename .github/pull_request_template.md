## What this PR does
<!-- one or two sentences. which module? (motion / input / kinematics / lcd / pi_link / modes / voice / core) -->

## How I tested it
- [ ] Built in STM32CubeIDE: 0 errors, no new warnings in `App/`
- [ ] Flashed and ran on the board (paste the relevant part of the boot log / serial output below)
- [ ] Host tests pass: `sh ProjectDevGuide/tests/host/run.sh` (if I changed safety / motion / modes / pi_link)
- [ ] Servo signal wires were disconnected for the first run of new motion code

```text
(serial output here)
```

## Rules check
- [ ] I only changed files in my own module (`App/<module>/`), or the core owner agreed
- [ ] `RoboticArmMain.ioc` is unchanged, or this is a core PR that says what was changed in CubeMX and why
- [ ] Feature flag defaults in `App/common/app_config.h` are unchanged (my local overrides are in `app_config_local.h`, which is not committed)
- [ ] Generated files (`Core/`, `X-CUBE-AI/`) are changed only between `USER CODE BEGIN` / `END`
- [ ] No `Debug/`, `.settings/`, `*.launch` or other local files in the diff

CI (`firmware-ci`) must be green before merging.
