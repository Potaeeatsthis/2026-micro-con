# Board evaluation - exp01_tcresnet-w16d2_aug-all_fold2 on S0

model `D:\Microcon69\ProjectRoboticArm\ProjectSpace\2026-micro-con\voice_command_research\workspace\experiments\exp01_tcresnet-w16d2_aug-all\runs\fold2\best.pth` (pytorch TC-ResNet w16 d2, 23 classes) · gain +15.0 dB · features librosa 0.11.0 (features.py) · 2026-10-21 17:35

## Clip mode (1 s clips, mono ch1)

n 109 · **accuracy 0.8349** · **macro-F1 0.8428** · commands 0.9681 · _unknown_ 0.0 · _silence_ None · commands rejected as unknown/silence 0 · speech peak median -32.4 dB (training clip -25.4 dB)

| cls | support | correct | recall | precision | f1 | mean_p_true | confused_with |
|---|---|---|---|---|---|---|---|
| move | 5 | 5 | 1.0 | 0.714 | 0.833 | 0.76 |  |
| pick | 2 | 2 | 1.0 | 0.667 | 0.8 | 0.995 |  |
| grab | 7 | 7 | 1.0 | 1.0 | 1.0 | 0.85 |  |
| blue box | 7 | 7 | 1.0 | 1.0 | 1.0 | 0.593 |  |
| green box | 6 | 6 | 1.0 | 0.857 | 0.923 | 0.98 |  |
| red box | 3 | 2 | 0.667 | 0.667 | 0.667 | 0.623 | green box 1 |
| black box | 4 | 4 | 1.0 | 1.0 | 1.0 | 0.759 |  |
| hand | 5 | 5 | 1.0 | 1.0 | 1.0 | 0.928 |  |
| station | 6 | 6 | 1.0 | 1.0 | 1.0 | 0.916 |  |
| base | 5 | 5 | 1.0 | 0.833 | 0.909 | 0.838 |  |
| forward | 4 | 4 | 1.0 | 1.0 | 1.0 | 0.984 |  |
| backward | 7 | 7 | 1.0 | 1.0 | 1.0 | 0.999 |  |
| left | 5 | 5 | 1.0 | 0.833 | 0.909 | 0.885 |  |
| right | 3 | 3 | 1.0 | 0.5 | 0.667 | 0.999 |  |
| place | 7 | 6 | 0.857 | 0.857 | 0.857 | 0.688 | base 1 |
| drop | 2 | 2 | 1.0 | 1.0 | 1.0 | 0.901 |  |
| yes | 2 | 1 | 0.5 | 1.0 | 0.667 | 0.523 | place 1 |
| no | 4 | 4 | 1.0 | 0.667 | 0.8 | 0.989 |  |
| home | 5 | 5 | 1.0 | 0.5 | 0.667 | 0.979 |  |
| stop | 5 | 5 | 1.0 | 1.0 | 1.0 | 0.993 |  |
| _unknown_ | 15 | 0 | 0.0 | 0.0 | 0.0 | 0.049 | home 5, right 3, move 2 |

### Mistakes (top-3)

| clip_path | label | pred | top3 | snr_db | flags |
|---|---|---|---|---|---|
| clips/supanut/S0/_unknown_/pink_01.wav | _unknown_ | pick | pick 0.77 | _unknown_ 0.08 | hand 0.08 | 19.61 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/nose_01.wav | _unknown_ | no | no 0.96 | yes 0.02 | _unknown_ 0.01 | 24.64 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/write_01.wav | _unknown_ | right | right 1.00 | _unknown_ 0.00 | backward 0.00 | 25.15 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/mood_01.wav | _unknown_ | move | move 0.83 | forward 0.05 | _unknown_ 0.03 | 22.31 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/phone_01.wav | _unknown_ | home | home 0.55 | forward 0.37 | _unknown_ 0.04 | 24.42 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/place/place_06.wav | place | base | base 0.42 | place 0.34 | _unknown_ 0.07 | 18.83 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/moon_01.wav | _unknown_ | home | home 0.57 | move 0.17 | hand 0.09 | 23.7 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/go_01.wav | _unknown_ | home | home 0.74 | hand 0.06 | move 0.03 | 27.44 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/foam_01.wav | _unknown_ | home | home 0.97 | hand 0.01 | _unknown_ 0.01 | 26.15 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/red_box/red_box_03.wav | red box | green box | green box 0.74 | red box 0.17 | _unknown_ 0.03 | 23.48 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/bright_01.wav | _unknown_ | right | right 1.00 | _unknown_ 0.00 | backward 0.00 | 24.09 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/open_01.wav | _unknown_ | home | home 0.57 | _unknown_ 0.21 | forward 0.12 | 24.52 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/yes/yes_02.wav | yes | place | place 0.35 | base 0.33 | _unknown_ 0.07 | 16.0 | sound longer than 1 s - the clip cuts it; S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/robot_01.wav | _unknown_ | red box | red box 0.64 | green box 0.06 | _unknown_ 0.06 | 22.74 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/wait_01.wav | _unknown_ | right | right 0.36 | place 0.34 | _unknown_ 0.09 | 19.73 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/blue_01.wav | _unknown_ | no | no 0.82 | move 0.16 | undo 0.01 | 23.87 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/bag_01.wav | _unknown_ | left | left 0.56 | backward 0.25 | grab 0.05 | 17.09 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/groove_01.wav | _unknown_ | move | move 0.52 | home 0.12 | _unknown_ 0.07 | 26.41 | S0 old wiring (VDD ~1.8 V) |

## Stream mode (raw 2 s takes, board logic: stride 10 frames, avg 3, threshold 0.7, refractory 1500 ms, channel auto)

command takes 94: **detected 0.2766** (hit 26, hit+extra 0), wrong 4, miss 64 · first detection correct 0.2766 · _unknown_ false alarms 3/15 · noise false alarms 0 in 0.0 min · channels {'ch1': 27, 'ch0/ch1': 16, 'ch0': 66}

| cls | takes | hit | hit_extra | wrong | miss | false_alarm | detected_rate | wrong_as | mean_max_p_true |
|---|---|---|---|---|---|---|---|---|---|
| move | 5 | 2 | 0 | 0 | 3 | 0 | 0.4 |  | 0.362 |
| pick | 2 | 1 | 0 | 0 | 1 | 0 | 0.5 |  | 0.5 |
| grab | 7 | 3 | 0 | 0 | 4 | 0 | 0.429 |  | 0.479 |
| blue box | 7 | 1 | 0 | 1 | 5 | 0 | 0.143 | no 1 | 0.262 |
| green box | 6 | 0 | 0 | 0 | 6 | 0 | 0.0 |  | 0.001 |
| red box | 3 | 0 | 0 | 1 | 2 | 0 | 0.0 | right 1 | 0.396 |
| black box | 4 | 0 | 0 | 1 | 3 | 0 | 0.0 | left 1 | 0.208 |
| hand | 5 | 1 | 0 | 0 | 4 | 0 | 0.2 |  | 0.202 |
| station | 6 | 2 | 0 | 0 | 4 | 0 | 0.333 |  | 0.317 |
| base | 5 | 1 | 0 | 0 | 4 | 0 | 0.2 |  | 0.165 |
| forward | 4 | 2 | 0 | 0 | 2 | 0 | 0.5 |  | 0.494 |
| backward | 7 | 0 | 0 | 1 | 6 | 0 | 0.0 | left 1 | 0.207 |
| left | 5 | 2 | 0 | 0 | 3 | 0 | 0.4 |  | 0.348 |
| right | 3 | 0 | 0 | 0 | 3 | 0 | 0.0 |  | 0.002 |
| place | 7 | 2 | 0 | 0 | 5 | 0 | 0.286 |  | 0.346 |
| drop | 2 | 2 | 0 | 0 | 0 | 0 | 1.0 |  | 0.897 |
| yes | 2 | 2 | 0 | 0 | 0 | 0 | 1.0 |  | 0.992 |
| no | 4 | 1 | 0 | 0 | 3 | 0 | 0.25 |  | 0.252 |
| home | 5 | 2 | 0 | 0 | 3 | 0 | 0.4 |  | 0.395 |
| stop | 5 | 2 | 0 | 0 | 3 | 0 | 0.4 |  | 0.398 |
| _unknown_ | 15 | 0 | 0 | 0 | 0 | 3 |  | right 1, home 1, no 1 | 0.048 |

### Takes that were not a clean hit

| raw_path | label | outcome | detections | max_avg_p_true | peak_top3 | channels_used |
|---|---|---|---|---|---|---|
| raw/supanut/S0/black_box/black_box_01_raw.wav | black box | wrong | left@1.01s 0.75 | 0.83 | black box 0.83 | backward 0.07 | red box 0.03 | ch1 |
| raw/supanut/S0/grab/grab_01_raw.wav | grab | miss |  | 0.666 | grab 0.67 | left 0.19 | right 0.06 | ch1 |
| raw/supanut/S0/blue_box/blue_box_01_raw.wav | blue box | miss |  | 0.0 | _silence_ 0.96 | move 0.00 | home 0.00 | ch0 |
| raw/supanut/S0/home/home_02_raw.wav | home | miss |  | 0.003 | _silence_ 0.97 | move 0.00 | left 0.00 | ch0 |
| raw/supanut/S0/red_box/red_box_01_raw.wav | red box | wrong | right@1.01s 0.76 | 0.827 | red box 0.83 | green box 0.07 | black box 0.03 | ch1 |
| raw/supanut/S0/backward/backward_01_raw.wav | backward | wrong | left@1.01s 0.76 | 1.0 | backward 1.00 | _unknown_ 0.00 | black box 0.00 | ch1 |
| raw/supanut/S0/station/station_01_raw.wav | station | miss |  | 0.001 | _silence_ 0.97 | left 0.00 | move 0.00 | ch0 |
| raw/supanut/S0/pick/pick_01_raw.wav | pick | miss |  | 0.006 | _silence_ 0.92 | hand 0.01 | move 0.01 | ch0 |
| raw/supanut/S0/black_box/black_box_02_raw.wav | black box | miss |  | 0.0 | _silence_ 0.93 | hand 0.01 | move 0.01 | ch0 |
| raw/supanut/S0/hand/hand_01_raw.wav | hand | miss |  | 0.007 | _silence_ 0.94 | left 0.01 | hand 0.01 | ch0 |
| raw/supanut/S0/move/move_02_raw.wav | move | miss |  | 0.005 | _silence_ 0.95 | left 0.01 | move 0.00 | ch0 |
| raw/supanut/S0/backward/backward_02_raw.wav | backward | miss |  | 0.002 | _silence_ 0.94 | hand 0.01 | home 0.01 | ch0 |
| raw/supanut/S0/green_box/green_box_01_raw.wav | green box | miss |  | 0.0 | _silence_ 0.96 | hand 0.01 | home 0.00 | ch0 |
| raw/supanut/S0/base/base_01_raw.wav | base | miss |  | 0.005 | _silence_ 0.89 | hand 0.01 | left 0.01 | ch0 |
| raw/supanut/S0/green_box/green_box_02_raw.wav | green box | miss |  | 0.001 | _silence_ 0.92 | left 0.01 | hand 0.01 | ch0 |
| raw/supanut/S0/no/no_01_raw.wav | no | miss |  | 0.005 | _silence_ 0.89 | hand 0.02 | home 0.01 | ch0 |
| raw/supanut/S0/blue_box/blue_box_02_raw.wav | blue box | miss |  | 0.001 | _silence_ 0.92 | hand 0.01 | left 0.01 | ch0 |
| raw/supanut/S0/place/place_02_raw.wav | place | miss |  | 0.001 | _silence_ 0.95 | left 0.01 | hand 0.01 | ch0 |
| raw/supanut/S0/hand/hand_02_raw.wav | hand | miss |  | 0.005 | _silence_ 0.95 | left 0.01 | pick 0.01 | ch0 |
| raw/supanut/S0/right/right_01_raw.wav | right | miss |  | 0.002 | _silence_ 0.97 | left 0.00 | move 0.00 | ch0 |
| raw/supanut/S0/grab/grab_02_raw.wav | grab | miss |  | 0.003 | _silence_ 0.95 | left 0.01 | pick 0.00 | ch0 |
| raw/supanut/S0/base/base_02_raw.wav | base | miss |  | 0.003 | _silence_ 0.94 | left 0.01 | home 0.01 | ch0 |
| raw/supanut/S0/green_box/green_box_03_raw.wav | green box | miss |  | 0.001 | _silence_ 0.92 | left 0.01 | pick 0.01 | ch0 |
| raw/supanut/S0/backward/backward_03_raw.wav | backward | miss |  | 0.001 | _silence_ 0.97 | left 0.00 | home 0.00 | ch0 |
| raw/supanut/S0/backward/backward_04_raw.wav | backward | miss |  | 0.002 | _silence_ 0.92 | hand 0.01 | home 0.01 | ch0 |
| raw/supanut/S0/right/right_02_raw.wav | right | miss |  | 0.002 | _silence_ 0.95 | hand 0.01 | home 0.01 | ch0 |
| raw/supanut/S0/black_box/black_box_03_raw.wav | black box | miss |  | 0.0 | _silence_ 0.97 | left 0.00 | hand 0.00 | ch0 |
| raw/supanut/S0/green_box/green_box_04_raw.wav | green box | miss |  | 0.0 | _silence_ 0.95 | left 0.01 | hand 0.00 | ch0 |
| raw/supanut/S0/base/base_03_raw.wav | base | miss |  | 0.002 | _silence_ 0.95 | hand 0.01 | home 0.01 | ch0 |
| raw/supanut/S0/green_box/green_box_05_raw.wav | green box | miss |  | 0.001 | _silence_ 0.93 | left 0.01 | hand 0.01 | ch0 |
| raw/supanut/S0/red_box/red_box_02_raw.wav | red box | miss |  | 0.001 | _silence_ 0.94 | left 0.01 | move 0.01 | ch0 |
| raw/supanut/S0/stop/stop_02_raw.wav | stop | miss |  | 0.003 | _silence_ 0.94 | left 0.01 | pick 0.01 | ch0 |
| raw/supanut/S0/backward/backward_05_raw.wav | backward | miss |  | 0.001 | _silence_ 0.96 | left 0.00 | move 0.00 | ch0 |
| raw/supanut/S0/blue_box/blue_box_03_raw.wav | blue box | miss |  | 0.0 | _silence_ 0.95 | left 0.01 | pick 0.00 | ch0 |
| raw/supanut/S0/home/home_03_raw.wav | home | miss |  | 0.006 | _silence_ 0.94 | home 0.01 | hand 0.01 | ch0 |
| raw/supanut/S0/green_box/green_box_06_raw.wav | green box | miss |  | 0.0 | _silence_ 0.95 | hand 0.01 | move 0.01 | ch0 |
| raw/supanut/S0/grab/grab_03_raw.wav | grab | miss |  | 0.003 | _silence_ 0.95 | hand 0.01 | move 0.01 | ch0 |
| raw/supanut/S0/black_box/black_box_04_raw.wav | black box | miss |  | 0.001 | _silence_ 0.92 | left 0.01 | pick 0.01 | ch0 |
| raw/supanut/S0/left/left_01_raw.wav | left | miss |  | 0.005 | _silence_ 0.95 | left 0.01 | pick 0.01 | ch0 |
| raw/supanut/S0/move/move_03_raw.wav | move | miss |  | 0.003 | _silence_ 0.97 | move 0.00 | home 0.00 | ch0 |
| raw/supanut/S0/no/no_02_raw.wav | no | miss |  | 0.005 | _silence_ 0.85 | hand 0.04 | home 0.02 | ch0 |
| raw/supanut/S0/blue_box/blue_box_04_raw.wav | blue box | miss |  | 0.001 | _silence_ 0.83 | hand 0.02 | left 0.02 | ch0 |
| raw/supanut/S0/stop/stop_03_raw.wav | stop | miss |  | 0.005 | _silence_ 0.94 | pick 0.01 | hand 0.01 | ch0 |
| raw/supanut/S0/place/place_04_raw.wav | place | miss |  | 0.003 | _silence_ 0.91 | hand 0.02 | home 0.01 | ch0 |
| raw/supanut/S0/grab/grab_06_raw.wav | grab | miss |  | 0.006 | _silence_ 0.91 | hand 0.01 | home 0.01 | ch0 |
| raw/supanut/S0/forward/forward_01_raw.wav | forward | miss |  | 0.003 | _silence_ 0.88 | stop 0.02 | home 0.02 | ch0 |
| raw/supanut/S0/station/station_03_raw.wav | station | miss |  | 0.0 | _silence_ 0.96 | move 0.01 | home 0.00 | ch0 |
| raw/supanut/S0/place/place_05_raw.wav | place | miss |  | 0.601 | place 0.60 | base 0.33 | right 0.02 | ch1 |
| raw/supanut/S0/backward/backward_06_raw.wav | backward | miss |  | 0.003 | _silence_ 0.87 | hand 0.02 | home 0.02 | ch0 |
| raw/supanut/S0/place/place_06_raw.wav | place | miss |  | 0.0 | _silence_ 0.96 | left 0.01 | pick 0.01 | ch0 |
| raw/supanut/S0/hand/hand_03_raw.wav | hand | miss |  | 0.009 | _silence_ 0.92 | stop 0.02 | home 0.02 | ch0 |
| raw/supanut/S0/hand/hand_04_raw.wav | hand | miss |  | 0.004 | _silence_ 0.89 | pick 0.02 | drop 0.02 | ch0 |
| raw/supanut/S0/station/station_04_raw.wav | station | miss |  | 0.001 | _silence_ 0.95 | home 0.01 | left 0.01 | ch0 |
| raw/supanut/S0/forward/forward_02_raw.wav | forward | miss |  | 0.005 | _silence_ 0.78 | left 0.04 | drop 0.03 | ch0 |
| raw/supanut/S0/move/move_04_raw.wav | move | miss |  | 0.034 | _silence_ 0.74 | hand 0.05 | move 0.03 | ch0 |
| raw/supanut/S0/station/station_05_raw.wav | station | miss |  | 0.0 | _silence_ 0.98 | home 0.00 | hand 0.00 | ch0 |
| raw/supanut/S0/red_box/red_box_03_raw.wav | red box | miss |  | 0.361 | green box 0.62 | red box 0.29 | _unknown_ 0.03 | ch1 |
| raw/supanut/S0/backward/backward_07_raw.wav | backward | miss |  | 0.44 | backward 0.44 | left 0.20 | forward 0.11 | ch0/ch1 |
| raw/supanut/S0/home/home_04_raw.wav | home | miss |  | 0.006 | _silence_ 0.94 | hand 0.01 | pick 0.01 | ch0 |
| raw/supanut/S0/right/right_03_raw.wav | right | miss |  | 0.003 | _silence_ 0.92 | pick 0.01 | left 0.01 | ch0 |
| raw/supanut/S0/_unknown_/bright_01_raw.wav | _unknown_ | false_alarm | right@1.31s 0.99 | 0.003 | right 1.00 | _unknown_ 0.00 | grab 0.00 | ch0/ch1 |
| raw/supanut/S0/left/left_03_raw.wav | left | miss |  | 0.003 | _silence_ 0.98 | move 0.00 | home 0.00 | ch0 |
| raw/supanut/S0/_unknown_/open_01_raw.wav | _unknown_ | false_alarm | home@1.01s 0.87 | 0.283 | home 0.87 | hand 0.04 | forward 0.03 | ch0/ch1 |
| raw/supanut/S0/stop/stop_04_raw.wav | stop | miss |  | 0.003 | _silence_ 0.96 | home 0.01 | hand 0.01 | ch0 |
| raw/supanut/S0/blue_box/blue_box_06_raw.wav | blue box | miss |  | 0.618 | blue box 0.62 | green box 0.15 | red box 0.05 | ch1 |
| raw/supanut/S0/base/base_04_raw.wav | base | miss |  | 0.002 | _silence_ 0.96 | move 0.00 | hand 0.00 | ch0 |
| raw/supanut/S0/blue_box/blue_box_07_raw.wav | blue box | wrong | no@1.01s 0.75 | 0.497 | no 0.75 | move 0.17 | blue box 0.02 | ch0/ch1 |
| raw/supanut/S0/left/left_05_raw.wav | left | miss |  | 0.004 | _silence_ 0.95 | hand 0.01 | grab 0.00 | ch0 |
| raw/supanut/S0/place/place_07_raw.wav | place | miss |  | 0.005 | _silence_ 0.66 | pick 0.07 | backward 0.05 | ch0 |
| raw/supanut/S0/no/no_03_raw.wav | no | miss |  | 0.003 | _silence_ 0.94 | pick 0.01 | stop 0.01 | ch0 |
| raw/supanut/S0/_unknown_/blue_01_raw.wav | _unknown_ | false_alarm | no@1.31s 0.85 | 0.043 | no 0.85 | move 0.12 | undo 0.01 | ch1 |
