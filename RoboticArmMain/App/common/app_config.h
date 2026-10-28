/* app_config.h - feature flags and shared settings
 *
 * Don't edit this for local testing: put overrides in App/common/app_config_local.h (gitignored).
 * Every value here is wrapped in #ifndef so the local file wins.
 */
#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#if defined(__has_include)
#  if __has_include("app_config_local.h")
#    include "app_config_local.h"
#    define APP_CONFIG_LOCAL_ACTIVE 1
#  endif
#endif
#ifndef APP_CONFIG_LOCAL_ACTIVE
#  define APP_CONFIG_LOCAL_ACTIVE 0
#endif

#define APP_FW_VERSION "0.1.0"

/* ---------------------------------------------------------------- feature flags
 * 0 = the module still compiles but never runs. safety has no flag, it always runs */
#ifndef APP_USE_MOTION
#define APP_USE_MOTION          1   /* servo pwm + trajectory (TIM3/TIM4, TIM6 tick) */
#endif
#ifndef APP_USE_INPUT
#define APP_USE_INPUT           1   /* joystick adc, encoder, encoder / joystick buttons */
#endif
#ifndef APP_USE_KINEMATICS
#define APP_USE_KINEMATICS      1   /* ik / fk */
#endif
#ifndef APP_USE_VOICE
#define APP_USE_VOICE           1   /* sai mics + kws cnn */
#endif
#ifndef APP_USE_PI
#define APP_USE_PI              1   /* usart6 link to the raspberry pi */
#endif
#ifndef APP_USE_LCD
#define APP_USE_LCD             1   /* ili9341 + touch */
#endif
#ifndef APP_USE_MODES
#define APP_USE_MODES           1   /* mode state machine */
#endif

/* bench tests: 0 here, turn on in app_config_local.h */
#ifndef APP_USE_SERVO_CAL_TEST
#define APP_USE_SERVO_CAL_TEST  0   /* servo limit / home screen, moves the arm */
#endif
#ifndef APP_USE_IK_BENCH_TEST
#define APP_USE_IK_BENCH_TEST   0   /* one supervised IK move, moves the arm */
#endif

/* ---------------------------------------------------------------- timing */
#ifndef APP_CONTROL_HZ
#define APP_CONTROL_HZ          50u   /* TIM6 tick, must match the .ioc (PSC 10799, ARR 199) */
#endif
#ifndef APP_LOOP_WARN_US
#define APP_LOOP_WARN_US        100000u /* one main-loop pass longer than this loses audio */
#endif

#ifndef APP_TIMING_LOG
#define APP_TIMING_LOG          0     /* 1 = print the slowest voice / modes / lcd pass every 2 s */
#endif

/* ---------------------------------------------------------------- motion */
#ifndef APP_SPEED_FINE_DEG_S
#define APP_SPEED_FINE_DEG_S    10.0f
#endif
#ifndef APP_SPEED_FAST_DEG_S
#define APP_SPEED_FAST_DEG_S    45.0f
#endif
#ifndef APP_RAMP_S
#define APP_RAMP_S              0.25f /* time to reach full speed */
#endif
#ifndef APP_JOG_TIMEOUT_MS
#define APP_JOG_TIMEOUT_MS      150u  /* a jog not refreshed within this stops */
#endif
/* J6: a lower angle closes. endpoints of its safe 0..60 range */
#ifndef APP_GRIPPER_OPEN_DEG
#define APP_GRIPPER_OPEN_DEG    60.0f
#endif
#ifndef APP_GRIPPER_CLOSED_DEG
#define APP_GRIPPER_CLOSED_DEG  0.0f
#endif

/* ---------------------------------------------------------------- pi link */
#ifndef APP_PI_TIMEOUT_MS
#define APP_PI_TIMEOUT_MS       3000u /* TODO: tune with the camera pipeline */
#endif

/* ---------------------------------------------------------------- voice bench flags (local config only) */
#ifndef APP_VOICE_DRY_RUN
#define APP_VOICE_DRY_RUN       0     /* 1 = log the moves instead of moving */
#endif
#ifndef APP_VOICE_DRY_RUN_STEP_MS
#define APP_VOICE_DRY_RUN_STEP_MS 1000u /* how long a faked step takes */
#endif
#ifndef APP_VOICE_FAKE_PI
#define APP_VOICE_FAKE_PI       0     /* 1 = fake pi reply for a box word */
#endif

/* ---------------------------------------------------------------- sanity checks */
#if APP_USE_SERVO_CAL_TEST && (!APP_USE_MOTION || APP_USE_MODES || APP_USE_LCD || !APP_USE_INPUT)
#error "APP_USE_SERVO_CAL_TEST needs APP_USE_MOTION 1, APP_USE_MODES 0, APP_USE_LCD 0 and APP_USE_INPUT 1"
#endif
#if APP_USE_IK_BENCH_TEST && (!APP_USE_MOTION || APP_USE_MODES || APP_USE_LCD || !APP_USE_INPUT || \
                              !APP_USE_KINEMATICS || APP_USE_SERVO_CAL_TEST)
#error "APP_USE_IK_BENCH_TEST needs APP_USE_MOTION 1, APP_USE_MODES 0, APP_USE_LCD 0, APP_USE_INPUT 1, APP_USE_KINEMATICS 1 and APP_USE_SERVO_CAL_TEST 0"
#endif
#if APP_USE_MODES && !APP_USE_MOTION
#error "APP_USE_MODES needs APP_USE_MOTION (modes only set motion targets)"
#endif
#if APP_VOICE_FAKE_PI && !APP_VOICE_DRY_RUN
#error "APP_VOICE_FAKE_PI needs APP_VOICE_DRY_RUN (a fake position must never move the real arm)"
#endif

#endif /* APP_CONFIG_H */
