# Board evaluation - exp01_tcresnet-w16d2_aug-all_fold3 on S0

model `D:\Microcon69\ProjectRoboticArm\ProjectSpace\2026-micro-con\voice_command_research\workspace\experiments\exp01_tcresnet-w16d2_aug-all\runs\fold3\best.pth` (pytorch TC-ResNet w16 d2, 23 classes) · gain +15.0 dB · features librosa 0.11.0 (features.py) · 2026-10-21 18:04

## Clip mode (1 s clips, mono ch1)

n 109 · **accuracy 0.7982** · **macro-F1 0.8105** · commands 0.9255 · _unknown_ 0.0 · _silence_ None · commands rejected as unknown/silence 0 · speech peak median -32.4 dB (training clip -25.4 dB)

| cls | support | correct | recall | precision | f1 | mean_p_true | confused_with |
|---|---|---|---|---|---|---|---|
| move | 5 | 2 | 0.4 | 0.667 | 0.5 | 0.393 | no 2, hand 1 |
| pick | 2 | 2 | 1.0 | 0.667 | 0.8 | 0.996 |  |
| grab | 7 | 7 | 1.0 | 1.0 | 1.0 | 0.923 |  |
| blue box | 7 | 7 | 1.0 | 1.0 | 1.0 | 0.807 |  |
| green box | 6 | 6 | 1.0 | 1.0 | 1.0 | 0.986 |  |
| red box | 3 | 3 | 1.0 | 1.0 | 1.0 | 0.886 |  |
| black box | 4 | 4 | 1.0 | 1.0 | 1.0 | 0.78 |  |
| hand | 5 | 5 | 1.0 | 0.833 | 0.909 | 0.95 |  |
| station | 6 | 6 | 1.0 | 1.0 | 1.0 | 0.848 |  |
| base | 5 | 5 | 1.0 | 0.625 | 0.769 | 0.84 |  |
| forward | 4 | 4 | 1.0 | 0.667 | 0.8 | 0.967 |  |
| backward | 7 | 7 | 1.0 | 1.0 | 1.0 | 0.992 |  |
| left | 5 | 5 | 1.0 | 0.833 | 0.909 | 0.915 |  |
| right | 3 | 3 | 1.0 | 0.5 | 0.667 | 0.998 |  |
| place | 7 | 4 | 0.571 | 0.8 | 0.667 | 0.607 | base 3 |
| drop | 2 | 2 | 1.0 | 1.0 | 1.0 | 0.946 |  |
| yes | 2 | 1 | 0.5 | 1.0 | 0.667 | 0.507 | place 1 |
| no | 4 | 4 | 1.0 | 0.5 | 0.667 | 0.976 |  |
| home | 5 | 5 | 1.0 | 0.5 | 0.667 | 0.972 |  |
| stop | 5 | 5 | 1.0 | 1.0 | 1.0 | 0.993 |  |
| _unknown_ | 15 | 0 | 0.0 | 0.0 | 0.0 | 0.075 | home 5, right 3, forward 2 |

### Mistakes (top-3)

| clip_path | label | pred | top3 | snr_db | flags |
|---|---|---|---|---|---|
| clips/supanut/S0/_unknown_/pink_01.wav | _unknown_ | pick | pick 0.41 | hand 0.28 | _unknown_ 0.15 | 19.61 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/move/move_02.wav | move | hand | hand 0.41 | _unknown_ 0.14 | no 0.14 | 22.06 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/nose_01.wav | _unknown_ | no | no 0.98 | _unknown_ 0.01 | yes 0.00 | 24.64 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/write_01.wav | _unknown_ | right | right 1.00 | _unknown_ 0.00 | grab 0.00 | 25.15 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/mood_01.wav | _unknown_ | move | move 0.41 | no 0.13 | _unknown_ 0.12 | 22.31 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/place/place_04.wav | place | base | base 0.56 | place 0.39 | _unknown_ 0.01 | 23.3 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/phone_01.wav | _unknown_ | forward | forward 0.68 | home 0.23 | _unknown_ 0.05 | 24.42 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/place/place_06.wav | place | base | base 0.74 | place 0.16 | _unknown_ 0.03 | 18.83 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/moon_01.wav | _unknown_ | home | home 0.33 | hand 0.22 | move 0.19 | 23.7 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/go_01.wav | _unknown_ | home | home 0.84 | _unknown_ 0.03 | hand 0.03 | 27.44 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/move/move_04.wav | move | no | no 0.77 | move 0.07 | _unknown_ 0.07 | 26.44 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/foam_01.wav | _unknown_ | home | home 0.95 | _unknown_ 0.01 | hand 0.01 | 26.15 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/bright_01.wav | _unknown_ | right | right 1.00 | _unknown_ 0.00 | grab 0.00 | 24.09 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/open_01.wav | _unknown_ | home | home 0.33 | _unknown_ 0.31 | forward 0.27 | 24.52 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/yes/yes_02.wav | yes | place | place 0.42 | base 0.23 | _unknown_ 0.05 | 16.0 | sound longer than 1 s - the clip cuts it; S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/robot_01.wav | _unknown_ | forward | forward 0.23 | _unknown_ 0.16 | red box 0.14 | 22.74 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/wait_01.wav | _unknown_ | right | right 0.50 | place 0.20 | _unknown_ 0.13 | 19.73 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/place/place_07.wav | place | base | base 0.70 | place 0.27 | _unknown_ 0.01 | 19.77 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/blue_01.wav | _unknown_ | no | no 0.95 | move 0.04 | _unknown_ 0.01 | 23.87 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/move/move_05.wav | move | no | no 0.55 | move 0.15 | _unknown_ 0.08 | 25.43 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/bag_01.wav | _unknown_ | left | left 0.76 | backward 0.09 | right 0.05 | 17.09 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/groove_01.wav | _unknown_ | home | home 0.37 | move 0.26 | no 0.10 | 26.41 | S0 old wiring (VDD ~1.8 V) |

## Stream mode (raw 2 s takes, board logic: stride 10 frames, avg 3, threshold 0.7, refractory 1500 ms, channel auto)

command takes 94: **detected 0.3191** (hit 30, hit+extra 0), wrong 2, miss 62 · first detection correct 0.3191 · _unknown_ false alarms 5/15 · noise false alarms 0 in 0.0 min · channels {'ch1': 27, 'ch0/ch1': 16, 'ch0': 66}

| cls | takes | hit | hit_extra | wrong | miss | false_alarm | detected_rate | wrong_as | mean_max_p_true |
|---|---|---|---|---|---|---|---|---|---|
| move | 5 | 1 | 0 | 0 | 4 | 0 | 0.2 |  | 0.229 |
| pick | 2 | 1 | 0 | 0 | 1 | 0 | 0.5 |  | 0.501 |
| grab | 7 | 4 | 0 | 0 | 3 | 0 | 0.571 |  | 0.531 |
| blue box | 7 | 1 | 0 | 1 | 5 | 0 | 0.143 | no 1 | 0.302 |
| green box | 6 | 0 | 0 | 0 | 6 | 0 | 0.0 |  | 0.003 |
| red box | 3 | 2 | 0 | 0 | 1 | 0 | 0.667 |  | 0.592 |
| black box | 4 | 0 | 0 | 1 | 3 | 0 | 0.0 | backward 1 | 0.184 |
| hand | 5 | 1 | 0 | 0 | 4 | 0 | 0.2 |  | 0.2 |
| station | 6 | 2 | 0 | 0 | 4 | 0 | 0.333 |  | 0.291 |
| base | 5 | 1 | 0 | 0 | 4 | 0 | 0.2 |  | 0.169 |
| forward | 4 | 2 | 0 | 0 | 2 | 0 | 0.5 |  | 0.485 |
| backward | 7 | 1 | 0 | 0 | 6 | 0 | 0.143 |  | 0.213 |
| left | 5 | 2 | 0 | 0 | 3 | 0 | 0.4 |  | 0.337 |
| right | 3 | 0 | 0 | 0 | 3 | 0 | 0.0 |  | 0.006 |
| place | 7 | 3 | 0 | 0 | 4 | 0 | 0.429 |  | 0.363 |
| drop | 2 | 2 | 0 | 0 | 0 | 0 | 1.0 |  | 0.952 |
| yes | 2 | 2 | 0 | 0 | 0 | 0 | 1.0 |  | 0.98 |
| no | 4 | 1 | 0 | 0 | 3 | 0 | 0.25 |  | 0.25 |
| home | 5 | 2 | 0 | 0 | 3 | 0 | 0.4 |  | 0.392 |
| stop | 5 | 2 | 0 | 0 | 3 | 0 | 0.4 |  | 0.4 |
| _unknown_ | 15 | 0 | 0 | 0 | 0 | 5 |  | forward 1, right 1, home 1 | 0.054 |

### Takes that were not a clean hit

| raw_path | label | outcome | detections | max_avg_p_true | peak_top3 | channels_used |
|---|---|---|---|---|---|---|
| raw/supanut/S0/black_box/black_box_01_raw.wav | black box | wrong | backward@1.31s 0.73 | 0.733 | backward 0.73 | black box 0.10 | no 0.04 | ch1 |
| raw/supanut/S0/blue_box/blue_box_01_raw.wav | blue box | miss |  | 0.0 | _silence_ 0.97 | move 0.00 | base 0.00 | ch0 |
| raw/supanut/S0/home/home_02_raw.wav | home | miss |  | 0.005 | _silence_ 0.95 | move 0.01 | base 0.01 | ch0 |
| raw/supanut/S0/station/station_01_raw.wav | station | miss |  | 0.001 | _silence_ 0.97 | stop 0.00 | move 0.00 | ch0 |
| raw/supanut/S0/pick/pick_01_raw.wav | pick | miss |  | 0.006 | _silence_ 0.95 | move 0.01 | pick 0.01 | ch0 |
| raw/supanut/S0/black_box/black_box_02_raw.wav | black box | miss |  | 0.001 | _silence_ 0.92 | base 0.01 | pick 0.01 | ch0 |
| raw/supanut/S0/hand/hand_01_raw.wav | hand | miss |  | 0.004 | _silence_ 0.95 | move 0.01 | pick 0.01 | ch0 |
| raw/supanut/S0/move/move_02_raw.wav | move | miss |  | 0.01 | _silence_ 0.94 | pick 0.01 | move 0.01 | ch0 |
| raw/supanut/S0/backward/backward_02_raw.wav | backward | miss |  | 0.001 | _silence_ 0.98 | stop 0.00 | move 0.00 | ch0 |
| raw/supanut/S0/green_box/green_box_01_raw.wav | green box | miss |  | 0.001 | _silence_ 0.94 | pick 0.01 | move 0.01 | ch0 |
| raw/supanut/S0/base/base_01_raw.wav | base | miss |  | 0.006 | _silence_ 0.93 | pick 0.02 | move 0.01 | ch0 |
| raw/supanut/S0/green_box/green_box_02_raw.wav | green box | miss |  | 0.006 | _silence_ 0.88 | place 0.02 | base 0.01 | ch0 |
| raw/supanut/S0/no/no_01_raw.wav | no | miss |  | 0.001 | _silence_ 0.97 | move 0.00 | home 0.00 | ch0 |
| raw/supanut/S0/blue_box/blue_box_02_raw.wav | blue box | miss |  | 0.001 | _silence_ 0.88 | move 0.02 | pick 0.02 | ch0 |
| raw/supanut/S0/place/place_02_raw.wav | place | miss |  | 0.004 | _silence_ 0.82 | pick 0.12 | right 0.01 | ch0 |
| raw/supanut/S0/hand/hand_02_raw.wav | hand | miss |  | 0.004 | _silence_ 0.92 | pick 0.01 | move 0.01 | ch0 |
| raw/supanut/S0/right/right_01_raw.wav | right | miss |  | 0.006 | _silence_ 0.92 | move 0.01 | home 0.01 | ch0 |
| raw/supanut/S0/grab/grab_02_raw.wav | grab | miss |  | 0.002 | _silence_ 0.92 | pick 0.01 | move 0.01 | ch0 |
| raw/supanut/S0/base/base_02_raw.wav | base | miss |  | 0.01 | _silence_ 0.79 | pick 0.06 | move 0.02 | ch0 |
| raw/supanut/S0/green_box/green_box_03_raw.wav | green box | miss |  | 0.001 | _silence_ 0.94 | stop 0.01 | home 0.01 | ch0 |
| raw/supanut/S0/backward/backward_03_raw.wav | backward | miss |  | 0.003 | _silence_ 0.93 | base 0.01 | pick 0.01 | ch0 |
| raw/supanut/S0/backward/backward_04_raw.wav | backward | miss |  | 0.002 | _silence_ 0.95 | pick 0.01 | move 0.00 | ch0 |
| raw/supanut/S0/right/right_02_raw.wav | right | miss |  | 0.004 | _silence_ 0.95 | pick 0.01 | move 0.00 | ch0 |
| raw/supanut/S0/black_box/black_box_03_raw.wav | black box | miss |  | 0.001 | _silence_ 0.94 | move 0.01 | stop 0.01 | ch0 |
| raw/supanut/S0/green_box/green_box_04_raw.wav | green box | miss |  | 0.002 | _silence_ 0.94 | move 0.01 | base 0.01 | ch0 |
| raw/supanut/S0/base/base_03_raw.wav | base | miss |  | 0.008 | _silence_ 0.94 | move 0.01 | base 0.01 | ch0 |
| raw/supanut/S0/green_box/green_box_05_raw.wav | green box | miss |  | 0.005 | _silence_ 0.87 | pick 0.02 | home 0.01 | ch0 |
| raw/supanut/S0/red_box/red_box_02_raw.wav | red box | miss |  | 0.002 | _silence_ 0.94 | pick 0.01 | move 0.01 | ch0 |
| raw/supanut/S0/stop/stop_02_raw.wav | stop | miss |  | 0.005 | _silence_ 0.92 | base 0.01 | pick 0.01 | ch0 |
| raw/supanut/S0/backward/backward_05_raw.wav | backward | miss |  | 0.002 | _silence_ 0.96 | base 0.01 | stop 0.00 | ch0 |
| raw/supanut/S0/blue_box/blue_box_03_raw.wav | blue box | miss |  | 0.0 | _silence_ 0.95 | move 0.01 | base 0.00 | ch0 |
| raw/supanut/S0/home/home_03_raw.wav | home | miss |  | 0.005 | _silence_ 0.92 | pick 0.01 | move 0.01 | ch0 |
| raw/supanut/S0/green_box/green_box_06_raw.wav | green box | miss |  | 0.001 | _silence_ 0.96 | move 0.01 | pick 0.00 | ch0 |
| raw/supanut/S0/grab/grab_03_raw.wav | grab | miss |  | 0.002 | _silence_ 0.93 | base 0.01 | pick 0.01 | ch0 |
| raw/supanut/S0/black_box/black_box_04_raw.wav | black box | miss |  | 0.0 | _silence_ 0.95 | home 0.01 | pick 0.01 | ch0 |
| raw/supanut/S0/left/left_01_raw.wav | left | miss |  | 0.004 | _silence_ 0.94 | pick 0.01 | stop 0.01 | ch0 |
| raw/supanut/S0/move/move_03_raw.wav | move | miss |  | 0.004 | _silence_ 0.96 | move 0.00 | home 0.00 | ch0 |
| raw/supanut/S0/no/no_02_raw.wav | no | miss |  | 0.002 | _silence_ 0.93 | pick 0.01 | base 0.01 | ch0 |
| raw/supanut/S0/blue_box/blue_box_04_raw.wav | blue box | miss |  | 0.003 | _silence_ 0.69 | pick 0.19 | drop 0.03 | ch0 |
| raw/supanut/S0/stop/stop_03_raw.wav | stop | miss |  | 0.005 | _silence_ 0.94 | base 0.01 | home 0.01 | ch0 |
| raw/supanut/S0/place/place_04_raw.wav | place | miss |  | 0.005 | _silence_ 0.86 | stop 0.03 | left 0.01 | ch0 |
| raw/supanut/S0/grab/grab_06_raw.wav | grab | miss |  | 0.003 | _silence_ 0.93 | home 0.01 | move 0.01 | ch0 |
| raw/supanut/S0/forward/forward_01_raw.wav | forward | miss |  | 0.001 | _silence_ 0.95 | move 0.01 | stop 0.01 | ch0 |
| raw/supanut/S0/station/station_03_raw.wav | station | miss |  | 0.01 | _silence_ 0.85 | stop 0.04 | drop 0.01 | ch0 |
| raw/supanut/S0/_unknown_/phone_01_raw.wav | _unknown_ | false_alarm | forward@1.31s 0.73 | 0.061 | forward 0.77 | home 0.16 | _unknown_ 0.03 | ch0/ch1 |
| raw/supanut/S0/backward/backward_06_raw.wav | backward | miss |  | 0.005 | _silence_ 0.93 | move 0.01 | home 0.01 | ch0 |
| raw/supanut/S0/place/place_06_raw.wav | place | miss |  | 0.002 | _silence_ 0.95 | pick 0.02 | left 0.00 | ch0 |
| raw/supanut/S0/hand/hand_03_raw.wav | hand | miss |  | 0.001 | _silence_ 0.94 | pick 0.01 | move 0.01 | ch0 |
| raw/supanut/S0/hand/hand_04_raw.wav | hand | miss |  | 0.001 | _silence_ 0.97 | pick 0.01 | stop 0.00 | ch0 |
| raw/supanut/S0/station/station_04_raw.wav | station | miss |  | 0.001 | _silence_ 0.90 | move 0.02 | home 0.02 | ch0 |
| raw/supanut/S0/forward/forward_02_raw.wav | forward | miss |  | 0.001 | _silence_ 0.57 | pick 0.08 | stop 0.07 | ch0 |
| raw/supanut/S0/move/move_04_raw.wav | move | miss |  | 0.011 | _silence_ 0.90 | pick 0.02 | move 0.01 | ch0 |
| raw/supanut/S0/station/station_05_raw.wav | station | miss |  | 0.002 | _silence_ 0.95 | move 0.01 | base 0.01 | ch0 |
| raw/supanut/S0/backward/backward_07_raw.wav | backward | miss |  | 0.487 | backward 0.49 | forward 0.11 | left 0.08 | ch0/ch1 |
| raw/supanut/S0/home/home_04_raw.wav | home | miss |  | 0.004 | _silence_ 0.97 | move 0.00 | home 0.00 | ch0 |
| raw/supanut/S0/right/right_03_raw.wav | right | miss |  | 0.008 | _silence_ 0.86 | move 0.02 | base 0.02 | ch0 |
| raw/supanut/S0/_unknown_/bright_01_raw.wav | _unknown_ | false_alarm | right@1.31s 0.99 | 0.002 | right 1.00 | _unknown_ 0.00 | grab 0.00 | ch0/ch1 |
| raw/supanut/S0/left/left_03_raw.wav | left | miss |  | 0.001 | _silence_ 0.97 | base 0.00 | stop 0.00 | ch0 |
| raw/supanut/S0/_unknown_/open_01_raw.wav | _unknown_ | false_alarm | home@1.01s 0.78 | 0.294 | home 0.78 | forward 0.13 | _unknown_ 0.04 | ch0/ch1 |
| raw/supanut/S0/stop/stop_04_raw.wav | stop | miss |  | 0.004 | _silence_ 0.94 | home 0.01 | move 0.01 | ch0 |
| raw/supanut/S0/blue_box/blue_box_06_raw.wav | blue box | wrong | no@1.01s 0.72 | 0.603 | no 0.72 | move 0.06 | _unknown_ 0.05 | ch1 |
| raw/supanut/S0/base/base_04_raw.wav | base | miss |  | 0.006 | _silence_ 0.96 | base 0.01 | move 0.00 | ch0 |
| raw/supanut/S0/blue_box/blue_box_07_raw.wav | blue box | miss |  | 0.609 | blue box 0.61 | move 0.19 | no 0.13 | ch0/ch1 |
| raw/supanut/S0/left/left_05_raw.wav | left | miss |  | 0.017 | _silence_ 0.80 | pick 0.03 | home 0.03 | ch0 |
| raw/supanut/S0/place/place_07_raw.wav | place | miss |  | 0.003 | _silence_ 0.58 | pick 0.18 | drop 0.06 | ch0 |
| raw/supanut/S0/no/no_03_raw.wav | no | miss |  | 0.019 | _silence_ 0.83 | home 0.05 | move 0.02 | ch0 |
| raw/supanut/S0/_unknown_/blue_01_raw.wav | _unknown_ | false_alarm | no@1.31s 0.89 | 0.037 | no 0.94 | move 0.04 | _unknown_ 0.01 | ch1 |
| raw/supanut/S0/move/move_05_raw.wav | move | miss |  | 0.229 | no 0.45 | move 0.18 | _unknown_ 0.10 | ch1 |
| raw/supanut/S0/_unknown_/bag_01_raw.wav | _unknown_ | false_alarm | left@1.01s 0.74 | 0.036 | left 0.76 | backward 0.06 | right 0.06 | ch1 |
