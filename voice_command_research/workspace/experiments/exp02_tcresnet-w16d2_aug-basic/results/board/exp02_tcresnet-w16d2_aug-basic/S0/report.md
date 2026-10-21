# Board evaluation - exp02_tcresnet-w16d2_aug-basic on S0

model `D:\Microcon69\ProjectRoboticArm\ProjectSpace\2026-micro-con\voice_command_research\workspace\experiments\exp02_tcresnet-w16d2_aug-basic\runs\final.pth` (pytorch TC-ResNet w16 d2, 23 classes) · gain +15.0 dB · features librosa 0.11.0 (features.py) · 2026-10-21 19:26

## Clip mode (1 s clips, mono ch1)

n 109 · **accuracy 0.8165** · **macro-F1 0.8265** · commands 0.9362 · _unknown_ 0.0667 · _silence_ None · commands rejected as unknown/silence 0 · speech peak median -32.4 dB (training clip -25.4 dB)

| cls | support | correct | recall | precision | f1 | mean_p_true | confused_with |
|---|---|---|---|---|---|---|---|
| move | 5 | 3 | 0.6 | 0.6 | 0.6 | 0.5 | hand 2 |
| pick | 2 | 2 | 1.0 | 1.0 | 1.0 | 0.991 |  |
| grab | 7 | 6 | 0.857 | 1.0 | 0.923 | 0.657 | left 1 |
| blue box | 7 | 7 | 1.0 | 1.0 | 1.0 | 0.657 |  |
| green box | 6 | 6 | 1.0 | 1.0 | 1.0 | 0.886 |  |
| red box | 3 | 3 | 1.0 | 0.75 | 0.857 | 0.64 |  |
| black box | 4 | 3 | 0.75 | 1.0 | 0.857 | 0.508 | yes 1 |
| hand | 5 | 5 | 1.0 | 0.625 | 0.769 | 0.898 |  |
| station | 6 | 6 | 1.0 | 1.0 | 1.0 | 0.972 |  |
| base | 5 | 5 | 1.0 | 0.833 | 0.909 | 0.808 |  |
| forward | 4 | 4 | 1.0 | 0.667 | 0.8 | 0.971 |  |
| backward | 7 | 7 | 1.0 | 1.0 | 1.0 | 0.996 |  |
| left | 5 | 5 | 1.0 | 0.714 | 0.833 | 0.867 |  |
| right | 3 | 3 | 1.0 | 0.6 | 0.75 | 0.997 |  |
| place | 7 | 6 | 0.857 | 0.75 | 0.8 | 0.599 | base 1 |
| drop | 2 | 2 | 1.0 | 1.0 | 1.0 | 0.787 |  |
| yes | 2 | 1 | 0.5 | 0.5 | 0.5 | 0.511 | place 1 |
| no | 4 | 4 | 1.0 | 0.667 | 0.8 | 0.987 |  |
| home | 5 | 5 | 1.0 | 0.714 | 0.833 | 0.969 |  |
| stop | 5 | 5 | 1.0 | 1.0 | 1.0 | 0.988 |  |
| _unknown_ | 15 | 1 | 0.067 | 1.0 | 0.125 | 0.065 | move 2, forward 2, right 2 |

### Mistakes (top-3)

| clip_path | label | pred | top3 | snr_db | flags |
|---|---|---|---|---|---|
| clips/supanut/S0/grab/grab_01.wav | grab | left | left 0.38 | grab 0.35 | right 0.11 | 27.57 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/pink_01.wav | _unknown_ | hand | hand 0.37 | pick 0.32 | home 0.06 | 19.61 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/move/move_02.wav | move | hand | hand 0.75 | _unknown_ 0.04 | home 0.03 | 22.06 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/nose_01.wav | _unknown_ | no | no 0.93 | yes 0.02 | _unknown_ 0.01 | 24.64 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/write_01.wav | _unknown_ | right | right 0.99 | grab 0.00 | _unknown_ 0.00 | 25.15 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/black_box/black_box_03.wav | black box | yes | yes 0.30 | black box 0.26 | red box 0.10 | 23.37 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/mood_01.wav | _unknown_ | move | move 0.30 | hand 0.20 | home 0.12 | 22.31 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/phone_01.wav | _unknown_ | forward | forward 0.56 | home 0.35 | _unknown_ 0.04 | 24.42 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/place/place_06.wav | place | base | base 0.87 | place 0.07 | move 0.01 | 18.83 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/moon_01.wav | _unknown_ | forward | forward 0.55 | home 0.14 | move 0.08 | 23.7 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/go_01.wav | _unknown_ | home | home 0.61 | hand 0.07 | _unknown_ 0.06 | 27.44 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/foam_01.wav | _unknown_ | home | home 0.93 | _unknown_ 0.01 | right 0.01 | 26.15 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/bright_01.wav | _unknown_ | right | right 0.98 | grab 0.01 | _unknown_ 0.00 | 24.09 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/yes/yes_02.wav | yes | place | place 0.34 | base 0.32 | _unknown_ 0.06 | 16.0 | sound longer than 1 s - the clip cuts it; S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/robot_01.wav | _unknown_ | red box | red box 0.30 | green box 0.24 | _unknown_ 0.09 | 22.74 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/wait_01.wav | _unknown_ | place | place 0.37 | right 0.14 | _unknown_ 0.11 | 19.73 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/blue_01.wav | _unknown_ | no | no 0.87 | move 0.10 | _unknown_ 0.01 | 23.87 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/move/move_05.wav | move | hand | hand 0.54 | move 0.14 | right 0.05 | 25.43 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/bag_01.wav | _unknown_ | left | left 0.47 | backward 0.13 | grab 0.08 | 17.09 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/groove_01.wav | _unknown_ | move | move 0.37 | _silence_ 0.08 | home 0.06 | 26.41 | S0 old wiring (VDD ~1.8 V) |

## Stream mode (raw 2 s takes, board logic: stride 10 frames, avg 3, threshold 0.7, refractory 1500 ms, channel auto)

command takes 94: **detected 0.266** (hit 25, hit+extra 0), wrong 2, miss 67 · first detection correct 0.266 · _unknown_ false alarms 3/15 · noise false alarms 0 in 0.0 min · channels {'ch1': 27, 'ch0/ch1': 16, 'ch0': 66}

| cls | takes | hit | hit_extra | wrong | miss | false_alarm | detected_rate | wrong_as | mean_max_p_true |
|---|---|---|---|---|---|---|---|---|---|
| move | 5 | 1 | 0 | 0 | 4 | 0 | 0.2 |  | 0.191 |
| pick | 2 | 1 | 0 | 0 | 1 | 0 | 0.5 |  | 0.498 |
| grab | 7 | 2 | 0 | 0 | 5 | 0 | 0.286 |  | 0.36 |
| blue box | 7 | 1 | 0 | 0 | 6 | 0 | 0.143 |  | 0.26 |
| green box | 6 | 0 | 0 | 0 | 6 | 0 | 0.0 |  | 0.001 |
| red box | 3 | 0 | 0 | 0 | 3 | 0 | 0.0 |  | 0.329 |
| black box | 4 | 0 | 0 | 1 | 3 | 0 | 0.0 | left 1 | 0.097 |
| hand | 5 | 1 | 0 | 0 | 4 | 0 | 0.2 |  | 0.201 |
| station | 6 | 2 | 0 | 0 | 4 | 0 | 0.333 |  | 0.32 |
| base | 5 | 1 | 0 | 0 | 4 | 0 | 0.2 |  | 0.144 |
| forward | 4 | 2 | 0 | 0 | 2 | 0 | 0.5 |  | 0.489 |
| backward | 7 | 0 | 0 | 1 | 6 | 0 | 0.0 | left 1 | 0.225 |
| left | 5 | 2 | 0 | 0 | 3 | 0 | 0.4 |  | 0.329 |
| right | 3 | 0 | 0 | 0 | 3 | 0 | 0.0 |  | 0.005 |
| place | 7 | 3 | 0 | 0 | 4 | 0 | 0.429 |  | 0.363 |
| drop | 2 | 2 | 0 | 0 | 0 | 0 | 1.0 |  | 0.789 |
| yes | 2 | 2 | 0 | 0 | 0 | 0 | 1.0 |  | 0.984 |
| no | 4 | 1 | 0 | 0 | 3 | 0 | 0.25 |  | 0.251 |
| home | 5 | 2 | 0 | 0 | 3 | 0 | 0.4 |  | 0.389 |
| stop | 5 | 2 | 0 | 0 | 3 | 0 | 0.4 |  | 0.398 |
| _unknown_ | 15 | 0 | 0 | 0 | 0 | 3 |  | forward 1, right 1, no 1 | 0.053 |

### Takes that were not a clean hit

| raw_path | label | outcome | detections | max_avg_p_true | peak_top3 | channels_used |
|---|---|---|---|---|---|---|
| raw/supanut/S0/black_box/black_box_01_raw.wav | black box | wrong | left@1.01s 0.80 | 0.386 | left 0.80 | yes 0.10 | no 0.02 | ch1 |
| raw/supanut/S0/grab/grab_01_raw.wav | grab | miss |  | 0.362 | left 0.39 | grab 0.32 | right 0.13 | ch1 |
| raw/supanut/S0/blue_box/blue_box_01_raw.wav | blue box | miss |  | 0.0 | _silence_ 0.98 | left 0.00 | move 0.00 | ch0 |
| raw/supanut/S0/home/home_02_raw.wav | home | miss |  | 0.001 | _silence_ 0.98 | pick 0.00 | move 0.00 | ch0 |
| raw/supanut/S0/red_box/red_box_01_raw.wav | red box | miss |  | 0.627 | red box 0.63 | green box 0.21 | _unknown_ 0.04 | ch1 |
| raw/supanut/S0/backward/backward_01_raw.wav | backward | wrong | left@1.01s 0.73 | 0.998 | backward 1.00 | _unknown_ 0.00 | left 0.00 | ch1 |
| raw/supanut/S0/station/station_01_raw.wav | station | miss |  | 0.001 | _silence_ 0.95 | left 0.01 | move 0.01 | ch0 |
| raw/supanut/S0/pick/pick_01_raw.wav | pick | miss |  | 0.004 | _silence_ 0.97 | move 0.00 | pick 0.00 | ch0 |
| raw/supanut/S0/black_box/black_box_02_raw.wav | black box | miss |  | 0.0 | _silence_ 0.97 | left 0.00 | pick 0.00 | ch0 |
| raw/supanut/S0/hand/hand_01_raw.wav | hand | miss |  | 0.003 | _silence_ 0.97 | left 0.01 | pick 0.00 | ch0 |
| raw/supanut/S0/move/move_02_raw.wav | move | miss |  | 0.007 | _silence_ 0.93 | left 0.01 | pick 0.01 | ch0 |
| raw/supanut/S0/backward/backward_02_raw.wav | backward | miss |  | 0.001 | _silence_ 0.96 | left 0.01 | pick 0.01 | ch0 |
| raw/supanut/S0/green_box/green_box_01_raw.wav | green box | miss |  | 0.0 | _silence_ 0.94 | left 0.01 | pick 0.01 | ch0 |
| raw/supanut/S0/base/base_01_raw.wav | base | miss |  | 0.005 | _silence_ 0.91 | left 0.01 | pick 0.01 | ch0 |
| raw/supanut/S0/green_box/green_box_02_raw.wav | green box | miss |  | 0.001 | _silence_ 0.92 | left 0.01 | hand 0.01 | ch0 |
| raw/supanut/S0/no/no_01_raw.wav | no | miss |  | 0.002 | _silence_ 0.93 | left 0.01 | pick 0.01 | ch0 |
| raw/supanut/S0/blue_box/blue_box_02_raw.wav | blue box | miss |  | 0.001 | _silence_ 0.85 | left 0.02 | hand 0.01 | ch0 |
| raw/supanut/S0/place/place_02_raw.wav | place | miss |  | 0.001 | _silence_ 0.96 | left 0.01 | pick 0.01 | ch0 |
| raw/supanut/S0/hand/hand_02_raw.wav | hand | miss |  | 0.005 | _silence_ 0.94 | left 0.01 | pick 0.01 | ch0 |
| raw/supanut/S0/right/right_01_raw.wav | right | miss |  | 0.003 | _silence_ 0.95 | left 0.01 | pick 0.01 | ch0 |
| raw/supanut/S0/grab/grab_02_raw.wav | grab | miss |  | 0.002 | _silence_ 0.95 | left 0.01 | pick 0.01 | ch0 |
| raw/supanut/S0/base/base_02_raw.wav | base | miss |  | 0.002 | _silence_ 0.96 | pick 0.01 | left 0.00 | ch0 |
| raw/supanut/S0/green_box/green_box_03_raw.wav | green box | miss |  | 0.0 | _silence_ 0.95 | left 0.01 | pick 0.01 | ch0 |
| raw/supanut/S0/backward/backward_03_raw.wav | backward | miss |  | 0.001 | _silence_ 0.96 | left 0.01 | move 0.00 | ch0 |
| raw/supanut/S0/backward/backward_04_raw.wav | backward | miss |  | 0.007 | _silence_ 0.81 | left 0.03 | hand 0.02 | ch0 |
| raw/supanut/S0/right/right_02_raw.wav | right | miss |  | 0.005 | _silence_ 0.91 | left 0.01 | pick 0.01 | ch0 |
| raw/supanut/S0/black_box/black_box_03_raw.wav | black box | miss |  | 0.0 | _silence_ 0.94 | left 0.01 | move 0.01 | ch0 |
| raw/supanut/S0/green_box/green_box_04_raw.wav | green box | miss |  | 0.001 | _silence_ 0.93 | left 0.01 | pick 0.01 | ch0 |
| raw/supanut/S0/base/base_03_raw.wav | base | miss |  | 0.002 | _silence_ 0.96 | left 0.01 | move 0.00 | ch0 |
| raw/supanut/S0/green_box/green_box_05_raw.wav | green box | miss |  | 0.001 | _silence_ 0.95 | left 0.01 | move 0.01 | ch0 |
| raw/supanut/S0/red_box/red_box_02_raw.wav | red box | miss |  | 0.0 | _silence_ 0.95 | left 0.01 | pick 0.01 | ch0 |
| raw/supanut/S0/stop/stop_02_raw.wav | stop | miss |  | 0.002 | _silence_ 0.95 | left 0.01 | move 0.01 | ch0 |
| raw/supanut/S0/backward/backward_05_raw.wav | backward | miss |  | 0.001 | _silence_ 0.96 | left 0.01 | pick 0.01 | ch0 |
| raw/supanut/S0/blue_box/blue_box_03_raw.wav | blue box | miss |  | 0.0 | _silence_ 0.96 | left 0.01 | move 0.00 | ch0 |
| raw/supanut/S0/home/home_03_raw.wav | home | miss |  | 0.002 | _silence_ 0.96 | left 0.01 | pick 0.00 | ch0 |
| raw/supanut/S0/green_box/green_box_06_raw.wav | green box | miss |  | 0.0 | _silence_ 0.94 | left 0.01 | pick 0.01 | ch0 |
| raw/supanut/S0/grab/grab_03_raw.wav | grab | miss |  | 0.002 | _silence_ 0.96 | left 0.01 | pick 0.00 | ch0 |
| raw/supanut/S0/black_box/black_box_04_raw.wav | black box | miss |  | 0.0 | _silence_ 0.93 | left 0.01 | pick 0.01 | ch0 |
| raw/supanut/S0/left/left_01_raw.wav | left | miss |  | 0.005 | _silence_ 0.96 | left 0.01 | pick 0.00 | ch0 |
| raw/supanut/S0/move/move_03_raw.wav | move | miss |  | 0.003 | _silence_ 0.98 | left 0.00 | move 0.00 | ch0 |
| raw/supanut/S0/no/no_02_raw.wav | no | miss |  | 0.001 | _silence_ 0.95 | left 0.01 | move 0.01 | ch0 |
| raw/supanut/S0/blue_box/blue_box_04_raw.wav | blue box | miss |  | 0.001 | _silence_ 0.89 | left 0.01 | pick 0.01 | ch0 |
| raw/supanut/S0/stop/stop_03_raw.wav | stop | miss |  | 0.007 | _silence_ 0.86 | left 0.02 | drop 0.01 | ch0 |
| raw/supanut/S0/place/place_04_raw.wav | place | miss |  | 0.002 | _silence_ 0.95 | pick 0.01 | left 0.01 | ch0 |
| raw/supanut/S0/grab/grab_06_raw.wav | grab | miss |  | 0.003 | _silence_ 0.93 | left 0.01 | move 0.01 | ch0 |
| raw/supanut/S0/forward/forward_01_raw.wav | forward | miss |  | 0.001 | _silence_ 0.96 | left 0.01 | move 0.00 | ch0 |
| raw/supanut/S0/station/station_03_raw.wav | station | miss |  | 0.002 | _silence_ 0.91 | left 0.01 | move 0.01 | ch0 |
| raw/supanut/S0/_unknown_/phone_01_raw.wav | _unknown_ | false_alarm | forward@1.41s 0.70 | 0.058 | forward 0.70 | home 0.22 | _unknown_ 0.04 | ch0/ch1 |
| raw/supanut/S0/backward/backward_06_raw.wav | backward | miss |  | 0.002 | _silence_ 0.94 | left 0.01 | pick 0.01 | ch0 |
| raw/supanut/S0/place/place_06_raw.wav | place | miss |  | 0.001 | _silence_ 0.95 | left 0.01 | move 0.01 | ch0 |
| raw/supanut/S0/hand/hand_03_raw.wav | hand | miss |  | 0.004 | _silence_ 0.95 | left 0.01 | pick 0.01 | ch0 |
| raw/supanut/S0/hand/hand_04_raw.wav | hand | miss |  | 0.003 | _silence_ 0.97 | left 0.01 | move 0.00 | ch0 |
| raw/supanut/S0/station/station_04_raw.wav | station | miss |  | 0.001 | _silence_ 0.95 | left 0.01 | move 0.01 | ch0 |
| raw/supanut/S0/forward/forward_02_raw.wav | forward | miss |  | 0.002 | _silence_ 0.90 | left 0.02 | drop 0.01 | ch0 |
| raw/supanut/S0/move/move_04_raw.wav | move | miss |  | 0.007 | _silence_ 0.95 | move 0.01 | pick 0.01 | ch0 |
| raw/supanut/S0/station/station_05_raw.wav | station | miss |  | 0.001 | _silence_ 0.98 | left 0.00 | move 0.00 | ch0 |
| raw/supanut/S0/red_box/red_box_03_raw.wav | red box | miss |  | 0.361 | green box 0.59 | red box 0.28 | base 0.05 | ch1 |
| raw/supanut/S0/backward/backward_07_raw.wav | backward | miss |  | 0.562 | backward 0.56 | left 0.11 | forward 0.08 | ch0/ch1 |
| raw/supanut/S0/home/home_04_raw.wav | home | miss |  | 0.002 | _silence_ 0.97 | left 0.00 | move 0.00 | ch0 |
| raw/supanut/S0/right/right_03_raw.wav | right | miss |  | 0.008 | _silence_ 0.87 | left 0.01 | pick 0.01 | ch0 |
| raw/supanut/S0/_unknown_/bright_01_raw.wav | _unknown_ | false_alarm | right@1.31s 0.98 | 0.004 | right 0.99 | grab 0.01 | _unknown_ 0.00 | ch0/ch1 |
| raw/supanut/S0/left/left_03_raw.wav | left | miss |  | 0.004 | _silence_ 0.97 | left 0.00 | move 0.00 | ch0 |
| raw/supanut/S0/stop/stop_04_raw.wav | stop | miss |  | 0.001 | _silence_ 0.97 | left 0.00 | move 0.00 | ch0 |
| raw/supanut/S0/blue_box/blue_box_06_raw.wav | blue box | miss |  | 0.643 | blue box 0.64 | green box 0.12 | _unknown_ 0.04 | ch1 |
| raw/supanut/S0/base/base_04_raw.wav | base | miss |  | 0.002 | _silence_ 0.97 | left 0.00 | move 0.00 | ch0 |
| raw/supanut/S0/blue_box/blue_box_07_raw.wav | blue box | miss |  | 0.434 | blue box 0.43 | no 0.20 | home 0.10 | ch0/ch1 |
| raw/supanut/S0/left/left_05_raw.wav | left | miss |  | 0.013 | _silence_ 0.91 | left 0.01 | pick 0.01 | ch0 |
| raw/supanut/S0/place/place_07_raw.wav | place | miss |  | 0.008 | _silence_ 0.82 | pick 0.03 | left 0.02 | ch0 |
| raw/supanut/S0/no/no_03_raw.wav | no | miss |  | 0.009 | _silence_ 0.69 | pick 0.06 | left 0.06 | ch0 |
| raw/supanut/S0/grab/grab_07_raw.wav | grab | miss |  | 0.577 | grab 0.58 | right 0.15 | left 0.09 | ch0/ch1 |
| raw/supanut/S0/_unknown_/blue_01_raw.wav | _unknown_ | false_alarm | no@1.31s 0.73 | 0.032 | no 0.88 | move 0.09 | _unknown_ 0.01 | ch1 |
| raw/supanut/S0/move/move_05_raw.wav | move | miss |  | 0.18 | hand 0.52 | move 0.17 | right 0.05 | ch1 |
