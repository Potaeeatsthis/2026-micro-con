#!/bin/sh
# host behaviour tests for safety / motion / modes / mode_voice / pi_link / events,
# plus test_pi_proto.c for the pi line parser (pi_link/pi_proto.c, pure code),
# plus test_app.c again as a voice dry run (voice "hand" end to end; ik is not measured yet).
# the screen logic (lcd.c) is tested; the real lcd_port.c talks to the ILI9341 driver (HAL), so a fake
# in fake/lcd_port_fake.c stands in for it here.
# needs gcc on a pc (linux, mac, or git-bash/msys2 on windows). run from anywhere:
#   sh ProjectDevGuide/tests/host/run.sh
set -e
HERE=$(cd "$(dirname "$0")" && pwd)
A="$HERE/../../../RoboticArmMain/App"
gcc -std=gnu11 -O1 -g -Wall -Wextra -Wshadow -Wno-unused-parameter \
  -DAPP_USE_INPUT=0 -DAPP_USE_VOICE=1 -DAPP_USE_PI=1 -DAPP_USE_LCD=1 -DAPP_USE_MODES=1 \
  -I"$HERE/fake" -I"$A" -o "$HERE/test_app" "$HERE/test_app.c" \
  "$A/common/app_events.c" "$A/common/app_fault.c" "$A/safety/safety.c" \
  "$A/motion/motion.c" "$A/motion/servo.c" "$A/motion/servo_cal.c" "$A/kinematics/kinematics.c" \
  "$A"/modes/*.c "$A/lcd/lcd.c" "$HERE/fake/lcd_port_fake.c" "$A/pi_link/pi_proto.c"
# the parser test binary goes to a temp dir (nothing new to gitignore)
T=$(mktemp -d)
trap 'rm -rf "$T"' EXIT
gcc -std=gnu11 -O1 -g -Wall -Wextra -Wshadow -Wno-unused-parameter \
  -I"$A" -o "$T/test_pi_proto" "$HERE/test_pi_proto.c" "$A/pi_link/pi_proto.c"
# same app test built as a voice dry run: the only way to reach S4 and test "hand" before ik exists
gcc -std=gnu11 -O1 -g -Wall -Wextra -Wshadow -Wno-unused-parameter \
  -DAPP_USE_INPUT=0 -DAPP_USE_VOICE=1 -DAPP_USE_PI=1 -DAPP_USE_LCD=1 -DAPP_USE_MODES=1 -DAPP_VOICE_DRY_RUN=1 \
  -I"$HERE/fake" -I"$A" -o "$T/test_app_dry" "$HERE/test_app.c" \
  "$A/common/app_events.c" "$A/common/app_fault.c" "$A/safety/safety.c" \
  "$A/motion/motion.c" "$A/motion/servo.c" "$A/motion/servo_cal.c" "$A/kinematics/kinematics.c" \
  "$A"/modes/*.c "$A/lcd/lcd.c" "$HERE/fake/lcd_port_fake.c" "$A/pi_link/pi_proto.c"
# keep the test's own exit code (a pipe into grep would hide a failure)
rc=0
"$HERE/test_app" > "$HERE/test_app.log" || rc=$?
grep -E "^==|FAIL|passed" "$HERE/test_app.log"
rc2=0
"$T/test_pi_proto" > "$T/test_pi_proto.log" || rc2=$?
grep -E "^==|FAIL|passed" "$T/test_pi_proto.log"
rc3=0
"$T/test_app_dry" > "$T/test_app_dry.log" || rc3=$?
grep -E "^==|FAIL|passed" "$T/test_app_dry.log"
[ $rc -ne 0 ] && exit $rc
[ $rc2 -ne 0 ] && exit $rc2
exit $rc3
