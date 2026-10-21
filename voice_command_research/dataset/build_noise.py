"""
build_noise.py - noise and room-echo (RIR) tables with folds, for the window builder.

    C:/Users/ASUS/anaconda3/envs/torch_env/python.exe build_noise.py --limit 20    quick test
    C:/Users/ASUS/anaconda3/envs/torch_env/python.exe build_noise.py               full tables

Sources (C.OPEN_NOISE, outside git):
    ESC-50 (20 indoor categories)      converted once 44.1 kHz -> 16 kHz into ESC-50-16k\
    OpenSLR 28 point-source noises     group = file
    OpenSLR 28 isotropic noises        group = room (--room-linkage), channel 0
    OpenSLR 28 real RIRs               group = room (--room-id), channel 0
    OpenSLR 28 simulated small rooms   group = room (Room001 ... Room200)
Board noise recordings (servo, fan, ...) are added here once S2 exists; S1 room noise never (test set).

Every group gets one fold 0..4: training windows of fold k use noise / RIRs from the other folds,
held-out windows of fold k only from fold k, so no noise recording or room crosses folds.
Output: built\noise.csv, built\rirs.csv
"""
import argparse
import csv
import random
import sys
from collections import Counter
from pathlib import Path

import librosa
import numpy as np
import soundfile as sf

sys.path.insert(0, str(Path(__file__).resolve().parent.parent))
import config as C

RIRS = C.OPEN_NOISE / "RIRS_NOISES"
ESC_DIR = C.OPEN_NOISE / "ESC-50-master"
ESC_16K = C.OPEN_NOISE / "ESC-50-16k"
NOISE_CSV = C.BUILT / "noise.csv"
RIR_CSV = C.BUILT / "rirs.csv"
N_FOLDS = 5
MIN_NOISE_S = 1.1                       # shorter noise files are left out (window is 1.025 s)
SEED = 42

NOISE_FIELDS = ["path", "kind", "group", "duration_s", "fold"]
RIR_FIELDS = ["path", "kind", "group", "fold"]


def kaldi_list(path):
    """--key value ... <path> lines -> list of dicts (paths made absolute)"""
    rows = []
    for line in Path(path).read_text().split("\n"):
        parts = line.split()
        if not parts:
            continue
        row = {parts[i].lstrip("-"): parts[i + 1] for i in range(0, len(parts) - 1, 2)}
        row["path"] = str(C.OPEN_NOISE / parts[-1])
        rows.append(row)
    return rows


def esc50_rows(limit):
    """convert the extracted ESC-50 clips to 16 kHz once; group = original freesound clip"""
    meta = list(csv.DictReader(open(ESC_DIR / "meta" / "esc50.csv", encoding="utf-8")))
    rows = []
    for r in meta:
        src, dst = ESC_DIR / "audio" / r["filename"], ESC_16K / r["filename"]
        if not src.exists() and not dst.exists():
            continue                              # category not extracted
        if not dst.exists():
            audio, sr = sf.read(src, dtype="float32")
            dst.parent.mkdir(parents=True, exist_ok=True)
            sf.write(dst, librosa.resample(audio, orig_sr=sr, target_sr=C.SR), C.SR, subtype="PCM_16")
        rows.append(dict(path=str(dst), kind=f"esc50_{r['category']}", group=f"esc50_{r['src_file']}",
                         duration_s=round(sf.info(dst).duration, 2)))   # ESC-50's own folds split 2 sources
        if limit and len(rows) >= limit:
            break
    return rows


def openslr_noise_rows(limit):
    rows = []
    for r in kaldi_list(RIRS / "pointsource_noises" / "noise_list")[:limit or None]:
        rows.append(dict(path=r["path"], kind=f"pointsource_{r['bg-fg-type']}", group=r["noise-id"],
                         duration_s=round(sf.info(r["path"]).duration, 2)))
    for r in kaldi_list(RIRS / "real_rirs_isotropic_noises" / "noise_list")[:limit or None]:
        rows.append(dict(path=r["path"], kind="isotropic", group=r["room-linkage"],
                         duration_s=round(sf.info(r["path"]).duration, 2)))
    return [r for r in rows if r["duration_s"] >= MIN_NOISE_S]


def rir_rows(limit):
    rows = [dict(path=r["path"], kind="real", group=r["room-id"])
            for r in kaldi_list(RIRS / "real_rirs_isotropic_noises" / "rir_list")[:limit or None]]
    rows += [dict(path=r["path"], kind="sim_small", group=r["room-id"])
             for r in kaldi_list(RIRS / "simulated_rirs" / "smallroom" / "rir_list")[:limit or None]]
    return rows


def assign_group_folds(rows):
    """shuffle the groups of each kind and deal them round-robin over the folds;
    a group that also appears under another kind keeps the fold it already has"""
    rng = random.Random(SEED)
    fold_of = {}
    for kind in sorted({r["kind"] for r in rows}):
        groups = sorted({r["group"] for r in rows if r["kind"] == kind} - set(fold_of))
        rng.shuffle(groups)
        fold_of.update({g: i % N_FOLDS for i, g in enumerate(groups)})
    for r in rows:
        r["fold"] = fold_of[r["group"]]
    return rows


def write(path, fields, rows):
    path.parent.mkdir(parents=True, exist_ok=True)
    with open(path, "w", newline="", encoding="utf-8") as f:
        writer = csv.DictWriter(f, fieldnames=fields)
        writer.writeheader()
        writer.writerows(rows)


def check_groups(rows, name):
    folds_per_group = Counter()
    for g in {r["group"] for r in rows}:
        folds_per_group[len({r["fold"] for r in rows if r["group"] == g})] += 1
    leaks = sum(v for k, v in folds_per_group.items() if k > 1)
    print(f"{name}: groups in more than one fold {leaks}")
    if leaks:
        raise SystemExit("LEAK - tables not written")


def summary(noise, rirs):
    print(f"\n{'noise kind':28s} {'files':>6s} {'groups':>6s} {'minutes':>8s}  per fold (files)")
    kinds = sorted({r["kind"] for r in noise}, key=lambda k: (not k.startswith("esc50"), k))
    for kind in kinds:
        r = [x for x in noise if x["kind"] == kind]
        folds = Counter(x["fold"] for x in r)
        print(f"{kind:28s} {len(r):6d} {len({x['group'] for x in r}):6d} "
              f"{sum(x['duration_s'] for x in r) / 60:8.1f}  {[folds[f] for f in range(N_FOLDS)]}")
    print(f"\n{'rir kind':12s} {'files':>6s} {'rooms':>6s}  per fold (rooms)")
    for kind in ("real", "sim_small"):
        r = [x for x in rirs if x["kind"] == kind]
        folds = Counter(fold for _, fold in {(x["group"], x["fold"]) for x in r})
        print(f"{kind:12s} {len(r):6d} {len({x['group'] for x in r}):6d}  {[folds[f] for f in range(N_FOLDS)]}")


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--limit", type=int, help="quick test: first N rows per list, nothing written")
    args = ap.parse_args()
    noise = assign_group_folds(esc50_rows(args.limit) + openslr_noise_rows(args.limit))
    rirs = assign_group_folds(rir_rows(args.limit))
    check_groups(noise, "noise")
    check_groups(rirs, "rirs")
    summary(noise, rirs)
    if args.limit:
        print("\n--limit: nothing written")
        return
    write(NOISE_CSV, NOISE_FIELDS, noise)
    write(RIR_CSV, RIR_FIELDS, rirs)
    print(f"\n-> {NOISE_CSV}\n-> {RIR_CSV}")


if __name__ == "__main__":
    main()
