r"""
eval_board.py - test keyword models on the board recordings (S0, S1, ...) on the PC,
exactly as the STM32 sees them.                                   (Session E, 18 Oct 2026)

  check   pipeline self-test: train00001.wav -> log-mel and probabilities vs kws_clip_test.h
          (the same comparison as the board's power-up self-test), mel table vs kws_feature_tables.h
  clip    the 1 s clips cut by recorder.py (mono ch1) -> accuracy per class, confusion matrix,
          top-3 of every mistake, macro-F1.  Fast: use it to compare models.
  stream  the raw 2 s stereo takes through the live firmware's decision logic
          (kws_stream.c + live_step() in model_realtime_code):
            frames WITHOUT centre padding (frame f = samples [160 f, 160 f + 400)),
            noise floor per channel, SNR channel choice with 2 dB hysteresis,
            CNN every 10 frames on the last 101, average of the last 3 results,
            threshold 0.70, refractory 1500 ms (the firmware value), _unknown_/_silence_ never fire.
          Per take: hit / hit+extra / wrong / miss.  False alarms on _unknown_ takes and on the
          long noise recordings (dataset\recordings\noise\<label>\<speaker>_<session>_*.wav).
  all     clip + stream

Board match: int16 / 32768 * 10^(gain/20), no clipping; log-mel = the librosa recipe
(n_fft 400, hop 160, periodic Hann, 40 slaney mel 20-8000 Hz, ln(S + 1e-6)); the model gets RAW
log-mel (normaliser inside).  Features come from the team's features.py when librosa is
importable, otherwise from an identical numpy copy ('check' compares both against the board tables).

Models: .onnx (onnxruntime) or .pth saved as nn.Sequential(Normalise, TCResNet)
(loaded with export_kws_onnx.build_from_weights; give --width/--depth).

use (from voice_command_research\workspace\; .onnx models with C:/Python314/python.exe,
.pth models with C:/Users/ASUS/anaconda3/envs/torch_env/python.exe):
  python eval_board.py check
  python eval_board.py all --sessions S1 S0                                 baseline (kws.onnx)
  python eval_board.py all --sessions S1 --model <experiment>\runs\xyz.pth --width 24 --depth 3 --tag w24d3_aug

Reports: workspace\eval\<tag>\<session>\  (report.md, summary.json, CSVs, confusion png)
         workspace\eval\<tag>\compare_sessions.md  when several sessions are given

S1 is the TEST set: never train on it, never pick thresholds or gain on it.
"""
import argparse
import csv
import json
import re
import struct
import sys
import time
from collections import Counter
from pathlib import Path

import numpy as np

HERE = Path(__file__).resolve().parent    # voice_command_research\workspace
sys.path.insert(0, str(HERE.parent))
import config as C

SR, N_FFT, HOP, N_MELS, N_FRAMES = 16000, 400, 160, 40, 101
CLASSES = C.CLASSES_22                    # set_classes() switches to the model's class list
UNK, SIL = CLASSES.index(C.UNKNOWN), CLASSES.index(C.SILENCE)
COMMANDS = CLASSES[:UNK]


def set_classes(n_outputs):
    """22 outputs = the deployed model (no undo), 23 = config.CLASSES"""
    global CLASSES, UNK, SIL, COMMANDS
    CLASSES = {len(C.CLASSES_22): C.CLASSES_22, len(C.CLASSES): C.CLASSES}[n_outputs]
    UNK, SIL = CLASSES.index(C.UNKNOWN), CLASSES.index(C.SILENCE)
    COMMANDS = CLASSES[:UNK]
LN_TO_DB = 4.3429448                      # 10 / ln(10): mean ln(mel power) -> dB, as the board prints

DEF_ROOT = C.RECORDINGS
DEF_PREFER = C.FINAL_MODEL
DEF_FEATURES = Path(r"D:\data_analytics\DA_Project\voice_command_classification\workspace\shared")


# ---------------------------------------------------------------- audio
def read_wav(path):
    """16-bit PCM WAV -> int16 array (n, channels). Plain RIFF parser (no soundfile needed)."""
    data = Path(path).read_bytes()
    if data[:4] != b"RIFF" or data[8:12] != b"WAVE":
        raise ValueError(f"{path}: not a WAV file")
    pos, fmt, pcm = 12, None, None
    while pos + 8 <= len(data):
        cid, size = data[pos:pos + 4], int.from_bytes(data[pos + 4:pos + 8], "little")
        body = data[pos + 8:pos + 8 + size]
        if cid == b"fmt ":
            tag, ch, sr = struct.unpack("<HHI", body[:8])
            bits = struct.unpack("<H", body[14:16])[0]
            if tag == 0xFFFE and len(body) >= 26:
                tag = struct.unpack("<H", body[24:26])[0]
            fmt = (tag, ch, sr, bits)
        elif cid == b"data":
            pcm = body
        pos += 8 + size + (size & 1)
    if fmt is None or pcm is None:
        raise ValueError(f"{path}: no fmt/data chunk")
    tag, ch, sr, bits = fmt
    if tag != 1 or bits != 16 or sr != SR:
        raise ValueError(f"{path}: need 16-bit PCM at {SR} Hz, got tag {tag}, {bits} bit, {sr} Hz")
    n = len(pcm) // (2 * ch)
    return np.frombuffer(pcm[:n * 2 * ch], dtype="<i2").reshape(n, ch)


def gain_scale(gain_db):
    """the board: scale = 10^(gain/20) / 32768 (float32), no clipping"""
    return np.float32(10.0 ** (gain_db / 20.0) / 32768.0)


# ---------------------------------------------------------------- features
def _hz_to_mel(f):                        # slaney (librosa htk=False)
    f = np.asarray(f, dtype=np.float64)
    f_sp, min_log_hz, min_log_mel, logstep = 200.0 / 3, 1000.0, 15.0, np.log(6.4) / 27.0
    m = f / f_sp
    return np.where(f >= min_log_hz, min_log_mel + np.log(np.maximum(f, 1e-10) / min_log_hz) / logstep, m)


def _mel_to_hz(m):
    m = np.asarray(m, dtype=np.float64)
    f_sp, min_log_hz, min_log_mel, logstep = 200.0 / 3, 1000.0, 15.0, np.log(6.4) / 27.0
    return np.where(m >= min_log_mel, min_log_hz * np.exp(logstep * (m - min_log_mel)), f_sp * m)


def mel_basis_numpy(fmin=20.0, fmax=SR / 2):
    """= librosa.filters.mel(sr=16000, n_fft=400, n_mels=40, fmin=20, fmax=8000), slaney norm"""
    fft_f = np.linspace(0, SR / 2, 1 + N_FFT // 2)
    mel_f = _mel_to_hz(np.linspace(_hz_to_mel(fmin), _hz_to_mel(fmax), N_MELS + 2))
    fdiff = np.diff(mel_f)
    ramps = mel_f[:, None] - fft_f[None, :]
    w = np.zeros((N_MELS, len(fft_f)))
    for i in range(N_MELS):
        w[i] = np.maximum(0, np.minimum(-ramps[i] / fdiff[i], ramps[i + 2] / fdiff[i + 1]))
    w *= (2.0 / (mel_f[2:N_MELS + 2] - mel_f[:N_MELS]))[:, None]
    return w.astype(np.float32)


class LogMel:
    """clip(): 1 s clip, centre padding (training / clip mode).  frames(): no padding (board stream)."""

    def __init__(self, backend="auto", features_py=DEF_FEATURES):
        self.win = (0.5 - 0.5 * np.cos(2 * np.pi * np.arange(N_FFT) / N_FFT)).astype(np.float32)
        self.team_logmel, self.basis, self.backend = None, None, "numpy"
        if backend in ("auto", "librosa"):
            try:
                sys.dont_write_bytecode = True            # keep the team repo clean
                if features_py and Path(features_py).is_dir():
                    sys.path.insert(0, str(features_py))
                import features as team_features          # workspace\shared\features.py (librosa)
                import librosa
                self.team_logmel = team_features.logmel
                self.basis = librosa.filters.mel(sr=SR, n_fft=N_FFT, n_mels=N_MELS, fmin=20,
                                                 fmax=SR // 2).astype(np.float32)
                self.backend = f"librosa {librosa.__version__} (features.py)"
            except Exception as e:
                if backend == "librosa":
                    raise
                print(f"[features] librosa/features.py not available ({type(e).__name__}: {e}) -> numpy copy")
        if self.basis is None:
            self.basis = mel_basis_numpy()

    def frames(self, x):
        """float waveform -> (40, T) raw log-mel, frame f = x[160 f : 160 f + 400] (board stream)"""
        x = np.asarray(x, dtype=np.float32)
        T = 0 if len(x) < N_FFT else 1 + (len(x) - N_FFT) // HOP
        if T == 0:
            return np.zeros((N_MELS, 0), np.float32)
        idx = np.arange(N_FFT)[None, :] + HOP * np.arange(T)[:, None]
        spec = np.fft.rfft(x[idx] * self.win, axis=1)
        power = (spec.real ** 2 + spec.imag ** 2).astype(np.float32)
        return np.log(power @ self.basis.T + 1e-6).astype(np.float32).T

    def clip(self, x):
        """1 s float waveform -> (40, 101), centre=True with zero padding (= features.logmel)"""
        x = np.asarray(x, dtype=np.float32)
        x = x[:SR] if len(x) >= SR else np.pad(x, (0, SR - len(x)))
        if self.team_logmel is not None:
            return np.asarray(self.team_logmel(x), dtype=np.float32)
        return self.frames(np.pad(x, (N_FFT // 2, N_FFT // 2)))


# ---------------------------------------------------------------- model
def softmax(z):
    z = z - z.max(axis=1, keepdims=True)
    e = np.exp(z)
    return e / e.sum(axis=1, keepdims=True)


class Model:
    """probs(X): (n, 40, 101) raw log-mel -> (n, 22 or 23) probabilities"""

    def __init__(self, path, width=16, depth=2, export_py=None, model_py=None):
        self.path = Path(path)
        if model_py:                                  # any network: experiments\<exp>\model.py build_model()
            import importlib.util
            import torch
            spec = importlib.util.spec_from_file_location("experiment_model", model_py)
            module = importlib.util.module_from_spec(spec)
            spec.loader.exec_module(module)
            sd = torch.load(self.path, map_location="cpu")
            self.n_outputs = int([v for v in sd.values() if v.ndim == 2][-1].shape[0])   # last Linear
            self.net = module.build_model(torch.zeros(N_MELS), torch.ones(N_MELS), self.n_outputs)
            self.net.load_state_dict(sd)
            self.net.eval()
            self.torch = torch
            self.kind = f"pytorch {Path(model_py).parent.name}, {self.n_outputs} classes"
        elif self.path.suffix.lower() == ".onnx":
            import onnxruntime as ort
            so = ort.SessionOptions()
            so.log_severity_level = 3
            self.sess = ort.InferenceSession(str(self.path), so, providers=["CPUExecutionProvider"])
            inp = self.sess.get_inputs()[0]
            self.inp = inp.name
            self.fixed_batch = isinstance(inp.shape[0], int)
            self.kind = f"onnx (input {inp.name} {inp.shape})"
            self.n_outputs = int(self.sess.get_outputs()[0].shape[-1])
        else:
            import torch
            sys.path.insert(0, str(Path(export_py or DEF_PREFER / "final_model_info" / "export_kws_onnx.py").parent))
            from export_kws_onnx import Normalize, TCResNet, read_state_dict
            sd = read_state_dict(str(self.path))      # build_from_weights() with the class count from the file
            norm = None
            if "0.mean" in sd and "0.std" in sd:
                norm = Normalize(sd["0.mean"], sd["0.std"])
                sd = {k[2:]: v for k, v in sd.items() if k.startswith("1.")}
            else:
                print("[model] WARNING: no normaliser in the file -> the network gets raw log-mel")
            self.n_outputs = int(sd["head.3.weight"].shape[0])
            net = TCResNet(width, depth, n_classes=self.n_outputs)
            net.load_state_dict(sd)                   # fails loudly if width/depth do not match the file
            self.torch = torch
            self.net = (net if norm is None else torch.nn.Sequential(norm, net)).eval()
            self.kind = f"pytorch TC-ResNet w{width} d{depth}, {self.n_outputs} classes"
        set_classes(self.n_outputs)

    def probs(self, X):
        X = np.ascontiguousarray(X, dtype=np.float32)
        if X.shape[0] == 0:
            return np.zeros((0, len(CLASSES)), np.float32)
        if hasattr(self, "sess"):
            if self.fixed_batch:
                out = np.concatenate([self.sess.run(None, {self.inp: X[i:i + 1]})[0] for i in range(len(X))])
            else:
                out = np.concatenate([self.sess.run(None, {self.inp: X[i:i + 256]})[0]
                                      for i in range(0, len(X), 256)])
            out = out.reshape(len(X), -1)
            if out.min() < 0 or np.abs(out.sum(1) - 1).max() > 1e-3:   # logits -> probabilities
                out = softmax(out)
        else:
            with self.torch.no_grad():
                out = self.torch.softmax(self.net(self.torch.from_numpy(X)), dim=1).numpy()
        assert out.shape[1] == len(CLASSES), f"model gives {out.shape[1]} outputs, expected {len(CLASSES)}"
        return out.astype(np.float32)


# ---------------------------------------------------------------- data
def load_rows(root, session, quality_only=False):
    rows = []
    with open(Path(root) / "manifest.csv", newline="", encoding="utf-8") as f:
        for r in csv.DictReader(f):
            if r["session"] != session or r.get("source", "board") != "board":
                continue
            if quality_only and r.get("quality_ok", "True") != "True":
                continue
            w = r["word"]
            r["label"] = w if w in CLASSES else "_unknown_"
            rows.append(r)
    return rows


def top3(p):
    order = np.argsort(-p)[:3]
    return " | ".join(f"{CLASSES[i]} {p[i]:.2f}" for i in order)


def write_csv(path, rows, fields=None):
    rows = list(rows)
    fields = fields or (list(rows[0].keys()) if rows else [])
    with open(path, "w", newline="", encoding="utf-8") as f:
        w = csv.DictWriter(f, fieldnames=fields, extrasaction="ignore")
        w.writeheader()
        w.writerows(rows)


# ---------------------------------------------------------------- clip mode
def eval_clips(rows, root, lm, model, gain_db):
    scale = gain_scale(gain_db)
    X, keep, peaks = [], [], []
    for r in rows:
        a = read_wav(Path(root) / r["clip_path"])
        F = lm.clip(a[:, 0].astype(np.float32) * scale)
        X.append(F)
        keep.append(r)
        peaks.append(LN_TO_DB * float(F.mean(axis=0).max()))   # loudest frame, same measure as the board status line
    P = model.probs(np.stack(X)) if X else np.zeros((0, len(CLASSES)))
    y = np.array([CLASSES.index(r["label"]) for r in keep])
    yhat = P.argmax(1) if len(P) else np.array([], int)

    preds = []
    for r, p, t, h, pk in zip(keep, P, y, yhat, peaks):
        preds.append(dict(clip_path=r["clip_path"], word=r["word"], label=r["label"], pred=CLASSES[h],
                          correct=int(t == h), p_pred=round(float(p[h]), 4), p_true=round(float(p[t]), 4),
                          rank_true=int((p > p[t]).sum()) + 1, top3=top3(p), speech_peak_db=round(pk, 1),
                          snr_db=r.get("snr_db", ""), quality_ok=r.get("quality_ok", ""), flags=r.get("flags", "")))

    cm = np.zeros((len(CLASSES), len(CLASSES)), int)
    for t, h in zip(y, yhat):
        cm[t, h] += 1
    per_class, f1s = [], []
    for i, c in enumerate(CLASSES):
        sup, tp, npred = cm[i].sum(), cm[i, i], cm[:, i].sum()
        if sup == 0 and npred == 0:
            continue
        rec = tp / sup if sup else float("nan")
        prec = tp / npred if npred else 0.0
        f1 = 2 * prec * rec / (prec + rec) if sup and (prec + rec) > 0 else 0.0
        if sup:
            f1s.append(f1)
        wrong = Counter({CLASSES[j]: int(cm[i, j]) for j in range(len(CLASSES)) if j != i and cm[i, j]})
        mp = float(np.mean([q["p_true"] for q in preds if q["label"] == c])) if sup else float("nan")
        per_class.append(dict(cls=c, support=int(sup), correct=int(tp), recall=round(rec, 3), precision=round(prec, 3),
                              f1=round(f1, 3), mean_p_true=round(mp, 3),
                              confused_with=", ".join(f"{k} {v}" for k, v in wrong.most_common(3))))
    is_cmd = np.isin(y, range(20))
    summary = dict(
        n=int(len(y)), accuracy=round(float((y == yhat).mean()), 4) if len(y) else None,
        macro_f1=round(float(np.mean(f1s)), 4) if f1s else None,
        macro_f1_note="mean F1 over the classes present in this set (true labels)",
        command_accuracy=round(float((y == yhat)[is_cmd].mean()), 4) if is_cmd.any() else None,
        unknown_accuracy=round(float((yhat[y == UNK] == UNK).mean()), 4) if (y == UNK).any() else None,
        silence_accuracy=round(float((yhat[y == SIL] == SIL).mean()), 4) if (y == SIL).any() else None,
        commands_rejected=int(np.isin(yhat[is_cmd], [UNK, SIL]).sum()),
        speech_peak_db_median=round(float(np.median([q["speech_peak_db"] for q in preds if q["label"] in COMMANDS])), 1)
        if is_cmd.any() else None,
    )
    qok = [q for q in preds if q["quality_ok"] == "True"]
    if qok and len(qok) < len(preds):
        summary["accuracy_quality_ok_only"] = round(float(np.mean([q["correct"] for q in qok])), 4)
    return dict(summary=summary, per_class=per_class, preds=preds, cm=cm)


# ---------------------------------------------------------------- stream mode (board logic)
def board_stream(audio, lm, model, gain_db, stride=10, avg=3, thr=0.70, refractory_ms=1500, channel="auto"):
    """audio (n, ch) int16 -> (inferences, detections), the same steps as live_step() on the STM32"""
    scale = gain_scale(gain_db)
    nch = audio.shape[1]
    feats = [lm.frames(audio[:, c].astype(np.float32) * scale) for c in range(nch)]
    energy = [F.mean(axis=0) for F in feats]                  # mean log-mel per frame
    T = feats[0].shape[1]
    if T < N_FRAMES:
        return [], []
    noise = np.zeros((nch, T))
    for c in range(nch):                                      # kws_stream_poll(): noise floor
        n = float(energy[c][0])
        for f in range(T):
            e = float(energy[c][f])
            if f > 0:
                n += (0.2 if e < n else 0.002) * (e - n)
            noise[c, f] = n
    infer_at = list(range(N_FRAMES, T + 1, stride))           # CNN every 10 new frames from frame 101
    cur, chans, snrs = 0, [], []
    for f in infer_at:                                        # kws_stream_choose_channel()
        lo = max(0, f - N_FRAMES)
        snr = [LN_TO_DB * (float(energy[c][lo:f].max()) - noise[c, f - 1]) for c in range(nch)]
        if channel == "auto" and nch == 2:
            if snr[1 - cur] > snr[cur] + 2.0:
                cur = 1 - cur
            ch = cur
        else:
            ch = min(int(channel), nch - 1) if channel != "auto" else 0
        chans.append(ch)
        snrs.append(snr[ch])
    P = model.probs(np.stack([feats[ch][:, f - N_FRAMES:f] for f, ch in zip(infer_at, chans)]))
    hist, last_ms, infs, dets = [], -1e12, [], []
    for i, f in enumerate(infer_at):                          # average of the last 3, threshold, refractory
        hist = (hist + [P[i]])[-avg:]
        a = np.mean(hist, axis=0)
        top = int(a.argmax())
        infs.append(dict(f=f, t=f / 100, ch=chans[i], snr=snrs[i], top=top, p=float(a[top]), avg=a))
        if top not in (UNK, SIL) and a[top] >= thr and f * 10 - last_ms >= refractory_ms:
            last_ms = f * 10
            dets.append(dict(t=f / 100, cls=CLASSES[top], p=float(a[top]), ch=chans[i], snr=snrs[i]))
    return infs, dets


def eval_stream(rows, root, session, lm, model, gain_db, cfg):
    takes, seen = [], set()
    for r in rows:
        if r["label"] == "_silence_" or not r.get("raw_path") or r["raw_path"] in seen:
            continue
        seen.add(r["raw_path"])
        audio = read_wav(Path(root) / r["raw_path"])
        infs, dets = board_stream(audio, lm, model, gain_db, **cfg)
        lab = r["label"]
        det_cls = [d["cls"] for d in dets]
        if lab in COMMANDS:
            other = [c for c in det_cls if c != lab]
            outcome = ("hit" if not other else "hit+extra") if lab in det_cls else ("wrong" if other else "miss")
        else:
            outcome = "false_alarm" if dets else "rejected"
        li = CLASSES.index(lab)
        best = max(infs, key=lambda d: float(d["avg"][:SIL].max())) if infs else None   # most speech-like moment
        takes.append(dict(raw_path=r["raw_path"], word=r["word"], label=lab, outcome=outcome,
                          first_detection=det_cls[0] if det_cls else "",
                          detections="; ".join(f"{d['cls']}@{d['t']:.2f}s {d['p']:.2f}" for d in dets),
                          max_avg_p_true=round(max(float(d["avg"][li]) for d in infs), 3) if infs else "",
                          peak_top3=top3(best["avg"]) if best else "",
                          channels_used="/".join(sorted({f"ch{d['ch']}" for d in infs})),
                          max_snr_db=round(max(d["snr"] for d in infs), 1) if infs else ""))

    noise_rows = []
    for p in sorted((Path(root) / "noise").glob(f"*/*_{session}_*.wav")):
        audio = read_wav(p)
        infs, dets = board_stream(audio, lm, model, gain_db, **cfg)
        dur = len(audio) / SR
        noise_rows.append(dict(file=str(p.relative_to(root)), duration_s=round(dur, 1), inferences=len(infs),
                               false_alarms=len(dets), fa_per_min=round(len(dets) / dur * 60, 2),
                               top_classes=", ".join(f"{k} {v}" for k, v in
                                                     Counter(CLASSES[d["top"]] for d in infs).most_common(3)),
                               detections="; ".join(f"{d['cls']}@{d['t']:.1f}s {d['p']:.2f}" for d in dets)))

    per_class = []
    for c in COMMANDS + ["_unknown_"]:
        ts = [t for t in takes if t["label"] == c]
        if not ts:
            continue
        oc = Counter(t["outcome"] for t in ts)
        wrong_as = Counter(x for t in ts for x in t["detections"].split("; ") if x and not x.startswith(c + "@"))
        wrong_as = Counter(x.split("@")[0] for x in wrong_as.elements())
        per_class.append(dict(cls=c, takes=len(ts), hit=oc["hit"], hit_extra=oc["hit+extra"], wrong=oc["wrong"],
                              miss=oc["miss"], false_alarm=oc["false_alarm"], rejected=oc["rejected"],
                              detected_rate=round((oc["hit"] + oc["hit+extra"]) / len(ts), 3) if c in COMMANDS else "",
                              wrong_as=", ".join(f"{k} {v}" for k, v in wrong_as.most_common(3)),
                              mean_max_p_true=round(float(np.mean([t["max_avg_p_true"] for t in ts])), 3)))
    cmd = [t for t in takes if t["label"] in COMMANDS]
    unk = [t for t in takes if t["label"] == "_unknown_"]
    oc = Counter(t["outcome"] for t in cmd)
    noise_min = sum(n["duration_s"] for n in noise_rows) / 60
    summary = dict(
        command_takes=len(cmd),
        hit=oc["hit"], hit_extra=oc["hit+extra"], wrong=oc["wrong"], miss=oc["miss"],
        detected_rate=round((oc["hit"] + oc["hit+extra"]) / len(cmd), 4) if cmd else None,
        first_detection_correct=round(float(np.mean([t["first_detection"] == t["label"] for t in cmd])), 4) if cmd else None,
        unknown_takes=len(unk), unknown_false_alarms=sum(t["outcome"] == "false_alarm" for t in unk),
        noise_minutes=round(noise_min, 2), noise_false_alarms=sum(n["false_alarms"] for n in noise_rows),
        channel_use=dict(Counter(t["channels_used"] for t in takes)),
    )
    return dict(summary=summary, per_class=per_class, takes=takes, noise=noise_rows)


# ---------------------------------------------------------------- reports
def save_confusion_png(cm, path, title):
    try:
        import matplotlib
        matplotlib.use("Agg")
        import matplotlib.pyplot as plt
    except Exception:
        return
    used = [i for i in range(len(CLASSES)) if cm[i].sum() or cm[:, i].sum()]
    m = cm[np.ix_(used, used)]
    fig, ax = plt.subplots(figsize=(9, 8))
    ax.imshow(m / np.maximum(m.sum(1, keepdims=True), 1), cmap="Blues", vmin=0, vmax=1)
    for i in range(len(used)):
        for j in range(len(used)):
            if m[i, j]:
                ax.text(j, i, m[i, j], ha="center", va="center", fontsize=7,
                        color="white" if m[i, j] > 0.6 * max(1, m[i].sum()) else "black")
    names = [CLASSES[i] for i in used]
    ax.set_xticks(range(len(used)), names, rotation=90, fontsize=8)
    ax.set_yticks(range(len(used)), names, fontsize=8)
    ax.set_xlabel("predicted")
    ax.set_ylabel("true")
    ax.set_title(title, fontsize=10)
    fig.tight_layout()
    fig.savefig(path, dpi=130)
    plt.close(fig)


def md_table(rows, cols):
    out = ["| " + " | ".join(cols) + " |", "|" + "---|" * len(cols)]
    out += ["| " + " | ".join(str(r.get(c, "")) for c in cols) + " |" for r in rows]
    return "\n".join(out)


def write_report(out, session, meta, clip, stream):
    out.mkdir(parents=True, exist_ok=True)
    lines = [f"# Board evaluation - {meta['tag']} on {session}", "",
             f"model `{meta['model']}` ({meta['model_kind']}) · gain {meta['gain_db']:+.1f} dB · "
             f"features {meta['features']} · {meta['when']}", ""]
    if clip:
        write_csv(out / "clip_predictions.csv", clip["preds"])
        write_csv(out / "clip_per_class.csv", clip["per_class"])
        with open(out / "clip_confusion.csv", "w", newline="", encoding="utf-8") as f:
            w = csv.writer(f)
            w.writerow(["true\\pred"] + CLASSES)
            for i, c in enumerate(CLASSES):
                w.writerow([c] + list(clip["cm"][i]))
        save_confusion_png(clip["cm"], out / "clip_confusion.png", f"{meta['tag']} on {session} (clip mode)")
        s = clip["summary"]
        lines += ["## Clip mode (1 s clips, mono ch1)", "",
                  f"n {s['n']} · **accuracy {s['accuracy']}** · **macro-F1 {s['macro_f1']}** · commands "
                  f"{s['command_accuracy']} · _unknown_ {s['unknown_accuracy']} · _silence_ {s['silence_accuracy']} · "
                  f"commands rejected as unknown/silence {s['commands_rejected']} · speech peak median "
                  f"{s['speech_peak_db_median']} dB (training clip -25.4 dB)", "",
                  md_table(clip["per_class"], ["cls", "support", "correct", "recall", "precision", "f1",
                                               "mean_p_true", "confused_with"]), "",
                  "### Mistakes (top-3)", "",
                  md_table([p for p in clip["preds"] if not p["correct"]],
                           ["clip_path", "label", "pred", "top3", "snr_db", "flags"]), ""]
    if stream:
        write_csv(out / "stream_takes.csv", stream["takes"])
        write_csv(out / "stream_per_class.csv", stream["per_class"])
        if stream["noise"]:
            write_csv(out / "stream_noise.csv", stream["noise"])
        s = stream["summary"]
        c = meta["stream_cfg"]
        lines += [f"## Stream mode (raw 2 s takes, board logic: stride {c['stride']} frames, avg {c['avg']}, "
                  f"threshold {c['thr']}, refractory {c['refractory_ms']} ms, channel {c['channel']})", "",
                  f"command takes {s['command_takes']}: **detected {s['detected_rate']}** "
                  f"(hit {s['hit']}, hit+extra {s['hit_extra']}), wrong {s['wrong']}, miss {s['miss']} · "
                  f"first detection correct {s['first_detection_correct']} · _unknown_ false alarms "
                  f"{s['unknown_false_alarms']}/{s['unknown_takes']} · noise false alarms "
                  f"{s['noise_false_alarms']} in {s['noise_minutes']} min · channels {s['channel_use']}", "",
                  md_table(stream["per_class"], ["cls", "takes", "hit", "hit_extra", "wrong", "miss", "false_alarm",
                                                 "detected_rate", "wrong_as", "mean_max_p_true"]), ""]
        bad = [t for t in stream["takes"] if t["outcome"] not in ("hit", "rejected")]
        if bad:
            lines += ["### Takes that were not a clean hit", "",
                      md_table(bad, ["raw_path", "label", "outcome", "detections", "max_avg_p_true", "peak_top3",
                                     "channels_used"]), ""]
        if stream["noise"]:
            lines += ["### Noise recordings", "", md_table(stream["noise"], ["file", "duration_s", "inferences",
                                                                           "false_alarms", "top_classes",
                                                                           "detections"]), ""]
    (out / "report.md").write_text("\n".join(lines), encoding="utf-8")
    summ = dict(meta=meta, session=session, clip=clip["summary"] if clip else None,
                stream=stream["summary"] if stream else None)
    (out / "summary.json").write_text(json.dumps(summ, indent=2), encoding="utf-8")
    return summ


def write_comparison(out, results):
    sess = list(results)
    lines = [f"# Session comparison - {out.name}", "",
             "| metric | " + " | ".join(sess) + " |", "|---|" + "---|" * len(sess)]
    keys = [("clip", "n"), ("clip", "accuracy"), ("clip", "macro_f1"), ("clip", "command_accuracy"),
            ("clip", "unknown_accuracy"), ("clip", "speech_peak_db_median"), ("stream", "detected_rate"),
            ("stream", "first_detection_correct"), ("stream", "wrong"), ("stream", "miss"),
            ("stream", "unknown_false_alarms")]
    for part, k in keys:
        vals = [str((results[s][part] or {}).get(k, "")) if results[s].get(part) else "" for s in sess]
        lines.append(f"| {part} {k} | " + " | ".join(vals) + " |")
    lines += ["", "## Per class: clip recall · stream detected (hit / takes)", "",
              "| class | " + " | ".join(sess) + " |", "|---|" + "---|" * len(sess)]
    for c in COMMANDS + ["_unknown_", "_silence_"]:
        cells = []
        for s in sess:
            pc = {r["cls"]: r for r in results[s]["_clip_pc"]}.get(c)
            st = {r["cls"]: r for r in results[s]["_stream_pc"]}.get(c)
            cell = f"{pc['correct']}/{pc['support']}" if pc and pc["support"] else "-"
            if st and c in COMMANDS:
                cell += f" · {st['hit'] + st['hit_extra']}/{st['takes']}"
            cells.append(cell)
        lines.append(f"| {c} | " + " | ".join(cells) + " |")
    (out / "compare_sessions.md").write_text("\n".join(lines) + "\n", encoding="utf-8")


# ---------------------------------------------------------------- check
def parse_c_array(text, name, dtype):
    m = re.search(rf"\b{name}\s*\[\s*\d*\s*\]\s*=\s*\{{(.*?)\}};", text, re.S)
    if not m:
        raise ValueError(f"array {name} not found")
    body = re.sub(r"/\*.*?\*/", "", m.group(1), flags=re.S)
    return np.array([float(v.rstrip("fF")) for v in re.findall(r"[-+]?[0-9.]+(?:[eE][-+]?\d+)?[fF]?", body)],
                    dtype=dtype)


def run_check(args, lm, model):
    kc = Path(args.kws_c)
    ok = True
    tables = kc / "kws_feature_tables.h"
    if tables.exists():
        t = tables.read_text()
        win = parse_c_array(t, "KWS_WINDOW", np.float32)
        start = parse_c_array(t, "KWS_MEL_START", np.int64)
        ln = parse_c_array(t, "KWS_MEL_LEN", np.int64)
        w = parse_c_array(t, "KWS_MEL_W", np.float32)
        board = np.zeros_like(lm.basis)
        k = 0
        for b in range(N_MELS):
            board[b, start[b]:start[b] + ln[b]] = w[k:k + ln[b]]
            k += ln[b]
        dw, dm = np.abs(win - lm.win).max(), np.abs(board - lm.basis).max()
        dn = np.abs(board - mel_basis_numpy()).max()
        print(f"window vs board table: max diff {dw:.2e} | mel table ({lm.backend}) vs board: {dm:.2e} | "
              f"numpy mel vs board: {dn:.2e}")
        ok &= dw < 1e-6 and dm < 1e-6 and dn < 1e-6
    t = (kc / "kws_clip_test.h").read_text()
    pcm = parse_c_array(t, "clip_pcm", np.float32)
    exp_f = parse_c_array(t, "expected_logmel", np.float32).reshape(N_MELS, N_FRAMES)
    exp_p = parse_c_array(t, "expected_probs", np.float32)
    F = lm.clip(pcm / 32768.0)
    Fs = lm.frames(np.pad(pcm / 32768.0, (N_FFT // 2, N_FFT // 2)))    # stream path on the same audio
    p = model.probs(F[None])[0]
    df, ds, dp = np.abs(F - exp_f).max(), np.abs(Fs - exp_f).max(), np.abs(p - exp_p).max()
    print(f"train00001.wav: clip features vs kws_clip_test.h max diff {df:.2e} | stream-path features {ds:.2e} | "
          f"probs {dp:.2e} | top {CLASSES[int(p.argmax())]} {p.max():.4f} (expected "
          f"{CLASSES[int(exp_p.argmax())]} {exp_p.max():.4f})")
    ok &= df < 1e-3 and ds < 1e-3 and dp < 1e-3
    tin, texp = Path(args.prefer) / "onnx_out" / "test_input.npy", Path(args.prefer) / "onnx_out" / "test_expected.npy"
    if tin.exists() and texp.exists():
        x = np.load(tin).astype(np.float32).reshape(1, N_MELS, -1)
        d = np.abs(model.probs(x)[0] - np.load(texp).ravel()).max()
        print(f"export test vector (test_input.npy) vs test_expected.npy: max diff {d:.2e}")
        ok &= d < 1e-4
    print("CHECK", "PASSED" if ok else "FAILED")
    return ok


# ---------------------------------------------------------------- main
def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("mode", choices=["check", "clip", "stream", "all"])
    ap.add_argument("--sessions", nargs="+", default=["S1"])
    ap.add_argument("--root", default=str(DEF_ROOT), help="recordings folder (manifest.csv)")
    ap.add_argument("--model", default=str(DEF_PREFER / "onnx_out" / "kws.onnx"), help=".onnx or .pth")
    ap.add_argument("--width", type=int, default=16)
    ap.add_argument("--depth", type=int, default=2)
    ap.add_argument("--export-py", default=None, help="export_kws_onnx.py (for .pth models)")
    ap.add_argument("--model-py", default=None, help="experiments\\<exp>\\model.py: build any network for a .pth")
    ap.add_argument("--tag", default=None, help="report folder name (default: model file name)")
    ap.add_argument("--out", default=str(HERE / "eval"))
    ap.add_argument("--gain-db", type=float, default=15.0, help="board KWS_GAIN_DB")
    ap.add_argument("--features", choices=["auto", "librosa", "numpy"], default="auto")
    ap.add_argument("--features-py", default=str(DEF_FEATURES), help="folder with the team's features.py")
    ap.add_argument("--quality-only", action="store_true", help="only rows with quality_ok = True")
    ap.add_argument("--stride", type=int, default=10)
    ap.add_argument("--avg", type=int, default=3)
    ap.add_argument("--threshold", type=float, default=0.70)
    ap.add_argument("--refractory-ms", type=int, default=1500, help="model_realtime_code KWS_REFRACTORY_MS")
    ap.add_argument("--channel", default="auto", choices=["auto", "0", "1"], help="stream mode: SNR choice or fixed")
    ap.add_argument("--prefer", default=str(DEF_PREFER), help="final_model\\prefer (for check)")
    ap.add_argument("--kws-c", default=None, help="folder with kws_clip_test.h (default <prefer>\\kws_c)")
    args = ap.parse_args()
    args.kws_c = args.kws_c or str(Path(args.prefer) / "kws_c")

    lm = LogMel(args.features, args.features_py)
    model = Model(args.model, args.width, args.depth, args.export_py, args.model_py)
    print(f"features: {lm.backend} | model: {model.kind} {args.model}")
    if args.mode == "check":
        sys.exit(0 if run_check(args, lm, model) else 1)

    tag = args.tag or Path(args.model).stem
    cfg = dict(stride=args.stride, avg=args.avg, thr=args.threshold, refractory_ms=args.refractory_ms,
               channel=args.channel)
    meta = dict(tag=tag, model=str(args.model), model_kind=model.kind, gain_db=args.gain_db, features=lm.backend,
                stream_cfg=cfg, quality_only=args.quality_only, when=time.strftime("%Y-%m-%d %H:%M"))
    results = {}
    for s in args.sessions:
        rows = load_rows(args.root, s, args.quality_only)
        if not rows:
            print(f"{s}: no rows in manifest.csv")
            continue
        t0 = time.time()
        clip = eval_clips(rows, args.root, lm, model, args.gain_db) if args.mode in ("clip", "all") else None
        stream = eval_stream(rows, args.root, s, lm, model, args.gain_db, cfg) if args.mode in ("stream", "all") else None
        summ = write_report(Path(args.out) / tag / s, s, meta, clip, stream)
        summ["_clip_pc"] = clip["per_class"] if clip else []
        summ["_stream_pc"] = stream["per_class"] if stream else []
        results[s] = summ
        print(f"\n=== {tag} on {s} ({len(rows)} rows, {time.time() - t0:.1f} s) -> {Path(args.out) / tag / s}")
        if clip:
            print("clip  ", json.dumps(clip["summary"]))
        if stream:
            print("stream", json.dumps(stream["summary"]))
    if len(results) > 1:
        write_comparison(Path(args.out) / tag, results)


if __name__ == "__main__":
    main()
