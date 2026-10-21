# Board evaluation - exp01_tcresnet-w16d2_aug-all_fold1 on S0

model `D:\Microcon69\ProjectRoboticArm\ProjectSpace\2026-micro-con\voice_command_research\workspace\experiments\exp01_tcresnet-w16d2_aug-all\runs\fold1\best.pth` (pytorch TC-ResNet w16 d2, 23 classes) · gain +15.0 dB · features librosa 0.11.0 (features.py) · 2026-10-21 17:10

## Clip mode (1 s clips, mono ch1)

n 109 · **accuracy 0.7706** · **macro-F1 0.7918** · commands 0.8936 · _unknown_ 0.0 · _silence_ None · commands rejected as unknown/silence 0 · speech peak median -32.4 dB (training clip -25.4 dB)

| cls | support | correct | recall | precision | f1 | mean_p_true | confused_with |
|---|---|---|---|---|---|---|---|
| move | 5 | 5 | 1.0 | 0.556 | 0.714 | 0.631 |  |
| pick | 2 | 2 | 1.0 | 0.667 | 0.8 | 0.994 |  |
| grab | 7 | 7 | 1.0 | 1.0 | 1.0 | 0.773 |  |
| blue box | 7 | 2 | 0.286 | 1.0 | 0.444 | 0.245 | no 4, move 1 |
| green box | 6 | 6 | 1.0 | 1.0 | 1.0 | 0.91 |  |
| red box | 3 | 3 | 1.0 | 0.75 | 0.857 | 0.828 |  |
| black box | 4 | 4 | 1.0 | 1.0 | 1.0 | 0.856 |  |
| hand | 5 | 5 | 1.0 | 1.0 | 1.0 | 0.956 |  |
| station | 6 | 4 | 0.667 | 1.0 | 0.8 | 0.665 | base 2 |
| base | 5 | 5 | 1.0 | 0.556 | 0.714 | 0.766 |  |
| forward | 4 | 4 | 1.0 | 1.0 | 1.0 | 0.94 |  |
| backward | 7 | 7 | 1.0 | 1.0 | 1.0 | 0.998 |  |
| left | 5 | 5 | 1.0 | 0.833 | 0.909 | 0.873 |  |
| right | 3 | 3 | 1.0 | 0.5 | 0.667 | 0.999 |  |
| place | 7 | 5 | 0.714 | 0.833 | 0.769 | 0.663 | base 2 |
| drop | 2 | 2 | 1.0 | 1.0 | 1.0 | 0.75 |  |
| yes | 2 | 1 | 0.5 | 1.0 | 0.667 | 0.537 | place 1 |
| no | 4 | 4 | 1.0 | 0.4 | 0.571 | 0.986 |  |
| home | 5 | 5 | 1.0 | 0.556 | 0.714 | 0.973 |  |
| stop | 5 | 5 | 1.0 | 1.0 | 1.0 | 0.973 |  |
| _unknown_ | 15 | 0 | 0.0 | 0.0 | 0.0 | 0.053 | home 4, move 3, right 3 |

### Mistakes (top-3)

| clip_path | label | pred | top3 | snr_db | flags |
|---|---|---|---|---|---|
| clips/supanut/S0/blue_box/blue_box_01.wav | blue box | no | no 0.32 | blue box 0.20 | _unknown_ 0.18 | 24.12 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/pink_01.wav | _unknown_ | pick | pick 0.63 | _unknown_ 0.14 | hand 0.05 | 19.61 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/nose_01.wav | _unknown_ | no | no 0.93 | _unknown_ 0.02 | yes 0.01 | 24.64 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/write_01.wav | _unknown_ | right | right 1.00 | _unknown_ 0.00 | red box 0.00 | 25.15 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/mood_01.wav | _unknown_ | move | move 0.82 | forward 0.06 | _unknown_ 0.03 | 22.31 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/blue_box/blue_box_03.wav | blue box | no | no 0.33 | move 0.21 | _unknown_ 0.15 | 23.84 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/phone_01.wav | _unknown_ | home | home 0.56 | forward 0.34 | _unknown_ 0.06 | 24.42 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/place/place_06.wav | place | base | base 0.72 | place 0.13 | yes 0.05 | 18.83 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/moon_01.wav | _unknown_ | move | move 0.40 | hand 0.20 | home 0.16 | 23.7 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/go_01.wav | _unknown_ | home | home 0.59 | hand 0.10 | pick 0.07 | 27.44 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/foam_01.wav | _unknown_ | home | home 0.96 | right 0.01 | _unknown_ 0.01 | 26.15 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/station/station_05.wav | station | base | base 0.51 | station 0.30 | _unknown_ 0.08 | 20.1 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/station/station_06.wav | station | base | base 0.39 | station 0.37 | _unknown_ 0.11 | 21.29 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/bright_01.wav | _unknown_ | right | right 1.00 | _unknown_ 0.00 | red box 0.00 | 24.09 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/open_01.wav | _unknown_ | home | home 0.64 | _unknown_ 0.15 | forward 0.07 | 24.52 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/yes/yes_02.wav | yes | place | place 0.43 | base 0.26 | yes 0.10 | 16.0 | sound longer than 1 s - the clip cuts it; S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/blue_box/blue_box_05.wav | blue box | no | no 0.40 | _unknown_ 0.14 | move 0.13 | 22.38 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/robot_01.wav | _unknown_ | red box | red box 0.32 | green box 0.15 | _unknown_ 0.10 | 22.74 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/wait_01.wav | _unknown_ | right | right 0.61 | place 0.15 | _unknown_ 0.13 | 19.73 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/blue_box/blue_box_06.wav | blue box | move | move 0.26 | _unknown_ 0.26 | no 0.19 | 22.3 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/blue_box/blue_box_07.wav | blue box | no | no 0.44 | _unknown_ 0.13 | move 0.12 | 25.13 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/place/place_07.wav | place | base | base 0.58 | place 0.38 | _unknown_ 0.01 | 19.77 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/blue_01.wav | _unknown_ | no | no 0.78 | move 0.19 | _unknown_ 0.01 | 23.87 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/bag_01.wav | _unknown_ | left | left 0.72 | backward 0.10 | pick 0.05 | 17.09 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/groove_01.wav | _unknown_ | move | move 0.93 | home 0.03 | blue box 0.01 | 26.41 | S0 old wiring (VDD ~1.8 V) |

## Stream mode (raw 2 s takes, board logic: stride 10 frames, avg 3, threshold 0.7, refractory 1500 ms, channel auto)

command takes 94: **detected 0.266** (hit 25, hit+extra 0), wrong 4, miss 65 · first detection correct 0.266 · _unknown_ false alarms 5/15 · noise false alarms 0 in 0.0 min · channels {'ch1': 27, 'ch0/ch1': 16, 'ch0': 66}

| cls | takes | hit | hit_extra | wrong | miss | false_alarm | detected_rate | wrong_as | mean_max_p_true |
|---|---|---|---|---|---|---|---|---|---|
| move | 5 | 1 | 0 | 0 | 4 | 0 | 0.2 |  | 0.271 |
| pick | 2 | 1 | 0 | 0 | 1 | 0 | 0.5 |  | 0.502 |
| grab | 7 | 2 | 0 | 0 | 5 | 0 | 0.286 |  | 0.417 |
| blue box | 7 | 0 | 0 | 1 | 6 | 0 | 0.0 | no 1 | 0.065 |
| green box | 6 | 0 | 0 | 0 | 6 | 0 | 0.0 |  | 0.002 |
| red box | 3 | 1 | 0 | 1 | 1 | 0 | 0.333 | right 1 | 0.559 |
| black box | 4 | 1 | 0 | 0 | 3 | 0 | 0.25 |  | 0.19 |
| hand | 5 | 1 | 0 | 0 | 4 | 0 | 0.2 |  | 0.205 |
| station | 6 | 1 | 0 | 1 | 4 | 0 | 0.167 | base 1 | 0.186 |
| base | 5 | 1 | 0 | 0 | 4 | 0 | 0.2 |  | 0.153 |
| forward | 4 | 2 | 0 | 0 | 2 | 0 | 0.5 |  | 0.486 |
| backward | 7 | 0 | 0 | 1 | 6 | 0 | 0.0 | left 1 | 0.229 |
| left | 5 | 2 | 0 | 0 | 3 | 0 | 0.4 |  | 0.344 |
| right | 3 | 0 | 0 | 0 | 3 | 0 | 0.0 |  | 0.005 |
| place | 7 | 3 | 0 | 0 | 4 | 0 | 0.429 |  | 0.386 |
| drop | 2 | 2 | 0 | 0 | 0 | 0 | 1.0 |  | 0.826 |
| yes | 2 | 2 | 0 | 0 | 0 | 0 | 1.0 |  | 0.986 |
| no | 4 | 1 | 0 | 0 | 3 | 0 | 0.25 |  | 0.25 |
| home | 5 | 2 | 0 | 0 | 3 | 0 | 0.4 |  | 0.389 |
| stop | 5 | 2 | 0 | 0 | 3 | 0 | 0.4 |  | 0.396 |
| _unknown_ | 15 | 0 | 0 | 0 | 0 | 5 |  | right 1, home 1, no 1 | 0.05 |

### Takes that were not a clean hit

| raw_path | label | outcome | detections | max_avg_p_true | peak_top3 | channels_used |
|---|---|---|---|---|---|---|
| raw/supanut/S0/grab/grab_01_raw.wav | grab | miss |  | 0.636 | grab 0.64 | left 0.12 | right 0.06 | ch1 |
| raw/supanut/S0/blue_box/blue_box_01_raw.wav | blue box | miss |  | 0.002 | _silence_ 0.94 | pick 0.01 | left 0.01 | ch0 |
| raw/supanut/S0/home/home_02_raw.wav | home | miss |  | 0.003 | _silence_ 0.90 | pick 0.01 | left 0.01 | ch0 |
| raw/supanut/S0/red_box/red_box_01_raw.wav | red box | wrong | right@1.01s 0.78 | 0.916 | red box 0.92 | green box 0.04 | black box 0.02 | ch1 |
| raw/supanut/S0/backward/backward_01_raw.wav | backward | wrong | left@1.01s 0.82 | 0.997 | backward 1.00 | black box 0.00 | no 0.00 | ch1 |
| raw/supanut/S0/station/station_01_raw.wav | station | miss |  | 0.002 | _silence_ 0.94 | left 0.01 | pick 0.01 | ch0 |
| raw/supanut/S0/pick/pick_01_raw.wav | pick | miss |  | 0.011 | _silence_ 0.92 | pick 0.01 | left 0.01 | ch0 |
| raw/supanut/S0/black_box/black_box_02_raw.wav | black box | miss |  | 0.002 | _silence_ 0.84 | pick 0.03 | left 0.02 | ch0 |
| raw/supanut/S0/hand/hand_01_raw.wav | hand | miss |  | 0.008 | _silence_ 0.80 | left 0.03 | pick 0.02 | ch0 |
| raw/supanut/S0/move/move_02_raw.wav | move | miss |  | 0.005 | _silence_ 0.93 | pick 0.01 | left 0.01 | ch0 |
| raw/supanut/S0/backward/backward_02_raw.wav | backward | miss |  | 0.007 | _silence_ 0.85 | left 0.02 | pick 0.02 | ch0 |
| raw/supanut/S0/green_box/green_box_01_raw.wav | green box | miss |  | 0.001 | _silence_ 0.92 | pick 0.01 | left 0.01 | ch0 |
| raw/supanut/S0/base/base_01_raw.wav | base | miss |  | 0.006 | _silence_ 0.83 | pick 0.04 | drop 0.01 | ch0 |
| raw/supanut/S0/green_box/green_box_02_raw.wav | green box | miss |  | 0.002 | _silence_ 0.91 | pick 0.01 | left 0.01 | ch0 |
| raw/supanut/S0/no/no_01_raw.wav | no | miss |  | 0.004 | _silence_ 0.91 | move 0.01 | pick 0.01 | ch0 |
| raw/supanut/S0/blue_box/blue_box_02_raw.wav | blue box | miss |  | 0.001 | _silence_ 0.93 | pick 0.01 | left 0.01 | ch0 |
| raw/supanut/S0/place/place_02_raw.wav | place | miss |  | 0.005 | _silence_ 0.90 | pick 0.01 | left 0.01 | ch0 |
| raw/supanut/S0/hand/hand_02_raw.wav | hand | miss |  | 0.003 | _silence_ 0.95 | pick 0.01 | left 0.01 | ch0 |
| raw/supanut/S0/right/right_01_raw.wav | right | miss |  | 0.004 | _silence_ 0.91 | left 0.01 | move 0.01 | ch0 |
| raw/supanut/S0/grab/grab_02_raw.wav | grab | miss |  | 0.005 | _silence_ 0.90 | left 0.01 | drop 0.01 | ch0 |
| raw/supanut/S0/base/base_02_raw.wav | base | miss |  | 0.007 | _silence_ 0.83 | left 0.03 | yes 0.02 | ch0 |
| raw/supanut/S0/green_box/green_box_03_raw.wav | green box | miss |  | 0.002 | _silence_ 0.87 | pick 0.02 | left 0.01 | ch0 |
| raw/supanut/S0/backward/backward_03_raw.wav | backward | miss |  | 0.005 | _silence_ 0.91 | pick 0.01 | left 0.01 | ch0 |
| raw/supanut/S0/backward/backward_04_raw.wav | backward | miss |  | 0.004 | _silence_ 0.87 | pick 0.02 | left 0.01 | ch0 |
| raw/supanut/S0/right/right_02_raw.wav | right | miss |  | 0.004 | _silence_ 0.88 | move 0.01 | left 0.01 | ch0 |
| raw/supanut/S0/black_box/black_box_03_raw.wav | black box | miss |  | 0.001 | _silence_ 0.93 | pick 0.01 | left 0.01 | ch0 |
| raw/supanut/S0/green_box/green_box_04_raw.wav | green box | miss |  | 0.001 | _silence_ 0.92 | left 0.01 | pick 0.01 | ch0 |
| raw/supanut/S0/base/base_03_raw.wav | base | miss |  | 0.004 | _silence_ 0.92 | pick 0.01 | left 0.01 | ch0 |
| raw/supanut/S0/green_box/green_box_05_raw.wav | green box | miss |  | 0.003 | _silence_ 0.85 | pick 0.03 | drop 0.01 | ch0 |
| raw/supanut/S0/red_box/red_box_02_raw.wav | red box | miss |  | 0.001 | _silence_ 0.94 | pick 0.01 | left 0.01 | ch0 |
| raw/supanut/S0/stop/stop_02_raw.wav | stop | miss |  | 0.005 | _silence_ 0.93 | pick 0.01 | left 0.01 | ch0 |
| raw/supanut/S0/backward/backward_05_raw.wav | backward | miss |  | 0.002 | _silence_ 0.93 | left 0.01 | pick 0.01 | ch0 |
| raw/supanut/S0/blue_box/blue_box_03_raw.wav | blue box | miss |  | 0.001 | _silence_ 0.90 | pick 0.01 | left 0.01 | ch0 |
| raw/supanut/S0/home/home_03_raw.wav | home | miss |  | 0.005 | _silence_ 0.85 | pick 0.02 | left 0.02 | ch0 |
| raw/supanut/S0/green_box/green_box_06_raw.wav | green box | miss |  | 0.003 | _silence_ 0.85 | pick 0.02 | left 0.01 | ch0 |
| raw/supanut/S0/grab/grab_03_raw.wav | grab | miss |  | 0.01 | _silence_ 0.80 | pick 0.03 | drop 0.02 | ch0 |
| raw/supanut/S0/black_box/black_box_04_raw.wav | black box | miss |  | 0.001 | _silence_ 0.90 | pick 0.02 | left 0.01 | ch0 |
| raw/supanut/S0/left/left_01_raw.wav | left | miss |  | 0.009 | _silence_ 0.91 | pick 0.01 | left 0.01 | ch0 |
| raw/supanut/S0/grab/grab_05_raw.wav | grab | miss |  | 0.524 | grab 0.52 | left 0.31 | backward 0.03 | ch1 |
| raw/supanut/S0/move/move_03_raw.wav | move | miss |  | 0.006 | _silence_ 0.49 | red box 0.08 | drop 0.07 | ch0 |
| raw/supanut/S0/no/no_02_raw.wav | no | miss |  | 0.003 | _silence_ 0.88 | pick 0.02 | left 0.01 | ch0 |
| raw/supanut/S0/blue_box/blue_box_04_raw.wav | blue box | miss |  | 0.013 | _silence_ 0.79 | move 0.03 | left 0.03 | ch0 |
| raw/supanut/S0/stop/stop_03_raw.wav | stop | miss |  | 0.007 | _silence_ 0.88 | move 0.01 | left 0.01 | ch0 |
| raw/supanut/S0/place/place_04_raw.wav | place | miss |  | 0.01 | _silence_ 0.74 | backward 0.03 | red box 0.02 | ch0 |
| raw/supanut/S0/grab/grab_06_raw.wav | grab | miss |  | 0.005 | _silence_ 0.83 | move 0.02 | blue box 0.01 | ch0 |
| raw/supanut/S0/forward/forward_01_raw.wav | forward | miss |  | 0.063 | _silence_ 0.54 | left 0.18 | move 0.03 | ch0 |
| raw/supanut/S0/station/station_03_raw.wav | station | miss |  | 0.009 | _silence_ 0.46 | yes 0.08 | red box 0.08 | ch0 |
| raw/supanut/S0/backward/backward_06_raw.wav | backward | miss |  | 0.009 | _silence_ 0.63 | stop 0.06 | drop 0.05 | ch0 |
| raw/supanut/S0/place/place_06_raw.wav | place | miss |  | 0.005 | _silence_ 0.88 | pick 0.01 | left 0.01 | ch0 |
| raw/supanut/S0/hand/hand_03_raw.wav | hand | miss |  | 0.012 | _silence_ 0.91 | pick 0.03 | move 0.01 | ch0 |
| raw/supanut/S0/hand/hand_04_raw.wav | hand | miss |  | 0.008 | _silence_ 0.80 | left 0.03 | yes 0.01 | ch0 |
| raw/supanut/S0/station/station_04_raw.wav | station | miss |  | 0.004 | _silence_ 0.91 | left 0.02 | move 0.01 | ch0 |
| raw/supanut/S0/forward/forward_02_raw.wav | forward | miss |  | 0.009 | _silence_ 0.60 | left 0.11 | move 0.03 | ch0 |
| raw/supanut/S0/move/move_04_raw.wav | move | miss |  | 0.019 | _silence_ 0.69 | pick 0.04 | left 0.03 | ch0 |
| raw/supanut/S0/station/station_05_raw.wav | station | miss |  | 0.004 | _silence_ 0.90 | yes 0.01 | green box 0.01 | ch0 |
| raw/supanut/S0/backward/backward_07_raw.wav | backward | miss |  | 0.576 | backward 0.58 | left 0.24 | _unknown_ 0.03 | ch0/ch1 |
| raw/supanut/S0/station/station_06_raw.wav | station | wrong | base@1.31s 0.72 | 0.293 | base 0.72 | station 0.11 | place 0.05 | ch0/ch1 |
| raw/supanut/S0/home/home_04_raw.wav | home | miss |  | 0.007 | _silence_ 0.73 | drop 0.04 | red box 0.03 | ch0 |
| raw/supanut/S0/right/right_03_raw.wav | right | miss |  | 0.007 | _silence_ 0.72 | pick 0.03 | drop 0.03 | ch0 |
| raw/supanut/S0/_unknown_/bright_01_raw.wav | _unknown_ | false_alarm | right@1.31s 0.99 | 0.003 | right 1.00 | _unknown_ 0.00 | grab 0.00 | ch0/ch1 |
| raw/supanut/S0/left/left_03_raw.wav | left | miss |  | 0.008 | _silence_ 0.89 | pick 0.02 | drop 0.01 | ch0 |
| raw/supanut/S0/_unknown_/open_01_raw.wav | _unknown_ | false_alarm | home@1.01s 0.84 | 0.233 | home 0.84 | forward 0.06 | _unknown_ 0.03 | ch0/ch1 |
| raw/supanut/S0/blue_box/blue_box_05_raw.wav | blue box | wrong | no@1.01s 0.78 | 0.156 | no 0.78 | move 0.08 | _unknown_ 0.06 | ch1 |
| raw/supanut/S0/stop/stop_04_raw.wav | stop | miss |  | 0.004 | _silence_ 0.84 | green box 0.03 | yes 0.02 | ch0 |
| raw/supanut/S0/blue_box/blue_box_06_raw.wav | blue box | miss |  | 0.222 | no 0.60 | move 0.14 | _unknown_ 0.08 | ch1 |
| raw/supanut/S0/base/base_04_raw.wav | base | miss |  | 0.008 | _silence_ 0.84 | green box 0.02 | red box 0.01 | ch0 |
| raw/supanut/S0/blue_box/blue_box_07_raw.wav | blue box | miss |  | 0.063 | no 0.63 | move 0.13 | _unknown_ 0.09 | ch0/ch1 |
| raw/supanut/S0/left/left_05_raw.wav | left | miss |  | 0.019 | _silence_ 0.86 | left 0.02 | move 0.02 | ch0 |
| raw/supanut/S0/place/place_07_raw.wav | place | miss |  | 0.02 | _silence_ 0.61 | move 0.06 | home 0.03 | ch0 |
| raw/supanut/S0/no/no_03_raw.wav | no | miss |  | 0.007 | _silence_ 0.86 | pick 0.02 | left 0.01 | ch0 |
| raw/supanut/S0/_unknown_/blue_01_raw.wav | _unknown_ | false_alarm | no@1.31s 0.82 | 0.043 | no 0.86 | move 0.11 | _unknown_ 0.01 | ch1 |
| raw/supanut/S0/move/move_05_raw.wav | move | miss |  | 0.467 | move 0.47 | hand 0.28 | no 0.10 | ch1 |
| raw/supanut/S0/_unknown_/bag_01_raw.wav | _unknown_ | false_alarm | left@1.01s 0.71 | 0.016 | left 0.76 | backward 0.06 | pick 0.04 | ch1 |
| raw/supanut/S0/_unknown_/groove_01_raw.wav | _unknown_ | false_alarm | move@1.01s 0.87 | 0.025 | move 0.92 | home 0.04 | blue box 0.01 | ch1 |
