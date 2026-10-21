# Board evaluation - exp08_tcresnet-w16d4_aug-all on S1

model `D:\Microcon69\ProjectRoboticArm\ProjectSpace\2026-micro-con\voice_command_research\workspace\experiments\exp08_tcresnet-w16d4_aug-all\runs\final.pth` (pytorch exp08_tcresnet-w16d4_aug-all, 23 classes) · gain +15.0 dB · features librosa 0.11.0 (features.py) · 2026-10-21 21:53

## Clip mode (1 s clips, mono ch1)

n 290 · **accuracy 0.8793** · **macro-F1 0.8763** · commands 0.97 · _unknown_ 0.0333 · _silence_ 1.0 · commands rejected as unknown/silence 0 · speech peak median -34.7 dB (training clip -25.4 dB)

| cls | support | correct | recall | precision | f1 | mean_p_true | confused_with |
|---|---|---|---|---|---|---|---|
| move | 10 | 9 | 0.9 | 0.818 | 0.857 | 0.765 | home 1 |
| pick | 10 | 10 | 1.0 | 0.909 | 0.952 | 0.97 |  |
| grab | 10 | 8 | 0.8 | 1.0 | 0.889 | 0.726 | right 2 |
| blue box | 10 | 10 | 1.0 | 1.0 | 1.0 | 0.911 |  |
| green box | 10 | 10 | 1.0 | 0.769 | 0.87 | 0.967 |  |
| red box | 10 | 8 | 0.8 | 1.0 | 0.889 | 0.788 | green box 2 |
| black box | 10 | 10 | 1.0 | 1.0 | 1.0 | 0.82 |  |
| hand | 10 | 10 | 1.0 | 1.0 | 1.0 | 0.87 |  |
| station | 10 | 10 | 1.0 | 0.909 | 0.952 | 0.954 |  |
| base | 10 | 10 | 1.0 | 0.909 | 0.952 | 0.873 |  |
| forward | 10 | 10 | 1.0 | 0.909 | 0.952 | 0.949 |  |
| backward | 10 | 10 | 1.0 | 1.0 | 1.0 | 0.959 |  |
| left | 10 | 10 | 1.0 | 0.769 | 0.87 | 0.937 |  |
| right | 10 | 10 | 1.0 | 0.625 | 0.769 | 0.973 |  |
| place | 10 | 9 | 0.9 | 0.9 | 0.9 | 0.866 | base 1 |
| drop | 10 | 10 | 1.0 | 0.909 | 0.952 | 0.957 |  |
| yes | 10 | 10 | 1.0 | 0.833 | 0.909 | 0.968 |  |
| no | 10 | 10 | 1.0 | 0.714 | 0.833 | 0.973 |  |
| home | 10 | 10 | 1.0 | 0.556 | 0.714 | 0.951 |  |
| stop | 10 | 10 | 1.0 | 0.909 | 0.952 | 0.97 |  |
| _unknown_ | 30 | 1 | 0.033 | 1.0 | 0.065 | 0.086 | home 7, right 4, no 4 |
| _silence_ | 60 | 60 | 1.0 | 1.0 | 1.0 | 0.961 |  |

### Mistakes (top-3)

| clip_path | label | pred | top3 | snr_db | flags |
|---|---|---|---|---|---|
| clips/supanut/S1/_unknown_/arm_01.wav | _unknown_ | home | home 0.94 | _unknown_ 0.01 | hand 0.01 | 22.45 |  |
| clips/supanut/S1/grab/grab_01.wav | grab | right | right 0.64 | grab 0.11 | left 0.08 | 27.02 |  |
| clips/supanut/S1/_unknown_/moon_01.wav | _unknown_ | home | home 0.36 | move 0.33 | forward 0.11 | 25.84 |  |
| clips/supanut/S1/_unknown_/hold_01.wav | _unknown_ | home | home 0.46 | forward 0.26 | _unknown_ 0.08 | 24.63 |  |
| clips/supanut/S1/_unknown_/books_01.wav | _unknown_ | move | move 0.38 | _unknown_ 0.26 | no 0.07 | 23.75 |  |
| clips/supanut/S1/_unknown_/close_01.wav | _unknown_ | yes | yes 0.63 | no 0.22 | _unknown_ 0.07 | 17.22 |  |
| clips/supanut/S1/grab/grab_03.wav | grab | right | right 0.66 | grab 0.19 | _unknown_ 0.03 | 24.47 |  |
| clips/supanut/S1/_unknown_/nose_01.wav | _unknown_ | no | no 0.86 | yes 0.03 | _unknown_ 0.02 | 21.15 |  |
| clips/supanut/S1/_unknown_/groove_01.wav | _unknown_ | forward | forward 0.44 | move 0.24 | home 0.17 | 25.92 |  |
| clips/supanut/S1/red_box/red_box_05.wav | red box | green box | green box 0.72 | red box 0.19 | _unknown_ 0.03 | 21.46 |  |
| clips/supanut/S1/_unknown_/wait_01.wav | _unknown_ | place | place 0.53 | right 0.25 | _unknown_ 0.12 | 19.04 |  |
| clips/supanut/S1/_unknown_/now_01.wav | _unknown_ | no | no 0.95 | _unknown_ 0.01 | left 0.01 | 19.3 |  |
| clips/supanut/S1/_unknown_/pink_01.wav | _unknown_ | pick | pick 0.28 | _unknown_ 0.28 | hand 0.24 | 13.71 |  |
| clips/supanut/S1/_unknown_/bright_01.wav | _unknown_ | right | right 0.98 | drop 0.00 | _unknown_ 0.00 | 20.23 |  |
| clips/supanut/S1/red_box/red_box_06.wav | red box | green box | green box 0.91 | red box 0.05 | _unknown_ 0.01 | 20.8 |  |
| clips/supanut/S1/_unknown_/back_01.wav | _unknown_ | left | left 0.80 | backward 0.10 | _unknown_ 0.02 | 18.02 |  |
| clips/supanut/S1/_unknown_/grape_01.wav | _unknown_ | right | right 0.95 | _unknown_ 0.02 | place 0.00 | 26.07 |  |
| clips/supanut/S1/_unknown_/phone_01.wav | _unknown_ | home | home 0.64 | forward 0.14 | hand 0.08 | 21.41 |  |
| clips/supanut/S1/_unknown_/black_01.wav | _unknown_ | left | left 0.89 | yes 0.03 | backward 0.03 | 18.92 |  |
| clips/supanut/S1/place/place_08.wav | place | base | base 0.47 | place 0.47 | _unknown_ 0.01 | 25.62 |  |
| clips/supanut/S1/_unknown_/box_01.wav | _unknown_ | yes | yes 0.18 | blue box 0.15 | red box 0.13 | 20.34 |  |
| clips/supanut/S1/_unknown_/mood_01.wav | _unknown_ | move | move 0.65 | home 0.20 | forward 0.04 | 20.3 |  |
| clips/supanut/S1/_unknown_/blue_01.wav | _unknown_ | no | no 0.77 | move 0.15 | _unknown_ 0.01 | 21.44 |  |
| clips/supanut/S1/_unknown_/go_01.wav | _unknown_ | home | home 0.89 | forward 0.02 | _unknown_ 0.02 | 24.55 |  |
| clips/supanut/S1/_unknown_/write_01.wav | _unknown_ | right | right 0.97 | _unknown_ 0.00 | drop 0.00 | 19.15 |  |
| clips/supanut/S1/_unknown_/sick_01.wav | _unknown_ | station | station 0.38 | pick 0.38 | _unknown_ 0.06 | 16.91 |  |
| clips/supanut/S1/_unknown_/hope_01.wav | _unknown_ | home | home 0.68 | forward 0.11 | move 0.06 | 19.75 |  |
| clips/supanut/S1/_unknown_/grass_01.wav | _unknown_ | drop | drop 0.57 | _unknown_ 0.13 | right 0.09 | 20.04 |  |
| clips/supanut/S1/_unknown_/bag_01.wav | _unknown_ | left | left 0.78 | backward 0.12 | _unknown_ 0.03 | 17.07 |  |
| clips/supanut/S1/_unknown_/more_01.wav | _unknown_ | no | no 0.81 | move 0.13 | _unknown_ 0.01 | 21.86 |  |
| clips/supanut/S1/_unknown_/foam_01.wav | _unknown_ | home | home 0.88 | hand 0.03 | _unknown_ 0.03 | 23.57 |  |
| clips/supanut/S1/_unknown_/up_01.wav | _unknown_ | stop | stop 0.56 | _silence_ 0.10 | drop 0.09 | 17.26 |  |
| clips/supanut/S1/_unknown_/robot_01.wav | _unknown_ | green box | green box 0.49 | _unknown_ 0.21 | blue box 0.09 | 20.02 | sound longer than 1 s - the clip cuts it |
| clips/supanut/S1/_unknown_/ride_01.wav | _unknown_ | right | right 0.60 | drop 0.17 | grab 0.08 | 23.37 |  |
| clips/supanut/S1/move/move_09.wav | move | home | home 0.37 | hand 0.30 | move 0.09 | 17.11 | sound longer than 1 s - the clip cuts it |

## Stream mode (raw 2 s takes, board logic: stride 10 frames, avg 3, threshold 0.7, refractory 1500 ms, channel auto)

command takes 200: **detected 0.94** (hit 188, hit+extra 0), wrong 8, miss 4 · first detection correct 0.94 · _unknown_ false alarms 17/30 · noise false alarms 0 in 1.0 min · channels {'ch0': 230}

| cls | takes | hit | hit_extra | wrong | miss | false_alarm | detected_rate | wrong_as | mean_max_p_true |
|---|---|---|---|---|---|---|---|---|---|
| move | 10 | 9 | 0 | 0 | 1 | 0 | 0.9 |  | 0.901 |
| pick | 10 | 10 | 0 | 0 | 0 | 0 | 1.0 |  | 0.971 |
| grab | 10 | 9 | 0 | 0 | 1 | 0 | 0.9 |  | 0.89 |
| blue box | 10 | 9 | 0 | 1 | 0 | 0 | 0.9 | move 1 | 0.891 |
| green box | 10 | 10 | 0 | 0 | 0 | 0 | 1.0 |  | 0.972 |
| red box | 10 | 6 | 0 | 3 | 1 | 0 | 0.6 | green box 2, right 1 | 0.744 |
| black box | 10 | 7 | 0 | 3 | 0 | 0 | 0.7 | left 2, backward 1 | 0.794 |
| hand | 10 | 10 | 0 | 0 | 0 | 0 | 1.0 |  | 0.939 |
| station | 10 | 9 | 0 | 1 | 0 | 0 | 0.9 | base 1 | 0.957 |
| base | 10 | 10 | 0 | 0 | 0 | 0 | 1.0 |  | 0.917 |
| forward | 10 | 10 | 0 | 0 | 0 | 0 | 1.0 |  | 0.96 |
| backward | 10 | 10 | 0 | 0 | 0 | 0 | 1.0 |  | 0.968 |
| left | 10 | 10 | 0 | 0 | 0 | 0 | 1.0 |  | 0.937 |
| right | 10 | 10 | 0 | 0 | 0 | 0 | 1.0 |  | 0.983 |
| place | 10 | 9 | 0 | 0 | 1 | 0 | 0.9 |  | 0.913 |
| drop | 10 | 10 | 0 | 0 | 0 | 0 | 1.0 |  | 0.96 |
| yes | 10 | 10 | 0 | 0 | 0 | 0 | 1.0 |  | 0.977 |
| no | 10 | 10 | 0 | 0 | 0 | 0 | 1.0 |  | 0.979 |
| home | 10 | 10 | 0 | 0 | 0 | 0 | 1.0 |  | 0.958 |
| stop | 10 | 10 | 0 | 0 | 0 | 0 | 1.0 |  | 0.977 |
| _unknown_ | 30 | 0 | 0 | 0 | 0 | 17 |  | right 4, home 3, no 3 | 0.132 |

### Takes that were not a clean hit

| raw_path | label | outcome | detections | max_avg_p_true | peak_top3 | channels_used |
|---|---|---|---|---|---|---|
| raw/supanut/S1/_unknown_/arm_01_raw.wav | _unknown_ | false_alarm | home@1.01s 0.86 | 0.041 | home 0.93 | _unknown_ 0.02 | hand 0.01 | ch0 |
| raw/supanut/S1/black_box/black_box_01_raw.wav | black box | wrong | left@1.01s 0.77 | 0.884 | black box 0.88 | _unknown_ 0.03 | backward 0.01 | ch0 |
| raw/supanut/S1/_unknown_/books_01_raw.wav | _unknown_ | false_alarm | move@1.01s 0.94 | 0.123 | move 0.94 | _unknown_ 0.01 | home 0.01 | ch0 |
| raw/supanut/S1/_unknown_/close_01_raw.wav | _unknown_ | false_alarm | yes@1.31s 0.77 | 0.083 | yes 0.88 | no 0.05 | _unknown_ 0.04 | ch0 |
| raw/supanut/S1/grab/grab_03_raw.wav | grab | miss |  | 0.662 | grab 0.66 | right 0.19 | _unknown_ 0.03 | ch0 |
| raw/supanut/S1/_unknown_/nose_01_raw.wav | _unknown_ | false_alarm | no@1.01s 0.97 | 0.15 | no 0.97 | yes 0.01 | _unknown_ 0.00 | ch0 |
| raw/supanut/S1/red_box/red_box_03_raw.wav | red box | wrong | right@1.01s 0.82 | 0.894 | red box 0.89 | green box 0.05 | _unknown_ 0.02 | ch0 |
| raw/supanut/S1/_unknown_/groove_01_raw.wav | _unknown_ | false_alarm | move@1.91s 0.71 | 0.043 | move 0.71 | home 0.08 | no 0.04 | ch0 |
| raw/supanut/S1/station/station_03_raw.wav | station | wrong | base@1.01s 0.89 | 0.956 | station 0.96 | base 0.01 | _unknown_ 0.01 | ch0 |
| raw/supanut/S1/red_box/red_box_05_raw.wav | red box | miss |  | 0.273 | green box 0.70 | red box 0.20 | _unknown_ 0.04 | ch0 |
| raw/supanut/S1/black_box/black_box_06_raw.wav | black box | wrong | left@1.01s 0.87 | 0.482 | left 0.87 | _unknown_ 0.04 | backward 0.03 | ch0 |
| raw/supanut/S1/_unknown_/now_01_raw.wav | _unknown_ | false_alarm | no@1.01s 0.96 | 0.034 | no 0.97 | _unknown_ 0.01 | yes 0.00 | ch0 |
| raw/supanut/S1/move/move_05_raw.wav | move | miss |  | 0.571 | move 0.57 | home 0.15 | hand 0.13 | ch0 |
| raw/supanut/S1/_unknown_/bright_01_raw.wav | _unknown_ | false_alarm | right@1.01s 0.87 | 0.017 | right 0.98 | _unknown_ 0.00 | drop 0.00 | ch0 |
| raw/supanut/S1/red_box/red_box_06_raw.wav | red box | wrong | green box@1.41s 0.85 | 0.195 | green box 0.92 | red box 0.03 | _unknown_ 0.01 | ch0 |
| raw/supanut/S1/_unknown_/back_01_raw.wav | _unknown_ | false_alarm | left@1.71s 0.72 | 0.032 | left 0.78 | backward 0.11 | _unknown_ 0.03 | ch0 |
| raw/supanut/S1/place/place_06_raw.wav | place | miss |  | 0.637 | base 0.68 | place 0.27 | yes 0.01 | ch0 |
| raw/supanut/S1/_unknown_/grape_01_raw.wav | _unknown_ | false_alarm | right@1.01s 0.94 | 0.033 | right 0.94 | _unknown_ 0.02 | grab 0.01 | ch0 |
| raw/supanut/S1/_unknown_/black_01_raw.wav | _unknown_ | false_alarm | left@1.01s 0.89 | 0.014 | left 0.94 | yes 0.02 | _unknown_ 0.01 | ch0 |
| raw/supanut/S1/red_box/red_box_07_raw.wav | red box | wrong | green box@1.01s 0.73 | 0.351 | green box 0.76 | _unknown_ 0.09 | red box 0.05 | ch0 |
| raw/supanut/S1/_unknown_/go_01_raw.wav | _unknown_ | false_alarm | home@1.01s 0.85 | 0.039 | home 0.85 | forward 0.03 | _unknown_ 0.03 | ch0 |
| raw/supanut/S1/_unknown_/write_01_raw.wav | _unknown_ | false_alarm | right@1.01s 0.98 | 0.022 | right 0.98 | _unknown_ 0.00 | left 0.00 | ch0 |
| raw/supanut/S1/blue_box/blue_box_08_raw.wav | blue box | wrong | move@1.01s 0.85 | 0.845 | move 0.85 | _unknown_ 0.05 | no 0.04 | ch0 |
| raw/supanut/S1/_unknown_/grass_01_raw.wav | _unknown_ | false_alarm | drop@1.01s 0.82 | 0.081 | drop 0.82 | _unknown_ 0.05 | grab 0.04 | ch0 |
| raw/supanut/S1/_unknown_/more_01_raw.wav | _unknown_ | false_alarm | no@1.41s 0.74 | 0.039 | no 0.74 | move 0.17 | _unknown_ 0.01 | ch0 |
| raw/supanut/S1/_unknown_/foam_01_raw.wav | _unknown_ | false_alarm | home@1.01s 0.86 | 0.039 | home 0.87 | _unknown_ 0.03 | forward 0.03 | ch0 |
| raw/supanut/S1/_unknown_/up_01_raw.wav | _unknown_ | false_alarm | stop@1.01s 0.76 | 0.041 | stop 0.79 | drop 0.06 | _unknown_ 0.03 | ch0 |
| raw/supanut/S1/_unknown_/ride_01_raw.wav | _unknown_ | false_alarm | right@1.21s 0.75 | 0.061 | right 0.88 | _unknown_ 0.04 | drop 0.02 | ch0 |
| raw/supanut/S1/black_box/black_box_10_raw.wav | black box | wrong | backward@1.01s 0.81 | 0.662 | backward 0.81 | _unknown_ 0.08 | left 0.03 | ch0 |

### Noise recordings

| file | duration_s | inferences | false_alarms | top_classes | detections |
|---|---|---|---|---|---|
| noise\room\supanut_S1_room.wav | 60.0 | 590 | 0 | _silence_ 590 |  |
