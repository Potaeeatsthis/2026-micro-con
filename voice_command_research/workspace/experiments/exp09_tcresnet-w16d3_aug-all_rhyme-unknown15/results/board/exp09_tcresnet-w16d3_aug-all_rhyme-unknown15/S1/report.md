# Board evaluation - exp09_tcresnet-w16d3_aug-all_rhyme-unknown15 on S1

model `D:\Microcon69\ProjectRoboticArm\ProjectSpace\2026-micro-con\voice_command_research\workspace\experiments\exp09_tcresnet-w16d3_aug-all_rhyme-unknown15\runs\final.pth` (pytorch exp09_tcresnet-w16d3_aug-all_rhyme-unknown15, 23 classes) · gain +15.0 dB · features librosa 0.11.0 (features.py) · 2026-10-21 22:46

## Clip mode (1 s clips, mono ch1)

n 290 · **accuracy 0.9034** · **macro-F1 0.9048** · commands 0.975 · _unknown_ 0.2333 · _silence_ 1.0 · commands rejected as unknown/silence 0 · speech peak median -34.7 dB (training clip -25.4 dB)

| cls | support | correct | recall | precision | f1 | mean_p_true | confused_with |
|---|---|---|---|---|---|---|---|
| move | 10 | 9 | 0.9 | 0.9 | 0.9 | 0.775 | home 1 |
| pick | 10 | 10 | 1.0 | 0.909 | 0.952 | 0.972 |  |
| grab | 10 | 10 | 1.0 | 1.0 | 1.0 | 0.865 |  |
| blue box | 10 | 10 | 1.0 | 0.909 | 0.952 | 0.971 |  |
| green box | 10 | 10 | 1.0 | 0.714 | 0.833 | 0.973 |  |
| red box | 10 | 6 | 0.6 | 1.0 | 0.75 | 0.475 | green box 4 |
| black box | 10 | 10 | 1.0 | 1.0 | 1.0 | 0.878 |  |
| hand | 10 | 10 | 1.0 | 1.0 | 1.0 | 0.848 |  |
| station | 10 | 10 | 1.0 | 1.0 | 1.0 | 0.918 |  |
| base | 10 | 10 | 1.0 | 1.0 | 1.0 | 0.789 |  |
| forward | 10 | 10 | 1.0 | 0.833 | 0.909 | 0.902 |  |
| backward | 10 | 10 | 1.0 | 1.0 | 1.0 | 0.976 |  |
| left | 10 | 10 | 1.0 | 0.769 | 0.87 | 0.956 |  |
| right | 10 | 10 | 1.0 | 0.714 | 0.833 | 0.979 |  |
| place | 10 | 10 | 1.0 | 1.0 | 1.0 | 0.878 |  |
| drop | 10 | 10 | 1.0 | 1.0 | 1.0 | 0.949 |  |
| yes | 10 | 10 | 1.0 | 1.0 | 1.0 | 0.975 |  |
| no | 10 | 10 | 1.0 | 0.714 | 0.833 | 0.987 |  |
| home | 10 | 10 | 1.0 | 0.588 | 0.741 | 0.908 |  |
| stop | 10 | 10 | 1.0 | 0.909 | 0.952 | 0.971 |  |
| _unknown_ | 30 | 7 | 0.233 | 1.0 | 0.378 | 0.183 | home 6, right 4, no 4 |
| _silence_ | 60 | 60 | 1.0 | 1.0 | 1.0 | 0.968 |  |

### Mistakes (top-3)

| clip_path | label | pred | top3 | snr_db | flags |
|---|---|---|---|---|---|
| clips/supanut/S1/_unknown_/arm_01.wav | _unknown_ | home | home 0.96 | _unknown_ 0.03 | hand 0.00 | 22.45 |  |
| clips/supanut/S1/_unknown_/moon_01.wav | _unknown_ | home | home 0.72 | move 0.10 | hand 0.06 | 25.84 |  |
| clips/supanut/S1/_unknown_/hold_01.wav | _unknown_ | forward | forward 0.31 | home 0.29 | _unknown_ 0.08 | 24.63 |  |
| clips/supanut/S1/_unknown_/books_01.wav | _unknown_ | blue box | blue box 0.29 | _unknown_ 0.19 | yes 0.13 | 23.75 |  |
| clips/supanut/S1/_unknown_/nose_01.wav | _unknown_ | no | no 0.98 | _unknown_ 0.01 | move 0.00 | 21.15 |  |
| clips/supanut/S1/_unknown_/groove_01.wav | _unknown_ | home | home 0.63 | move 0.23 | hand 0.04 | 25.92 |  |
| clips/supanut/S1/red_box/red_box_05.wav | red box | green box | green box 0.77 | red box 0.11 | blue box 0.05 | 21.46 |  |
| clips/supanut/S1/_unknown_/now_01.wav | _unknown_ | no | no 0.83 | _unknown_ 0.07 | stop 0.01 | 19.3 |  |
| clips/supanut/S1/_unknown_/bright_01.wav | _unknown_ | right | right 0.96 | _unknown_ 0.03 | left 0.00 | 20.23 |  |
| clips/supanut/S1/red_box/red_box_06.wav | red box | green box | green box 0.98 | red box 0.01 | blue box 0.00 | 20.8 |  |
| clips/supanut/S1/_unknown_/back_01.wav | _unknown_ | left | left 0.91 | _unknown_ 0.03 | backward 0.02 | 18.02 |  |
| clips/supanut/S1/_unknown_/grape_01.wav | _unknown_ | right | right 0.88 | _unknown_ 0.04 | grab 0.02 | 26.07 |  |
| clips/supanut/S1/_unknown_/phone_01.wav | _unknown_ | home | home 0.48 | forward 0.26 | _unknown_ 0.17 | 21.41 |  |
| clips/supanut/S1/_unknown_/black_01.wav | _unknown_ | left | left 0.80 | yes 0.04 | backward 0.03 | 18.92 |  |
| clips/supanut/S1/red_box/red_box_07.wav | red box | green box | green box 0.86 | red box 0.07 | blue box 0.03 | 20.62 |  |
| clips/supanut/S1/_unknown_/mood_01.wav | _unknown_ | move | move 0.84 | home 0.07 | _unknown_ 0.02 | 20.3 |  |
| clips/supanut/S1/_unknown_/blue_01.wav | _unknown_ | no | no 0.58 | move 0.33 | _unknown_ 0.02 | 21.44 |  |
| clips/supanut/S1/_unknown_/go_01.wav | _unknown_ | home | home 0.85 | _unknown_ 0.05 | forward 0.02 | 24.55 |  |
| clips/supanut/S1/_unknown_/write_01.wav | _unknown_ | right | right 0.97 | _unknown_ 0.02 | left 0.00 | 19.15 |  |
| clips/supanut/S1/_unknown_/sick_01.wav | _unknown_ | pick | pick 0.40 | _unknown_ 0.26 | station 0.19 | 16.91 |  |
| clips/supanut/S1/red_box/red_box_08.wav | red box | green box | green box 0.68 | red box 0.25 | blue box 0.04 | 20.13 |  |
| clips/supanut/S1/_unknown_/hope_01.wav | _unknown_ | forward | forward 0.31 | home 0.28 | move 0.17 | 19.75 |  |
| clips/supanut/S1/_unknown_/bag_01.wav | _unknown_ | left | left 0.51 | backward 0.22 | _unknown_ 0.15 | 17.07 |  |
| clips/supanut/S1/_unknown_/more_01.wav | _unknown_ | no | no 0.93 | move 0.02 | _unknown_ 0.01 | 21.86 |  |
| clips/supanut/S1/_unknown_/foam_01.wav | _unknown_ | home | home 0.58 | hand 0.11 | forward 0.08 | 23.57 |  |
| clips/supanut/S1/_unknown_/up_01.wav | _unknown_ | stop | stop 0.66 | _unknown_ 0.12 | drop 0.08 | 17.26 |  |
| clips/supanut/S1/_unknown_/ride_01.wav | _unknown_ | right | right 0.76 | _unknown_ 0.14 | drop 0.02 | 23.37 |  |
| clips/supanut/S1/move/move_09.wav | move | home | home 0.54 | _unknown_ 0.13 | hand 0.12 | 17.11 | sound longer than 1 s - the clip cuts it |

## Stream mode (raw 2 s takes, board logic: stride 10 frames, avg 3, threshold 0.7, refractory 1500 ms, channel auto)

command takes 200: **detected 0.94** (hit 188, hit+extra 0), wrong 5, miss 7 · first detection correct 0.94 · _unknown_ false alarms 15/30 · noise false alarms 0 in 1.0 min · channels {'ch0': 230}

| cls | takes | hit | hit_extra | wrong | miss | false_alarm | detected_rate | wrong_as | mean_max_p_true |
|---|---|---|---|---|---|---|---|---|---|
| move | 10 | 10 | 0 | 0 | 0 | 0 | 1.0 |  | 0.873 |
| pick | 10 | 10 | 0 | 0 | 0 | 0 | 1.0 |  | 0.974 |
| grab | 10 | 10 | 0 | 0 | 0 | 0 | 1.0 |  | 0.898 |
| blue box | 10 | 10 | 0 | 0 | 0 | 0 | 1.0 |  | 0.972 |
| green box | 10 | 10 | 0 | 0 | 0 | 0 | 1.0 |  | 0.982 |
| red box | 10 | 4 | 0 | 4 | 2 | 0 | 0.4 | green box 4 | 0.562 |
| black box | 10 | 10 | 0 | 0 | 0 | 0 | 1.0 |  | 0.877 |
| hand | 10 | 10 | 0 | 0 | 0 | 0 | 1.0 |  | 0.918 |
| station | 10 | 9 | 0 | 1 | 0 | 0 | 0.9 | base 1 | 0.923 |
| base | 10 | 7 | 0 | 0 | 3 | 0 | 0.7 |  | 0.813 |
| forward | 10 | 10 | 0 | 0 | 0 | 0 | 1.0 |  | 0.923 |
| backward | 10 | 10 | 0 | 0 | 0 | 0 | 1.0 |  | 0.978 |
| left | 10 | 10 | 0 | 0 | 0 | 0 | 1.0 |  | 0.958 |
| right | 10 | 10 | 0 | 0 | 0 | 0 | 1.0 |  | 0.987 |
| place | 10 | 9 | 0 | 0 | 1 | 0 | 0.9 |  | 0.897 |
| drop | 10 | 10 | 0 | 0 | 0 | 0 | 1.0 |  | 0.936 |
| yes | 10 | 10 | 0 | 0 | 0 | 0 | 1.0 |  | 0.981 |
| no | 10 | 10 | 0 | 0 | 0 | 0 | 1.0 |  | 0.991 |
| home | 10 | 9 | 0 | 0 | 1 | 0 | 0.9 |  | 0.899 |
| stop | 10 | 10 | 0 | 0 | 0 | 0 | 1.0 |  | 0.976 |
| _unknown_ | 30 | 0 | 0 | 0 | 0 | 15 |  | home 4, no 4, right 4 | 0.251 |

### Takes that were not a clean hit

| raw_path | label | outcome | detections | max_avg_p_true | peak_top3 | channels_used |
|---|---|---|---|---|---|---|
| raw/supanut/S1/_unknown_/arm_01_raw.wav | _unknown_ | false_alarm | home@1.01s 0.82 | 0.132 | home 0.93 | _unknown_ 0.04 | stop 0.00 | ch0 |
| raw/supanut/S1/_unknown_/moon_01_raw.wav | _unknown_ | false_alarm | home@1.01s 0.81 | 0.132 | home 0.81 | hand 0.07 | _unknown_ 0.03 | ch0 |
| raw/supanut/S1/_unknown_/close_01_raw.wav | _unknown_ | false_alarm | no@1.01s 0.71 | 0.375 | no 0.73 | _unknown_ 0.10 | yes 0.05 | ch0 |
| raw/supanut/S1/_unknown_/nose_01_raw.wav | _unknown_ | false_alarm | no@1.01s 0.99 | 0.231 | no 0.99 | _unknown_ 0.00 | move 0.00 | ch0 |
| raw/supanut/S1/red_box/red_box_03_raw.wav | red box | wrong | green box@1.71s 0.78 | 0.46 | green box 0.78 | red box 0.14 | blue box 0.04 | ch0 |
| raw/supanut/S1/base/base_04_raw.wav | base | miss |  | 0.646 | base 0.65 | place 0.24 | yes 0.05 | ch0 |
| raw/supanut/S1/station/station_03_raw.wav | station | wrong | base@1.01s 0.91 | 0.905 | base 0.91 | _unknown_ 0.03 | place 0.02 | ch0 |
| raw/supanut/S1/red_box/red_box_05_raw.wav | red box | wrong | green box@1.31s 0.71 | 0.232 | green box 0.80 | red box 0.11 | blue box 0.04 | ch0 |
| raw/supanut/S1/_unknown_/now_01_raw.wav | _unknown_ | false_alarm | no@1.01s 0.87 | 0.116 | no 0.87 | _unknown_ 0.05 | yes 0.03 | ch0 |
| raw/supanut/S1/_unknown_/bright_01_raw.wav | _unknown_ | false_alarm | right@1.11s 0.76 | 0.062 | right 0.98 | _unknown_ 0.02 | left 0.00 | ch0 |
| raw/supanut/S1/red_box/red_box_06_raw.wav | red box | wrong | green box@1.31s 0.91 | 0.307 | green box 0.95 | red box 0.02 | blue box 0.01 | ch0 |
| raw/supanut/S1/_unknown_/back_01_raw.wav | _unknown_ | false_alarm | left@1.11s 0.72 | 0.04 | left 0.90 | backward 0.04 | _unknown_ 0.02 | ch0 |
| raw/supanut/S1/home/home_02_raw.wav | home | miss |  | 0.593 | home 0.59 | _unknown_ 0.09 | forward 0.08 | ch0 |
| raw/supanut/S1/_unknown_/grape_01_raw.wav | _unknown_ | false_alarm | right@1.01s 0.83 | 0.156 | right 0.83 | _unknown_ 0.08 | grab 0.01 | ch0 |
| raw/supanut/S1/_unknown_/black_01_raw.wav | _unknown_ | false_alarm | left@1.01s 0.84 | 0.046 | left 0.85 | yes 0.06 | _unknown_ 0.03 | ch0 |
| raw/supanut/S1/place/place_08_raw.wav | place | miss |  | 0.684 | place 0.68 | base 0.23 | _unknown_ 0.06 | ch0 |
| raw/supanut/S1/red_box/red_box_07_raw.wav | red box | wrong | green box@1.01s 0.84 | 0.284 | green box 0.86 | red box 0.06 | blue box 0.04 | ch0 |
| raw/supanut/S1/_unknown_/mood_01_raw.wav | _unknown_ | false_alarm | move@1.01s 0.77 | 0.101 | move 0.81 | home 0.11 | _unknown_ 0.02 | ch0 |
| raw/supanut/S1/base/base_07_raw.wav | base | miss |  | 0.628 | base 0.63 | _unknown_ 0.20 | place 0.09 | ch0 |
| raw/supanut/S1/_unknown_/go_01_raw.wav | _unknown_ | false_alarm | home@1.01s 0.81 | 0.099 | home 0.88 | _unknown_ 0.05 | forward 0.02 | ch0 |
| raw/supanut/S1/_unknown_/write_01_raw.wav | _unknown_ | false_alarm | right@1.01s 0.98 | 0.092 | right 0.98 | _unknown_ 0.01 | left 0.00 | ch0 |
| raw/supanut/S1/red_box/red_box_08_raw.wav | red box | miss |  | 0.555 | green box 0.62 | red box 0.30 | blue box 0.03 | ch0 |
| raw/supanut/S1/_unknown_/more_01_raw.wav | _unknown_ | false_alarm | no@1.01s 0.93 | 0.16 | no 0.93 | _unknown_ 0.02 | move 0.01 | ch0 |
| raw/supanut/S1/base/base_10_raw.wav | base | miss |  | 0.676 | base 0.68 | place 0.15 | _unknown_ 0.09 | ch0 |
| raw/supanut/S1/_unknown_/foam_01_raw.wav | _unknown_ | false_alarm | home@1.01s 0.72 | 0.092 | home 0.74 | _unknown_ 0.08 | hand 0.07 | ch0 |
| raw/supanut/S1/red_box/red_box_10_raw.wav | red box | miss |  | 0.668 | red box 0.67 | green box 0.26 | blue box 0.04 | ch0 |
| raw/supanut/S1/_unknown_/ride_01_raw.wav | _unknown_ | false_alarm | right@1.01s 0.86 | 0.375 | right 0.89 | _unknown_ 0.08 | grab 0.01 | ch0 |

### Noise recordings

| file | duration_s | inferences | false_alarms | top_classes | detections |
|---|---|---|---|---|---|
| noise\room\supanut_S1_room.wav | 60.0 | 590 | 0 | _silence_ 590 |  |
