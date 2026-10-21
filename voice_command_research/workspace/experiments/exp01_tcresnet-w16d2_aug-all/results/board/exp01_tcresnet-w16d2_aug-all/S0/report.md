# Board evaluation - exp01_tcresnet-w16d2_aug-all on S0

model `D:\Microcon69\ProjectRoboticArm\ProjectSpace\2026-micro-con\voice_command_research\workspace\experiments\exp01_tcresnet-w16d2_aug-all\runs\final.pth` (pytorch TC-ResNet w16 d2, 23 classes) · gain +15.0 dB · features librosa 0.11.0 (features.py) · 2026-10-21 19:01

## Clip mode (1 s clips, mono ch1)

n 109 · **accuracy 0.8257** · **macro-F1 0.8384** · commands 0.9574 · _unknown_ 0.0 · _silence_ None · commands rejected as unknown/silence 0 · speech peak median -32.4 dB (training clip -25.4 dB)

| cls | support | correct | recall | precision | f1 | mean_p_true | confused_with |
|---|---|---|---|---|---|---|---|
| move | 5 | 4 | 0.8 | 0.571 | 0.667 | 0.613 | hand 1 |
| pick | 2 | 2 | 1.0 | 0.667 | 0.8 | 0.995 |  |
| grab | 7 | 7 | 1.0 | 1.0 | 1.0 | 0.824 |  |
| blue box | 7 | 6 | 0.857 | 1.0 | 0.923 | 0.471 | no 1 |
| green box | 6 | 6 | 1.0 | 1.0 | 1.0 | 0.957 |  |
| red box | 3 | 3 | 1.0 | 0.75 | 0.857 | 0.951 |  |
| black box | 4 | 4 | 1.0 | 1.0 | 1.0 | 0.686 |  |
| hand | 5 | 5 | 1.0 | 0.833 | 0.909 | 0.938 |  |
| station | 6 | 6 | 1.0 | 1.0 | 1.0 | 0.857 |  |
| base | 5 | 5 | 1.0 | 0.833 | 0.909 | 0.812 |  |
| forward | 4 | 4 | 1.0 | 1.0 | 1.0 | 0.965 |  |
| backward | 7 | 7 | 1.0 | 1.0 | 1.0 | 0.996 |  |
| left | 5 | 5 | 1.0 | 0.833 | 0.909 | 0.886 |  |
| right | 3 | 3 | 1.0 | 0.5 | 0.667 | 0.999 |  |
| place | 7 | 6 | 0.857 | 0.857 | 0.857 | 0.658 | base 1 |
| drop | 2 | 2 | 1.0 | 1.0 | 1.0 | 0.808 |  |
| yes | 2 | 1 | 0.5 | 1.0 | 0.667 | 0.508 | place 1 |
| no | 4 | 4 | 1.0 | 0.571 | 0.727 | 0.982 |  |
| home | 5 | 5 | 1.0 | 0.556 | 0.714 | 0.95 |  |
| stop | 5 | 5 | 1.0 | 1.0 | 1.0 | 0.985 |  |
| _unknown_ | 15 | 0 | 0.0 | 0.0 | 0.0 | 0.037 | home 4, move 3, right 3 |

### Mistakes (top-3)

| clip_path | label | pred | top3 | snr_db | flags |
|---|---|---|---|---|---|
| clips/supanut/S0/_unknown_/pink_01.wav | _unknown_ | pick | pick 0.94 | _unknown_ 0.02 | hand 0.02 | 19.61 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/move/move_02.wav | move | hand | hand 0.60 | move 0.10 | pick 0.08 | 22.06 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/nose_01.wav | _unknown_ | no | no 0.95 | _unknown_ 0.01 | red box 0.01 | 24.64 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/write_01.wav | _unknown_ | right | right 1.00 | _unknown_ 0.00 | grab 0.00 | 25.15 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/mood_01.wav | _unknown_ | move | move 0.82 | forward 0.04 | _unknown_ 0.03 | 22.31 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/blue_box/blue_box_03.wav | blue box | no | no 0.31 | blue box 0.30 | move 0.07 | 23.84 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/phone_01.wav | _unknown_ | home | home 0.47 | forward 0.43 | _unknown_ 0.05 | 24.42 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/place/place_06.wav | place | base | base 0.50 | place 0.32 | yes 0.04 | 18.83 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/moon_01.wav | _unknown_ | move | move 0.58 | home 0.22 | forward 0.08 | 23.7 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/go_01.wav | _unknown_ | home | home 0.79 | hand 0.05 | _unknown_ 0.02 | 27.44 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/foam_01.wav | _unknown_ | home | home 0.90 | right 0.03 | forward 0.02 | 26.15 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/bright_01.wav | _unknown_ | right | right 1.00 | grab 0.00 | _unknown_ 0.00 | 24.09 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/open_01.wav | _unknown_ | home | home 0.65 | forward 0.11 | _unknown_ 0.10 | 24.52 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/yes/yes_02.wav | yes | place | place 0.37 | base 0.20 | _silence_ 0.10 | 16.0 | sound longer than 1 s - the clip cuts it; S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/robot_01.wav | _unknown_ | red box | red box 0.41 | green box 0.12 | _unknown_ 0.12 | 22.74 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/wait_01.wav | _unknown_ | right | right 0.47 | place 0.26 | _unknown_ 0.11 | 19.73 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/blue_01.wav | _unknown_ | no | no 0.86 | move 0.12 | _unknown_ 0.01 | 23.87 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/bag_01.wav | _unknown_ | left | left 0.87 | move 0.03 | backward 0.03 | 17.09 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/groove_01.wav | _unknown_ | move | move 0.81 | home 0.08 | _unknown_ 0.03 | 26.41 | S0 old wiring (VDD ~1.8 V) |

## Stream mode (raw 2 s takes, board logic: stride 10 frames, avg 3, threshold 0.7, refractory 1500 ms, channel auto)

command takes 94: **detected 0.2872** (hit 27, hit+extra 0), wrong 3, miss 64 · first detection correct 0.2872 · _unknown_ false alarms 5/15 · noise false alarms 0 in 0.0 min · channels {'ch1': 27, 'ch0/ch1': 16, 'ch0': 66}

| cls | takes | hit | hit_extra | wrong | miss | false_alarm | detected_rate | wrong_as | mean_max_p_true |
|---|---|---|---|---|---|---|---|---|---|
| move | 5 | 1 | 0 | 0 | 4 | 0 | 0.2 |  | 0.303 |
| pick | 2 | 1 | 0 | 0 | 1 | 0 | 0.5 |  | 0.501 |
| grab | 7 | 4 | 0 | 0 | 3 | 0 | 0.571 |  | 0.475 |
| blue box | 7 | 0 | 0 | 1 | 6 | 0 | 0.0 | no 1 | 0.174 |
| green box | 6 | 0 | 0 | 0 | 6 | 0 | 0.0 |  | 0.003 |
| red box | 3 | 1 | 0 | 1 | 1 | 0 | 0.333 | right 1 | 0.633 |
| black box | 4 | 1 | 0 | 0 | 3 | 0 | 0.25 |  | 0.182 |
| hand | 5 | 1 | 0 | 0 | 4 | 0 | 0.2 |  | 0.203 |
| station | 6 | 2 | 0 | 0 | 4 | 0 | 0.333 |  | 0.275 |
| base | 5 | 1 | 0 | 0 | 4 | 0 | 0.2 |  | 0.144 |
| forward | 4 | 2 | 0 | 0 | 2 | 0 | 0.5 |  | 0.486 |
| backward | 7 | 0 | 0 | 1 | 6 | 0 | 0.0 | left 1 | 0.206 |
| left | 5 | 2 | 0 | 0 | 3 | 0 | 0.4 |  | 0.358 |
| right | 3 | 0 | 0 | 0 | 3 | 0 | 0.0 |  | 0.005 |
| place | 7 | 2 | 0 | 0 | 5 | 0 | 0.286 |  | 0.362 |
| drop | 2 | 2 | 0 | 0 | 0 | 0 | 1.0 |  | 0.833 |
| yes | 2 | 2 | 0 | 0 | 0 | 0 | 1.0 |  | 0.988 |
| no | 4 | 1 | 0 | 0 | 3 | 0 | 0.25 |  | 0.252 |
| home | 5 | 2 | 0 | 0 | 3 | 0 | 0.4 |  | 0.393 |
| stop | 5 | 2 | 0 | 0 | 3 | 0 | 0.4 |  | 0.399 |
| _unknown_ | 15 | 0 | 0 | 0 | 0 | 5 |  | right 1, home 1, no 1 | 0.044 |

### Takes that were not a clean hit

| raw_path | label | outcome | detections | max_avg_p_true | peak_top3 | channels_used |
|---|---|---|---|---|---|---|
| raw/supanut/S0/blue_box/blue_box_01_raw.wav | blue box | miss |  | 0.001 | _silence_ 0.93 | left 0.01 | pick 0.01 | ch0 |
| raw/supanut/S0/home/home_02_raw.wav | home | miss |  | 0.011 | _silence_ 0.85 | left 0.02 | pick 0.02 | ch0 |
| raw/supanut/S0/red_box/red_box_01_raw.wav | red box | wrong | right@1.01s 0.76 | 0.971 | red box 0.97 | green box 0.01 | black box 0.01 | ch1 |
| raw/supanut/S0/backward/backward_01_raw.wav | backward | wrong | left@1.01s 0.79 | 0.997 | backward 1.00 | black box 0.00 | left 0.00 | ch1 |
| raw/supanut/S0/station/station_01_raw.wav | station | miss |  | 0.001 | _silence_ 0.96 | left 0.00 | pick 0.00 | ch0 |
| raw/supanut/S0/pick/pick_01_raw.wav | pick | miss |  | 0.009 | _silence_ 0.92 | left 0.01 | pick 0.01 | ch0 |
| raw/supanut/S0/black_box/black_box_02_raw.wav | black box | miss |  | 0.002 | _silence_ 0.87 | pick 0.02 | left 0.02 | ch0 |
| raw/supanut/S0/hand/hand_01_raw.wav | hand | miss |  | 0.007 | _silence_ 0.92 | left 0.01 | pick 0.01 | ch0 |
| raw/supanut/S0/move/move_02_raw.wav | move | miss |  | 0.003 | _silence_ 0.95 | left 0.01 | pick 0.01 | ch0 |
| raw/supanut/S0/backward/backward_02_raw.wav | backward | miss |  | 0.005 | _silence_ 0.91 | pick 0.01 | left 0.01 | ch0 |
| raw/supanut/S0/green_box/green_box_01_raw.wav | green box | miss |  | 0.003 | _silence_ 0.83 | left 0.02 | pick 0.02 | ch0 |
| raw/supanut/S0/base/base_01_raw.wav | base | miss |  | 0.004 | _silence_ 0.89 | left 0.01 | pick 0.01 | ch0 |
| raw/supanut/S0/green_box/green_box_02_raw.wav | green box | miss |  | 0.001 | _silence_ 0.95 | left 0.01 | pick 0.01 | ch0 |
| raw/supanut/S0/no/no_01_raw.wav | no | miss |  | 0.002 | _silence_ 0.94 | left 0.01 | home 0.01 | ch0 |
| raw/supanut/S0/blue_box/blue_box_02_raw.wav | blue box | miss |  | 0.001 | _silence_ 0.91 | pick 0.01 | left 0.01 | ch0 |
| raw/supanut/S0/place/place_02_raw.wav | place | miss |  | 0.002 | _silence_ 0.95 | pick 0.01 | left 0.01 | ch0 |
| raw/supanut/S0/hand/hand_02_raw.wav | hand | miss |  | 0.005 | _silence_ 0.92 | backward 0.01 | stop 0.01 | ch0 |
| raw/supanut/S0/right/right_01_raw.wav | right | miss |  | 0.004 | _silence_ 0.89 | left 0.01 | pick 0.01 | ch0 |
| raw/supanut/S0/grab/grab_02_raw.wav | grab | miss |  | 0.006 | _silence_ 0.89 | left 0.02 | pick 0.01 | ch0 |
| raw/supanut/S0/base/base_02_raw.wav | base | miss |  | 0.003 | _silence_ 0.95 | left 0.01 | pick 0.00 | ch0 |
| raw/supanut/S0/green_box/green_box_03_raw.wav | green box | miss |  | 0.001 | _silence_ 0.97 | left 0.00 | pick 0.00 | ch0 |
| raw/supanut/S0/backward/backward_03_raw.wav | backward | miss |  | 0.002 | _silence_ 0.97 | left 0.00 | home 0.00 | ch0 |
| raw/supanut/S0/backward/backward_04_raw.wav | backward | miss |  | 0.002 | _silence_ 0.96 | hand 0.00 | left 0.00 | ch0 |
| raw/supanut/S0/right/right_02_raw.wav | right | miss |  | 0.001 | _silence_ 0.96 | hand 0.00 | left 0.00 | ch0 |
| raw/supanut/S0/black_box/black_box_03_raw.wav | black box | miss |  | 0.001 | _silence_ 0.92 | left 0.01 | home 0.01 | ch0 |
| raw/supanut/S0/green_box/green_box_04_raw.wav | green box | miss |  | 0.003 | _silence_ 0.88 | left 0.02 | pick 0.01 | ch0 |
| raw/supanut/S0/base/base_03_raw.wav | base | miss |  | 0.005 | _silence_ 0.87 | left 0.01 | move 0.01 | ch0 |
| raw/supanut/S0/green_box/green_box_05_raw.wav | green box | miss |  | 0.006 | _silence_ 0.88 | left 0.02 | pick 0.02 | ch0 |
| raw/supanut/S0/red_box/red_box_02_raw.wav | red box | miss |  | 0.002 | _silence_ 0.93 | left 0.01 | pick 0.01 | ch0 |
| raw/supanut/S0/stop/stop_02_raw.wav | stop | miss |  | 0.004 | _silence_ 0.93 | pick 0.01 | left 0.01 | ch0 |
| raw/supanut/S0/backward/backward_05_raw.wav | backward | miss |  | 0.003 | _silence_ 0.92 | left 0.01 | pick 0.01 | ch0 |
| raw/supanut/S0/blue_box/blue_box_03_raw.wav | blue box | miss |  | 0.001 | _silence_ 0.96 | left 0.01 | home 0.00 | ch0 |
| raw/supanut/S0/home/home_03_raw.wav | home | miss |  | 0.004 | _silence_ 0.95 | left 0.01 | pick 0.01 | ch0 |
| raw/supanut/S0/green_box/green_box_06_raw.wav | green box | miss |  | 0.001 | _silence_ 0.92 | left 0.01 | pick 0.01 | ch0 |
| raw/supanut/S0/grab/grab_03_raw.wav | grab | miss |  | 0.007 | _silence_ 0.87 | left 0.01 | drop 0.01 | ch0 |
| raw/supanut/S0/black_box/black_box_04_raw.wav | black box | miss |  | 0.001 | _silence_ 0.94 | left 0.01 | pick 0.01 | ch0 |
| raw/supanut/S0/left/left_01_raw.wav | left | miss |  | 0.01 | _silence_ 0.90 | pick 0.01 | left 0.01 | ch0 |
| raw/supanut/S0/move/move_03_raw.wav | move | miss |  | 0.009 | _silence_ 0.70 | drop 0.06 | left 0.03 | ch0 |
| raw/supanut/S0/no/no_02_raw.wav | no | miss |  | 0.005 | _silence_ 0.87 | left 0.01 | drop 0.01 | ch0 |
| raw/supanut/S0/blue_box/blue_box_04_raw.wav | blue box | miss |  | 0.006 | _silence_ 0.85 | left 0.02 | hand 0.01 | ch0 |
| raw/supanut/S0/stop/stop_03_raw.wav | stop | miss |  | 0.012 | _silence_ 0.83 | left 0.01 | stop 0.01 | ch0 |
| raw/supanut/S0/place/place_04_raw.wav | place | miss |  | 0.004 | _silence_ 0.91 | left 0.01 | move 0.01 | ch0 |
| raw/supanut/S0/grab/grab_06_raw.wav | grab | miss |  | 0.005 | _silence_ 0.85 | backward 0.02 | stop 0.01 | ch0 |
| raw/supanut/S0/forward/forward_01_raw.wav | forward | miss |  | 0.001 | _silence_ 0.97 | home 0.00 | drop 0.00 | ch0 |
| raw/supanut/S0/station/station_03_raw.wav | station | miss |  | 0.007 | _silence_ 0.83 | drop 0.03 | left 0.02 | ch0 |
| raw/supanut/S0/place/place_05_raw.wav | place | miss |  | 0.693 | place 0.69 | base 0.18 | right 0.05 | ch1 |
| raw/supanut/S0/backward/backward_06_raw.wav | backward | miss |  | 0.004 | _silence_ 0.81 | left 0.06 | drop 0.03 | ch0 |
| raw/supanut/S0/place/place_06_raw.wav | place | miss |  | 0.001 | _silence_ 0.98 | left 0.00 | drop 0.00 | ch0 |
| raw/supanut/S0/hand/hand_03_raw.wav | hand | miss |  | 0.003 | _silence_ 0.96 | stop 0.01 | drop 0.00 | ch0 |
| raw/supanut/S0/hand/hand_04_raw.wav | hand | miss |  | 0.003 | _silence_ 0.93 | left 0.01 | home 0.01 | ch0 |
| raw/supanut/S0/station/station_04_raw.wav | station | miss |  | 0.0 | _silence_ 0.99 | home 0.00 | left 0.00 | ch0 |
| raw/supanut/S0/forward/forward_02_raw.wav | forward | miss |  | 0.001 | _silence_ 0.95 | black box 0.01 | home 0.00 | ch0 |
| raw/supanut/S0/move/move_04_raw.wav | move | miss |  | 0.007 | _silence_ 0.86 | left 0.02 | hand 0.02 | ch0 |
| raw/supanut/S0/station/station_05_raw.wav | station | miss |  | 0.007 | _silence_ 0.86 | drop 0.04 | left 0.02 | ch0 |
| raw/supanut/S0/backward/backward_07_raw.wav | backward | miss |  | 0.429 | backward 0.43 | left 0.22 | forward 0.06 | ch0/ch1 |
| raw/supanut/S0/home/home_04_raw.wav | home | miss |  | 0.014 | _silence_ 0.71 | drop 0.08 | left 0.02 | ch0 |
| raw/supanut/S0/right/right_03_raw.wav | right | miss |  | 0.009 | _silence_ 0.80 | left 0.03 | pick 0.02 | ch0 |
| raw/supanut/S0/_unknown_/bright_01_raw.wav | _unknown_ | false_alarm | right@1.31s 1.00 | 0.003 | right 1.00 | grab 0.00 | _unknown_ 0.00 | ch0/ch1 |
| raw/supanut/S0/left/left_03_raw.wav | left | miss |  | 0.017 | _silence_ 0.87 | drop 0.02 | left 0.02 | ch0 |
| raw/supanut/S0/_unknown_/open_01_raw.wav | _unknown_ | false_alarm | home@1.01s 0.84 | 0.156 | home 0.84 | forward 0.03 | hand 0.02 | ch0/ch1 |
| raw/supanut/S0/blue_box/blue_box_05_raw.wav | blue box | miss |  | 0.629 | blue box 0.63 | no 0.10 | green box 0.10 | ch1 |
| raw/supanut/S0/stop/stop_04_raw.wav | stop | miss |  | 0.013 | _silence_ 0.59 | drop 0.05 | backward 0.04 | ch0 |
| raw/supanut/S0/blue_box/blue_box_06_raw.wav | blue box | wrong | no@1.01s 0.71 | 0.321 | no 0.71 | grab 0.05 | _unknown_ 0.04 | ch1 |
| raw/supanut/S0/base/base_04_raw.wav | base | miss |  | 0.005 | _silence_ 0.77 | drop 0.03 | left 0.03 | ch0 |
| raw/supanut/S0/blue_box/blue_box_07_raw.wav | blue box | miss |  | 0.257 | no 0.53 | move 0.22 | blue box 0.06 | ch0/ch1 |
| raw/supanut/S0/left/left_05_raw.wav | left | miss |  | 0.009 | _silence_ 0.94 | left 0.01 | pick 0.01 | ch0 |
| raw/supanut/S0/place/place_07_raw.wav | place | miss |  | 0.005 | _silence_ 0.82 | backward 0.02 | stop 0.02 | ch0 |
| raw/supanut/S0/no/no_03_raw.wav | no | miss |  | 0.012 | _silence_ 0.63 | pick 0.13 | drop 0.04 | ch0 |
| raw/supanut/S0/_unknown_/blue_01_raw.wav | _unknown_ | false_alarm | no@1.31s 0.75 | 0.039 | no 0.85 | move 0.13 | _unknown_ 0.01 | ch1 |
| raw/supanut/S0/move/move_05_raw.wav | move | miss |  | 0.545 | move 0.54 | no 0.16 | hand 0.10 | ch1 |
| raw/supanut/S0/_unknown_/bag_01_raw.wav | _unknown_ | false_alarm | left@1.01s 0.91 | 0.021 | left 0.91 | move 0.02 | backward 0.01 | ch1 |
| raw/supanut/S0/_unknown_/groove_01_raw.wav | _unknown_ | false_alarm | move@1.01s 0.76 | 0.043 | move 0.77 | home 0.11 | _unknown_ 0.03 | ch1 |
