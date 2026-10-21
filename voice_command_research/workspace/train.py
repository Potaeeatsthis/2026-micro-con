"""
train.py - train one experiment (experiments\<exp>\model.py: CONFIG + build_model) and score it on
your board recordings.

    python train.py exp02_tcresnet-w16d2_aug-basic --limit 2     quick test (results_test\, runs_test\)
    python train.py exp02_tcresnet-w16d2_aug-basic               all training clips, CONFIG["epochs"] epochs
    python train.py exp01_tcresnet-w16d2_aug-all --folds         5-fold CV (how exp01 found the epoch count)
(python = C:/Users/ASUS/anaconda3/envs/torch_env/python.exe, run from voice_command_research\workspace\)

Default: train on every training clip for a fixed number of epochs, keep the EMA weights
(runs\final.pth), score S0 and S1 with eval_board.py (results\board\), write results\summary.json
and update experiments\leaderboard.csv. Since 21 Oct S1 is shown per experiment (Supanut); the final
test is a later session (S4 undo takes, unseen speakers).
--folds: per fold k train on folds != k (clips, noise, rooms, babble talkers all from folds != k),
score fold k with the same realistic noisy windows for every experiment, early stop on its macro-F1,
robustness curve, board sessions. Fold 0 below FAIL_MACRO_F1 stops the other folds.
"""
import argparse
import copy
import importlib.util
import json
import subprocess
import sys
import time
import zlib
from pathlib import Path

import torch                        # before numpy/librosa (OpenMP, see features.py)
import numpy as np
import pandas as pd
from sklearn.metrics import f1_score

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE.parent))
import config as C
import windows as W
from features import LogMelTorch, WINDOW_SAMPLES

CACHE = C.BUILT / "cache"
EXPERIMENTS = HERE / "experiments"
N_FOLDS = 5
FAIL_MACRO_F1 = 0.20                # fold 0 below this = something is broken, stop before folds 1-4
EVAL_VERSION = 100                  # the fixed version used to score the held-out fold
ROBUST_SNRS = [None, 20, 10, 5]     # None = clean (quiet floor only)
ROBUST_PER_CLASS = 200
STATS_WINDOWS = 4000
WORKERS = 6
SILENCE_LEVEL_DB = (-70.0, -35.0)   # rms of _silence_ windows
BOARD_SESSIONS = ["S0", "S1"]       # S1 shown per experiment since 21 Oct (Supanut); final test = later sessions
DEVICE = "cuda"


# ---------------------------------------------------------------- windows
class Windows(torch.utils.data.Dataset):
    """key = (clip row, version) -> rendered window; every random choice is seeded by the key,
    the held-out fold and the side (train / eval), so any window can be rebuilt exactly.
    The augmentation switches in cfg apply to the train side only; eval always gets everything."""

    use_babble = True                   # default for objects pickled before the babble switch existed

    def __init__(self, held_out, side, cfg, force_snr=False, snr=None):
        self.held_out, self.side, self.force_snr, self.snr = held_out, side, force_snr, snr
        self.noise_only_share = cfg["noise_only_share_silence"]
        full = side == "eval"
        self.use_noise = full or cfg["noise"]
        self.use_reverb = full or cfg["reverb"]
        self.use_babble = full or cfg["babble"]
        self.stream_positions = full or cfg["stream_positions"]
        self.clips = pd.read_csv(CACHE / "clips.csv", low_memory=False)
        noise = pd.read_csv(CACHE / "noise.csv")
        rirs = pd.read_csv(CACHE / "rirs.csv")
        use = (lambda f: f == held_out) if side == "eval" else (lambda f: f != held_out)
        self.noise = noise[use(noise["fold"])]
        self.rirs = rirs[use(rirs["fold"])]
        talkers = self.clips[(self.clips["label"] == C.UNKNOWN) & (self.clips["set"] == "train")]
        self.talkers = talkers[use(talkers["fold"])].index.to_numpy()
        self.label_index = {c: i for i, c in enumerate(C.CLASSES)}
        self.audio = self.noise_audio = self.rir_audio = None

    def _open(self):
        if self.audio is None:
            self.audio = np.load(CACHE / "clips.npy", mmap_mode="r")
            self.noise_audio = np.load(CACHE / "noise.npy", mmap_mode="r")
            self.rir_audio = np.load(CACHE / "rirs.npy", mmap_mode="r")

    def piece(self, row):
        r = self.clips.iloc[row]
        return np.asarray(self.audio[r.offset:r.offset + r.length], np.float32) / 32768, r

    def noise_piece(self, rng, kinds):
        pool = self.noise[self.noise["kind"].str.startswith("esc50")] if kinds == "event" \
            else self.noise[self.noise["kind"].isin(kinds)]
        for _ in range(20):
            r = pool.iloc[int(rng.integers(len(pool)))]
            piece = W.noise_segment(np.asarray(self.noise_audio[r.offset:r.offset + r.length], np.float32) / 32768,
                                    rng)
            if piece is not None:
                return piece, r.kind
        raise RuntimeError(f"no usable noise for {kinds}")

    def background(self, rng, kind):
        if kind == "babble":
            picks = rng.choice(self.talkers, int(rng.integers(W.BABBLE_TALKERS[0], W.BABBLE_TALKERS[1] + 1)),
                               replace=False)
            return W.babble([self.piece(p)[0] for p in picks], rng), "babble"
        kinds = {"quiet": W.QUIET_KINDS, "room": W.ROOM_KINDS, "event": "event"}[kind]
        return self.noise_piece(rng, kinds)

    def render(self, row, version):
        self._open()
        audio, r = self.piece(row)
        rng = np.random.default_rng([zlib.crc32(r.audio_path.encode()), version, self.held_out + 1,
                                     0 if self.side == "train" else 1])
        if r.label == C.SILENCE:
            if rng.random() < self.noise_only_share:
                audio, note = self.noise_piece(rng, W.ROOM_KINDS if rng.random() < 0.5 else "event")
            else:
                audio, note = np.resize(audio, WINDOW_SAMPLES), "silence_clip"
            level = rng.uniform(*SILENCE_LEVEL_DB)
            window = audio / (np.sqrt(np.mean(audio ** 2)) + 1e-9) * 10 ** (level / 20)
            return window.astype(np.float32), dict(kind=note, level_db=round(level, 2))
        reverb = ""
        if self.use_reverb and not self.force_snr and rng.random() < W.REVERB_SHARE:
            pool = self.rirs[self.rirs["kind"] == rng.choice(["real", "sim_small"])]   # 50/50, not 325 vs 20,000
            rr = pool.iloc[int(rng.integers(len(pool)))]
            audio = W.reverb(audio, np.asarray(self.rir_audio[rr.offset:rr.offset + rr.length], np.float32))
            reverb = rr.kind
        kind, snr = W.noise_kind_of(rng)
        if not self.use_noise:
            kind, snr = "quiet", float(rng.uniform(*W.QUIET_SNR_RANGE_DB))
        if kind == "babble" and not self.use_babble:
            kind = str(rng.choice(["room", "event"], p=[2 / 3, 1 / 3]))     # keeps room:event at 2:1
        if self.force_snr:
            kind, snr = ("quiet", 40.0) if self.snr is None else (str(rng.choice(["room", "event", "babble"])),
                                                                 float(self.snr))
        background, noise_kind = self.background(rng, kind)
        word_len = (r.word_end - r.word_start) / C.SR
        params = W.sample_params(rng, word_len)
        params["snr_db"] = round(snr, 2)
        if not self.stream_positions:
            params["offset_s"] = round((W.WINDOW_S - word_len) / 2, 4)
        window = W.make_window(audio, r.word_start / C.SR, r.word_end / C.SR, background, params)
        return window, dict(kind=kind, noise=noise_kind, reverb=reverb, **params)

    def __len__(self):
        return len(self.clips) * 1000

    def __getitem__(self, key):
        row, version = divmod(int(key), 1000)
        window, params = self.render(row, version)
        return window, self.label_index[self.clips.iloc[row].label], json.dumps(params)


# ---------------------------------------------------------------- sampling
def epoch_keys(clips, train_rows, cfg, rng):
    """class-balanced draws (or a set unknown_share for _unknown_); Piper and rhyme shares fixed inside _unknown_"""
    n_unknown = int(cfg["windows_per_epoch"] * cfg.get("unknown_share", 1 / len(C.CLASSES)))
    per_class = (cfg["windows_per_epoch"] - n_unknown) // (len(C.CLASSES) - 1)
    keys = []
    for label in C.CLASSES:
        rows = train_rows[clips.loc[train_rows, "label"].to_numpy() == label]
        if label == C.UNKNOWN:
            source = clips.loc[rows, "source"].to_numpy()
            n_rhyme = int(round(n_unknown * cfg.get("rhyme_share_unknown", 0)))
            n_piper = int(round(n_unknown * cfg["piper_share_unknown"]))
            picks = [rng.choice(rows[source == "piper"], n_piper),
                     rng.choice(rows[~np.isin(source, ["piper", "piper_rhyme"])], n_unknown - n_piper - n_rhyme)]
            if n_rhyme:
                picks.append(rng.choice(rows[source == "piper_rhyme"], n_rhyme))
            picks = np.concatenate(picks)
        else:
            picks = rng.choice(rows, per_class)
        keys.append(picks * 1000 + rng.integers(0, cfg["versions"], len(picks)))
    keys = np.concatenate(keys)
    rng.shuffle(keys)
    return keys


class Keys(torch.utils.data.Sampler):
    """the keys of the current epoch; set_keys() before each epoch so the workers can stay alive"""

    def __init__(self, keys=()):
        self.keys = list(keys)

    def set_keys(self, keys):
        self.keys = list(keys)

    def __iter__(self):
        return iter(self.keys)

    def __len__(self):
        return len(self.keys)


def cap_mswc(clips, rows, cap, seed):
    """keep at most `cap` MSWC clips per command class (random, fixed seed); _unknown_ / _silence_ untouched"""
    rng = np.random.default_rng(seed)
    labels, sources = clips["label"].to_numpy()[rows], clips["source"].to_numpy()[rows]
    is_mswc = np.isin(sources, ["open_mswc", "open_mswc_joined"])
    keep = ~is_mswc | np.isin(labels, [C.UNKNOWN, C.SILENCE])
    for label in C.TOKENS:
        idx = np.flatnonzero(is_mswc & (labels == label))
        keep[rng.choice(idx, min(cap, len(idx)), replace=False)] = True
    print(f"  MSWC cap {cap} per command: {len(rows)} -> {int(keep.sum())} training clips")
    return rows[keep]


def loader(dataset, keys, batch, workers=WORKERS, persistent=False):
    sampler = keys if isinstance(keys, Keys) else Keys(keys)
    return torch.utils.data.DataLoader(dataset, batch_size=batch, sampler=sampler, num_workers=workers,
                                       persistent_workers=persistent, pin_memory=True)


def render_all(dataset, keys, logmel, batch=512):
    """render + features for a fixed key list -> (features on GPU as float16, labels, params)"""
    feats, labels, params = [], [], []
    with torch.no_grad():
        for x, y, p in loader(dataset, keys, batch):
            feats.append(logmel(x.to(DEVICE)).half())
            labels.append(y)
            params += list(p)
    return torch.cat(feats), torch.cat(labels).numpy(), params


# ---------------------------------------------------------------- scoring
def predict(model, feats, batch=2048):
    model.eval()
    out = []
    with torch.no_grad():
        for i in range(0, len(feats), batch):
            out.append(model(feats[i:i + batch].float()).argmax(1).cpu())
    return torch.cat(out).numpy()


def scores(y, pred):
    labels = list(range(len(C.CLASSES)))
    return dict(accuracy=float(np.mean(y == pred)),
                macro_f1=float(f1_score(y, pred, labels=labels, average="macro", zero_division=0)))


def breakdown(y, pred, meta, column):
    rows = []
    for value, idx in meta.groupby(column).indices.items():
        rows.append(dict(**{column: value}, n=len(idx), recall=round(float(np.mean(y[idx] == pred[idx])), 4)))
    return pd.DataFrame(rows)


def per_class(y, pred):
    f1 = f1_score(y, pred, labels=list(range(len(C.CLASSES))), average=None, zero_division=0)
    return pd.DataFrame(dict(label=C.CLASSES, n=np.bincount(y, minlength=len(C.CLASSES)),
                             recall=[round(float(np.mean(pred[y == i] == i)), 4) if np.any(y == i) else np.nan
                                     for i in range(len(C.CLASSES))],
                             f1=np.round(f1, 4)))


# ---------------------------------------------------------------- training
def load_experiment(name):
    spec = importlib.util.spec_from_file_location("experiment_model", EXPERIMENTS / name / "model.py")
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def norm_stats(dataset, train_rows, cfg, logmel, rng):
    keys = rng.choice(train_rows, STATS_WINDOWS) * 1000
    feats, _, _ = render_all(dataset, keys, logmel)
    feats = feats.float()
    return feats.mean(dim=(0, 2)).cpu(), feats.std(dim=(0, 2)).cpu()


def spec_augment(feats, mean, cfg, gen):
    """1 time mask and 1 band mask on a share of the batch; masked values = per-band training mean"""
    b, bands, frames = feats.shape
    chosen = torch.rand(b, device=DEVICE, generator=gen) < cfg["specaug_share"]
    t_len = torch.randint(0, cfg["time_mask_max"] + 1, (b,), device=DEVICE, generator=gen)
    t_start = (torch.rand(b, device=DEVICE, generator=gen) * (frames - t_len)).long()
    f_len = torch.randint(0, cfg["freq_mask_max"] + 1, (b,), device=DEVICE, generator=gen)
    f_start = (torch.rand(b, device=DEVICE, generator=gen) * (bands - f_len)).long()
    t = torch.arange(frames, device=DEVICE)[None, :]
    f = torch.arange(bands, device=DEVICE)[None, :]
    t_mask = (t >= t_start[:, None]) & (t < (t_start + t_len)[:, None])
    f_mask = (f >= f_start[:, None]) & (f < (f_start + f_len)[:, None])
    mask = (t_mask[:, None, :] | f_mask[:, :, None]) & chosen[:, None, None]
    return torch.where(mask, mean.to(DEVICE).reshape(1, -1, 1).expand_as(feats), feats)


def train_run(exp, cfg, held_out, out_dir, run_dir, limit):
    """one training run; held_out = -1 trains on every fold (final model)"""
    torch.manual_seed(cfg["seed"])
    rng = np.random.default_rng(cfg["seed"] + held_out + 1)
    gen = torch.Generator(device=DEVICE).manual_seed(cfg["seed"])
    logmel = LogMelTorch().to(DEVICE)
    train_set = Windows(held_out, "train", cfg)
    clips = train_set.clips
    is_train = (clips["set"] == "train").to_numpy()
    train_rows = np.flatnonzero(is_train & (clips["fold"].to_numpy() != held_out))
    if cfg.get("mswc_cap_per_class"):
        train_rows = cap_mswc(clips, train_rows, cfg["mswc_cap_per_class"], cfg["seed"])
    if limit:
        cfg = dict(cfg, windows_per_epoch=2000, max_epochs=limit)

    mean, std = norm_stats(train_set, train_rows, cfg, logmel, rng)
    model = exp.build_model(mean, std, len(C.CLASSES)).to(DEVICE)
    ema = copy.deepcopy(model)
    opt = torch.optim.AdamW(model.parameters(), lr=cfg["lr"], weight_decay=cfg["weight_decay"])
    steps_per_epoch = -(-cfg["windows_per_epoch"] // cfg["batch"])
    sched = torch.optim.lr_scheduler.OneCycleLR(opt, cfg["lr"], total_steps=cfg["max_epochs"] * steps_per_epoch,
                                                pct_start=0.05, anneal_strategy="cos")
    loss_fn = torch.nn.CrossEntropyLoss(label_smoothing=cfg["label_smoothing"])

    eval_data = None
    if held_out >= 0:
        eval_set = Windows(held_out, "eval", cfg)
        eval_rows = np.flatnonzero(is_train & (clips["fold"].to_numpy() == held_out))
        if limit:
            eval_rows = rng.choice(eval_rows, 2000, replace=False)
        t0 = time.time()
        eval_data = render_all(eval_set, eval_rows * 1000 + EVAL_VERSION, logmel)
        print(f"  held-out fold {held_out}: {len(eval_rows)} windows rendered ({time.time() - t0:.0f} s)")
        pd.DataFrame([dict(row=r, audio_path=clips.iloc[r].audio_path, label=clips.iloc[r].label, **json.loads(p))
                      for r, p in zip(eval_rows, eval_data[2])]).to_csv(out_dir / "eval_windows.csv", index=False)

    history, best, best_epoch, waited = [], -1.0, 0, 0
    keys = Keys()
    train_loader = loader(train_set, keys, cfg["batch"], persistent=True)
    for epoch in range(1, cfg["max_epochs"] + 1):
        t0 = time.time()
        model.train()
        total, seen = 0.0, 0
        keys.set_keys(epoch_keys(clips, train_rows, cfg, rng))
        for x, y, _ in train_loader:
            x, y = x.to(DEVICE, non_blocking=True), y.to(DEVICE, non_blocking=True)
            gain = 10 ** (torch.empty(len(x), 1, device=DEVICE).uniform_(-cfg["gain_jitter_db"],
                                                                         cfg["gain_jitter_db"], generator=gen) / 20)
            feats = logmel(x * gain)
            if cfg["specaug"]:
                feats = spec_augment(feats, mean, cfg, gen)
            loss = loss_fn(model(feats), y)
            opt.zero_grad(set_to_none=True)
            loss.backward()
            opt.step()
            sched.step()
            with torch.no_grad():
                for p_ema, p in zip(ema.parameters(), model.parameters()):
                    p_ema.mul_(cfg["ema_decay"]).add_(p, alpha=1 - cfg["ema_decay"])
                for b_ema, b in zip(ema.buffers(), model.buffers()):
                    b_ema.copy_(b)
            total += loss.item() * len(y)
            seen += len(y)
        row = dict(epoch=epoch, loss=round(total / seen, 4), lr=round(sched.get_last_lr()[0], 6),
                   seconds=round(time.time() - t0, 1))
        if eval_data is not None:
            s = scores(eval_data[1], predict(ema, eval_data[0]))
            row.update(val_accuracy=round(s["accuracy"], 4), val_macro_f1=round(s["macro_f1"], 4))
            if s["macro_f1"] > best:
                best, best_epoch, waited = s["macro_f1"], epoch, 0
                torch.save(ema.state_dict(), run_dir / "best.pth")
            else:
                waited += 1
        history.append(row)
        print("  " + " | ".join(f"{k} {v}" for k, v in row.items()))
        pd.DataFrame(history).to_csv(out_dir / "history.csv", index=False)
        if eval_data is not None and waited >= cfg["patience"]:
            print(f"  early stop: no gain for {cfg['patience']} epochs")
            break
    if eval_data is None:
        torch.save(ema.state_dict(), run_dir / "final.pth")
    return best_epoch, eval_data, logmel


def evaluate_fold(exp, held_out, eval_data, logmel, out_dir, run_dir, limit):
    clips = pd.read_csv(CACHE / "clips.csv", low_memory=False)
    model = exp.build_model(torch.zeros(40), torch.ones(40), len(C.CLASSES)).to(DEVICE)
    model.load_state_dict(torch.load(run_dir / "best.pth", map_location=DEVICE))
    feats, y, params = eval_data
    pred = predict(model, feats)
    rows = pd.read_csv(out_dir / "eval_windows.csv")["row"].to_numpy()
    meta = clips.iloc[rows].reset_index(drop=True).assign(gender=lambda d: d["gender"].fillna("unknown"))
    per_class(y, pred).to_csv(out_dir / "per_class.csv", index=False)
    breakdown(y, pred, meta, "source").to_csv(out_dir / "per_source.csv", index=False)
    breakdown(y, pred, meta, "gender").to_csv(out_dir / "per_gender.csv", index=False)
    summary = dict(fold=held_out, **{f"val_{k}": round(v, 4) for k, v in scores(y, pred).items()})

    robust = []                     # same clips, forced SNR, no echo, ROBUST_PER_CLASS per class
    rng = np.random.default_rng(held_out)
    clips_eval = clips.iloc[rows]
    pick = np.concatenate([rng.choice(idx, min(len(idx), 50 if limit else ROBUST_PER_CLASS), replace=False)
                           for idx in clips_eval.groupby("label").indices.values()])
    for snr in ROBUST_SNRS:
        ds = Windows(held_out, "eval", exp.CONFIG, force_snr=True, snr=snr)
        f, yy, _ = render_all(ds, rows[pick] * 1000 + EVAL_VERSION, logmel)
        s = scores(yy, predict(model, f))
        robust.append(dict(snr_db="clean" if snr is None else snr, n=len(yy), **{k: round(v, 4) for k, v in s.items()}))
    pd.DataFrame(robust).to_csv(out_dir / "robustness.csv", index=False)
    summary.update({f"macro_f1_{r['snr_db']}": r["macro_f1"] for r in robust})
    return summary


def board_eval(model_py, tag, model_path, out_dir):
    """eval_board.py on the board sessions; returns its stream / clip summaries"""
    result = {}
    for session in BOARD_SESSIONS:
        cmd = [sys.executable, str(HERE / "eval_board.py"), "all", "--sessions", session,
               "--model", str(model_path), "--model-py", str(model_py),
               "--tag", tag, "--out", str(out_dir / "board")]
        done = subprocess.run(cmd, capture_output=True, text=True, cwd=HERE)
        if done.returncode != 0:
            print(done.stdout[-2000:], done.stderr[-2000:])
            continue
        s = json.loads((out_dir / "board" / tag / session / "summary.json").read_text())
        result.update({f"{session}_clip_macro_f1": s["clip"]["macro_f1"],
                       f"{session}_command_accuracy": s["clip"]["command_accuracy"],
                       f"{session}_stream_detected": s["stream"]["detected_rate"],
                       f"{session}_unknown_false_alarms": s["stream"]["unknown_false_alarms"]})
    return result


def git_commit():
    done = subprocess.run(["git", "rev-parse", "--short", "HEAD"], capture_output=True, text=True, cwd=HERE)
    dirty = subprocess.run(["git", "status", "--porcelain", "."], capture_output=True, text=True, cwd=HERE)
    return done.stdout.strip() + (" (uncommitted changes)" if dirty.stdout.strip() else "")


def check_cache():
    table = pd.read_csv(C.BUILT / "clips.csv", low_memory=False)
    expected = int((table["keep"] & table["set"].isin(["train", "dev"])).sum())
    cached = len(pd.read_csv(CACHE / "clips.csv", usecols=["label"]))
    if expected != cached:
        raise SystemExit(f"stale cache: clips.csv has {expected} rows, cache {cached}. Run cache_audio.py")


def update_leaderboard():
    """experiments\\leaderboard.csv: one row per experiment (all-data run, or the mean of its folds)"""
    rows = []
    for exp_dir in sorted(p for p in EXPERIMENTS.iterdir() if (p / "model.py").exists()):
        all_run = exp_dir / "results" / "summary.json"
        folds = [json.loads(p.read_text()) for p in sorted(exp_dir.glob("results/fold*/summary.json"))]
        if all_run.exists():
            scores_, how = json.loads(all_run.read_text()), "all data"
        elif len(folds) == N_FOLDS:
            scores_, how = pd.DataFrame(folds).drop(columns="fold").mean().round(4).to_dict(), "mean of 5 folds"
        else:
            continue
        exp = load_experiment(exp_dir.name)
        cfg = exp.CONFIG
        n_params = sum(p.numel() for p in exp.build_model(torch.zeros(40), torch.ones(40), len(C.CLASSES))
                       .parameters())
        rows.append(dict(experiment=exp_dir.name, trained_on=how, architecture=cfg["architecture"],
                         width=cfg["width"], depth=cfg["depth"], params=n_params,
                         stream_positions=cfg["stream_positions"], noise=cfg["noise"], babble=cfg["babble"],
                         reverb=cfg["reverb"], specaug=cfg["specaug"],
                         mswc_cap=cfg.get("mswc_cap_per_class", ""),
                         unknown_share=round(cfg.get("unknown_share", 1 / len(C.CLASSES)), 3),
                         rhyme_share=cfg.get("rhyme_share_unknown", 0),
                         **{k: v for k, v in scores_.items() if k.startswith(("S0_", "S1_"))}))
    if rows:
        board = pd.DataFrame(rows).sort_values("S0_clip_macro_f1", ascending=False)
        board.to_csv(EXPERIMENTS / "leaderboard.csv", index=False)
        print(board.to_string(index=False))


def run_all(name, exp, limit):
    """train on every training clip for CONFIG["epochs"] epochs, then score the board sessions"""
    cfg = exp.CONFIG
    if not cfg.get("epochs") and not limit:
        raise SystemExit(f"{name}: set CONFIG['epochs'] in model.py first")
    results = EXPERIMENTS / name / ("results_test" if limit else "results")
    run_dir = EXPERIMENTS / name / ("runs_test" if limit else "runs")
    results.mkdir(parents=True, exist_ok=True)
    run_dir.mkdir(parents=True, exist_ok=True)
    (results / "config.json").write_text(json.dumps(dict(cfg, commit=git_commit()), indent=2))
    print(f"{name}: all training clips, {cfg['epochs']} epochs")
    t0 = time.time()
    train_run(exp, dict(cfg, max_epochs=cfg["epochs"] or limit), -1, results, run_dir, limit)
    summary = dict(epochs=cfg["epochs"] if not limit else limit, minutes=round((time.time() - t0) / 60, 1))
    summary.update(board_eval(EXPERIMENTS / name / "model.py", name, run_dir / "final.pth", results))
    (results / "summary.json").write_text(json.dumps(summary, indent=2))
    print("  " + json.dumps(summary))


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("experiment", help="folder name in experiments\\")
    ap.add_argument("--folds", action="store_true", help="5-fold cross-validation instead of one all-data run")
    ap.add_argument("--fold", type=int, help="with --folds: only this fold")
    ap.add_argument("--limit", type=int, help="quick test: N short epochs, results_test\\")
    args = ap.parse_args()
    check_cache()
    exp = load_experiment(args.experiment)
    cfg = exp.CONFIG
    if not args.folds:
        run_all(args.experiment, exp, args.limit)
        if not args.limit:
            update_leaderboard()
        return
    results = EXPERIMENTS / args.experiment / ("results_test" if args.limit else "results")
    runs = EXPERIMENTS / args.experiment / ("runs_test" if args.limit else "runs")

    folds = [args.fold] if args.fold is not None else range(N_FOLDS)
    for k in folds:
        out_dir, run_dir = results / f"fold{k}", runs / f"fold{k}"
        if (out_dir / "summary.json").exists():
            print(f"fold {k}: finished, skipped")
            continue
        out_dir.mkdir(parents=True, exist_ok=True)
        run_dir.mkdir(parents=True, exist_ok=True)
        (out_dir / "config.json").write_text(json.dumps(dict(cfg, commit=git_commit()), indent=2))
        print(f"fold {k}")
        t0 = time.time()
        best_epoch, eval_data, logmel = train_run(exp, cfg, k, out_dir, run_dir, args.limit)
        summary = evaluate_fold(exp, k, eval_data, logmel, out_dir, run_dir, args.limit)
        summary.update(best_epoch=best_epoch, minutes=round((time.time() - t0) / 60, 1))
        summary.update(board_eval(EXPERIMENTS / args.experiment / "model.py", f"{args.experiment}_fold{k}",
                                  run_dir / "best.pth", out_dir))
        (out_dir / "summary.json").write_text(json.dumps(summary, indent=2))
        print("  " + json.dumps(summary))
        if k == 0 and not args.limit and summary["val_macro_f1"] < FAIL_MACRO_F1:
            print(f"fold 0 macro-F1 {summary['val_macro_f1']} < {FAIL_MACRO_F1}: something is broken, "
                  f"folds 1-4 not run")
            break

    done = [json.loads(p.read_text()) for p in sorted(results.glob("fold*/summary.json"))]
    table = pd.DataFrame(done)
    if len(table) > 1:
        numeric = table.drop(columns="fold")
        table = pd.concat([table, pd.DataFrame([dict(fold="mean", **numeric.mean().round(4)),
                                                dict(fold="std", **numeric.std().round(4))])])
    table.to_csv(results / "summary.csv", index=False)
    print(table.to_string(index=False))
    if not args.limit:
        update_leaderboard()


if __name__ == "__main__":
    main()
