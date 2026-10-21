"""
tune_threshold.py - pick the board's detection threshold and averaging window for one experiment.

Runs the model once over every raw take and noise recording of a session (board stream logic from
eval_board.py), then replays the firmware decision (average of the last AVG results, threshold,
refractory 1500 ms) for every setting in the grid. Chooses on TUNE_SESSION, shows the other
sessions at the chosen setting.

Choice rule: most  hits - ERROR_WEIGHT * (wrong takes + unknown false alarms + noise false alarms).
A wrong action costs more than a miss (a miss only means saying the word again).

Run from voice_command_research\workspace\:
    C:/Users/ASUS/anaconda3/envs/torch_env/python.exe tune_threshold.py exp04_tcresnet-w16d3_aug-all

Writes experiments\<exp>\results\threshold_grid.csv and threshold.md
"""
import argparse
from collections import Counter
from pathlib import Path

import numpy as np

import eval_board as eb

TUNE_SESSION = "S0"
SHOW_SESSIONS = ["S1"]
THRESHOLDS = [round(t, 2) for t in np.arange(0.40, 0.951, 0.05)]
AVERAGES = [1, 2, 3]
REFRACTORY_MS = 1500
ERROR_WEIGHT = 2
GAIN_DB = 15.0
EXPERIMENTS = Path(__file__).resolve().parent / "experiments"


def model_outputs(audio, lm, model):
    """frame index and raw probabilities of every CNN run on a take (avg=1, nothing fires)"""
    infs, _ = eb.board_stream(audio, lm, model, GAIN_DB, avg=1, thr=2.0)
    if not infs:
        return np.zeros(0, int), np.zeros((0, len(eb.CLASSES)))
    return np.array([d["f"] for d in infs]), np.stack([d["avg"] for d in infs])


def detections(frames, probs, avg, threshold):
    """the firmware decision: mean of the last `avg` results, threshold, refractory"""
    found, last_ms = [], -1e12
    for i, f in enumerate(frames):
        mean = probs[max(0, i - avg + 1):i + 1].mean(axis=0)
        top = int(mean.argmax())
        if top not in (eb.UNK, eb.SIL) and mean[top] >= threshold and f * 10 - last_ms >= REFRACTORY_MS:
            last_ms = f * 10
            found.append(eb.CLASSES[top])
    return found


def load_session(session, lm, model):
    """model outputs for every command / unknown take and every noise recording of a session"""
    takes, seen = [], set()
    for r in eb.load_rows(eb.DEF_ROOT, session):
        if r["label"] == eb.C.SILENCE or not r.get("raw_path") or r["raw_path"] in seen:
            continue
        seen.add(r["raw_path"])
        frames, probs = model_outputs(eb.read_wav(eb.DEF_ROOT / r["raw_path"]), lm, model)
        takes.append(dict(label=r["label"], frames=frames, probs=probs))
    noise = []
    for path in sorted((eb.DEF_ROOT / "noise").glob(f"*/*_{session}_*.wav")):
        audio = eb.read_wav(path)
        frames, probs = model_outputs(audio, lm, model)
        noise.append(dict(minutes=len(audio) / eb.SR / 60, frames=frames, probs=probs))
    return takes, noise


def score(takes, noise, avg, threshold):
    outcome = Counter()
    for t in takes:
        found = detections(t["frames"], t["probs"], avg, threshold)
        if t["label"] == eb.C.UNKNOWN:
            outcome["unknown_false_alarms"] += bool(found)
        else:
            outcome["hits"] += t["label"] in found
            outcome["wrong"] += any(c != t["label"] for c in found)
    noise_alarms = sum(len(detections(n["frames"], n["probs"], avg, threshold)) for n in noise)
    commands = sum(t["label"] != eb.C.UNKNOWN for t in takes)
    errors = outcome["wrong"] + outcome["unknown_false_alarms"] + noise_alarms
    return dict(avg=avg, threshold=threshold, command_takes=commands, hits=outcome["hits"],
                detected_rate=round(outcome["hits"] / commands, 3), wrong=outcome["wrong"],
                unknown_takes=sum(t["label"] == eb.C.UNKNOWN for t in takes),
                unknown_false_alarms=outcome["unknown_false_alarms"], noise_false_alarms=noise_alarms,
                noise_minutes=round(sum(n["minutes"] for n in noise), 1),
                score=outcome["hits"] - ERROR_WEIGHT * errors)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("experiment")
    args = ap.parse_args()
    exp = EXPERIMENTS / args.experiment
    model = eb.Model(exp / "runs" / "final.pth", model_py=exp / "model.py")
    lm = eb.LogMel("numpy")

    rows = []
    for session in [TUNE_SESSION] + SHOW_SESSIONS:
        takes, noise = load_session(session, lm, model)
        print(f"{session}: {len(takes)} takes, {len(noise)} noise recordings")
        rows += [dict(session=session, **score(takes, noise, a, t)) for a in AVERAGES for t in THRESHOLDS]

    eb.write_csv(exp / "results" / "threshold_grid.csv", rows)
    tune = [r for r in rows if r["session"] == TUNE_SESSION]
    best = max(tune, key=lambda r: (r["score"], r["threshold"]))        # tie: the stricter threshold
    firmware = next(r for r in tune if r["avg"] == 3 and r["threshold"] == 0.70)
    cols = ["session", "avg", "threshold", "detected_rate", "hits", "wrong", "unknown_false_alarms",
            "noise_false_alarms", "score"]
    pick = lambda s, ref: next(r for r in rows if r["session"] == s and r["avg"] == ref["avg"]
                               and r["threshold"] == ref["threshold"])
    lines = [f"# Threshold tuning: {args.experiment}", "",
             f"Chosen on {TUNE_SESSION} by hits - {ERROR_WEIGHT} x (wrong + unknown false alarms + noise false alarms); "
             f"refractory {REFRACTORY_MS} ms. Full grid: threshold_grid.csv", "",
             f"## {TUNE_SESSION} grid", "", eb.md_table(tune, cols), "",
             "## Chosen setting vs firmware now (avg 3, threshold 0.70)", ""]
    compare = []
    for s in [TUNE_SESSION] + SHOW_SESSIONS:
        compare += [dict(setting="firmware now", **pick(s, firmware)), dict(setting="chosen", **pick(s, best))]
    lines.append(eb.md_table(compare, ["setting"] + cols))
    (exp / "results" / "threshold.md").write_text("\n".join(lines) + "\n", encoding="utf-8")
    print("\n".join(lines[-1:]))
    print(f"chosen: avg {best['avg']}, threshold {best['threshold']}")


if __name__ == "__main__":
    main()
