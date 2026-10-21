"""
make_synthetic.py - text-to-speech training clips with Piper: many speakers, 50/50 male/female,
a new speed / timing / intonation for every clip.

    C:/Users/ASUS/anaconda3/envs/torch_env/python.exe make_synthetic.py --limit 5     quick test
    C:/Users/ASUS/anaconda3/envs/torch_env/python.exe make_synthetic.py               full run (resumes)

Step 1 (once): every speaker says TEST_SENTENCE; median pitch -> estimated gender -> speakers.csv.
Step 2: CLIPS_PER_WORD clips per class -> synthetic/clips/<class>/*.wav + one row each in clips.csv.
Spoken part of every clip <= MAX_SPEECH_S (a take that is longer is made again, faster).
The word is trimmed to its spoken part + PAD_S on each side; level and position in the 1 s window
are set later, when a training dataset is built.

Piper adds its own random noise, so a rerun does not give identical audio: clips are made once
and kept. Group splits by speaker_key (one speaker never on both sides of a split).
"""
import argparse
import csv
import random
import time
import wave
import zlib
from pathlib import Path

import librosa
import numpy as np
from piper import PiperVoice, SynthesisConfig

import sys
sys.path.insert(0, str(Path(__file__).resolve().parent.parent))
import config as C

VOICES_DIR = C.SYNTHETIC / "voices"
CLIPS_DIR = C.SYNTHETIC / "clips"
SPEAKERS_CSV = C.SYNTHETIC / "speakers.csv"
CLIPS_CSV = C.SYNTHETIC / "clips.csv"
VOICES = ["en_US-libritts_r-medium", "en_GB-vctk-medium"]

# clips per class (weak words get more; every word gets some so "Piper voice" is no shortcut)
CLIPS_PER_WORD = {
    **{w: 1500 for w in ["undo", "grab", "blue box", "black box", "green box", "red box", "pick", "drop"]},
    **{w: 1000 for w in ["place", "move", "base", "hand", "station", "home"]},
    **{w: 500 for w in ["backward", "forward", "no", "stop", "yes", "left", "right"]},
    C.UNKNOWN: 12000,                 # 0..9999 near-miss + MSWC words, 10000..11999 rhymes (RHYME_START)
}
# _unknown_ clips 0..NEAR_MISS_CLIPS-1 cycle through NEAR_MISS_WORDS; later clips through EXTRA_WORDS,
# short words from the MSWC _unknown_ pool on disk, so Piper unknowns are many words, not a few
NEAR_MISS_CLIPS = 2000
N_EXTRA_WORDS = 1000
EXTRA_MAX_LETTERS = 8
SEED_WORDS = 3
MSWC_QC = C.DA_DATA / "interim" / "qc" / "manifest_qc.csv"
HOMOPHONES = {"write", "rite", "wright", "know", "bass", "plaice", "foreword", "undue", "pic"}
# near-misses of the commands + everyday words. No homophones of commands.
NEAR_MISS_WORDS = [
    "undone", "under", "until", "redo", "remove", "moved", "movie", "groove", "grass", "grand", "crab",
    "pink", "picked", "kick", "box", "boxes", "blue", "green", "red", "black", "socks", "fox", "band",
    "handle", "nation", "case", "face", "play", "space", "drip", "drum", "top", "shop", "step", "hope",
    "phone", "foam", "yeah", "yet", "less", "light", "lift", "bright", "for", "back", "ward",
    "hello", "robot", "table", "water", "seven", "happy", "music", "orange", "window", "coffee",
    "computer", "paper", "maybe", "okay", "turn", "open", "close", "start", "again", "please",
]
# _unknown_ clips from RHYME_START on: rhymes / sound-alikes of the commands (the false alarms on the board).
# Never a word of the board unknown takes (checked in make_clips), so those takes stay a fair check.
RHYME_START = 10000
RHYME_CLIPS = 2000
RHYME_WORDS = [
    "prove", "lose", "moose", "smooth",                 # move
    "tick", "stick", "brick", "thick",                  # pick
    "cab", "grub", "glad", "slab",                      # grab
    "rocks", "locks", "boss", "blue socks", "red fox", "green rocks", "black socks",   # the boxes
    "sand", "land", "stand",                            # hand
    "vacation", "statue", "patient",                    # station
    "vase", "chase", "race", "pace",                    # base
    "toward", "coward", "onward",                       # forward
    "awkward", "backyard",                              # backward
    "theft", "laughed", "loft",                         # left
    "night", "white", "kite", "ripe",                   # right
    "plate", "lace", "plays",                           # place
    "crop", "prop", "chop",                             # drop
    "guess", "mess", "chess",                           # yes
    "snow", "low", "show", "toe",                       # no
    "comb", "dome", "roam",                             # home
    "spot", "stomp", "mop", "swap",                     # stop
    "into", "onto", "indoor",                           # undo
]

TEST_SENTENCE = "The quick brown fox jumps over the lazy dog, and then it runs home."
MALE_MAX_HZ, FEMALE_MIN_HZ = 145.0, 175.0     # between = "unclear", left out of the pool

MAX_SPEECH_S = 0.90
MAX_TRIES = 5
LENGTH_MEDIAN, LENGTH_SIGMA, LENGTH_RANGE = 1.25, 0.22, (0.8, 2.0)
NOISE_W_RANGE = (0.6, 1.3)
NOISE_RANGE = (0.5, 0.9)
ENDINGS = ["", ".", "!"]
TRIM_DB = 35
PAD_S = 0.10

SPEAKER_FIELDS = ["speaker_key", "voice", "speaker_id", "dataset_speaker", "f0_median_hz", "gender_est"]
CLIP_FIELDS = ["clip_path", "label", "word", "text", "speaker_key", "gender_est", "length_scale",
               "noise_w_scale", "noise_scale", "speech_s", "tries"]


# ---------------------------------------------------------------- audio helpers
def synth(voice, text, config):
    """text -> float32 at C.SR"""
    chunks = list(voice.synthesize(text, syn_config=config))
    audio = np.concatenate([c.audio_float_array for c in chunks])
    return librosa.resample(audio, orig_sr=chunks[0].sample_rate, target_sr=C.SR).astype(np.float32)


def speech_bounds(audio):
    """(start, end) samples of the 10 ms frames within TRIM_DB of the loudest frame"""
    hop = C.SR // 100
    frames = audio[: len(audio) // hop * hop].reshape(-1, hop)
    db = 10 * np.log10((frames ** 2).mean(axis=1) + 1e-12)
    active = np.flatnonzero(db > db.max() - TRIM_DB)
    return active[0] * hop, (active[-1] + 1) * hop


def write_wav(path, audio):
    path.parent.mkdir(parents=True, exist_ok=True)
    with wave.open(str(path), "wb") as w:
        w.setnchannels(1)
        w.setsampwidth(2)
        w.setframerate(C.SR)
        w.writeframes((np.clip(audio, -1, 1) * 32767).astype("<i2").tobytes())


def read_csv(path):
    if not path.exists():
        return []
    with open(path, newline="", encoding="utf-8") as f:
        return list(csv.DictReader(f))


def append_csv(path, fields, row):
    new = not path.exists()
    with open(path, "a", newline="", encoding="utf-8") as f:
        writer = csv.DictWriter(f, fieldnames=fields)
        if new:
            writer.writeheader()
        writer.writerow(row)


def extra_unknown_words():
    """N_EXTRA_WORDS short MSWC _unknown_ words: letters only, not a command, homophone or near-miss"""
    import pandas as pd
    qc = pd.read_csv(MSWC_QC, usecols=["word", "source", "cls", "keep"])
    pool = qc[(qc["source"] == "mswc") & (qc["cls"] == C.UNKNOWN) & qc["keep"]]["word"].dropna().unique()
    blocked = set(C.TOKENS) | HOMOPHONES | set(NEAR_MISS_WORDS)
    words = sorted(w for w in pool if w.isalpha() and len(w) <= EXTRA_MAX_LETTERS and w not in blocked)
    return sorted(random.Random(SEED_WORDS).sample(words, N_EXTRA_WORDS))


def unknown_word(index, extra_words):
    if index >= RHYME_START:
        return RHYME_WORDS[(index - RHYME_START) % len(RHYME_WORDS)]
    if index < NEAR_MISS_CLIPS:
        return NEAR_MISS_WORDS[index % len(NEAR_MISS_WORDS)]
    return extra_words[(index - NEAR_MISS_CLIPS) % len(extra_words)]


# ---------------------------------------------------------------- step 1: speakers
def scan_speakers(voices, limit):
    done = {r["speaker_key"] for r in read_csv(SPEAKERS_CSV)}
    for name, voice in voices.items():
        id_map = voice.config.speaker_id_map
        todo = sorted(id_map.items(), key=lambda kv: kv[1])[:limit or None]
        todo = [(ds, i) for ds, i in todo if f"{name}:{i}" not in done]
        t0 = time.time()
        for n, (dataset_speaker, speaker_id) in enumerate(todo, 1):
            audio = synth(voice, TEST_SENTENCE, SynthesisConfig(speaker_id=speaker_id))
            f0, voiced, _ = librosa.pyin(audio, fmin=60, fmax=400, sr=C.SR)
            f0_median = float(np.nanmedian(f0[voiced])) if voiced.any() else float("nan")
            gender = ("male" if f0_median < MALE_MAX_HZ else "female" if f0_median > FEMALE_MIN_HZ
                      else "unclear")
            append_csv(SPEAKERS_CSV, SPEAKER_FIELDS, dict(
                speaker_key=f"{name}:{speaker_id}", voice=name, speaker_id=speaker_id,
                dataset_speaker=dataset_speaker, f0_median_hz=round(f0_median, 1), gender_est=gender))
            if n % 50 == 0:
                print(f"  {name}: {n}/{len(todo)} speakers ({time.time() - t0:.0f} s)")
    rows = read_csv(SPEAKERS_CSV)
    for name in voices:
        counts = {g: sum(r["voice"] == name and r["gender_est"] == g for r in rows)
                  for g in ("male", "female", "unclear")}
        print(f"speakers {name}: {counts}")
    return rows


# ---------------------------------------------------------------- step 2: clips
def clip_path(label, index, word):
    folder = C.token_dir(label)
    stem = f"{index:05d}_{C.token_dir(word)}" if label == C.UNKNOWN else f"{folder}_{index:05d}"
    return CLIPS_DIR / folder / f"{stem}.wav"


def make_clip(voices, pool, label, index, extra_words):
    """one clip; every random choice comes from a seed made of the clip name"""
    rng = random.Random(zlib.crc32(f"{label}_{index}".encode()))
    word = unknown_word(index, extra_words) if label == C.UNKNOWN else label
    speaker = rng.choice(pool[rng.choice(["male", "female"])])
    voice = voices[speaker["voice"]]
    text = word + rng.choice(ENDINGS)
    length = min(max(rng.lognormvariate(np.log(LENGTH_MEDIAN), LENGTH_SIGMA), LENGTH_RANGE[0]), LENGTH_RANGE[1])
    noise_w, noise = rng.uniform(*NOISE_W_RANGE), rng.uniform(*NOISE_RANGE)
    for tries in range(1, MAX_TRIES + 1):
        config = SynthesisConfig(speaker_id=int(speaker["speaker_id"]), length_scale=length,
                                 noise_w_scale=noise_w, noise_scale=noise)
        audio = synth(voice, text, config)
        start, end = speech_bounds(audio)
        speech_s = (end - start) / C.SR
        if speech_s <= MAX_SPEECH_S:
            break
        length *= 0.95 * MAX_SPEECH_S / speech_s          # too long: same speaker, faster
    else:
        return None
    pad = int(PAD_S * C.SR)
    audio = audio[max(start - pad, 0): end + pad]
    path = clip_path(label, index, word)
    write_wav(path, audio)
    return dict(clip_path=path.relative_to(C.SYNTHETIC).as_posix(), label=label, word=word, text=text,
                speaker_key=speaker["speaker_key"], gender_est=speaker["gender_est"],
                length_scale=round(length, 3), noise_w_scale=round(noise_w, 3), noise_scale=round(noise, 3),
                speech_s=round(speech_s, 3), tries=tries)


def make_clips(voices, speakers, limit):
    pool = {g: [s for s in speakers if s["gender_est"] == g] for g in ("male", "female")}
    if not pool["male"] or not pool["female"]:
        raise SystemExit("need male and female speakers in speakers.csv")
    board_unknown = {r["word"] for r in read_csv(C.RECORDINGS / "manifest.csv")} - set(C.CLASSES)
    if board_unknown & set(RHYME_WORDS):
        raise SystemExit(f"RHYME_WORDS overlap the board unknown takes: {sorted(board_unknown & set(RHYME_WORDS))}")
    done = {r["clip_path"] for r in read_csv(CLIPS_CSV)}
    extra_words = extra_unknown_words()
    print(f"_unknown_ words: {len(NEAR_MISS_WORDS)} near-miss + {len(extra_words)} from MSWC "
          f"(e.g. {', '.join(extra_words[:8])})")
    made, dropped, t0 = 0, 0, time.time()
    for label, n in CLIPS_PER_WORD.items():
        n = min(n, limit) if limit else n
        for index in range(n):
            word = unknown_word(index, extra_words) if label == C.UNKNOWN else label
            if clip_path(label, index, word).relative_to(C.SYNTHETIC).as_posix() in done:
                continue
            row = make_clip(voices, pool, label, index, extra_words)
            if row is None:
                dropped += 1
                continue
            append_csv(CLIPS_CSV, CLIP_FIELDS, row)
            made += 1
            if made % 500 == 0:
                print(f"  {made} clips ({time.time() - t0:.0f} s)")
        print(f"  {label:10s} done")
    return made, dropped, time.time() - t0


def summary():
    rows = read_csv(CLIPS_CSV)
    speech = np.array([float(r["speech_s"]) for r in rows])
    print(f"\nclips.csv: {len(rows)} clips, {len({r['speaker_key'] for r in rows})} speakers")
    print(f"gender: male {sum(r['gender_est'] == 'male' for r in rows)}, "
          f"female {sum(r['gender_est'] == 'female' for r in rows)}")
    print(f"speech length s: min {speech.min():.2f}  median {np.median(speech):.2f}  max {speech.max():.2f}")
    print(f"{'class':10s} {'clips':>6s} {'speakers':>8s} {'speech_med':>10s} {'remade':>6s}")
    for label in CLIPS_PER_WORD:
        r = [x for x in rows if x["label"] == label]
        if r:
            print(f"{label:10s} {len(r):6d} {len({x['speaker_key'] for x in r}):8d} "
                  f"{np.median([float(x['speech_s']) for x in r]):10.2f} {sum(x['tries'] != '1' for x in r):6d}")


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--limit", type=int, help="quick test: N speakers per voice and N clips per class")
    args = ap.parse_args()
    voices = {name: PiperVoice.load(VOICES_DIR / f"{name}.onnx") for name in VOICES}
    speakers = scan_speakers(voices, args.limit)
    made, dropped, seconds = make_clips(voices, speakers, args.limit)
    print(f"\nmade {made} clips, dropped {dropped} (still > {MAX_SPEECH_S} s after {MAX_TRIES} tries), "
          f"{seconds:.0f} s ({seconds / max(made, 1):.2f} s per clip)")
    summary()


if __name__ == "__main__":
    main()
