"""
compare_board.py - your real board recordings, model by model: one row per word, one column per model.

    C:/Users/ASUS/anaconda3/envs/torch_env/python.exe compare_board.py                 S0 and S1
    C:/Users/ASUS/anaconda3/envs/torch_env/python.exe compare_board.py --session S1

Reads the eval_board.py reports that train.py writes (experiments\<exp>\results\board\...\<session>\
for an all-data run, or results\fold<k>\board\... averaged over the folds) and the deployed model's
report (eval\baseline_w16d2_noaug\<session>\).
Cells: clip mode "correct/takes"; stream "detected/takes".
Output: experiments\board_<session>.csv, _totals.csv and .md
"""
import argparse
import json
import sys
from pathlib import Path

import pandas as pd

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE.parent))
import config as C

EXPERIMENTS = HERE / "experiments"
BASELINE = ("deployed_w16d2_noaug", HERE / "eval" / "baseline_w16d2_noaug")
NOTES = {"S0": "S0 = old wiring (mic VDD about 1.8 V), 109 clips, few takes per word. "
               "Its _unknown_ takes include 'write', which sounds like 'right'.",
         "S1": "S1 = fixed wiring, 290 clips (10 per command, 30 _unknown_, 60 room _silence_). "
               "Its _unknown_ takes include 'write', which sounds like 'right'."}


def reports(session):
    """(model name, [report folders]): the all-data run if there is one, else every fold"""
    models = [(BASELINE[0], [BASELINE[1] / session])]
    for exp in sorted(p for p in EXPERIMENTS.iterdir() if p.is_dir()):
        folders = sorted(exp.glob(f"results/board/*/{session}")) or sorted(exp.glob(f"results/fold*/board/*/{session}"))
        if folders:
            models.append((exp.name, folders))
    return models


def model_columns(folders):
    clip = pd.concat([pd.read_csv(f / "clip_per_class.csv") for f in folders])
    stream = pd.concat([pd.read_csv(f / "stream_per_class.csv") for f in folders])
    clip = clip.groupby("cls").agg(takes=("support", "first"), correct=("correct", "mean"))
    stream = stream.assign(detected=stream["hit"] + stream["hit_extra"]).groupby("cls").agg(
        stream_takes=("takes", "first"), detected=("detected", "mean"), wrong=("wrong", "mean"))
    summaries = [json.loads((f / "summary.json").read_text()) for f in folders]
    totals = {k: sum(s[part][k] for s in summaries) / len(summaries) for part, k in
              [("clip", "accuracy"), ("clip", "macro_f1"), ("clip", "command_accuracy"),
               ("stream", "detected_rate"), ("stream", "wrong"), ("stream", "unknown_false_alarms"),
               ("stream", "noise_false_alarms")]}
    return clip, stream, totals, len(folders), summaries[0]["stream"]["unknown_takes"]


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--session", choices=["S0", "S1"], help="default: both")
    args = ap.parse_args()
    for session in [args.session] if args.session else ["S0", "S1"]:
        compare(session)


def compare(session):
    rows = {c: {"word": c} for c in C.CLASSES}
    total_rows = []
    for name, folders in reports(session):
        clip, stream, totals, n_models, unknown_takes = model_columns(folders)
        for c in C.CLASSES:
            if c in clip.index:
                rows[c]["takes"] = int(clip.loc[c, "takes"])
                rows[c][f"{name} clip"] = f"{clip.loc[c, 'correct']:.1f}/{int(clip.loc[c, 'takes'])}"
            if c in stream.index and c in C.TOKENS:
                rows[c][f"{name} stream"] = f"{stream.loc[c, 'detected']:.1f}/{int(stream.loc[c, 'stream_takes'])}"
        total_rows.append(dict(model=name, fold_models=n_models,
                               clip_accuracy=round(totals["accuracy"], 3), clip_macro_f1=round(totals["macro_f1"], 3),
                               command_accuracy=round(totals["command_accuracy"], 3),
                               stream_detected=round(totals["detected_rate"], 3),
                               stream_wrong=round(totals["wrong"], 1),
                               unknown_false_alarms=f"{totals['unknown_false_alarms']:.1f}/{unknown_takes}",
                               noise_false_alarms=round(totals["noise_false_alarms"], 1)))
    per_word = pd.DataFrame([r for r in rows.values() if r.get("takes")])
    totals = pd.DataFrame(total_rows)

    out = EXPERIMENTS / f"board_{session}"
    per_word.to_csv(out.with_suffix(".csv"), index=False)
    totals.to_csv(out.with_name(out.name + "_totals.csv"), index=False)
    md = [f"# Board recordings {session}: model by model", "", NOTES[session], "",
          "Clip cells: correct/takes (mean over the fold models when a model has folds). "
          "Stream cells: detected/takes.", "",
          totals.to_markdown(index=False), "", per_word.fillna("").to_markdown(index=False), ""]
    out.with_suffix(".md").write_text("\n".join(md), encoding="utf-8")
    print(totals.to_string(index=False))
    print()
    print(per_word.fillna("").to_string(index=False))
    print(f"\n-> {out.with_suffix('.md')}")


if __name__ == "__main__":
    main()
