r"""
recorder.py - Session R: record board takes through the streaming firmware (MicrophoneTest02)

The STM32 streams 646-byte packets every 10 ms (same parser as stream_test.py):
    "AU", uint16 seq, uint16 count = 160, then 160 x (ch0, ch1) int16   (ch0 = left, ch1 = right)
The audio is saved EXACTLY as the board sees it (>>8 + DC block on chip). No gain, no filters here;
the +15 dB board gain (KWS_GAIN_DB) is applied later, in eval_board.py.

Lives in voice_command_research\dataset\ and imports ..\config.py for paths and the vocabulary.
Run with torch_env, from voice_command_research\dataset\:
    C:/Users/ASUS/anaconda3/envs/torch_env/python.exe recorder.py summary --session S1
Needs: numpy, pyserial.  Optional: sounddevice (playback).

Commands
    python recorder.py check   --port COM3                       5 s link + level check
    python recorder.py words   --port COM3 --session S1           prompted word takes (resumable)
    python recorder.py noise   --port COM3 --session S1 --label room  --seconds 60
    python recorder.py noise   --port COM3 --session S1 --label servo --seconds 60
    python recorder.py summary --session S1 [--listen 3]          counts, SNR, play random clips

words: every prompt shows the word, counts down, records 2.0 s of stereo, finds the word
(loudest region, centred 1.0 s window) and shows levels.  Keys:
    Enter = keep   r = redo   p = play 1 s clip   P = play whole 2 s   s = skip   q = quit
Quitting is safe: every kept take is written at once; running the same command again resumes.

Saved per kept take, under dataset\recordings\
    raw\<speaker>\<session>\<word>\<word>_NN_raw.wav     2 s stereo int16 (both mics)
    clips\<speaker>\<session>\<word>\<word>_NN.wav       1 s mono int16, chosen channel
    manifest.csv                                         one row (manifest_open.csv columns
                                                         + snr_db, raw_path, split, ...)
Noise recordings: noise\<label>\<speaker>_<session>_<label>.wav, cut into 1 s _silence_ clips.
Objects are folders like blue_box, but "blue box" in the CSV. _unknown_ prompts go to the
_unknown_ folder with the spoken word in the file name; noise clips go to _silence_.

S1 is the TEST set: its rows get split = "test", so any training code must drop
split == "test" rows.
"""
import argparse
import csv
import json
import os
import random
import struct
import sys
import threading
import time
import wave
from collections import Counter, defaultdict
from datetime import datetime
from pathlib import Path

import numpy as np

try:
    import serial
except ImportError:                       # message later, so 'summary' still works without it
    serial = None

sys.path.insert(0, str(Path(__file__).resolve().parent.parent))
import config as C

# ---------------------------------------------------------------- stream format (stream_test.py)
FS = 16000
SYNC = b"AU"
FRAMES = 160                                  # stereo frames per packet = 10 ms
PKT_SIZE = 6 + FRAMES * 2 * 2                 # 646 bytes
HEADER = struct.Struct("<2sHH")               # sync, seq, count
MAX_GAP = 500                                 # bigger seq jump = board restarted, not packet loss

# ---------------------------------------------------------------- take settings
TAKE_S = 2.0
CLIP_S = 1.0
PRE_ROLL_S = 0.15                             # part of the 2 s take that comes from before "GO"
TAKE_PKTS = int(TAKE_S * 100)
CLIP_N = int(CLIP_S * FS)
WIN_FR = int(CLIP_S * 100)                    # 1 s window in 10 ms frames
PRE_PKTS = int(PRE_ROLL_S * 100)
BOARD_GAIN_DB = 15.0                          # only for the "what the model sees" display
SNR_MIN_DB = 10.0
BREAK_EVERY = 40

UNKNOWN_WORDS = ["go", "now", "nose", "more", "moon", "mood", "groove", "grass", "grape",
                 "bright", "ride", "hope",              # "write" removed 21 Oct: sounds like "right" "phone", "foam", "hold", "bag", "back",
                 "black", "blue", "box", "books", "pink", "sick", "hello", "robot", "arm",
                 "open", "close", "up", "down", "wait"]
CONDITIONS = ["quiet", "aircon", "servo"]

FIELDS = ["clip_path", "word", "role", "source", "speaker", "gender", "session", "mic",
          "position", "distance_cm", "condition", "valid", "orig_path", "duration_s",
          "rms_dbfs", "peak", "clip_frac", "error", "quality_ok",
          # board-only columns
          "snr_db", "raw_path", "cut_start_s", "lost_pkts", "flags", "split", "recorded_at"]


# ================================================================ serial stream
class PacketParser:
    """stream_test.py packet logic: sync search, count check, sequence gaps"""

    def __init__(self):
        self.buf = bytearray()
        self.locked = False
        self.last_seq = None
        self.n_pkts = self.n_lost = self.n_resync = self.n_restart = 0

    def feed(self, data):
        """add bytes; return [(gap, samples(160, 2) int16)], gap = packets missing before
        this one, -1 = the board restarted (sequence jumped)"""
        buf = self.buf
        buf += data
        out = []
        while len(buf) >= PKT_SIZE:
            if buf[0:2] != SYNC:
                # not at a packet boundary: search for the next "AU"
                idx = buf.find(SYNC, 1)
                drop = idx if idx > 0 else len(buf) - 1
                if self.locked:
                    self.n_resync += 1
                del buf[:drop]
                self.locked = False
                continue
            _, seq, count = HEADER.unpack_from(buf, 0)
            if count != FRAMES:
                # "AU" appeared inside audio data by chance: not a real header
                del buf[:1]
                continue
            samples = np.frombuffer(bytes(buf[6:PKT_SIZE]), dtype="<i2").reshape(FRAMES, 2).copy()
            del buf[:PKT_SIZE]
            self.locked = True
            gap = 0
            if self.last_seq is not None:
                gap = (seq - self.last_seq - 1) & 0xFFFF
                if gap > MAX_GAP:
                    self.n_restart += 1
                    gap = -1
                else:
                    self.n_lost += gap
            self.last_seq = seq
            self.n_pkts += 1
            out.append((gap, samples))
        return out


class BoardStream:
    """reads the port in a background thread into a ring buffer, so nothing is lost
    (and nothing goes stale) while you look at a take"""

    def __init__(self, port, baud, ring_s=30.0):
        if serial is None:
            raise SystemExit("pyserial is missing:  python -m pip install pyserial")
        self.ring_n = int(ring_s * 100)
        self.ring = np.zeros((self.ring_n, FRAMES, 2), np.int16)
        self.bad = np.zeros(self.ring_n, bool)        # missing packet (zeros) or restart
        self.total = 0                                # packets written so far (incl. gaps)
        self.lock = threading.Lock()
        self.parser = PacketParser()
        self.err = None
        self.last_rx = time.time()
        self._stop = threading.Event()
        try:
            self.ser = serial.Serial(port, baud, timeout=0.05)
        except Exception as e:
            raise SystemExit(f"Could not open {port}: {e}\n"
                             "Close stream_test.py / the serial monitor / STM32CubeIDE console, "
                             "check the COM number.")
        try:
            self.ser.set_buffer_size(rx_size=1 << 20)  # Windows only; harmless elsewhere
        except Exception:
            pass
        self.ser.reset_input_buffer()
        self.thread = threading.Thread(target=self._run, daemon=True)
        self.thread.start()

    def _put(self, samples, bad):
        i = self.total % self.ring_n
        self.ring[i] = samples
        self.bad[i] = bad
        self.total += 1

    def _run(self):
        zero = np.zeros((FRAMES, 2), np.int16)
        try:
            while not self._stop.is_set():
                data = self.ser.read(4096)
                if not data:
                    continue
                pkts = self.parser.feed(data)
                if not pkts:
                    continue
                with self.lock:
                    for gap, s in pkts:
                        for _ in range(max(gap, 0)):   # keep timing: silence for lost audio
                            self._put(zero, True)
                        self._put(s, gap < 0)
                self.last_rx = time.time()
        except Exception as e:
            self.err = e

    def now(self):
        with self.lock:
            return self.total

    def wait_until(self, n, progress=None, no_data_s=2.0):
        while True:
            with self.lock:
                t = self.total
            if t >= n:
                return
            if self.err is not None:
                raise RuntimeError(f"serial error: {self.err}")
            if time.time() - self.last_rx > no_data_s:
                raise RuntimeError("no packets from the board - is MicrophoneTest02 running "
                                   "(Run / Resume, not paused in the debugger)?")
            if progress:
                progress(t)
            time.sleep(0.01)

    def get(self, start, n):
        """stereo int16 (n*160, 2) and a per-packet 'bad' mask"""
        with self.lock:
            if self.total - start > self.ring_n:
                raise RuntimeError("ring buffer overwritten (recording longer than the ring)")
            idx = np.arange(start, start + n) % self.ring_n
            return self.ring[idx].reshape(-1, 2).copy(), self.bad[idx].copy()

    def capture(self, n_pkts, pre_pkts=0, progress=None):
        start = max(self.now() - pre_pkts, 0)
        self.wait_until(start + n_pkts, progress=progress)
        return self.get(start, n_pkts)

    def close(self):
        self._stop.set()
        self.thread.join(timeout=1.0)
        self.ser.close()


# ================================================================ analysis
def dbfs(x_int16):
    x = x_int16.astype(np.float64) / 32768.0
    return 20 * np.log10(np.sqrt(np.mean(x * x)) + 1e-9)


def frame_db(x_int16):
    """10 ms frame energy in dBFS"""
    n = len(x_int16) // FRAMES * FRAMES
    f = x_int16[:n].astype(np.float64).reshape(-1, FRAMES) / 32768.0
    return 10 * np.log10((f * f).mean(axis=1) + 1e-12)


def clip_stats(x_int16):
    """same numbers as ingest_open.stats(), on the int16 clip"""
    x = x_int16.astype(np.float64) / 32768.0
    n = len(x)
    rms = float(np.sqrt(np.mean(x ** 2))) if n else 0.0
    return {"duration_s": round(n / FS, 4),
            "rms_dbfs": round(20 * np.log10(rms + 1e-9), 2),
            "peak": round(float(np.abs(x).max()) if n else 0.0, 4),
            "clip_frac": round(float(np.mean(np.abs(x) >= 0.999)) if n else 0.0, 5)}


def find_word(db, gap_fr=15):
    """word region = frames above threshold around the loudest frame (pauses up to 150 ms
    bridged, e.g. the stop in 'pick'); the 1 s window is centred on that region"""
    nf = len(db)
    noise = float(np.percentile(db, 10))
    top = float(db.max())
    thr = noise + max(6.0, 0.2 * (top - noise))
    act = db > thr
    k = int(np.argmax(db))
    s = e = k
    miss = 0
    for i in range(k - 1, -1, -1):
        if act[i]:
            s, miss = i, 0
        else:
            miss += 1
            if miss > gap_fr:
                break
    miss = 0
    for i in range(k + 1, nf):
        if act[i]:
            e, miss = i, 0
        else:
            miss += 1
            if miss > gap_fr:
                break
    centre = (s + e + 1) / 2
    start = int(round(centre - WIN_FR / 2))
    start = min(max(start, 0), nf - WIN_FR)
    seg = 10 ** (db[s:e + 1] / 10)
    snr = 10 * np.log10(seg.mean()) - noise
    # other loud frames outside the word region and outside the clip
    outside = np.ones(nf, bool)
    outside[s:e + 1] = False
    outside[start:start + WIN_FR] = False
    extra = int(np.sum(outside & (db > thr + 3)))
    return {"noise_db": noise, "thr_db": thr, "s": s, "e": e, "start": start,
            "snr_db": float(snr), "extra_fr": extra}


def analyse(take, bad, ch):
    """everything shown and saved for one 2 s take on channel ch"""
    x = take[:, ch]
    r = {"ch": ch, "dead": not np.any(x)}
    if r["dead"]:
        r["flags"] = ["channel is all zeros"]
        return r
    db = frame_db(x)
    r.update(find_word(db))
    r["db"] = db
    r["lost"] = int(bad.sum())
    r["clipped"] = int(np.sum(np.abs(x.astype(np.int32)) >= 32767))
    clip = x[r["start"] * FRAMES: r["start"] * FRAMES + CLIP_N]
    r["clip"] = clip
    r["peak_dbfs"] = 20 * np.log10(np.abs(clip.astype(np.int32)).max() / 32768.0 + 1e-9)
    r["speech_db"] = r["noise_db"] + r["snr_db"]
    nf = len(db)
    flags = []
    if r["lost"]:
        flags.append(f"lost {r['lost']} packets")
    if r["snr_db"] < SNR_MIN_DB:
        flags.append(f"low SNR {r['snr_db']:.1f} dB")
    if r["clipped"]:
        flags.append(f"clipping ({r['clipped']} samples)")
    if r["s"] <= 1 or r["e"] >= nf - 2:
        flags.append("word touches the edge of the 2 s take")
    if r["e"] - r["s"] + 1 > WIN_FR:
        flags.append("sound longer than 1 s - the clip cuts it")
    if r["extra_fr"] >= 5:
        flags.append("other sound outside the clip (2 words? click?)")
    r["flags"] = flags
    return r


def pick_channel(take, bad, mode):
    if mode in ("0", "1"):
        return analyse(take, bad, int(mode))
    cands = [analyse(take, bad, ch) for ch in (0, 1)]
    live = [c for c in cands if not c["dead"]]
    return max(live, key=lambda c: c["snr_db"]) if live else cands[1]


# ================================================================ display + keys
LEVELS = " .:-=+*#%@"


def meter(db, start, lo=-80.0, hi=-30.0, cols=50):
    per = len(db) // cols
    m = db[:per * cols].reshape(cols, per).max(axis=1)
    idx = np.clip(np.round((m - lo) / (hi - lo) * (len(LEVELS) - 1)), 0, len(LEVELS) - 1)
    bar = "".join(LEVELS[int(i)] for i in idx)
    a, b = start // per, (start + WIN_FR) // per
    marks = " " * a + "[" + "-" * max(b - a - 2, 0) + "]"
    return f"  |{bar}|  2 s, {lo:.0f}..{hi:.0f} dBFS\n   {marks}  <- saved 1 s"


def big(word):
    try:
        import pyfiglet
        return pyfiglet.figlet_format(word.upper(), font="big")
    except Exception:
        t = "   ".join(" ".join(w.upper()) for w in word.split())
        w = len(t) + 10
        return ("\n  +" + "-" * w + "+\n  |" + " " * w + "|\n  |     " + t + "     |\n  |"
                + " " * w + "|\n  +" + "-" * w + "+\n")


def clear():
    os.system("cls" if os.name == "nt" else "clear")


if os.name == "nt":
    import msvcrt

    def flush_keys():
        while msvcrt.kbhit():
            msvcrt.getwch()

    def getkey():
        ch = msvcrt.getwch()
        if ch in ("\x00", "\xe0"):          # arrow / function key: ignore
            msvcrt.getwch()
            return ""
        return ch
else:
    import termios
    import tty

    def flush_keys():
        try:
            termios.tcflush(sys.stdin, termios.TCIFLUSH)
        except Exception:
            pass

    def getkey():
        fd = sys.stdin.fileno()
        old = termios.tcgetattr(fd)
        try:
            tty.setraw(fd)
            return sys.stdin.read(1)
        finally:
            termios.tcsetattr(fd, termios.TCSADRAIN, old)


def ask_key(allowed):
    """returns 'enter' or one of the allowed characters ('q' also for Ctrl+C)"""
    flush_keys()
    while True:
        k = getkey()
        if k in ("\r", "\n"):
            return "enter"
        if k == "\x03":
            return "q"
        if k in allowed:
            return k


def play(x_int16, label=""):
    try:
        import sounddevice as sd
    except ImportError:
        print("  (playback needs:  python -m pip install sounddevice)")
        return
    y = x_int16.astype(np.float32) / 32768.0
    pk = float(np.abs(y).max())
    if pk > 0:
        y = y * (0.7 / pk)                   # louder for listening only; files are not changed
    if label:
        print(f"  playing {label}")
    sd.play(y, FS)
    sd.wait()


# ================================================================ files
def write_wav(path, x_int16):
    path.parent.mkdir(parents=True, exist_ok=True)
    x = np.ascontiguousarray(x_int16, dtype="<i2")
    with wave.open(str(path), "wb") as w:
        w.setnchannels(1 if x.ndim == 1 else x.shape[1])
        w.setsampwidth(2)
        w.setframerate(FS)
        w.writeframes(x.tobytes())


def rel(p):
    return Path(p).relative_to(C.RECORDINGS).as_posix()


def manifest_path():
    return C.RECORDINGS / "manifest.csv"


def read_manifest():
    p = manifest_path()
    if not p.exists():
        return []
    with open(p, newline="", encoding="utf-8") as f:
        return list(csv.DictReader(f))


def append_row(row):
    p = manifest_path()
    while True:
        try:
            new = not p.exists()
            if not new:
                with open(p, newline="", encoding="utf-8") as f:
                    header = next(csv.reader(f), [])
                if header != FIELDS:
                    raise SystemExit(f"{p} has different columns than this recorder writes:\n"
                                     f"  {header}\nRename it and run again.")
            with open(p, "a", newline="", encoding="utf-8") as f:
                w = csv.DictWriter(f, fieldnames=FIELDS)
                if new:
                    w.writeheader()
                w.writerow({k: row.get(k, "") for k in FIELDS})
            return
        except PermissionError:
            input(f"  Cannot write {p.name} (open in Excel?). Close it, then press Enter ... ")


def next_index(folder, stem):
    """first free NN for <stem>_NN.wav in folder"""
    used = {int(f.stem.rsplit("_", 1)[1]) for f in folder.glob(f"{stem}_*.wav")
            if f.stem.rsplit("_", 1)[1].isdigit()} if folder.exists() else set()
    i = 1
    while i in used:
        i += 1
    return i


def clean_id(s):
    s = "".join(c if c.isalnum() or c in "-_" else "_" for c in s.strip().lower())
    return s.strip("_")


def ask(prompt, default="", choices=None):
    while True:
        d = f" [{default}]" if default != "" else ""
        v = input(f"  {prompt}{d}: ").strip() or str(default)
        if choices and v not in choices:
            print(f"    choose one of: {', '.join(choices)}")
            continue
        if v:
            return v


# ================================================================ session metadata
def session_meta(args):
    speaker = clean_id(args.speaker or ask("speaker id (e.g. supanut)"))
    session = clean_id(args.session).upper()
    sdir = C.RECORDINGS / "raw" / speaker / session
    mpath = sdir / "session.json"
    if mpath.exists():
        meta = json.loads(mpath.read_text(encoding="utf-8"))
        print(f"\n  Session {session} of {speaker} already exists:")
        for k in ("position", "distance_cm", "condition", "channel", "split"):
            print(f"    {k:12s} {meta.get(k)}")
        if input("  Enter = continue with these, e = edit: ").strip().lower() != "e":
            return meta
    else:
        meta = {}
    print(f"\n  Session {session} / speaker {speaker}")
    meta.update({
        "speaker": speaker, "session": session,
        "position": ask("position (left / centre / right of the mic)",
                        meta.get("position", "centre"), ["left", "centre", "right"]),
        "distance_cm": int(ask("distance to the mic in cm", meta.get("distance_cm", 40))),
        "condition": ask("condition", meta.get("condition", "quiet"), CONDITIONS),
        "channel": args.channel,
        "split": args.split or meta.get("split") or ("test" if session == "S1" else "train"),
        "firmware": "MicrophoneTest02 (>>8, DC block, no gain)",
        "port": args.port, "baud": args.baud,
        "created": meta.get("created", datetime.now().isoformat(timespec="seconds")),
    })
    sdir.mkdir(parents=True, exist_ok=True)
    mpath.write_text(json.dumps(meta, indent=2), encoding="utf-8")
    return meta


def base_row(meta, ch):
    return {"source": "board", "speaker": "board_" + meta["speaker"], "gender": "",
            "session": meta["session"], "mic": f"inmp441_ch{ch}",
            "position": meta["position"], "distance_cm": meta["distance_cm"],
            "condition": meta["condition"], "valid": True, "error": "",
            "split": meta["split"], "recorded_at": datetime.now().isoformat(timespec="seconds")}


# ================================================================ link check
def precheck(stream, seconds, channel):
    t0 = time.time()
    s0 = stream.now()
    n = int(seconds * 100)
    stream.wait_until(s0 + n)
    dt = time.time() - t0
    x, bad = stream.get(s0, n)
    p = stream.parser
    print(f"\n  Link: {n / dt:5.1f} packets/s (expect 100)   lost {int(bad.sum())}   "
          f"resyncs {p.n_resync}   restarts {p.n_restart}")
    dead = []
    for ch, name in ((0, "left "), (1, "right")):
        if not np.any(x[:, ch]):
            dead.append(ch)
            print(f"  ch{ch} ({name}): ALL ZEROS")
        else:
            print(f"  ch{ch} ({name}): noise {dbfs(x[:, ch]):6.1f} dBFS   "
                  f"peak {20 * np.log10(np.abs(x[:, ch].astype(np.int32)).max() / 32768):6.1f} dBFS")
    if channel in ("0", "1") and int(channel) in dead:
        raise SystemExit(f"\n  The chosen channel ch{channel} is silent. Use --channel "
                         f"{1 - int(channel)} or fix the wiring.")
    if len(dead) == 2:
        raise SystemExit("\n  Both channels are silent - check the SAI clock / mics.")
    return x


# ================================================================ commands
def cmd_check(args):
    stream = BoardStream(args.port, args.baud)
    try:
        print(f"Listening on {args.port} for {args.seconds:.0f} s - stay quiet for a moment, "
              "then say a word ...")
        precheck(stream, args.seconds, args.channel)
    finally:
        stream.close()


def build_prompts(meta, done, words, takes, n_unknown):
    prompts = []
    for tok in words:
        prompts += [("vocab", tok)] * max(takes - done[("vocab", tok)], 0)
    # same unknown list on every resume of this session
    rng = random.Random(f"{meta['speaker']}/{meta['session']}")
    unk = []
    while len(unk) < n_unknown:
        unk += rng.sample(UNKNOWN_WORDS, min(len(UNKNOWN_WORDS), n_unknown - len(unk)))
    for w, c in Counter(unk).items():
        prompts += [("unknown", w)] * max(c - done[("unknown", w)], 0)
    random.shuffle(prompts)
    for i in range(1, len(prompts)):             # no word twice in a row
        if prompts[i] == prompts[i - 1]:
            for j in range(i + 1, len(prompts)):
                if prompts[j] != prompts[i - 1]:
                    prompts[i], prompts[j] = prompts[j], prompts[i]
                    break
    return prompts


def parse_words(s):
    if not s:
        return list(C.TOKENS)
    out = []
    for w in s.split(","):
        w = w.strip().lower().replace("_", " ")
        if w not in C.TOKENS:
            raise SystemExit(f"'{w}' is not a vocabulary word. Words: {', '.join(C.TOKENS)}")
        out.append(w)
    return out


def show_result(r, both):
    print()
    for c in both:
        if c["ch"] != r["ch"]:
            print(f"  ch{c['ch']}: " + ("all zeros" if c["dead"]
                                       else f"SNR {c['snr_db']:5.1f} dB (not used)"))
    if r["dead"]:
        return
    print(f"  ch{r['ch']} (saved)  peak {r['peak_dbfs']:6.1f} dBFS   speech {r['speech_db']:6.1f}   "
          f"noise {r['noise_db']:6.1f}   SNR {r['snr_db']:5.1f} dB")
    print(f"  with the +{BOARD_GAIN_DB:.0f} dB board gain the model sees peak "
          f"{r['peak_dbfs'] + BOARD_GAIN_DB:6.1f} dBFS")
    print(meter(r["db"], r["start"]))
    for f in r["flags"]:
        print(f"  !! {f}")


def cmd_words(args):
    meta = session_meta(args)
    words = parse_words(args.words)
    spk, sess = meta["speaker"], meta["session"]
    done = Counter()
    for row in read_manifest():
        if row["speaker"] == "board_" + spk and row["session"] == sess:
            done[(row["role"], row["word"])] += 1
    prompts = build_prompts(meta, done, words, args.takes, args.unknown)
    if not prompts:
        print(f"\n  Nothing left to record for {sess} (all takes kept). "
              "Use 'summary' to check, or raise --takes.")
        return

    stream = BoardStream(args.port, args.baud)
    try:
        print("\n  Checking the link (1 s, stay quiet) ...")
        precheck(stream, 1.0, args.channel)
        print(f"\n  {len(prompts)} takes to record (split = {meta['split']}). "
              "Speak naturally, like you talk to the robot.")
        print("  Keys after each take: Enter keep | r redo | p play | P play 2 s | s skip | q quit")
        input("  Press Enter to start ... ")

        kept = skipped = 0
        for n, (role, word) in enumerate(prompts, 1):
            while True:                                          # redo loop
                clear()
                tag = "  (NOT a command - just say it)" if role == "unknown" else ""
                print(f"  {sess} / {spk}    take {n}/{len(prompts)}    kept {kept}")
                print(big(word) + tag)
                time.sleep(args.read_s)
                for c in "321":
                    print(f"     {c} ...", end="\r", flush=True)
                    time.sleep(0.35)
                print("     >>> SPEAK <<<   ")
                take, bad = stream.capture(TAKE_PKTS, PRE_PKTS)
                r = pick_channel(take, bad, args.channel)
                both = [analyse(take, bad, 0), analyse(take, bad, 1)]
                show_result(r, both)
                if r["dead"]:
                    print("  !! the chosen channel is all zeros - nothing to save")
                print("\n  Enter keep | r redo | p play | P play 2 s | s skip | q quit")
                while True:
                    k = ask_key("rpPsq")
                    if k == "p" and not r["dead"]:
                        play(r["clip"], "1 s clip")
                    elif k == "P":
                        play(take[:, r["ch"]], "whole 2 s take")
                    elif k in ("p",):
                        pass
                    else:
                        break
                if k == "r" or (k == "enter" and r["dead"]):
                    continue
                break
            if k == "q":
                print(f"\n  Stopped. Kept {kept} this run; run the same command again to resume.")
                return
            if k == "s":
                skipped += 1
                continue

            # ---- keep: save raw take, clip, manifest row
            folder = C.token_dir(word) if role == "vocab" else C.UNKNOWN
            stem = C.token_dir(word)
            cdir = C.RECORDINGS / "clips" / spk / sess / folder
            rdir = C.RECORDINGS / "raw" / spk / sess / folder
            name = f"{stem}_{next_index(cdir, stem):02d}"
            raw_p = rdir / f"{name}_raw.wav"
            clip_p = cdir / f"{name}.wav"
            write_wav(raw_p, take)
            write_wav(clip_p, r["clip"])
            cut = r["start"] * FRAMES / FS
            row = base_row(meta, r["ch"])
            row.update(clip_stats(r["clip"]))
            row.update({
                "clip_path": rel(clip_p), "word": word, "role": role,
                "orig_path": f"{rel(raw_p)}@{cut:.2f}s", "raw_path": rel(raw_p),
                "cut_start_s": round(cut, 2), "snr_db": round(r["snr_db"], 2),
                "lost_pkts": r["lost"], "flags": "; ".join(r["flags"]),
            })
            row["quality_ok"] = (r["lost"] == 0 and r["clipped"] == 0
                                 and r["snr_db"] >= SNR_MIN_DB and not r["flags"])
            append_row(row)
            kept += 1
            if kept % BREAK_EVERY == 0 and n < len(prompts):
                input(f"\n  {kept} kept - short break (water, relax your voice). Enter to go on ... ")

        print(f"\n  Done: kept {kept}, skipped {skipped}. "
              f"Next: python recorder.py summary --session {sess}")
    finally:
        stream.close()


def cmd_noise(args):
    meta = session_meta(args)
    spk, sess, label = meta["speaker"], meta["session"], args.label
    cond = "servo" if label == "servo" else meta["condition"]
    cdir = C.RECORDINGS / "clips" / spk / sess / C.SILENCE
    have = len(list(cdir.glob(f"{label}_*.wav"))) if cdir.exists() else 0
    if have and input(f"  {have} '{label}' clips already in {sess}. Record more? [y/N]: "
                      ).strip().lower() != "y":
        return

    stream = BoardStream(args.port, args.baud, ring_s=args.seconds + 15)
    try:
        print("\n  Checking the link (1 s) ...")
        precheck(stream, 1.0, args.channel)
        if label == "servo":
            input("\n  Start the servos moving and keep them moving the whole time.\n"
                  "  Nobody talks. Press Enter when they are running ... ")
        else:
            input("\n  Leave the room as it is (aircon as in this session), nobody talks.\n"
                  "  Press Enter, then stay silent ... ")
        for c in "321":
            print(f"     {c} ...", end="\r", flush=True)
            time.sleep(0.6)
        n = int(args.seconds * 100)
        start = stream.now()
        last = [-1]

        def progress(t):
            sec = (t - start) // 100
            if sec > last[0] and t - start >= 100:
                last[0] = sec
                x, _ = stream.get(t - 100, 100)
                print(f"     {sec:3d}/{args.seconds:.0f} s   "
                      f"ch1 {dbfs(x[:, 1]):6.1f} dBFS   ch0 {dbfs(x[:, 0]):6.1f}", end="\r", flush=True)

        stream.wait_until(start + n, progress=progress)
        take, bad = stream.get(start, n)
    finally:
        stream.close()
    print()

    ch = 1 if args.channel == "auto" else int(args.channel)
    if not np.any(take[:, ch]):
        raise SystemExit(f"  ch{ch} is all zeros - nothing saved.")
    ndir = C.RECORDINGS / "noise" / label
    raw_p = ndir / f"{spk}_{sess}_{label}.wav"
    k = 2
    while raw_p.exists():
        raw_p = ndir / f"{spk}_{sess}_{label}_{k}.wav"
        k += 1
    write_wav(raw_p, take)

    x = take[:, ch]
    first = next_index(cdir, label)
    levels = []
    for i in range(int(args.seconds)):
        seg = x[i * FS:(i + 1) * FS]
        lost = int(bad[i * 100:(i + 1) * 100].sum())
        clip_p = cdir / f"{label}_{first + i:03d}.wav"
        write_wav(clip_p, seg)
        row = base_row(meta, ch)
        row["condition"] = cond
        row.update(clip_stats(seg))
        clipped = int(np.sum(np.abs(seg.astype(np.int32)) >= 32767))
        flags = ([f"lost {lost} packets"] if lost else []) + (["clipping"] if clipped else [])
        row.update({"clip_path": rel(clip_p), "word": C.SILENCE, "role": "silence",
                    "orig_path": f"{rel(raw_p)}@{float(i):.1f}s", "raw_path": rel(raw_p),
                    "cut_start_s": float(i), "snr_db": "", "lost_pkts": lost,
                    "flags": "; ".join(flags), "quality_ok": not flags})
        append_row(row)
        levels.append((dbfs(seg), clip_p.name))
    lv = sorted(levels)
    print(f"  Saved {raw_p.name} + {len(levels)} x 1 s _silence_ clips ({label}, ch{ch}).")
    print(f"  Level per second: {lv[0][0]:.1f} .. {lv[-1][0]:.1f} dBFS, lost packets {int(bad.sum())}")
    print(f"  Loudest second: {lv[-1][1]} - listen to it; if someone spoke there, delete that "
          "clip and its manifest row.")


def cmd_summary(args):
    rows = [r for r in read_manifest()
            if r["session"] == args.session.upper()
            and (not args.speaker or r["speaker"] == "board_" + clean_id(args.speaker))]
    if not rows:
        raise SystemExit(f"No rows for session {args.session} in {manifest_path()}")
    groups = defaultdict(list)
    for r in rows:
        key = r["word"] if r["role"] == "vocab" else (
            C.UNKNOWN if r["role"] == "unknown" else
            f"{C.SILENCE} ({Path(r['clip_path']).stem.rsplit('_', 1)[0]})")
        groups[key].append(r)
    order = [w for w in C.TOKENS if w in groups] + sorted(k for k in groups if k not in C.TOKENS)
    print(f"\n  Session {args.session.upper()}: {len(rows)} clips, "
          f"speakers {sorted({r['speaker'] for r in rows})}, split {sorted({r['split'] for r in rows})}")
    print(f"\n  {'class':22s} {'clips':>5s} {'SNR min':>8s} {'SNR med':>8s} {'flagged':>8s}")
    for k in order:
        g = groups[k]
        snr = [float(r["snr_db"]) for r in g if r["snr_db"] not in ("", None)]
        bad = sum(r["quality_ok"] != "True" for r in g)
        mn = f"{min(snr):8.1f}" if snr else f"{'-':>8s}"
        md = f"{float(np.median(snr)):8.1f}" if snr else f"{'-':>8s}"
        print(f"  {k:22s} {len(g):5d} {mn} {md} {bad:8d}")
    missing = [w for w in C.TOKENS if len(groups.get(w, [])) < args.takes]
    if missing:
        print(f"\n  Fewer than {args.takes} takes: {', '.join(missing)}")
    flagged = [r for r in rows if r["flags"]]
    if flagged:
        print(f"\n  Flagged clips ({len(flagged)}):")
        for r in flagged[:30]:
            print(f"    {r['clip_path']}: {r['flags']}")

    if args.listen:
        print(f"\n  Listening: {args.listen} random clips per class (Ctrl+C stops)")
        rng = random.Random()
        try:
            for k in order:
                g = groups[k]
                for r in rng.sample(g, min(args.listen, len(g))):
                    with wave.open(str(C.RECORDINGS / r["clip_path"])) as w:
                        x = np.frombuffer(w.readframes(w.getnframes()), dtype="<i2")
                    play(x, f"{k:12s} {r['clip_path']}")
                    time.sleep(0.3)
        except KeyboardInterrupt:
            print("\n  stopped")


# ================================================================ main
def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[1],
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = ap.add_subparsers(dest="cmd", required=True)

    def common(p, port=True):
        if port:
            p.add_argument("--port", required=True, help="e.g. COM3")
            p.add_argument("--baud", type=int, default=921600)
            p.add_argument("--channel", default="1", choices=["0", "1", "auto"],
                           help="channel saved as the 1 s clip (default 1; ch0 is dead for now)")
        p.add_argument("--root", help="recordings folder (default: from config.py)")

    p = sub.add_parser("check", help="link + level check")
    common(p)
    p.add_argument("--seconds", type=float, default=5.0)

    p = sub.add_parser("words", help="prompted word takes")
    common(p)
    p.add_argument("--session", required=True)
    p.add_argument("--speaker")
    p.add_argument("--takes", type=int, default=10, help="kept takes per word (target)")
    p.add_argument("--unknown", type=int, default=30, help="_unknown_ takes (target)")
    p.add_argument("--words", help="comma list, e.g. move,grab,blue_box (default: all 20)")
    p.add_argument("--split", choices=["test", "train"], help="default: test for S1, else train")
    p.add_argument("--read-s", type=float, default=0.8, help="time to read the word first")

    p = sub.add_parser("noise", help="continuous noise -> 1 s _silence_ clips")
    common(p)
    p.add_argument("--session", required=True)
    p.add_argument("--speaker")
    p.add_argument("--label", required=True, choices=["room", "servo"])
    p.add_argument("--seconds", type=float, default=60.0)
    p.add_argument("--split", choices=["test", "train"])

    p = sub.add_parser("summary", help="counts, SNR, flags, listen")
    common(p, port=False)
    p.add_argument("--session", required=True)
    p.add_argument("--speaker")
    p.add_argument("--takes", type=int, default=10)
    p.add_argument("--listen", type=int, default=0, help="play N random clips per class")

    args = ap.parse_args()
    if args.root:
        C.RECORDINGS = Path(args.root).resolve()
    {"check": cmd_check, "words": cmd_words, "noise": cmd_noise, "summary": cmd_summary}[args.cmd](args)


if __name__ == "__main__":
    main()
