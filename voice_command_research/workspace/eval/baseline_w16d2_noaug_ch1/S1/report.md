# Board evaluation - baseline_w16d2_noaug_ch1 on S1

model `/sessions/rcw-ac6fce39038343a9b68f42328258/mnt/prefer/onnx_out/kws.onnx` (onnx (input logmel [1, 40, 101])) · gain +15.0 dB · features numpy · 2026-10-18 11:07

## Stream mode (raw 2 s takes, board logic: stride 10 frames, avg 3, threshold 0.7, refractory 1500 ms, channel 1)

command takes 200: **detected 0.375** (hit 75, hit+extra 0), wrong 44, miss 81 · first detection correct 0.375 · _unknown_ false alarms 16/30 · noise false alarms 0 in 1.0 min · channels {'ch1': 230}

| cls | takes | hit | hit_extra | wrong | miss | false_alarm | detected_rate | wrong_as | mean_max_p_true |
|---|---|---|---|---|---|---|---|---|---|
| move | 10 | 0 | 0 | 1 | 9 | 0 | 0.0 | no 1 | 0.003 |
| pick | 10 | 0 | 0 | 0 | 10 | 0 | 0.0 |  | 0.145 |
| grab | 10 | 0 | 0 | 5 | 5 | 0 | 0.0 | right 5 | 0.005 |
| blue box | 10 | 0 | 0 | 10 | 0 | 0 | 0.0 | no 10 | 0.024 |
| green box | 10 | 0 | 0 | 1 | 9 | 0 | 0.0 | right 1 | 0.142 |
| red box | 10 | 0 | 0 | 7 | 3 | 0 | 0.0 | backward 5, right 2 | 0.041 |
| black box | 10 | 0 | 0 | 8 | 2 | 0 | 0.0 | backward 8 | 0.004 |
| hand | 10 | 0 | 0 | 0 | 10 | 0 | 0.0 |  | 0.31 |
| station | 10 | 6 | 0 | 0 | 4 | 0 | 0.6 |  | 0.698 |
| base | 10 | 0 | 0 | 0 | 10 | 0 | 0.0 |  | 0.242 |
| forward | 10 | 10 | 0 | 0 | 0 | 0 | 1.0 |  | 0.885 |
| backward | 10 | 10 | 0 | 0 | 0 | 0 | 1.0 |  | 0.997 |
| left | 10 | 9 | 0 | 0 | 1 | 0 | 0.9 |  | 0.876 |
| right | 10 | 10 | 0 | 0 | 0 | 0 | 1.0 |  | 0.973 |
| place | 10 | 0 | 0 | 1 | 9 | 0 | 0.0 | left 1 | 0.38 |
| drop | 10 | 0 | 0 | 1 | 9 | 0 | 0.0 | stop 1 | 0.198 |
| yes | 10 | 10 | 0 | 0 | 0 | 0 | 1.0 |  | 0.942 |
| no | 10 | 10 | 0 | 0 | 0 | 0 | 1.0 |  | 0.99 |
| home | 10 | 0 | 0 | 10 | 0 | 0 | 0.0 | forward 10 | 0.033 |
| stop | 10 | 10 | 0 | 0 | 0 | 0 | 1.0 |  | 0.994 |
| _unknown_ | 30 | 0 | 0 | 0 | 0 | 16 |  | no 4, right 4, forward 3 | 0.113 |

### Takes that were not a clean hit

| raw_path | label | outcome | detections | max_avg_p_true | peak_top3 | channels_used |
|---|---|---|---|---|---|---|
| raw/real/supanut/S1/pick/pick_01_raw.wav | pick | miss |  | 0.582 | pick 0.58 | left 0.13 | _unknown_ 0.12 | ch1 |
| raw/real/supanut/S1/drop/drop_01_raw.wav | drop | miss |  | 0.577 | left 0.69 | right 0.21 | backward 0.05 | ch1 |
| raw/real/supanut/S1/base/base_01_raw.wav | base | miss |  | 0.389 | place 0.45 | base 0.30 | _unknown_ 0.15 | ch1 |
| raw/real/supanut/S1/home/home_01_raw.wav | home | wrong | forward@1.01s 0.79 | 0.04 | forward 0.87 | _unknown_ 0.08 | home 0.02 | ch1 |
| raw/real/supanut/S1/place/place_01_raw.wav | place | wrong | left@1.01s 0.77 | 0.725 | left 0.77 | right 0.11 | yes 0.03 | ch1 |
| raw/real/supanut/S1/base/base_02_raw.wav | base | miss |  | 0.573 | backward 0.59 | left 0.12 | right 0.12 | ch1 |
| raw/real/supanut/S1/hand/hand_01_raw.wav | hand | miss |  | 0.235 | _unknown_ 0.36 | hand 0.20 | left 0.15 | ch1 |
| raw/real/supanut/S1/move/move_01_raw.wav | move | miss |  | 0.002 | no 0.60 | backward 0.26 | left 0.05 | ch1 |
| raw/real/supanut/S1/black_box/black_box_01_raw.wav | black box | wrong | backward@1.01s 0.86 | 0.001 | backward 0.86 | no 0.07 | left 0.06 | ch1 |
| raw/real/supanut/S1/move/move_02_raw.wav | move | miss |  | 0.001 | no 0.57 | backward 0.26 | left 0.08 | ch1 |
| raw/real/supanut/S1/black_box/black_box_02_raw.wav | black box | wrong | backward@1.01s 0.85 | 0.009 | backward 0.85 | no 0.12 | left 0.02 | ch1 |
| raw/real/supanut/S1/blue_box/blue_box_01_raw.wav | blue box | wrong | no@1.01s 0.90 | 0.035 | no 0.90 | yes 0.07 | _unknown_ 0.02 | ch1 |
| raw/real/supanut/S1/red_box/red_box_01_raw.wav | red box | miss |  | 0.016 | _unknown_ 0.82 | backward 0.08 | right 0.06 | ch1 |
| raw/real/supanut/S1/grab/grab_01_raw.wav | grab | miss |  | 0.0 | left 0.70 | right 0.30 | _unknown_ 0.00 | ch1 |
| raw/real/supanut/S1/place/place_02_raw.wav | place | miss |  | 0.494 | left 0.58 | yes 0.14 | right 0.09 | ch1 |
| raw/real/supanut/S1/place/place_03_raw.wav | place | miss |  | 0.525 | place 0.52 | _unknown_ 0.17 | yes 0.11 | ch1 |
| raw/real/supanut/S1/hand/hand_02_raw.wav | hand | miss |  | 0.263 | _unknown_ 0.46 | hand 0.23 | right 0.09 | ch1 |
| raw/real/supanut/S1/drop/drop_02_raw.wav | drop | miss |  | 0.1 | stop 0.45 | no 0.21 | _unknown_ 0.14 | ch1 |
| raw/real/supanut/S1/place/place_04_raw.wav | place | miss |  | 0.168 | left 0.35 | yes 0.29 | _unknown_ 0.21 | ch1 |
| raw/real/supanut/S1/red_box/red_box_02_raw.wav | red box | wrong | backward@1.01s 0.79 | 0.11 | backward 0.80 | right 0.16 | _unknown_ 0.04 | ch1 |
| raw/real/supanut/S1/_unknown_/hold_01_raw.wav | _unknown_ | false_alarm | forward@1.01s 0.79 | 0.083 | forward 0.89 | _unknown_ 0.05 | no 0.02 | ch1 |
| raw/real/supanut/S1/green_box/green_box_01_raw.wav | green box | miss |  | 0.036 | _unknown_ 0.82 | right 0.07 | yes 0.04 | ch1 |
| raw/real/supanut/S1/place/place_05_raw.wav | place | miss |  | 0.279 | left 0.52 | yes 0.15 | right 0.10 | ch1 |
| raw/real/supanut/S1/green_box/green_box_02_raw.wav | green box | miss |  | 0.262 | _unknown_ 0.82 | green box 0.07 | right 0.06 | ch1 |
| raw/real/supanut/S1/drop/drop_03_raw.wav | drop | miss |  | 0.062 | stop 0.53 | no 0.31 | _unknown_ 0.09 | ch1 |
| raw/real/supanut/S1/_unknown_/close_01_raw.wav | _unknown_ | false_alarm | yes@1.61s 0.72 | 0.049 | yes 0.72 | left 0.12 | no 0.10 | ch1 |
| raw/real/supanut/S1/grab/grab_02_raw.wav | grab | miss |  | 0.001 | right 0.55 | left 0.42 | backward 0.02 | ch1 |
| raw/real/supanut/S1/base/base_03_raw.wav | base | miss |  | 0.601 | base 0.60 | _unknown_ 0.22 | place 0.11 | ch1 |
| raw/real/supanut/S1/move/move_03_raw.wav | move | wrong | no@1.31s 0.74 | 0.007 | no 0.74 | backward 0.18 | _unknown_ 0.03 | ch1 |
| raw/real/supanut/S1/grab/grab_03_raw.wav | grab | wrong | right@1.01s 0.82 | 0.0 | right 0.83 | left 0.15 | backward 0.01 | ch1 |
| raw/real/supanut/S1/green_box/green_box_03_raw.wav | green box | wrong | right@1.01s 0.71 | 0.161 | _unknown_ 0.88 | green box 0.08 | right 0.03 | ch1 |
| raw/real/supanut/S1/_unknown_/nose_01_raw.wav | _unknown_ | false_alarm | no@1.01s 0.95 | 0.038 | no 0.95 | backward 0.04 | _unknown_ 0.01 | ch1 |
| raw/real/supanut/S1/green_box/green_box_04_raw.wav | green box | miss |  | 0.182 | _unknown_ 0.90 | green box 0.09 | right 0.01 | ch1 |
| raw/real/supanut/S1/black_box/black_box_03_raw.wav | black box | wrong | backward@1.01s 0.79 | 0.001 | backward 0.79 | no 0.14 | left 0.05 | ch1 |
| raw/real/supanut/S1/red_box/red_box_03_raw.wav | red box | wrong | right@1.01s 0.81 | 0.044 | right 0.81 | backward 0.14 | _unknown_ 0.03 | ch1 |
| raw/real/supanut/S1/black_box/black_box_04_raw.wav | black box | wrong | backward@1.01s 0.97 | 0.002 | backward 0.97 | no 0.02 | left 0.01 | ch1 |
| raw/real/supanut/S1/base/base_04_raw.wav | base | miss |  | 0.072 | yes 0.57 | _unknown_ 0.12 | place 0.12 | ch1 |
| raw/real/supanut/S1/drop/drop_04_raw.wav | drop | miss |  | 0.514 | drop 0.51 | _unknown_ 0.19 | right 0.09 | ch1 |
| raw/real/supanut/S1/red_box/red_box_04_raw.wav | red box | wrong | backward@1.01s 0.90 | 0.044 | backward 0.90 | _unknown_ 0.06 | right 0.02 | ch1 |
| raw/real/supanut/S1/blue_box/blue_box_02_raw.wav | blue box | wrong | no@1.01s 0.93 | 0.009 | no 0.96 | _unknown_ 0.02 | yes 0.01 | ch1 |
| raw/real/supanut/S1/grab/grab_04_raw.wav | grab | miss |  | 0.002 | right 0.66 | left 0.32 | backward 0.02 | ch1 |
| raw/real/supanut/S1/hand/hand_03_raw.wav | hand | miss |  | 0.103 | _unknown_ 0.42 | no 0.20 | hand 0.10 | ch1 |
| raw/real/supanut/S1/blue_box/blue_box_03_raw.wav | blue box | wrong | no@1.01s 0.90 | 0.044 | no 0.90 | _unknown_ 0.04 | yes 0.02 | ch1 |
| raw/real/supanut/S1/black_box/black_box_05_raw.wav | black box | wrong | backward@1.01s 0.85 | 0.002 | backward 0.85 | no 0.09 | left 0.05 | ch1 |
| raw/real/supanut/S1/red_box/red_box_05_raw.wav | red box | miss |  | 0.019 | _unknown_ 0.68 | right 0.15 | backward 0.09 | ch1 |
| raw/real/supanut/S1/base/base_05_raw.wav | base | miss |  | 0.341 | yes 0.38 | _unknown_ 0.24 | backward 0.10 | ch1 |
| raw/real/supanut/S1/black_box/black_box_06_raw.wav | black box | wrong | backward@1.01s 0.79 | 0.001 | backward 0.79 | no 0.12 | left 0.07 | ch1 |
| raw/real/supanut/S1/base/base_06_raw.wav | base | miss |  | 0.242 | _unknown_ 0.38 | backward 0.29 | yes 0.10 | ch1 |
| raw/real/supanut/S1/move/move_04_raw.wav | move | miss |  | 0.004 | no 0.70 | backward 0.16 | _unknown_ 0.05 | ch1 |
| raw/real/supanut/S1/_unknown_/wait_01_raw.wav | _unknown_ | false_alarm | right@1.11s 0.71 | 0.04 | right 0.77 | backward 0.11 | left 0.08 | ch1 |
| raw/real/supanut/S1/_unknown_/now_01_raw.wav | _unknown_ | false_alarm | no@1.01s 0.97 | 0.032 | no 0.99 | _unknown_ 0.01 | backward 0.00 | ch1 |
| raw/real/supanut/S1/move/move_05_raw.wav | move | miss |  | 0.001 | no 0.43 | backward 0.16 | forward 0.15 | ch1 |
| raw/real/supanut/S1/_unknown_/bright_01_raw.wav | _unknown_ | false_alarm | left@1.01s 0.87 | 0.014 | right 0.97 | left 0.02 | _unknown_ 0.00 | ch1 |
| raw/real/supanut/S1/red_box/red_box_06_raw.wav | red box | wrong | right@1.01s 0.82 | 0.038 | right 0.82 | backward 0.09 | _unknown_ 0.07 | ch1 |
| raw/real/supanut/S1/_unknown_/back_01_raw.wav | _unknown_ | false_alarm | backward@1.01s 0.82 | 0.022 | backward 0.84 | left 0.10 | no 0.02 | ch1 |
| raw/real/supanut/S1/pick/pick_02_raw.wav | pick | miss |  | 0.126 | left 0.41 | backward 0.15 | _unknown_ 0.15 | ch1 |
| raw/real/supanut/S1/drop/drop_05_raw.wav | drop | miss |  | 0.329 | left 0.33 | right 0.26 | drop 0.13 | ch1 |
| raw/real/supanut/S1/place/place_06_raw.wav | place | miss |  | 0.25 | right 0.33 | _unknown_ 0.22 | left 0.18 | ch1 |
| raw/real/supanut/S1/drop/drop_06_raw.wav | drop | miss |  | 0.059 | stop 0.63 | no 0.15 | _unknown_ 0.08 | ch1 |
| raw/real/supanut/S1/home/home_02_raw.wav | home | wrong | forward@1.01s 0.86 | 0.007 | forward 0.88 | _unknown_ 0.06 | no 0.02 | ch1 |
| raw/real/supanut/S1/pick/pick_03_raw.wav | pick | miss |  | 0.165 | left 0.55 | _unknown_ 0.17 | right 0.08 | ch1 |
| raw/real/supanut/S1/_unknown_/grape_01_raw.wav | _unknown_ | false_alarm | right@1.01s 0.99 | 0.022 | right 0.99 | _unknown_ 0.00 | forward 0.00 | ch1 |
| raw/real/supanut/S1/blue_box/blue_box_04_raw.wav | blue box | wrong | no@1.11s 0.71 | 0.049 | no 0.71 | _unknown_ 0.09 | yes 0.07 | ch1 |
| raw/real/supanut/S1/grab/grab_05_raw.wav | grab | wrong | right@1.01s 0.86 | 0.017 | right 0.86 | left 0.12 | grab 0.01 | ch1 |
| raw/real/supanut/S1/place/place_07_raw.wav | place | miss |  | 0.147 | right 0.30 | _unknown_ 0.24 | left 0.23 | ch1 |
| raw/real/supanut/S1/grab/grab_06_raw.wav | grab | miss |  | 0.002 | right 0.52 | left 0.31 | backward 0.15 | ch1 |
| raw/real/supanut/S1/place/place_08_raw.wav | place | miss |  | 0.25 | _unknown_ 0.60 | place 0.11 | yes 0.10 | ch1 |
| raw/real/supanut/S1/blue_box/blue_box_05_raw.wav | blue box | wrong | no@1.01s 0.82 | 0.063 | no 0.82 | _unknown_ 0.07 | backward 0.05 | ch1 |
| raw/real/supanut/S1/left/left_05_raw.wav | left | miss |  | 0.607 | left 0.61 | yes 0.21 | backward 0.10 | ch1 |
| raw/real/supanut/S1/home/home_03_raw.wav | home | wrong | forward@1.01s 0.84 | 0.008 | forward 0.87 | _unknown_ 0.05 | stop 0.03 | ch1 |
| raw/real/supanut/S1/black_box/black_box_07_raw.wav | black box | wrong | backward@1.01s 0.80 | 0.005 | backward 0.80 | no 0.18 | left 0.02 | ch1 |
| raw/real/supanut/S1/hand/hand_04_raw.wav | hand | miss |  | 0.282 | _unknown_ 0.36 | no 0.25 | hand 0.21 | ch1 |
| raw/real/supanut/S1/red_box/red_box_07_raw.wav | red box | wrong | backward@1.01s 0.80 | 0.007 | backward 0.80 | _unknown_ 0.18 | left 0.01 | ch1 |
| raw/real/supanut/S1/blue_box/blue_box_06_raw.wav | blue box | wrong | no@1.01s 0.89 | 0.005 | no 0.89 | yes 0.06 | _unknown_ 0.03 | ch1 |
| raw/real/supanut/S1/green_box/green_box_05_raw.wav | green box | miss |  | 0.102 | _unknown_ 0.77 | right 0.10 | yes 0.05 | ch1 |
| raw/real/supanut/S1/base/base_07_raw.wav | base | miss |  | 0.043 | yes 0.41 | _unknown_ 0.27 | place 0.09 | ch1 |
| raw/real/supanut/S1/home/home_04_raw.wav | home | wrong | forward@1.11s 0.70 | 0.007 | forward 0.75 | _unknown_ 0.09 | no 0.06 | ch1 |
| raw/real/supanut/S1/_unknown_/blue_01_raw.wav | _unknown_ | false_alarm | no@1.01s 0.98 | 0.036 | no 0.98 | backward 0.01 | _unknown_ 0.01 | ch1 |
| raw/real/supanut/S1/green_box/green_box_06_raw.wav | green box | miss |  | 0.257 | _unknown_ 0.77 | green box 0.14 | right 0.08 | ch1 |
| raw/real/supanut/S1/grab/grab_07_raw.wav | grab | wrong | right@1.01s 0.85 | 0.002 | right 0.86 | left 0.09 | backward 0.03 | ch1 |
| raw/real/supanut/S1/green_box/green_box_07_raw.wav | green box | miss |  | 0.141 | _unknown_ 0.79 | green box 0.11 | right 0.06 | ch1 |
| raw/real/supanut/S1/move/move_06_raw.wav | move | miss |  | 0.002 | no 0.34 | backward 0.30 | left 0.14 | ch1 |
| raw/real/supanut/S1/hand/hand_05_raw.wav | hand | miss |  | 0.32 | _unknown_ 0.42 | hand 0.27 | no 0.16 | ch1 |
| raw/real/supanut/S1/move/move_07_raw.wav | move | miss |  | 0.004 | no 0.58 | backward 0.22 | left 0.07 | ch1 |
| raw/real/supanut/S1/home/home_05_raw.wav | home | wrong | forward@1.21s 0.72 | 0.03 | forward 0.77 | _unknown_ 0.10 | no 0.05 | ch1 |
| raw/real/supanut/S1/pick/pick_04_raw.wav | pick | miss |  | 0.118 | right 0.34 | backward 0.22 | left 0.19 | ch1 |
| raw/real/supanut/S1/grab/grab_08_raw.wav | grab | miss |  | 0.0 | right 0.61 | left 0.34 | backward 0.04 | ch1 |
| raw/real/supanut/S1/hand/hand_06_raw.wav | hand | miss |  | 0.264 | _unknown_ 0.44 | hand 0.17 | no 0.13 | ch1 |
| raw/real/supanut/S1/home/home_06_raw.wav | home | wrong | forward@1.01s 0.73 | 0.004 | forward 0.84 | _unknown_ 0.07 | stop 0.04 | ch1 |
| raw/real/supanut/S1/hand/hand_07_raw.wav | hand | miss |  | 0.504 | hand 0.50 | _unknown_ 0.29 | no 0.11 | ch1 |
| raw/real/supanut/S1/blue_box/blue_box_07_raw.wav | blue box | wrong | no@1.01s 0.73 | 0.006 | no 0.73 | yes 0.12 | left 0.06 | ch1 |
| raw/real/supanut/S1/_unknown_/go_01_raw.wav | _unknown_ | false_alarm | forward@1.01s 0.79 | 0.116 | forward 0.90 | _unknown_ 0.06 | stop 0.02 | ch1 |
| raw/real/supanut/S1/hand/hand_08_raw.wav | hand | miss |  | 0.339 | _unknown_ 0.39 | hand 0.32 | no 0.10 | ch1 |
| raw/real/supanut/S1/_unknown_/write_01_raw.wav | _unknown_ | false_alarm | right@1.01s 0.99 | 0.01 | right 0.99 | red box 0.00 | left 0.00 | ch1 |
| raw/real/supanut/S1/drop/drop_07_raw.wav | drop | wrong | stop@1.01s 0.83 | 0.068 | stop 0.85 | drop 0.07 | no 0.04 | ch1 |
| raw/real/supanut/S1/home/home_07_raw.wav | home | wrong | forward@1.01s 0.73 | 0.087 | forward 0.74 | _unknown_ 0.11 | home 0.07 | ch1 |
| raw/real/supanut/S1/pick/pick_05_raw.wav | pick | miss |  | 0.031 | left 0.41 | backward 0.26 | right 0.12 | ch1 |
| raw/real/supanut/S1/green_box/green_box_08_raw.wav | green box | miss |  | 0.052 | _unknown_ 0.92 | no 0.02 | yes 0.02 | ch1 |
| raw/real/supanut/S1/pick/pick_06_raw.wav | pick | miss |  | 0.102 | left 0.41 | backward 0.24 | _unknown_ 0.11 | ch1 |
| raw/real/supanut/S1/base/base_08_raw.wav | base | miss |  | 0.067 | _unknown_ 0.48 | yes 0.15 | right 0.12 | ch1 |
| raw/real/supanut/S1/red_box/red_box_08_raw.wav | red box | wrong | backward@1.01s 0.82 | 0.072 | backward 0.82 | right 0.13 | _unknown_ 0.05 | ch1 |
| raw/real/supanut/S1/home/home_08_raw.wav | home | wrong | forward@1.01s 0.74 | 0.119 | forward 0.78 | home 0.10 | _unknown_ 0.08 | ch1 |
| raw/real/supanut/S1/blue_box/blue_box_08_raw.wav | blue box | wrong | no@1.01s 0.71 | 0.008 | no 0.71 | yes 0.17 | left 0.05 | ch1 |
| raw/real/supanut/S1/pick/pick_07_raw.wav | pick | miss |  | 0.083 | left 0.35 | _unknown_ 0.20 | backward 0.18 | ch1 |
| raw/real/supanut/S1/drop/drop_08_raw.wav | drop | miss |  | 0.04 | no 0.43 | stop 0.21 | _unknown_ 0.14 | ch1 |
| raw/real/supanut/S1/home/home_09_raw.wav | home | wrong | forward@1.01s 0.73 | 0.02 | forward 0.81 | _unknown_ 0.09 | stop 0.04 | ch1 |
| raw/real/supanut/S1/black_box/black_box_08_raw.wav | black box | wrong | backward@1.01s 0.92 | 0.011 | backward 0.92 | left 0.06 | no 0.01 | ch1 |
| raw/real/supanut/S1/pick/pick_08_raw.wav | pick | miss |  | 0.041 | backward 0.32 | left 0.22 | _unknown_ 0.18 | ch1 |
| raw/real/supanut/S1/hand/hand_09_raw.wav | hand | miss |  | 0.324 | _unknown_ 0.35 | hand 0.31 | no 0.17 | ch1 |
| raw/real/supanut/S1/station/station_06_raw.wav | station | miss |  | 0.616 | station 0.62 | _unknown_ 0.33 | stop 0.03 | ch1 |
| raw/real/supanut/S1/pick/pick_09_raw.wav | pick | miss |  | 0.047 | left 0.36 | backward 0.23 | right 0.15 | ch1 |
| raw/real/supanut/S1/pick/pick_10_raw.wav | pick | miss |  | 0.157 | backward 0.34 | left 0.26 | _unknown_ 0.14 | ch1 |
| raw/real/supanut/S1/home/home_10_raw.wav | home | wrong | forward@1.01s 0.83 | 0.004 | forward 0.84 | _unknown_ 0.06 | stop 0.06 | ch1 |
| raw/real/supanut/S1/station/station_07_raw.wav | station | miss |  | 0.25 | _unknown_ 0.66 | station 0.19 | right 0.07 | ch1 |
| raw/real/supanut/S1/grab/grab_09_raw.wav | grab | wrong | right@1.01s 0.84 | 0.001 | right 0.87 | left 0.12 | backward 0.01 | ch1 |
| raw/real/supanut/S1/hand/hand_10_raw.wav | hand | miss |  | 0.471 | hand 0.47 | _unknown_ 0.31 | no 0.14 | ch1 |
| raw/real/supanut/S1/grab/grab_10_raw.wav | grab | wrong | right@1.01s 0.89 | 0.02 | right 0.89 | left 0.10 | grab 0.01 | ch1 |
| raw/real/supanut/S1/_unknown_/bag_01_raw.wav | _unknown_ | false_alarm | backward@1.41s 0.71 | 0.027 | backward 0.71 | left 0.24 | _unknown_ 0.03 | ch1 |
| raw/real/supanut/S1/green_box/green_box_09_raw.wav | green box | miss |  | 0.163 | _unknown_ 0.74 | green box 0.12 | right 0.10 | ch1 |
| raw/real/supanut/S1/drop/drop_09_raw.wav | drop | miss |  | 0.124 | stop 0.51 | _unknown_ 0.14 | no 0.12 | ch1 |
| raw/real/supanut/S1/base/base_09_raw.wav | base | miss |  | 0.032 | yes 0.61 | _unknown_ 0.14 | place 0.08 | ch1 |
| raw/real/supanut/S1/black_box/black_box_09_raw.wav | black box | miss |  | 0.003 | backward 0.65 | no 0.34 | _unknown_ 0.01 | ch1 |
| raw/real/supanut/S1/place/place_09_raw.wav | place | miss |  | 0.543 | place 0.54 | _unknown_ 0.13 | base 0.13 | ch1 |
| raw/real/supanut/S1/_unknown_/more_01_raw.wav | _unknown_ | false_alarm | no@1.11s 0.74 | 0.06 | no 0.84 | backward 0.06 | _unknown_ 0.04 | ch1 |
| raw/real/supanut/S1/move/move_08_raw.wav | move | miss |  | 0.002 | no 0.40 | left 0.30 | backward 0.14 | ch1 |
| raw/real/supanut/S1/base/base_10_raw.wav | base | miss |  | 0.062 | yes 0.55 | _unknown_ 0.17 | left 0.08 | ch1 |
| raw/real/supanut/S1/blue_box/blue_box_09_raw.wav | blue box | wrong | no@1.01s 0.83 | 0.005 | no 0.83 | yes 0.07 | _unknown_ 0.04 | ch1 |
| raw/real/supanut/S1/blue_box/blue_box_10_raw.wav | blue box | wrong | no@1.01s 0.85 | 0.014 | no 0.85 | yes 0.08 | _unknown_ 0.03 | ch1 |
| raw/real/supanut/S1/_unknown_/foam_01_raw.wav | _unknown_ | false_alarm | forward@1.11s 0.75 | 0.169 | forward 0.81 | _unknown_ 0.10 | stop 0.05 | ch1 |
| raw/real/supanut/S1/place/place_10_raw.wav | place | miss |  | 0.423 | place 0.42 | _unknown_ 0.17 | yes 0.17 | ch1 |
| raw/real/supanut/S1/green_box/green_box_10_raw.wav | green box | miss |  | 0.059 | _unknown_ 0.97 | green box 0.01 | no 0.01 | ch1 |
| raw/real/supanut/S1/_unknown_/up_01_raw.wav | _unknown_ | false_alarm | stop@1.01s 0.72 | 0.029 | stop 0.73 | no 0.14 | backward 0.06 | ch1 |
| raw/real/supanut/S1/station/station_09_raw.wav | station | miss |  | 0.584 | _unknown_ 0.61 | station 0.17 | base 0.11 | ch1 |
| raw/real/supanut/S1/red_box/red_box_09_raw.wav | red box | wrong | backward@1.01s 0.83 | 0.051 | backward 0.83 | _unknown_ 0.15 | right 0.02 | ch1 |
| raw/real/supanut/S1/red_box/red_box_10_raw.wav | red box | miss |  | 0.01 | backward 0.67 | _unknown_ 0.30 | left 0.01 | ch1 |
| raw/real/supanut/S1/drop/drop_10_raw.wav | drop | miss |  | 0.107 | stop 0.54 | no 0.19 | _unknown_ 0.09 | ch1 |
| raw/real/supanut/S1/_unknown_/ride_01_raw.wav | _unknown_ | false_alarm | right@1.11s 0.73 | 0.085 | right 0.86 | _unknown_ 0.06 | backward 0.04 | ch1 |
| raw/real/supanut/S1/move/move_09_raw.wav | move | miss |  | 0.004 | no 0.62 | backward 0.15 | left 0.09 | ch1 |
| raw/real/supanut/S1/station/station_10_raw.wav | station | miss |  | 0.571 | _unknown_ 0.66 | station 0.17 | right 0.03 | ch1 |
| raw/real/supanut/S1/move/move_10_raw.wav | move | miss |  | 0.001 | left 0.32 | no 0.27 | yes 0.16 | ch1 |
| raw/real/supanut/S1/black_box/black_box_10_raw.wav | black box | miss |  | 0.001 | backward 0.60 | no 0.36 | _unknown_ 0.02 | ch1 |

### Noise recordings

| file | duration_s | inferences | false_alarms | top_classes | detections |
|---|---|---|---|---|---|
| raw/noise/board_room/supanut_S1_room.wav | 60.0 | 590 | 0 | _silence_ 590 |  |
