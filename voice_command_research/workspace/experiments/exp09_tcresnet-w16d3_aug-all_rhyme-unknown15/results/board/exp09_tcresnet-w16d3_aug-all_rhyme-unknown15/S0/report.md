# Board evaluation - exp09_tcresnet-w16d3_aug-all_rhyme-unknown15 on S0

model `D:\Microcon69\ProjectRoboticArm\ProjectSpace\2026-micro-con\voice_command_research\workspace\experiments\exp09_tcresnet-w16d3_aug-all_rhyme-unknown15\runs\final.pth` (pytorch exp09_tcresnet-w16d3_aug-all_rhyme-unknown15, 23 classes) · gain +15.0 dB · features librosa 0.11.0 (features.py) · 2026-10-21 22:46

## Clip mode (1 s clips, mono ch1)

n 109 · **accuracy 0.8624** · **macro-F1 0.868** · commands 0.9681 · _unknown_ 0.2 · _silence_ None · commands rejected as unknown/silence 0 · speech peak median -32.4 dB (training clip -25.4 dB)

| cls | support | correct | recall | precision | f1 | mean_p_true | confused_with |
|---|---|---|---|---|---|---|---|
| move | 5 | 4 | 0.8 | 0.667 | 0.727 | 0.766 | no 1 |
| pick | 2 | 2 | 1.0 | 0.667 | 0.8 | 0.987 |  |
| grab | 7 | 7 | 1.0 | 1.0 | 1.0 | 0.909 |  |
| blue box | 7 | 7 | 1.0 | 1.0 | 1.0 | 0.926 |  |
| green box | 6 | 6 | 1.0 | 0.857 | 0.923 | 0.982 |  |
| red box | 3 | 2 | 0.667 | 1.0 | 0.8 | 0.713 | green box 1 |
| black box | 4 | 4 | 1.0 | 1.0 | 1.0 | 0.907 |  |
| hand | 5 | 5 | 1.0 | 1.0 | 1.0 | 0.843 |  |
| station | 6 | 6 | 1.0 | 1.0 | 1.0 | 0.901 |  |
| base | 5 | 5 | 1.0 | 1.0 | 1.0 | 0.774 |  |
| forward | 4 | 4 | 1.0 | 0.8 | 0.889 | 0.917 |  |
| backward | 7 | 7 | 1.0 | 1.0 | 1.0 | 0.966 |  |
| left | 5 | 5 | 1.0 | 0.833 | 0.909 | 0.932 |  |
| right | 3 | 3 | 1.0 | 0.6 | 0.75 | 0.988 |  |
| place | 7 | 7 | 1.0 | 0.875 | 0.933 | 0.907 |  |
| drop | 2 | 2 | 1.0 | 1.0 | 1.0 | 0.913 |  |
| yes | 2 | 1 | 0.5 | 1.0 | 0.667 | 0.507 | place 1 |
| no | 4 | 4 | 1.0 | 0.571 | 0.727 | 0.983 |  |
| home | 5 | 5 | 1.0 | 0.625 | 0.769 | 0.961 |  |
| stop | 5 | 5 | 1.0 | 1.0 | 1.0 | 0.964 |  |
| _unknown_ | 15 | 3 | 0.2 | 1.0 | 0.333 | 0.171 | home 3, move 2, right 2 |

### Mistakes (top-3)

| clip_path | label | pred | top3 | snr_db | flags |
|---|---|---|---|---|---|
| clips/supanut/S0/_unknown_/pink_01.wav | _unknown_ | pick | pick 0.69 | _unknown_ 0.18 | hand 0.06 | 19.61 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/move/move_02.wav | move | no | no 0.36 | move 0.32 | hand 0.13 | 22.06 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/nose_01.wav | _unknown_ | no | no 0.96 | _unknown_ 0.02 | move 0.01 | 24.64 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/write_01.wav | _unknown_ | right | right 0.98 | _unknown_ 0.01 | left 0.00 | 25.15 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/mood_01.wav | _unknown_ | move | move 0.92 | home 0.02 | _unknown_ 0.02 | 22.31 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/phone_01.wav | _unknown_ | forward | forward 0.60 | _unknown_ 0.17 | home 0.13 | 24.42 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/moon_01.wav | _unknown_ | home | home 0.51 | move 0.23 | _unknown_ 0.10 | 23.7 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/go_01.wav | _unknown_ | home | home 0.91 | _unknown_ 0.04 | forward 0.02 | 27.44 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/foam_01.wav | _unknown_ | home | home 0.85 | forward 0.06 | _unknown_ 0.05 | 26.15 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/red_box/red_box_03.wav | red box | green box | green box 0.56 | red box 0.31 | blue box 0.03 | 23.48 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/bright_01.wav | _unknown_ | right | right 0.94 | _unknown_ 0.05 | left 0.00 | 24.09 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/yes/yes_02.wav | yes | place | place 0.67 | base 0.19 | _unknown_ 0.06 | 16.0 | sound longer than 1 s - the clip cuts it; S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/blue_01.wav | _unknown_ | no | no 0.84 | move 0.12 | _unknown_ 0.01 | 23.87 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/bag_01.wav | _unknown_ | left | left 0.42 | backward 0.23 | _unknown_ 0.13 | 17.09 | S0 old wiring (VDD ~1.8 V) |
| clips/supanut/S0/_unknown_/groove_01.wav | _unknown_ | move | move 0.90 | no 0.03 | home 0.03 | 26.41 | S0 old wiring (VDD ~1.8 V) |

## Stream mode (raw 2 s takes, board logic: stride 10 frames, avg 3, threshold 0.7, refractory 1500 ms, channel auto)

command takes 94: **detected 0.3298** (hit 31, hit+extra 0), wrong 1, miss 62 · first detection correct 0.3298 · _unknown_ false alarms 4/15 · noise false alarms 0 in 0.0 min · channels {'ch1': 27, 'ch0/ch1': 16, 'ch0': 66}

| cls | takes | hit | hit_extra | wrong | miss | false_alarm | detected_rate | wrong_as | mean_max_p_true |
|---|---|---|---|---|---|---|---|---|---|
| move | 5 | 1 | 0 | 0 | 4 | 0 | 0.2 |  | 0.323 |
| pick | 2 | 1 | 0 | 0 | 1 | 0 | 0.5 |  | 0.495 |
| grab | 7 | 4 | 0 | 0 | 3 | 0 | 0.571 |  | 0.516 |
| blue box | 7 | 3 | 0 | 0 | 4 | 0 | 0.429 |  | 0.369 |
| green box | 6 | 0 | 0 | 0 | 6 | 0 | 0.0 |  | 0.001 |
| red box | 3 | 0 | 0 | 1 | 2 | 0 | 0.0 | right 1 | 0.478 |
| black box | 4 | 1 | 0 | 0 | 3 | 0 | 0.25 |  | 0.235 |
| hand | 5 | 1 | 0 | 0 | 4 | 0 | 0.2 |  | 0.187 |
| station | 6 | 2 | 0 | 0 | 4 | 0 | 0.333 |  | 0.295 |
| base | 5 | 1 | 0 | 0 | 4 | 0 | 0.2 |  | 0.178 |
| forward | 4 | 2 | 0 | 0 | 2 | 0 | 0.5 |  | 0.467 |
| backward | 7 | 1 | 0 | 0 | 6 | 0 | 0.143 |  | 0.219 |
| left | 5 | 2 | 0 | 0 | 3 | 0 | 0.4 |  | 0.364 |
| right | 3 | 0 | 0 | 0 | 3 | 0 | 0.0 |  | 0.006 |
| place | 7 | 3 | 0 | 0 | 4 | 0 | 0.429 |  | 0.423 |
| drop | 2 | 2 | 0 | 0 | 0 | 0 | 1.0 |  | 0.92 |
| yes | 2 | 2 | 0 | 0 | 0 | 0 | 1.0 |  | 0.98 |
| no | 4 | 1 | 0 | 0 | 3 | 0 | 0.25 |  | 0.253 |
| home | 5 | 2 | 0 | 0 | 3 | 0 | 0.4 |  | 0.389 |
| stop | 5 | 2 | 0 | 0 | 3 | 0 | 0.4 |  | 0.394 |
| _unknown_ | 15 | 0 | 0 | 0 | 0 | 4 |  | right 1, home 1, no 1 | 0.174 |

### Takes that were not a clean hit

| raw_path | label | outcome | detections | max_avg_p_true | peak_top3 | channels_used |
|---|---|---|---|---|---|---|
| raw/supanut/S0/blue_box/blue_box_01_raw.wav | blue box | miss |  | 0.001 | _silence_ 0.98 | left 0.00 | pick 0.00 | ch0 |
| raw/supanut/S0/home/home_02_raw.wav | home | miss |  | 0.002 | _silence_ 0.97 | left 0.00 | place 0.00 | ch0 |
| raw/supanut/S0/red_box/red_box_01_raw.wav | red box | wrong | right@1.01s 0.79 | 0.947 | red box 0.95 | green box 0.03 | blue box 0.01 | ch1 |
| raw/supanut/S0/station/station_01_raw.wav | station | miss |  | 0.001 | _silence_ 0.96 | place 0.01 | pick 0.00 | ch0 |
| raw/supanut/S0/pick/pick_01_raw.wav | pick | miss |  | 0.003 | _silence_ 0.97 | pick 0.00 | left 0.00 | ch0 |
| raw/supanut/S0/black_box/black_box_02_raw.wav | black box | miss |  | 0.001 | _silence_ 0.97 | place 0.00 | left 0.00 | ch0 |
| raw/supanut/S0/hand/hand_01_raw.wav | hand | miss |  | 0.001 | _silence_ 0.97 | place 0.00 | pick 0.00 | ch0 |
| raw/supanut/S0/move/move_02_raw.wav | move | miss |  | 0.001 | _silence_ 0.94 | pick 0.01 | left 0.01 | ch0 |
| raw/supanut/S0/backward/backward_02_raw.wav | backward | miss |  | 0.002 | _silence_ 0.97 | left 0.00 | place 0.00 | ch0 |
| raw/supanut/S0/green_box/green_box_01_raw.wav | green box | miss |  | 0.001 | _silence_ 0.91 | place 0.01 | pick 0.01 | ch0 |
| raw/supanut/S0/base/base_01_raw.wav | base | miss |  | 0.002 | _silence_ 0.96 | left 0.00 | pick 0.00 | ch0 |
| raw/supanut/S0/green_box/green_box_02_raw.wav | green box | miss |  | 0.001 | _silence_ 0.96 | left 0.01 | place 0.00 | ch0 |
| raw/supanut/S0/no/no_01_raw.wav | no | miss |  | 0.001 | _silence_ 0.95 | pick 0.01 | left 0.01 | ch0 |
| raw/supanut/S0/blue_box/blue_box_02_raw.wav | blue box | miss |  | 0.001 | _silence_ 0.96 | left 0.01 | pick 0.01 | ch0 |
| raw/supanut/S0/place/place_02_raw.wav | place | miss |  | 0.003 | _silence_ 0.97 | left 0.01 | pick 0.00 | ch0 |
| raw/supanut/S0/hand/hand_02_raw.wav | hand | miss |  | 0.0 | _silence_ 0.98 | left 0.00 | pick 0.00 | ch0 |
| raw/supanut/S0/right/right_01_raw.wav | right | miss |  | 0.002 | _silence_ 0.95 | left 0.01 | place 0.00 | ch0 |
| raw/supanut/S0/grab/grab_02_raw.wav | grab | miss |  | 0.003 | _silence_ 0.97 | left 0.00 | place 0.00 | ch0 |
| raw/supanut/S0/base/base_02_raw.wav | base | miss |  | 0.004 | _silence_ 0.92 | pick 0.01 | left 0.01 | ch0 |
| raw/supanut/S0/green_box/green_box_03_raw.wav | green box | miss |  | 0.001 | _silence_ 0.96 | left 0.01 | pick 0.00 | ch0 |
| raw/supanut/S0/backward/backward_03_raw.wav | backward | miss |  | 0.002 | _silence_ 0.96 | left 0.01 | drop 0.01 | ch0 |
| raw/supanut/S0/backward/backward_04_raw.wav | backward | miss |  | 0.004 | _silence_ 0.95 | _unknown_ 0.01 | pick 0.01 | ch0 |
| raw/supanut/S0/right/right_02_raw.wav | right | miss |  | 0.001 | _silence_ 0.97 | left 0.00 | place 0.00 | ch0 |
| raw/supanut/S0/black_box/black_box_03_raw.wav | black box | miss |  | 0.001 | _silence_ 0.97 | left 0.00 | pick 0.00 | ch0 |
| raw/supanut/S0/green_box/green_box_04_raw.wav | green box | miss |  | 0.0 | _silence_ 0.98 | left 0.00 | pick 0.00 | ch0 |
| raw/supanut/S0/base/base_03_raw.wav | base | miss |  | 0.002 | _silence_ 0.95 | pick 0.01 | left 0.01 | ch0 |
| raw/supanut/S0/green_box/green_box_05_raw.wav | green box | miss |  | 0.0 | _silence_ 0.99 | pick 0.00 | _unknown_ 0.00 | ch0 |
| raw/supanut/S0/red_box/red_box_02_raw.wav | red box | miss |  | 0.0 | _silence_ 0.98 | left 0.00 | place 0.00 | ch0 |
| raw/supanut/S0/stop/stop_02_raw.wav | stop | miss |  | 0.002 | _silence_ 0.95 | pick 0.01 | drop 0.01 | ch0 |
| raw/supanut/S0/backward/backward_05_raw.wav | backward | miss |  | 0.003 | _silence_ 0.93 | left 0.01 | pick 0.01 | ch0 |
| raw/supanut/S0/blue_box/blue_box_03_raw.wav | blue box | miss |  | 0.001 | _silence_ 0.98 | left 0.00 | place 0.00 | ch0 |
| raw/supanut/S0/home/home_03_raw.wav | home | miss |  | 0.001 | _silence_ 0.98 | left 0.00 | place 0.00 | ch0 |
| raw/supanut/S0/green_box/green_box_06_raw.wav | green box | miss |  | 0.001 | _silence_ 0.96 | pick 0.01 | place 0.00 | ch0 |
| raw/supanut/S0/grab/grab_03_raw.wav | grab | miss |  | 0.003 | _silence_ 0.93 | _unknown_ 0.01 | pick 0.01 | ch0 |
| raw/supanut/S0/black_box/black_box_04_raw.wav | black box | miss |  | 0.001 | _silence_ 0.97 | left 0.00 | place 0.00 | ch0 |
| raw/supanut/S0/left/left_01_raw.wav | left | miss |  | 0.005 | _silence_ 0.97 | left 0.01 | place 0.00 | ch0 |
| raw/supanut/S0/move/move_03_raw.wav | move | miss |  | 0.001 | _silence_ 0.96 | left 0.01 | backward 0.00 | ch0 |
| raw/supanut/S0/no/no_02_raw.wav | no | miss |  | 0.004 | _silence_ 0.85 | left 0.02 | backward 0.02 | ch0 |
| raw/supanut/S0/blue_box/blue_box_04_raw.wav | blue box | miss |  | 0.007 | _silence_ 0.80 | station 0.02 | left 0.02 | ch0 |
| raw/supanut/S0/stop/stop_03_raw.wav | stop | miss |  | 0.008 | _silence_ 0.62 | backward 0.05 | left 0.03 | ch0 |
| raw/supanut/S0/place/place_04_raw.wav | place | miss |  | 0.005 | _silence_ 0.92 | left 0.01 | backward 0.01 | ch0 |
| raw/supanut/S0/grab/grab_06_raw.wav | grab | miss |  | 0.004 | _silence_ 0.91 | left 0.01 | pick 0.01 | ch0 |
| raw/supanut/S0/forward/forward_01_raw.wav | forward | miss |  | 0.003 | _silence_ 0.89 | place 0.02 | backward 0.02 | ch0 |
| raw/supanut/S0/station/station_03_raw.wav | station | miss |  | 0.008 | _silence_ 0.60 | left 0.09 | _unknown_ 0.05 | ch0 |
| raw/supanut/S0/backward/backward_06_raw.wav | backward | miss |  | 0.004 | _silence_ 0.95 | left 0.01 | _unknown_ 0.00 | ch0 |
| raw/supanut/S0/place/place_06_raw.wav | place | miss |  | 0.02 | _silence_ 0.74 | left 0.05 | right 0.04 | ch0 |
| raw/supanut/S0/hand/hand_03_raw.wav | hand | miss |  | 0.006 | _silence_ 0.65 | pick 0.07 | left 0.04 | ch0 |
| raw/supanut/S0/hand/hand_04_raw.wav | hand | miss |  | 0.001 | _silence_ 0.85 | left 0.03 | backward 0.02 | ch0 |
| raw/supanut/S0/station/station_04_raw.wav | station | miss |  | 0.013 | _silence_ 0.80 | left 0.03 | move 0.02 | ch0 |
| raw/supanut/S0/forward/forward_02_raw.wav | forward | miss |  | 0.001 | _silence_ 0.89 | left 0.02 | right 0.01 | ch0 |
| raw/supanut/S0/move/move_04_raw.wav | move | miss |  | 0.002 | _silence_ 0.92 | left 0.01 | place 0.01 | ch0 |
| raw/supanut/S0/station/station_05_raw.wav | station | miss |  | 0.001 | _silence_ 0.97 | left 0.00 | pick 0.00 | ch0 |
| raw/supanut/S0/red_box/red_box_03_raw.wav | red box | miss |  | 0.488 | _unknown_ 0.69 | right 0.08 | grab 0.04 | ch1 |
| raw/supanut/S0/backward/backward_07_raw.wav | backward | miss |  | 0.551 | backward 0.55 | left 0.16 | _unknown_ 0.09 | ch0/ch1 |
| raw/supanut/S0/home/home_04_raw.wav | home | miss |  | 0.001 | _silence_ 0.97 | left 0.00 | backward 0.00 | ch0 |
| raw/supanut/S0/right/right_03_raw.wav | right | miss |  | 0.014 | _silence_ 0.64 | backward 0.06 | _unknown_ 0.03 | ch0 |
| raw/supanut/S0/_unknown_/bright_01_raw.wav | _unknown_ | false_alarm | right@1.31s 0.94 | 0.041 | right 0.95 | _unknown_ 0.04 | left 0.00 | ch0/ch1 |
| raw/supanut/S0/left/left_03_raw.wav | left | miss |  | 0.004 | _silence_ 0.97 | left 0.00 | _unknown_ 0.00 | ch0 |
| raw/supanut/S0/_unknown_/open_01_raw.wav | _unknown_ | false_alarm | home@1.01s 0.92 | 0.63 | home 0.92 | _unknown_ 0.05 | forward 0.01 | ch0/ch1 |
| raw/supanut/S0/stop/stop_04_raw.wav | stop | miss |  | 0.003 | _silence_ 0.96 | left 0.01 | _unknown_ 0.00 | ch0 |
| raw/supanut/S0/base/base_04_raw.wav | base | miss |  | 0.001 | _silence_ 0.98 | left 0.00 | backward 0.00 | ch0 |
| raw/supanut/S0/left/left_05_raw.wav | left | miss |  | 0.028 | _silence_ 0.83 | left 0.03 | station 0.02 | ch0 |
| raw/supanut/S0/place/place_07_raw.wav | place | miss |  | 0.016 | _silence_ 0.73 | left 0.04 | right 0.03 | ch0 |
| raw/supanut/S0/no/no_03_raw.wav | no | miss |  | 0.014 | _silence_ 0.73 | left 0.08 | pick 0.02 | ch0 |
| raw/supanut/S0/_unknown_/blue_01_raw.wav | _unknown_ | false_alarm | no@1.21s 0.72 | 0.131 | no 0.90 | move 0.08 | _unknown_ 0.01 | ch1 |
| raw/supanut/S0/move/move_05_raw.wav | move | miss |  | 0.645 | move 0.64 | no 0.20 | home 0.04 | ch1 |
| raw/supanut/S0/_unknown_/groove_01_raw.wav | _unknown_ | false_alarm | move@1.01s 0.84 | 0.065 | move 0.91 | no 0.03 | _unknown_ 0.03 | ch1 |
