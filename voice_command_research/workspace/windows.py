"""
windows.py - turn one clip into one 1.025 s window the way the live stream sees a word.

    params = sample_params(rng, word_len_s)                 every random choice, stored in a table
    window = make_window(audio, start_s, end_s, background, params)

The word (speech_start_s..speech_end_s) plus MARGIN_S on each side is cut out with short fades,
put at params["offset_s"] (where the word starts in the window; may be negative or run past the end,
but at least MIN_INSIDE of the word stays inside), scaled so its peak is params["level_db"] dBFS
and laid on a background params["snr_db"] below the word. The original clip's own edges are not
kept, so every source (open data, Piper, board) ends up with the same kind of edges.

Listening / statistics test:
    C:/Users/ASUS/anaconda3/envs/torch_env/python.exe windows.py
"""
import wave

import numpy as np
import soundfile as sf
from scipy.signal import fftconvolve

from features import SR, WINDOW_SAMPLES

WINDOW_S = WINDOW_SAMPLES / SR             # 1.025 s = 101 stream frames
MARGIN_S = 0.10                            # audio kept before and after the spoken part
FADE_S = 0.02
MIN_INSIDE = 0.75                          # share of the word that must be inside the window
LEVEL_RANGE_DB = (-45.0, -15.0)            # word peak, dBFS after board gain (PROPOSAL: board S1 median -34.7)
QUIET_SNR_RANGE_DB = (25.0, 40.0)          # quiet room floor

# augmentation mix (PROPOSAL, from lessons-real-use.md: board SNR median 20.9 dB, min 12 dB; re-measure on S2)
NOISY_SHARE = 0.60                         # the rest only gets a quiet room floor
NOISY_SNR_DB = (12.0, 28.0)                # most noisy windows
NOISY_TAIL_SNR_DB = (5.0, 12.0)            # servo-like loud moments
NOISY_TAIL_SHARE = 0.15
NOISE_KIND_SHARES = {"room": 0.50, "event": 0.25, "babble": 0.25}
REVERB_SHARE = 0.25
ROOM_KINDS = ("isotropic", "pointsource_background", "pointsource_foreground")
QUIET_KINDS = ("isotropic", "pointsource_background")
BABBLE_TALKERS = (3, 5)
MIN_NOISE_DBFS = -60.0                     # a noise segment quieter than this is skipped (digital silence)


def read_audio(path):
    """16 kHz WAV -> float32 mono (channel 0 of multi-channel files)"""
    audio, sr = sf.read(str(path), dtype="float32", always_2d=True)
    if sr != SR:
        raise ValueError(f"{path}: {sr} Hz, need {SR}")
    return audio[:, 0]


def noise_kind_of(rng):
    """'quiet' or one of NOISE_KIND_SHARES, and the SNR for it"""
    if rng.random() >= NOISY_SHARE:
        return "quiet", float(rng.uniform(*QUIET_SNR_RANGE_DB))
    kind = rng.choice(list(NOISE_KIND_SHARES), p=list(NOISE_KIND_SHARES.values()))
    snr = rng.uniform(*(NOISY_TAIL_SNR_DB if rng.random() < NOISY_TAIL_SHARE else NOISY_SNR_DB))
    return str(kind), float(snr)


def noise_segment(audio, rng, tries=20):
    """random WINDOW_SAMPLES piece that is not digital silence (None if the file has none)"""
    if len(audio) < WINDOW_SAMPLES:
        return None
    for _ in range(tries):
        start = int(rng.integers(0, len(audio) - WINDOW_SAMPLES + 1))
        piece = audio[start:start + WINDOW_SAMPLES]
        if 10 * np.log10(np.mean(piece ** 2) + 1e-20) > MIN_NOISE_DBFS:
            return piece.copy()
    return None


def babble(clips, rng):
    """3-5 different _unknown_ clips (float32 arrays) at random places and levels -> one window"""
    out = np.zeros(WINDOW_SAMPLES, np.float32)
    for audio in clips:
        audio = audio / (np.sqrt(np.mean(audio ** 2)) + 1e-9) * 10 ** (rng.uniform(-6, 0) / 20)
        start = int(rng.integers(-len(audio) // 2, WINDOW_SAMPLES - len(audio) // 2))
        lo, hi = max(start, 0), min(start + len(audio), WINDOW_SAMPLES)
        out[lo:hi] += audio[lo - start:hi - start]
    return out


def reverb(audio, rir):
    """convolve with a room impulse response, aligned so the direct sound stays where it was"""
    rir = rir / (np.sqrt(np.sum(rir ** 2)) + 1e-9)
    peak = int(np.argmax(np.abs(rir)))
    wet = fftconvolve(audio, rir)[peak:peak + len(audio)]
    return wet.astype(np.float32)


def speech_bounds(audio, trim_db=35):
    """(start_s, end_s) of the 10 ms frames within trim_db of the loudest frame (rows without QC times)"""
    hop = SR // 100
    frames = audio[: len(audio) // hop * hop].reshape(-1, hop)
    db = 10 * np.log10((frames ** 2).mean(axis=1) + 1e-12)
    active = np.flatnonzero(db > db.max() - trim_db)
    return active[0] / 100, (active[-1] + 1) / 100


def sample_params(rng, word_len_s, snr_range_db=QUIET_SNR_RANGE_DB):
    """random placement, level and SNR for one window"""
    lowest = -(1 - MIN_INSIDE) * word_len_s
    highest = WINDOW_S - MIN_INSIDE * word_len_s
    return dict(offset_s=round(float(rng.uniform(lowest, highest)), 4),
                level_db=round(float(rng.uniform(*LEVEL_RANGE_DB)), 2),
                snr_db=round(float(rng.uniform(*snr_range_db)), 2))


def inside_share(offset_s, word_len_s):
    inside = min(offset_s + word_len_s, WINDOW_S) - max(offset_s, 0.0)
    return max(inside, 0.0) / word_len_s


def make_window(audio, start_s, end_s, background, params):
    """float32 window of WINDOW_SAMPLES; background is a float32 array of WINDOW_SAMPLES"""
    margin, fade = int(MARGIN_S * SR), int(FADE_S * SR)
    a, b = int(start_s * SR), int(end_s * SR)
    lo, hi = max(a - margin, 0), min(b + margin, len(audio))
    word = audio[lo:hi].copy()
    ramp = np.linspace(0.0, 1.0, fade, dtype=np.float32)
    word[:fade] *= ramp[: len(word[:fade])]
    word[-fade:] *= ramp[::-1][-len(word[-fade:]):]

    gain = 10 ** (params["level_db"] / 20) / (np.abs(audio[a:b]).max() + 1e-9)
    word *= gain
    speech_power = float(np.mean((audio[a:b] * gain) ** 2))
    noise_power = float(np.mean(background ** 2)) + 1e-12
    window = background * np.sqrt(speech_power / noise_power / 10 ** (params["snr_db"] / 10))

    first = int(round(params["offset_s"] * SR)) - (a - lo)       # where word[0] lands in the window
    src_lo, dst_lo = max(-first, 0), max(first, 0)
    n = min(len(word) - src_lo, WINDOW_SAMPLES - dst_lo)
    if n > 0:
        window[dst_lo:dst_lo + n] += word[src_lo:src_lo + n]
    return window.astype(np.float32)


def _test():
    """listening sample + placement statistics; writes into the session scratchpad"""
    import sys
    import zlib
    from pathlib import Path

    import pandas as pd
    sys.path.insert(0, str(Path(__file__).resolve().parent.parent))
    import config as C

    out = Path(r"C:\Users\ASUS\AppData\Local\Temp\claude\D--Microcon69-ProjectRoboticArm-ProjectSpace"
               r"\ea8570a2-aa63-45f9-9723-ef06b46131a4\scratchpad\windows_test")
    out.mkdir(exist_ok=True)
    table = pd.read_csv(C.BUILT / "clips.csv", low_memory=False)
    train = table[table["keep"] & (table["set"] == "train") & table["speech_start_s"].notna()]

    # placement statistics over 20,000 sampled params
    rng = np.random.default_rng(0)
    lengths = (train["speech_end_s"] - train["speech_start_s"]).sample(20000, random_state=0).to_numpy()
    shares = np.array([inside_share(sample_params(rng, L)["offset_s"], L) for L in lengths])
    print(f"word length s: median {np.median(lengths):.2f}  p95 {np.percentile(lengths, 95):.2f}")
    print(f"windows with the whole word inside {np.mean(shares >= 0.999):.3f} | partly cut "
          f"{np.mean(shares < 0.999):.3f} | min inside share {shares.min():.3f}")

    # listening: 4 sources x 3 words, background = Speech Commands pink noise (placeholder until step 2)
    noise_file = C.DA_DATA / "raw" / "open" / "speech_commands_v0.02" / "_background_noise_" / "pink_noise.wav"
    with wave.open(str(noise_file)) as w:
        pink = np.frombuffer(w.readframes(w.getnframes()), "<i2").astype(np.float32) / 32768
    rows, parts = [], []
    picks = [("open_speech_commands", "stop"), ("open_mswc", "move"), ("open_mswc_joined", "blue box"),
             ("piper", "undo")]
    for source, word in picks:
        for r in train[(train["source"] == source) & (train["label"] == word)].sample(3, random_state=1).itertuples():
            rng = np.random.default_rng(zlib.crc32(r.audio_path.encode()))
            audio = read_audio(r.audio_path)
            params = sample_params(rng, r.speech_end_s - r.speech_start_s)
            start = int(rng.integers(0, len(pink) - WINDOW_SAMPLES))
            window = make_window(audio, r.speech_start_s, r.speech_end_s, pink[start:start + WINDOW_SAMPLES].copy(),
                                 params)
            parts += [window, np.zeros(int(0.6 * SR), np.float32)]
            rows.append(dict(source=source, word=word, word_len_s=round(r.speech_end_s - r.speech_start_s, 2),
                             **params, inside=round(inside_share(params["offset_s"],
                                                                 r.speech_end_s - r.speech_start_s), 2),
                             peak_dbfs=round(20 * np.log10(np.abs(window).max()), 1)))
    with wave.open(str(out / "ALL_windows.wav"), "wb") as w:
        w.setnchannels(1)
        w.setsampwidth(2)
        w.setframerate(SR)
        w.writeframes((np.clip(np.concatenate(parts), -1, 1) * 32767).astype("<i2").tobytes())
    print(pd.DataFrame(rows).to_string(index=False))
    print(f"-> {out / 'ALL_windows.wav'}")


if __name__ == "__main__":
    _test()
