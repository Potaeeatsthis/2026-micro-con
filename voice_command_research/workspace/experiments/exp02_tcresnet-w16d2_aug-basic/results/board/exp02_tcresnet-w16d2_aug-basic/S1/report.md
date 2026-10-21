# Board evaluation - exp02_tcresnet-w16d2_aug-basic on S1

model `D:\Microcon69\ProjectRoboticArm\ProjectSpace\2026-micro-con\voice_command_research\workspace\experiments\exp02_tcresnet-w16d2_aug-basic\runs\final.pth` (pytorch TC-ResNet w16 d2, 23 classes) · gain +15.0 dB · features librosa 0.11.0 (features.py) · 2026-10-21 19:26

## Clip mode (1 s clips, mono ch1)

n 290 · **accuracy 0.8414** · **macro-F1 0.8245** · commands 0.915 · _unknown_ 0.0333 · _silence_ 1.0 · commands rejected as unknown/silence 0 · speech peak median -34.7 dB (training clip -25.4 dB)

| cls | support | correct | recall | precision | f1 | mean_p_true | confused_with |
|---|---|---|---|---|---|---|---|
| move | 10 | 6 | 0.6 | 0.857 | 0.706 | 0.453 | hand 4 |
| pick | 10 | 10 | 1.0 | 0.833 | 0.909 | 0.937 |  |
| grab | 10 | 8 | 0.8 | 0.8 | 0.8 | 0.664 | right 2 |
| blue box | 10 | 10 | 1.0 | 1.0 | 1.0 | 0.864 |  |
| green box | 10 | 10 | 1.0 | 0.714 | 0.833 | 0.879 |  |
| red box | 10 | 7 | 0.7 | 0.778 | 0.737 | 0.514 | green box 3 |
| black box | 10 | 6 | 0.6 | 1.0 | 0.75 | 0.33 | yes 4 |
| hand | 10 | 10 | 1.0 | 0.714 | 0.833 | 0.921 |  |
| station | 10 | 10 | 1.0 | 1.0 | 1.0 | 0.967 |  |
| base | 10 | 10 | 1.0 | 0.769 | 0.87 | 0.727 |  |
| forward | 10 | 10 | 1.0 | 0.769 | 0.87 | 0.938 |  |
| backward | 10 | 10 | 1.0 | 1.0 | 1.0 | 0.998 |  |
| left | 10 | 10 | 1.0 | 0.769 | 0.87 | 0.933 |  |
| right | 10 | 10 | 1.0 | 0.588 | 0.741 | 0.982 |  |
| place | 10 | 7 | 0.7 | 1.0 | 0.824 | 0.515 | base 3 |
| drop | 10 | 9 | 0.9 | 1.0 | 0.947 | 0.571 | grab 1 |
| yes | 10 | 10 | 1.0 | 0.714 | 0.833 | 0.966 |  |
| no | 10 | 10 | 1.0 | 0.667 | 0.8 | 0.985 |  |
| home | 10 | 10 | 1.0 | 0.667 | 0.8 | 0.838 |  |
| stop | 10 | 10 | 1.0 | 0.909 | 0.952 | 0.986 |  |
| _unknown_ | 30 | 1 | 0.033 | 1.0 | 0.065 | 0.062 | right 5, no 5, home 5 |
| _silence_ | 60 | 60 | 1.0 | 1.0 | 1.0 | 0.895 |  |

### Mistakes (top-3)

| clip_path | label | pred | top3 | snr_db | flags |
|---|---|---|---|---|---|
| clips/supanut/S1/_unknown_/arm_01.wav | _unknown_ | home | home 0.90 | _unknown_ 0.03 | stop 0.02 | 22.45 |  |
| clips/supanut/S1/black_box/black_box_01.wav | black box | yes | yes 0.63 | black box 0.13 | no 0.11 | 20.72 |  |
| clips/supanut/S1/black_box/black_box_02.wav | black box | yes | yes 0.67 | no 0.13 | black box 0.08 | 21.89 |  |
| clips/supanut/S1/grab/grab_01.wav | grab | right | right 0.33 | left 0.30 | grab 0.29 | 27.02 |  |
| clips/supanut/S1/place/place_03.wav | place | base | base 0.70 | place 0.25 | _unknown_ 0.01 | 21.67 |  |
| clips/supanut/S1/_unknown_/moon_01.wav | _unknown_ | forward | forward 0.45 | home 0.18 | hand 0.13 | 25.84 |  |
| clips/supanut/S1/_unknown_/hold_01.wav | _unknown_ | home | home 0.37 | forward 0.25 | _unknown_ 0.08 | 24.63 |  |
| clips/supanut/S1/_unknown_/books_01.wav | _unknown_ | red box | red box 0.18 | hand 0.12 | base 0.12 | 23.75 |  |
| clips/supanut/S1/_unknown_/close_01.wav | _unknown_ | no | no 0.38 | yes 0.33 | _unknown_ 0.06 | 17.22 |  |
| clips/supanut/S1/grab/grab_03.wav | grab | right | right 0.32 | grab 0.31 | left 0.22 | 24.47 |  |
| clips/supanut/S1/_unknown_/nose_01.wav | _unknown_ | no | no 0.94 | yes 0.02 | _unknown_ 0.01 | 21.15 |  |
| clips/supanut/S1/black_box/black_box_03.wav | black box | yes | yes 0.52 | black box 0.19 | no 0.09 | 20.67 |  |
| clips/supanut/S1/_unknown_/groove_01.wav | _unknown_ | move | move 0.27 | forward 0.25 | home 0.15 | 25.92 |  |
| clips/supanut/S1/red_box/red_box_05.wav | red box | green box | green box 0.30 | red box 0.22 | base 0.12 | 21.46 |  |
| clips/supanut/S1/black_box/black_box_06.wav | black box | yes | yes 0.37 | no 0.21 | black box 0.12 | 20.09 |  |
| clips/supanut/S1/move/move_04.wav | move | hand | hand 0.45 | move 0.15 | no 0.09 | 22.97 |  |
| clips/supanut/S1/_unknown_/wait_01.wav | _unknown_ | right | right 0.80 | place 0.09 | _unknown_ 0.04 | 19.04 |  |
| clips/supanut/S1/_unknown_/now_01.wav | _unknown_ | no | no 0.97 | move 0.01 | _unknown_ 0.01 | 19.3 |  |
| clips/supanut/S1/move/move_05.wav | move | hand | hand 0.31 | move 0.23 | home 0.13 | 18.8 |  |
| clips/supanut/S1/_unknown_/pink_01.wav | _unknown_ | pick | pick 0.26 | hand 0.18 | _unknown_ 0.16 | 13.71 |  |
| clips/supanut/S1/_unknown_/bright_01.wav | _unknown_ | right | right 0.99 | grab 0.01 | _unknown_ 0.00 | 20.23 |  |
| clips/supanut/S1/red_box/red_box_06.wav | red box | green box | green box 0.65 | red box 0.17 | _unknown_ 0.05 | 20.8 |  |
| clips/supanut/S1/_unknown_/back_01.wav | _unknown_ | left | left 0.80 | backward 0.15 | pick 0.01 | 18.02 |  |
| clips/supanut/S1/place/place_06.wav | place | base | base 0.59 | place 0.35 | _unknown_ 0.01 | 23.77 |  |
| clips/supanut/S1/_unknown_/grape_01.wav | _unknown_ | right | right 0.96 | _unknown_ 0.02 | place 0.01 | 26.07 |  |
| clips/supanut/S1/_unknown_/phone_01.wav | _unknown_ | forward | forward 0.46 | hand 0.21 | home 0.11 | 21.41 |  |
| clips/supanut/S1/_unknown_/black_01.wav | _unknown_ | left | left 0.90 | backward 0.03 | yes 0.03 | 18.92 |  |
| clips/supanut/S1/place/place_08.wav | place | base | base 0.90 | place 0.06 | _unknown_ 0.01 | 25.62 |  |
| clips/supanut/S1/_unknown_/box_01.wav | _unknown_ | red box | red box 0.47 | blue box 0.17 | black box 0.09 | 20.34 |  |
| clips/supanut/S1/red_box/red_box_07.wav | red box | green box | green box 0.58 | red box 0.16 | base 0.07 | 20.62 |  |
| clips/supanut/S1/_unknown_/mood_01.wav | _unknown_ | home | home 0.24 | hand 0.18 | _unknown_ 0.15 | 20.3 |  |
| clips/supanut/S1/_unknown_/blue_01.wav | _unknown_ | no | no 0.82 | move 0.12 | _unknown_ 0.01 | 21.44 |  |
| clips/supanut/S1/move/move_07.wav | move | hand | hand 0.62 | home 0.19 | move 0.05 | 23.14 |  |
| clips/supanut/S1/_unknown_/go_01.wav | _unknown_ | home | home 0.54 | no 0.10 | _unknown_ 0.08 | 24.55 |  |
| clips/supanut/S1/_unknown_/write_01.wav | _unknown_ | right | right 0.98 | grab 0.01 | _unknown_ 0.00 | 19.15 |  |
| clips/supanut/S1/_unknown_/sick_01.wav | _unknown_ | pick | pick 0.34 | left 0.19 | station 0.14 | 16.91 |  |
| clips/supanut/S1/drop/drop_08.wav | drop | grab | grab 0.28 | red box 0.20 | drop 0.08 | 19.3 |  |
| clips/supanut/S1/_unknown_/hope_01.wav | _unknown_ | home | home 0.52 | forward 0.12 | _unknown_ 0.08 | 19.75 |  |
| clips/supanut/S1/_unknown_/grass_01.wav | _unknown_ | grab | grab 0.37 | drop 0.24 | red box 0.15 | 20.04 |  |
| clips/supanut/S1/_unknown_/bag_01.wav | _unknown_ | left | left 0.80 | backward 0.07 | pick 0.02 | 17.07 |  |
| clips/supanut/S1/_unknown_/more_01.wav | _unknown_ | no | no 0.96 | move 0.02 | _unknown_ 0.01 | 21.86 |  |
| clips/supanut/S1/_unknown_/foam_01.wav | _unknown_ | forward | forward 0.38 | home 0.16 | hand 0.14 | 23.57 |  |
| clips/supanut/S1/_unknown_/up_01.wav | _unknown_ | stop | stop 0.47 | left 0.13 | drop 0.12 | 17.26 |  |
| clips/supanut/S1/_unknown_/robot_01.wav | _unknown_ | green box | green box 0.50 | blue box 0.16 | _unknown_ 0.09 | 20.02 | sound longer than 1 s - the clip cuts it |
| clips/supanut/S1/_unknown_/ride_01.wav | _unknown_ | right | right 0.89 | grab 0.05 | _unknown_ 0.02 | 23.37 |  |
| clips/supanut/S1/move/move_09.wav | move | hand | hand 0.51 | home 0.14 | _unknown_ 0.08 | 17.11 | sound longer than 1 s - the clip cuts it |

## Stream mode (raw 2 s takes, board logic: stride 10 frames, avg 3, threshold 0.7, refractory 1500 ms, channel auto)

command takes 200: **detected 0.77** (hit 154, hit+extra 0), wrong 7, miss 39 · first detection correct 0.77 · _unknown_ false alarms 13/30 · noise false alarms 0 in 1.0 min · channels {'ch0': 230}

| cls | takes | hit | hit_extra | wrong | miss | false_alarm | detected_rate | wrong_as | mean_max_p_true |
|---|---|---|---|---|---|---|---|---|---|
| move | 10 | 3 | 0 | 0 | 7 | 0 | 0.3 |  | 0.494 |
| pick | 10 | 10 | 0 | 0 | 0 | 0 | 1.0 |  | 0.958 |
| grab | 10 | 5 | 0 | 0 | 5 | 0 | 0.5 |  | 0.654 |
| blue box | 10 | 8 | 0 | 2 | 0 | 0 | 0.8 | move 2 | 0.857 |
| green box | 10 | 10 | 0 | 0 | 0 | 0 | 1.0 |  | 0.888 |
| red box | 10 | 4 | 0 | 0 | 6 | 0 | 0.4 |  | 0.629 |
| black box | 10 | 1 | 0 | 0 | 9 | 0 | 0.1 |  | 0.419 |
| hand | 10 | 10 | 0 | 0 | 0 | 0 | 1.0 |  | 0.954 |
| station | 10 | 9 | 0 | 1 | 0 | 0 | 0.9 | base 1 | 0.964 |
| base | 10 | 9 | 0 | 0 | 1 | 0 | 0.9 |  | 0.795 |
| forward | 10 | 10 | 0 | 0 | 0 | 0 | 1.0 |  | 0.91 |
| backward | 10 | 10 | 0 | 0 | 0 | 0 | 1.0 |  | 0.997 |
| left | 10 | 10 | 0 | 0 | 0 | 0 | 1.0 |  | 0.932 |
| right | 10 | 10 | 0 | 0 | 0 | 0 | 1.0 |  | 0.979 |
| place | 10 | 2 | 0 | 4 | 4 | 0 | 0.2 | base 4 | 0.508 |
| drop | 10 | 5 | 0 | 0 | 5 | 0 | 0.5 |  | 0.709 |
| yes | 10 | 10 | 0 | 0 | 0 | 0 | 1.0 |  | 0.969 |
| no | 10 | 10 | 0 | 0 | 0 | 0 | 1.0 |  | 0.989 |
| home | 10 | 8 | 0 | 0 | 2 | 0 | 0.8 |  | 0.791 |
| stop | 10 | 10 | 0 | 0 | 0 | 0 | 1.0 |  | 0.99 |
| _unknown_ | 30 | 0 | 0 | 0 | 0 | 13 |  | right 5, no 4, left 3 | 0.082 |

### Takes that were not a clean hit

| raw_path | label | outcome | detections | max_avg_p_true | peak_top3 | channels_used |
|---|---|---|---|---|---|---|
| raw/supanut/S1/_unknown_/arm_01_raw.wav | _unknown_ | false_alarm | home@1.11s 0.78 | 0.042 | home 0.88 | _unknown_ 0.03 | stop 0.02 | ch0 |
| raw/supanut/S1/base/base_01_raw.wav | base | miss |  | 0.409 | place 0.64 | base 0.32 | move 0.02 | ch0 |
| raw/supanut/S1/move/move_01_raw.wav | move | miss |  | 0.649 | move 0.65 | hand 0.12 | _unknown_ 0.04 | ch0 |
| raw/supanut/S1/black_box/black_box_01_raw.wav | black box | miss |  | 0.42 | backward 0.45 | left 0.19 | no 0.09 | ch0 |
| raw/supanut/S1/black_box/black_box_02_raw.wav | black box | miss |  | 0.226 | yes 0.65 | black box 0.16 | no 0.06 | ch0 |
| raw/supanut/S1/grab/grab_01_raw.wav | grab | miss |  | 0.403 | grab 0.40 | right 0.26 | left 0.22 | ch0 |
| raw/supanut/S1/place/place_02_raw.wav | place | wrong | base@1.81s 0.72 | 0.369 | base 0.72 | place 0.20 | yes 0.01 | ch0 |
| raw/supanut/S1/place/place_03_raw.wav | place | wrong | base@1.71s 0.71 | 0.378 | base 0.71 | place 0.23 | _unknown_ 0.01 | ch0 |
| raw/supanut/S1/drop/drop_02_raw.wav | drop | miss |  | 0.657 | drop 0.66 | grab 0.11 | stop 0.05 | ch0 |
| raw/supanut/S1/place/place_04_raw.wav | place | miss |  | 0.632 | place 0.63 | base 0.30 | move 0.02 | ch0 |
| raw/supanut/S1/drop/drop_03_raw.wav | drop | miss |  | 0.605 | drop 0.60 | grab 0.10 | stop 0.05 | ch0 |
| raw/supanut/S1/_unknown_/close_01_raw.wav | _unknown_ | false_alarm | no@1.01s 0.85 | 0.06 | no 0.85 | yes 0.03 | _unknown_ 0.02 | ch0 |
| raw/supanut/S1/grab/grab_02_raw.wav | grab | miss |  | 0.404 | grab 0.40 | left 0.26 | right 0.22 | ch0 |
| raw/supanut/S1/grab/grab_03_raw.wav | grab | miss |  | 0.337 | left 0.38 | right 0.25 | grab 0.22 | ch0 |
| raw/supanut/S1/_unknown_/nose_01_raw.wav | _unknown_ | false_alarm | no@1.01s 0.98 | 0.034 | no 0.98 | yes 0.01 | _unknown_ 0.00 | ch0 |
| raw/supanut/S1/black_box/black_box_03_raw.wav | black box | miss |  | 0.443 | black box 0.44 | yes 0.20 | red box 0.14 | ch0 |
| raw/supanut/S1/red_box/red_box_03_raw.wav | red box | miss |  | 0.687 | red box 0.69 | green box 0.12 | _unknown_ 0.06 | ch0 |
| raw/supanut/S1/black_box/black_box_04_raw.wav | black box | miss |  | 0.355 | left 0.37 | backward 0.37 | black box 0.06 | ch0 |
| raw/supanut/S1/station/station_03_raw.wav | station | wrong | base@1.01s 0.75 | 0.98 | station 0.98 | base 0.01 | _unknown_ 0.01 | ch0 |
| raw/supanut/S1/black_box/black_box_05_raw.wav | black box | miss |  | 0.296 | backward 0.45 | no 0.14 | left 0.10 | ch0 |
| raw/supanut/S1/red_box/red_box_05_raw.wav | red box | miss |  | 0.398 | yes 0.48 | green box 0.10 | _unknown_ 0.07 | ch0 |
| raw/supanut/S1/black_box/black_box_06_raw.wav | black box | miss |  | 0.19 | yes 0.55 | black box 0.19 | no 0.09 | ch0 |
| raw/supanut/S1/move/move_04_raw.wav | move | miss |  | 0.304 | hand 0.45 | move 0.27 | no 0.06 | ch0 |
| raw/supanut/S1/_unknown_/wait_01_raw.wav | _unknown_ | false_alarm | right@1.01s 0.90 | 0.039 | right 0.91 | place 0.03 | _unknown_ 0.01 | ch0 |
| raw/supanut/S1/_unknown_/now_01_raw.wav | _unknown_ | false_alarm | no@1.01s 0.87 | 0.033 | no 0.95 | move 0.02 | _unknown_ 0.01 | ch0 |
| raw/supanut/S1/move/move_05_raw.wav | move | miss |  | 0.123 | hand 0.51 | home 0.13 | move 0.10 | ch0 |
| raw/supanut/S1/_unknown_/bright_01_raw.wav | _unknown_ | false_alarm | right@1.21s 0.76 | 0.024 | right 0.99 | grab 0.01 | _unknown_ 0.00 | ch0 |
| raw/supanut/S1/red_box/red_box_06_raw.wav | red box | miss |  | 0.304 | green box 0.48 | red box 0.29 | base 0.06 | ch0 |
| raw/supanut/S1/_unknown_/back_01_raw.wav | _unknown_ | false_alarm | left@1.01s 0.89 | 0.008 | left 0.92 | backward 0.06 | black box 0.00 | ch0 |
| raw/supanut/S1/place/place_06_raw.wav | place | wrong | base@1.11s 0.77 | 0.205 | base 0.84 | place 0.13 | _unknown_ 0.01 | ch0 |
| raw/supanut/S1/drop/drop_06_raw.wav | drop | miss |  | 0.698 | drop 0.70 | grab 0.06 | stop 0.03 | ch0 |
| raw/supanut/S1/home/home_02_raw.wav | home | miss |  | 0.233 | home 0.23 | hand 0.22 | right 0.11 | ch0 |
| raw/supanut/S1/_unknown_/grape_01_raw.wav | _unknown_ | false_alarm | right@1.01s 0.74 | 0.123 | right 0.85 | _unknown_ 0.06 | place 0.02 | ch0 |
| raw/supanut/S1/place/place_07_raw.wav | place | miss |  | 0.621 | place 0.62 | base 0.34 | _unknown_ 0.01 | ch0 |
| raw/supanut/S1/_unknown_/black_01_raw.wav | _unknown_ | false_alarm | left@1.01s 0.88 | 0.008 | left 0.90 | yes 0.04 | backward 0.02 | ch0 |
| raw/supanut/S1/place/place_08_raw.wav | place | wrong | base@1.01s 0.85 | 0.139 | base 0.86 | place 0.11 | _unknown_ 0.01 | ch0 |
| raw/supanut/S1/blue_box/blue_box_05_raw.wav | blue box | wrong | move@1.01s 0.73 | 0.914 | blue box 0.91 | move 0.05 | _unknown_ 0.01 | ch0 |
| raw/supanut/S1/red_box/red_box_07_raw.wav | red box | miss |  | 0.392 | green box 0.43 | red box 0.26 | blue box 0.07 | ch0 |
| raw/supanut/S1/home/home_04_raw.wav | home | miss |  | 0.666 | home 0.67 | move 0.15 | hand 0.06 | ch0 |
| raw/supanut/S1/grab/grab_07_raw.wav | grab | miss |  | 0.635 | grab 0.63 | right 0.19 | left 0.09 | ch0 |
| raw/supanut/S1/move/move_06_raw.wav | move | miss |  | 0.269 | home 0.34 | move 0.24 | hand 0.22 | ch0 |
| raw/supanut/S1/move/move_07_raw.wav | move | miss |  | 0.095 | hand 0.68 | move 0.08 | home 0.07 | ch0 |
| raw/supanut/S1/_unknown_/write_01_raw.wav | _unknown_ | false_alarm | right@1.01s 0.98 | 0.016 | right 0.99 | grab 0.01 | _unknown_ 0.00 | ch0 |
| raw/supanut/S1/red_box/red_box_08_raw.wav | red box | miss |  | 0.692 | red box 0.69 | green box 0.20 | blue box 0.02 | ch0 |
| raw/supanut/S1/blue_box/blue_box_08_raw.wav | blue box | wrong | move@1.01s 0.79 | 0.852 | blue box 0.85 | move 0.11 | _unknown_ 0.01 | ch0 |
| raw/supanut/S1/drop/drop_08_raw.wav | drop | miss |  | 0.367 | drop 0.37 | grab 0.11 | _silence_ 0.08 | ch0 |
| raw/supanut/S1/black_box/black_box_08_raw.wav | black box | miss |  | 0.501 | black box 0.50 | yes 0.28 | red box 0.05 | ch0 |
| raw/supanut/S1/grab/grab_09_raw.wav | grab | miss |  | 0.657 | grab 0.66 | right 0.13 | left 0.07 | ch0 |
| raw/supanut/S1/_unknown_/bag_01_raw.wav | _unknown_ | false_alarm | left@1.01s 0.85 | 0.021 | left 0.85 | backward 0.04 | pick 0.02 | ch0 |
| raw/supanut/S1/black_box/black_box_09_raw.wav | black box | miss |  | 0.619 | black box 0.62 | red box 0.16 | yes 0.10 | ch0 |
| raw/supanut/S1/place/place_09_raw.wav | place | miss |  | 0.687 | place 0.69 | base 0.27 | _unknown_ 0.01 | ch0 |
| raw/supanut/S1/_unknown_/more_01_raw.wav | _unknown_ | false_alarm | no@1.11s 0.70 | 0.04 | no 0.82 | move 0.10 | _unknown_ 0.02 | ch0 |
| raw/supanut/S1/move/move_08_raw.wav | move | miss |  | 0.681 | move 0.68 | hand 0.11 | base 0.05 | ch0 |
| raw/supanut/S1/place/place_10_raw.wav | place | miss |  | 0.383 | base 0.68 | place 0.29 | move 0.01 | ch0 |
| raw/supanut/S1/red_box/red_box_10_raw.wav | red box | miss |  | 0.679 | red box 0.68 | green box 0.07 | base 0.06 | ch0 |
| raw/supanut/S1/drop/drop_10_raw.wav | drop | miss |  | 0.695 | drop 0.69 | grab 0.10 | no 0.05 | ch0 |
| raw/supanut/S1/_unknown_/ride_01_raw.wav | _unknown_ | false_alarm | right@1.01s 0.91 | 0.034 | right 0.94 | grab 0.03 | _unknown_ 0.01 | ch0 |
| raw/supanut/S1/move/move_09_raw.wav | move | miss |  | 0.188 | hand 0.50 | move 0.18 | home 0.13 | ch0 |
| raw/supanut/S1/black_box/black_box_10_raw.wav | black box | miss |  | 0.421 | black box 0.42 | yes 0.19 | red box 0.16 | ch0 |

### Noise recordings

| file | duration_s | inferences | false_alarms | top_classes | detections |
|---|---|---|---|---|---|
| noise\room\supanut_S1_room.wav | 60.0 | 590 | 0 | _silence_ 590 |  |
