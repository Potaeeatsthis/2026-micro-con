# Board evaluation - exp01_tcresnet-w16d2_aug-all_fold0 on S0

model `D:\Microcon69\ProjectRoboticArm\ProjectSpace\2026-micro-con\voice_command_research\workspace\experiments\exp01_tcresnet-w16d2_aug-all\runs\fold0\best.pth` (pytorch TC-ResNet w16 d2, 23 classes) · gain +15.0 dB · features librosa 0.11.0 (features.py) · 2026-10-21 16:43

## Clip mode (1 s clips, mono ch1)

n 109 · **accuracy 0.7982** · **macro-F1 0.8126** · commands 0.9149 · _unknown_ 0.0667 · _silence_ None · commands rejected as unknown/silence 1 · speech peak median -32.4 dB (training clip -25.4 dB)

| cls | support | correct | recall | precision | f1 | mean_p_true | confused_with |
|---|---|---|---|---|---|---|---|
| move | 5 | 4 | 0.8 | 0.667 | 0.727 | 0.572 | hand 1 |
| pick | 2 | 2 | 1.0 | 0.667 | 0.8 | 0.997 |  |
| grab | 7 | 7 | 1.0 | 0.875 | 0.933 | 0.979 |  |
| blue box | 7 | 6 | 0.857 | 1.0 | 0.923 | 0.472 | _unknown_ 1 |
| green box | 6 | 6 | 1.0 | 0.857 | 0.923 | 0.982 |  |
| red box | 3 | 3 | 1.0 | 1.0 | 1.0 | 0.929 |  |
| black box | 4 | 4 | 1.0 | 1.0 | 1.0 | 0.63 |  |
| hand | 5 | 5 | 1.0 | 0.833 | 0.909 | 0.963 |  |
| station | 6 | 6 | 1.0 | 1.0 | 1.0 | 0.728 |  |
| base | 5 | 3 | 0.6 | 0.6 | 0.6 | 0.605 | place 2 |
| forward | 4 | 4 | 1.0 | 1.0 | 1.0 | 0.903 |  |
| backward | 7 | 7 | 1.0 | 1.0 | 1.0 | 0.996 |  |
| left | 5 | 5 | 1.0 | 0.833 | 0.909 | 0.817 |  |
| right | 3 | 3 | 1.0 | 0.6 | 0.75 | 0.998 |  |
| place | 7 | 5 | 0.714 | 0.556 | 0.625 | 0.632 | base 2 |
| drop | 2 | 1 | 0.5 | 1.0 | 0.667 | 0.546 | grab 1 |
| yes | 2 | 1 | 0.5 | 1.0 | 0.667 | 0.542 | place 1 |
| no | 4 | 4 | 1.0 | 0.667 | 0.8 | 0.986 |  |
| home | 5 | 5 | 1.0 | 0.556 | 0.714 | 0.978 |  |
| stop | 5 | 5 | 1.0 | 1.0 | 1.0 | 0.991 |  |
| _unknown_ | 15 | 1 | 0.067 | 0.5 | 0.118 | 0.082 | home 4, move 2, right 2 |

### Mistakes (top-3)

| clip_path | label | pred | top3 | snr_db | flags |
|---|---|---|---|---|---|
| clips/supanut/S0/_unknown_/pink_01.wav | _unknown_ | pick | pick 0.74 | hand 0.15 | _unknown_ 0.06 | 19.61 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/move/move_02.wav | move | hand | hand 0.61 | home 0.14 | no 0.10 | 22.06 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/nose_01.wav | _unknown_ | no | no 0.89 | _unknown_ 0.04 | yes 0.03 | 24.64 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/write_01.wav | _unknown_ | right | right 1.00 | _unknown_ 0.00 | backward 0.00 | 25.15 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/mood_01.wav | _unknown_ | move | move 0.60 | no 0.09 | _unknown_ 0.09 | 22.31 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/drop/drop_01.wav | drop | grab | grab 0.49 | drop 0.45 | _unknown_ 0.02 | 22.54 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/phone_01.wav | _unknown_ | home | home 0.60 | forward 0.19 | _unknown_ 0.12 | 24.42 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/place/place_06.wav | place | base | base 0.92 | place 0.05 | _unknown_ 0.01 | 18.83 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/moon_01.wav | _unknown_ | home | home 0.36 | move 0.22 | _unknown_ 0.13 | 23.7 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/go_01.wav | _unknown_ | home | home 0.58 | hand 0.24 | _unknown_ 0.05 | 27.44 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/foam_01.wav | _unknown_ | home | home 0.96 | _unknown_ 0.01 | no 0.01 | 26.15 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/bright_01.wav | _unknown_ | right | right 0.99 | grab 0.01 | _unknown_ 0.00 | 24.09 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/yes/yes_02.wav | yes | place | place 0.52 | base 0.15 | yes 0.09 | 16.0 | sound longer than 1 s - the clip cuts it; S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/robot_01.wav | _unknown_ | green box | green box 0.34 | red box 0.22 | _unknown_ 0.15 | 22.74 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/wait_01.wav | _unknown_ | place | place 0.66 | right 0.12 | _unknown_ 0.07 | 19.73 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/blue_box/blue_box_06.wav | blue box | _unknown_ | _unknown_ 0.23 | green box 0.21 | blue box 0.19 | 22.3 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/base/base_04.wav | base | place | place 0.72 | base 0.24 | _unknown_ 0.01 | 17.58 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/place/place_07.wav | place | base | base 0.85 | place 0.13 | _unknown_ 0.01 | 19.77 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/blue_01.wav | _unknown_ | no | no 0.85 | move 0.14 | _unknown_ 0.00 | 23.87 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/base/base_05.wav | base | place | place 0.56 | base 0.37 | _unknown_ 0.02 | 16.74 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/bag_01.wav | _unknown_ | left | left 0.72 | backward 0.12 | no 0.04 | 17.09 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/groove_01.wav | _unknown_ | move | move 0.66 | _unknown_ 0.11 | home 0.08 | 26.41 | S0 old wiring (VDD ~1.8 V) |

## Stream mode (raw 2 s takes, board logic: stride 10 frames, avg 3, threshold 0.7, refractory 1500 ms, channel auto)

command takes 94: **detected 0.2553** (hit 24, hit+extra 0), wrong 3, miss 67 · first detection correct 0.2553 · _unknown_ false alarms 7/15 · noise false alarms 0 in 0.0 min · channels {'ch1': 27, 'ch0/ch1': 16, 'ch0': 66}

| cls | takes | hit | hit_extra | wrong | miss | false_alarm | detected_rate | wrong_as | mean_max_p_true |
|---|---|---|---|---|---|---|---|---|---|
| move | 5 | 1 | 0 | 0 | 4 | 0 | 0.2 |  | 0.311 |
| pick | 2 | 1 | 0 | 0 | 1 | 0 | 0.5 |  | 0.499 |
| grab | 7 | 4 | 0 | 0 | 3 | 0 | 0.571 |  | 0.561 |
| blue box | 7 | 0 | 0 | 0 | 7 | 0 | 0.0 |  | 0.176 |
| green box | 6 | 0 | 0 | 0 | 6 | 0 | 0.0 |  | 0.002 |
| red box | 3 | 2 | 0 | 0 | 1 | 0 | 0.667 |  | 0.605 |
| black box | 4 | 0 | 0 | 1 | 3 | 0 | 0.0 | left 1 | 0.146 |
| hand | 5 | 1 | 0 | 0 | 4 | 0 | 0.2 |  | 0.204 |
| station | 6 | 1 | 0 | 1 | 4 | 0 | 0.167 | base 1 | 0.195 |
| base | 5 | 0 | 0 | 0 | 5 | 0 | 0.0 |  | 0.123 |
| forward | 4 | 2 | 0 | 0 | 2 | 0 | 0.5 |  | 0.473 |
| backward | 7 | 0 | 0 | 1 | 6 | 0 | 0.0 | left 1 | 0.222 |
| left | 5 | 2 | 0 | 0 | 3 | 0 | 0.4 |  | 0.316 |
| right | 3 | 0 | 0 | 0 | 3 | 0 | 0.0 |  | 0.003 |
| place | 7 | 2 | 0 | 0 | 5 | 0 | 0.286 |  | 0.361 |
| drop | 2 | 1 | 0 | 0 | 1 | 0 | 0.5 |  | 0.625 |
| yes | 2 | 2 | 0 | 0 | 0 | 0 | 1.0 |  | 0.996 |
| no | 4 | 1 | 0 | 0 | 3 | 0 | 0.25 |  | 0.255 |
| home | 5 | 2 | 0 | 0 | 3 | 0 | 0.4 |  | 0.396 |
| stop | 5 | 2 | 0 | 0 | 3 | 0 | 0.4 |  | 0.398 |
| _unknown_ | 15 | 0 | 0 | 0 | 0 | 7 |  | home 2, right 1, place 1 | 0.062 |

### Takes that were not a clean hit

| raw_path | label | outcome | detections | max_avg_p_true | peak_top3 | channels_used |
|---|---|---|---|---|---|---|
| raw/supanut/S0/black_box/black_box_01_raw.wav | black box | wrong | left@1.01s 0.74 | 0.586 | left 0.74 | yes 0.13 | no 0.03 | ch1 |
| raw/supanut/S0/blue_box/blue_box_01_raw.wav | blue box | miss |  | 0.0 | _silence_ 0.98 | home 0.00 | left 0.00 | ch0 |
| raw/supanut/S0/home/home_02_raw.wav | home | miss |  | 0.003 | _silence_ 0.98 | home 0.00 | left 0.00 | ch0 |
| raw/supanut/S0/backward/backward_01_raw.wav | backward | wrong | left@1.01s 0.79 | 0.996 | backward 1.00 | black box 0.00 | left 0.00 | ch1 |
| raw/supanut/S0/station/station_01_raw.wav | station | miss |  | 0.0 | _silence_ 0.98 | home 0.00 | move 0.00 | ch0 |
| raw/supanut/S0/pick/pick_01_raw.wav | pick | miss |  | 0.002 | _silence_ 0.97 | home 0.00 | left 0.00 | ch0 |
| raw/supanut/S0/black_box/black_box_02_raw.wav | black box | miss |  | 0.0 | _silence_ 0.96 | move 0.00 | home 0.00 | ch0 |
| raw/supanut/S0/hand/hand_01_raw.wav | hand | miss |  | 0.002 | _silence_ 0.97 | home 0.00 | move 0.00 | ch0 |
| raw/supanut/S0/move/move_02_raw.wav | move | miss |  | 0.003 | _silence_ 0.96 | home 0.01 | move 0.00 | ch0 |
| raw/supanut/S0/backward/backward_02_raw.wav | backward | miss |  | 0.004 | _silence_ 0.95 | home 0.01 | backward 0.00 | ch0 |
| raw/supanut/S0/green_box/green_box_01_raw.wav | green box | miss |  | 0.003 | _silence_ 0.94 | home 0.01 | move 0.01 | ch0 |
| raw/supanut/S0/base/base_01_raw.wav | base | miss |  | 0.004 | _silence_ 0.96 | left 0.00 | base 0.00 | ch0 |
| raw/supanut/S0/green_box/green_box_02_raw.wav | green box | miss |  | 0.001 | _silence_ 0.98 | home 0.01 | move 0.00 | ch0 |
| raw/supanut/S0/no/no_01_raw.wav | no | miss |  | 0.002 | _silence_ 0.94 | home 0.01 | move 0.01 | ch0 |
| raw/supanut/S0/blue_box/blue_box_02_raw.wav | blue box | miss |  | 0.001 | _silence_ 0.93 | home 0.01 | move 0.01 | ch0 |
| raw/supanut/S0/place/place_02_raw.wav | place | miss |  | 0.001 | _silence_ 0.96 | home 0.01 | move 0.00 | ch0 |
| raw/supanut/S0/hand/hand_02_raw.wav | hand | miss |  | 0.002 | _silence_ 0.96 | home 0.01 | move 0.00 | ch0 |
| raw/supanut/S0/right/right_01_raw.wav | right | miss |  | 0.002 | _silence_ 0.96 | home 0.01 | left 0.00 | ch0 |
| raw/supanut/S0/grab/grab_02_raw.wav | grab | miss |  | 0.002 | _silence_ 0.96 | left 0.01 | home 0.00 | ch0 |
| raw/supanut/S0/base/base_02_raw.wav | base | miss |  | 0.006 | _silence_ 0.90 | home 0.01 | backward 0.01 | ch0 |
| raw/supanut/S0/green_box/green_box_03_raw.wav | green box | miss |  | 0.002 | _silence_ 0.89 | backward 0.01 | base 0.01 | ch0 |
| raw/supanut/S0/backward/backward_03_raw.wav | backward | miss |  | 0.005 | _silence_ 0.93 | home 0.01 | move 0.01 | ch0 |
| raw/supanut/S0/backward/backward_04_raw.wav | backward | miss |  | 0.004 | _silence_ 0.93 | home 0.01 | drop 0.01 | ch0 |
| raw/supanut/S0/right/right_02_raw.wav | right | miss |  | 0.003 | _silence_ 0.92 | left 0.01 | move 0.01 | ch0 |
| raw/supanut/S0/black_box/black_box_03_raw.wav | black box | miss |  | 0.0 | _silence_ 0.96 | home 0.01 | backward 0.00 | ch0 |
| raw/supanut/S0/green_box/green_box_04_raw.wav | green box | miss |  | 0.001 | _silence_ 0.97 | home 0.00 | move 0.00 | ch0 |
| raw/supanut/S0/base/base_03_raw.wav | base | miss |  | 0.004 | _silence_ 0.95 | home 0.01 | backward 0.01 | ch0 |
| raw/supanut/S0/green_box/green_box_05_raw.wav | green box | miss |  | 0.002 | _silence_ 0.96 | base 0.00 | move 0.00 | ch0 |
| raw/supanut/S0/red_box/red_box_02_raw.wav | red box | miss |  | 0.001 | _silence_ 0.96 | backward 0.00 | base 0.00 | ch0 |
| raw/supanut/S0/stop/stop_02_raw.wav | stop | miss |  | 0.001 | _silence_ 0.98 | home 0.00 | left 0.00 | ch0 |
| raw/supanut/S0/backward/backward_05_raw.wav | backward | miss |  | 0.003 | _silence_ 0.95 | base 0.00 | home 0.00 | ch0 |
| raw/supanut/S0/blue_box/blue_box_03_raw.wav | blue box | miss |  | 0.001 | _silence_ 0.95 | left 0.01 | base 0.00 | ch0 |
| raw/supanut/S0/home/home_03_raw.wav | home | miss |  | 0.004 | _silence_ 0.96 | home 0.00 | left 0.00 | ch0 |
| raw/supanut/S0/green_box/green_box_06_raw.wav | green box | miss |  | 0.0 | _silence_ 0.97 | home 0.00 | move 0.00 | ch0 |
| raw/supanut/S0/grab/grab_03_raw.wav | grab | miss |  | 0.003 | _silence_ 0.94 | home 0.01 | move 0.01 | ch0 |
| raw/supanut/S0/black_box/black_box_04_raw.wav | black box | miss |  | 0.0 | _silence_ 0.94 | home 0.01 | move 0.00 | ch0 |
| raw/supanut/S0/left/left_01_raw.wav | left | miss |  | 0.004 | _silence_ 0.96 | home 0.00 | left 0.00 | ch0 |
| raw/supanut/S0/drop/drop_01_raw.wav | drop | miss |  | 0.53 | drop 0.53 | grab 0.41 | _unknown_ 0.02 | ch0/ch1 |
| raw/supanut/S0/move/move_03_raw.wav | move | miss |  | 0.002 | _silence_ 0.98 | base 0.00 | home 0.00 | ch0 |
| raw/supanut/S0/no/no_02_raw.wav | no | miss |  | 0.001 | _silence_ 0.98 | home 0.00 | base 0.00 | ch0 |
| raw/supanut/S0/blue_box/blue_box_04_raw.wav | blue box | miss |  | 0.005 | _silence_ 0.77 | _unknown_ 0.02 | drop 0.02 | ch0 |
| raw/supanut/S0/stop/stop_03_raw.wav | stop | miss |  | 0.005 | _silence_ 0.90 | home 0.02 | _unknown_ 0.01 | ch0 |
| raw/supanut/S0/place/place_04_raw.wav | place | miss |  | 0.002 | _silence_ 0.88 | home 0.02 | backward 0.02 | ch0 |
| raw/supanut/S0/grab/grab_06_raw.wav | grab | miss |  | 0.002 | _silence_ 0.93 | home 0.01 | move 0.01 | ch0 |
| raw/supanut/S0/forward/forward_01_raw.wav | forward | miss |  | 0.049 | _silence_ 0.29 | backward 0.11 | _unknown_ 0.09 | ch0 |
| raw/supanut/S0/station/station_03_raw.wav | station | miss |  | 0.002 | _silence_ 0.88 | drop 0.01 | left 0.01 | ch0 |
| raw/supanut/S0/place/place_05_raw.wav | place | miss |  | 0.626 | place 0.63 | base 0.35 | _unknown_ 0.01 | ch1 |
| raw/supanut/S0/_unknown_/phone_01_raw.wav | _unknown_ | false_alarm | home@1.81s 0.71 | 0.104 | home 0.71 | forward 0.12 | _unknown_ 0.07 | ch0/ch1 |
| raw/supanut/S0/backward/backward_06_raw.wav | backward | miss |  | 0.008 | _silence_ 0.84 | home 0.04 | drop 0.02 | ch0 |
| raw/supanut/S0/place/place_06_raw.wav | place | miss |  | 0.001 | _silence_ 0.95 | home 0.02 | left 0.00 | ch0 |
| raw/supanut/S0/hand/hand_03_raw.wav | hand | miss |  | 0.014 | _silence_ 0.58 | home 0.16 | drop 0.04 | ch0 |
| raw/supanut/S0/hand/hand_04_raw.wav | hand | miss |  | 0.003 | _silence_ 0.82 | home 0.06 | drop 0.03 | ch0 |
| raw/supanut/S0/station/station_04_raw.wav | station | miss |  | 0.0 | _silence_ 0.62 | home 0.27 | no 0.03 | ch0 |
| raw/supanut/S0/forward/forward_02_raw.wav | forward | miss |  | 0.004 | _silence_ 0.73 | home 0.07 | no 0.06 | ch0 |
| raw/supanut/S0/move/move_04_raw.wav | move | miss |  | 0.004 | _silence_ 0.95 | base 0.01 | move 0.00 | ch0 |
| raw/supanut/S0/station/station_05_raw.wav | station | miss |  | 0.0 | _silence_ 0.98 | home 0.00 | base 0.00 | ch0 |
| raw/supanut/S0/backward/backward_07_raw.wav | backward | miss |  | 0.537 | backward 0.54 | left 0.23 | _unknown_ 0.05 | ch0/ch1 |
| raw/supanut/S0/station/station_06_raw.wav | station | wrong | base@1.31s 0.80 | 0.318 | base 0.80 | station 0.08 | place 0.06 | ch0/ch1 |
| raw/supanut/S0/home/home_04_raw.wav | home | miss |  | 0.004 | _silence_ 0.96 | home 0.00 | move 0.00 | ch0 |
| raw/supanut/S0/right/right_03_raw.wav | right | miss |  | 0.004 | _silence_ 0.90 | home 0.01 | move 0.01 | ch0 |
| raw/supanut/S0/_unknown_/bright_01_raw.wav | _unknown_ | false_alarm | right@1.31s 0.98 | 0.002 | right 0.99 | grab 0.01 | _unknown_ 0.00 | ch0/ch1 |
| raw/supanut/S0/left/left_03_raw.wav | left | miss |  | 0.001 | _silence_ 0.98 | base 0.00 | home 0.00 | ch0 |
| raw/supanut/S0/_unknown_/open_01_raw.wav | _unknown_ | false_alarm | home@1.01s 0.87 | 0.37 | home 0.87 | hand 0.04 | _unknown_ 0.03 | ch0/ch1 |
| raw/supanut/S0/blue_box/blue_box_05_raw.wav | blue box | miss |  | 0.57 | blue box 0.57 | green box 0.16 | no 0.08 | ch1 |
| raw/supanut/S0/stop/stop_04_raw.wav | stop | miss |  | 0.001 | _silence_ 0.97 | home 0.00 | undo 0.00 | ch0 |
| raw/supanut/S0/_unknown_/wait_01_raw.wav | _unknown_ | false_alarm | place@1.31s 0.71 | 0.067 | place 0.71 | right 0.11 | move 0.06 | ch1 |
| raw/supanut/S0/blue_box/blue_box_06_raw.wav | blue box | miss |  | 0.357 | no 0.55 | move 0.20 | home 0.10 | ch1 |
| raw/supanut/S0/base/base_04_raw.wav | base | miss |  | 0.002 | _silence_ 0.98 | base 0.00 | home 0.00 | ch0 |
| raw/supanut/S0/blue_box/blue_box_07_raw.wav | blue box | miss |  | 0.301 | no 0.61 | move 0.18 | _unknown_ 0.07 | ch0/ch1 |
| raw/supanut/S0/left/left_05_raw.wav | left | miss |  | 0.011 | _silence_ 0.75 | home 0.06 | backward 0.03 | ch0 |
| raw/supanut/S0/place/place_07_raw.wav | place | miss |  | 0.003 | _silence_ 0.83 | home 0.04 | drop 0.02 | ch0 |
| raw/supanut/S0/no/no_03_raw.wav | no | miss |  | 0.028 | _silence_ 0.72 | home 0.13 | drop 0.02 | ch0 |
| raw/supanut/S0/_unknown_/blue_01_raw.wav | _unknown_ | false_alarm | no@1.31s 0.74 | 0.036 | no 0.87 | move 0.12 | _unknown_ 0.00 | ch1 |
| raw/supanut/S0/move/move_05_raw.wav | move | miss |  | 0.601 | move 0.60 | hand 0.14 | no 0.12 | ch1 |
| raw/supanut/S0/base/base_05_raw.wav | base | miss |  | 0.599 | place 0.61 | base 0.33 | _unknown_ 0.02 | ch1 |
| raw/supanut/S0/_unknown_/bag_01_raw.wav | _unknown_ | false_alarm | left@1.11s 0.75 | 0.038 | left 0.76 | backward 0.09 | grab 0.03 | ch1 |
| raw/supanut/S0/_unknown_/groove_01_raw.wav | _unknown_ | false_alarm | move@1.81s 0.76 | 0.099 | move 0.76 | home 0.08 | _unknown_ 0.04 | ch1 |
