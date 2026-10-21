# Board evaluation - exp01_tcresnet-w16d2_aug-all on S1

model `D:\Microcon69\ProjectRoboticArm\ProjectSpace\2026-micro-con\voice_command_research\workspace\experiments\exp01_tcresnet-w16d2_aug-all\runs\final.pth` (pytorch TC-ResNet w16 d2, 23 classes) · gain +15.0 dB · features librosa 0.11.0 (features.py) · 2026-10-21 19:01

## Clip mode (1 s clips, mono ch1)

n 290 · **accuracy 0.8759** · **macro-F1 0.8719** · commands 0.965 · _unknown_ 0.0333 · _silence_ 1.0 · commands rejected as unknown/silence 0 · speech peak median -34.7 dB (training clip -25.4 dB)

| cls | support | correct | recall | precision | f1 | mean_p_true | confused_with |
|---|---|---|---|---|---|---|---|
| move | 10 | 8 | 0.8 | 0.8 | 0.8 | 0.636 | hand 1, no 1 |
| pick | 10 | 10 | 1.0 | 0.909 | 0.952 | 0.968 |  |
| grab | 10 | 10 | 1.0 | 0.909 | 0.952 | 0.791 |  |
| blue box | 10 | 9 | 0.9 | 1.0 | 0.947 | 0.508 | no 1 |
| green box | 10 | 10 | 1.0 | 1.0 | 1.0 | 0.965 |  |
| red box | 10 | 10 | 1.0 | 0.667 | 0.8 | 0.887 |  |
| black box | 10 | 8 | 0.8 | 1.0 | 0.889 | 0.648 | red box 2 |
| hand | 10 | 10 | 1.0 | 0.833 | 0.909 | 0.943 |  |
| station | 10 | 10 | 1.0 | 1.0 | 1.0 | 0.887 |  |
| base | 10 | 10 | 1.0 | 0.833 | 0.909 | 0.747 |  |
| forward | 10 | 10 | 1.0 | 0.909 | 0.952 | 0.913 |  |
| backward | 10 | 10 | 1.0 | 1.0 | 1.0 | 0.997 |  |
| left | 10 | 10 | 1.0 | 0.769 | 0.87 | 0.963 |  |
| right | 10 | 10 | 1.0 | 0.667 | 0.8 | 0.995 |  |
| place | 10 | 8 | 0.8 | 0.889 | 0.842 | 0.661 | base 2 |
| drop | 10 | 10 | 1.0 | 1.0 | 1.0 | 0.799 |  |
| yes | 10 | 10 | 1.0 | 1.0 | 1.0 | 0.985 |  |
| no | 10 | 10 | 1.0 | 0.588 | 0.741 | 0.984 |  |
| home | 10 | 10 | 1.0 | 0.667 | 0.8 | 0.883 |  |
| stop | 10 | 10 | 1.0 | 0.909 | 0.952 | 0.974 |  |
| _unknown_ | 30 | 1 | 0.033 | 1.0 | 0.065 | 0.056 | right 5, no 5, home 5 |
| _silence_ | 60 | 60 | 1.0 | 1.0 | 1.0 | 0.867 |  |

### Mistakes (top-3)

| clip_path | label | pred | top3 | snr_db | flags |
|---|---|---|---|---|---|
| clips/supanut/S1/_unknown_/arm_01.wav | _unknown_ | home | home 0.90 | _unknown_ 0.02 | hand 0.02 | 22.45 |  |
| clips/supanut/S1/place/place_03.wav | place | base | base 0.56 | place 0.32 | _unknown_ 0.03 | 21.67 |  |
| clips/supanut/S1/_unknown_/moon_01.wav | _unknown_ | forward | forward 0.50 | home 0.15 | move 0.13 | 25.84 |  |
| clips/supanut/S1/_unknown_/hold_01.wav | _unknown_ | right | right 0.28 | home 0.20 | base 0.10 | 24.63 |  |
| clips/supanut/S1/_unknown_/books_01.wav | _unknown_ | red box | red box 0.31 | pick 0.10 | blue box 0.10 | 23.75 |  |
| clips/supanut/S1/_unknown_/close_01.wav | _unknown_ | no | no 0.60 | yes 0.13 | _unknown_ 0.04 | 17.22 |  |
| clips/supanut/S1/_unknown_/nose_01.wav | _unknown_ | no | no 0.89 | blue box 0.02 | red box 0.02 | 21.15 |  |
| clips/supanut/S1/black_box/black_box_03.wav | black box | red box | red box 0.29 | black box 0.26 | yes 0.15 | 20.67 |  |
| clips/supanut/S1/_unknown_/groove_01.wav | _unknown_ | move | move 0.64 | home 0.14 | forward 0.11 | 25.92 |  |
| clips/supanut/S1/move/move_04.wav | move | no | no 0.39 | move 0.30 | hand 0.07 | 22.97 |  |
| clips/supanut/S1/_unknown_/wait_01.wav | _unknown_ | place | place 0.39 | right 0.33 | _unknown_ 0.08 | 19.04 |  |
| clips/supanut/S1/_unknown_/now_01.wav | _unknown_ | no | no 0.61 | left 0.15 | yes 0.04 | 19.3 |  |
| clips/supanut/S1/_unknown_/pink_01.wav | _unknown_ | hand | hand 0.40 | pick 0.14 | _unknown_ 0.14 | 13.71 |  |
| clips/supanut/S1/_unknown_/bright_01.wav | _unknown_ | right | right 1.00 | grab 0.00 | _unknown_ 0.00 | 20.23 |  |
| clips/supanut/S1/_unknown_/back_01.wav | _unknown_ | left | left 0.94 | backward 0.03 | pick 0.01 | 18.02 |  |
| clips/supanut/S1/place/place_06.wav | place | base | base 0.73 | place 0.23 | _unknown_ 0.01 | 23.77 |  |
| clips/supanut/S1/_unknown_/grape_01.wav | _unknown_ | right | right 0.84 | _unknown_ 0.05 | pick 0.03 | 26.07 |  |
| clips/supanut/S1/_unknown_/phone_01.wav | _unknown_ | home | home 0.53 | forward 0.26 | _unknown_ 0.08 | 21.41 |  |
| clips/supanut/S1/_unknown_/black_01.wav | _unknown_ | left | left 0.91 | backward 0.03 | yes 0.02 | 18.92 |  |
| clips/supanut/S1/_unknown_/box_01.wav | _unknown_ | red box | red box 0.42 | blue box 0.21 | black box 0.17 | 20.34 |  |
| clips/supanut/S1/blue_box/blue_box_06.wav | blue box | no | no 0.66 | blue box 0.20 | yes 0.03 | 23.83 |  |
| clips/supanut/S1/_unknown_/mood_01.wav | _unknown_ | move | move 0.47 | no 0.08 | _unknown_ 0.08 | 20.3 |  |
| clips/supanut/S1/_unknown_/blue_01.wav | _unknown_ | no | no 0.57 | move 0.37 | _unknown_ 0.02 | 21.44 |  |
| clips/supanut/S1/_unknown_/go_01.wav | _unknown_ | home | home 0.88 | no 0.03 | _unknown_ 0.01 | 24.55 |  |
| clips/supanut/S1/_unknown_/write_01.wav | _unknown_ | right | right 0.97 | grab 0.01 | left 0.00 | 19.15 |  |
| clips/supanut/S1/_unknown_/sick_01.wav | _unknown_ | pick | pick 0.33 | station 0.17 | _unknown_ 0.14 | 16.91 |  |
| clips/supanut/S1/_unknown_/hope_01.wav | _unknown_ | home | home 0.53 | forward 0.15 | move 0.07 | 19.75 |  |
| clips/supanut/S1/_unknown_/grass_01.wav | _unknown_ | grab | grab 0.36 | drop 0.23 | right 0.14 | 20.04 |  |
| clips/supanut/S1/_unknown_/bag_01.wav | _unknown_ | left | left 0.89 | backward 0.04 | pick 0.01 | 17.07 |  |
| clips/supanut/S1/_unknown_/more_01.wav | _unknown_ | no | no 0.98 | _unknown_ 0.01 | move 0.00 | 21.86 |  |
| clips/supanut/S1/_unknown_/foam_01.wav | _unknown_ | home | home 0.49 | forward 0.19 | _unknown_ 0.12 | 23.57 |  |
| clips/supanut/S1/_unknown_/up_01.wav | _unknown_ | stop | stop 0.41 | drop 0.12 | left 0.08 | 17.26 |  |
| clips/supanut/S1/_unknown_/robot_01.wav | _unknown_ | red box | red box 0.30 | green box 0.28 | blue box 0.20 | 20.02 | sound longer than 1 s - the clip cuts it |
| clips/supanut/S1/_unknown_/ride_01.wav | _unknown_ | right | right 0.49 | grab 0.40 | drop 0.06 | 23.37 |  |
| clips/supanut/S1/move/move_09.wav | move | hand | hand 0.35 | home 0.25 | move 0.11 | 17.11 | sound longer than 1 s - the clip cuts it |
| clips/supanut/S1/black_box/black_box_10.wav | black box | red box | red box 0.41 | black box 0.39 | yes 0.05 | 15.47 |  |

## Stream mode (raw 2 s takes, board logic: stride 10 frames, avg 3, threshold 0.7, refractory 1500 ms, channel auto)

command takes 200: **detected 0.825** (hit 165, hit+extra 0), wrong 3, miss 32 · first detection correct 0.825 · _unknown_ false alarms 13/30 · noise false alarms 0 in 1.0 min · channels {'ch0': 230}

| cls | takes | hit | hit_extra | wrong | miss | false_alarm | detected_rate | wrong_as | mean_max_p_true |
|---|---|---|---|---|---|---|---|---|---|
| move | 10 | 5 | 0 | 0 | 5 | 0 | 0.5 |  | 0.652 |
| pick | 10 | 10 | 0 | 0 | 0 | 0 | 1.0 |  | 0.972 |
| grab | 10 | 7 | 0 | 0 | 3 | 0 | 0.7 |  | 0.788 |
| blue box | 10 | 2 | 0 | 0 | 8 | 0 | 0.2 |  | 0.499 |
| green box | 10 | 10 | 0 | 0 | 0 | 0 | 1.0 |  | 0.939 |
| red box | 10 | 10 | 0 | 0 | 0 | 0 | 1.0 |  | 0.912 |
| black box | 10 | 2 | 0 | 1 | 7 | 0 | 0.2 | backward 1 | 0.591 |
| hand | 10 | 10 | 0 | 0 | 0 | 0 | 1.0 |  | 0.971 |
| station | 10 | 7 | 0 | 2 | 1 | 0 | 0.7 | base 2 | 0.849 |
| base | 10 | 8 | 0 | 0 | 2 | 0 | 0.8 |  | 0.815 |
| forward | 10 | 10 | 0 | 0 | 0 | 0 | 1.0 |  | 0.879 |
| backward | 10 | 10 | 0 | 0 | 0 | 0 | 1.0 |  | 0.996 |
| left | 10 | 10 | 0 | 0 | 0 | 0 | 1.0 |  | 0.951 |
| right | 10 | 10 | 0 | 0 | 0 | 0 | 1.0 |  | 0.995 |
| place | 10 | 6 | 0 | 0 | 4 | 0 | 0.6 |  | 0.679 |
| drop | 10 | 9 | 0 | 0 | 1 | 0 | 0.9 |  | 0.792 |
| yes | 10 | 10 | 0 | 0 | 0 | 0 | 1.0 |  | 0.982 |
| no | 10 | 10 | 0 | 0 | 0 | 0 | 1.0 |  | 0.985 |
| home | 10 | 9 | 0 | 0 | 1 | 0 | 0.9 |  | 0.816 |
| stop | 10 | 10 | 0 | 0 | 0 | 0 | 1.0 |  | 0.981 |
| _unknown_ | 30 | 0 | 0 | 0 | 0 | 13 |  | right 4, home 3, no 3 | 0.069 |

### Takes that were not a clean hit

| raw_path | label | outcome | detections | max_avg_p_true | peak_top3 | channels_used |
|---|---|---|---|---|---|---|
| raw/supanut/S1/_unknown_/arm_01_raw.wav | _unknown_ | false_alarm | home@1.11s 0.74 | 0.071 | home 0.83 | _unknown_ 0.05 | hand 0.03 | ch0 |
| raw/supanut/S1/base/base_01_raw.wav | base | miss |  | 0.48 | place 0.59 | base 0.38 | move 0.01 | ch0 |
| raw/supanut/S1/base/base_02_raw.wav | base | miss |  | 0.682 | base 0.68 | place 0.29 | move 0.01 | ch0 |
| raw/supanut/S1/black_box/black_box_01_raw.wav | black box | miss |  | 0.631 | black box 0.63 | yes 0.11 | backward 0.08 | ch0 |
| raw/supanut/S1/black_box/black_box_02_raw.wav | black box | miss |  | 0.581 | backward 0.62 | no 0.08 | left 0.07 | ch0 |
| raw/supanut/S1/blue_box/blue_box_01_raw.wav | blue box | miss |  | 0.452 | move 0.48 | no 0.31 | blue box 0.06 | ch0 |
| raw/supanut/S1/grab/grab_01_raw.wav | grab | miss |  | 0.587 | grab 0.59 | right 0.25 | left 0.06 | ch0 |
| raw/supanut/S1/place/place_03_raw.wav | place | miss |  | 0.63 | place 0.63 | base 0.28 | _unknown_ 0.02 | ch0 |
| raw/supanut/S1/_unknown_/close_01_raw.wav | _unknown_ | false_alarm | no@1.01s 0.84 | 0.045 | no 0.84 | drop 0.02 | stop 0.02 | ch0 |
| raw/supanut/S1/grab/grab_02_raw.wav | grab | miss |  | 0.685 | grab 0.69 | left 0.12 | right 0.07 | ch0 |
| raw/supanut/S1/grab/grab_03_raw.wav | grab | miss |  | 0.521 | grab 0.52 | right 0.28 | left 0.09 | ch0 |
| raw/supanut/S1/_unknown_/nose_01_raw.wav | _unknown_ | false_alarm | no@1.01s 0.94 | 0.044 | no 0.94 | _unknown_ 0.01 | move 0.01 | ch0 |
| raw/supanut/S1/black_box/black_box_03_raw.wav | black box | miss |  | 0.427 | backward 0.48 | no 0.13 | left 0.11 | ch0 |
| raw/supanut/S1/black_box/black_box_04_raw.wav | black box | wrong | backward@1.01s 0.71 | 0.787 | black box 0.79 | left 0.06 | yes 0.04 | ch0 |
| raw/supanut/S1/blue_box/blue_box_02_raw.wav | blue box | miss |  | 0.47 | blue box 0.47 | green box 0.22 | red box 0.14 | ch0 |
| raw/supanut/S1/station/station_03_raw.wav | station | wrong | base@1.01s 0.85 | 0.947 | station 0.95 | base 0.04 | _unknown_ 0.00 | ch0 |
| raw/supanut/S1/blue_box/blue_box_03_raw.wav | blue box | miss |  | 0.331 | move 0.38 | no 0.28 | blue box 0.09 | ch0 |
| raw/supanut/S1/black_box/black_box_05_raw.wav | black box | miss |  | 0.567 | black box 0.57 | red box 0.10 | left 0.07 | ch0 |
| raw/supanut/S1/black_box/black_box_06_raw.wav | black box | miss |  | 0.34 | backward 0.43 | left 0.20 | no 0.07 | ch0 |
| raw/supanut/S1/move/move_04_raw.wav | move | miss |  | 0.382 | move 0.38 | no 0.24 | hand 0.11 | ch0 |
| raw/supanut/S1/move/move_05_raw.wav | move | miss |  | 0.378 | move 0.38 | no 0.14 | hand 0.12 | ch0 |
| raw/supanut/S1/_unknown_/bright_01_raw.wav | _unknown_ | false_alarm | right@1.11s 0.76 | 0.02 | right 1.00 | grab 0.00 | _unknown_ 0.00 | ch0 |
| raw/supanut/S1/_unknown_/back_01_raw.wav | _unknown_ | false_alarm | left@1.01s 0.84 | 0.015 | left 0.95 | backward 0.02 | black box 0.01 | ch0 |
| raw/supanut/S1/place/place_06_raw.wav | place | miss |  | 0.34 | base 0.66 | place 0.29 | _unknown_ 0.01 | ch0 |
| raw/supanut/S1/home/home_02_raw.wav | home | miss |  | 0.403 | home 0.40 | hand 0.13 | move 0.13 | ch0 |
| raw/supanut/S1/station/station_04_raw.wav | station | wrong | base@1.01s 0.78 | 0.944 | station 0.94 | base 0.04 | _unknown_ 0.00 | ch0 |
| raw/supanut/S1/_unknown_/grape_01_raw.wav | _unknown_ | false_alarm | right@1.01s 0.70 | 0.115 | right 0.72 | _unknown_ 0.10 | pick 0.03 | ch0 |
| raw/supanut/S1/blue_box/blue_box_04_raw.wav | blue box | miss |  | 0.537 | blue box 0.54 | green box 0.16 | _unknown_ 0.05 | ch0 |
| raw/supanut/S1/place/place_07_raw.wav | place | miss |  | 0.569 | base 0.63 | place 0.32 | _unknown_ 0.01 | ch0 |
| raw/supanut/S1/_unknown_/black_01_raw.wav | _unknown_ | false_alarm | left@1.01s 0.96 | 0.013 | left 0.96 | yes 0.01 | backward 0.01 | ch0 |
| raw/supanut/S1/place/place_08_raw.wav | place | miss |  | 0.516 | base 0.53 | place 0.37 | yes 0.02 | ch0 |
| raw/supanut/S1/move/move_07_raw.wav | move | miss |  | 0.51 | move 0.51 | hand 0.16 | _unknown_ 0.06 | ch0 |
| raw/supanut/S1/blue_box/blue_box_07_raw.wav | blue box | miss |  | 0.397 | move 0.45 | no 0.32 | blue box 0.08 | ch0 |
| raw/supanut/S1/_unknown_/go_01_raw.wav | _unknown_ | false_alarm | home@1.01s 0.72 | 0.033 | home 0.78 | move 0.03 | forward 0.03 | ch0 |
| raw/supanut/S1/_unknown_/write_01_raw.wav | _unknown_ | false_alarm | right@1.01s 0.99 | 0.024 | right 0.99 | grab 0.00 | drop 0.00 | ch0 |
| raw/supanut/S1/blue_box/blue_box_08_raw.wav | blue box | miss |  | 0.259 | move 0.59 | no 0.13 | _unknown_ 0.09 | ch0 |
| raw/supanut/S1/drop/drop_08_raw.wav | drop | miss |  | 0.47 | drop 0.47 | grab 0.31 | _unknown_ 0.06 | ch0 |
| raw/supanut/S1/black_box/black_box_08_raw.wav | black box | miss |  | 0.647 | black box 0.65 | yes 0.17 | left 0.05 | ch0 |
| raw/supanut/S1/station/station_07_raw.wav | station | miss |  | 0.427 | base 0.65 | station 0.24 | place 0.05 | ch0 |
| raw/supanut/S1/_unknown_/bag_01_raw.wav | _unknown_ | false_alarm | left@1.01s 0.95 | 0.022 | left 0.95 | pick 0.01 | backward 0.01 | ch0 |
| raw/supanut/S1/_unknown_/more_01_raw.wav | _unknown_ | false_alarm | no@1.01s 0.94 | 0.056 | no 0.94 | _unknown_ 0.02 | move 0.01 | ch0 |
| raw/supanut/S1/move/move_08_raw.wav | move | miss |  | 0.483 | move 0.48 | undo 0.17 | hand 0.09 | ch0 |
| raw/supanut/S1/blue_box/blue_box_09_raw.wav | blue box | miss |  | 0.567 | no 0.69 | move 0.08 | blue box 0.07 | ch0 |
| raw/supanut/S1/blue_box/blue_box_10_raw.wav | blue box | miss |  | 0.497 | move 0.68 | no 0.19 | blue box 0.04 | ch0 |
| raw/supanut/S1/_unknown_/foam_01_raw.wav | _unknown_ | false_alarm | home@1.71s 0.72 | 0.073 | home 0.72 | hand 0.06 | no 0.05 | ch0 |
| raw/supanut/S1/_unknown_/ride_01_raw.wav | _unknown_ | false_alarm | right@1.01s 0.72 | 0.051 | right 0.80 | grab 0.13 | drop 0.04 | ch0 |
| raw/supanut/S1/move/move_09_raw.wav | move | miss |  | 0.592 | move 0.59 | no 0.13 | _unknown_ 0.06 | ch0 |
| raw/supanut/S1/black_box/black_box_10_raw.wav | black box | miss |  | 0.237 | yes 0.26 | black box 0.21 | red box 0.20 | ch0 |

### Noise recordings

| file | duration_s | inferences | false_alarms | top_classes | detections |
|---|---|---|---|---|---|
| noise\room\supanut_S1_room.wav | 60.0 | 590 | 0 | _silence_ 590 |  |
