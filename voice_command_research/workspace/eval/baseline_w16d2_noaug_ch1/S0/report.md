# Board evaluation - baseline_w16d2_noaug_ch1 on S0

model `/sessions/rcw-ac6fce39038343a9b68f42328258/mnt/prefer/onnx_out/kws.onnx` (onnx (input logmel [1, 40, 101])) · gain +15.0 dB · features numpy · 2026-10-18 11:07

## Stream mode (raw 2 s takes, board logic: stride 10 frames, avg 3, threshold 0.7, refractory 1500 ms, channel 1)

command takes 94: **detected 0.4043** (hit 38, hit+extra 0), wrong 25, miss 31 · first detection correct 0.4043 · _unknown_ false alarms 9/15 · noise false alarms 0 in 0.0 min · channels {'ch1': 109}

| cls | takes | hit | hit_extra | wrong | miss | false_alarm | detected_rate | wrong_as | mean_max_p_true |
|---|---|---|---|---|---|---|---|---|---|
| move | 5 | 0 | 0 | 4 | 1 | 0 | 0.0 | no 4 | 0.009 |
| pick | 2 | 0 | 0 | 0 | 2 | 0 | 0.0 |  | 0.464 |
| grab | 7 | 0 | 0 | 5 | 2 | 0 | 0.0 | right 5 | 0.003 |
| blue box | 7 | 0 | 0 | 7 | 0 | 0 | 0.0 | no 7 | 0.032 |
| green box | 6 | 0 | 0 | 3 | 3 | 0 | 0.0 | right 3 | 0.298 |
| red box | 3 | 0 | 0 | 2 | 1 | 0 | 0.0 | right 2 | 0.212 |
| black box | 4 | 0 | 0 | 3 | 1 | 0 | 0.0 | backward 3 | 0.011 |
| hand | 5 | 3 | 0 | 0 | 2 | 0 | 0.6 |  | 0.639 |
| station | 6 | 6 | 0 | 0 | 0 | 0 | 1.0 |  | 0.872 |
| base | 5 | 2 | 0 | 0 | 3 | 0 | 0.4 |  | 0.428 |
| forward | 4 | 4 | 0 | 0 | 0 | 0 | 1.0 |  | 0.97 |
| backward | 7 | 7 | 0 | 0 | 0 | 0 | 1.0 |  | 1.0 |
| left | 5 | 2 | 0 | 0 | 3 | 0 | 0.4 |  | 0.73 |
| right | 3 | 3 | 0 | 0 | 0 | 0 | 1.0 |  | 0.998 |
| place | 7 | 0 | 0 | 0 | 7 | 0 | 0.0 |  | 0.283 |
| drop | 2 | 0 | 0 | 0 | 2 | 0 | 0.0 |  | 0.287 |
| yes | 2 | 2 | 0 | 0 | 0 | 0 | 1.0 |  | 0.994 |
| no | 4 | 4 | 0 | 0 | 0 | 0 | 1.0 |  | 0.988 |
| home | 5 | 0 | 0 | 1 | 4 | 0 | 0.0 | forward 1 | 0.215 |
| stop | 5 | 5 | 0 | 0 | 0 | 0 | 1.0 |  | 0.996 |
| _unknown_ | 15 | 0 | 0 | 0 | 0 | 9 |  | forward 4, no 2, right 2 | 0.174 |

### Takes that were not a clean hit

| raw_path | label | outcome | detections | max_avg_p_true | peak_top3 | channels_used |
|---|---|---|---|---|---|---|
| raw/real/supanut/S0/move/move_01_raw.wav | move | wrong | no@1.41s 0.77 | 0.013 | no 0.79 | backward 0.11 | left 0.04 | ch1 |
| raw/real/supanut/S0/black_box/black_box_01_raw.wav | black box | wrong | backward@1.01s 0.72 | 0.001 | backward 0.95 | no 0.04 | left 0.01 | ch1 |
| raw/real/supanut/S0/grab/grab_01_raw.wav | grab | wrong | right@1.41s 0.76 | 0.0 | right 0.77 | left 0.19 | _unknown_ 0.02 | ch1 |
| raw/real/supanut/S0/home/home_01_raw.wav | home | wrong | forward@1.31s 0.72 | 0.035 | forward 0.74 | _unknown_ 0.11 | no 0.06 | ch1 |
| raw/real/supanut/S0/blue_box/blue_box_01_raw.wav | blue box | wrong | no@1.01s 0.83 | 0.029 | no 0.93 | _unknown_ 0.02 | backward 0.02 | ch1 |
| raw/real/supanut/S0/home/home_02_raw.wav | home | miss |  | 0.142 | forward 0.67 | _unknown_ 0.15 | home 0.13 | ch1 |
| raw/real/supanut/S0/red_box/red_box_01_raw.wav | red box | wrong | right@1.01s 0.95 | 0.499 | right 0.97 | backward 0.02 | _unknown_ 0.01 | ch1 |
| raw/real/supanut/S0/place/place_01_raw.wav | place | miss |  | 0.363 | left 0.45 | right 0.15 | yes 0.15 | ch1 |
| raw/real/supanut/S0/pick/pick_01_raw.wav | pick | miss |  | 0.567 | pick 0.57 | left 0.17 | _unknown_ 0.15 | ch1 |
| raw/real/supanut/S0/black_box/black_box_02_raw.wav | black box | wrong | backward@1.11s 0.81 | 0.026 | backward 0.94 | no 0.04 | left 0.02 | ch1 |
| raw/real/supanut/S0/hand/hand_01_raw.wav | hand | miss |  | 0.237 | right 0.54 | _unknown_ 0.13 | left 0.08 | ch1 |
| raw/real/supanut/S0/move/move_02_raw.wav | move | wrong | no@1.31s 0.74 | 0.002 | no 0.75 | backward 0.14 | _unknown_ 0.06 | ch1 |
| raw/real/supanut/S0/green_box/green_box_01_raw.wav | green box | miss |  | 0.277 | _unknown_ 0.91 | green box 0.08 | right 0.01 | ch1 |
| raw/real/supanut/S0/base/base_01_raw.wav | base | miss |  | 0.264 | place 0.40 | _unknown_ 0.23 | yes 0.13 | ch1 |
| raw/real/supanut/S0/green_box/green_box_02_raw.wav | green box | wrong | right@1.01s 0.81 | 0.495 | right 0.81 | _unknown_ 0.18 | green box 0.00 | ch1 |
| raw/real/supanut/S0/blue_box/blue_box_02_raw.wav | blue box | wrong | no@1.11s 0.76 | 0.083 | no 0.84 | move 0.05 | _unknown_ 0.05 | ch1 |
| raw/real/supanut/S0/_unknown_/nose_01_raw.wav | _unknown_ | false_alarm | no@1.01s 0.95 | 0.064 | no 0.95 | backward 0.04 | _unknown_ 0.01 | ch1 |
| raw/real/supanut/S0/place/place_02_raw.wav | place | miss |  | 0.376 | left 0.52 | backward 0.18 | no 0.09 | ch1 |
| raw/real/supanut/S0/hand/hand_02_raw.wav | hand | miss |  | 0.567 | hand 0.57 | _unknown_ 0.29 | no 0.07 | ch1 |
| raw/real/supanut/S0/grab/grab_02_raw.wav | grab | wrong | right@1.01s 0.71 | 0.004 | right 0.82 | left 0.09 | backward 0.06 | ch1 |
| raw/real/supanut/S0/green_box/green_box_03_raw.wav | green box | miss |  | 0.194 | _unknown_ 0.85 | green box 0.10 | right 0.04 | ch1 |
| raw/real/supanut/S0/_unknown_/write_01_raw.wav | _unknown_ | false_alarm | right@1.11s 0.72 | 0.011 | right 1.00 | left 0.00 | _unknown_ 0.00 | ch1 |
| raw/real/supanut/S0/black_box/black_box_03_raw.wav | black box | miss |  | 0.008 | no 0.67 | backward 0.19 | yes 0.07 | ch1 |
| raw/real/supanut/S0/green_box/green_box_04_raw.wav | green box | miss |  | 0.464 | _unknown_ 0.80 | green box 0.16 | right 0.04 | ch1 |
| raw/real/supanut/S0/green_box/green_box_05_raw.wav | green box | wrong | right@1.01s 0.85 | 0.259 | right 0.85 | _unknown_ 0.11 | backward 0.02 | ch1 |
| raw/real/supanut/S0/red_box/red_box_02_raw.wav | red box | wrong | right@1.11s 0.74 | 0.127 | backward 0.79 | right 0.12 | _unknown_ 0.06 | ch1 |
| raw/real/supanut/S0/blue_box/blue_box_03_raw.wav | blue box | wrong | no@1.01s 0.91 | 0.012 | no 0.95 | _unknown_ 0.03 | yes 0.01 | ch1 |
| raw/real/supanut/S0/home/home_03_raw.wav | home | miss |  | 0.548 | home 0.55 | forward 0.22 | _unknown_ 0.15 | ch1 |
| raw/real/supanut/S0/green_box/green_box_06_raw.wav | green box | wrong | right@1.01s 0.86 | 0.1 | right 0.86 | _unknown_ 0.13 | backward 0.00 | ch1 |
| raw/real/supanut/S0/grab/grab_03_raw.wav | grab | wrong | right@1.11s 0.75 | 0.003 | right 0.86 | left 0.06 | backward 0.04 | ch1 |
| raw/real/supanut/S0/black_box/black_box_04_raw.wav | black box | wrong | backward@1.01s 0.74 | 0.009 | backward 0.98 | no 0.01 | left 0.00 | ch1 |
| raw/real/supanut/S0/grab/grab_04_raw.wav | grab | wrong | right@1.01s 0.82 | 0.001 | right 0.82 | left 0.10 | backward 0.04 | ch1 |
| raw/real/supanut/S0/grab/grab_05_raw.wav | grab | miss |  | 0.005 | left 0.53 | right 0.24 | backward 0.15 | ch1 |
| raw/real/supanut/S0/drop/drop_01_raw.wav | drop | miss |  | 0.431 | drop 0.43 | stop 0.24 | _unknown_ 0.16 | ch1 |
| raw/real/supanut/S0/place/place_03_raw.wav | place | miss |  | 0.465 | place 0.46 | yes 0.30 | left 0.10 | ch1 |
| raw/real/supanut/S0/move/move_03_raw.wav | move | miss |  | 0.02 | no 0.61 | backward 0.23 | _unknown_ 0.06 | ch1 |
| raw/real/supanut/S0/blue_box/blue_box_04_raw.wav | blue box | wrong | no@1.21s 0.71 | 0.037 | no 0.86 | _unknown_ 0.06 | blue box 0.03 | ch1 |
| raw/real/supanut/S0/place/place_04_raw.wav | place | miss |  | 0.272 | right 0.36 | _unknown_ 0.26 | left 0.13 | ch1 |
| raw/real/supanut/S0/grab/grab_06_raw.wav | grab | miss |  | 0.007 | right 0.51 | left 0.27 | backward 0.16 | ch1 |
| raw/real/supanut/S0/place/place_05_raw.wav | place | miss |  | 0.11 | right 0.70 | _unknown_ 0.11 | left 0.07 | ch1 |
| raw/real/supanut/S0/_unknown_/phone_01_raw.wav | _unknown_ | false_alarm | forward@1.01s 0.70 | 0.14 | forward 0.91 | _unknown_ 0.06 | stop 0.01 | ch1 |
| raw/real/supanut/S0/place/place_06_raw.wav | place | miss |  | 0.073 | yes 0.67 | _unknown_ 0.13 | left 0.10 | ch1 |
| raw/real/supanut/S0/_unknown_/go_01_raw.wav | _unknown_ | false_alarm | forward@1.41s 0.73 | 0.152 | forward 0.76 | _unknown_ 0.12 | stop 0.08 | ch1 |
| raw/real/supanut/S0/move/move_04_raw.wav | move | wrong | no@1.01s 0.75 | 0.006 | no 0.75 | backward 0.15 | _unknown_ 0.05 | ch1 |
| raw/real/supanut/S0/left/left_02_raw.wav | left | miss |  | 0.617 | left 0.62 | backward 0.17 | yes 0.13 | ch1 |
| raw/real/supanut/S0/_unknown_/foam_01_raw.wav | _unknown_ | false_alarm | forward@1.01s 0.88 | 0.167 | forward 0.92 | _unknown_ 0.05 | home 0.01 | ch1 |
| raw/real/supanut/S0/red_box/red_box_03_raw.wav | red box | miss |  | 0.011 | backward 0.69 | _unknown_ 0.29 | right 0.02 | ch1 |
| raw/real/supanut/S0/home/home_04_raw.wav | home | miss |  | 0.18 | forward 0.59 | _unknown_ 0.17 | home 0.15 | ch1 |
| raw/real/supanut/S0/home/home_05_raw.wav | home | miss |  | 0.171 | forward 0.64 | _unknown_ 0.15 | home 0.14 | ch1 |
| raw/real/supanut/S0/_unknown_/bright_01_raw.wav | _unknown_ | false_alarm | right@1.21s 0.79 | 0.01 | right 0.99 | left 0.00 | _unknown_ 0.00 | ch1 |
| raw/real/supanut/S0/left/left_03_raw.wav | left | miss |  | 0.669 | left 0.67 | yes 0.16 | backward 0.12 | ch1 |
| raw/real/supanut/S0/_unknown_/open_01_raw.wav | _unknown_ | false_alarm | forward@1.01s 0.72 | 0.313 | forward 0.72 | _unknown_ 0.12 | stop 0.10 | ch1 |
| raw/real/supanut/S0/pick/pick_02_raw.wav | pick | miss |  | 0.362 | pick 0.36 | _unknown_ 0.24 | left 0.19 | ch1 |
| raw/real/supanut/S0/blue_box/blue_box_05_raw.wav | blue box | wrong | no@1.01s 0.87 | 0.009 | no 0.91 | _unknown_ 0.04 | yes 0.02 | ch1 |
| raw/real/supanut/S0/left/left_04_raw.wav | left | miss |  | 0.634 | left 0.63 | backward 0.20 | yes 0.11 | ch1 |
| raw/real/supanut/S0/drop/drop_02_raw.wav | drop | miss |  | 0.144 | stop 0.62 | no 0.17 | drop 0.12 | ch1 |
| raw/real/supanut/S0/blue_box/blue_box_06_raw.wav | blue box | wrong | no@1.01s 0.87 | 0.013 | no 0.87 | backward 0.06 | _unknown_ 0.03 | ch1 |
| raw/real/supanut/S0/base/base_04_raw.wav | base | miss |  | 0.074 | _unknown_ 0.49 | right 0.28 | backward 0.17 | ch1 |
| raw/real/supanut/S0/blue_box/blue_box_07_raw.wav | blue box | wrong | no@1.01s 0.92 | 0.039 | no 0.94 | _unknown_ 0.03 | backward 0.02 | ch1 |
| raw/real/supanut/S0/place/place_07_raw.wav | place | miss |  | 0.321 | yes 0.36 | _unknown_ 0.24 | place 0.23 | ch1 |
| raw/real/supanut/S0/grab/grab_07_raw.wav | grab | wrong | right@1.01s 0.91 | 0.0 | right 0.91 | left 0.05 | _unknown_ 0.02 | ch1 |
| raw/real/supanut/S0/_unknown_/blue_01_raw.wav | _unknown_ | false_alarm | no@1.21s 0.74 | 0.016 | no 0.99 | _unknown_ 0.00 | move 0.00 | ch1 |
| raw/real/supanut/S0/move/move_05_raw.wav | move | wrong | no@1.41s 0.72 | 0.004 | no 0.73 | backward 0.16 | _unknown_ 0.07 | ch1 |
| raw/real/supanut/S0/base/base_05_raw.wav | base | miss |  | 0.283 | backward 0.58 | _unknown_ 0.22 | right 0.08 | ch1 |
| raw/real/supanut/S0/_unknown_/bag_01_raw.wav | _unknown_ | false_alarm | backward@1.31s 0.71 | 0.042 | backward 0.75 | left 0.18 | _unknown_ 0.04 | ch1 |
