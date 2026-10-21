# Board evaluation - exp03_tcresnet-w16d2_aug-all_mswc-cap1000 on S0

model `D:\Microcon69\ProjectRoboticArm\ProjectSpace\2026-micro-con\voice_command_research\workspace\experiments\exp03_tcresnet-w16d2_aug-all_mswc-cap1000\runs\final.pth` (pytorch exp03_tcresnet-w16d2_aug-all_mswc-cap1000, 23 classes) · gain +15.0 dB · features librosa 0.11.0 (features.py) · 2026-10-21 20:03

## Clip mode (1 s clips, mono ch1)

n 109 · **accuracy 0.8073** · **macro-F1 0.8195** · commands 0.9255 · _unknown_ 0.0667 · _silence_ None · commands rejected as unknown/silence 0 · speech peak median -32.4 dB (training clip -25.4 dB)

| cls | support | correct | recall | precision | f1 | mean_p_true | confused_with |
|---|---|---|---|---|---|---|---|
| move | 5 | 4 | 0.8 | 0.571 | 0.667 | 0.624 | hand 1 |
| pick | 2 | 2 | 1.0 | 0.667 | 0.8 | 0.998 |  |
| grab | 7 | 7 | 1.0 | 1.0 | 1.0 | 0.875 |  |
| blue box | 7 | 7 | 1.0 | 1.0 | 1.0 | 0.73 |  |
| green box | 6 | 6 | 1.0 | 1.0 | 1.0 | 0.972 |  |
| red box | 3 | 3 | 1.0 | 0.75 | 0.857 | 0.705 |  |
| black box | 4 | 3 | 0.75 | 1.0 | 0.857 | 0.517 | backward 1 |
| hand | 5 | 5 | 1.0 | 0.833 | 0.909 | 0.92 |  |
| station | 6 | 6 | 1.0 | 1.0 | 1.0 | 0.851 |  |
| base | 5 | 2 | 0.4 | 0.667 | 0.5 | 0.532 | place 3 |
| forward | 4 | 4 | 1.0 | 1.0 | 1.0 | 0.962 |  |
| backward | 7 | 7 | 1.0 | 0.875 | 0.933 | 0.998 |  |
| left | 5 | 5 | 1.0 | 0.833 | 0.909 | 0.978 |  |
| right | 3 | 3 | 1.0 | 0.6 | 0.75 | 1.0 |  |
| place | 7 | 6 | 0.857 | 0.545 | 0.667 | 0.809 | base 1 |
| drop | 2 | 2 | 1.0 | 1.0 | 1.0 | 0.929 |  |
| yes | 2 | 1 | 0.5 | 1.0 | 0.667 | 0.536 | place 1 |
| no | 4 | 4 | 1.0 | 0.667 | 0.8 | 0.989 |  |
| home | 5 | 5 | 1.0 | 0.625 | 0.769 | 0.957 |  |
| stop | 5 | 5 | 1.0 | 1.0 | 1.0 | 0.971 |  |
| _unknown_ | 15 | 1 | 0.067 | 1.0 | 0.125 | 0.048 | move 3, home 3, right 2 |

### Mistakes (top-3)

| clip_path | label | pred | top3 | snr_db | flags |
|---|---|---|---|---|---|
| clips/supanut/S0/_unknown_/pink_01.wav | _unknown_ | pick | pick 0.92 | hand 0.04 | _unknown_ 0.02 | 19.61 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/move/move_02.wav | move | hand | hand 0.59 | move 0.17 | no 0.05 | 22.06 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/nose_01.wav | _unknown_ | no | no 0.96 | _unknown_ 0.01 | move 0.01 | 24.64 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/write_01.wav | _unknown_ | right | right 1.00 | _unknown_ 0.00 | backward 0.00 | 25.15 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/black_box/black_box_03.wav | black box | backward | backward 0.43 | black box 0.27 | yes 0.11 | 23.37 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/base/base_03.wav | base | place | place 0.50 | base 0.47 | right 0.01 | 20.77 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/mood_01.wav | _unknown_ | move | move 0.86 | no 0.03 | _unknown_ 0.02 | 22.31 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/phone_01.wav | _unknown_ | home | home 0.48 | forward 0.31 | _unknown_ 0.08 | 24.42 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/place/place_06.wav | place | base | base 0.49 | place 0.36 | yes 0.05 | 18.83 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/moon_01.wav | _unknown_ | move | move 0.57 | home 0.22 | hand 0.06 | 23.7 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/go_01.wav | _unknown_ | home | home 0.75 | _unknown_ 0.05 | hand 0.04 | 27.44 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/foam_01.wav | _unknown_ | home | home 0.91 | _unknown_ 0.03 | place 0.02 | 26.15 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/bright_01.wav | _unknown_ | right | right 1.00 | _unknown_ 0.00 | grab 0.00 | 24.09 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/yes/yes_02.wav | yes | place | place 0.39 | base 0.19 | yes 0.08 | 16.0 | sound longer than 1 s - the clip cuts it; S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/robot_01.wav | _unknown_ | red box | red box 0.32 | grab 0.12 | _unknown_ 0.08 | 22.74 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/wait_01.wav | _unknown_ | place | place 0.61 | left 0.21 | _unknown_ 0.03 | 19.73 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/base/base_04.wav | base | place | place 0.69 | base 0.26 | move 0.02 | 17.58 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/blue_01.wav | _unknown_ | no | no 0.84 | move 0.13 | undo 0.01 | 23.87 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/base/base_05.wav | base | place | place 0.61 | base 0.29 | move 0.02 | 16.74 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/bag_01.wav | _unknown_ | left | left 0.67 | right 0.08 | backward 0.08 | 17.09 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/groove_01.wav | _unknown_ | move | move 0.73 | home 0.13 | no 0.03 | 26.41 | S0 old wiring (VDD ~1.8 V) |

## Stream mode (raw 2 s takes, board logic: stride 10 frames, avg 3, threshold 0.7, refractory 1500 ms, channel auto)

command takes 94: **detected 0.2979** (hit 28, hit+extra 0), wrong 2, miss 64 · first detection correct 0.2979 · _unknown_ false alarms 3/15 · noise false alarms 0 in 0.0 min · channels {'ch1': 27, 'ch0/ch1': 16, 'ch0': 66}

| cls | takes | hit | hit_extra | wrong | miss | false_alarm | detected_rate | wrong_as | mean_max_p_true |
|---|---|---|---|---|---|---|---|---|---|
| move | 5 | 1 | 0 | 0 | 4 | 0 | 0.2 |  | 0.313 |
| pick | 2 | 1 | 0 | 0 | 1 | 0 | 0.5 |  | 0.505 |
| grab | 7 | 4 | 0 | 0 | 3 | 0 | 0.571 |  | 0.501 |
| blue box | 7 | 2 | 0 | 0 | 5 | 0 | 0.286 |  | 0.317 |
| green box | 6 | 0 | 0 | 0 | 6 | 0 | 0.0 |  | 0.001 |
| red box | 3 | 1 | 0 | 0 | 2 | 0 | 0.333 |  | 0.476 |
| black box | 4 | 0 | 0 | 1 | 3 | 0 | 0.0 | left 1 | 0.153 |
| hand | 5 | 1 | 0 | 0 | 4 | 0 | 0.2 |  | 0.202 |
| station | 6 | 2 | 0 | 0 | 4 | 0 | 0.333 |  | 0.299 |
| base | 5 | 0 | 0 | 0 | 5 | 0 | 0.0 |  | 0.088 |
| forward | 4 | 2 | 0 | 0 | 2 | 0 | 0.5 |  | 0.482 |
| backward | 7 | 0 | 0 | 1 | 6 | 0 | 0.0 | left 1 | 0.224 |
| left | 5 | 2 | 0 | 0 | 3 | 0 | 0.4 |  | 0.393 |
| right | 3 | 0 | 0 | 0 | 3 | 0 | 0.0 |  | 0.005 |
| place | 7 | 3 | 0 | 0 | 4 | 0 | 0.429 |  | 0.415 |
| drop | 2 | 2 | 0 | 0 | 0 | 0 | 1.0 |  | 0.944 |
| yes | 2 | 2 | 0 | 0 | 0 | 0 | 1.0 |  | 0.996 |
| no | 4 | 1 | 0 | 0 | 3 | 0 | 0.25 |  | 0.249 |
| home | 5 | 2 | 0 | 0 | 3 | 0 | 0.4 |  | 0.381 |
| stop | 5 | 2 | 0 | 0 | 3 | 0 | 0.4 |  | 0.395 |
| _unknown_ | 15 | 0 | 0 | 0 | 0 | 3 |  | right 1, no 1, move 1 | 0.047 |

### Takes that were not a clean hit

| raw_path | label | outcome | detections | max_avg_p_true | peak_top3 | channels_used |
|---|---|---|---|---|---|---|
| raw/supanut/S0/black_box/black_box_01_raw.wav | black box | wrong | left@1.01s 0.76 | 0.613 | left 0.76 | black box 0.05 | yes 0.05 | ch1 |
| raw/supanut/S0/blue_box/blue_box_01_raw.wav | blue box | miss |  | 0.0 | _silence_ 0.96 | pick 0.01 | left 0.00 | ch0 |
| raw/supanut/S0/home/home_02_raw.wav | home | miss |  | 0.001 | _silence_ 0.97 | pick 0.00 | left 0.00 | ch0 |
| raw/supanut/S0/backward/backward_01_raw.wav | backward | wrong | left@1.01s 0.82 | 0.996 | backward 1.00 | left 0.00 | black box 0.00 | ch1 |
| raw/supanut/S0/station/station_01_raw.wav | station | miss |  | 0.001 | _silence_ 0.96 | pick 0.01 | left 0.00 | ch0 |
| raw/supanut/S0/pick/pick_01_raw.wav | pick | miss |  | 0.013 | _silence_ 0.93 | pick 0.01 | left 0.01 | ch0 |
| raw/supanut/S0/black_box/black_box_02_raw.wav | black box | miss |  | 0.0 | _silence_ 0.97 | pick 0.01 | left 0.00 | ch0 |
| raw/supanut/S0/hand/hand_01_raw.wav | hand | miss |  | 0.002 | _silence_ 0.97 | pick 0.01 | left 0.00 | ch0 |
| raw/supanut/S0/move/move_02_raw.wav | move | miss |  | 0.005 | _silence_ 0.90 | pick 0.01 | drop 0.01 | ch0 |
| raw/supanut/S0/backward/backward_02_raw.wav | backward | miss |  | 0.002 | _silence_ 0.95 | pick 0.01 | right 0.00 | ch0 |
| raw/supanut/S0/green_box/green_box_01_raw.wav | green box | miss |  | 0.002 | _silence_ 0.94 | pick 0.01 | left 0.01 | ch0 |
| raw/supanut/S0/base/base_01_raw.wav | base | miss |  | 0.005 | _silence_ 0.88 | pick 0.02 | left 0.02 | ch0 |
| raw/supanut/S0/green_box/green_box_02_raw.wav | green box | miss |  | 0.001 | _silence_ 0.95 | pick 0.01 | left 0.01 | ch0 |
| raw/supanut/S0/no/no_01_raw.wav | no | miss |  | 0.002 | _silence_ 0.89 | pick 0.04 | left 0.01 | ch0 |
| raw/supanut/S0/blue_box/blue_box_02_raw.wav | blue box | miss |  | 0.001 | _silence_ 0.82 | pick 0.05 | left 0.02 | ch0 |
| raw/supanut/S0/place/place_02_raw.wav | place | miss |  | 0.002 | _silence_ 0.96 | pick 0.01 | left 0.00 | ch0 |
| raw/supanut/S0/hand/hand_02_raw.wav | hand | miss |  | 0.002 | _silence_ 0.96 | pick 0.01 | left 0.00 | ch0 |
| raw/supanut/S0/right/right_01_raw.wav | right | miss |  | 0.004 | _silence_ 0.93 | pick 0.01 | left 0.01 | ch0 |
| raw/supanut/S0/grab/grab_02_raw.wav | grab | miss |  | 0.004 | _silence_ 0.93 | pick 0.01 | left 0.01 | ch0 |
| raw/supanut/S0/base/base_02_raw.wav | base | miss |  | 0.009 | _silence_ 0.84 | pick 0.02 | left 0.01 | ch0 |
| raw/supanut/S0/green_box/green_box_03_raw.wav | green box | miss |  | 0.001 | _silence_ 0.92 | pick 0.01 | left 0.01 | ch0 |
| raw/supanut/S0/backward/backward_03_raw.wav | backward | miss |  | 0.002 | _silence_ 0.95 | pick 0.01 | left 0.00 | ch0 |
| raw/supanut/S0/backward/backward_04_raw.wav | backward | miss |  | 0.005 | _silence_ 0.91 | pick 0.01 | left 0.01 | ch0 |
| raw/supanut/S0/right/right_02_raw.wav | right | miss |  | 0.005 | _silence_ 0.94 | pick 0.01 | left 0.01 | ch0 |
| raw/supanut/S0/black_box/black_box_03_raw.wav | black box | miss |  | 0.001 | _silence_ 0.91 | pick 0.03 | left 0.01 | ch0 |
| raw/supanut/S0/green_box/green_box_04_raw.wav | green box | miss |  | 0.001 | _silence_ 0.96 | pick 0.01 | left 0.00 | ch0 |
| raw/supanut/S0/base/base_03_raw.wav | base | miss |  | 0.002 | _silence_ 0.93 | pick 0.02 | left 0.01 | ch0 |
| raw/supanut/S0/green_box/green_box_05_raw.wav | green box | miss |  | 0.001 | _silence_ 0.93 | pick 0.01 | left 0.01 | ch0 |
| raw/supanut/S0/red_box/red_box_02_raw.wav | red box | miss |  | 0.0 | _silence_ 0.95 | pick 0.01 | left 0.01 | ch0 |
| raw/supanut/S0/stop/stop_02_raw.wav | stop | miss |  | 0.003 | _silence_ 0.96 | pick 0.01 | left 0.00 | ch0 |
| raw/supanut/S0/backward/backward_05_raw.wav | backward | miss |  | 0.003 | _silence_ 0.96 | pick 0.01 | left 0.00 | ch0 |
| raw/supanut/S0/blue_box/blue_box_03_raw.wav | blue box | miss |  | 0.0 | _silence_ 0.95 | pick 0.01 | left 0.01 | ch0 |
| raw/supanut/S0/home/home_03_raw.wav | home | miss |  | 0.002 | _silence_ 0.96 | pick 0.01 | left 0.00 | ch0 |
| raw/supanut/S0/green_box/green_box_06_raw.wav | green box | miss |  | 0.001 | _silence_ 0.94 | pick 0.02 | left 0.01 | ch0 |
| raw/supanut/S0/grab/grab_03_raw.wav | grab | miss |  | 0.003 | _silence_ 0.95 | pick 0.01 | left 0.00 | ch0 |
| raw/supanut/S0/black_box/black_box_04_raw.wav | black box | miss |  | 0.0 | _silence_ 0.95 | pick 0.01 | left 0.01 | ch0 |
| raw/supanut/S0/left/left_01_raw.wav | left | miss |  | 0.005 | _silence_ 0.96 | pick 0.01 | left 0.00 | ch0 |
| raw/supanut/S0/move/move_03_raw.wav | move | miss |  | 0.001 | _silence_ 0.91 | drop 0.02 | stop 0.01 | ch0 |
| raw/supanut/S0/no/no_02_raw.wav | no | miss |  | 0.001 | _silence_ 0.71 | pick 0.17 | drop 0.03 | ch0 |
| raw/supanut/S0/blue_box/blue_box_04_raw.wav | blue box | miss |  | 0.001 | _silence_ 0.89 | pick 0.02 | left 0.01 | ch0 |
| raw/supanut/S0/stop/stop_03_raw.wav | stop | miss |  | 0.007 | _silence_ 0.87 | pick 0.02 | right 0.01 | ch0 |
| raw/supanut/S0/place/place_04_raw.wav | place | miss |  | 0.005 | _silence_ 0.94 | pick 0.01 | right 0.00 | ch0 |
| raw/supanut/S0/grab/grab_06_raw.wav | grab | miss |  | 0.006 | _silence_ 0.89 | pick 0.02 | left 0.01 | ch0 |
| raw/supanut/S0/forward/forward_01_raw.wav | forward | miss |  | 0.004 | _silence_ 0.92 | pick 0.01 | left 0.01 | ch0 |
| raw/supanut/S0/station/station_03_raw.wav | station | miss |  | 0.005 | _silence_ 0.89 | pick 0.02 | drop 0.02 | ch0 |
| raw/supanut/S0/backward/backward_06_raw.wav | backward | miss |  | 0.012 | _silence_ 0.84 | stop 0.03 | drop 0.02 | ch0 |
| raw/supanut/S0/place/place_06_raw.wav | place | miss |  | 0.001 | _silence_ 0.99 | pick 0.00 | left 0.00 | ch0 |
| raw/supanut/S0/hand/hand_03_raw.wav | hand | miss |  | 0.009 | _silence_ 0.86 | left 0.02 | pick 0.01 | ch0 |
| raw/supanut/S0/hand/hand_04_raw.wav | hand | miss |  | 0.003 | _silence_ 0.92 | left 0.01 | pick 0.01 | ch0 |
| raw/supanut/S0/station/station_04_raw.wav | station | miss |  | 0.001 | _silence_ 0.98 | pick 0.01 | left 0.00 | ch0 |
| raw/supanut/S0/forward/forward_02_raw.wav | forward | miss |  | 0.002 | _silence_ 0.96 | pick 0.01 | left 0.01 | ch0 |
| raw/supanut/S0/move/move_04_raw.wav | move | miss |  | 0.005 | _silence_ 0.88 | pick 0.04 | left 0.01 | ch0 |
| raw/supanut/S0/station/station_05_raw.wav | station | miss |  | 0.001 | _silence_ 0.97 | pick 0.00 | stop 0.00 | ch0 |
| raw/supanut/S0/red_box/red_box_03_raw.wav | red box | miss |  | 0.671 | red box 0.67 | green box 0.18 | backward 0.04 | ch1 |
| raw/supanut/S0/backward/backward_07_raw.wav | backward | miss |  | 0.546 | backward 0.55 | left 0.15 | forward 0.10 | ch0/ch1 |
| raw/supanut/S0/home/home_04_raw.wav | home | miss |  | 0.003 | _silence_ 0.92 | pick 0.01 | grab 0.01 | ch0 |
| raw/supanut/S0/right/right_03_raw.wav | right | miss |  | 0.007 | _silence_ 0.90 | pick 0.01 | stop 0.01 | ch0 |
| raw/supanut/S0/_unknown_/bright_01_raw.wav | _unknown_ | false_alarm | right@1.31s 1.00 | 0.005 | right 1.00 | _unknown_ 0.00 | grab 0.00 | ch0/ch1 |
| raw/supanut/S0/left/left_03_raw.wav | left | miss |  | 0.002 | _silence_ 0.97 | pick 0.00 | backward 0.00 | ch0 |
| raw/supanut/S0/stop/stop_04_raw.wav | stop | miss |  | 0.021 | _silence_ 0.73 | backward 0.03 | green box 0.02 | ch0 |
| raw/supanut/S0/base/base_04_raw.wav | base | miss |  | 0.002 | _silence_ 0.96 | pick 0.00 | drop 0.00 | ch0 |
| raw/supanut/S0/blue_box/blue_box_07_raw.wav | blue box | miss |  | 0.597 | move 0.61 | blue box 0.17 | no 0.14 | ch0/ch1 |
| raw/supanut/S0/left/left_05_raw.wav | left | miss |  | 0.009 | _silence_ 0.90 | pick 0.02 | left 0.01 | ch0 |
| raw/supanut/S0/place/place_07_raw.wav | place | miss |  | 0.008 | _silence_ 0.85 | pick 0.03 | right 0.01 | ch0 |
| raw/supanut/S0/no/no_03_raw.wav | no | miss |  | 0.004 | _silence_ 0.87 | pick 0.02 | left 0.02 | ch0 |
| raw/supanut/S0/_unknown_/blue_01_raw.wav | _unknown_ | false_alarm | no@1.31s 0.78 | 0.052 | no 0.86 | move 0.12 | undo 0.01 | ch1 |
| raw/supanut/S0/move/move_05_raw.wav | move | miss |  | 0.604 | move 0.60 | no 0.13 | hand 0.13 | ch1 |
| raw/supanut/S0/base/base_05_raw.wav | base | miss |  | 0.424 | place 0.65 | base 0.27 | move 0.02 | ch1 |
| raw/supanut/S0/_unknown_/groove_01_raw.wav | _unknown_ | false_alarm | move@1.21s 0.71 | 0.028 | move 0.73 | home 0.13 | no 0.04 | ch1 |
