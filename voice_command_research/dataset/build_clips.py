"""
build_clips.py - one clip table for every source, with sets and speaker-grouped folds.

    C:/Users/ASUS/anaconda3/envs/torch_env/python.exe build_clips.py --limit 2000    quick test
    C:/Users/ASUS/anaconda3/envs/torch_env/python.exe build_clips.py                 full table

Sources (audio is read where it is, nothing is copied):
    open        DA dataset\\interim clips + the DA QC decisions (manifest_qc.csv), read-only
    piper       synthetic\\clips.csv (make_synthetic.py); _unknown_ rhymes get source piper_rhyme
    board       recordings\\manifest.csv (recorder.py); the session decides the set (SESSION_SET)

set:  train (5 folds) | dev | test_seen | test_unseen | comparison
fold: 0..4 for kept train rows, StratifiedGroupKFold by class with speaker as the group,
      so one speaker (or one noise recording) never sits in two folds.
Output: built\\clips.csv (one row per clip, keep / reject_reason columns).
"""
import argparse
import sys
from pathlib import Path

import numpy as np
import pandas as pd
from sklearn.model_selection import StratifiedGroupKFold

sys.path.insert(0, str(Path(__file__).resolve().parent.parent))
import config as C
from make_synthetic import HOMOPHONES, RHYME_START

OPEN_DIR = C.DA_DATA / "interim"
OPEN_QC = OPEN_DIR / "qc" / "manifest_qc.csv"
PIPER_CSV = C.SYNTHETIC / "clips.csv"
PIPER_PAD_S = 0.10                    # make_synthetic.py trims to the word + 0.10 s each side
BOARD_CSV = C.RECORDINGS / "manifest.csv"
OUT_CSV = C.BUILT / "clips.csv"

# board session -> set (option A, 21 Oct: S1 = Supanut as seen user; unseen-speaker test comes later)
SESSION_SET = {"S0": "comparison", "S1": "test_seen", "S2": "train", "S3": "dev"}
N_FOLDS = 5
SEED = 42

COLUMNS = ["audio_path", "label", "word", "source", "speaker", "gender", "set", "fold",
           "speech_start_s", "speech_end_s", "keep", "reject_reason"]


def open_rows():
    d = pd.read_csv(OPEN_QC)
    d = d[d["cls"].notna()]                          # 'part' rows (blue, box, ...) are not classes
    return pd.DataFrame(dict(
        audio_path=[str(OPEN_DIR / p) for p in d["clip_path"]],
        label=d["cls"], word=d["word"], source="open_" + d["source"], speaker=d["speaker"],
        gender=d["gender"].where(d["gender"].isin(["male", "female"]), ""),
        set="train", speech_start_s=d["speech_start_s"], speech_end_s=d["speech_end_s"],
        keep=d["keep"].astype(bool), reject_reason=d["qc_reason"].fillna("")))


def piper_rows():
    if not PIPER_CSV.exists():
        print(f"  no {PIPER_CSV.name} yet: Piper rows skipped")
        return pd.DataFrame(columns=COLUMNS)
    d = pd.read_csv(PIPER_CSV)
    index = d["clip_path"].str.extract(r"/(\d{5})_", expand=False).astype(float)   # _unknown_ clip number
    rhyme = (d["label"] == C.UNKNOWN) & (index >= RHYME_START)
    return pd.DataFrame(dict(
        audio_path=[str(C.SYNTHETIC / p) for p in d["clip_path"]],
        label=d["label"], word=d["word"], source=np.where(rhyme, "piper_rhyme", "piper"),
        speaker="piper_" + d["speaker_key"],
        gender=d["gender_est"], set="train",
        speech_start_s=PIPER_PAD_S, speech_end_s=(PIPER_PAD_S + d["speech_s"]).round(3),
        keep=True, reject_reason=""))


def board_rows():
    d = pd.read_csv(BOARD_CSV)
    label = np.where(d["role"] == "silence", C.SILENCE,
                     np.where(d["role"] == "unknown", C.UNKNOWN, d["word"]))
    unknown_sessions = sorted(set(d["session"]) - set(SESSION_SET))
    if unknown_sessions:
        raise SystemExit(f"sessions without a set in SESSION_SET: {unknown_sessions}")
    return pd.DataFrame(dict(
        audio_path=[str(C.RECORDINGS / p) for p in d["clip_path"]],
        label=label, word=d["word"], source="board", speaker=d["speaker"],
        gender=d["gender"].fillna(""), set=d["session"].map(SESSION_SET),
        speech_start_s=np.nan, speech_end_s=np.nan,
        keep=d["valid"].astype(bool), reject_reason=np.where(d["valid"].astype(bool), "", "invalid")))


def assign_folds(table):
    table["fold"] = -1
    train = (table["set"] == "train") & table["keep"]
    sgkf = StratifiedGroupKFold(n_splits=N_FOLDS, shuffle=True, random_state=SEED)
    index = table.index[train]
    for fold, (_, held_out) in enumerate(sgkf.split(index, table.loc[train, "label"], table.loc[train, "speaker"])):
        table.loc[index[held_out], "fold"] = fold
    return table


def check(table):
    """stop on any leak or bad label"""
    bad_labels = sorted(set(table["label"]) - set(C.CLASSES))
    if bad_labels:
        raise SystemExit(f"labels not in config.CLASSES: {bad_labels}")
    kept = table[table["keep"]]
    train = kept[kept["set"] == "train"]
    two_folds = int((train.groupby("speaker")["fold"].nunique() > 1).sum())
    seen = set(kept.loc[kept["set"].isin(["train", "dev"]), "speaker"])
    unseen_leak = sorted(set(kept.loc[kept["set"] == "test_unseen", "speaker"]) & seen)
    dev_leak = sorted(set(kept.loc[kept["set"] == "dev", "speaker"]) & set(train["speaker"]))
    print(f"\nleak check: speakers in two folds {two_folds} | dev speakers also in train {len(dev_leak)} | "
          f"test_unseen speakers also in train/dev {len(unseen_leak)}")
    if two_folds or unseen_leak or dev_leak:
        raise SystemExit("LEAK - table not written")


def summary(table):
    kept = table[table["keep"]]
    train = kept[kept["set"] == "train"]
    print(f"\nrows {len(table)} | kept {len(kept)} | rejected {len(table) - len(kept)}")
    print("kept rows per set:", kept["set"].value_counts().to_dict())
    print("\ntrain clips per class and source (+ speakers):")
    counts = train.pivot_table(index="label", columns="source", values="speaker", aggfunc="size", fill_value=0)
    counts["total"] = counts.sum(axis=1)
    counts["speakers"] = train.groupby("label")["speaker"].nunique()
    print(counts.reindex(C.CLASSES).fillna(0).astype(int).to_string())
    print("\ntrain clips per fold:", train["fold"].value_counts().sort_index().to_dict())
    share = pd.crosstab(train["label"], train["fold"], normalize="index").reindex(C.CLASSES)
    print(f"class share per fold: min {share.min().min():.3f}  max {share.max().max():.3f} (ideal {1 / N_FOLDS:.3f})")
    print("train gender:", train["gender"].replace("", "unknown").value_counts().to_dict())


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--limit", type=int, help="quick test: first N rows per source, nothing written")
    args = ap.parse_args()
    parts = [open_rows(), piper_rows(), board_rows()]
    if args.limit:
        parts = [p.head(args.limit) for p in parts]
    table = pd.concat([p for p in parts if len(p)], ignore_index=True)
    homophone = (table["label"] == C.UNKNOWN) & table["word"].isin(HOMOPHONES) & (table["source"] != "board")
    table.loc[homophone, ["keep", "reject_reason"]] = [False, "homophone of a command"]
    print(f"_unknown_ clips dropped as homophones of a command: {int(homophone.sum())}")
    table = assign_folds(table)
    check(table)
    summary(table)
    if args.limit:
        print("\n--limit: nothing written")
        return
    OUT_CSV.parent.mkdir(parents=True, exist_ok=True)
    table[COLUMNS].to_csv(OUT_CSV, index=False)
    print(f"\n-> {OUT_CSV}")


if __name__ == "__main__":
    main()
