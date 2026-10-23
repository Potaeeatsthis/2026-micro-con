#!/bin/sh
# host behaviour tests for safety / motion / modes / mode_voice / pi_link / events.
# the screen logic (lcd.c) is tested; the real lcd_port.c talks to the ILI9341 driver (HAL), so a fake
# in fake/lcd_port_fake.c stands in for it here.
# needs gcc on a pc (linux, mac, or git-bash/msys2 on windows). run from anywhere:
#   sh RoboticArmMain/tests/host/run.sh
set -e
HERE=$(cd "$(dirname "$0")" && pwd)
A="$HERE/../../App"
gcc -std=gnu11 -O1 -g -Wall -Wextra -Wshadow -Wno-unused-parameter \
  -DAPP_USE_INPUT=0 -DAPP_USE_VOICE=1 -DAPP_USE_PI=1 -DAPP_USE_LCD=1 -DAPP_USE_MODES=1 \
  -I"$HERE/fake" -I"$A" -o "$HERE/test_app" "$HERE/test_app.c" \
  "$A/common/app_events.c" "$A/common/app_fault.c" "$A/safety/safety.c" \
  "$A/motion/motion.c" "$A/motion/servo.c" "$A/motion/servo_cal.c" "$A/kinematics/kinematics.c" \
  "$A"/modes/*.c "$A/lcd/lcd.c" "$HERE/fake/lcd_port_fake.c"
# keep the test's own exit code (a pipe into grep would hide a failure)
rc=0
"$HERE/test_app" > "$HERE/test_app.log" || rc=$?
grep -E "^==|FAIL|passed" "$HERE/test_app.log"
exit $rc
