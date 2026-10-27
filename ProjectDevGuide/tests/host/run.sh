#!/bin/sh
# host behaviour tests for safety / motion / modes / mode_voice / pi_link / events,
# plus test_pi_proto.c for the pi line parser (pi_link/pi_proto.c, pure code),
# plus test_app.c again as a voice dry run (voice "hand" and "station" end to end; ik is not measured yet).
# plus test_servo_cal.c for the bench limit screen (App/servo_cal_test/).
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
  "$A"/modes/*.c "$A/lcd/lcd.c" "$HERE/fake/lcd_port_fake.c" "$A/pi_link/pi_proto.c" -lm
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
  "$A"/modes/*.c "$A/lcd/lcd.c" "$HERE/fake/lcd_port_fake.c" "$A/pi_link/pi_proto.c" -lm
# the bench limit screen (App/servo_cal_test/) with its own flags: motion on, modes and lcd.c off
gcc -std=gnu11 -O1 -g -Wall -Wextra -Wshadow -Wno-unused-parameter \
  -DAPP_USE_SERVO_CAL_TEST=1 -DAPP_USE_MOTION=1 -DAPP_USE_MODES=0 -DAPP_USE_LCD=0 -DAPP_USE_INPUT=1 \
  -I"$HERE/fake" -I"$A" -o "$T/test_servo_cal" "$HERE/test_servo_cal.c" "$A/servo_cal_test/servo_cal_test.c" \
  "$A/common/app_events.c" "$A/common/app_fault.c" "$A/safety/safety.c" \
  "$A/motion/motion.c" "$A/motion/servo.c" "$A/motion/servo_cal.c" "$HERE/fake/lcd_port_fake.c"
# kinematics: the default build has FK on and IK fail-closed; the second build enables IK for the test only.
gcc -std=gnu11 -O1 -g -Wall -Wextra -Werror -Wshadow -Wno-unused-parameter \
  -I"$A" -o "$T/test_kinematics_failclosed" "$HERE/test_kinematics.c" \
  "$A/kinematics/kinematics.c" "$A/motion/servo_cal.c" -lm
gcc -std=gnu11 -O1 -g -Wall -Wextra -Werror -Wshadow -Wno-unused-parameter \
  -DARM_GEOMETRY_MEASURED=1 -I"$A" -o "$T/test_kinematics" "$HERE/test_kinematics.c" \
  "$A/kinematics/kinematics.c" "$A/motion/servo_cal.c" -lm
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
rc4=0
"$T/test_servo_cal" > "$T/test_servo_cal.log" || rc4=$?
grep -E "^==|FAIL|passed" "$T/test_servo_cal.log"
rc5=0
"$T/test_kinematics_failclosed" > "$T/test_kinematics_failclosed.log" || rc5=$?
grep -E "^==|FAIL|passed" "$T/test_kinematics_failclosed.log"
rc6=0
"$T/test_kinematics" > "$T/test_kinematics.log" || rc6=$?
grep -E "^==|FAIL|passed" "$T/test_kinematics.log"
[ $rc -ne 0 ] && exit $rc
[ $rc2 -ne 0 ] && exit $rc2
[ $rc3 -ne 0 ] && exit $rc3
[ $rc4 -ne 0 ] && exit $rc4
[ $rc5 -ne 0 ] && exit $rc5
exit $rc6
