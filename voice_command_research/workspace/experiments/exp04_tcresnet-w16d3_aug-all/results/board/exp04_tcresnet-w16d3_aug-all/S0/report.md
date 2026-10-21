# Board evaluation - exp04_tcresnet-w16d3_aug-all on S0

model `D:\Microcon69\ProjectRoboticArm\ProjectSpace\2026-micro-con\voice_command_research\workspace\experiments\exp04_tcresnet-w16d3_aug-all\runs\final.pth` (pytorch exp04_tcresnet-w16d3_aug-all, 23 classes) · gain +15.0 dB · features librosa 0.11.0 (features.py) · 2026-10-21 20:32

## Clip mode (1 s clips, mono ch1)

n 109 · **accuracy 0.844** · **macro-F1 0.855** · commands 0.9681 · _unknown_ 0.0667 · _silence_ None · commands rejected as unknown/silence 0 · speech peak median -32.4 dB (training clip -25.4 dB)

| cls | support | correct | recall | precision | f1 | mean_p_true | confused_with |
|---|---|---|---|---|---|---|---|
| move | 5 | 3 | 0.6 | 0.6 | 0.6 | 0.597 | hand 1, no 1 |
| pick | 2 | 2 | 1.0 | 0.667 | 0.8 | 0.978 |  |
| grab | 7 | 7 | 1.0 | 1.0 | 1.0 | 0.904 |  |
| blue box | 7 | 7 | 1.0 | 1.0 | 1.0 | 0.913 |  |
| green box | 6 | 6 | 1.0 | 1.0 | 1.0 | 0.978 |  |
| red box | 3 | 3 | 1.0 | 1.0 | 1.0 | 0.847 |  |
| black box | 4 | 4 | 1.0 | 1.0 | 1.0 | 0.805 |  |
| hand | 5 | 5 | 1.0 | 0.833 | 0.909 | 0.818 |  |
| station | 6 | 6 | 1.0 | 1.0 | 1.0 | 0.962 |  |
| base | 5 | 5 | 1.0 | 1.0 | 1.0 | 0.817 |  |
| forward | 4 | 4 | 1.0 | 0.667 | 0.8 | 0.948 |  |
| backward | 7 | 7 | 1.0 | 0.875 | 0.933 | 0.993 |  |
| left | 5 | 5 | 1.0 | 1.0 | 1.0 | 0.894 |  |
| right | 3 | 3 | 1.0 | 0.6 | 0.75 | 0.995 |  |
| place | 7 | 7 | 1.0 | 0.778 | 0.875 | 0.91 |  |
| drop | 2 | 2 | 1.0 | 1.0 | 1.0 | 0.973 |  |
| yes | 2 | 1 | 0.5 | 1.0 | 0.667 | 0.578 | place 1 |
| no | 4 | 4 | 1.0 | 0.571 | 0.727 | 0.973 |  |
| home | 5 | 5 | 1.0 | 0.625 | 0.769 | 0.958 |  |
| stop | 5 | 5 | 1.0 | 1.0 | 1.0 | 0.978 |  |
| _unknown_ | 15 | 1 | 0.067 | 1.0 | 0.125 | 0.081 | home 3, move 2, forward 2 |

### Mistakes (top-3)

| clip_path | label | pred | top3 | snr_db | flags |
|---|---|---|---|---|---|
| clips/supanut/S0/_unknown_/pink_01.wav | _unknown_ | pick | pick 0.89 | _unknown_ 0.05 | hand 0.02 | 19.61 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/move/move_02.wav | move | hand | hand 0.48 | no 0.19 | move 0.10 | 22.06 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/nose_01.wav | _unknown_ | no | no 0.93 | _unknown_ 0.03 | move 0.01 | 24.64 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/write_01.wav | _unknown_ | right | right 0.99 | _unknown_ 0.00 | home 0.00 | 25.15 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/mood_01.wav | _unknown_ | move | move 0.75 | forward 0.10 | _unknown_ 0.04 | 22.31 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/phone_01.wav | _unknown_ | forward | forward 0.80 | home 0.07 | _unknown_ 0.05 | 24.42 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/moon_01.wav | _unknown_ | forward | forward 0.32 | move 0.29 | _unknown_ 0.16 | 23.7 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/go_01.wav | _unknown_ | home | home 0.91 | hand 0.01 | _unknown_ 0.01 | 27.44 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/foam_01.wav | _unknown_ | home | home 0.94 | right 0.02 | _unknown_ 0.01 | 26.15 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/bright_01.wav | _unknown_ | right | right 0.99 | _unknown_ 0.00 | backward 0.00 | 24.09 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/open_01.wav | _unknown_ | home | home 0.34 | _unknown_ 0.30 | forward 0.10 | 24.52 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/yes/yes_02.wav | yes | place | place 0.49 | base 0.22 | yes 0.19 | 16.0 | sound longer than 1 s - the clip cuts it; S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/wait_01.wav | _unknown_ | place | place 0.33 | right 0.30 | _unknown_ 0.10 | 19.73 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/blue_01.wav | _unknown_ | no | no 0.90 | move 0.04 | _unknown_ 0.01 | 23.87 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/move/move_05.wav | move | no | no 0.34 | move 0.32 | hand 0.09 | 25.43 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/bag_01.wav | _unknown_ | backward | backward 0.63 | left 0.19 | right 0.09 | 17.09 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/groove_01.wav | _unknown_ | move | move 0.86 | home 0.03 | no 0.03 | 26.41 | S0 old wiring (VDD ~1.8 V) |

## Stream mode (raw 2 s takes, board logic: stride 10 frames, avg 3, threshold 0.7, refractory 1500 ms, channel auto)

command takes 94: **detected 0.3191** (hit 30, hit+extra 0), wrong 1, miss 63 · first detection correct 0.3191 · _unknown_ false alarms 6/15 · noise false alarms 0 in 0.0 min · channels {'ch1': 27, 'ch0/ch1': 16, 'ch0': 66}

| cls | takes | hit | hit_extra | wrong | miss | false_alarm | detected_rate | wrong_as | mean_max_p_true |
|---|---|---|---|---|---|---|---|---|---|
| move | 5 | 1 | 0 | 0 | 4 | 0 | 0.2 |  | 0.276 |
| pick | 2 | 1 | 0 | 0 | 1 | 0 | 0.5 |  | 0.495 |
| grab | 7 | 4 | 0 | 0 | 3 | 0 | 0.571 |  | 0.535 |
| blue box | 7 | 3 | 0 | 0 | 4 | 0 | 0.429 |  | 0.379 |
| green box | 6 | 0 | 0 | 0 | 6 | 0 | 0.0 |  | 0.003 |
| red box | 3 | 0 | 0 | 1 | 2 | 0 | 0.0 | right 1 | 0.492 |
| black box | 4 | 1 | 0 | 0 | 3 | 0 | 0.25 |  | 0.23 |
| hand | 5 | 1 | 0 | 0 | 4 | 0 | 0.2 |  | 0.196 |
| station | 6 | 2 | 0 | 0 | 4 | 0 | 0.333 |  | 0.324 |
| base | 5 | 0 | 0 | 0 | 5 | 0 | 0.0 |  | 0.143 |
| forward | 4 | 2 | 0 | 0 | 2 | 0 | 0.5 |  | 0.478 |
| backward | 7 | 1 | 0 | 0 | 6 | 0 | 0.143 |  | 0.228 |
| left | 5 | 2 | 0 | 0 | 3 | 0 | 0.4 |  | 0.365 |
| right | 3 | 0 | 0 | 0 | 3 | 0 | 0.0 |  | 0.003 |
| place | 7 | 3 | 0 | 0 | 4 | 0 | 0.429 |  | 0.42 |
| drop | 2 | 2 | 0 | 0 | 0 | 0 | 1.0 |  | 0.971 |
| yes | 2 | 2 | 0 | 0 | 0 | 0 | 1.0 |  | 0.981 |
| no | 4 | 1 | 0 | 0 | 3 | 0 | 0.25 |  | 0.249 |
| home | 5 | 2 | 0 | 0 | 3 | 0 | 0.4 |  | 0.375 |
| stop | 5 | 2 | 0 | 0 | 3 | 0 | 0.4 |  | 0.4 |
| _unknown_ | 15 | 0 | 0 | 0 | 0 | 6 |  | forward 1, right 1, home 1 | 0.093 |

### Takes that were not a clean hit

| raw_path | label | outcome | detections | max_avg_p_true | peak_top3 | channels_used |
|---|---|---|---|---|---|---|
| raw/supanut/S0/blue_box/blue_box_01_raw.wav | blue box | miss |  | 0.002 | _silence_ 0.95 | stop 0.01 | base 0.00 | ch0 |
| raw/supanut/S0/home/home_02_raw.wav | home | miss |  | 0.002 | _silence_ 0.95 | stop 0.01 | yes 0.00 | ch0 |
| raw/supanut/S0/red_box/red_box_01_raw.wav | red box | wrong | right@1.01s 0.70 | 0.92 | red box 0.92 | green box 0.02 | _unknown_ 0.01 | ch1 |
| raw/supanut/S0/station/station_01_raw.wav | station | miss |  | 0.001 | _silence_ 0.96 | pick 0.00 | base 0.00 | ch0 |
| raw/supanut/S0/pick/pick_01_raw.wav | pick | miss |  | 0.002 | _silence_ 0.97 | stop 0.00 | base 0.00 | ch0 |
| raw/supanut/S0/black_box/black_box_02_raw.wav | black box | miss |  | 0.001 | _silence_ 0.98 | stop 0.00 | pick 0.00 | ch0 |
| raw/supanut/S0/hand/hand_01_raw.wav | hand | miss |  | 0.001 | _silence_ 0.96 | stop 0.00 | backward 0.00 | ch0 |
| raw/supanut/S0/move/move_02_raw.wav | move | miss |  | 0.004 | _silence_ 0.93 | pick 0.01 | stop 0.01 | ch0 |
| raw/supanut/S0/backward/backward_02_raw.wav | backward | miss |  | 0.002 | _silence_ 0.96 | pick 0.01 | stop 0.00 | ch0 |
| raw/supanut/S0/green_box/green_box_01_raw.wav | green box | miss |  | 0.01 | _silence_ 0.85 | pick 0.02 | backward 0.01 | ch0 |
| raw/supanut/S0/base/base_01_raw.wav | base | miss |  | 0.004 | _silence_ 0.94 | stop 0.01 | base 0.00 | ch0 |
| raw/supanut/S0/green_box/green_box_02_raw.wav | green box | miss |  | 0.002 | _silence_ 0.94 | pick 0.01 | stop 0.01 | ch0 |
| raw/supanut/S0/no/no_01_raw.wav | no | miss |  | 0.002 | _silence_ 0.94 | stop 0.01 | backward 0.00 | ch0 |
| raw/supanut/S0/blue_box/blue_box_02_raw.wav | blue box | miss |  | 0.007 | _silence_ 0.90 | green box 0.02 | backward 0.01 | ch0 |
| raw/supanut/S0/place/place_02_raw.wav | place | miss |  | 0.001 | _silence_ 0.96 | pick 0.01 | stop 0.00 | ch0 |
| raw/supanut/S0/hand/hand_02_raw.wav | hand | miss |  | 0.001 | _silence_ 0.97 | stop 0.00 | pick 0.00 | ch0 |
| raw/supanut/S0/right/right_01_raw.wav | right | miss |  | 0.003 | _silence_ 0.96 | pick 0.01 | base 0.00 | ch0 |
| raw/supanut/S0/grab/grab_02_raw.wav | grab | miss |  | 0.002 | _silence_ 0.95 | pick 0.01 | backward 0.00 | ch0 |
| raw/supanut/S0/base/base_02_raw.wav | base | miss |  | 0.008 | _silence_ 0.94 | base 0.01 | stop 0.01 | ch0 |
| raw/supanut/S0/green_box/green_box_03_raw.wav | green box | miss |  | 0.002 | _silence_ 0.94 | stop 0.01 | pick 0.01 | ch0 |
| raw/supanut/S0/backward/backward_03_raw.wav | backward | miss |  | 0.002 | _silence_ 0.96 | pick 0.01 | stop 0.00 | ch0 |
| raw/supanut/S0/backward/backward_04_raw.wav | backward | miss |  | 0.002 | _silence_ 0.98 | stop 0.00 | pick 0.00 | ch0 |
| raw/supanut/S0/right/right_02_raw.wav | right | miss |  | 0.001 | _silence_ 0.98 | pick 0.00 | base 0.00 | ch0 |
| raw/supanut/S0/black_box/black_box_03_raw.wav | black box | miss |  | 0.001 | _silence_ 0.94 | pick 0.02 | drop 0.00 | ch0 |
| raw/supanut/S0/green_box/green_box_04_raw.wav | green box | miss |  | 0.002 | _silence_ 0.95 | stop 0.01 | base 0.00 | ch0 |
| raw/supanut/S0/base/base_03_raw.wav | base | miss |  | 0.006 | _silence_ 0.94 | stop 0.01 | base 0.01 | ch0 |
| raw/supanut/S0/green_box/green_box_05_raw.wav | green box | miss |  | 0.001 | _silence_ 0.97 | stop 0.00 | pick 0.00 | ch0 |
| raw/supanut/S0/red_box/red_box_02_raw.wav | red box | miss |  | 0.002 | _silence_ 0.94 | pick 0.00 | stop 0.00 | ch0 |
| raw/supanut/S0/stop/stop_02_raw.wav | stop | miss |  | 0.003 | _silence_ 0.98 | stop 0.00 | base 0.00 | ch0 |
| raw/supanut/S0/backward/backward_05_raw.wav | backward | miss |  | 0.002 | _silence_ 0.97 | stop 0.00 | pick 0.00 | ch0 |
| raw/supanut/S0/blue_box/blue_box_03_raw.wav | blue box | miss |  | 0.001 | _silence_ 0.97 | pick 0.00 | stop 0.00 | ch0 |
| raw/supanut/S0/home/home_03_raw.wav | home | miss |  | 0.002 | _silence_ 0.94 | pick 0.01 | stop 0.01 | ch0 |
| raw/supanut/S0/green_box/green_box_06_raw.wav | green box | miss |  | 0.003 | _silence_ 0.94 | pick 0.01 | drop 0.01 | ch0 |
| raw/supanut/S0/grab/grab_03_raw.wav | grab | miss |  | 0.002 | _silence_ 0.96 | stop 0.01 | pick 0.00 | ch0 |
| raw/supanut/S0/black_box/black_box_04_raw.wav | black box | miss |  | 0.0 | _silence_ 0.99 | pick 0.00 | base 0.00 | ch0 |
| raw/supanut/S0/left/left_01_raw.wav | left | miss |  | 0.002 | _silence_ 0.98 | stop 0.00 | base 0.00 | ch0 |
| raw/supanut/S0/move/move_03_raw.wav | move | miss |  | 0.003 | _silence_ 0.89 | pick 0.01 | blue box 0.01 | ch0 |
| raw/supanut/S0/no/no_02_raw.wav | no | miss |  | 0.002 | _silence_ 0.94 | stop 0.01 | station 0.00 | ch0 |
| raw/supanut/S0/blue_box/blue_box_04_raw.wav | blue box | miss |  | 0.015 | _silence_ 0.88 | pick 0.02 | station 0.01 | ch0 |
| raw/supanut/S0/stop/stop_03_raw.wav | stop | miss |  | 0.011 | _silence_ 0.60 | green box 0.08 | red box 0.04 | ch0 |
| raw/supanut/S0/place/place_04_raw.wav | place | miss |  | 0.002 | _silence_ 0.89 | pick 0.02 | hand 0.02 | ch0 |
| raw/supanut/S0/grab/grab_06_raw.wav | grab | miss |  | 0.001 | _silence_ 0.96 | pick 0.01 | backward 0.01 | ch0 |
| raw/supanut/S0/forward/forward_01_raw.wav | forward | miss |  | 0.004 | _silence_ 0.87 | pick 0.02 | move 0.01 | ch0 |
| raw/supanut/S0/station/station_03_raw.wav | station | miss |  | 0.006 | _silence_ 0.94 | stop 0.02 | left 0.01 | ch0 |
| raw/supanut/S0/_unknown_/phone_01_raw.wav | _unknown_ | false_alarm | forward@1.11s 0.80 | 0.076 | forward 0.89 | _unknown_ 0.05 | home 0.03 | ch0/ch1 |
| raw/supanut/S0/backward/backward_06_raw.wav | backward | miss |  | 0.017 | _silence_ 0.77 | pick 0.07 | base 0.02 | ch0 |
| raw/supanut/S0/place/place_06_raw.wav | place | miss |  | 0.0 | _silence_ 0.99 | pick 0.00 | move 0.00 | ch0 |
| raw/supanut/S0/hand/hand_03_raw.wav | hand | miss |  | 0.017 | _silence_ 0.62 | pick 0.05 | right 0.04 | ch0 |
| raw/supanut/S0/hand/hand_04_raw.wav | hand | miss |  | 0.002 | _silence_ 0.98 | pick 0.01 | drop 0.00 | ch0 |
| raw/supanut/S0/station/station_04_raw.wav | station | miss |  | 0.003 | _silence_ 0.89 | pick 0.01 | hand 0.01 | ch0 |
| raw/supanut/S0/forward/forward_02_raw.wav | forward | miss |  | 0.009 | _silence_ 0.72 | pick 0.06 | hand 0.03 | ch0 |
| raw/supanut/S0/move/move_04_raw.wav | move | miss |  | 0.006 | _silence_ 0.93 | pick 0.01 | backward 0.01 | ch0 |
| raw/supanut/S0/station/station_05_raw.wav | station | miss |  | 0.001 | _silence_ 0.98 | stop 0.00 | base 0.00 | ch0 |
| raw/supanut/S0/red_box/red_box_03_raw.wav | red box | miss |  | 0.554 | green box 0.58 | red box 0.27 | _unknown_ 0.02 | ch1 |
| raw/supanut/S0/backward/backward_07_raw.wav | backward | miss |  | 0.577 | backward 0.58 | forward 0.24 | _unknown_ 0.05 | ch0/ch1 |
| raw/supanut/S0/home/home_04_raw.wav | home | miss |  | 0.004 | _silence_ 0.88 | pick 0.01 | _unknown_ 0.01 | ch0 |
| raw/supanut/S0/right/right_03_raw.wav | right | miss |  | 0.004 | _silence_ 0.90 | backward 0.01 | base 0.01 | ch0 |
| raw/supanut/S0/_unknown_/bright_01_raw.wav | _unknown_ | false_alarm | right@1.31s 0.98 | 0.004 | right 0.99 | _unknown_ 0.00 | grab 0.00 | ch0/ch1 |
| raw/supanut/S0/left/left_03_raw.wav | left | miss |  | 0.003 | _silence_ 0.96 | stop 0.01 | backward 0.00 | ch0 |
| raw/supanut/S0/_unknown_/open_01_raw.wav | _unknown_ | false_alarm | home@1.01s 0.76 | 0.409 | home 0.76 | _unknown_ 0.05 | hand 0.04 | ch0/ch1 |
| raw/supanut/S0/stop/stop_04_raw.wav | stop | miss |  | 0.01 | _silence_ 0.91 | stop 0.01 | backward 0.01 | ch0 |
| raw/supanut/S0/base/base_04_raw.wav | base | miss |  | 0.004 | _silence_ 0.95 | stop 0.01 | base 0.00 | ch0 |
| raw/supanut/S0/left/left_05_raw.wav | left | miss |  | 0.02 | _silence_ 0.31 | pick 0.27 | green box 0.09 | ch0 |
| raw/supanut/S0/place/place_07_raw.wav | place | miss |  | 0.003 | _silence_ 0.87 | pick 0.02 | backward 0.01 | ch0 |
| raw/supanut/S0/no/no_03_raw.wav | no | miss |  | 0.008 | _silence_ 0.64 | pick 0.08 | drop 0.03 | ch0 |
| raw/supanut/S0/_unknown_/blue_01_raw.wav | _unknown_ | false_alarm | no@1.31s 0.88 | 0.073 | no 0.92 | move 0.03 | _unknown_ 0.01 | ch1 |
| raw/supanut/S0/move/move_05_raw.wav | move | miss |  | 0.393 | move 0.39 | no 0.28 | hand 0.11 | ch1 |
| raw/supanut/S0/base/base_05_raw.wav | base | miss |  | 0.691 | base 0.69 | place 0.20 | _unknown_ 0.03 | ch1 |
| raw/supanut/S0/_unknown_/bag_01_raw.wav | _unknown_ | false_alarm | backward@1.01s 0.79 | 0.027 | backward 0.79 | left 0.13 | right 0.03 | ch1 |
| raw/supanut/S0/_unknown_/groove_01_raw.wav | _unknown_ | false_alarm | move@1.01s 0.82 | 0.049 | move 0.85 | no 0.04 | home 0.04 | ch1 |
