#!/bin/sh
# host behaviour tests for safety / motion / modes / mode_voice / pi_link / events,
# plus test_pi_proto.c for the pi line parser (pi_link/pi_proto.c, pure code),
# plus test_app.c again as a voice dry run (voice "hand" and "station" end to end; voice IK remains blocked).
# plus test_servo_cal.c for the bench limit screen (App/servo_cal_test/).
# plus test_app.c with IK activated for the test only (APP_TEST_IK): the IK callers in voice / cartesian.
# plus test_ik_bench.c for the one supervised IK move (App/ik_bench_test/), global/voice IK guard still 0
# and the bench build keeps the Cartesian IK API off.
# plus test_kinematics.c three times: firmware defaults (Cartesian on), Cartesian explicitly off, global IK math.
# plus test_cartesian.c with APP_USE_INPUT 1: the joystick XY path, centre, refusal/rebase and no auto gripper.
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
# same app test built as a voice dry run: voice IK remains blocked, so this tests "hand" end to end
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
# same app test with IK activated (test only): the IK callers (voice pitch search, refusals, cartesian)
gcc -std=gnu11 -O1 -g -Wall -Wextra -Wshadow -Wno-unused-parameter \
  -DAPP_USE_INPUT=0 -DAPP_USE_VOICE=1 -DAPP_USE_PI=1 -DAPP_USE_LCD=1 -DAPP_USE_MODES=1 \
  -DARM_GEOMETRY_MEASURED=1 -DAPP_TEST_IK=1 \
  -I"$HERE/fake" -I"$A" -o "$T/test_app_ik" "$HERE/test_app.c" \
  "$A/common/app_events.c" "$A/common/app_fault.c" "$A/safety/safety.c" \
  "$A/motion/motion.c" "$A/motion/servo.c" "$A/motion/servo_cal.c" "$A/kinematics/kinematics.c" \
  "$A"/modes/*.c "$A/lcd/lcd.c" "$HERE/fake/lcd_port_fake.c" "$A/pi_link/pi_proto.c" -lm
# the IK bench screen: motion on, modes and lcd.c off, ARM_GEOMETRY_MEASURED left at 0
gcc -std=gnu11 -O1 -g -Wall -Wextra -Werror -Wshadow -Wno-unused-parameter \
  -DAPP_USE_IK_BENCH_TEST=1 -DAPP_USE_MOTION=1 -DAPP_USE_MODES=0 -DAPP_USE_LCD=0 -DAPP_USE_INPUT=1 \
  -DAPP_USE_KINEMATICS=1 \
  -I"$HERE/fake" -I"$A" -o "$T/test_ik_bench" "$HERE/test_ik_bench.c" "$A/ik_bench_test/ik_bench_test.c" \
  "$A/input/input.c" "$A/common/app_events.c" "$A/common/app_fault.c" "$A/safety/safety.c" \
  "$A/motion/motion.c" "$A/motion/servo.c" "$A/motion/servo_cal.c" "$A/kinematics/kinematics.c" \
  "$HERE/fake/lcd_port_fake.c" -lm
# the input-enabled Cartesian path: real mode + motion + kinematics, fake joystick and lcd message
gcc -std=gnu11 -O1 -g -Wall -Wextra -Werror -Wshadow -Wno-unused-parameter \
  -DAPP_USE_INPUT=1 -I"$HERE/fake" -I"$A" -o "$T/test_cartesian" "$HERE/test_cartesian.c" \
  "$A/modes/mode_cartesian.c" "$A/modes/gripper.c" \
  "$A/common/app_events.c" "$A/common/app_fault.c" "$A/safety/safety.c" \
  "$A/motion/motion.c" "$A/motion/servo.c" "$A/motion/servo_cal.c" "$A/kinematics/kinematics.c" -lm
# kinematics: the default build has FK on, voice IK off and Cartesian IK on; the cart-off build
# disables both normal IK APIs; the global build enables the math for the test only with Cartesian off.
gcc -std=gnu11 -O1 -g -Wall -Wextra -Werror -Wshadow -Wno-unused-parameter \
  -I"$A" -o "$T/test_kinematics_failclosed" "$HERE/test_kinematics.c" \
  "$A/kinematics/kinematics.c" "$A/motion/servo_cal.c" -lm
gcc -std=gnu11 -O1 -g -Wall -Wextra -Werror -Wshadow -Wno-unused-parameter \
  -DARM_CARTESIAN_IK_ENABLED=0 -I"$A" -o "$T/test_kinematics_cart_off" "$HERE/test_kinematics.c" \
  "$A/kinematics/kinematics.c" "$A/motion/servo_cal.c" -lm
gcc -std=gnu11 -O1 -g -Wall -Wextra -Werror -Wshadow -Wno-unused-parameter \
  -DARM_GEOMETRY_MEASURED=1 -DARM_CARTESIAN_IK_ENABLED=0 -I"$A" -o "$T/test_kinematics" \
  "$HERE/test_kinematics.c" "$A/kinematics/kinematics.c" "$A/motion/servo_cal.c" -lm
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
rc8=0
"$T/test_app_ik" > "$T/test_app_ik.log" || rc8=$?
grep -E "^==|FAIL|passed" "$T/test_app_ik.log"
rc4=0
"$T/test_servo_cal" > "$T/test_servo_cal.log" || rc4=$?
grep -E "^==|FAIL|passed" "$T/test_servo_cal.log"
rc7=0
"$T/test_ik_bench" > "$T/test_ik_bench.log" || rc7=$?
grep -E "^==|FAIL|passed" "$T/test_ik_bench.log"
rc10=0
"$T/test_cartesian" > "$T/test_cartesian.log" || rc10=$?
grep -E "^==|FAIL|passed" "$T/test_cartesian.log"
rc5=0
"$T/test_kinematics_failclosed" > "$T/test_kinematics_failclosed.log" || rc5=$?
grep -E "^==|FAIL|passed" "$T/test_kinematics_failclosed.log"
rc9=0
"$T/test_kinematics_cart_off" > "$T/test_kinematics_cart_off.log" || rc9=$?
grep -E "^==|FAIL|passed" "$T/test_kinematics_cart_off.log"
rc6=0
"$T/test_kinematics" > "$T/test_kinematics.log" || rc6=$?
grep -E "^==|FAIL|passed" "$T/test_kinematics.log"
[ $rc -ne 0 ] && exit $rc
[ $rc2 -ne 0 ] && exit $rc2
[ $rc3 -ne 0 ] && exit $rc3
[ $rc4 -ne 0 ] && exit $rc4
[ $rc5 -ne 0 ] && exit $rc5
[ $rc7 -ne 0 ] && exit $rc7
[ $rc8 -ne 0 ] && exit $rc8
[ $rc9 -ne 0 ] && exit $rc9
[ $rc10 -ne 0 ] && exit $rc10
exit $rc6
