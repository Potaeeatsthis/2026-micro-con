"""
cache_audio.py - put all training audio into a few big arrays so training does not read
130k small files every epoch (the arrays are opened with np.load(mmap_mode="r")).

    C:/Users/ASUS/anaconda3/envs/torch_env/python.exe cache_audio.py --limit 500    quick test
    C:/Users/ASUS/anaconda3/envs/torch_env/python.exe cache_audio.py                full cache

built\cache\
    clips.npy  int16   word + MARGIN_S each side of every kept train / dev clip (whole clip for _silence_)
    clips.csv          one row per clip: offset, length, word_start, word_end (samples inside the piece)
    noise.npy  int16   every file of built\noise.csv (channel 0)       + noise.csv
    rirs.npy   float32 every file of built\rirs.csv (channel 0)        + rirs.csv
Rebuild after build_clips.py or build_noise.py changes (train.py stops if the row counts differ).
"""
import argparse
import sys
import time
from pathlib import Path

import numpy as np
import pandas as pd
import soundfile as sf

sys.path.insert(0, str(Path(__file__).resolve().parent.parent))
sys.path.insert(0, str(Path(__file__).resolve().parent.parent / "workspace"))
import config as C
from windows import speech_bounds

CACHE = C.BUILT / "cache"
MARGIN_S = 0.10
SETS = ["train", "dev"]


def read(path):
    audio, sr = sf.read(path, dtype="float32", always_2d=True)
    if sr != C.SR:
        raise ValueError(f"{path}: {sr} Hz")
    return audio[:, 0]


def cache_clips(limit):
    table = pd.read_csv(C.BUILT / "clips.csv", low_memory=False)
    rows = table[table["keep"] & table["set"].isin(SETS)].copy()
    if limit:
        rows = rows.groupby("label").head(limit // len(C.CLASSES) + 1)
    pieces, offsets, lengths, word_starts, word_ends = [], [], [], [], []
    offset, margin, t0 = 0, int(MARGIN_S * C.SR), time.time()
    for n, r in enumerate(rows.itertuples(), 1):
        audio = read(r.audio_path)
        if r.label == C.SILENCE:
            lo, hi, a, b = 0, len(audio), 0, len(audio)
        else:
            start_s, end_s = ((r.speech_start_s, r.speech_end_s) if not np.isnan(r.speech_start_s)
                              else speech_bounds(audio))
            a, b = int(start_s * C.SR), min(int(end_s * C.SR), len(audio))
            lo, hi = max(a - margin, 0), min(b + margin, len(audio))
        piece = np.clip(np.round(audio[lo:hi] * 32768), -32768, 32767).astype(np.int16)
        pieces.append(piece)
        offsets.append(offset)
        lengths.append(len(piece))
        word_starts.append(a - lo)
        word_ends.append(b - lo)
        offset += len(piece)
        if n % 10000 == 0:
            print(f"  clips {n}/{len(rows)} ({time.time() - t0:.0f} s)")
    rows = rows.assign(offset=offsets, length=lengths, word_start=word_starts, word_end=word_ends)
    return np.concatenate(pieces), rows


def cache_files(table_csv, dtype, limit):
    rows = pd.read_csv(table_csv)
    if limit:
        rows = rows.groupby("kind").head(3)
    pieces = [read(p) for p in rows["path"]]
    if dtype == np.int16:
        pieces = [np.clip(np.round(p * 32768), -32768, 32767).astype(np.int16) for p in pieces]
    lengths = [len(p) for p in pieces]
    rows = rows.assign(offset=np.concatenate([[0], np.cumsum(lengths)[:-1]]).astype(np.int64), length=lengths)
    return np.concatenate(pieces).astype(dtype), rows


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--limit", type=int, help="quick test: about N clips and 3 files per noise kind")
    args = ap.parse_args()
    out = CACHE if not args.limit else CACHE.parent / "cache_test"
    out.mkdir(parents=True, exist_ok=True)
    t0 = time.time()
    for name, make in [("clips", lambda: cache_clips(args.limit)),
                       ("noise", lambda: cache_files(C.BUILT / "noise.csv", np.int16, args.limit)),
                       ("rirs", lambda: cache_files(C.BUILT / "rirs.csv", np.float32, args.limit))]:
        audio, rows = make()
        np.save(out / f"{name}.npy", audio)
        rows.to_csv(out / f"{name}.csv", index=False)
        print(f"{name:6s} {len(rows):7d} rows  {len(audio) / C.SR / 3600:6.2f} h  "
              f"{audio.nbytes / 1e9:5.2f} GB  ({time.time() - t0:.0f} s)")
    if "length" in rows:
        print(f"rir length s: median {np.median(rows['length']) / C.SR:.2f}  max {rows['length'].max() / C.SR:.2f}")
    print(f"-> {out}")


if __name__ == "__main__":
    main()
