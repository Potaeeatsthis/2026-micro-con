# Board evaluation - exp05_tcresnet-w24d2_aug-all on S0

model `D:\Microcon69\ProjectRoboticArm\ProjectSpace\2026-micro-con\voice_command_research\workspace\experiments\exp05_tcresnet-w24d2_aug-all\runs\final.pth` (pytorch exp05_tcresnet-w24d2_aug-all, 23 classes) · gain +15.0 dB · features librosa 0.11.0 (features.py) · 2026-10-21 21:03

## Clip mode (1 s clips, mono ch1)

n 109 · **accuracy 0.8532** · **macro-F1 0.8623** · commands 0.9681 · _unknown_ 0.1333 · _silence_ None · commands rejected as unknown/silence 0 · speech peak median -32.4 dB (training clip -25.4 dB)

| cls | support | correct | recall | precision | f1 | mean_p_true | confused_with |
|---|---|---|---|---|---|---|---|
| move | 5 | 4 | 0.8 | 0.667 | 0.727 | 0.737 | hand 1 |
| pick | 2 | 2 | 1.0 | 0.667 | 0.8 | 0.996 |  |
| grab | 7 | 7 | 1.0 | 1.0 | 1.0 | 0.819 |  |
| blue box | 7 | 7 | 1.0 | 1.0 | 1.0 | 0.787 |  |
| green box | 6 | 6 | 1.0 | 0.857 | 0.923 | 0.965 |  |
| red box | 3 | 2 | 0.667 | 1.0 | 0.8 | 0.667 | green box 1 |
| black box | 4 | 4 | 1.0 | 1.0 | 1.0 | 0.543 |  |
| hand | 5 | 5 | 1.0 | 0.833 | 0.909 | 0.979 |  |
| station | 6 | 6 | 1.0 | 1.0 | 1.0 | 0.971 |  |
| base | 5 | 5 | 1.0 | 1.0 | 1.0 | 0.764 |  |
| forward | 4 | 4 | 1.0 | 1.0 | 1.0 | 0.922 |  |
| backward | 7 | 7 | 1.0 | 0.875 | 0.933 | 0.993 |  |
| left | 5 | 5 | 1.0 | 1.0 | 1.0 | 0.97 |  |
| right | 3 | 3 | 1.0 | 0.5 | 0.667 | 0.999 |  |
| place | 7 | 7 | 1.0 | 0.875 | 0.933 | 0.915 |  |
| drop | 2 | 2 | 1.0 | 1.0 | 1.0 | 0.897 |  |
| yes | 2 | 1 | 0.5 | 1.0 | 0.667 | 0.509 | place 1 |
| no | 4 | 4 | 1.0 | 0.667 | 0.8 | 0.989 |  |
| home | 5 | 5 | 1.0 | 0.556 | 0.714 | 0.963 |  |
| stop | 5 | 5 | 1.0 | 1.0 | 1.0 | 0.987 |  |
| _unknown_ | 15 | 2 | 0.133 | 1.0 | 0.235 | 0.087 | home 4, right 3, move 2 |

### Mistakes (top-3)

| clip_path | label | pred | top3 | snr_db | flags |
|---|---|---|---|---|---|
| clips/supanut/S0/_unknown_/pink_01.wav | _unknown_ | pick | pick 0.95 | _unknown_ 0.02 | base 0.01 | 19.61 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/move/move_02.wav | move | hand | hand 0.28 | pick 0.26 | move 0.14 | 22.06 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/nose_01.wav | _unknown_ | no | no 0.94 | move 0.03 | _unknown_ 0.01 | 24.64 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/write_01.wav | _unknown_ | right | right 1.00 | _unknown_ 0.00 | left 0.00 | 25.15 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/mood_01.wav | _unknown_ | move | move 0.68 | forward 0.14 | home 0.05 | 22.31 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/phone_01.wav | _unknown_ | home | home 0.76 | forward 0.17 | _unknown_ 0.04 | 24.42 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/moon_01.wav | _unknown_ | home | home 0.40 | move 0.19 | forward 0.15 | 23.7 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/go_01.wav | _unknown_ | home | home 0.77 | forward 0.03 | no 0.03 | 27.44 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/foam_01.wav | _unknown_ | home | home 0.96 | _unknown_ 0.01 | forward 0.01 | 26.15 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/red_box/red_box_03.wav | red box | green box | green box 0.57 | red box 0.28 | base 0.04 | 23.48 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/bright_01.wav | _unknown_ | right | right 1.00 | _unknown_ 0.00 | red box 0.00 | 24.09 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/yes/yes_02.wav | yes | place | place 0.81 | base 0.08 | yes 0.03 | 16.0 | sound longer than 1 s - the clip cuts it; S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/wait_01.wav | _unknown_ | right | right 0.46 | place 0.35 | _unknown_ 0.09 | 19.73 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/blue_01.wav | _unknown_ | no | no 0.89 | move 0.08 | _unknown_ 0.01 | 23.87 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/bag_01.wav | _unknown_ | backward | backward 0.59 | left 0.19 | right 0.10 | 17.09 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/groove_01.wav | _unknown_ | move | move 0.77 | home 0.06 | no 0.04 | 26.41 | S0 old wiring (VDD ~1.8 V) |

## Stream mode (raw 2 s takes, board logic: stride 10 frames, avg 3, threshold 0.7, refractory 1500 ms, channel auto)

command takes 94: **detected 0.3085** (hit 29, hit+extra 0), wrong 1, miss 64 · first detection correct 0.3085 · _unknown_ false alarms 4/15 · noise false alarms 0 in 0.0 min · channels {'ch1': 27, 'ch0/ch1': 16, 'ch0': 66}

| cls | takes | hit | hit_extra | wrong | miss | false_alarm | detected_rate | wrong_as | mean_max_p_true |
|---|---|---|---|---|---|---|---|---|---|
| move | 5 | 2 | 0 | 0 | 3 | 0 | 0.4 |  | 0.352 |
| pick | 2 | 1 | 0 | 0 | 1 | 0 | 0.5 |  | 0.5 |
| grab | 7 | 3 | 0 | 0 | 4 | 0 | 0.429 |  | 0.472 |
| blue box | 7 | 2 | 0 | 0 | 5 | 0 | 0.286 |  | 0.283 |
| green box | 6 | 0 | 0 | 0 | 6 | 0 | 0.0 |  | 0.001 |
| red box | 3 | 0 | 0 | 1 | 2 | 0 | 0.0 | right 1 | 0.369 |
| black box | 4 | 0 | 0 | 0 | 4 | 0 | 0.0 |  | 0.123 |
| hand | 5 | 1 | 0 | 0 | 4 | 0 | 0.2 |  | 0.206 |
| station | 6 | 2 | 0 | 0 | 4 | 0 | 0.333 |  | 0.326 |
| base | 5 | 1 | 0 | 0 | 4 | 0 | 0.2 |  | 0.157 |
| forward | 4 | 2 | 0 | 0 | 2 | 0 | 0.5 |  | 0.474 |
| backward | 7 | 1 | 0 | 0 | 6 | 0 | 0.143 |  | 0.224 |
| left | 5 | 2 | 0 | 0 | 3 | 0 | 0.4 |  | 0.416 |
| right | 3 | 0 | 0 | 0 | 3 | 0 | 0.0 |  | 0.008 |
| place | 7 | 3 | 0 | 0 | 4 | 0 | 0.429 |  | 0.437 |
| drop | 2 | 2 | 0 | 0 | 0 | 0 | 1.0 |  | 0.924 |
| yes | 2 | 2 | 0 | 0 | 0 | 0 | 1.0 |  | 0.986 |
| no | 4 | 1 | 0 | 0 | 3 | 0 | 0.25 |  | 0.253 |
| home | 5 | 2 | 0 | 0 | 3 | 0 | 0.4 |  | 0.39 |
| stop | 5 | 2 | 0 | 0 | 3 | 0 | 0.4 |  | 0.407 |
| _unknown_ | 15 | 0 | 0 | 0 | 0 | 4 |  | right 1, home 1, no 1 | 0.081 |

### Takes that were not a clean hit

| raw_path | label | outcome | detections | max_avg_p_true | peak_top3 | channels_used |
|---|---|---|---|---|---|---|
| raw/supanut/S0/black_box/black_box_01_raw.wav | black box | miss |  | 0.491 | left 0.69 | backward 0.08 | yes 0.06 | ch1 |
| raw/supanut/S0/grab/grab_01_raw.wav | grab | miss |  | 0.53 | grab 0.53 | right 0.34 | left 0.05 | ch1 |
| raw/supanut/S0/blue_box/blue_box_01_raw.wav | blue box | miss |  | 0.0 | _silence_ 0.96 | pick 0.00 | forward 0.00 | ch0 |
| raw/supanut/S0/home/home_02_raw.wav | home | miss |  | 0.003 | _silence_ 0.94 | stop 0.01 | base 0.01 | ch0 |
| raw/supanut/S0/red_box/red_box_01_raw.wav | red box | wrong | right@1.01s 0.85 | 0.848 | red box 0.85 | green box 0.05 | right 0.02 | ch1 |
| raw/supanut/S0/station/station_01_raw.wav | station | miss |  | 0.001 | _silence_ 0.95 | pick 0.01 | stop 0.01 | ch0 |
| raw/supanut/S0/pick/pick_01_raw.wav | pick | miss |  | 0.007 | _silence_ 0.94 | pick 0.01 | stop 0.00 | ch0 |
| raw/supanut/S0/black_box/black_box_02_raw.wav | black box | miss |  | 0.0 | _silence_ 0.97 | pick 0.00 | stop 0.00 | ch0 |
| raw/supanut/S0/hand/hand_01_raw.wav | hand | miss |  | 0.004 | _silence_ 0.92 | left 0.01 | stop 0.01 | ch0 |
| raw/supanut/S0/move/move_02_raw.wav | move | miss |  | 0.002 | _silence_ 0.93 | pick 0.01 | left 0.01 | ch0 |
| raw/supanut/S0/backward/backward_02_raw.wav | backward | miss |  | 0.002 | _silence_ 0.95 | pick 0.00 | place 0.00 | ch0 |
| raw/supanut/S0/green_box/green_box_01_raw.wav | green box | miss |  | 0.002 | _silence_ 0.80 | backward 0.02 | no 0.02 | ch0 |
| raw/supanut/S0/base/base_01_raw.wav | base | miss |  | 0.006 | _silence_ 0.90 | pick 0.01 | stop 0.01 | ch0 |
| raw/supanut/S0/green_box/green_box_02_raw.wav | green box | miss |  | 0.003 | _silence_ 0.86 | stop 0.02 | backward 0.02 | ch0 |
| raw/supanut/S0/no/no_01_raw.wav | no | miss |  | 0.005 | _silence_ 0.85 | left 0.01 | stop 0.01 | ch0 |
| raw/supanut/S0/blue_box/blue_box_02_raw.wav | blue box | miss |  | 0.002 | _silence_ 0.86 | pick 0.02 | left 0.02 | ch0 |
| raw/supanut/S0/place/place_02_raw.wav | place | miss |  | 0.002 | _silence_ 0.96 | pick 0.01 | right 0.00 | ch0 |
| raw/supanut/S0/hand/hand_02_raw.wav | hand | miss |  | 0.004 | _silence_ 0.96 | pick 0.01 | hand 0.00 | ch0 |
| raw/supanut/S0/right/right_01_raw.wav | right | miss |  | 0.003 | _silence_ 0.94 | pick 0.01 | base 0.00 | ch0 |
| raw/supanut/S0/grab/grab_02_raw.wav | grab | miss |  | 0.004 | _silence_ 0.89 | stop 0.01 | left 0.01 | ch0 |
| raw/supanut/S0/base/base_02_raw.wav | base | miss |  | 0.007 | _silence_ 0.86 | pick 0.03 | left 0.01 | ch0 |
| raw/supanut/S0/green_box/green_box_03_raw.wav | green box | miss |  | 0.001 | _silence_ 0.91 | left 0.02 | pick 0.01 | ch0 |
| raw/supanut/S0/backward/backward_03_raw.wav | backward | miss |  | 0.007 | _silence_ 0.84 | stop 0.02 | pick 0.01 | ch0 |
| raw/supanut/S0/backward/backward_04_raw.wav | backward | miss |  | 0.019 | _silence_ 0.76 | left 0.05 | stop 0.03 | ch0 |
| raw/supanut/S0/right/right_02_raw.wav | right | miss |  | 0.004 | _silence_ 0.95 | stop 0.01 | left 0.00 | ch0 |
| raw/supanut/S0/black_box/black_box_03_raw.wav | black box | miss |  | 0.0 | _silence_ 0.94 | stop 0.01 | pick 0.01 | ch0 |
| raw/supanut/S0/green_box/green_box_04_raw.wav | green box | miss |  | 0.0 | _silence_ 0.96 | left 0.00 | pick 0.00 | ch0 |
| raw/supanut/S0/base/base_03_raw.wav | base | miss |  | 0.004 | _silence_ 0.93 | pick 0.01 | left 0.01 | ch0 |
| raw/supanut/S0/green_box/green_box_05_raw.wav | green box | miss |  | 0.001 | _silence_ 0.91 | pick 0.01 | backward 0.01 | ch0 |
| raw/supanut/S0/red_box/red_box_02_raw.wav | red box | miss |  | 0.001 | _silence_ 0.88 | stop 0.02 | pick 0.01 | ch0 |
| raw/supanut/S0/stop/stop_02_raw.wav | stop | miss |  | 0.003 | _silence_ 0.95 | pick 0.00 | left 0.00 | ch0 |
| raw/supanut/S0/backward/backward_05_raw.wav | backward | miss |  | 0.002 | _silence_ 0.96 | pick 0.00 | place 0.00 | ch0 |
| raw/supanut/S0/blue_box/blue_box_03_raw.wav | blue box | miss |  | 0.001 | _silence_ 0.89 | pick 0.01 | left 0.01 | ch0 |
| raw/supanut/S0/home/home_03_raw.wav | home | miss |  | 0.004 | _silence_ 0.92 | pick 0.01 | stop 0.01 | ch0 |
| raw/supanut/S0/green_box/green_box_06_raw.wav | green box | miss |  | 0.001 | _silence_ 0.93 | pick 0.01 | left 0.01 | ch0 |
| raw/supanut/S0/grab/grab_03_raw.wav | grab | miss |  | 0.002 | _silence_ 0.94 | stop 0.01 | place 0.01 | ch0 |
| raw/supanut/S0/black_box/black_box_04_raw.wav | black box | miss |  | 0.0 | _silence_ 0.93 | stop 0.01 | pick 0.01 | ch0 |
| raw/supanut/S0/left/left_01_raw.wav | left | miss |  | 0.005 | _silence_ 0.94 | pick 0.01 | left 0.00 | ch0 |
| raw/supanut/S0/move/move_03_raw.wav | move | miss |  | 0.001 | _silence_ 0.98 | forward 0.00 | place 0.00 | ch0 |
| raw/supanut/S0/no/no_02_raw.wav | no | miss |  | 0.004 | _silence_ 0.93 | stop 0.01 | place 0.01 | ch0 |
| raw/supanut/S0/blue_box/blue_box_04_raw.wav | blue box | miss |  | 0.002 | _silence_ 0.81 | right 0.03 | left 0.02 | ch0 |
| raw/supanut/S0/stop/stop_03_raw.wav | stop | miss |  | 0.062 | _silence_ 0.66 | stop 0.06 | base 0.03 | ch0 |
| raw/supanut/S0/place/place_04_raw.wav | place | miss |  | 0.085 | _silence_ 0.43 | pick 0.10 | base 0.06 | ch0 |
| raw/supanut/S0/grab/grab_06_raw.wav | grab | miss |  | 0.004 | _silence_ 0.67 | stop 0.08 | pick 0.05 | ch0 |
| raw/supanut/S0/forward/forward_01_raw.wav | forward | miss |  | 0.013 | _silence_ 0.66 | stop 0.06 | place 0.03 | ch0 |
| raw/supanut/S0/station/station_03_raw.wav | station | miss |  | 0.008 | _silence_ 0.78 | stop 0.04 | yes 0.02 | ch0 |
| raw/supanut/S0/backward/backward_06_raw.wav | backward | miss |  | 0.008 | _silence_ 0.80 | place 0.03 | right 0.02 | ch0 |
| raw/supanut/S0/place/place_06_raw.wav | place | miss |  | 0.008 | _silence_ 0.84 | pick 0.05 | move 0.02 | ch0 |
| raw/supanut/S0/hand/hand_03_raw.wav | hand | miss |  | 0.01 | _silence_ 0.73 | pick 0.10 | move 0.02 | ch0 |
| raw/supanut/S0/hand/hand_04_raw.wav | hand | miss |  | 0.017 | _silence_ 0.81 | pick 0.02 | hand 0.02 | ch0 |
| raw/supanut/S0/station/station_04_raw.wav | station | miss |  | 0.005 | _silence_ 0.38 | pick 0.10 | left 0.09 | ch0 |
| raw/supanut/S0/forward/forward_02_raw.wav | forward | miss |  | 0.007 | _silence_ 0.74 | left 0.07 | pick 0.05 | ch0 |
| raw/supanut/S0/move/move_04_raw.wav | move | miss |  | 0.004 | _silence_ 0.91 | stop 0.01 | pick 0.01 | ch0 |
| raw/supanut/S0/station/station_05_raw.wav | station | miss |  | 0.001 | _silence_ 0.97 | forward 0.00 | place 0.00 | ch0 |
| raw/supanut/S0/red_box/red_box_03_raw.wav | red box | miss |  | 0.259 | green box 0.61 | red box 0.21 | base 0.05 | ch1 |
| raw/supanut/S0/backward/backward_07_raw.wav | backward | miss |  | 0.534 | backward 0.53 | forward 0.13 | left 0.13 | ch0/ch1 |
| raw/supanut/S0/home/home_04_raw.wav | home | miss |  | 0.005 | _silence_ 0.88 | stop 0.01 | place 0.01 | ch0 |
| raw/supanut/S0/right/right_03_raw.wav | right | miss |  | 0.016 | _silence_ 0.77 | stop 0.03 | place 0.02 | ch0 |
| raw/supanut/S0/_unknown_/bright_01_raw.wav | _unknown_ | false_alarm | right@1.31s 1.00 | 0.005 | right 1.00 | _unknown_ 0.00 | red box 0.00 | ch0/ch1 |
| raw/supanut/S0/left/left_03_raw.wav | left | miss |  | 0.002 | _silence_ 0.96 | place 0.00 | stop 0.00 | ch0 |
| raw/supanut/S0/_unknown_/open_01_raw.wav | _unknown_ | false_alarm | home@1.01s 0.87 | 0.631 | home 0.87 | hand 0.03 | _unknown_ 0.02 | ch0/ch1 |
| raw/supanut/S0/stop/stop_04_raw.wav | stop | miss |  | 0.009 | _silence_ 0.93 | stop 0.01 | base 0.01 | ch0 |
| raw/supanut/S0/base/base_04_raw.wav | base | miss |  | 0.007 | _silence_ 0.93 | forward 0.01 | place 0.01 | ch0 |
| raw/supanut/S0/blue_box/blue_box_07_raw.wav | blue box | miss |  | 0.537 | no 0.55 | move 0.30 | blue box 0.11 | ch0/ch1 |
| raw/supanut/S0/left/left_05_raw.wav | left | miss |  | 0.141 | _silence_ 0.51 | left 0.14 | pick 0.07 | ch0 |
| raw/supanut/S0/place/place_07_raw.wav | place | miss |  | 0.027 | _silence_ 0.44 | stop 0.06 | _unknown_ 0.05 | ch0 |
| raw/supanut/S0/no/no_03_raw.wav | no | miss |  | 0.009 | _silence_ 0.67 | pick 0.09 | right 0.05 | ch0 |
| raw/supanut/S0/_unknown_/blue_01_raw.wav | _unknown_ | false_alarm | no@1.31s 0.88 | 0.019 | no 0.93 | move 0.05 | _unknown_ 0.01 | ch1 |
| raw/supanut/S0/_unknown_/groove_01_raw.wav | _unknown_ | false_alarm | move@1.01s 0.87 | 0.023 | move 0.87 | home 0.03 | no 0.03 | ch1 |
