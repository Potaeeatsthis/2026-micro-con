# Board evaluation - exp08_tcresnet-w16d4_aug-all on S0

model `D:\Microcon69\ProjectRoboticArm\ProjectSpace\2026-micro-con\voice_command_research\workspace\experiments\exp08_tcresnet-w16d4_aug-all\runs\final.pth` (pytorch exp08_tcresnet-w16d4_aug-all, 23 classes) · gain +15.0 dB · features librosa 0.11.0 (features.py) · 2026-10-21 21:53

## Clip mode (1 s clips, mono ch1)

n 109 · **accuracy 0.8624** · **macro-F1 0.8782** · commands 0.9787 · _unknown_ 0.1333 · _silence_ None · commands rejected as unknown/silence 0 · speech peak median -32.4 dB (training clip -25.4 dB)

| cls | support | correct | recall | precision | f1 | mean_p_true | confused_with |
|---|---|---|---|---|---|---|---|
| move | 5 | 5 | 1.0 | 0.714 | 0.833 | 0.872 |  |
| pick | 2 | 2 | 1.0 | 0.667 | 0.8 | 0.975 |  |
| grab | 7 | 7 | 1.0 | 1.0 | 1.0 | 0.921 |  |
| blue box | 7 | 7 | 1.0 | 1.0 | 1.0 | 0.921 |  |
| green box | 6 | 6 | 1.0 | 1.0 | 1.0 | 0.977 |  |
| red box | 3 | 3 | 1.0 | 1.0 | 1.0 | 0.865 |  |
| black box | 4 | 4 | 1.0 | 1.0 | 1.0 | 0.846 |  |
| hand | 5 | 5 | 1.0 | 1.0 | 1.0 | 0.962 |  |
| station | 6 | 6 | 1.0 | 1.0 | 1.0 | 0.937 |  |
| base | 5 | 5 | 1.0 | 0.833 | 0.909 | 0.921 |  |
| forward | 4 | 4 | 1.0 | 1.0 | 1.0 | 0.957 |  |
| backward | 7 | 7 | 1.0 | 0.875 | 0.933 | 0.954 |  |
| left | 5 | 5 | 1.0 | 1.0 | 1.0 | 0.905 |  |
| right | 3 | 3 | 1.0 | 0.6 | 0.75 | 0.975 |  |
| place | 7 | 6 | 0.857 | 0.75 | 0.8 | 0.872 | base 1 |
| drop | 2 | 2 | 1.0 | 1.0 | 1.0 | 0.963 |  |
| yes | 2 | 1 | 0.5 | 1.0 | 0.667 | 0.566 | place 1 |
| no | 4 | 4 | 1.0 | 0.667 | 0.8 | 0.973 |  |
| home | 5 | 5 | 1.0 | 0.556 | 0.714 | 0.969 |  |
| stop | 5 | 5 | 1.0 | 1.0 | 1.0 | 0.973 |  |
| _unknown_ | 15 | 2 | 0.133 | 1.0 | 0.235 | 0.12 | home 4, move 2, right 2 |

### Mistakes (top-3)

| clip_path | label | pred | top3 | snr_db | flags |
|---|---|---|---|---|---|
| clips/supanut/S0/_unknown_/pink_01.wav | _unknown_ | pick | pick 0.77 | _unknown_ 0.06 | move 0.03 | 19.61 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/nose_01.wav | _unknown_ | no | no 0.90 | _unknown_ 0.04 | move 0.02 | 24.64 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/write_01.wav | _unknown_ | right | right 0.99 | drop 0.00 | _unknown_ 0.00 | 25.15 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/mood_01.wav | _unknown_ | move | move 0.75 | forward 0.07 | _unknown_ 0.06 | 22.31 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/phone_01.wav | _unknown_ | home | home 0.75 | forward 0.14 | _unknown_ 0.05 | 24.42 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/place/place_06.wav | place | base | base 0.55 | place 0.37 | yes 0.02 | 18.83 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/moon_01.wav | _unknown_ | home | home 0.41 | move 0.40 | forward 0.07 | 23.7 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/go_01.wav | _unknown_ | home | home 0.84 | _unknown_ 0.02 | forward 0.02 | 27.44 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/foam_01.wav | _unknown_ | home | home 0.95 | _unknown_ 0.01 | forward 0.01 | 26.15 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/bright_01.wav | _unknown_ | right | right 0.97 | _unknown_ 0.00 | drop 0.00 | 24.09 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/yes/yes_02.wav | yes | place | place 0.58 | base 0.17 | yes 0.16 | 16.0 | sound longer than 1 s - the clip cuts it; S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/wait_01.wav | _unknown_ | place | place 0.57 | right 0.09 | move 0.09 | 19.73 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/blue_01.wav | _unknown_ | no | no 0.92 | move 0.03 | _unknown_ 0.01 | 23.87 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/bag_01.wav | _unknown_ | backward | backward 0.53 | left 0.30 | _unknown_ 0.06 | 17.09 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/groove_01.wav | _unknown_ | move | move 0.81 | _unknown_ 0.06 | no 0.05 | 26.41 | S0 old wiring (VDD ~1.8 V) |

## Stream mode (raw 2 s takes, board logic: stride 10 frames, avg 3, threshold 0.7, refractory 1500 ms, channel auto)

command takes 94: **detected 0.2872** (hit 27, hit+extra 0), wrong 7, miss 60 · first detection correct 0.2872 · _unknown_ false alarms 5/15 · noise false alarms 0 in 0.0 min · channels {'ch1': 27, 'ch0/ch1': 16, 'ch0': 66}

| cls | takes | hit | hit_extra | wrong | miss | false_alarm | detected_rate | wrong_as | mean_max_p_true |
|---|---|---|---|---|---|---|---|---|---|
| move | 5 | 2 | 0 | 0 | 3 | 0 | 0.4 |  | 0.356 |
| pick | 2 | 1 | 0 | 0 | 1 | 0 | 0.5 |  | 0.491 |
| grab | 7 | 3 | 0 | 1 | 3 | 0 | 0.429 | left 1 | 0.543 |
| blue box | 7 | 0 | 0 | 3 | 4 | 0 | 0.0 | move 2, no 1 | 0.336 |
| green box | 6 | 0 | 0 | 0 | 6 | 0 | 0.0 |  | 0.002 |
| red box | 3 | 1 | 0 | 1 | 1 | 0 | 0.333 | right 1 | 0.577 |
| black box | 4 | 0 | 0 | 1 | 3 | 0 | 0.0 | left 1 | 0.215 |
| hand | 5 | 1 | 0 | 0 | 4 | 0 | 0.2 |  | 0.2 |
| station | 6 | 1 | 0 | 1 | 4 | 0 | 0.167 | base 1 | 0.321 |
| base | 5 | 1 | 0 | 0 | 4 | 0 | 0.2 |  | 0.186 |
| forward | 4 | 2 | 0 | 0 | 2 | 0 | 0.5 |  | 0.487 |
| backward | 7 | 1 | 0 | 0 | 6 | 0 | 0.143 |  | 0.233 |
| left | 5 | 2 | 0 | 0 | 3 | 0 | 0.4 |  | 0.357 |
| right | 3 | 0 | 0 | 0 | 3 | 0 | 0.0 |  | 0.004 |
| place | 7 | 3 | 0 | 0 | 4 | 0 | 0.429 |  | 0.422 |
| drop | 2 | 2 | 0 | 0 | 0 | 0 | 1.0 |  | 0.958 |
| yes | 2 | 2 | 0 | 0 | 0 | 0 | 1.0 |  | 0.974 |
| no | 4 | 1 | 0 | 0 | 3 | 0 | 0.25 |  | 0.25 |
| home | 5 | 2 | 0 | 0 | 3 | 0 | 0.4 |  | 0.392 |
| stop | 5 | 2 | 0 | 0 | 3 | 0 | 0.4 |  | 0.397 |
| _unknown_ | 15 | 0 | 0 | 0 | 0 | 5 |  | right 1, home 1, no 1 | 0.145 |

### Takes that were not a clean hit

| raw_path | label | outcome | detections | max_avg_p_true | peak_top3 | channels_used |
|---|---|---|---|---|---|---|
| raw/supanut/S0/black_box/black_box_01_raw.wav | black box | wrong | left@1.01s 0.95 | 0.854 | left 0.95 | yes 0.01 | _unknown_ 0.01 | ch1 |
| raw/supanut/S0/grab/grab_01_raw.wav | grab | wrong | left@1.01s 0.79 | 0.908 | grab 0.91 | right 0.02 | _unknown_ 0.02 | ch1 |
| raw/supanut/S0/blue_box/blue_box_01_raw.wav | blue box | miss |  | 0.002 | _silence_ 0.96 | pick 0.01 | undo 0.00 | ch0 |
| raw/supanut/S0/home/home_02_raw.wav | home | miss |  | 0.004 | _silence_ 0.94 | pick 0.01 | backward 0.00 | ch0 |
| raw/supanut/S0/red_box/red_box_01_raw.wav | red box | wrong | right@1.01s 0.94 | 0.952 | red box 0.95 | green box 0.02 | _unknown_ 0.00 | ch1 |
| raw/supanut/S0/station/station_01_raw.wav | station | miss |  | 0.002 | _silence_ 0.94 | pick 0.01 | drop 0.00 | ch0 |
| raw/supanut/S0/pick/pick_01_raw.wav | pick | miss |  | 0.004 | _silence_ 0.96 | pick 0.00 | home 0.00 | ch0 |
| raw/supanut/S0/black_box/black_box_02_raw.wav | black box | miss |  | 0.002 | _silence_ 0.94 | undo 0.00 | pick 0.00 | ch0 |
| raw/supanut/S0/hand/hand_01_raw.wav | hand | miss |  | 0.002 | _silence_ 0.95 | pick 0.01 | home 0.00 | ch0 |
| raw/supanut/S0/move/move_02_raw.wav | move | miss |  | 0.001 | _silence_ 0.96 | pick 0.01 | drop 0.00 | ch0 |
| raw/supanut/S0/backward/backward_02_raw.wav | backward | miss |  | 0.003 | _silence_ 0.94 | pick 0.01 | grab 0.00 | ch0 |
| raw/supanut/S0/green_box/green_box_01_raw.wav | green box | miss |  | 0.002 | _silence_ 0.95 | pick 0.01 | backward 0.00 | ch0 |
| raw/supanut/S0/base/base_01_raw.wav | base | miss |  | 0.003 | _silence_ 0.90 | pick 0.02 | home 0.01 | ch0 |
| raw/supanut/S0/green_box/green_box_02_raw.wav | green box | miss |  | 0.001 | _silence_ 0.93 | undo 0.01 | no 0.01 | ch0 |
| raw/supanut/S0/no/no_01_raw.wav | no | miss |  | 0.003 | _silence_ 0.92 | pick 0.01 | stop 0.00 | ch0 |
| raw/supanut/S0/blue_box/blue_box_02_raw.wav | blue box | miss |  | 0.002 | _silence_ 0.91 | pick 0.02 | stop 0.01 | ch0 |
| raw/supanut/S0/place/place_02_raw.wav | place | miss |  | 0.003 | _silence_ 0.93 | pick 0.01 | left 0.01 | ch0 |
| raw/supanut/S0/hand/hand_02_raw.wav | hand | miss |  | 0.002 | _silence_ 0.95 | pick 0.01 | home 0.00 | ch0 |
| raw/supanut/S0/right/right_01_raw.wav | right | miss |  | 0.003 | _silence_ 0.94 | pick 0.01 | _unknown_ 0.00 | ch0 |
| raw/supanut/S0/grab/grab_02_raw.wav | grab | miss |  | 0.004 | _silence_ 0.93 | pick 0.01 | drop 0.01 | ch0 |
| raw/supanut/S0/base/base_02_raw.wav | base | miss |  | 0.007 | _silence_ 0.87 | pick 0.04 | yes 0.01 | ch0 |
| raw/supanut/S0/green_box/green_box_03_raw.wav | green box | miss |  | 0.002 | _silence_ 0.95 | pick 0.01 | backward 0.00 | ch0 |
| raw/supanut/S0/backward/backward_03_raw.wav | backward | miss |  | 0.003 | _silence_ 0.94 | pick 0.01 | drop 0.00 | ch0 |
| raw/supanut/S0/backward/backward_04_raw.wav | backward | miss |  | 0.004 | _silence_ 0.93 | pick 0.02 | left 0.00 | ch0 |
| raw/supanut/S0/right/right_02_raw.wav | right | miss |  | 0.004 | _silence_ 0.93 | backward 0.01 | red box 0.00 | ch0 |
| raw/supanut/S0/black_box/black_box_03_raw.wav | black box | miss |  | 0.001 | _silence_ 0.94 | pick 0.01 | home 0.01 | ch0 |
| raw/supanut/S0/green_box/green_box_04_raw.wav | green box | miss |  | 0.002 | _silence_ 0.92 | undo 0.01 | grab 0.01 | ch0 |
| raw/supanut/S0/base/base_03_raw.wav | base | miss |  | 0.002 | _silence_ 0.93 | undo 0.01 | pick 0.01 | ch0 |
| raw/supanut/S0/green_box/green_box_05_raw.wav | green box | miss |  | 0.003 | _silence_ 0.95 | backward 0.01 | undo 0.00 | ch0 |
| raw/supanut/S0/red_box/red_box_02_raw.wav | red box | miss |  | 0.003 | _silence_ 0.92 | pick 0.01 | backward 0.00 | ch0 |
| raw/supanut/S0/stop/stop_02_raw.wav | stop | miss |  | 0.002 | _silence_ 0.96 | pick 0.00 | undo 0.00 | ch0 |
| raw/supanut/S0/backward/backward_05_raw.wav | backward | miss |  | 0.003 | _silence_ 0.95 | pick 0.00 | home 0.00 | ch0 |
| raw/supanut/S0/blue_box/blue_box_03_raw.wav | blue box | miss |  | 0.001 | _silence_ 0.95 | pick 0.01 | home 0.00 | ch0 |
| raw/supanut/S0/home/home_03_raw.wav | home | miss |  | 0.005 | _silence_ 0.93 | undo 0.01 | pick 0.01 | ch0 |
| raw/supanut/S0/green_box/green_box_06_raw.wav | green box | miss |  | 0.003 | _silence_ 0.94 | pick 0.01 | backward 0.00 | ch0 |
| raw/supanut/S0/grab/grab_03_raw.wav | grab | miss |  | 0.002 | _silence_ 0.96 | pick 0.01 | backward 0.00 | ch0 |
| raw/supanut/S0/black_box/black_box_04_raw.wav | black box | miss |  | 0.002 | _silence_ 0.94 | pick 0.01 | undo 0.00 | ch0 |
| raw/supanut/S0/left/left_01_raw.wav | left | miss |  | 0.003 | _silence_ 0.95 | pick 0.00 | hand 0.00 | ch0 |
| raw/supanut/S0/station/station_02_raw.wav | station | wrong | base@1.01s 0.79 | 0.957 | station 0.96 | _unknown_ 0.01 | base 0.01 | ch1 |
| raw/supanut/S0/move/move_03_raw.wav | move | miss |  | 0.004 | _silence_ 0.88 | forward 0.02 | pick 0.01 | ch0 |
| raw/supanut/S0/no/no_02_raw.wav | no | miss |  | 0.002 | _silence_ 0.94 | pick 0.01 | stop 0.00 | ch0 |
| raw/supanut/S0/blue_box/blue_box_04_raw.wav | blue box | miss |  | 0.01 | _silence_ 0.50 | pick 0.08 | stop 0.05 | ch0 |
| raw/supanut/S0/stop/stop_03_raw.wav | stop | miss |  | 0.025 | _silence_ 0.75 | pick 0.03 | stop 0.02 | ch0 |
| raw/supanut/S0/place/place_04_raw.wav | place | miss |  | 0.024 | _silence_ 0.43 | stop 0.22 | blue box 0.06 | ch0 |
| raw/supanut/S0/grab/grab_06_raw.wav | grab | miss |  | 0.04 | _silence_ 0.50 | drop 0.09 | pick 0.04 | ch0 |
| raw/supanut/S0/forward/forward_01_raw.wav | forward | miss |  | 0.005 | _silence_ 0.42 | stop 0.12 | red box 0.07 | ch0 |
| raw/supanut/S0/station/station_03_raw.wav | station | miss |  | 0.007 | _silence_ 0.60 | stop 0.04 | grab 0.04 | ch0 |
| raw/supanut/S0/backward/backward_06_raw.wav | backward | miss |  | 0.011 | _silence_ 0.80 | grab 0.02 | stop 0.02 | ch0 |
| raw/supanut/S0/place/place_06_raw.wav | place | miss |  | 0.014 | _silence_ 0.48 | stop 0.08 | home 0.06 | ch0 |
| raw/supanut/S0/hand/hand_03_raw.wav | hand | miss |  | 0.012 | pick 0.18 | stop 0.16 | drop 0.13 | ch0 |
| raw/supanut/S0/hand/hand_04_raw.wav | hand | miss |  | 0.006 | _silence_ 0.63 | pick 0.17 | drop 0.03 | ch0 |
| raw/supanut/S0/station/station_04_raw.wav | station | miss |  | 0.008 | _silence_ 0.51 | left 0.10 | pick 0.04 | ch0 |
| raw/supanut/S0/forward/forward_02_raw.wav | forward | miss |  | 0.007 | _silence_ 0.33 | stop 0.13 | pick 0.10 | ch0 |
| raw/supanut/S0/move/move_04_raw.wav | move | miss |  | 0.005 | _silence_ 0.76 | pick 0.06 | right 0.02 | ch0 |
| raw/supanut/S0/station/station_05_raw.wav | station | miss |  | 0.003 | _silence_ 0.93 | pick 0.01 | undo 0.00 | ch0 |
| raw/supanut/S0/backward/backward_07_raw.wav | backward | miss |  | 0.646 | backward 0.65 | forward 0.22 | _unknown_ 0.05 | ch0/ch1 |
| raw/supanut/S0/home/home_04_raw.wav | home | miss |  | 0.005 | _silence_ 0.92 | pick 0.01 | undo 0.01 | ch0 |
| raw/supanut/S0/right/right_03_raw.wav | right | miss |  | 0.006 | _silence_ 0.86 | stop 0.04 | grab 0.01 | ch0 |
| raw/supanut/S0/_unknown_/bright_01_raw.wav | _unknown_ | false_alarm | right@1.31s 0.98 | 0.005 | right 0.99 | _unknown_ 0.00 | drop 0.00 | ch0/ch1 |
| raw/supanut/S0/left/left_03_raw.wav | left | miss |  | 0.003 | _silence_ 0.93 | pick 0.01 | undo 0.01 | ch0 |
| raw/supanut/S0/_unknown_/open_01_raw.wav | _unknown_ | false_alarm | home@1.01s 0.94 | 0.636 | home 0.94 | _unknown_ 0.01 | forward 0.01 | ch0/ch1 |
| raw/supanut/S0/blue_box/blue_box_05_raw.wav | blue box | wrong | move@1.01s 0.90 | 0.831 | move 0.90 | no 0.02 | _unknown_ 0.02 | ch1 |
| raw/supanut/S0/stop/stop_04_raw.wav | stop | miss |  | 0.003 | _silence_ 0.89 | pick 0.01 | hand 0.01 | ch0 |
| raw/supanut/S0/blue_box/blue_box_06_raw.wav | blue box | wrong | no@1.01s 0.90 | 0.922 | blue box 0.92 | green box 0.02 | _unknown_ 0.01 | ch1 |
| raw/supanut/S0/base/base_04_raw.wav | base | miss |  | 0.003 | _silence_ 0.92 | pick 0.01 | undo 0.01 | ch0 |
| raw/supanut/S0/blue_box/blue_box_07_raw.wav | blue box | wrong | move@1.01s 0.85 | 0.587 | move 0.85 | no 0.05 | _unknown_ 0.02 | ch0/ch1 |
| raw/supanut/S0/left/left_05_raw.wav | left | miss |  | 0.012 | _silence_ 0.66 | pick 0.13 | stop 0.03 | ch0 |
| raw/supanut/S0/place/place_07_raw.wav | place | miss |  | 0.014 | _silence_ 0.61 | stop 0.11 | left 0.04 | ch0 |
| raw/supanut/S0/no/no_03_raw.wav | no | miss |  | 0.014 | _silence_ 0.43 | pick 0.09 | home 0.07 | ch0 |
| raw/supanut/S0/_unknown_/blue_01_raw.wav | _unknown_ | false_alarm | no@1.01s 0.72 | 0.03 | no 0.94 | move 0.03 | _unknown_ 0.01 | ch1 |
| raw/supanut/S0/_unknown_/bag_01_raw.wav | _unknown_ | false_alarm | backward@1.01s 0.81 | 0.059 | backward 0.81 | left 0.07 | _unknown_ 0.03 | ch1 |
| raw/supanut/S0/_unknown_/groove_01_raw.wav | _unknown_ | false_alarm | move@1.01s 0.81 | 0.06 | move 0.83 | no 0.05 | home 0.04 | ch1 |
