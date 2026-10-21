# Board evaluation - baseline_w16d2_noaug on S1

model `/sessions/rcw-ac6fce39038343a9b68f42328258/mnt/prefer/onnx_out/kws.onnx` (onnx (input logmel [1, 40, 101])) · gain +15.0 dB · features numpy · 2026-10-18 11:07

## Clip mode (1 s clips, mono ch1)

n 290 · **accuracy 0.5414** · **macro-F1 0.3801** · commands 0.47 · _unknown_ 0.1 · _silence_ 1.0 · commands rejected as unknown/silence 30 · speech peak median -34.7 dB (training clip -25.4 dB)

| cls | support | correct | recall | precision | f1 | mean_p_true | confused_with |
|---|---|---|---|---|---|---|---|
| move | 10 | 0 | 0.0 | 0.0 | 0.0 | 0.002 | no 8, left 1, _silence_ 1 |
| pick | 10 | 1 | 0.1 | 1.0 | 0.182 | 0.117 | left 5, backward 2, right 1 |
| grab | 10 | 0 | 0.0 | 0.0 | 0.0 | 0.002 | right 9, left 1 |
| blue box | 10 | 0 | 0.0 | 0.0 | 0.0 | 0.008 | no 10 |
| green box | 10 | 0 | 0.0 | 0.0 | 0.0 | 0.117 | _unknown_ 10 |
| red box | 10 | 0 | 0.0 | 0.0 | 0.0 | 0.022 | backward 5, _unknown_ 5 |
| black box | 10 | 0 | 0.0 | 0.0 | 0.0 | 0.002 | backward 6, no 4 |
| hand | 10 | 2 | 0.2 | 1.0 | 0.333 | 0.287 | _unknown_ 8 |
| station | 10 | 9 | 0.9 | 1.0 | 0.947 | 0.7 | _unknown_ 1 |
| base | 10 | 3 | 0.3 | 1.0 | 0.462 | 0.227 | yes 4, _unknown_ 2, place 1 |
| forward | 10 | 10 | 1.0 | 0.345 | 0.513 | 0.889 |  |
| backward | 10 | 10 | 1.0 | 0.4 | 0.571 | 0.997 |  |
| left | 10 | 10 | 1.0 | 0.526 | 0.69 | 0.855 |  |
| right | 10 | 10 | 1.0 | 0.385 | 0.556 | 0.971 |  |
| place | 10 | 7 | 0.7 | 0.875 | 0.778 | 0.381 | _unknown_ 2, yes 1 |
| drop | 10 | 2 | 0.2 | 1.0 | 0.333 | 0.165 | stop 6, left 1, no 1 |
| yes | 10 | 10 | 1.0 | 0.625 | 0.769 | 0.939 |  |
| no | 10 | 10 | 1.0 | 0.25 | 0.4 | 0.99 |  |
| home | 10 | 0 | 0.0 | 0.0 | 0.0 | 0.022 | forward 10 |
| stop | 10 | 10 | 1.0 | 0.588 | 0.741 | 0.994 |  |
| _unknown_ | 30 | 3 | 0.1 | 0.094 | 0.097 | 0.099 | forward 9, no 7, right 6 |
| _silence_ | 60 | 60 | 1.0 | 0.984 | 0.992 | 0.92 |  |

### Mistakes (top-3)

| clip_path | label | pred | top3 | snr_db | flags |
|---|---|---|---|---|---|
| clips/real/supanut/S1/base/base_01.wav | base | place | place 0.45 | base 0.32 | _unknown_ 0.14 | 19.4 |  |
| clips/real/supanut/S1/home/home_01.wav | home | forward | forward 0.89 | _unknown_ 0.08 | home 0.02 | 25.51 |  |
| clips/real/supanut/S1/hand/hand_01.wav | hand | _unknown_ | _unknown_ 0.33 | hand 0.25 | left 0.15 | 21.11 | other sound outside the clip (2 words? click?) |
| clips/real/supanut/S1/move/move_01.wav | move | no | no 0.59 | backward 0.25 | left 0.05 | 25.03 |  |
| clips/real/supanut/S1/black_box/black_box_01.wav | black box | backward | backward 0.52 | no 0.37 | left 0.06 | 20.72 |  |
| clips/real/supanut/S1/move/move_02.wav | move | no | no 0.52 | backward 0.27 | left 0.11 | 24.06 |  |
| clips/real/supanut/S1/black_box/black_box_02.wav | black box | no | no 0.61 | backward 0.27 | _unknown_ 0.04 | 21.89 |  |
| clips/real/supanut/S1/blue_box/blue_box_01.wav | blue box | no | no 0.63 | yes 0.30 | _unknown_ 0.05 | 24.19 |  |
| clips/real/supanut/S1/red_box/red_box_01.wav | red box | _unknown_ | _unknown_ 0.80 | right 0.10 | backward 0.07 | 21.35 |  |
| clips/real/supanut/S1/grab/grab_01.wav | grab | left | left 0.67 | right 0.33 | _unknown_ 0.00 | 27.02 |  |
| clips/real/supanut/S1/_unknown_/moon_01.wav | _unknown_ | forward | forward 0.24 | backward 0.23 | _unknown_ 0.22 | 25.84 |  |
| clips/real/supanut/S1/hand/hand_02.wav | hand | _unknown_ | _unknown_ 0.49 | hand 0.23 | right 0.06 | 20.96 |  |
| clips/real/supanut/S1/drop/drop_02.wav | drop | stop | stop 0.36 | no 0.23 | _unknown_ 0.19 | 22.65 |  |
| clips/real/supanut/S1/place/place_04.wav | place | yes | yes 0.38 | place 0.21 | left 0.18 | 24.43 |  |
| clips/real/supanut/S1/red_box/red_box_02.wav | red box | backward | backward 0.62 | right 0.17 | _unknown_ 0.12 | 22.82 |  |
| clips/real/supanut/S1/_unknown_/hold_01.wav | _unknown_ | forward | forward 0.91 | _unknown_ 0.04 | no 0.02 | 24.63 |  |
| clips/real/supanut/S1/green_box/green_box_01.wav | green box | _unknown_ | _unknown_ 0.82 | yes 0.06 | green box 0.04 | 20.93 | sound longer than 1 s - the clip cuts it |
| clips/real/supanut/S1/_unknown_/books_01.wav | _unknown_ | no | no 0.34 | left 0.22 | backward 0.18 | 23.75 |  |
| clips/real/supanut/S1/green_box/green_box_02.wav | green box | _unknown_ | _unknown_ 0.74 | green box 0.19 | yes 0.03 | 23.4 |  |
| clips/real/supanut/S1/drop/drop_03.wav | drop | stop | stop 0.59 | no 0.21 | _unknown_ 0.11 | 25.13 |  |
| clips/real/supanut/S1/_unknown_/close_01.wav | _unknown_ | yes | yes 0.71 | left 0.11 | no 0.10 | 17.22 |  |
| clips/real/supanut/S1/grab/grab_02.wav | grab | right | right 0.50 | left 0.45 | backward 0.03 | 25.52 |  |
| clips/real/supanut/S1/move/move_03.wav | move | no | no 0.75 | backward 0.17 | _unknown_ 0.03 | 26.46 |  |
| clips/real/supanut/S1/grab/grab_03.wav | grab | right | right 0.76 | left 0.20 | _unknown_ 0.02 | 24.47 |  |
| clips/real/supanut/S1/green_box/green_box_03.wav | green box | _unknown_ | _unknown_ 0.74 | green box 0.21 | right 0.02 | 23.89 |  |
| clips/real/supanut/S1/_unknown_/nose_01.wav | _unknown_ | no | no 0.91 | yes 0.03 | backward 0.03 | 21.15 |  |
| clips/real/supanut/S1/green_box/green_box_04.wav | green box | _unknown_ | _unknown_ 0.87 | green box 0.13 | right 0.00 | 22.96 |  |
| clips/real/supanut/S1/black_box/black_box_03.wav | black box | no | no 0.58 | backward 0.31 | left 0.05 | 20.67 |  |
| clips/real/supanut/S1/red_box/red_box_03.wav | red box | backward | backward 0.48 | _unknown_ 0.27 | right 0.22 | 23.3 |  |
| clips/real/supanut/S1/black_box/black_box_04.wav | black box | backward | backward 0.73 | no 0.22 | left 0.02 | 22.28 |  |
| clips/real/supanut/S1/base/base_04.wav | base | yes | yes 0.56 | _unknown_ 0.13 | place 0.11 | 20.67 |  |
| clips/real/supanut/S1/red_box/red_box_04.wav | red box | backward | backward 0.69 | _unknown_ 0.22 | right 0.04 | 23.02 |  |
| clips/real/supanut/S1/blue_box/blue_box_02.wav | blue box | no | no 0.93 | _unknown_ 0.03 | yes 0.02 | 21.38 |  |
| clips/real/supanut/S1/_unknown_/groove_01.wav | _unknown_ | forward | forward 0.60 | no 0.20 | _unknown_ 0.10 | 25.92 |  |
| clips/real/supanut/S1/grab/grab_04.wav | grab | right | right 0.59 | left 0.37 | backward 0.02 | 25.77 |  |
| clips/real/supanut/S1/hand/hand_03.wav | hand | _unknown_ | _unknown_ 0.39 | no 0.23 | hand 0.11 | 19.66 |  |
| clips/real/supanut/S1/blue_box/blue_box_03.wav | blue box | no | no 0.80 | _unknown_ 0.06 | move 0.04 | 25.08 |  |
| clips/real/supanut/S1/black_box/black_box_05.wav | black box | backward | backward 0.78 | no 0.18 | left 0.03 | 19.41 |  |
| clips/real/supanut/S1/red_box/red_box_05.wav | red box | _unknown_ | _unknown_ 0.81 | right 0.07 | backward 0.06 | 21.46 |  |
| clips/real/supanut/S1/black_box/black_box_06.wav | black box | backward | backward 0.59 | no 0.35 | _unknown_ 0.04 | 20.09 |  |
| clips/real/supanut/S1/base/base_06.wav | base | _unknown_ | _unknown_ 0.32 | base 0.28 | place 0.12 | 19.82 |  |
| clips/real/supanut/S1/move/move_04.wav | move | no | no 0.73 | backward 0.15 | _unknown_ 0.05 | 22.97 |  |
| clips/real/supanut/S1/_unknown_/wait_01.wav | _unknown_ | right | right 0.81 | backward 0.09 | left 0.05 | 19.04 |  |
| clips/real/supanut/S1/_unknown_/now_01.wav | _unknown_ | no | no 0.99 | _unknown_ 0.01 | backward 0.00 | 19.3 |  |
| clips/real/supanut/S1/move/move_05.wav | move | no | no 0.45 | forward 0.17 | backward 0.16 | 18.8 |  |
| clips/real/supanut/S1/_unknown_/bright_01.wav | _unknown_ | right | right 0.97 | left 0.02 | _unknown_ 0.00 | 20.23 |  |
| clips/real/supanut/S1/red_box/red_box_06.wav | red box | _unknown_ | _unknown_ 0.78 | backward 0.11 | right 0.07 | 20.8 |  |
| clips/real/supanut/S1/_unknown_/back_01.wav | _unknown_ | backward | backward 0.84 | left 0.10 | no 0.02 | 18.02 |  |
| clips/real/supanut/S1/pick/pick_02.wav | pick | left | left 0.42 | backward 0.19 | _unknown_ 0.17 | 18.03 |  |
| clips/real/supanut/S1/drop/drop_05.wav | drop | left | left 0.23 | drop 0.22 | stop 0.21 | 25.57 | other sound outside the clip (2 words? click?) |
| clips/real/supanut/S1/drop/drop_06.wav | drop | stop | stop 0.63 | no 0.13 | _unknown_ 0.10 | 24.5 |  |
| clips/real/supanut/S1/home/home_02.wav | home | forward | forward 0.88 | _unknown_ 0.06 | no 0.02 | 25.17 |  |
| clips/real/supanut/S1/pick/pick_03.wav | pick | left | left 0.44 | _unknown_ 0.34 | pick 0.08 | 19.46 |  |
| clips/real/supanut/S1/_unknown_/grape_01.wav | _unknown_ | right | right 0.99 | _unknown_ 0.00 | left 0.00 | 26.07 |  |
| clips/real/supanut/S1/blue_box/blue_box_04.wav | blue box | no | no 0.70 | yes 0.14 | _unknown_ 0.10 | 24.3 |  |
| clips/real/supanut/S1/_unknown_/phone_01.wav | _unknown_ | forward | forward 0.65 | _unknown_ 0.16 | stop 0.12 | 21.41 |  |
| clips/real/supanut/S1/grab/grab_05.wav | grab | right | right 0.78 | left 0.19 | grab 0.01 | 26.09 |  |
| clips/real/supanut/S1/place/place_07.wav | place | _unknown_ | _unknown_ 0.22 | right 0.21 | yes 0.20 | 20.44 |  |
| clips/real/supanut/S1/grab/grab_06.wav | grab | right | right 0.46 | left 0.43 | backward 0.09 | 25.19 |  |
| clips/real/supanut/S1/_unknown_/black_01.wav | _unknown_ | left | left 0.61 | backward 0.21 | yes 0.09 | 18.92 |  |
| clips/real/supanut/S1/place/place_08.wav | place | _unknown_ | _unknown_ 0.53 | place 0.22 | yes 0.11 | 25.62 |  |
| clips/real/supanut/S1/blue_box/blue_box_05.wav | blue box | no | no 0.72 | _unknown_ 0.10 | yes 0.08 | 23.25 |  |
| clips/real/supanut/S1/_unknown_/box_01.wav | _unknown_ | no | no 0.46 | backward 0.15 | yes 0.14 | 20.34 |  |
| clips/real/supanut/S1/home/home_03.wav | home | forward | forward 0.88 | _unknown_ 0.05 | stop 0.03 | 22.38 |  |
| clips/real/supanut/S1/black_box/black_box_07.wav | black box | backward | backward 0.67 | no 0.25 | left 0.04 | 21.07 |  |
| clips/real/supanut/S1/hand/hand_04.wav | hand | _unknown_ | _unknown_ 0.34 | hand 0.30 | no 0.25 | 19.54 |  |
| clips/real/supanut/S1/red_box/red_box_07.wav | red box | backward | backward 0.49 | _unknown_ 0.48 | left 0.01 | 20.62 |  |
| clips/real/supanut/S1/blue_box/blue_box_06.wav | blue box | no | no 0.82 | yes 0.13 | _unknown_ 0.03 | 23.83 |  |
| clips/real/supanut/S1/_unknown_/mood_01.wav | _unknown_ | forward | forward 0.46 | no 0.21 | backward 0.11 | 20.3 |  |
| clips/real/supanut/S1/green_box/green_box_05.wav | green box | _unknown_ | _unknown_ 0.76 | green box 0.09 | yes 0.07 | 21.68 |  |
| clips/real/supanut/S1/base/base_07.wav | base | yes | yes 0.35 | _unknown_ 0.33 | place 0.09 | 19.16 |  |
| clips/real/supanut/S1/home/home_04.wav | home | forward | forward 0.75 | _unknown_ 0.09 | no 0.06 | 21.45 |  |
| clips/real/supanut/S1/_unknown_/blue_01.wav | _unknown_ | no | no 0.98 | _unknown_ 0.01 | backward 0.01 | 21.44 |  |
| clips/real/supanut/S1/green_box/green_box_06.wav | green box | _unknown_ | _unknown_ 0.76 | green box 0.15 | right 0.05 | 19.76 |  |
| clips/real/supanut/S1/grab/grab_07.wav | grab | right | right 0.80 | left 0.12 | backward 0.04 | 23.84 |  |
| clips/real/supanut/S1/green_box/green_box_07.wav | green box | _unknown_ | _unknown_ 0.77 | green box 0.14 | right 0.05 | 20.75 |  |
| clips/real/supanut/S1/move/move_06.wav | move | no | no 0.30 | backward 0.30 | left 0.15 | 24.06 |  |
| clips/real/supanut/S1/hand/hand_05.wav | hand | _unknown_ | _unknown_ 0.44 | hand 0.26 | no 0.16 | 19.31 |  |
| clips/real/supanut/S1/move/move_07.wav | move | no | no 0.56 | backward 0.21 | left 0.08 | 23.14 |  |
| clips/real/supanut/S1/home/home_05.wav | home | forward | forward 0.74 | _unknown_ 0.10 | no 0.06 | 20.09 |  |
| clips/real/supanut/S1/pick/pick_04.wav | pick | right | right 0.26 | left 0.22 | backward 0.20 | 18.17 |  |
| clips/real/supanut/S1/grab/grab_08.wav | grab | right | right 0.53 | left 0.39 | backward 0.06 | 22.95 | other sound outside the clip (2 words? click?) |
| clips/real/supanut/S1/hand/hand_06.wav | hand | _unknown_ | _unknown_ 0.44 | hand 0.18 | no 0.15 | 19.41 |  |
| clips/real/supanut/S1/home/home_06.wav | home | forward | forward 0.85 | _unknown_ 0.07 | stop 0.04 | 24.41 |  |
| clips/real/supanut/S1/blue_box/blue_box_07.wav | blue box | no | no 0.61 | yes 0.24 | left 0.07 | 23.12 |  |
| clips/real/supanut/S1/_unknown_/go_01.wav | _unknown_ | forward | forward 0.91 | _unknown_ 0.05 | no 0.02 | 24.55 |  |
| clips/real/supanut/S1/hand/hand_08.wav | hand | _unknown_ | _unknown_ 0.40 | hand 0.30 | no 0.10 | 21.15 |  |
| clips/real/supanut/S1/_unknown_/write_01.wav | _unknown_ | right | right 0.99 | left 0.01 | red box 0.00 | 19.15 |  |
| clips/real/supanut/S1/drop/drop_07.wav | drop | stop | stop 0.84 | drop 0.08 | no 0.04 | 22.74 |  |
| clips/real/supanut/S1/home/home_07.wav | home | forward | forward 0.76 | _unknown_ 0.11 | home 0.07 | 19.94 |  |
| clips/real/supanut/S1/pick/pick_05.wav | pick | left | left 0.36 | backward 0.29 | _unknown_ 0.10 | 12.02 |  |
| clips/real/supanut/S1/green_box/green_box_08.wav | green box | _unknown_ | _unknown_ 0.88 | green box 0.06 | yes 0.03 | 22.0 |  |
| clips/real/supanut/S1/pick/pick_06.wav | pick | left | left 0.36 | backward 0.25 | _unknown_ 0.14 | 15.07 |  |
| clips/real/supanut/S1/base/base_08.wav | base | _unknown_ | _unknown_ 0.46 | yes 0.17 | right 0.10 | 16.57 |  |
| clips/real/supanut/S1/red_box/red_box_08.wav | red box | backward | backward 0.56 | _unknown_ 0.26 | right 0.12 | 20.13 |  |
| clips/real/supanut/S1/home/home_08.wav | home | forward | forward 0.29 | _unknown_ 0.19 | right 0.16 | 18.85 |  |
| clips/real/supanut/S1/blue_box/blue_box_08.wav | blue box | no | no 0.50 | yes 0.42 | _unknown_ 0.05 | 23.06 |  |
| clips/real/supanut/S1/pick/pick_07.wav | pick | _unknown_ | _unknown_ 0.29 | left 0.27 | backward 0.25 | 19.22 |  |
| clips/real/supanut/S1/drop/drop_08.wav | drop | no | no 0.31 | stop 0.28 | _unknown_ 0.17 | 19.3 |  |
| clips/real/supanut/S1/home/home_09.wav | home | forward | forward 0.24 | _unknown_ 0.17 | no 0.16 | 15.74 | sound longer than 1 s - the clip cuts it |
| clips/real/supanut/S1/_unknown_/open_01.wav | _unknown_ | forward | forward 0.64 | _unknown_ 0.27 | stop 0.03 | 21.59 |  |
| clips/real/supanut/S1/black_box/black_box_08.wav | black box | backward | backward 0.82 | left 0.09 | no 0.07 | 19.33 |  |
| clips/real/supanut/S1/pick/pick_08.wav | pick | backward | backward 0.29 | _unknown_ 0.23 | right 0.16 | 19.78 |  |
| clips/real/supanut/S1/hand/hand_09.wav | hand | _unknown_ | _unknown_ 0.36 | hand 0.27 | no 0.16 | 19.05 |  |
| clips/real/supanut/S1/pick/pick_09.wav | pick | left | left 0.32 | backward 0.28 | _unknown_ 0.16 | 18.1 |  |
| clips/real/supanut/S1/pick/pick_10.wav | pick | backward | backward 0.34 | left 0.27 | _unknown_ 0.14 | 14.91 |  |
| clips/real/supanut/S1/home/home_10.wav | home | forward | forward 0.84 | _unknown_ 0.06 | stop 0.05 | 22.93 |  |
| clips/real/supanut/S1/station/station_07.wav | station | _unknown_ | _unknown_ 0.65 | station 0.19 | right 0.06 | 19.02 |  |
| clips/real/supanut/S1/_unknown_/hope_01.wav | _unknown_ | forward | forward 0.62 | stop 0.16 | _unknown_ 0.08 | 19.75 |  |
| clips/real/supanut/S1/grab/grab_09.wav | grab | right | right 0.88 | left 0.11 | _unknown_ 0.00 | 22.29 |  |
| clips/real/supanut/S1/_unknown_/grass_01.wav | _unknown_ | right | right 0.53 | left 0.41 | drop 0.02 | 20.04 |  |
| clips/real/supanut/S1/grab/grab_10.wav | grab | right | right 0.94 | left 0.03 | backward 0.01 | 20.07 |  |
| clips/real/supanut/S1/_unknown_/bag_01.wav | _unknown_ | backward | backward 0.75 | left 0.21 | _unknown_ 0.02 | 17.07 |  |
| clips/real/supanut/S1/green_box/green_box_09.wav | green box | _unknown_ | _unknown_ 0.73 | green box 0.14 | right 0.11 | 21.07 |  |
| clips/real/supanut/S1/drop/drop_09.wav | drop | stop | stop 0.42 | no 0.19 | _unknown_ 0.17 | 17.06 | sound longer than 1 s - the clip cuts it |
| clips/real/supanut/S1/base/base_09.wav | base | yes | yes 0.65 | _unknown_ 0.12 | left 0.08 | 16.52 |  |
| clips/real/supanut/S1/black_box/black_box_09.wav | black box | no | no 0.49 | backward 0.46 | _unknown_ 0.03 | 19.23 |  |
| clips/real/supanut/S1/_unknown_/more_01.wav | _unknown_ | no | no 0.86 | backward 0.06 | _unknown_ 0.04 | 21.86 |  |
| clips/real/supanut/S1/move/move_08.wav | move | no | no 0.38 | left 0.28 | backward 0.18 | 16.54 |  |
| clips/real/supanut/S1/base/base_10.wav | base | yes | yes 0.57 | _unknown_ 0.16 | place 0.11 | 14.71 | sound longer than 1 s - the clip cuts it |
| clips/real/supanut/S1/blue_box/blue_box_09.wav | blue box | no | no 0.52 | yes 0.35 | _unknown_ 0.07 | 20.81 |  |
| clips/real/supanut/S1/blue_box/blue_box_10.wav | blue box | no | no 0.74 | yes 0.15 | _unknown_ 0.04 | 23.44 |  |
| clips/real/supanut/S1/_unknown_/foam_01.wav | _unknown_ | forward | forward 0.78 | _unknown_ 0.11 | stop 0.06 | 23.57 |  |
| clips/real/supanut/S1/green_box/green_box_10.wav | green box | _unknown_ | _unknown_ 0.95 | green box 0.03 | no 0.01 | 21.72 |  |
| clips/real/supanut/S1/_unknown_/up_01.wav | _unknown_ | stop | stop 0.71 | no 0.13 | backward 0.08 | 17.26 |  |
| clips/real/supanut/S1/_unknown_/robot_01.wav | _unknown_ | no | no 0.44 | _unknown_ 0.29 | backward 0.14 | 20.02 | sound longer than 1 s - the clip cuts it |
| clips/real/supanut/S1/red_box/red_box_09.wav | red box | _unknown_ | _unknown_ 0.62 | backward 0.31 | right 0.03 | 20.68 |  |
| clips/real/supanut/S1/red_box/red_box_10.wav | red box | _unknown_ | _unknown_ 0.75 | backward 0.21 | left 0.01 | 21.75 |  |
| clips/real/supanut/S1/drop/drop_10.wav | drop | stop | stop 0.49 | no 0.19 | _unknown_ 0.15 | 20.38 |  |
| clips/real/supanut/S1/_unknown_/ride_01.wav | _unknown_ | right | right 0.87 | _unknown_ 0.06 | backward 0.03 | 23.37 |  |
| clips/real/supanut/S1/move/move_09.wav | move | _silence_ | _silence_ 0.30 | right 0.14 | no 0.14 | 17.11 | sound longer than 1 s - the clip cuts it |
| clips/real/supanut/S1/move/move_10.wav | move | left | left 0.32 | no 0.30 | yes 0.14 | 14.01 |  |
| clips/real/supanut/S1/black_box/black_box_10.wav | black box | no | no 0.65 | backward 0.24 | _unknown_ 0.05 | 15.47 |  |

## Stream mode (raw 2 s takes, board logic: stride 10 frames, avg 3, threshold 0.7, refractory 1500 ms, channel auto)

command takes 200: **detected 0.38** (hit 76, hit+extra 0), wrong 45, miss 79 · first detection correct 0.38 · _unknown_ false alarms 15/30 · noise false alarms 0 in 1.0 min · channels {'ch0': 230}

| cls | takes | hit | hit_extra | wrong | miss | false_alarm | detected_rate | wrong_as | mean_max_p_true |
|---|---|---|---|---|---|---|---|---|---|
| move | 10 | 0 | 0 | 2 | 8 | 0 | 0.0 | no 2 | 0.004 |
| pick | 10 | 0 | 0 | 0 | 10 | 0 | 0.0 |  | 0.157 |
| grab | 10 | 0 | 0 | 7 | 3 | 0 | 0.0 | right 7 | 0.002 |
| blue box | 10 | 0 | 0 | 7 | 3 | 0 | 0.0 | no 7 | 0.013 |
| green box | 10 | 0 | 0 | 0 | 10 | 0 | 0.0 |  | 0.134 |
| red box | 10 | 0 | 0 | 7 | 3 | 0 | 0.0 | backward 5, right 2 | 0.042 |
| black box | 10 | 0 | 0 | 10 | 0 | 0 | 0.0 | backward 10 | 0.008 |
| hand | 10 | 0 | 0 | 0 | 10 | 0 | 0.0 |  | 0.391 |
| station | 10 | 7 | 0 | 0 | 3 | 0 | 0.7 |  | 0.75 |
| base | 10 | 0 | 0 | 0 | 10 | 0 | 0.0 |  | 0.23 |
| forward | 10 | 10 | 0 | 0 | 0 | 0 | 1.0 |  | 0.878 |
| backward | 10 | 10 | 0 | 0 | 0 | 0 | 1.0 |  | 0.996 |
| left | 10 | 8 | 0 | 0 | 2 | 0 | 0.8 |  | 0.835 |
| right | 10 | 9 | 0 | 1 | 0 | 0 | 0.9 | left 1 | 0.962 |
| place | 10 | 1 | 0 | 0 | 9 | 0 | 0.1 |  | 0.36 |
| drop | 10 | 1 | 0 | 1 | 8 | 0 | 0.1 | stop 1 | 0.319 |
| yes | 10 | 10 | 0 | 0 | 0 | 0 | 1.0 |  | 0.948 |
| no | 10 | 10 | 0 | 0 | 0 | 0 | 1.0 |  | 0.989 |
| home | 10 | 0 | 0 | 10 | 0 | 0 | 0.0 | forward 10 | 0.011 |
| stop | 10 | 10 | 0 | 0 | 0 | 0 | 1.0 |  | 0.992 |
| _unknown_ | 30 | 0 | 0 | 0 | 0 | 15 |  | forward 5, no 4, right 4 | 0.123 |

### Takes that were not a clean hit

| raw_path | label | outcome | detections | max_avg_p_true | peak_top3 | channels_used |
|---|---|---|---|---|---|---|
| raw/real/supanut/S1/pick/pick_01_raw.wav | pick | miss |  | 0.548 | pick 0.55 | left 0.14 | _unknown_ 0.13 | ch0 |
| raw/real/supanut/S1/base/base_01_raw.wav | base | miss |  | 0.268 | place 0.47 | base 0.23 | _unknown_ 0.18 | ch0 |
| raw/real/supanut/S1/home/home_01_raw.wav | home | wrong | forward@1.01s 0.89 | 0.01 | forward 0.93 | _unknown_ 0.05 | home 0.01 | ch0 |
| raw/real/supanut/S1/base/base_02_raw.wav | base | miss |  | 0.447 | backward 0.52 | left 0.13 | _silence_ 0.08 | ch0 |
| raw/real/supanut/S1/hand/hand_01_raw.wav | hand | miss |  | 0.286 | _unknown_ 0.37 | hand 0.29 | no 0.09 | ch0 |
| raw/real/supanut/S1/move/move_01_raw.wav | move | miss |  | 0.002 | no 0.68 | backward 0.19 | left 0.05 | ch0 |
| raw/real/supanut/S1/black_box/black_box_01_raw.wav | black box | wrong | backward@1.01s 0.86 | 0.003 | backward 0.88 | no 0.06 | left 0.05 | ch0 |
| raw/real/supanut/S1/move/move_02_raw.wav | move | miss |  | 0.002 | no 0.60 | backward 0.20 | left 0.10 | ch0 |
| raw/real/supanut/S1/black_box/black_box_02_raw.wav | black box | wrong | backward@1.01s 0.85 | 0.025 | backward 0.85 | no 0.12 | left 0.03 | ch0 |
| raw/real/supanut/S1/blue_box/blue_box_01_raw.wav | blue box | wrong | no@1.01s 0.90 | 0.04 | no 0.90 | _unknown_ 0.03 | yes 0.03 | ch0 |
| raw/real/supanut/S1/red_box/red_box_01_raw.wav | red box | miss |  | 0.086 | _unknown_ 0.66 | backward 0.13 | right 0.10 | ch0 |
| raw/real/supanut/S1/grab/grab_01_raw.wav | grab | wrong | right@1.31s 0.72 | 0.0 | right 0.72 | left 0.27 | _unknown_ 0.00 | ch0 |
| raw/real/supanut/S1/place/place_02_raw.wav | place | miss |  | 0.363 | left 0.64 | yes 0.15 | _unknown_ 0.08 | ch0 |
| raw/real/supanut/S1/place/place_03_raw.wav | place | miss |  | 0.397 | place 0.40 | _unknown_ 0.19 | yes 0.18 | ch0 |
| raw/real/supanut/S1/hand/hand_02_raw.wav | hand | miss |  | 0.481 | hand 0.48 | _unknown_ 0.34 | no 0.06 | ch0 |
| raw/real/supanut/S1/drop/drop_02_raw.wav | drop | miss |  | 0.171 | left 0.33 | _unknown_ 0.14 | stop 0.13 | ch0 |
| raw/real/supanut/S1/place/place_04_raw.wav | place | miss |  | 0.166 | yes 0.36 | left 0.24 | _unknown_ 0.18 | ch0 |
| raw/real/supanut/S1/red_box/red_box_02_raw.wav | red box | wrong | backward@1.01s 0.94 | 0.024 | backward 0.94 | right 0.04 | _unknown_ 0.02 | ch0 |
| raw/real/supanut/S1/_unknown_/hold_01_raw.wav | _unknown_ | false_alarm | forward@1.01s 0.83 | 0.086 | forward 0.93 | _unknown_ 0.04 | backward 0.01 | ch0 |
| raw/real/supanut/S1/green_box/green_box_01_raw.wav | green box | miss |  | 0.043 | _unknown_ 0.78 | right 0.16 | yes 0.02 | ch0 |
| raw/real/supanut/S1/place/place_05_raw.wav | place | miss |  | 0.377 | left 0.50 | right 0.20 | yes 0.11 | ch0 |
| raw/real/supanut/S1/green_box/green_box_02_raw.wav | green box | miss |  | 0.156 | _unknown_ 0.79 | right 0.14 | green box 0.03 | ch0 |
| raw/real/supanut/S1/drop/drop_03_raw.wav | drop | miss |  | 0.16 | stop 0.55 | no 0.17 | _unknown_ 0.12 | ch0 |
| raw/real/supanut/S1/grab/grab_02_raw.wav | grab | miss |  | 0.002 | left 0.61 | right 0.38 | _unknown_ 0.01 | ch0 |
| raw/real/supanut/S1/base/base_03_raw.wav | base | miss |  | 0.565 | base 0.57 | _unknown_ 0.22 | place 0.13 | ch0 |
| raw/real/supanut/S1/move/move_03_raw.wav | move | wrong | no@1.21s 0.71 | 0.008 | no 0.81 | backward 0.12 | _unknown_ 0.03 | ch0 |
| raw/real/supanut/S1/grab/grab_03_raw.wav | grab | wrong | right@1.01s 0.79 | 0.0 | right 0.79 | left 0.20 | _unknown_ 0.00 | ch0 |
| raw/real/supanut/S1/green_box/green_box_03_raw.wav | green box | miss |  | 0.141 | _unknown_ 0.83 | right 0.08 | green box 0.08 | ch0 |
| raw/real/supanut/S1/_unknown_/nose_01_raw.wav | _unknown_ | false_alarm | no@1.01s 0.92 | 0.036 | no 0.92 | backward 0.04 | _unknown_ 0.01 | ch0 |
| raw/real/supanut/S1/green_box/green_box_04_raw.wav | green box | miss |  | 0.154 | _unknown_ 0.92 | green box 0.06 | right 0.02 | ch0 |
| raw/real/supanut/S1/black_box/black_box_03_raw.wav | black box | wrong | backward@1.01s 0.80 | 0.001 | backward 0.80 | no 0.13 | left 0.05 | ch0 |
| raw/real/supanut/S1/red_box/red_box_03_raw.wav | red box | wrong | right@1.01s 0.87 | 0.029 | right 0.87 | _unknown_ 0.08 | backward 0.05 | ch0 |
| raw/real/supanut/S1/black_box/black_box_04_raw.wav | black box | wrong | backward@1.01s 0.97 | 0.001 | backward 0.97 | no 0.02 | left 0.01 | ch0 |
| raw/real/supanut/S1/base/base_04_raw.wav | base | miss |  | 0.055 | yes 0.54 | left 0.16 | _unknown_ 0.11 | ch0 |
| raw/real/supanut/S1/drop/drop_04_raw.wav | drop | miss |  | 0.688 | drop 0.69 | right 0.12 | _unknown_ 0.09 | ch0 |
| raw/real/supanut/S1/red_box/red_box_04_raw.wav | red box | wrong | backward@1.01s 0.91 | 0.024 | backward 0.91 | _unknown_ 0.07 | left 0.02 | ch0 |
| raw/real/supanut/S1/blue_box/blue_box_02_raw.wav | blue box | wrong | no@1.01s 0.91 | 0.013 | no 0.92 | _unknown_ 0.03 | backward 0.03 | ch0 |
| raw/real/supanut/S1/grab/grab_04_raw.wav | grab | miss |  | 0.002 | left 0.53 | right 0.36 | backward 0.06 | ch0 |
| raw/real/supanut/S1/hand/hand_03_raw.wav | hand | miss |  | 0.18 | _unknown_ 0.46 | no 0.23 | hand 0.12 | ch0 |
| raw/real/supanut/S1/blue_box/blue_box_03_raw.wav | blue box | wrong | no@1.01s 0.80 | 0.007 | no 0.80 | left 0.05 | yes 0.05 | ch0 |
| raw/real/supanut/S1/black_box/black_box_05_raw.wav | black box | wrong | backward@1.01s 0.93 | 0.001 | backward 0.93 | left 0.03 | no 0.03 | ch0 |
| raw/real/supanut/S1/red_box/red_box_05_raw.wav | red box | miss |  | 0.014 | _unknown_ 0.79 | right 0.11 | backward 0.04 | ch0 |
| raw/real/supanut/S1/base/base_05_raw.wav | base | miss |  | 0.4 | base 0.40 | place 0.20 | yes 0.19 | ch0 |
| raw/real/supanut/S1/black_box/black_box_06_raw.wav | black box | wrong | backward@1.01s 0.77 | 0.001 | backward 0.77 | no 0.12 | left 0.08 | ch0 |
| raw/real/supanut/S1/base/base_06_raw.wav | base | miss |  | 0.229 | _unknown_ 0.38 | backward 0.21 | yes 0.14 | ch0 |
| raw/real/supanut/S1/move/move_04_raw.wav | move | wrong | no@1.01s 0.71 | 0.003 | no 0.71 | backward 0.15 | _unknown_ 0.05 | ch0 |
| raw/real/supanut/S1/_unknown_/wait_01_raw.wav | _unknown_ | false_alarm | right@1.11s 0.73 | 0.085 | right 0.80 | backward 0.09 | left 0.05 | ch0 |
| raw/real/supanut/S1/_unknown_/now_01_raw.wav | _unknown_ | false_alarm | no@1.01s 0.94 | 0.022 | no 0.98 | _unknown_ 0.01 | backward 0.01 | ch0 |
| raw/real/supanut/S1/move/move_05_raw.wav | move | miss |  | 0.003 | no 0.37 | forward 0.22 | backward 0.14 | ch0 |
| raw/real/supanut/S1/_unknown_/bright_01_raw.wav | _unknown_ | false_alarm | left@1.01s 0.82 | 0.018 | right 0.96 | left 0.03 | _unknown_ 0.00 | ch0 |
| raw/real/supanut/S1/right/right_04_raw.wav | right | wrong | left@1.01s 0.70 | 0.998 | right 1.00 | left 0.00 | _unknown_ 0.00 | ch0 |
| raw/real/supanut/S1/red_box/red_box_06_raw.wav | red box | wrong | right@1.01s 0.77 | 0.06 | right 0.77 | backward 0.12 | _unknown_ 0.08 | ch0 |
| raw/real/supanut/S1/_unknown_/back_01_raw.wav | _unknown_ | false_alarm | backward@1.01s 0.78 | 0.02 | backward 0.86 | left 0.09 | no 0.03 | ch0 |
| raw/real/supanut/S1/pick/pick_02_raw.wav | pick | miss |  | 0.11 | left 0.48 | _unknown_ 0.15 | backward 0.12 | ch0 |
| raw/real/supanut/S1/drop/drop_05_raw.wav | drop | miss |  | 0.391 | right 0.49 | left 0.20 | drop 0.15 | ch0 |
| raw/real/supanut/S1/place/place_06_raw.wav | place | miss |  | 0.208 | _unknown_ 0.39 | yes 0.18 | right 0.14 | ch0 |
| raw/real/supanut/S1/drop/drop_06_raw.wav | drop | miss |  | 0.138 | stop 0.34 | _unknown_ 0.17 | no 0.14 | ch0 |
| raw/real/supanut/S1/home/home_02_raw.wav | home | wrong | forward@1.01s 0.72 | 0.008 | forward 0.82 | _unknown_ 0.08 | no 0.04 | ch0 |
| raw/real/supanut/S1/pick/pick_03_raw.wav | pick | miss |  | 0.169 | left 0.42 | _unknown_ 0.19 | right 0.15 | ch0 |
| raw/real/supanut/S1/_unknown_/grape_01_raw.wav | _unknown_ | false_alarm | right@1.01s 0.98 | 0.027 | right 0.98 | _unknown_ 0.01 | backward 0.00 | ch0 |
| raw/real/supanut/S1/blue_box/blue_box_04_raw.wav | blue box | wrong | no@1.51s 0.70 | 0.025 | no 0.70 | _unknown_ 0.12 | yes 0.08 | ch0 |
| raw/real/supanut/S1/_unknown_/phone_01_raw.wav | _unknown_ | false_alarm | forward@1.41s 0.71 | 0.189 | forward 0.71 | _unknown_ 0.14 | stop 0.08 | ch0 |
| raw/real/supanut/S1/grab/grab_05_raw.wav | grab | wrong | right@1.01s 0.88 | 0.009 | right 0.88 | left 0.10 | backward 0.01 | ch0 |
| raw/real/supanut/S1/place/place_07_raw.wav | place | miss |  | 0.228 | right 0.41 | _unknown_ 0.25 | left 0.17 | ch0 |
| raw/real/supanut/S1/grab/grab_06_raw.wav | grab | miss |  | 0.005 | right 0.69 | left 0.14 | backward 0.14 | ch0 |
| raw/real/supanut/S1/place/place_08_raw.wav | place | miss |  | 0.272 | _unknown_ 0.55 | place 0.13 | yes 0.12 | ch0 |
| raw/real/supanut/S1/blue_box/blue_box_05_raw.wav | blue box | wrong | no@1.01s 0.74 | 0.026 | no 0.74 | _unknown_ 0.08 | backward 0.08 | ch0 |
| raw/real/supanut/S1/left/left_05_raw.wav | left | miss |  | 0.674 | left 0.67 | yes 0.14 | backward 0.11 | ch0 |
| raw/real/supanut/S1/home/home_03_raw.wav | home | wrong | forward@1.01s 0.89 | 0.007 | forward 0.90 | _unknown_ 0.05 | stop 0.02 | ch0 |
| raw/real/supanut/S1/black_box/black_box_07_raw.wav | black box | wrong | backward@1.01s 0.94 | 0.009 | backward 0.94 | no 0.04 | left 0.02 | ch0 |
| raw/real/supanut/S1/hand/hand_04_raw.wav | hand | miss |  | 0.317 | _unknown_ 0.41 | hand 0.27 | no 0.15 | ch0 |
| raw/real/supanut/S1/red_box/red_box_07_raw.wav | red box | wrong | backward@1.01s 0.78 | 0.018 | backward 0.78 | _unknown_ 0.18 | left 0.02 | ch0 |
| raw/real/supanut/S1/blue_box/blue_box_06_raw.wav | blue box | wrong | no@1.01s 0.75 | 0.003 | no 0.78 | yes 0.09 | left 0.06 | ch0 |
| raw/real/supanut/S1/green_box/green_box_05_raw.wav | green box | miss |  | 0.07 | _unknown_ 0.77 | right 0.14 | yes 0.04 | ch0 |
| raw/real/supanut/S1/base/base_07_raw.wav | base | miss |  | 0.06 | _unknown_ 0.37 | yes 0.14 | left 0.14 | ch0 |
| raw/real/supanut/S1/home/home_04_raw.wav | home | wrong | forward@1.11s 0.73 | 0.008 | forward 0.80 | _unknown_ 0.08 | no 0.04 | ch0 |
| raw/real/supanut/S1/_unknown_/blue_01_raw.wav | _unknown_ | false_alarm | no@1.01s 0.95 | 0.038 | no 0.96 | backward 0.02 | _unknown_ 0.01 | ch0 |
| raw/real/supanut/S1/green_box/green_box_06_raw.wav | green box | miss |  | 0.149 | _unknown_ 0.86 | green box 0.07 | right 0.05 | ch0 |
| raw/real/supanut/S1/grab/grab_07_raw.wav | grab | wrong | right@1.01s 0.76 | 0.001 | right 0.85 | left 0.11 | _unknown_ 0.02 | ch0 |
| raw/real/supanut/S1/green_box/green_box_07_raw.wav | green box | miss |  | 0.244 | _unknown_ 0.77 | right 0.12 | green box 0.10 | ch0 |
| raw/real/supanut/S1/move/move_06_raw.wav | move | miss |  | 0.001 | left 0.31 | backward 0.24 | no 0.20 | ch0 |
| raw/real/supanut/S1/hand/hand_05_raw.wav | hand | miss |  | 0.411 | _unknown_ 0.56 | hand 0.21 | no 0.12 | ch0 |
| raw/real/supanut/S1/move/move_07_raw.wav | move | miss |  | 0.006 | no 0.70 | backward 0.16 | _unknown_ 0.05 | ch0 |
| raw/real/supanut/S1/home/home_05_raw.wav | home | wrong | forward@1.31s 0.76 | 0.022 | forward 0.76 | _unknown_ 0.11 | no 0.05 | ch0 |
| raw/real/supanut/S1/pick/pick_04_raw.wav | pick | miss |  | 0.243 | pick 0.24 | left 0.20 | backward 0.19 | ch0 |
| raw/real/supanut/S1/grab/grab_08_raw.wav | grab | wrong | right@1.01s 0.72 | 0.0 | right 0.73 | left 0.22 | backward 0.03 | ch0 |
| raw/real/supanut/S1/hand/hand_06_raw.wav | hand | miss |  | 0.241 | _unknown_ 0.45 | hand 0.23 | no 0.10 | ch0 |
| raw/real/supanut/S1/home/home_06_raw.wav | home | wrong | forward@1.01s 0.78 | 0.004 | forward 0.89 | _unknown_ 0.07 | stop 0.02 | ch0 |
| raw/real/supanut/S1/hand/hand_07_raw.wav | hand | miss |  | 0.596 | hand 0.60 | _unknown_ 0.31 | no 0.06 | ch0 |
| raw/real/supanut/S1/blue_box/blue_box_07_raw.wav | blue box | miss |  | 0.003 | no 0.62 | yes 0.18 | left 0.12 | ch0 |
| raw/real/supanut/S1/_unknown_/go_01_raw.wav | _unknown_ | false_alarm | forward@1.01s 0.72 | 0.157 | forward 0.89 | _unknown_ 0.07 | no 0.02 | ch0 |
| raw/real/supanut/S1/hand/hand_08_raw.wav | hand | miss |  | 0.468 | _unknown_ 0.47 | hand 0.16 | left 0.11 | ch0 |
| raw/real/supanut/S1/_unknown_/write_01_raw.wav | _unknown_ | false_alarm | right@1.01s 0.99 | 0.01 | right 0.99 | left 0.01 | _unknown_ 0.00 | ch0 |
| raw/real/supanut/S1/drop/drop_07_raw.wav | drop | wrong | stop@1.01s 0.75 | 0.191 | stop 0.76 | drop 0.14 | _unknown_ 0.04 | ch0 |
| raw/real/supanut/S1/home/home_07_raw.wav | home | wrong | forward@1.01s 0.75 | 0.026 | forward 0.85 | _unknown_ 0.08 | stop 0.03 | ch0 |
| raw/real/supanut/S1/pick/pick_05_raw.wav | pick | miss |  | 0.043 | left 0.44 | backward 0.15 | _silence_ 0.13 | ch0 |
| raw/real/supanut/S1/green_box/green_box_08_raw.wav | green box | miss |  | 0.072 | _unknown_ 0.91 | right 0.03 | green box 0.02 | ch0 |
| raw/real/supanut/S1/pick/pick_06_raw.wav | pick | miss |  | 0.069 | left 0.38 | backward 0.27 | _unknown_ 0.14 | ch0 |
| raw/real/supanut/S1/base/base_08_raw.wav | base | miss |  | 0.135 | _unknown_ 0.51 | backward 0.14 | yes 0.14 | ch0 |
| raw/real/supanut/S1/red_box/red_box_08_raw.wav | red box | wrong | backward@1.01s 0.88 | 0.053 | backward 0.88 | right 0.07 | _unknown_ 0.04 | ch0 |
| raw/real/supanut/S1/left/left_06_raw.wav | left | miss |  | 0.621 | left 0.62 | yes 0.21 | backward 0.08 | ch0 |
| raw/real/supanut/S1/home/home_08_raw.wav | home | wrong | forward@1.01s 0.89 | 0.016 | forward 0.91 | _unknown_ 0.05 | home 0.01 | ch0 |
| raw/real/supanut/S1/blue_box/blue_box_08_raw.wav | blue box | miss |  | 0.005 | no 0.63 | yes 0.24 | left 0.07 | ch0 |
| raw/real/supanut/S1/pick/pick_07_raw.wav | pick | miss |  | 0.061 | left 0.43 | _unknown_ 0.16 | backward 0.14 | ch0 |
| raw/real/supanut/S1/drop/drop_08_raw.wav | drop | miss |  | 0.053 | no 0.26 | _unknown_ 0.20 | backward 0.18 | ch0 |
| raw/real/supanut/S1/home/home_09_raw.wav | home | wrong | forward@1.01s 0.74 | 0.002 | forward 0.83 | _unknown_ 0.06 | stop 0.06 | ch0 |
| raw/real/supanut/S1/black_box/black_box_08_raw.wav | black box | wrong | backward@1.01s 0.83 | 0.03 | backward 0.83 | no 0.10 | left 0.06 | ch0 |
| raw/real/supanut/S1/pick/pick_08_raw.wav | pick | miss |  | 0.132 | left 0.27 | right 0.21 | backward 0.18 | ch0 |
| raw/real/supanut/S1/hand/hand_09_raw.wav | hand | miss |  | 0.446 | hand 0.45 | _unknown_ 0.30 | no 0.12 | ch0 |
| raw/real/supanut/S1/pick/pick_09_raw.wav | pick | miss |  | 0.094 | _unknown_ 0.33 | left 0.22 | backward 0.18 | ch0 |
| raw/real/supanut/S1/pick/pick_10_raw.wav | pick | miss |  | 0.103 | left 0.36 | backward 0.23 | _unknown_ 0.10 | ch0 |
| raw/real/supanut/S1/home/home_10_raw.wav | home | wrong | forward@1.01s 0.86 | 0.006 | forward 0.86 | _unknown_ 0.06 | stop 0.05 | ch0 |
| raw/real/supanut/S1/station/station_07_raw.wav | station | miss |  | 0.358 | _unknown_ 0.65 | station 0.14 | stop 0.08 | ch0 |
| raw/real/supanut/S1/_unknown_/hope_01_raw.wav | _unknown_ | false_alarm | forward@1.11s 0.72 | 0.09 | forward 0.73 | stop 0.11 | _unknown_ 0.08 | ch0 |
| raw/real/supanut/S1/grab/grab_09_raw.wav | grab | wrong | right@1.01s 0.96 | 0.0 | right 0.96 | left 0.02 | _unknown_ 0.00 | ch0 |
| raw/real/supanut/S1/hand/hand_10_raw.wav | hand | miss |  | 0.482 | hand 0.48 | _unknown_ 0.34 | no 0.11 | ch0 |
| raw/real/supanut/S1/grab/grab_10_raw.wav | grab | wrong | right@1.01s 0.91 | 0.005 | right 0.92 | left 0.05 | backward 0.01 | ch0 |
| raw/real/supanut/S1/green_box/green_box_09_raw.wav | green box | miss |  | 0.136 | _unknown_ 0.73 | right 0.14 | green box 0.11 | ch0 |
| raw/real/supanut/S1/drop/drop_09_raw.wav | drop | miss |  | 0.237 | stop 0.31 | no 0.16 | _unknown_ 0.14 | ch0 |
| raw/real/supanut/S1/base/base_09_raw.wav | base | miss |  | 0.069 | yes 0.50 | _unknown_ 0.24 | place 0.13 | ch0 |
| raw/real/supanut/S1/black_box/black_box_09_raw.wav | black box | wrong | backward@1.01s 0.74 | 0.002 | backward 0.74 | no 0.25 | left 0.01 | ch0 |
| raw/real/supanut/S1/place/place_09_raw.wav | place | miss |  | 0.501 | place 0.50 | right 0.17 | _unknown_ 0.16 | ch0 |
| raw/real/supanut/S1/_unknown_/more_01_raw.wav | _unknown_ | false_alarm | no@1.31s 0.72 | 0.052 | no 0.77 | backward 0.10 | _unknown_ 0.05 | ch0 |
| raw/real/supanut/S1/move/move_08_raw.wav | move | miss |  | 0.003 | no 0.59 | left 0.15 | backward 0.11 | ch0 |
| raw/real/supanut/S1/base/base_10_raw.wav | base | miss |  | 0.072 | yes 0.56 | _unknown_ 0.21 | backward 0.08 | ch0 |
| raw/real/supanut/S1/blue_box/blue_box_09_raw.wav | blue box | wrong | no@1.01s 0.78 | 0.004 | no 0.78 | yes 0.07 | left 0.05 | ch0 |
| raw/real/supanut/S1/blue_box/blue_box_10_raw.wav | blue box | miss |  | 0.007 | no 0.67 | yes 0.18 | left 0.09 | ch0 |
| raw/real/supanut/S1/_unknown_/foam_01_raw.wav | _unknown_ | false_alarm | forward@1.01s 0.70 | 0.156 | forward 0.84 | _unknown_ 0.09 | stop 0.05 | ch0 |
| raw/real/supanut/S1/place/place_10_raw.wav | place | miss |  | 0.378 | place 0.38 | right 0.20 | _unknown_ 0.16 | ch0 |
| raw/real/supanut/S1/green_box/green_box_10_raw.wav | green box | miss |  | 0.175 | _unknown_ 0.96 | green box 0.03 | right 0.00 | ch0 |
| raw/real/supanut/S1/station/station_09_raw.wav | station | miss |  | 0.616 | station 0.62 | _unknown_ 0.33 | base 0.02 | ch0 |
| raw/real/supanut/S1/red_box/red_box_09_raw.wav | red box | miss |  | 0.105 | _unknown_ 0.69 | backward 0.24 | red box 0.03 | ch0 |
| raw/real/supanut/S1/red_box/red_box_10_raw.wav | red box | wrong | backward@1.01s 0.82 | 0.006 | backward 0.82 | _unknown_ 0.17 | left 0.01 | ch0 |
| raw/real/supanut/S1/drop/drop_10_raw.wav | drop | miss |  | 0.253 | stop 0.36 | no 0.21 | drop 0.14 | ch0 |
| raw/real/supanut/S1/_unknown_/ride_01_raw.wav | _unknown_ | false_alarm | right@1.01s 0.87 | 0.043 | right 0.94 | _unknown_ 0.03 | left 0.02 | ch0 |
| raw/real/supanut/S1/move/move_09_raw.wav | move | miss |  | 0.007 | no 0.67 | backward 0.12 | left 0.07 | ch0 |
| raw/real/supanut/S1/station/station_10_raw.wav | station | miss |  | 0.615 | _unknown_ 0.66 | station 0.18 | base 0.05 | ch0 |
| raw/real/supanut/S1/move/move_10_raw.wav | move | miss |  | 0.002 | left 0.45 | no 0.20 | yes 0.11 | ch0 |
| raw/real/supanut/S1/black_box/black_box_10_raw.wav | black box | wrong | backward@1.01s 0.73 | 0.002 | backward 0.73 | no 0.23 | _unknown_ 0.02 | ch0 |

### Noise recordings

| file | duration_s | inferences | false_alarms | top_classes | detections |
|---|---|---|---|---|---|
| raw/noise/board_room/supanut_S1_room.wav | 60.0 | 590 | 0 | _silence_ 590 |  |
