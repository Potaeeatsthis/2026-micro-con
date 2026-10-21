# Board evaluation - exp01_tcresnet-w16d2_aug-all_fold4 on S0

model `D:\Microcon69\ProjectRoboticArm\ProjectSpace\2026-micro-con\voice_command_research\workspace\experiments\exp01_tcresnet-w16d2_aug-all\runs\fold4\best.pth` (pytorch TC-ResNet w16 d2, 23 classes) · gain +15.0 dB · features librosa 0.11.0 (features.py) · 2026-10-21 18:37

## Clip mode (1 s clips, mono ch1)

n 109 · **accuracy 0.7798** · **macro-F1 0.7996** · commands 0.8936 · _unknown_ 0.0667 · _silence_ None · commands rejected as unknown/silence 0 · speech peak median -32.4 dB (training clip -25.4 dB)

| cls | support | correct | recall | precision | f1 | mean_p_true | confused_with |
|---|---|---|---|---|---|---|---|
| move | 5 | 4 | 0.8 | 0.4 | 0.533 | 0.605 | no 1 |
| pick | 2 | 2 | 1.0 | 0.667 | 0.8 | 0.994 |  |
| grab | 7 | 6 | 0.857 | 1.0 | 0.923 | 0.716 | right 1 |
| blue box | 7 | 1 | 0.143 | 1.0 | 0.25 | 0.215 | move 3, no 3 |
| green box | 6 | 6 | 1.0 | 1.0 | 1.0 | 0.95 |  |
| red box | 3 | 3 | 1.0 | 1.0 | 1.0 | 0.893 |  |
| black box | 4 | 4 | 1.0 | 1.0 | 1.0 | 0.506 |  |
| hand | 5 | 5 | 1.0 | 1.0 | 1.0 | 0.923 |  |
| station | 6 | 6 | 1.0 | 1.0 | 1.0 | 0.966 |  |
| base | 5 | 4 | 0.8 | 1.0 | 0.889 | 0.681 | place 1 |
| forward | 4 | 4 | 1.0 | 0.667 | 0.8 | 0.984 |  |
| backward | 7 | 7 | 1.0 | 1.0 | 1.0 | 0.989 |  |
| left | 5 | 5 | 1.0 | 0.833 | 0.909 | 0.799 |  |
| right | 3 | 3 | 1.0 | 0.5 | 0.667 | 0.999 |  |
| place | 7 | 7 | 1.0 | 0.7 | 0.824 | 0.771 |  |
| drop | 2 | 2 | 1.0 | 1.0 | 1.0 | 0.84 |  |
| yes | 2 | 1 | 0.5 | 1.0 | 0.667 | 0.53 | place 1 |
| no | 4 | 4 | 1.0 | 0.4 | 0.571 | 0.99 |  |
| home | 5 | 5 | 1.0 | 0.714 | 0.833 | 0.953 |  |
| stop | 5 | 5 | 1.0 | 1.0 | 1.0 | 0.982 |  |
| _unknown_ | 15 | 1 | 0.067 | 1.0 | 0.125 | 0.071 | move 3, forward 2, right 2 |

### Mistakes (top-3)

| clip_path | label | pred | top3 | snr_db | flags |
|---|---|---|---|---|---|
| clips/supanut/S0/grab/grab_01.wav | grab | right | right 0.44 | grab 0.32 | left 0.13 | 27.57 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/blue_box/blue_box_01.wav | blue box | no | no 0.42 | blue box 0.22 | move 0.17 | 24.12 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/pink_01.wav | _unknown_ | pick | pick 0.94 | _unknown_ 0.02 | hand 0.01 | 19.61 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/move/move_02.wav | move | no | no 0.34 | hand 0.25 | move 0.19 | 22.06 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/blue_box/blue_box_02.wav | blue box | no | no 0.51 | move 0.22 | blue box 0.10 | 20.71 | word touches the edge of the 2 s take; sound longer than 1 s - the clip cuts it; S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/nose_01.wav | _unknown_ | no | no 0.98 | move 0.01 | _unknown_ 0.00 | 24.64 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/write_01.wav | _unknown_ | right | right 1.00 | _unknown_ 0.00 | grab 0.00 | 25.15 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/mood_01.wav | _unknown_ | move | move 0.65 | forward 0.14 | no 0.05 | 22.31 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/blue_box/blue_box_04.wav | blue box | move | move 0.49 | blue box 0.25 | no 0.09 | 26.78 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/phone_01.wav | _unknown_ | forward | forward 0.50 | home 0.37 | _unknown_ 0.08 | 24.42 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/moon_01.wav | _unknown_ | move | move 0.28 | forward 0.24 | home 0.22 | 23.7 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/go_01.wav | _unknown_ | home | home 0.65 | stop 0.08 | _unknown_ 0.06 | 27.44 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/foam_01.wav | _unknown_ | home | home 0.76 | _unknown_ 0.09 | place 0.04 | 26.15 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/bright_01.wav | _unknown_ | right | right 1.00 | _unknown_ 0.00 | grab 0.00 | 24.09 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/open_01.wav | _unknown_ | forward | forward 0.36 | _unknown_ 0.34 | home 0.19 | 24.52 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/yes/yes_02.wav | yes | place | place 0.50 | base 0.15 | _unknown_ 0.09 | 16.0 | sound longer than 1 s - the clip cuts it; S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/blue_box/blue_box_05.wav | blue box | move | move 0.44 | blue box 0.19 | no 0.18 | 22.38 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/wait_01.wav | _unknown_ | place | place 0.46 | left 0.27 | right 0.12 | 19.73 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/blue_box/blue_box_06.wav | blue box | move | move 0.36 | no 0.21 | blue box 0.16 | 22.3 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/base/base_04.wav | base | place | place 0.49 | base 0.47 | _unknown_ 0.02 | 17.58 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/blue_box/blue_box_07.wav | blue box | no | no 0.45 | blue box 0.21 | move 0.16 | 25.13 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/blue_01.wav | _unknown_ | no | no 0.93 | move 0.05 | _unknown_ 0.01 | 23.87 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/bag_01.wav | _unknown_ | left | left 0.77 | move 0.04 | backward 0.03 | 17.09 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/groove_01.wav | _unknown_ | move | move 0.60 | home 0.21 | forward 0.06 | 26.41 | S0 old wiring (VDD ~1.8 V) |

## Stream mode (raw 2 s takes, board logic: stride 10 frames, avg 3, threshold 0.7, refractory 1500 ms, channel auto)

command takes 94: **detected 0.266** (hit 25, hit+extra 0), wrong 2, miss 67 · first detection correct 0.266 · _unknown_ false alarms 3/15 · noise false alarms 0 in 0.0 min · channels {'ch1': 27, 'ch0/ch1': 16, 'ch0': 66}

| cls | takes | hit | hit_extra | wrong | miss | false_alarm | detected_rate | wrong_as | mean_max_p_true |
|---|---|---|---|---|---|---|---|---|---|
| move | 5 | 1 | 0 | 0 | 4 | 0 | 0.2 |  | 0.294 |
| pick | 2 | 1 | 0 | 0 | 1 | 0 | 0.5 |  | 0.501 |
| grab | 7 | 3 | 0 | 0 | 4 | 0 | 0.429 |  | 0.425 |
| blue box | 7 | 0 | 0 | 1 | 6 | 0 | 0.0 | no 1 | 0.073 |
| green box | 6 | 0 | 0 | 0 | 6 | 0 | 0.0 |  | 0.001 |
| red box | 3 | 2 | 0 | 0 | 1 | 0 | 0.667 |  | 0.596 |
| black box | 4 | 0 | 0 | 0 | 4 | 0 | 0.0 |  | 0.109 |
| hand | 5 | 1 | 0 | 0 | 4 | 0 | 0.2 |  | 0.202 |
| station | 6 | 2 | 0 | 0 | 4 | 0 | 0.333 |  | 0.319 |
| base | 5 | 0 | 0 | 0 | 5 | 0 | 0.0 |  | 0.122 |
| forward | 4 | 2 | 0 | 0 | 2 | 0 | 0.5 |  | 0.492 |
| backward | 7 | 0 | 0 | 1 | 6 | 0 | 0.0 | left 1 | 0.237 |
| left | 5 | 1 | 0 | 0 | 4 | 0 | 0.2 |  | 0.29 |
| right | 3 | 0 | 0 | 0 | 3 | 0 | 0.0 |  | 0.005 |
| place | 7 | 3 | 0 | 0 | 4 | 0 | 0.429 |  | 0.386 |
| drop | 2 | 2 | 0 | 0 | 0 | 0 | 1.0 |  | 0.883 |
| yes | 2 | 2 | 0 | 0 | 0 | 0 | 1.0 |  | 0.99 |
| no | 4 | 1 | 0 | 0 | 3 | 0 | 0.25 |  | 0.255 |
| home | 5 | 2 | 0 | 0 | 3 | 0 | 0.4 |  | 0.4 |
| stop | 5 | 2 | 0 | 0 | 3 | 0 | 0.4 |  | 0.391 |
| _unknown_ | 15 | 0 | 0 | 0 | 0 | 3 |  | right 1, no 1, left 1 | 0.058 |

### Takes that were not a clean hit

| raw_path | label | outcome | detections | max_avg_p_true | peak_top3 | channels_used |
|---|---|---|---|---|---|---|
| raw/supanut/S0/black_box/black_box_01_raw.wav | black box | miss |  | 0.434 | left 0.55 | yes 0.14 | no 0.12 | ch1 |
| raw/supanut/S0/grab/grab_01_raw.wav | grab | miss |  | 0.415 | right 0.53 | grab 0.28 | left 0.10 | ch1 |
| raw/supanut/S0/blue_box/blue_box_01_raw.wav | blue box | miss |  | 0.0 | _silence_ 0.92 | hand 0.01 | home 0.01 | ch0 |
| raw/supanut/S0/home/home_02_raw.wav | home | miss |  | 0.013 | _silence_ 0.89 | hand 0.01 | home 0.01 | ch0 |
| raw/supanut/S0/backward/backward_01_raw.wav | backward | wrong | left@1.01s 0.89 | 0.992 | backward 0.99 | left 0.00 | black box 0.00 | ch1 |
| raw/supanut/S0/station/station_01_raw.wav | station | miss |  | 0.002 | _silence_ 0.95 | home 0.01 | hand 0.01 | ch0 |
| raw/supanut/S0/pick/pick_01_raw.wav | pick | miss |  | 0.006 | _silence_ 0.87 | hand 0.02 | home 0.02 | ch0 |
| raw/supanut/S0/black_box/black_box_02_raw.wav | black box | miss |  | 0.001 | _silence_ 0.91 | hand 0.01 | home 0.01 | ch0 |
| raw/supanut/S0/hand/hand_01_raw.wav | hand | miss |  | 0.007 | _silence_ 0.93 | left 0.01 | pick 0.01 | ch0 |
| raw/supanut/S0/move/move_02_raw.wav | move | miss |  | 0.003 | _silence_ 0.94 | hand 0.01 | home 0.01 | ch0 |
| raw/supanut/S0/backward/backward_02_raw.wav | backward | miss |  | 0.003 | _silence_ 0.94 | home 0.01 | hand 0.01 | ch0 |
| raw/supanut/S0/green_box/green_box_01_raw.wav | green box | miss |  | 0.002 | _silence_ 0.91 | hand 0.01 | home 0.01 | ch0 |
| raw/supanut/S0/base/base_01_raw.wav | base | miss |  | 0.005 | _silence_ 0.88 | left 0.01 | pick 0.01 | ch0 |
| raw/supanut/S0/green_box/green_box_02_raw.wav | green box | miss |  | 0.001 | _silence_ 0.94 | hand 0.01 | home 0.01 | ch0 |
| raw/supanut/S0/no/no_01_raw.wav | no | miss |  | 0.012 | _silence_ 0.89 | home 0.01 | no 0.01 | ch0 |
| raw/supanut/S0/blue_box/blue_box_02_raw.wav | blue box | miss |  | 0.0 | _silence_ 0.93 | left 0.01 | hand 0.01 | ch0 |
| raw/supanut/S0/place/place_02_raw.wav | place | miss |  | 0.002 | _silence_ 0.94 | home 0.01 | move 0.01 | ch0 |
| raw/supanut/S0/hand/hand_02_raw.wav | hand | miss |  | 0.006 | _silence_ 0.93 | home 0.01 | left 0.01 | ch0 |
| raw/supanut/S0/right/right_01_raw.wav | right | miss |  | 0.003 | _silence_ 0.94 | hand 0.01 | home 0.01 | ch0 |
| raw/supanut/S0/grab/grab_02_raw.wav | grab | miss |  | 0.005 | _silence_ 0.87 | left 0.01 | hand 0.01 | ch0 |
| raw/supanut/S0/base/base_02_raw.wav | base | miss |  | 0.001 | _silence_ 0.97 | hand 0.00 | home 0.00 | ch0 |
| raw/supanut/S0/green_box/green_box_03_raw.wav | green box | miss |  | 0.001 | _silence_ 0.91 | hand 0.01 | home 0.01 | ch0 |
| raw/supanut/S0/backward/backward_03_raw.wav | backward | miss |  | 0.006 | _silence_ 0.93 | pick 0.01 | drop 0.01 | ch0 |
| raw/supanut/S0/backward/backward_04_raw.wav | backward | miss |  | 0.006 | _silence_ 0.91 | pick 0.01 | move 0.01 | ch0 |
| raw/supanut/S0/right/right_02_raw.wav | right | miss |  | 0.003 | _silence_ 0.94 | left 0.01 | pick 0.01 | ch0 |
| raw/supanut/S0/black_box/black_box_03_raw.wav | black box | miss |  | 0.001 | _silence_ 0.91 | home 0.01 | left 0.01 | ch0 |
| raw/supanut/S0/green_box/green_box_04_raw.wav | green box | miss |  | 0.001 | _silence_ 0.95 | hand 0.00 | home 0.00 | ch0 |
| raw/supanut/S0/base/base_03_raw.wav | base | miss |  | 0.002 | _silence_ 0.94 | home 0.01 | hand 0.01 | ch0 |
| raw/supanut/S0/green_box/green_box_05_raw.wav | green box | miss |  | 0.001 | _silence_ 0.93 | pick 0.01 | left 0.01 | ch0 |
| raw/supanut/S0/red_box/red_box_02_raw.wav | red box | miss |  | 0.0 | _silence_ 0.94 | home 0.01 | left 0.01 | ch0 |
| raw/supanut/S0/stop/stop_02_raw.wav | stop | miss |  | 0.006 | _silence_ 0.90 | hand 0.01 | home 0.01 | ch0 |
| raw/supanut/S0/backward/backward_05_raw.wav | backward | miss |  | 0.003 | _silence_ 0.96 | home 0.00 | hand 0.00 | ch0 |
| raw/supanut/S0/blue_box/blue_box_03_raw.wav | blue box | miss |  | 0.0 | _silence_ 0.93 | hand 0.01 | drop 0.01 | ch0 |
| raw/supanut/S0/home/home_03_raw.wav | home | miss |  | 0.009 | _silence_ 0.91 | hand 0.01 | home 0.01 | ch0 |
| raw/supanut/S0/green_box/green_box_06_raw.wav | green box | miss |  | 0.001 | _silence_ 0.91 | hand 0.01 | left 0.01 | ch0 |
| raw/supanut/S0/grab/grab_03_raw.wav | grab | miss |  | 0.002 | _silence_ 0.94 | home 0.01 | left 0.01 | ch0 |
| raw/supanut/S0/black_box/black_box_04_raw.wav | black box | miss |  | 0.0 | _silence_ 0.95 | home 0.01 | hand 0.00 | ch0 |
| raw/supanut/S0/left/left_01_raw.wav | left | miss |  | 0.013 | _silence_ 0.79 | pick 0.02 | home 0.02 | ch0 |
| raw/supanut/S0/move/move_03_raw.wav | move | miss |  | 0.025 | _silence_ 0.45 | _unknown_ 0.07 | stop 0.05 | ch0 |
| raw/supanut/S0/no/no_02_raw.wav | no | miss |  | 0.009 | _silence_ 0.81 | hand 0.02 | home 0.02 | ch0 |
| raw/supanut/S0/blue_box/blue_box_04_raw.wav | blue box | miss |  | 0.003 | _silence_ 0.69 | left 0.04 | hand 0.04 | ch0 |
| raw/supanut/S0/stop/stop_03_raw.wav | stop | miss |  | 0.004 | _silence_ 0.95 | hand 0.01 | home 0.01 | ch0 |
| raw/supanut/S0/place/place_04_raw.wav | place | miss |  | 0.006 | _silence_ 0.83 | hand 0.04 | pick 0.02 | ch0 |
| raw/supanut/S0/grab/grab_06_raw.wav | grab | miss |  | 0.004 | _silence_ 0.90 | hand 0.02 | left 0.01 | ch0 |
| raw/supanut/S0/forward/forward_01_raw.wav | forward | miss |  | 0.002 | _silence_ 0.73 | left 0.11 | right 0.03 | ch0 |
| raw/supanut/S0/station/station_03_raw.wav | station | miss |  | 0.016 | _silence_ 0.48 | stop 0.06 | left 0.06 | ch0 |
| raw/supanut/S0/backward/backward_06_raw.wav | backward | miss |  | 0.013 | _silence_ 0.64 | move 0.07 | left 0.04 | ch0 |
| raw/supanut/S0/place/place_06_raw.wav | place | miss |  | 0.0 | _silence_ 1.00 | pick 0.00 | left 0.00 | ch0 |
| raw/supanut/S0/hand/hand_03_raw.wav | hand | miss |  | 0.001 | _silence_ 0.98 | pick 0.00 | move 0.00 | ch0 |
| raw/supanut/S0/hand/hand_04_raw.wav | hand | miss |  | 0.003 | _silence_ 0.96 | left 0.01 | home 0.01 | ch0 |
| raw/supanut/S0/station/station_04_raw.wav | station | miss |  | 0.0 | _silence_ 0.98 | left 0.01 | pick 0.00 | ch0 |
| raw/supanut/S0/forward/forward_02_raw.wav | forward | miss |  | 0.002 | _silence_ 0.73 | left 0.07 | pick 0.03 | ch0 |
| raw/supanut/S0/move/move_04_raw.wav | move | miss |  | 0.004 | _silence_ 0.89 | hand 0.01 | drop 0.01 | ch0 |
| raw/supanut/S0/left/left_02_raw.wav | left | miss |  | 0.538 | left 0.54 | pick 0.23 | move 0.07 | ch0/ch1 |
| raw/supanut/S0/station/station_05_raw.wav | station | miss |  | 0.007 | _silence_ 0.83 | hand 0.02 | home 0.02 | ch0 |
| raw/supanut/S0/backward/backward_07_raw.wav | backward | miss |  | 0.638 | backward 0.64 | forward 0.08 | left 0.07 | ch0/ch1 |
| raw/supanut/S0/home/home_04_raw.wav | home | miss |  | 0.037 | _silence_ 0.73 | home 0.04 | hand 0.04 | ch0 |
| raw/supanut/S0/right/right_03_raw.wav | right | miss |  | 0.01 | _silence_ 0.74 | move 0.03 | pick 0.02 | ch0 |
| raw/supanut/S0/_unknown_/bright_01_raw.wav | _unknown_ | false_alarm | right@1.31s 1.00 | 0.03 | right 1.00 | grab 0.00 | _unknown_ 0.00 | ch0/ch1 |
| raw/supanut/S0/left/left_03_raw.wav | left | miss |  | 0.009 | _silence_ 0.77 | hand 0.03 | home 0.02 | ch0 |
| raw/supanut/S0/blue_box/blue_box_05_raw.wav | blue box | miss |  | 0.167 | move 0.58 | no 0.25 | _unknown_ 0.05 | ch1 |
| raw/supanut/S0/stop/stop_04_raw.wav | stop | miss |  | 0.012 | _silence_ 0.80 | hand 0.03 | home 0.02 | ch0 |
| raw/supanut/S0/blue_box/blue_box_06_raw.wav | blue box | wrong | no@1.01s 0.82 | 0.177 | no 0.82 | move 0.06 | home 0.03 | ch1 |
| raw/supanut/S0/base/base_04_raw.wav | base | miss |  | 0.01 | _silence_ 0.73 | hand 0.04 | home 0.03 | ch0 |
| raw/supanut/S0/blue_box/blue_box_07_raw.wav | blue box | miss |  | 0.161 | no 0.57 | move 0.32 | _unknown_ 0.03 | ch0/ch1 |
| raw/supanut/S0/left/left_05_raw.wav | left | miss |  | 0.018 | _silence_ 0.90 | left 0.02 | hand 0.01 | ch0 |
| raw/supanut/S0/place/place_07_raw.wav | place | miss |  | 0.003 | _silence_ 0.90 | left 0.02 | hand 0.01 | ch0 |
| raw/supanut/S0/no/no_03_raw.wav | no | miss |  | 0.003 | _silence_ 0.92 | pick 0.02 | left 0.01 | ch0 |
| raw/supanut/S0/_unknown_/blue_01_raw.wav | _unknown_ | false_alarm | no@1.31s 0.88 | 0.042 | no 0.91 | move 0.07 | _unknown_ 0.01 | ch1 |
| raw/supanut/S0/move/move_05_raw.wav | move | miss |  | 0.511 | move 0.51 | no 0.24 | hand 0.10 | ch1 |
| raw/supanut/S0/base/base_05_raw.wav | base | miss |  | 0.59 | base 0.59 | place 0.35 | _unknown_ 0.02 | ch1 |
| raw/supanut/S0/_unknown_/bag_01_raw.wav | _unknown_ | false_alarm | left@1.01s 0.75 | 0.029 | left 0.75 | backward 0.04 | grab 0.04 | ch1 |
