# Board evaluation - exp05_tcresnet-w24d2_aug-all on S1

model `D:\Microcon69\ProjectRoboticArm\ProjectSpace\2026-micro-con\voice_command_research\workspace\experiments\exp05_tcresnet-w24d2_aug-all\runs\final.pth` (pytorch exp05_tcresnet-w24d2_aug-all, 23 classes) · gain +15.0 dB · features librosa 0.11.0 (features.py) · 2026-10-21 21:04

## Clip mode (1 s clips, mono ch1)

n 290 · **accuracy 0.8759** · **macro-F1 0.8715** · commands 0.96 · _unknown_ 0.0667 · _silence_ 1.0 · commands rejected as unknown/silence 0 · speech peak median -34.7 dB (training clip -25.4 dB)

| cls | support | correct | recall | precision | f1 | mean_p_true | confused_with |
|---|---|---|---|---|---|---|---|
| move | 10 | 9 | 0.9 | 0.692 | 0.783 | 0.632 | hand 1 |
| pick | 10 | 10 | 1.0 | 0.833 | 0.909 | 0.984 |  |
| grab | 10 | 8 | 0.8 | 0.889 | 0.842 | 0.756 | right 2 |
| blue box | 10 | 10 | 1.0 | 1.0 | 1.0 | 0.814 |  |
| green box | 10 | 10 | 1.0 | 0.909 | 0.952 | 0.951 |  |
| red box | 10 | 9 | 0.9 | 1.0 | 0.947 | 0.661 | green box 1 |
| black box | 10 | 8 | 0.8 | 1.0 | 0.889 | 0.467 | yes 2 |
| hand | 10 | 10 | 1.0 | 0.909 | 0.952 | 0.953 |  |
| station | 10 | 10 | 1.0 | 1.0 | 1.0 | 0.968 |  |
| base | 10 | 8 | 0.8 | 1.0 | 0.889 | 0.676 | place 2 |
| forward | 10 | 10 | 1.0 | 0.909 | 0.952 | 0.886 |  |
| backward | 10 | 10 | 1.0 | 1.0 | 1.0 | 0.995 |  |
| left | 10 | 10 | 1.0 | 0.769 | 0.87 | 0.988 |  |
| right | 10 | 10 | 1.0 | 0.625 | 0.769 | 0.993 |  |
| place | 10 | 10 | 1.0 | 0.833 | 0.909 | 0.919 |  |
| drop | 10 | 10 | 1.0 | 0.909 | 0.952 | 0.901 |  |
| yes | 10 | 10 | 1.0 | 0.833 | 0.909 | 0.986 |  |
| no | 10 | 10 | 1.0 | 0.667 | 0.8 | 0.99 |  |
| home | 10 | 10 | 1.0 | 0.625 | 0.769 | 0.89 |  |
| stop | 10 | 10 | 1.0 | 0.909 | 0.952 | 0.988 |  |
| _unknown_ | 30 | 2 | 0.067 | 1.0 | 0.125 | 0.079 | home 6, no 5, move 4 |
| _silence_ | 60 | 60 | 1.0 | 1.0 | 1.0 | 0.895 |  |

### Mistakes (top-3)

| clip_path | label | pred | top3 | snr_db | flags |
|---|---|---|---|---|---|
| clips/supanut/S1/_unknown_/arm_01.wav | _unknown_ | home | home 0.90 | no 0.05 | _unknown_ 0.02 | 22.45 |  |
| clips/supanut/S1/black_box/black_box_01.wav | black box | yes | yes 0.30 | black box 0.27 | backward 0.21 | 20.72 |  |
| clips/supanut/S1/grab/grab_01.wav | grab | right | right 0.77 | grab 0.15 | left 0.03 | 27.02 |  |
| clips/supanut/S1/_unknown_/moon_01.wav | _unknown_ | move | move 0.43 | home 0.17 | forward 0.15 | 25.84 |  |
| clips/supanut/S1/_unknown_/hold_01.wav | _unknown_ | forward | forward 0.21 | place 0.18 | base 0.14 | 24.63 |  |
| clips/supanut/S1/_unknown_/books_01.wav | _unknown_ | move | move 0.70 | blue box 0.07 | left 0.04 | 23.75 |  |
| clips/supanut/S1/_unknown_/close_01.wav | _unknown_ | no | no 0.28 | _unknown_ 0.19 | red box 0.16 | 17.22 |  |
| clips/supanut/S1/grab/grab_03.wav | grab | right | right 0.44 | grab 0.41 | left 0.06 | 24.47 |  |
| clips/supanut/S1/_unknown_/nose_01.wav | _unknown_ | no | no 0.94 | move 0.01 | yes 0.01 | 21.15 |  |
| clips/supanut/S1/black_box/black_box_03.wav | black box | yes | yes 0.37 | black box 0.17 | no 0.11 | 20.67 |  |
| clips/supanut/S1/_unknown_/groove_01.wav | _unknown_ | move | move 0.50 | forward 0.20 | home 0.14 | 25.92 |  |
| clips/supanut/S1/_unknown_/wait_01.wav | _unknown_ | right | right 0.49 | place 0.41 | _unknown_ 0.04 | 19.04 |  |
| clips/supanut/S1/_unknown_/now_01.wav | _unknown_ | no | no 0.86 | left 0.03 | _unknown_ 0.02 | 19.3 |  |
| clips/supanut/S1/_unknown_/pink_01.wav | _unknown_ | pick | pick 0.69 | _unknown_ 0.15 | move 0.06 | 13.71 |  |
| clips/supanut/S1/_unknown_/bright_01.wav | _unknown_ | right | right 1.00 | left 0.00 | _unknown_ 0.00 | 20.23 |  |
| clips/supanut/S1/red_box/red_box_06.wav | red box | green box | green box 0.77 | red box 0.10 | base 0.03 | 20.8 |  |
| clips/supanut/S1/_unknown_/back_01.wav | _unknown_ | left | left 0.67 | backward 0.14 | pick 0.05 | 18.02 |  |
| clips/supanut/S1/_unknown_/grape_01.wav | _unknown_ | right | right 0.98 | _unknown_ 0.01 | place 0.00 | 26.07 |  |
| clips/supanut/S1/_unknown_/phone_01.wav | _unknown_ | home | home 0.55 | forward 0.28 | _unknown_ 0.08 | 21.41 |  |
| clips/supanut/S1/_unknown_/black_01.wav | _unknown_ | left | left 0.88 | yes 0.04 | backward 0.02 | 18.92 |  |
| clips/supanut/S1/_unknown_/box_01.wav | _unknown_ | no | no 0.26 | black box 0.21 | blue box 0.17 | 20.34 |  |
| clips/supanut/S1/_unknown_/mood_01.wav | _unknown_ | home | home 0.48 | move 0.20 | forward 0.08 | 20.3 |  |
| clips/supanut/S1/_unknown_/blue_01.wav | _unknown_ | move | move 0.50 | no 0.46 | blue box 0.01 | 21.44 |  |
| clips/supanut/S1/_unknown_/go_01.wav | _unknown_ | home | home 0.85 | forward 0.04 | _unknown_ 0.03 | 24.55 |  |
| clips/supanut/S1/_unknown_/write_01.wav | _unknown_ | right | right 0.98 | grab 0.00 | _unknown_ 0.00 | 19.15 |  |
| clips/supanut/S1/_unknown_/sick_01.wav | _unknown_ | pick | pick 0.49 | _unknown_ 0.24 | station 0.08 | 16.91 |  |
| clips/supanut/S1/_unknown_/hope_01.wav | _unknown_ | home | home 0.58 | move 0.17 | forward 0.11 | 19.75 |  |
| clips/supanut/S1/_unknown_/grass_01.wav | _unknown_ | drop | drop 0.52 | grab 0.16 | right 0.06 | 20.04 |  |
| clips/supanut/S1/_unknown_/bag_01.wav | _unknown_ | left | left 0.58 | right 0.17 | backward 0.12 | 17.07 |  |
| clips/supanut/S1/base/base_09.wav | base | place | place 0.70 | base 0.25 | yes 0.02 | 16.52 |  |
| clips/supanut/S1/_unknown_/more_01.wav | _unknown_ | no | no 0.74 | move 0.20 | blue box 0.02 | 21.86 |  |
| clips/supanut/S1/base/base_10.wav | base | place | place 0.49 | base 0.41 | move 0.04 | 14.71 | sound longer than 1 s - the clip cuts it |
| clips/supanut/S1/_unknown_/foam_01.wav | _unknown_ | home | home 0.79 | forward 0.07 | hand 0.06 | 23.57 |  |
| clips/supanut/S1/_unknown_/up_01.wav | _unknown_ | stop | stop 0.49 | left 0.10 | drop 0.08 | 17.26 |  |
| clips/supanut/S1/_unknown_/ride_01.wav | _unknown_ | grab | grab 0.42 | right 0.40 | drop 0.10 | 23.37 |  |
| clips/supanut/S1/move/move_09.wav | move | hand | hand 0.41 | home 0.28 | move 0.13 | 17.11 | sound longer than 1 s - the clip cuts it |

## Stream mode (raw 2 s takes, board logic: stride 10 frames, avg 3, threshold 0.7, refractory 1500 ms, channel auto)

command takes 200: **detected 0.835** (hit 167, hit+extra 0), wrong 6, miss 27 · first detection correct 0.835 · _unknown_ false alarms 18/30 · noise false alarms 0 in 1.0 min · channels {'ch0': 230}

| cls | takes | hit | hit_extra | wrong | miss | false_alarm | detected_rate | wrong_as | mean_max_p_true |
|---|---|---|---|---|---|---|---|---|---|
| move | 10 | 5 | 0 | 0 | 5 | 0 | 0.5 |  | 0.695 |
| pick | 10 | 10 | 0 | 0 | 0 | 0 | 1.0 |  | 0.985 |
| grab | 10 | 8 | 0 | 0 | 2 | 0 | 0.8 |  | 0.796 |
| blue box | 10 | 8 | 0 | 0 | 2 | 0 | 0.8 |  | 0.812 |
| green box | 10 | 10 | 0 | 0 | 0 | 0 | 1.0 |  | 0.964 |
| red box | 10 | 5 | 0 | 1 | 4 | 0 | 0.5 | right 1 | 0.65 |
| black box | 10 | 0 | 0 | 1 | 9 | 0 | 0.0 | backward 1 | 0.376 |
| hand | 10 | 10 | 0 | 0 | 0 | 0 | 1.0 |  | 0.968 |
| station | 10 | 6 | 0 | 4 | 0 | 0 | 0.6 | base 4 | 0.961 |
| base | 10 | 7 | 0 | 0 | 3 | 0 | 0.7 |  | 0.758 |
| forward | 10 | 10 | 0 | 0 | 0 | 0 | 1.0 |  | 0.905 |
| backward | 10 | 10 | 0 | 0 | 0 | 0 | 1.0 |  | 0.996 |
| left | 10 | 10 | 0 | 0 | 0 | 0 | 1.0 |  | 0.988 |
| right | 10 | 10 | 0 | 0 | 0 | 0 | 1.0 |  | 0.995 |
| place | 10 | 9 | 0 | 0 | 1 | 0 | 0.9 |  | 0.929 |
| drop | 10 | 10 | 0 | 0 | 0 | 0 | 1.0 |  | 0.89 |
| yes | 10 | 10 | 0 | 0 | 0 | 0 | 1.0 |  | 0.987 |
| no | 10 | 10 | 0 | 0 | 0 | 0 | 1.0 |  | 0.993 |
| home | 10 | 9 | 0 | 0 | 1 | 0 | 0.9 |  | 0.864 |
| stop | 10 | 10 | 0 | 0 | 0 | 0 | 1.0 |  | 0.984 |
| _unknown_ | 30 | 0 | 0 | 0 | 0 | 18 |  | home 4, no 4, right 4 | 0.082 |

### Takes that were not a clean hit

| raw_path | label | outcome | detections | max_avg_p_true | peak_top3 | channels_used |
|---|---|---|---|---|---|---|
| raw/supanut/S1/_unknown_/arm_01_raw.wav | _unknown_ | false_alarm | home@1.11s 0.77 | 0.05 | home 0.87 | no 0.08 | _unknown_ 0.02 | ch0 |
| raw/supanut/S1/base/base_01_raw.wav | base | miss |  | 0.423 | right 0.66 | place 0.09 | base 0.07 | ch0 |
| raw/supanut/S1/black_box/black_box_01_raw.wav | black box | miss |  | 0.405 | left 0.57 | backward 0.24 | black box 0.04 | ch0 |
| raw/supanut/S1/station/station_02_raw.wav | station | wrong | base@1.01s 0.71 | 0.991 | station 0.99 | base 0.00 | _unknown_ 0.00 | ch0 |
| raw/supanut/S1/black_box/black_box_02_raw.wav | black box | wrong | backward@1.01s 0.72 | 0.501 | backward 0.72 | black box 0.07 | left 0.06 | ch0 |
| raw/supanut/S1/red_box/red_box_01_raw.wav | red box | miss |  | 0.664 | red box 0.66 | green box 0.17 | base 0.03 | ch0 |
| raw/supanut/S1/grab/grab_01_raw.wav | grab | miss |  | 0.539 | right 0.60 | grab 0.28 | left 0.05 | ch0 |
| raw/supanut/S1/_unknown_/books_01_raw.wav | _unknown_ | false_alarm | move@1.01s 0.76 | 0.034 | move 0.76 | hand 0.04 | left 0.04 | ch0 |
| raw/supanut/S1/_unknown_/close_01_raw.wav | _unknown_ | false_alarm | no@1.01s 0.74 | 0.06 | no 0.74 | stop 0.14 | _unknown_ 0.02 | ch0 |
| raw/supanut/S1/grab/grab_03_raw.wav | grab | miss |  | 0.404 | right 0.59 | grab 0.30 | left 0.03 | ch0 |
| raw/supanut/S1/_unknown_/nose_01_raw.wav | _unknown_ | false_alarm | no@1.01s 0.96 | 0.045 | no 0.96 | move 0.02 | _unknown_ 0.00 | ch0 |
| raw/supanut/S1/black_box/black_box_03_raw.wav | black box | miss |  | 0.209 | backward 0.31 | no 0.16 | yes 0.10 | ch0 |
| raw/supanut/S1/red_box/red_box_03_raw.wav | red box | wrong | right@1.01s 0.71 | 0.458 | right 0.71 | _unknown_ 0.06 | base 0.04 | ch0 |
| raw/supanut/S1/black_box/black_box_04_raw.wav | black box | miss |  | 0.439 | left 0.63 | backward 0.15 | black box 0.08 | ch0 |
| raw/supanut/S1/station/station_03_raw.wav | station | wrong | base@1.01s 0.93 | 0.984 | station 0.98 | base 0.01 | _unknown_ 0.00 | ch0 |
| raw/supanut/S1/black_box/black_box_05_raw.wav | black box | miss |  | 0.224 | left 0.40 | no 0.15 | black box 0.11 | ch0 |
| raw/supanut/S1/red_box/red_box_05_raw.wav | red box | miss |  | 0.681 | red box 0.68 | green box 0.10 | yes 0.05 | ch0 |
| raw/supanut/S1/black_box/black_box_06_raw.wav | black box | miss |  | 0.159 | left 0.63 | backward 0.11 | yes 0.05 | ch0 |
| raw/supanut/S1/_unknown_/now_01_raw.wav | _unknown_ | false_alarm | no@1.01s 0.89 | 0.028 | no 0.93 | _unknown_ 0.01 | move 0.01 | ch0 |
| raw/supanut/S1/move/move_05_raw.wav | move | miss |  | 0.255 | home 0.31 | hand 0.27 | move 0.22 | ch0 |
| raw/supanut/S1/_unknown_/bright_01_raw.wav | _unknown_ | false_alarm | right@1.01s 0.73 | 0.004 | right 1.00 | left 0.00 | _unknown_ 0.00 | ch0 |
| raw/supanut/S1/red_box/red_box_06_raw.wav | red box | miss |  | 0.279 | green box 0.55 | red box 0.26 | _unknown_ 0.04 | ch0 |
| raw/supanut/S1/_unknown_/back_01_raw.wav | _unknown_ | false_alarm | left@1.01s 0.73 | 0.014 | left 0.86 | backward 0.07 | pick 0.01 | ch0 |
| raw/supanut/S1/place/place_06_raw.wav | place | miss |  | 0.615 | place 0.61 | base 0.35 | move 0.01 | ch0 |
| raw/supanut/S1/home/home_02_raw.wav | home | miss |  | 0.283 | home 0.28 | left 0.14 | place 0.12 | ch0 |
| raw/supanut/S1/station/station_04_raw.wav | station | wrong | base@1.01s 0.86 | 0.972 | station 0.97 | base 0.01 | _unknown_ 0.00 | ch0 |
| raw/supanut/S1/_unknown_/grape_01_raw.wav | _unknown_ | false_alarm | right@1.01s 0.97 | 0.014 | right 0.97 | _unknown_ 0.01 | place 0.00 | ch0 |
| raw/supanut/S1/_unknown_/phone_01_raw.wav | _unknown_ | false_alarm | home@1.01s 0.75 | 0.048 | home 0.75 | forward 0.12 | hand 0.05 | ch0 |
| raw/supanut/S1/_unknown_/black_01_raw.wav | _unknown_ | false_alarm | left@1.01s 0.88 | 0.007 | left 0.88 | yes 0.07 | no 0.01 | ch0 |
| raw/supanut/S1/station/station_05_raw.wav | station | wrong | base@1.01s 0.95 | 0.895 | base 0.95 | place 0.03 | station 0.01 | ch0 |
| raw/supanut/S1/black_box/black_box_07_raw.wav | black box | miss |  | 0.433 | black box 0.43 | left 0.15 | yes 0.10 | ch0 |
| raw/supanut/S1/red_box/red_box_07_raw.wav | red box | miss |  | 0.327 | green box 0.65 | red box 0.22 | blue box 0.02 | ch0 |
| raw/supanut/S1/_unknown_/blue_01_raw.wav | _unknown_ | false_alarm | move@1.01s 0.78 | 0.024 | move 0.78 | no 0.19 | blue box 0.01 | ch0 |
| raw/supanut/S1/move/move_06_raw.wav | move | miss |  | 0.624 | move 0.62 | home 0.18 | hand 0.04 | ch0 |
| raw/supanut/S1/move/move_07_raw.wav | move | miss |  | 0.666 | move 0.67 | hand 0.10 | no 0.05 | ch0 |
| raw/supanut/S1/blue_box/blue_box_07_raw.wav | blue box | miss |  | 0.663 | blue box 0.66 | no 0.17 | move 0.13 | ch0 |
| raw/supanut/S1/_unknown_/go_01_raw.wav | _unknown_ | false_alarm | home@1.01s 0.86 | 0.026 | home 0.88 | forward 0.02 | _unknown_ 0.02 | ch0 |
| raw/supanut/S1/_unknown_/write_01_raw.wav | _unknown_ | false_alarm | right@1.01s 0.99 | 0.021 | right 0.99 | _unknown_ 0.00 | grab 0.00 | ch0 |
| raw/supanut/S1/_unknown_/sick_01_raw.wav | _unknown_ | false_alarm | pick@1.61s 0.75 | 0.191 | pick 0.81 | _unknown_ 0.05 | right 0.04 | ch0 |
| raw/supanut/S1/blue_box/blue_box_08_raw.wav | blue box | miss |  | 0.69 | blue box 0.69 | move 0.16 | no 0.09 | ch0 |
| raw/supanut/S1/_unknown_/open_01_raw.wav | _unknown_ | false_alarm | pick@1.81s 0.83 | 0.804 | pick 0.83 | _unknown_ 0.03 | hand 0.03 | ch0 |
| raw/supanut/S1/black_box/black_box_08_raw.wav | black box | miss |  | 0.601 | black box 0.60 | yes 0.13 | left 0.09 | ch0 |
| raw/supanut/S1/base/base_09_raw.wav | base | miss |  | 0.647 | base 0.65 | place 0.29 | yes 0.02 | ch0 |
| raw/supanut/S1/black_box/black_box_09_raw.wav | black box | miss |  | 0.556 | black box 0.56 | backward 0.14 | yes 0.10 | ch0 |
| raw/supanut/S1/_unknown_/more_01_raw.wav | _unknown_ | false_alarm | no@1.01s 0.81 | 0.041 | no 0.82 | move 0.09 | _unknown_ 0.02 | ch0 |
| raw/supanut/S1/base/base_10_raw.wav | base | miss |  | 0.644 | base 0.64 | place 0.25 | yes 0.04 | ch0 |
| raw/supanut/S1/_unknown_/foam_01_raw.wav | _unknown_ | false_alarm | home@1.01s 0.82 | 0.036 | home 0.84 | forward 0.04 | hand 0.04 | ch0 |
| raw/supanut/S1/_unknown_/ride_01_raw.wav | _unknown_ | false_alarm | right@1.01s 0.73 | 0.049 | right 0.88 | grab 0.07 | drop 0.03 | ch0 |
| raw/supanut/S1/move/move_09_raw.wav | move | miss |  | 0.513 | move 0.51 | hand 0.13 | home 0.13 | ch0 |
| raw/supanut/S1/move/move_10_raw.wav | move | miss |  | 0.652 | move 0.65 | hand 0.09 | no 0.05 | ch0 |
| raw/supanut/S1/black_box/black_box_10_raw.wav | black box | miss |  | 0.237 | yes 0.29 | black box 0.23 | no 0.08 | ch0 |

### Noise recordings

| file | duration_s | inferences | false_alarms | top_classes | detections |
|---|---|---|---|---|---|
| noise\room\supanut_S1_room.wav | 60.0 | 590 | 0 | _silence_ 590 |  |
