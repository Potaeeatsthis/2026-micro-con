# Board evaluation - exp03_tcresnet-w16d2_aug-all_mswc-cap1000 on S1

model `D:\Microcon69\ProjectRoboticArm\ProjectSpace\2026-micro-con\voice_command_research\workspace\experiments\exp03_tcresnet-w16d2_aug-all_mswc-cap1000\runs\final.pth` (pytorch exp03_tcresnet-w16d2_aug-all_mswc-cap1000, 23 classes) · gain +15.0 dB · features librosa 0.11.0 (features.py) · 2026-10-21 20:03

## Clip mode (1 s clips, mono ch1)

n 290 · **accuracy 0.8621** · **macro-F1 0.8522** · commands 0.945 · _unknown_ 0.0333 · _silence_ 1.0 · commands rejected as unknown/silence 0 · speech peak median -34.7 dB (training clip -25.4 dB)

| cls | support | correct | recall | precision | f1 | mean_p_true | confused_with |
|---|---|---|---|---|---|---|---|
| move | 10 | 9 | 0.9 | 0.75 | 0.818 | 0.674 | hand 1 |
| pick | 10 | 10 | 1.0 | 0.833 | 0.909 | 0.971 |  |
| grab | 10 | 9 | 0.9 | 1.0 | 0.947 | 0.806 | left 1 |
| blue box | 10 | 10 | 1.0 | 0.909 | 0.952 | 0.844 |  |
| green box | 10 | 10 | 1.0 | 0.909 | 0.952 | 0.958 |  |
| red box | 10 | 9 | 0.9 | 1.0 | 0.947 | 0.727 | green box 1 |
| black box | 10 | 8 | 0.8 | 1.0 | 0.889 | 0.528 | backward 2 |
| hand | 10 | 10 | 1.0 | 0.714 | 0.833 | 0.949 |  |
| station | 10 | 10 | 1.0 | 1.0 | 1.0 | 0.888 |  |
| base | 10 | 5 | 0.5 | 1.0 | 0.667 | 0.555 | place 5 |
| forward | 10 | 9 | 0.9 | 1.0 | 0.947 | 0.793 | hand 1 |
| backward | 10 | 10 | 1.0 | 0.833 | 0.909 | 0.998 |  |
| left | 10 | 10 | 1.0 | 0.667 | 0.8 | 0.987 |  |
| right | 10 | 10 | 1.0 | 0.667 | 0.8 | 0.997 |  |
| place | 10 | 10 | 1.0 | 0.667 | 0.8 | 0.821 |  |
| drop | 10 | 10 | 1.0 | 0.833 | 0.909 | 0.887 |  |
| yes | 10 | 10 | 1.0 | 1.0 | 1.0 | 0.99 |  |
| no | 10 | 10 | 1.0 | 0.625 | 0.769 | 0.988 |  |
| home | 10 | 10 | 1.0 | 0.714 | 0.833 | 0.902 |  |
| stop | 10 | 10 | 1.0 | 1.0 | 1.0 | 0.966 |  |
| _unknown_ | 30 | 1 | 0.033 | 1.0 | 0.065 | 0.061 | no 6, right 5, left 4 |
| _silence_ | 60 | 60 | 1.0 | 1.0 | 1.0 | 0.803 |  |

### Mistakes (top-3)

| clip_path | label | pred | top3 | snr_db | flags |
|---|---|---|---|---|---|
| clips/supanut/S1/_unknown_/arm_01.wav | _unknown_ | home | home 0.87 | hand 0.04 | no 0.03 | 22.45 |  |
| clips/supanut/S1/base/base_01.wav | base | place | place 0.61 | base 0.35 | right 0.01 | 19.4 |  |
| clips/supanut/S1/base/base_02.wav | base | place | place 0.53 | base 0.43 | right 0.01 | 20.95 |  |
| clips/supanut/S1/grab/grab_01.wav | grab | left | left 0.49 | grab 0.33 | right 0.11 | 27.02 |  |
| clips/supanut/S1/_unknown_/moon_01.wav | _unknown_ | move | move 0.43 | forward 0.16 | home 0.11 | 25.84 |  |
| clips/supanut/S1/_unknown_/hold_01.wav | _unknown_ | home | home 0.62 | forward 0.08 | _unknown_ 0.07 | 24.63 |  |
| clips/supanut/S1/_unknown_/books_01.wav | _unknown_ | left | left 0.23 | move 0.19 | blue box 0.10 | 23.75 |  |
| clips/supanut/S1/_unknown_/close_01.wav | _unknown_ | no | no 0.50 | yes 0.13 | _unknown_ 0.09 | 17.22 |  |
| clips/supanut/S1/base/base_03.wav | base | place | place 0.64 | base 0.34 | _unknown_ 0.01 | 18.37 |  |
| clips/supanut/S1/_unknown_/nose_01.wav | _unknown_ | no | no 0.87 | yes 0.03 | _unknown_ 0.02 | 21.15 |  |
| clips/supanut/S1/black_box/black_box_03.wav | black box | backward | backward 0.35 | left 0.20 | black box 0.19 | 20.67 |  |
| clips/supanut/S1/_unknown_/groove_01.wav | _unknown_ | move | move 0.53 | home 0.20 | forward 0.08 | 25.92 |  |
| clips/supanut/S1/black_box/black_box_06.wav | black box | backward | backward 0.44 | black box 0.35 | left 0.09 | 20.09 |  |
| clips/supanut/S1/_unknown_/wait_01.wav | _unknown_ | right | right 0.40 | place 0.28 | left 0.10 | 19.04 |  |
| clips/supanut/S1/forward/forward_05.wav | forward | hand | hand 0.48 | forward 0.16 | _unknown_ 0.10 | 23.94 |  |
| clips/supanut/S1/_unknown_/now_01.wav | _unknown_ | no | no 0.80 | left 0.08 | _unknown_ 0.03 | 19.3 |  |
| clips/supanut/S1/_unknown_/pink_01.wav | _unknown_ | pick | pick 0.40 | hand 0.26 | _unknown_ 0.12 | 13.71 |  |
| clips/supanut/S1/_unknown_/bright_01.wav | _unknown_ | right | right 1.00 | _unknown_ 0.00 | left 0.00 | 20.23 |  |
| clips/supanut/S1/red_box/red_box_06.wav | red box | green box | green box 0.58 | red box 0.33 | grab 0.02 | 20.8 |  |
| clips/supanut/S1/_unknown_/back_01.wav | _unknown_ | left | left 0.89 | backward 0.05 | grab 0.01 | 18.02 |  |
| clips/supanut/S1/_unknown_/grape_01.wav | _unknown_ | right | right 0.97 | _unknown_ 0.01 | green box 0.00 | 26.07 |  |
| clips/supanut/S1/_unknown_/phone_01.wav | _unknown_ | hand | hand 0.33 | home 0.27 | _unknown_ 0.13 | 21.41 |  |
| clips/supanut/S1/_unknown_/black_01.wav | _unknown_ | left | left 0.99 | yes 0.00 | backward 0.00 | 18.92 |  |
| clips/supanut/S1/_unknown_/box_01.wav | _unknown_ | no | no 0.20 | left 0.12 | black box 0.11 | 20.34 |  |
| clips/supanut/S1/_unknown_/mood_01.wav | _unknown_ | move | move 0.48 | home 0.19 | _unknown_ 0.08 | 20.3 |  |
| clips/supanut/S1/_unknown_/blue_01.wav | _unknown_ | no | no 0.57 | move 0.39 | blue box 0.02 | 21.44 |  |
| clips/supanut/S1/_unknown_/go_01.wav | _unknown_ | home | home 0.90 | no 0.02 | _unknown_ 0.02 | 24.55 |  |
| clips/supanut/S1/_unknown_/write_01.wav | _unknown_ | right | right 0.99 | grab 0.00 | drop 0.00 | 19.15 |  |
| clips/supanut/S1/_unknown_/sick_01.wav | _unknown_ | pick | pick 0.48 | station 0.16 | _unknown_ 0.12 | 16.91 |  |
| clips/supanut/S1/_unknown_/hope_01.wav | _unknown_ | home | home 0.69 | pick 0.07 | _unknown_ 0.06 | 19.75 |  |
| clips/supanut/S1/_unknown_/grass_01.wav | _unknown_ | drop | drop 0.46 | grab 0.33 | left 0.05 | 20.04 |  |
| clips/supanut/S1/_unknown_/bag_01.wav | _unknown_ | left | left 0.81 | backward 0.05 | right 0.04 | 17.07 |  |
| clips/supanut/S1/base/base_09.wav | base | place | place 0.69 | base 0.18 | yes 0.08 | 16.52 |  |
| clips/supanut/S1/_unknown_/more_01.wav | _unknown_ | no | no 0.97 | _unknown_ 0.01 | move 0.01 | 21.86 |  |
| clips/supanut/S1/base/base_10.wav | base | place | place 0.63 | base 0.20 | yes 0.07 | 14.71 | sound longer than 1 s - the clip cuts it |
| clips/supanut/S1/_unknown_/foam_01.wav | _unknown_ | hand | hand 0.39 | home 0.32 | _unknown_ 0.11 | 23.57 |  |
| clips/supanut/S1/_unknown_/up_01.wav | _unknown_ | drop | drop 0.20 | stop 0.18 | _silence_ 0.13 | 17.26 |  |
| clips/supanut/S1/_unknown_/robot_01.wav | _unknown_ | blue box | blue box 0.27 | green box 0.16 | red box 0.11 | 20.02 | sound longer than 1 s - the clip cuts it |
| clips/supanut/S1/_unknown_/ride_01.wav | _unknown_ | right | right 0.54 | drop 0.25 | grab 0.17 | 23.37 |  |
| clips/supanut/S1/move/move_09.wav | move | hand | hand 0.48 | home 0.17 | move 0.14 | 17.11 | sound longer than 1 s - the clip cuts it |

## Stream mode (raw 2 s takes, board logic: stride 10 frames, avg 3, threshold 0.7, refractory 1500 ms, channel auto)

command takes 200: **detected 0.815** (hit 163, hit+extra 0), wrong 8, miss 29 · first detection correct 0.815 · _unknown_ false alarms 16/30 · noise false alarms 0 in 1.0 min · channels {'ch0': 230}

| cls | takes | hit | hit_extra | wrong | miss | false_alarm | detected_rate | wrong_as | mean_max_p_true |
|---|---|---|---|---|---|---|---|---|---|
| move | 10 | 8 | 0 | 0 | 2 | 0 | 0.8 |  | 0.796 |
| pick | 10 | 10 | 0 | 0 | 0 | 0 | 1.0 |  | 0.968 |
| grab | 10 | 9 | 0 | 0 | 1 | 0 | 0.9 |  | 0.852 |
| blue box | 10 | 8 | 0 | 0 | 2 | 0 | 0.8 |  | 0.78 |
| green box | 10 | 10 | 0 | 0 | 0 | 0 | 1.0 |  | 0.946 |
| red box | 10 | 5 | 0 | 0 | 5 | 0 | 0.5 |  | 0.706 |
| black box | 10 | 0 | 0 | 1 | 9 | 0 | 0.0 | backward 1 | 0.464 |
| hand | 10 | 10 | 0 | 0 | 0 | 0 | 1.0 |  | 0.963 |
| station | 10 | 6 | 0 | 4 | 0 | 0 | 0.6 | base 4 | 0.901 |
| base | 10 | 4 | 0 | 3 | 3 | 0 | 0.4 | place 3 | 0.664 |
| forward | 10 | 7 | 0 | 0 | 3 | 0 | 0.7 |  | 0.771 |
| backward | 10 | 10 | 0 | 0 | 0 | 0 | 1.0 |  | 0.996 |
| left | 10 | 10 | 0 | 0 | 0 | 0 | 1.0 |  | 0.988 |
| right | 10 | 10 | 0 | 0 | 0 | 0 | 1.0 |  | 0.996 |
| place | 10 | 8 | 0 | 0 | 2 | 0 | 0.8 |  | 0.831 |
| drop | 10 | 9 | 0 | 0 | 1 | 0 | 0.9 |  | 0.881 |
| yes | 10 | 10 | 0 | 0 | 0 | 0 | 1.0 |  | 0.988 |
| no | 10 | 10 | 0 | 0 | 0 | 0 | 1.0 |  | 0.992 |
| home | 10 | 9 | 0 | 0 | 1 | 0 | 0.9 |  | 0.828 |
| stop | 10 | 10 | 0 | 0 | 0 | 0 | 1.0 |  | 0.978 |
| _unknown_ | 30 | 0 | 0 | 0 | 0 | 16 |  | right 5, no 4, home 3 | 0.068 |

### Takes that were not a clean hit

| raw_path | label | outcome | detections | max_avg_p_true | peak_top3 | channels_used |
|---|---|---|---|---|---|---|
| raw/supanut/S1/_unknown_/arm_01_raw.wav | _unknown_ | false_alarm | home@1.21s 0.72 | 0.03 | home 0.83 | no 0.07 | stop 0.03 | ch0 |
| raw/supanut/S1/base/base_01_raw.wav | base | wrong | place@1.41s 0.75 | 0.364 | place 0.75 | base 0.21 | right 0.01 | ch0 |
| raw/supanut/S1/base/base_02_raw.wav | base | wrong | place@1.51s 0.72 | 0.41 | place 0.73 | base 0.23 | yes 0.01 | ch0 |
| raw/supanut/S1/black_box/black_box_01_raw.wav | black box | miss |  | 0.613 | black box 0.61 | left 0.15 | backward 0.15 | ch0 |
| raw/supanut/S1/station/station_02_raw.wav | station | wrong | base@1.01s 0.71 | 0.981 | station 0.98 | base 0.01 | _unknown_ 0.00 | ch0 |
| raw/supanut/S1/black_box/black_box_02_raw.wav | black box | miss |  | 0.57 | backward 0.60 | left 0.13 | black box 0.12 | ch0 |
| raw/supanut/S1/grab/grab_01_raw.wav | grab | miss |  | 0.613 | grab 0.61 | left 0.26 | right 0.05 | ch0 |
| raw/supanut/S1/_unknown_/close_01_raw.wav | _unknown_ | false_alarm | no@1.01s 0.72 | 0.073 | no 0.72 | yes 0.06 | left 0.05 | ch0 |
| raw/supanut/S1/base/base_03_raw.wav | base | wrong | place@1.01s 0.70 | 0.649 | place 0.70 | base 0.27 | _unknown_ 0.01 | ch0 |
| raw/supanut/S1/_unknown_/nose_01_raw.wav | _unknown_ | false_alarm | no@1.01s 0.94 | 0.051 | no 0.94 | _unknown_ 0.01 | move 0.01 | ch0 |
| raw/supanut/S1/black_box/black_box_03_raw.wav | black box | wrong | backward@1.01s 0.77 | 0.306 | backward 0.77 | left 0.06 | black box 0.05 | ch0 |
| raw/supanut/S1/red_box/red_box_03_raw.wav | red box | miss |  | 0.698 | red box 0.70 | green box 0.13 | backward 0.04 | ch0 |
| raw/supanut/S1/black_box/black_box_04_raw.wav | black box | miss |  | 0.329 | left 0.60 | black box 0.28 | backward 0.07 | ch0 |
| raw/supanut/S1/base/base_04_raw.wav | base | miss |  | 0.617 | base 0.62 | place 0.31 | yes 0.03 | ch0 |
| raw/supanut/S1/red_box/red_box_04_raw.wav | red box | miss |  | 0.571 | red box 0.57 | backward 0.10 | grab 0.07 | ch0 |
| raw/supanut/S1/station/station_03_raw.wav | station | wrong | base@1.01s 0.88 | 0.946 | station 0.95 | base 0.03 | _unknown_ 0.01 | ch0 |
| raw/supanut/S1/blue_box/blue_box_03_raw.wav | blue box | miss |  | 0.671 | blue box 0.67 | move 0.11 | no 0.09 | ch0 |
| raw/supanut/S1/black_box/black_box_05_raw.wav | black box | miss |  | 0.535 | black box 0.54 | left 0.20 | backward 0.18 | ch0 |
| raw/supanut/S1/red_box/red_box_05_raw.wav | red box | miss |  | 0.494 | red box 0.49 | green box 0.24 | grab 0.05 | ch0 |
| raw/supanut/S1/black_box/black_box_06_raw.wav | black box | miss |  | 0.37 | backward 0.52 | black box 0.18 | left 0.10 | ch0 |
| raw/supanut/S1/_unknown_/wait_01_raw.wav | _unknown_ | false_alarm | right@1.41s 0.72 | 0.05 | right 0.72 | place 0.13 | left 0.03 | ch0 |
| raw/supanut/S1/forward/forward_05_raw.wav | forward | miss |  | 0.401 | forward 0.40 | home 0.14 | hand 0.13 | ch0 |
| raw/supanut/S1/move/move_05_raw.wav | move | miss |  | 0.662 | move 0.66 | home 0.09 | hand 0.07 | ch0 |
| raw/supanut/S1/_unknown_/bright_01_raw.wav | _unknown_ | false_alarm | right@1.01s 0.89 | 0.007 | right 1.00 | _unknown_ 0.00 | left 0.00 | ch0 |
| raw/supanut/S1/red_box/red_box_06_raw.wav | red box | miss |  | 0.498 | red box 0.50 | green box 0.32 | grab 0.04 | ch0 |
| raw/supanut/S1/_unknown_/back_01_raw.wav | _unknown_ | false_alarm | left@1.01s 0.89 | 0.007 | left 0.96 | backward 0.02 | yes 0.00 | ch0 |
| raw/supanut/S1/place/place_06_raw.wav | place | miss |  | 0.482 | base 0.64 | place 0.30 | _unknown_ 0.01 | ch0 |
| raw/supanut/S1/home/home_02_raw.wav | home | miss |  | 0.435 | home 0.43 | pick 0.18 | hand 0.13 | ch0 |
| raw/supanut/S1/station/station_04_raw.wav | station | wrong | base@1.01s 0.80 | 0.922 | station 0.92 | base 0.05 | _unknown_ 0.01 | ch0 |
| raw/supanut/S1/_unknown_/grape_01_raw.wav | _unknown_ | false_alarm | right@1.01s 0.87 | 0.08 | right 0.87 | _unknown_ 0.05 | place 0.02 | ch0 |
| raw/supanut/S1/_unknown_/black_01_raw.wav | _unknown_ | false_alarm | left@1.01s 0.98 | 0.011 | left 0.98 | yes 0.01 | backward 0.00 | ch0 |
| raw/supanut/S1/place/place_08_raw.wav | place | miss |  | 0.662 | place 0.66 | base 0.29 | _unknown_ 0.02 | ch0 |
| raw/supanut/S1/station/station_05_raw.wav | station | wrong | base@1.01s 0.78 | 0.797 | station 0.80 | base 0.15 | _unknown_ 0.02 | ch0 |
| raw/supanut/S1/black_box/black_box_07_raw.wav | black box | miss |  | 0.69 | black box 0.69 | backward 0.14 | left 0.13 | ch0 |
| raw/supanut/S1/red_box/red_box_07_raw.wav | red box | miss |  | 0.407 | green box 0.49 | red box 0.34 | _unknown_ 0.04 | ch0 |
| raw/supanut/S1/_unknown_/blue_01_raw.wav | _unknown_ | false_alarm | no@1.01s 0.70 | 0.018 | no 0.70 | move 0.27 | undo 0.01 | ch0 |
| raw/supanut/S1/blue_box/blue_box_07_raw.wav | blue box | miss |  | 0.432 | blue box 0.43 | no 0.18 | move 0.16 | ch0 |
| raw/supanut/S1/_unknown_/go_01_raw.wav | _unknown_ | false_alarm | home@1.01s 0.87 | 0.034 | home 0.92 | _unknown_ 0.02 | no 0.01 | ch0 |
| raw/supanut/S1/_unknown_/write_01_raw.wav | _unknown_ | false_alarm | right@1.01s 1.00 | 0.019 | right 1.00 | grab 0.00 | _unknown_ 0.00 | ch0 |
| raw/supanut/S1/_unknown_/sick_01_raw.wav | _unknown_ | false_alarm | pick@1.61s 0.72 | 0.093 | pick 0.75 | _unknown_ 0.05 | left 0.04 | ch0 |
| raw/supanut/S1/forward/forward_07_raw.wav | forward | miss |  | 0.66 | forward 0.66 | _unknown_ 0.10 | hand 0.06 | ch0 |
| raw/supanut/S1/drop/drop_08_raw.wav | drop | miss |  | 0.552 | drop 0.55 | grab 0.29 | pick 0.05 | ch0 |
| raw/supanut/S1/black_box/black_box_08_raw.wav | black box | miss |  | 0.461 | backward 0.58 | black box 0.16 | left 0.15 | ch0 |
| raw/supanut/S1/_unknown_/hope_01_raw.wav | _unknown_ | false_alarm | home@1.51s 0.71 | 0.079 | home 0.71 | _unknown_ 0.06 | pick 0.05 | ch0 |
| raw/supanut/S1/_unknown_/bag_01_raw.wav | _unknown_ | false_alarm | left@1.01s 0.79 | 0.021 | left 0.81 | right 0.06 | backward 0.04 | ch0 |
| raw/supanut/S1/base/base_09_raw.wav | base | miss |  | 0.606 | base 0.61 | place 0.32 | yes 0.03 | ch0 |
| raw/supanut/S1/black_box/black_box_09_raw.wav | black box | miss |  | 0.42 | backward 0.63 | left 0.15 | black box 0.12 | ch0 |
| raw/supanut/S1/_unknown_/more_01_raw.wav | _unknown_ | false_alarm | no@1.01s 0.89 | 0.042 | no 0.92 | _unknown_ 0.02 | move 0.01 | ch0 |
| raw/supanut/S1/base/base_10_raw.wav | base | miss |  | 0.534 | base 0.53 | place 0.30 | yes 0.05 | ch0 |
| raw/supanut/S1/forward/forward_09_raw.wav | forward | miss |  | 0.496 | forward 0.50 | hand 0.18 | _unknown_ 0.10 | ch0 |
| raw/supanut/S1/_unknown_/ride_01_raw.wav | _unknown_ | false_alarm | right@1.01s 0.85 | 0.039 | right 0.86 | drop 0.09 | grab 0.03 | ch0 |
| raw/supanut/S1/move/move_10_raw.wav | move | miss |  | 0.699 | move 0.70 | hand 0.11 | _unknown_ 0.03 | ch0 |
| raw/supanut/S1/black_box/black_box_10_raw.wav | black box | miss |  | 0.347 | backward 0.62 | black box 0.10 | left 0.10 | ch0 |

### Noise recordings

| file | duration_s | inferences | false_alarms | top_classes | detections |
|---|---|---|---|---|---|
| noise\room\supanut_S1_room.wav | 60.0 | 590 | 0 | _silence_ 590 |  |
