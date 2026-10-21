# Board evaluation - exp04_tcresnet-w16d3_aug-all on S1

model `D:\Microcon69\ProjectRoboticArm\ProjectSpace\2026-micro-con\voice_command_research\workspace\experiments\exp04_tcresnet-w16d3_aug-all\runs\final.pth` (pytorch exp04_tcresnet-w16d3_aug-all, 23 classes) · gain +15.0 dB · features librosa 0.11.0 (features.py) · 2026-10-21 20:32

## Clip mode (1 s clips, mono ch1)

n 290 · **accuracy 0.8931** · **macro-F1 0.8925** · commands 0.99 · _unknown_ 0.0333 · _silence_ 1.0 · commands rejected as unknown/silence 0 · speech peak median -34.7 dB (training clip -25.4 dB)

| cls | support | correct | recall | precision | f1 | mean_p_true | confused_with |
|---|---|---|---|---|---|---|---|
| move | 10 | 10 | 1.0 | 0.909 | 0.952 | 0.722 |  |
| pick | 10 | 10 | 1.0 | 0.833 | 0.909 | 0.967 |  |
| grab | 10 | 10 | 1.0 | 1.0 | 1.0 | 0.83 |  |
| blue box | 10 | 10 | 1.0 | 1.0 | 1.0 | 0.948 |  |
| green box | 10 | 10 | 1.0 | 0.833 | 0.909 | 0.948 |  |
| red box | 10 | 9 | 0.9 | 1.0 | 0.947 | 0.767 | green box 1 |
| black box | 10 | 10 | 1.0 | 1.0 | 1.0 | 0.798 |  |
| hand | 10 | 10 | 1.0 | 1.0 | 1.0 | 0.855 |  |
| station | 10 | 10 | 1.0 | 1.0 | 1.0 | 0.972 |  |
| base | 10 | 9 | 0.9 | 1.0 | 0.947 | 0.69 | place 1 |
| forward | 10 | 10 | 1.0 | 0.833 | 0.909 | 0.924 |  |
| backward | 10 | 10 | 1.0 | 1.0 | 1.0 | 0.994 |  |
| left | 10 | 10 | 1.0 | 0.769 | 0.87 | 0.969 |  |
| right | 10 | 10 | 1.0 | 0.714 | 0.833 | 0.992 |  |
| place | 10 | 10 | 1.0 | 0.833 | 0.909 | 0.886 |  |
| drop | 10 | 10 | 1.0 | 0.909 | 0.952 | 0.967 |  |
| yes | 10 | 10 | 1.0 | 0.833 | 0.909 | 0.977 |  |
| no | 10 | 10 | 1.0 | 0.667 | 0.8 | 0.979 |  |
| home | 10 | 10 | 1.0 | 0.625 | 0.769 | 0.933 |  |
| stop | 10 | 10 | 1.0 | 0.909 | 0.952 | 0.982 |  |
| _unknown_ | 30 | 1 | 0.033 | 1.0 | 0.065 | 0.068 | home 6, no 5, right 4 |
| _silence_ | 60 | 60 | 1.0 | 1.0 | 1.0 | 0.937 |  |

### Mistakes (top-3)

| clip_path | label | pred | top3 | snr_db | flags |
|---|---|---|---|---|---|
| clips/supanut/S1/_unknown_/arm_01.wav | _unknown_ | home | home 0.83 | _unknown_ 0.05 | no 0.03 | 22.45 |  |
| clips/supanut/S1/_unknown_/moon_01.wav | _unknown_ | forward | forward 0.49 | home 0.20 | move 0.10 | 25.84 |  |
| clips/supanut/S1/_unknown_/hold_01.wav | _unknown_ | home | home 0.55 | hand 0.13 | pick 0.06 | 24.63 |  |
| clips/supanut/S1/_unknown_/books_01.wav | _unknown_ | yes | yes 0.30 | no 0.08 | move 0.08 | 23.75 |  |
| clips/supanut/S1/_unknown_/close_01.wav | _unknown_ | yes | yes 0.47 | _unknown_ 0.24 | no 0.09 | 17.22 |  |
| clips/supanut/S1/_unknown_/nose_01.wav | _unknown_ | no | no 0.94 | _unknown_ 0.02 | move 0.01 | 21.15 |  |
| clips/supanut/S1/_unknown_/groove_01.wav | _unknown_ | home | home 0.46 | move 0.30 | forward 0.10 | 25.92 |  |
| clips/supanut/S1/_unknown_/wait_01.wav | _unknown_ | place | place 0.36 | right 0.30 | _unknown_ 0.09 | 19.04 |  |
| clips/supanut/S1/_unknown_/now_01.wav | _unknown_ | no | no 0.97 | _unknown_ 0.01 | stop 0.00 | 19.3 |  |
| clips/supanut/S1/_unknown_/pink_01.wav | _unknown_ | pick | pick 0.85 | hand 0.07 | _unknown_ 0.04 | 13.71 |  |
| clips/supanut/S1/_unknown_/bright_01.wav | _unknown_ | right | right 0.99 | _unknown_ 0.00 | backward 0.00 | 20.23 |  |
| clips/supanut/S1/red_box/red_box_06.wav | red box | green box | green box 0.84 | red box 0.07 | blue box 0.02 | 20.8 |  |
| clips/supanut/S1/_unknown_/back_01.wav | _unknown_ | left | left 0.83 | backward 0.10 | _unknown_ 0.01 | 18.02 |  |
| clips/supanut/S1/_unknown_/grape_01.wav | _unknown_ | right | right 0.93 | _unknown_ 0.04 | grab 0.01 | 26.07 |  |
| clips/supanut/S1/_unknown_/phone_01.wav | _unknown_ | forward | forward 0.85 | _unknown_ 0.05 | home 0.03 | 21.41 |  |
| clips/supanut/S1/_unknown_/black_01.wav | _unknown_ | left | left 0.90 | yes 0.04 | _unknown_ 0.01 | 18.92 |  |
| clips/supanut/S1/_unknown_/box_01.wav | _unknown_ | no | no 0.61 | yes 0.10 | left 0.08 | 20.34 |  |
| clips/supanut/S1/_unknown_/mood_01.wav | _unknown_ | move | move 0.49 | home 0.31 | no 0.07 | 20.3 |  |
| clips/supanut/S1/_unknown_/blue_01.wav | _unknown_ | no | no 0.81 | move 0.08 | _unknown_ 0.02 | 21.44 |  |
| clips/supanut/S1/_unknown_/go_01.wav | _unknown_ | home | home 0.96 | _unknown_ 0.01 | no 0.01 | 24.55 |  |
| clips/supanut/S1/_unknown_/write_01.wav | _unknown_ | right | right 0.99 | _unknown_ 0.00 | home 0.00 | 19.15 |  |
| clips/supanut/S1/_unknown_/sick_01.wav | _unknown_ | pick | pick 0.82 | _unknown_ 0.08 | station 0.02 | 16.91 |  |
| clips/supanut/S1/_unknown_/hope_01.wav | _unknown_ | home | home 0.43 | move 0.24 | forward 0.12 | 19.75 |  |
| clips/supanut/S1/_unknown_/grass_01.wav | _unknown_ | drop | drop 0.36 | grab 0.27 | _unknown_ 0.11 | 20.04 |  |
| clips/supanut/S1/_unknown_/bag_01.wav | _unknown_ | left | left 0.48 | backward 0.40 | right 0.02 | 17.07 |  |
| clips/supanut/S1/_unknown_/more_01.wav | _unknown_ | no | no 0.95 | move 0.01 | _unknown_ 0.01 | 21.86 |  |
| clips/supanut/S1/base/base_10.wav | base | place | place 0.41 | base 0.34 | _unknown_ 0.09 | 14.71 | sound longer than 1 s - the clip cuts it |
| clips/supanut/S1/_unknown_/foam_01.wav | _unknown_ | home | home 0.63 | hand 0.14 | _unknown_ 0.06 | 23.57 |  |
| clips/supanut/S1/_unknown_/up_01.wav | _unknown_ | stop | stop 0.37 | drop 0.20 | _unknown_ 0.08 | 17.26 |  |
| clips/supanut/S1/_unknown_/robot_01.wav | _unknown_ | green box | green box 0.71 | blue box 0.07 | _unknown_ 0.06 | 20.02 | sound longer than 1 s - the clip cuts it |
| clips/supanut/S1/_unknown_/ride_01.wav | _unknown_ | right | right 0.55 | grab 0.23 | _unknown_ 0.06 | 23.37 |  |

## Stream mode (raw 2 s takes, board logic: stride 10 frames, avg 3, threshold 0.7, refractory 1500 ms, channel auto)

command takes 200: **detected 0.925** (hit 185, hit+extra 0), wrong 4, miss 11 · first detection correct 0.925 · _unknown_ false alarms 16/30 · noise false alarms 0 in 1.0 min · channels {'ch0': 230}

| cls | takes | hit | hit_extra | wrong | miss | false_alarm | detected_rate | wrong_as | mean_max_p_true |
|---|---|---|---|---|---|---|---|---|---|
| move | 10 | 8 | 0 | 0 | 2 | 0 | 0.8 |  | 0.798 |
| pick | 10 | 10 | 0 | 0 | 0 | 0 | 1.0 |  | 0.976 |
| grab | 10 | 10 | 0 | 0 | 0 | 0 | 1.0 |  | 0.896 |
| blue box | 10 | 10 | 0 | 0 | 0 | 0 | 1.0 |  | 0.915 |
| green box | 10 | 10 | 0 | 0 | 0 | 0 | 1.0 |  | 0.97 |
| red box | 10 | 8 | 0 | 0 | 2 | 0 | 0.8 |  | 0.817 |
| black box | 10 | 5 | 0 | 4 | 1 | 0 | 0.5 | backward 4 | 0.809 |
| hand | 10 | 10 | 0 | 0 | 0 | 0 | 1.0 |  | 0.911 |
| station | 10 | 10 | 0 | 0 | 0 | 0 | 1.0 |  | 0.974 |
| base | 10 | 5 | 0 | 0 | 5 | 0 | 0.5 |  | 0.698 |
| forward | 10 | 10 | 0 | 0 | 0 | 0 | 1.0 |  | 0.952 |
| backward | 10 | 10 | 0 | 0 | 0 | 0 | 1.0 |  | 0.994 |
| left | 10 | 10 | 0 | 0 | 0 | 0 | 1.0 |  | 0.98 |
| right | 10 | 10 | 0 | 0 | 0 | 0 | 1.0 |  | 0.994 |
| place | 10 | 10 | 0 | 0 | 0 | 0 | 1.0 |  | 0.946 |
| drop | 10 | 10 | 0 | 0 | 0 | 0 | 1.0 |  | 0.963 |
| yes | 10 | 10 | 0 | 0 | 0 | 0 | 1.0 |  | 0.981 |
| no | 10 | 10 | 0 | 0 | 0 | 0 | 1.0 |  | 0.981 |
| home | 10 | 9 | 0 | 0 | 1 | 0 | 0.9 |  | 0.884 |
| stop | 10 | 10 | 0 | 0 | 0 | 0 | 1.0 |  | 0.986 |
| _unknown_ | 30 | 0 | 0 | 0 | 0 | 16 |  | home 4, no 4, right 4 | 0.104 |

### Takes that were not a clean hit

| raw_path | label | outcome | detections | max_avg_p_true | peak_top3 | channels_used |
|---|---|---|---|---|---|---|
| raw/supanut/S1/_unknown_/arm_01_raw.wav | _unknown_ | false_alarm | home@1.11s 0.74 | 0.132 | home 0.90 | _unknown_ 0.03 | no 0.02 | ch0 |
| raw/supanut/S1/base/base_01_raw.wav | base | miss |  | 0.684 | base 0.68 | place 0.26 | move 0.01 | ch0 |
| raw/supanut/S1/black_box/black_box_01_raw.wav | black box | wrong | backward@1.01s 0.74 | 0.93 | black box 0.93 | backward 0.03 | _unknown_ 0.01 | ch0 |
| raw/supanut/S1/black_box/black_box_02_raw.wav | black box | wrong | backward@1.01s 0.71 | 0.899 | black box 0.90 | base 0.01 | yes 0.01 | ch0 |
| raw/supanut/S1/_unknown_/nose_01_raw.wav | _unknown_ | false_alarm | no@1.01s 0.96 | 0.142 | no 0.96 | _unknown_ 0.01 | move 0.01 | ch0 |
| raw/supanut/S1/black_box/black_box_03_raw.wav | black box | wrong | backward@1.01s 0.73 | 0.8 | black box 0.80 | red box 0.04 | backward 0.02 | ch0 |
| raw/supanut/S1/base/base_04_raw.wav | base | miss |  | 0.592 | base 0.59 | place 0.29 | yes 0.05 | ch0 |
| raw/supanut/S1/black_box/black_box_05_raw.wav | black box | miss |  | 0.686 | black box 0.69 | blue box 0.04 | left 0.03 | ch0 |
| raw/supanut/S1/red_box/red_box_05_raw.wav | red box | miss |  | 0.638 | red box 0.64 | green box 0.19 | _unknown_ 0.04 | ch0 |
| raw/supanut/S1/black_box/black_box_06_raw.wav | black box | wrong | backward@1.01s 0.74 | 0.699 | backward 0.74 | left 0.08 | _unknown_ 0.04 | ch0 |
| raw/supanut/S1/_unknown_/now_01_raw.wav | _unknown_ | false_alarm | no@1.01s 0.97 | 0.026 | no 0.97 | _unknown_ 0.01 | yes 0.00 | ch0 |
| raw/supanut/S1/move/move_05_raw.wav | move | miss |  | 0.545 | move 0.55 | no 0.12 | home 0.12 | ch0 |
| raw/supanut/S1/_unknown_/pink_01_raw.wav | _unknown_ | false_alarm | pick@1.01s 0.89 | 0.128 | pick 0.89 | _unknown_ 0.04 | hand 0.01 | ch0 |
| raw/supanut/S1/_unknown_/bright_01_raw.wav | _unknown_ | false_alarm | right@1.11s 0.72 | 0.031 | right 0.99 | _unknown_ 0.00 | backward 0.00 | ch0 |
| raw/supanut/S1/red_box/red_box_06_raw.wav | red box | miss |  | 0.553 | right 0.68 | _unknown_ 0.15 | grab 0.03 | ch0 |
| raw/supanut/S1/_unknown_/back_01_raw.wav | _unknown_ | false_alarm | left@1.11s 0.75 | 0.03 | left 0.93 | backward 0.04 | _unknown_ 0.01 | ch0 |
| raw/supanut/S1/home/home_02_raw.wav | home | miss |  | 0.275 | home 0.27 | hand 0.25 | pick 0.10 | ch0 |
| raw/supanut/S1/_unknown_/grape_01_raw.wav | _unknown_ | false_alarm | right@1.01s 0.89 | 0.107 | right 0.91 | _unknown_ 0.04 | pick 0.01 | ch0 |
| raw/supanut/S1/_unknown_/black_01_raw.wav | _unknown_ | false_alarm | left@1.01s 0.89 | 0.008 | left 0.93 | yes 0.03 | _unknown_ 0.01 | ch0 |
| raw/supanut/S1/base/base_07_raw.wav | base | miss |  | 0.407 | base 0.41 | yes 0.24 | place 0.15 | ch0 |
| raw/supanut/S1/_unknown_/blue_01_raw.wav | _unknown_ | false_alarm | no@1.01s 0.73 | 0.027 | no 0.75 | move 0.14 | _unknown_ 0.02 | ch0 |
| raw/supanut/S1/_unknown_/go_01_raw.wav | _unknown_ | false_alarm | home@1.01s 0.92 | 0.043 | home 0.94 | forward 0.01 | _unknown_ 0.01 | ch0 |
| raw/supanut/S1/_unknown_/write_01_raw.wav | _unknown_ | false_alarm | right@1.01s 0.99 | 0.025 | right 0.99 | _unknown_ 0.00 | home 0.00 | ch0 |
| raw/supanut/S1/_unknown_/sick_01_raw.wav | _unknown_ | false_alarm | pick@1.01s 0.77 | 0.094 | pick 0.90 | _unknown_ 0.03 | right 0.02 | ch0 |
| raw/supanut/S1/_unknown_/hope_01_raw.wav | _unknown_ | false_alarm | home@1.01s 0.75 | 0.033 | home 0.75 | forward 0.06 | move 0.05 | ch0 |
| raw/supanut/S1/base/base_09_raw.wav | base | miss |  | 0.599 | base 0.60 | place 0.30 | yes 0.04 | ch0 |
| raw/supanut/S1/_unknown_/more_01_raw.wav | _unknown_ | false_alarm | no@1.01s 0.95 | 0.036 | no 0.95 | move 0.02 | _unknown_ 0.01 | ch0 |
| raw/supanut/S1/move/move_08_raw.wav | move | miss |  | 0.659 | move 0.66 | no 0.22 | _unknown_ 0.02 | ch0 |
| raw/supanut/S1/base/base_10_raw.wav | base | miss |  | 0.509 | base 0.51 | place 0.30 | yes 0.05 | ch0 |
| raw/supanut/S1/_unknown_/foam_01_raw.wav | _unknown_ | false_alarm | home@1.01s 0.78 | 0.043 | home 0.78 | hand 0.05 | _unknown_ 0.04 | ch0 |
| raw/supanut/S1/_unknown_/ride_01_raw.wav | _unknown_ | false_alarm | right@1.31s 0.71 | 0.068 | right 0.77 | _unknown_ 0.06 | grab 0.05 | ch0 |

### Noise recordings

| file | duration_s | inferences | false_alarms | top_classes | detections |
|---|---|---|---|---|---|
| noise\room\supanut_S1_room.wav | 60.0 | 590 | 0 | _silence_ 590 |  |
