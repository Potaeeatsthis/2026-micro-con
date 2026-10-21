# Board evaluation - baseline_w16d2_noaug on S0

model `/sessions/rcw-ac6fce39038343a9b68f42328258/mnt/prefer/onnx_out/kws.onnx` (onnx (input logmel [1, 40, 101])) · gain +15.0 dB · features numpy · 2026-10-18 11:07

## Clip mode (1 s clips, mono ch1)

n 109 · **accuracy 0.4495** · **macro-F1 0.411** · commands 0.5106 · _unknown_ 0.0667 · _silence_ None · commands rejected as unknown/silence 10 · speech peak median -32.4 dB (training clip -25.4 dB)

| cls | support | correct | recall | precision | f1 | mean_p_true | confused_with |
|---|---|---|---|---|---|---|---|
| move | 5 | 0 | 0.0 | 0.0 | 0.0 | 0.007 | no 5 |
| pick | 2 | 2 | 1.0 | 1.0 | 1.0 | 0.471 |  |
| grab | 7 | 0 | 0.0 | 0.0 | 0.0 | 0.002 | right 6, left 1 |
| blue box | 7 | 0 | 0.0 | 0.0 | 0.0 | 0.023 | no 7 |
| green box | 6 | 1 | 0.167 | 1.0 | 0.286 | 0.215 | _unknown_ 4, stop 1 |
| red box | 3 | 0 | 0.0 | 0.0 | 0.0 | 0.19 | backward 1, right 1, _unknown_ 1 |
| black box | 4 | 0 | 0.0 | 0.0 | 0.0 | 0.006 | backward 3, no 1 |
| hand | 5 | 4 | 0.8 | 1.0 | 0.889 | 0.645 | _unknown_ 1 |
| station | 6 | 6 | 1.0 | 1.0 | 1.0 | 0.85 |  |
| base | 5 | 2 | 0.4 | 1.0 | 0.571 | 0.293 | _unknown_ 3 |
| forward | 4 | 4 | 1.0 | 0.333 | 0.5 | 0.972 |  |
| backward | 7 | 7 | 1.0 | 0.538 | 0.7 | 1.0 |  |
| left | 5 | 5 | 1.0 | 0.833 | 0.909 | 0.739 |  |
| right | 3 | 3 | 1.0 | 0.214 | 0.353 | 0.997 |  |
| place | 7 | 3 | 0.429 | 1.0 | 0.6 | 0.286 | yes 3, right 1 |
| drop | 2 | 0 | 0.0 | 0.0 | 0.0 | 0.127 | stop 2 |
| yes | 2 | 1 | 0.5 | 0.25 | 0.333 | 0.565 | _silence_ 1 |
| no | 4 | 4 | 1.0 | 0.182 | 0.308 | 0.986 |  |
| home | 5 | 1 | 0.2 | 1.0 | 0.333 | 0.216 | forward 4 |
| stop | 5 | 5 | 1.0 | 0.625 | 0.769 | 0.995 |  |
| _unknown_ | 15 | 1 | 0.067 | 0.1 | 0.08 | 0.149 | no 5, forward 4, right 3 |
| _silence_ | 0 | 0 | nan | 0.0 | 0.0 | nan |  |

### Mistakes (top-3)

| clip_path | label | pred | top3 | snr_db | flags |
|---|---|---|---|---|---|
| clips/real/supanut/S0/move/move_01.wav | move | no | no 0.79 | backward 0.09 | left 0.04 | 25.49 | S0 old wiring (VDD ~1.8 V) |
| clips/real/supanut/S0/black_box/black_box_01.wav | black box | backward | backward 0.96 | no 0.03 | _unknown_ 0.01 | 24.02 | S0 old wiring (VDD ~1.8 V) |
| clips/real/supanut/S0/grab/grab_01.wav | grab | right | right 0.80 | left 0.16 | _unknown_ 0.02 | 27.57 | S0 old wiring (VDD ~1.8 V) |
| clips/real/supanut/S0/home/home_01.wav | home | forward | forward 0.76 | _unknown_ 0.10 | no 0.04 | 20.59 | sound longer than 1 s - the clip cuts it; S0 old wiring (VDD ~1.8 V) |
| clips/real/supanut/S0/blue_box/blue_box_01.wav | blue box | no | no 0.85 | _unknown_ 0.08 | move 0.03 | 24.12 | S0 old wiring (VDD ~1.8 V) |
| clips/real/supanut/S0/home/home_02.wav | home | forward | forward 0.67 | _unknown_ 0.16 | home 0.13 | 22.69 | S0 old wiring (VDD ~1.8 V) |
| clips/real/supanut/S0/red_box/red_box_01.wav | red box | right | right 0.53 | red box 0.42 | _unknown_ 0.03 | 23.21 | sound longer than 1 s - the clip cuts it; S0 old wiring (VDD ~1.8 V) |
| clips/real/supanut/S0/black_box/black_box_02.wav | black box | backward | backward 0.85 | no 0.06 | _unknown_ 0.04 | 22.08 | S0 old wiring (VDD ~1.8 V) |
| clips/real/supanut/S0/hand/hand_01.wav | hand | _unknown_ | _unknown_ 0.33 | right 0.31 | hand 0.23 | 19.99 | S0 old wiring (VDD ~1.8 V) |
| clips/real/supanut/S0/move/move_02.wav | move | no | no 0.67 | backward 0.19 | _unknown_ 0.06 | 22.06 | S0 old wiring (VDD ~1.8 V) |
| clips/real/supanut/S0/green_box/green_box_01.wav | green box | _unknown_ | _unknown_ 0.80 | green box 0.19 | right 0.01 | 24.95 | S0 old wiring (VDD ~1.8 V) |
| clips/real/supanut/S0/base/base_01.wav | base | _unknown_ | _unknown_ 0.33 | place 0.32 | yes 0.18 | 21.05 | S0 old wiring (VDD ~1.8 V) |
| clips/real/supanut/S0/green_box/green_box_02.wav | green box | stop | stop 0.35 | _unknown_ 0.20 | yes 0.18 | 23.11 | word touches the edge of the 2 s take; sound longer than 1 s - the clip cuts it; S0 old wiring (VDD ~1.8 V) |
| clips/real/supanut/S0/blue_box/blue_box_02.wav | blue box | no | no 0.53 | move 0.30 | _unknown_ 0.08 | 20.71 | word touches the edge of the 2 s take; sound longer than 1 s - the clip cuts it; S0 old wiring (VDD ~1.8 V) |
| clips/real/supanut/S0/_unknown_/nose_01.wav | _unknown_ | no | no 0.80 | backward 0.13 | _unknown_ 0.06 | 24.64 | S0 old wiring (VDD ~1.8 V) |
| clips/real/supanut/S0/place/place_02.wav | place | yes | yes 0.30 | place 0.24 | _unknown_ 0.19 | 16.09 | sound longer than 1 s - the clip cuts it; S0 old wiring (VDD ~1.8 V) |
| clips/real/supanut/S0/grab/grab_02.wav | grab | right | right 0.85 | left 0.10 | backward 0.02 | 27.23 | S0 old wiring (VDD ~1.8 V) |
| clips/real/supanut/S0/green_box/green_box_03.wav | green box | _unknown_ | _unknown_ 0.78 | green box 0.13 | right 0.07 | 22.01 | sound longer than 1 s - the clip cuts it; S0 old wiring (VDD ~1.8 V) |
| clips/real/supanut/S0/_unknown_/write_01.wav | _unknown_ | right | right 1.00 | left 0.00 | _unknown_ 0.00 | 25.15 | S0 old wiring (VDD ~1.8 V) |
| clips/real/supanut/S0/black_box/black_box_03.wav | black box | no | no 0.82 | backward 0.10 | yes 0.04 | 23.37 | S0 old wiring (VDD ~1.8 V) |
| clips/real/supanut/S0/_unknown_/mood_01.wav | _unknown_ | no | no 0.64 | backward 0.28 | _unknown_ 0.04 | 22.31 | S0 old wiring (VDD ~1.8 V) |
| clips/real/supanut/S0/green_box/green_box_05.wav | green box | _unknown_ | _unknown_ 0.56 | green box 0.38 | right 0.04 | 25.31 | S0 old wiring (VDD ~1.8 V) |
| clips/real/supanut/S0/red_box/red_box_02.wav | red box | backward | backward 0.68 | red box 0.14 | _unknown_ 0.09 | 23.01 | S0 old wiring (VDD ~1.8 V) |
| clips/real/supanut/S0/blue_box/blue_box_03.wav | blue box | no | no 0.88 | _unknown_ 0.06 | yes 0.02 | 23.84 | S0 old wiring (VDD ~1.8 V) |
| clips/real/supanut/S0/green_box/green_box_06.wav | green box | _unknown_ | _unknown_ 0.76 | right 0.17 | green box 0.06 | 24.68 | S0 old wiring (VDD ~1.8 V) |
| clips/real/supanut/S0/grab/grab_03.wav | grab | right | right 0.82 | _unknown_ 0.07 | left 0.06 | 27.98 | S0 old wiring (VDD ~1.8 V) |
| clips/real/supanut/S0/black_box/black_box_04.wav | black box | backward | backward 0.96 | no 0.02 | left 0.01 | 25.66 | other sound outside the clip (2 words? click?); S0 old wiring (VDD ~1.8 V) |
| clips/real/supanut/S0/grab/grab_04.wav | grab | right | right 0.81 | left 0.10 | _unknown_ 0.05 | 28.08 | S0 old wiring (VDD ~1.8 V) |
| clips/real/supanut/S0/grab/grab_05.wav | grab | left | left 0.44 | right 0.30 | backward 0.08 | 28.1 | S0 old wiring (VDD ~1.8 V) |
| clips/real/supanut/S0/drop/drop_01.wav | drop | stop | stop 0.50 | drop 0.17 | _unknown_ 0.13 | 22.54 | S0 old wiring (VDD ~1.8 V) |
| clips/real/supanut/S0/move/move_03.wav | move | no | no 0.61 | backward 0.20 | _unknown_ 0.06 | 22.9 | S0 old wiring (VDD ~1.8 V) |
| clips/real/supanut/S0/blue_box/blue_box_04.wav | blue box | no | no 0.84 | _unknown_ 0.07 | blue box 0.05 | 26.78 | S0 old wiring (VDD ~1.8 V) |
| clips/real/supanut/S0/grab/grab_06.wav | grab | right | right 0.42 | left 0.35 | backward 0.16 | 23.23 | S0 old wiring (VDD ~1.8 V) |
| clips/real/supanut/S0/place/place_05.wav | place | right | right 0.58 | _unknown_ 0.17 | place 0.11 | 23.09 | S0 old wiring (VDD ~1.8 V) |
| clips/real/supanut/S0/_unknown_/phone_01.wav | _unknown_ | forward | forward 0.92 | _unknown_ 0.06 | stop 0.01 | 24.42 | S0 old wiring (VDD ~1.8 V) |
| clips/real/supanut/S0/place/place_06.wav | place | yes | yes 0.65 | left 0.12 | _unknown_ 0.11 | 18.83 | S0 old wiring (VDD ~1.8 V) |
| clips/real/supanut/S0/_unknown_/moon_01.wav | _unknown_ | no | no 0.30 | backward 0.23 | forward 0.14 | 23.7 | S0 old wiring (VDD ~1.8 V) |
| clips/real/supanut/S0/_unknown_/go_01.wav | _unknown_ | forward | forward 0.69 | _unknown_ 0.13 | stop 0.12 | 27.44 | S0 old wiring (VDD ~1.8 V) |
| clips/real/supanut/S0/move/move_04.wav | move | no | no 0.74 | backward 0.14 | _unknown_ 0.06 | 26.44 | S0 old wiring (VDD ~1.8 V) |
| clips/real/supanut/S0/_unknown_/foam_01.wav | _unknown_ | forward | forward 0.92 | _unknown_ 0.05 | home 0.01 | 26.15 | S0 old wiring (VDD ~1.8 V) |
| clips/real/supanut/S0/red_box/red_box_03.wav | red box | _unknown_ | _unknown_ 0.53 | backward 0.44 | right 0.02 | 23.48 | S0 old wiring (VDD ~1.8 V) |
| clips/real/supanut/S0/home/home_04.wav | home | forward | forward 0.56 | _unknown_ 0.18 | home 0.17 | 18.94 | sound longer than 1 s - the clip cuts it; S0 old wiring (VDD ~1.8 V) |
| clips/real/supanut/S0/home/home_05.wav | home | forward | forward 0.61 | home 0.19 | _unknown_ 0.15 | 20.96 | S0 old wiring (VDD ~1.8 V) |
| clips/real/supanut/S0/_unknown_/bright_01.wav | _unknown_ | right | right 0.99 | left 0.00 | _unknown_ 0.00 | 24.09 | S0 old wiring (VDD ~1.8 V) |
| clips/real/supanut/S0/_unknown_/open_01.wav | _unknown_ | forward | forward 0.64 | _unknown_ 0.27 | stop 0.06 | 24.52 | S0 old wiring (VDD ~1.8 V) |
| clips/real/supanut/S0/yes/yes_02.wav | yes | _silence_ | _silence_ 0.53 | yes 0.14 | left 0.12 | 16.0 | sound longer than 1 s - the clip cuts it; S0 old wiring (VDD ~1.8 V) |
| clips/real/supanut/S0/blue_box/blue_box_05.wav | blue box | no | no 0.86 | _unknown_ 0.05 | yes 0.05 | 22.38 | S0 old wiring (VDD ~1.8 V) |
| clips/real/supanut/S0/_unknown_/robot_01.wav | _unknown_ | backward | backward 0.42 | _unknown_ 0.33 | no 0.09 | 22.74 | S0 old wiring (VDD ~1.8 V) |
| clips/real/supanut/S0/drop/drop_02.wav | drop | stop | stop 0.64 | no 0.20 | drop 0.09 | 24.34 | S0 old wiring (VDD ~1.8 V) |
| clips/real/supanut/S0/_unknown_/wait_01.wav | _unknown_ | right | right 0.29 | backward 0.27 | _unknown_ 0.27 | 19.73 | S0 old wiring (VDD ~1.8 V) |
| clips/real/supanut/S0/blue_box/blue_box_06.wav | blue box | no | no 0.84 | yes 0.07 | _unknown_ 0.05 | 22.3 | S0 old wiring (VDD ~1.8 V) |
| clips/real/supanut/S0/base/base_04.wav | base | _unknown_ | _unknown_ 0.48 | right 0.26 | backward 0.10 | 17.58 | S0 old wiring (VDD ~1.8 V) |
| clips/real/supanut/S0/blue_box/blue_box_07.wav | blue box | no | no 0.81 | _unknown_ 0.08 | yes 0.03 | 25.13 | S0 old wiring (VDD ~1.8 V) |
| clips/real/supanut/S0/place/place_07.wav | place | yes | yes 0.37 | place 0.29 | _unknown_ 0.20 | 19.77 | S0 old wiring (VDD ~1.8 V) |
| clips/real/supanut/S0/grab/grab_07.wav | grab | right | right 0.69 | backward 0.21 | left 0.04 | 20.98 | sound longer than 1 s - the clip cuts it; S0 old wiring (VDD ~1.8 V) |
| clips/real/supanut/S0/_unknown_/blue_01.wav | _unknown_ | no | no 0.98 | _unknown_ 0.01 | move 0.01 | 23.87 | S0 old wiring (VDD ~1.8 V) |
| clips/real/supanut/S0/move/move_05.wav | move | no | no 0.76 | backward 0.14 | _unknown_ 0.06 | 25.43 | S0 old wiring (VDD ~1.8 V) |
| clips/real/supanut/S0/base/base_05.wav | base | _unknown_ | _unknown_ 0.56 | backward 0.13 | right 0.11 | 16.74 | S0 old wiring (VDD ~1.8 V) |
| clips/real/supanut/S0/_unknown_/bag_01.wav | _unknown_ | backward | backward 0.76 | left 0.19 | _unknown_ 0.03 | 17.09 | S0 old wiring (VDD ~1.8 V) |
| clips/real/supanut/S0/_unknown_/groove_01.wav | _unknown_ | no | no 0.44 | forward 0.26 | _unknown_ 0.21 | 26.41 | S0 old wiring (VDD ~1.8 V) |

## Stream mode (raw 2 s takes, board logic: stride 10 frames, avg 3, threshold 0.7, refractory 1500 ms, channel auto)

command takes 94: **detected 0.1277** (hit 12, hit+extra 0), wrong 37, miss 45 · first detection correct 0.1277 · _unknown_ false alarms 11/15 · noise false alarms 0 in 0.0 min · channels {'ch1': 27, 'ch0/ch1': 16, 'ch0': 66}

| cls | takes | hit | hit_extra | wrong | miss | false_alarm | detected_rate | wrong_as | mean_max_p_true |
|---|---|---|---|---|---|---|---|---|---|
| move | 5 | 0 | 0 | 2 | 3 | 0 | 0.0 | no 2 | 0.003 |
| pick | 2 | 0 | 0 | 0 | 2 | 0 | 0.0 |  | 0.181 |
| grab | 7 | 0 | 0 | 5 | 2 | 0 | 0.0 | right 3, left 2 | 0.003 |
| blue box | 7 | 0 | 0 | 4 | 3 | 0 | 0.0 | no 3, left 1 | 0.004 |
| green box | 6 | 0 | 0 | 4 | 2 | 0 | 0.0 | left 4 | 0.0 |
| red box | 3 | 0 | 0 | 1 | 2 | 0 | 0.0 | right 1 | 0.17 |
| black box | 4 | 0 | 0 | 2 | 2 | 0 | 0.0 | backward 1, left 1 | 0.004 |
| hand | 5 | 1 | 0 | 4 | 0 | 0 | 0.2 | left 4 | 0.182 |
| station | 6 | 2 | 0 | 3 | 1 | 0 | 0.333 | left 3 | 0.279 |
| base | 5 | 0 | 0 | 0 | 5 | 0 | 0.0 |  | 0.057 |
| forward | 4 | 2 | 0 | 1 | 1 | 0 | 0.5 | left 1 | 0.483 |
| backward | 7 | 1 | 0 | 3 | 3 | 0 | 0.143 | left 3 | 0.265 |
| left | 5 | 1 | 0 | 0 | 4 | 0 | 0.2 |  | 0.565 |
| right | 3 | 0 | 0 | 2 | 1 | 0 | 0.0 | left 2 | 0.001 |
| place | 7 | 0 | 0 | 3 | 4 | 0 | 0.0 | left 3 | 0.134 |
| drop | 2 | 0 | 0 | 0 | 2 | 0 | 0.0 |  | 0.287 |
| yes | 2 | 2 | 0 | 0 | 0 | 0 | 1.0 |  | 0.994 |
| no | 4 | 1 | 0 | 2 | 1 | 0 | 0.25 | left 2 | 0.253 |
| home | 5 | 0 | 0 | 1 | 4 | 0 | 0.0 | forward 1 | 0.041 |
| stop | 5 | 2 | 0 | 0 | 3 | 0 | 0.4 |  | 0.515 |
| _unknown_ | 15 | 0 | 0 | 0 | 0 | 11 |  | left 6, forward 2, right 1 | 0.202 |

### Takes that were not a clean hit

| raw_path | label | outcome | detections | max_avg_p_true | peak_top3 | channels_used |
|---|---|---|---|---|---|---|
| raw/real/supanut/S0/move/move_01_raw.wav | move | wrong | no@1.41s 0.77 | 0.013 | no 0.79 | backward 0.11 | left 0.04 | ch1 |
| raw/real/supanut/S0/black_box/black_box_01_raw.wav | black box | wrong | backward@1.01s 0.72 | 0.001 | backward 0.95 | no 0.04 | left 0.01 | ch1 |
| raw/real/supanut/S0/grab/grab_01_raw.wav | grab | wrong | right@1.41s 0.76 | 0.0 | right 0.77 | left 0.19 | _unknown_ 0.02 | ch1 |
| raw/real/supanut/S0/home/home_01_raw.wav | home | wrong | forward@1.31s 0.72 | 0.035 | forward 0.74 | _unknown_ 0.11 | no 0.06 | ch0/ch1 |
| raw/real/supanut/S0/blue_box/blue_box_01_raw.wav | blue box | miss |  | 0.0 | _silence_ 1.00 | yes 0.00 | left 0.00 | ch0 |
| raw/real/supanut/S0/home/home_02_raw.wav | home | miss |  | 0.0 | _silence_ 1.00 | left 0.00 | yes 0.00 | ch0 |
| raw/real/supanut/S0/red_box/red_box_01_raw.wav | red box | wrong | right@1.01s 0.95 | 0.499 | right 0.97 | backward 0.02 | _unknown_ 0.01 | ch1 |
| raw/real/supanut/S0/place/place_01_raw.wav | place | miss |  | 0.363 | left 0.45 | right 0.15 | yes 0.15 | ch1 |
| raw/real/supanut/S0/station/station_01_raw.wav | station | wrong | left@1.01s 0.88 | 0.0 | left 0.88 | backward 0.07 | yes 0.03 | ch0 |
| raw/real/supanut/S0/pick/pick_01_raw.wav | pick | miss |  | 0.0 | yes 0.31 | left 0.27 | _silence_ 0.14 | ch0 |
| raw/real/supanut/S0/black_box/black_box_02_raw.wav | black box | miss |  | 0.0 | _silence_ 0.60 | left 0.24 | yes 0.08 | ch0 |
| raw/real/supanut/S0/hand/hand_01_raw.wav | hand | wrong | left@1.01s 0.91 | 0.0 | left 0.91 | _unknown_ 0.07 | yes 0.02 | ch0 |
| raw/real/supanut/S0/_unknown_/pink_01_raw.wav | _unknown_ | false_alarm | left@1.01s 0.83 | 0.589 | left 0.83 | _unknown_ 0.13 | yes 0.02 | ch0 |
| raw/real/supanut/S0/move/move_02_raw.wav | move | miss |  | 0.0 | left 0.57 | _unknown_ 0.19 | yes 0.17 | ch0 |
| raw/real/supanut/S0/backward/backward_02_raw.wav | backward | wrong | left@1.71s 0.72 | 0.146 | left 0.72 | yes 0.08 | _silence_ 0.06 | ch0 |
| raw/real/supanut/S0/green_box/green_box_01_raw.wav | green box | wrong | left@1.91s 0.72 | 0.0 | left 0.72 | _unknown_ 0.10 | yes 0.08 | ch0 |
| raw/real/supanut/S0/base/base_01_raw.wav | base | miss |  | 0.0 | left 0.60 | _unknown_ 0.21 | yes 0.11 | ch0 |
| raw/real/supanut/S0/green_box/green_box_02_raw.wav | green box | miss |  | 0.0 | stop 0.56 | _unknown_ 0.30 | left 0.07 | ch0 |
| raw/real/supanut/S0/no/no_01_raw.wav | no | wrong | left@1.01s 0.70 | 0.015 | left 0.70 | yes 0.14 | _unknown_ 0.08 | ch0 |
| raw/real/supanut/S0/blue_box/blue_box_02_raw.wav | blue box | miss |  | 0.0 | _unknown_ 0.82 | stop 0.08 | yes 0.06 | ch0 |
| raw/real/supanut/S0/_unknown_/nose_01_raw.wav | _unknown_ | false_alarm | left@1.51s 0.81 | 0.327 | left 0.90 | _unknown_ 0.04 | yes 0.04 | ch0 |
| raw/real/supanut/S0/place/place_02_raw.wav | place | miss |  | 0.0 | stop 0.65 | _unknown_ 0.30 | pick 0.03 | ch0 |
| raw/real/supanut/S0/hand/hand_02_raw.wav | hand | wrong | left@1.01s 0.94 | 0.0 | left 0.94 | _unknown_ 0.04 | yes 0.01 | ch0 |
| raw/real/supanut/S0/right/right_01_raw.wav | right | miss |  | 0.001 | _unknown_ 0.68 | left 0.13 | stop 0.12 | ch0 |
| raw/real/supanut/S0/grab/grab_02_raw.wav | grab | wrong | left@1.01s 0.99 | 0.0 | left 0.99 | _unknown_ 0.01 | yes 0.01 | ch0 |
| raw/real/supanut/S0/base/base_02_raw.wav | base | miss |  | 0.0 | left 0.70 | _unknown_ 0.12 | yes 0.06 | ch0 |
| raw/real/supanut/S0/green_box/green_box_03_raw.wav | green box | wrong | left@1.01s 0.95 | 0.0 | left 0.95 | yes 0.02 | _unknown_ 0.02 | ch0 |
| raw/real/supanut/S0/backward/backward_03_raw.wav | backward | wrong | left@1.01s 0.85 | 0.125 | left 0.85 | backward 0.08 | _unknown_ 0.04 | ch0 |
| raw/real/supanut/S0/_unknown_/write_01_raw.wav | _unknown_ | false_alarm | left@1.91s 0.84 | 0.499 | left 0.84 | _unknown_ 0.09 | yes 0.05 | ch0 |
| raw/real/supanut/S0/backward/backward_04_raw.wav | backward | miss |  | 0.02 | _unknown_ 0.66 | stop 0.22 | left 0.10 | ch0 |
| raw/real/supanut/S0/right/right_02_raw.wav | right | wrong | left@1.01s 0.87 | 0.001 | left 0.90 | _unknown_ 0.06 | yes 0.04 | ch0 |
| raw/real/supanut/S0/black_box/black_box_03_raw.wav | black box | wrong | left@1.01s 0.95 | 0.012 | left 0.95 | _unknown_ 0.02 | yes 0.01 | ch0 |
| raw/real/supanut/S0/green_box/green_box_04_raw.wav | green box | wrong | left@1.01s 0.85 | 0.0 | left 0.88 | yes 0.06 | backward 0.03 | ch0 |
| raw/real/supanut/S0/base/base_03_raw.wav | base | miss |  | 0.0 | left 0.54 | yes 0.26 | _unknown_ 0.13 | ch0 |
| raw/real/supanut/S0/green_box/green_box_05_raw.wav | green box | wrong | left@1.01s 0.85 | 0.0 | left 0.88 | _unknown_ 0.05 | yes 0.04 | ch0 |
| raw/real/supanut/S0/red_box/red_box_02_raw.wav | red box | miss |  | 0.0 | left 0.69 | backward 0.11 | yes 0.08 | ch0 |
| raw/real/supanut/S0/stop/stop_02_raw.wav | stop | miss |  | 0.514 | stop 0.51 | _unknown_ 0.36 | left 0.05 | ch0 |
| raw/real/supanut/S0/backward/backward_05_raw.wav | backward | miss |  | 0.005 | _silence_ 0.75 | left 0.21 | yes 0.03 | ch0 |
| raw/real/supanut/S0/blue_box/blue_box_03_raw.wav | blue box | miss |  | 0.0 | left 0.60 | yes 0.22 | backward 0.07 | ch0 |
| raw/real/supanut/S0/home/home_03_raw.wav | home | miss |  | 0.0 | left 0.47 | _silence_ 0.31 | backward 0.07 | ch0 |
| raw/real/supanut/S0/green_box/green_box_06_raw.wav | green box | miss |  | 0.0 | stop 0.52 | _unknown_ 0.37 | yes 0.05 | ch0 |
| raw/real/supanut/S0/grab/grab_03_raw.wav | grab | miss |  | 0.0 | _unknown_ 0.56 | stop 0.24 | left 0.13 | ch0 |
| raw/real/supanut/S0/black_box/black_box_04_raw.wav | black box | miss |  | 0.001 | left 0.45 | backward 0.21 | yes 0.20 | ch0 |
| raw/real/supanut/S0/left/left_01_raw.wav | left | miss |  | 0.696 | left 0.70 | _unknown_ 0.18 | yes 0.05 | ch0 |
| raw/real/supanut/S0/grab/grab_04_raw.wav | grab | wrong | right@1.01s 0.82 | 0.001 | right 0.82 | left 0.10 | backward 0.04 | ch1 |
| raw/real/supanut/S0/grab/grab_05_raw.wav | grab | miss |  | 0.005 | left 0.53 | right 0.24 | backward 0.15 | ch1 |
| raw/real/supanut/S0/drop/drop_01_raw.wav | drop | miss |  | 0.431 | drop 0.43 | stop 0.24 | _unknown_ 0.16 | ch0/ch1 |
| raw/real/supanut/S0/place/place_03_raw.wav | place | miss |  | 0.465 | place 0.46 | yes 0.30 | left 0.10 | ch1 |
| raw/real/supanut/S0/move/move_03_raw.wav | move | miss |  | 0.0 | _silence_ 1.00 | yes 0.00 | left 0.00 | ch0 |
| raw/real/supanut/S0/no/no_02_raw.wav | no | miss |  | 0.013 | stop 0.32 | _unknown_ 0.28 | yes 0.18 | ch0 |
| raw/real/supanut/S0/blue_box/blue_box_04_raw.wav | blue box | wrong | left@1.51s 0.73 | 0.0 | left 1.00 | _unknown_ 0.00 | yes 0.00 | ch0 |
| raw/real/supanut/S0/stop/stop_03_raw.wav | stop | miss |  | 0.066 | left 0.56 | yes 0.16 | _unknown_ 0.14 | ch0 |
| raw/real/supanut/S0/place/place_04_raw.wav | place | wrong | left@1.31s 0.77 | 0.0 | left 0.83 | yes 0.09 | _unknown_ 0.06 | ch0 |
| raw/real/supanut/S0/grab/grab_06_raw.wav | grab | wrong | left@1.41s 0.83 | 0.0 | left 0.83 | _unknown_ 0.09 | yes 0.07 | ch0 |
| raw/real/supanut/S0/forward/forward_01_raw.wav | forward | miss |  | 0.0 | left 0.69 | _unknown_ 0.31 | stop 0.00 | ch0 |
| raw/real/supanut/S0/station/station_03_raw.wav | station | wrong | left@1.01s 1.00 | 0.0 | left 1.00 | _unknown_ 0.00 | yes 0.00 | ch0 |
| raw/real/supanut/S0/place/place_05_raw.wav | place | miss |  | 0.11 | right 0.70 | _unknown_ 0.11 | left 0.07 | ch1 |
| raw/real/supanut/S0/_unknown_/phone_01_raw.wav | _unknown_ | false_alarm | forward@1.01s 0.70 | 0.133 | forward 0.91 | _unknown_ 0.06 | stop 0.01 | ch0/ch1 |
| raw/real/supanut/S0/backward/backward_06_raw.wav | backward | wrong | left@1.31s 0.84 | 0.0 | left 0.98 | yes 0.01 | _unknown_ 0.01 | ch0 |
| raw/real/supanut/S0/place/place_06_raw.wav | place | wrong | left@1.01s 1.00 | 0.0 | left 1.00 | right 0.00 | _unknown_ 0.00 | ch0 |
| raw/real/supanut/S0/hand/hand_03_raw.wav | hand | wrong | left@1.01s 0.81 | 0.013 | left 0.82 | _unknown_ 0.08 | right 0.05 | ch0 |
| raw/real/supanut/S0/_unknown_/moon_01_raw.wav | _unknown_ | false_alarm | left@1.01s 0.99 | 0.022 | left 1.00 | right 0.00 | _unknown_ 0.00 | ch0 |
| raw/real/supanut/S0/hand/hand_04_raw.wav | hand | wrong | left@1.01s 1.00 | 0.0 | left 1.00 | right 0.00 | _unknown_ 0.00 | ch0 |
| raw/real/supanut/S0/_unknown_/go_01_raw.wav | _unknown_ | false_alarm | left@1.01s 1.00 | 0.043 | left 1.00 | _unknown_ 0.00 | right 0.00 | ch0 |
| raw/real/supanut/S0/station/station_04_raw.wav | station | wrong | left@1.01s 1.00 | 0.0 | left 1.00 | _unknown_ 0.00 | black box 0.00 | ch0 |
| raw/real/supanut/S0/forward/forward_02_raw.wav | forward | wrong | left@1.01s 0.87 | 0.0 | left 0.99 | yes 0.01 | black box 0.00 | ch0 |
| raw/real/supanut/S0/move/move_04_raw.wav | move | miss |  | 0.0 | _unknown_ 0.49 | left 0.37 | stop 0.08 | ch0 |
| raw/real/supanut/S0/left/left_02_raw.wav | left | miss |  | 0.617 | left 0.62 | backward 0.17 | yes 0.13 | ch0/ch1 |
| raw/real/supanut/S0/_unknown_/foam_01_raw.wav | _unknown_ | false_alarm | left@1.41s 0.71 | 0.021 | left 0.71 | backward 0.13 | yes 0.10 | ch0 |
| raw/real/supanut/S0/station/station_05_raw.wav | station | miss |  | 0.0 | _silence_ 1.00 | yes 0.00 | left 0.00 | ch0 |
| raw/real/supanut/S0/red_box/red_box_03_raw.wav | red box | miss |  | 0.011 | backward 0.69 | _unknown_ 0.29 | right 0.02 | ch1 |
| raw/real/supanut/S0/backward/backward_07_raw.wav | backward | miss |  | 0.56 | backward 0.56 | left 0.20 | forward 0.05 | ch0/ch1 |
| raw/real/supanut/S0/home/home_04_raw.wav | home | miss |  | 0.0 | yes 0.46 | left 0.19 | _unknown_ 0.12 | ch0 |
| raw/real/supanut/S0/right/right_03_raw.wav | right | wrong | left@1.41s 0.72 | 0.002 | left 0.81 | yes 0.07 | _unknown_ 0.06 | ch0 |
| raw/real/supanut/S0/home/home_05_raw.wav | home | miss |  | 0.171 | forward 0.64 | _unknown_ 0.15 | home 0.14 | ch0/ch1 |
| raw/real/supanut/S0/_unknown_/bright_01_raw.wav | _unknown_ | false_alarm | right@1.31s 0.98 | 0.008 | right 0.99 | left 0.00 | _unknown_ 0.00 | ch0/ch1 |
| raw/real/supanut/S0/left/left_03_raw.wav | left | miss |  | 0.001 | _silence_ 1.00 | left 0.00 | yes 0.00 | ch0 |
| raw/real/supanut/S0/_unknown_/open_01_raw.wav | _unknown_ | false_alarm | forward@1.01s 0.72 | 0.313 | forward 0.72 | _unknown_ 0.12 | stop 0.10 | ch0/ch1 |
| raw/real/supanut/S0/pick/pick_02_raw.wav | pick | miss |  | 0.362 | pick 0.36 | _unknown_ 0.24 | left 0.19 | ch0/ch1 |
| raw/real/supanut/S0/blue_box/blue_box_05_raw.wav | blue box | wrong | no@1.01s 0.87 | 0.009 | no 0.91 | _unknown_ 0.04 | yes 0.02 | ch1 |
| raw/real/supanut/S0/stop/stop_04_raw.wav | stop | miss |  | 0.0 | _silence_ 1.00 | yes 0.00 | no 0.00 | ch0 |
| raw/real/supanut/S0/left/left_04_raw.wav | left | miss |  | 0.634 | left 0.63 | backward 0.20 | yes 0.11 | ch0/ch1 |
| raw/real/supanut/S0/drop/drop_02_raw.wav | drop | miss |  | 0.144 | stop 0.62 | no 0.17 | drop 0.12 | ch0/ch1 |
| raw/real/supanut/S0/blue_box/blue_box_06_raw.wav | blue box | wrong | no@1.01s 0.87 | 0.013 | no 0.87 | backward 0.06 | _unknown_ 0.03 | ch1 |
| raw/real/supanut/S0/base/base_04_raw.wav | base | miss |  | 0.0 | _silence_ 1.00 | yes 0.00 | left 0.00 | ch0 |
| raw/real/supanut/S0/blue_box/blue_box_07_raw.wav | blue box | wrong | no@1.01s 0.92 | 0.005 | no 0.94 | _unknown_ 0.03 | backward 0.02 | ch0/ch1 |
| raw/real/supanut/S0/place/place_07_raw.wav | place | wrong | left@1.01s 1.00 | 0.0 | left 1.00 | _unknown_ 0.00 | yes 0.00 | ch0 |
| raw/real/supanut/S0/no/no_03_raw.wav | no | wrong | left@1.01s 0.91 | 0.0 | left 0.91 | pick 0.08 | grab 0.00 | ch0 |
| raw/real/supanut/S0/grab/grab_07_raw.wav | grab | wrong | right@1.01s 0.91 | 0.016 | right 0.91 | left 0.05 | _unknown_ 0.02 | ch0/ch1 |
| raw/real/supanut/S0/_unknown_/blue_01_raw.wav | _unknown_ | false_alarm | no@1.21s 0.74 | 0.016 | no 0.99 | _unknown_ 0.00 | move 0.00 | ch1 |
| raw/real/supanut/S0/move/move_05_raw.wav | move | wrong | no@1.41s 0.72 | 0.004 | no 0.73 | backward 0.16 | _unknown_ 0.07 | ch1 |
| raw/real/supanut/S0/base/base_05_raw.wav | base | miss |  | 0.283 | backward 0.58 | _unknown_ 0.22 | right 0.08 | ch1 |
| raw/real/supanut/S0/_unknown_/bag_01_raw.wav | _unknown_ | false_alarm | backward@1.31s 0.71 | 0.042 | backward 0.75 | left 0.18 | _unknown_ 0.04 | ch1 |
