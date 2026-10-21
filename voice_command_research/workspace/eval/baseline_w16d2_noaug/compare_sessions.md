# Session comparison - baseline_w16d2_noaug

| metric | S1 | S0 |
|---|---|---|
| clip n | 290 | 109 |
| clip accuracy | 0.5414 | 0.4495 |
| clip macro_f1 | 0.3801 | 0.411 |
| clip command_accuracy | 0.47 | 0.5106 |
| clip unknown_accuracy | 0.1 | 0.0667 |
| clip speech_peak_db_median | -34.7 | -32.4 |
| stream detected_rate | 0.38 | 0.1277 |
| stream first_detection_correct | 0.38 | 0.1277 |
| stream wrong | 45 | 37 |
| stream miss | 79 | 45 |
| stream unknown_false_alarms | 15 | 11 |

## Per class: clip recall · stream detected (hit / takes)

| class | S1 | S0 |
|---|---|---|
| move | 0/10 · 0/10 | 0/5 · 0/5 |
| pick | 1/10 · 0/10 | 2/2 · 0/2 |
| grab | 0/10 · 0/10 | 0/7 · 0/7 |
| blue box | 0/10 · 0/10 | 0/7 · 0/7 |
| green box | 0/10 · 0/10 | 1/6 · 0/6 |
| red box | 0/10 · 0/10 | 0/3 · 0/3 |
| black box | 0/10 · 0/10 | 0/4 · 0/4 |
| hand | 2/10 · 0/10 | 4/5 · 1/5 |
| station | 9/10 · 7/10 | 6/6 · 2/6 |
| base | 3/10 · 0/10 | 2/5 · 0/5 |
| forward | 10/10 · 10/10 | 4/4 · 2/4 |
| backward | 10/10 · 10/10 | 7/7 · 1/7 |
| left | 10/10 · 8/10 | 5/5 · 1/5 |
| right | 10/10 · 9/10 | 3/3 · 0/3 |
| place | 7/10 · 1/10 | 3/7 · 0/7 |
| drop | 2/10 · 1/10 | 0/2 · 0/2 |
| yes | 10/10 · 10/10 | 1/2 · 2/2 |
| no | 10/10 · 10/10 | 4/4 · 1/4 |
| home | 0/10 · 0/10 | 1/5 · 0/5 |
| stop | 10/10 · 10/10 | 5/5 · 2/5 |
| _unknown_ | 3/30 | 1/15 |
| _silence_ | 60/60 | - |
