"""
features.py - log-mel exactly as the live firmware stream computes it (no centre padding).

    logmel(x)          numpy, (n_samples,) -> (40, T)        reference, CPU
    LogMelTorch()(X)   torch, (batch, n_samples) -> (batch, 40, T) on the GPU, for training

Frame f = x[160 f : 160 f + 400], periodic Hann, |rfft|^2, 40 slaney mel bands 20-8000 Hz,
ln(mel + 1e-6). A window of WINDOW_SAMPLES = 16400 samples gives exactly the 101 frames
the CNN sees on the board. Check against the board tables:

    C:/Users/ASUS/anaconda3/envs/torch_env/python.exe features.py
"""
from pathlib import Path

import torch                    # before librosa/numpy: otherwise two OpenMP runtimes load (Windows)
import librosa
import numpy as np

SR = 16000
N_FFT = 400
HOP = 160
N_MELS = 40
N_FRAMES = 101
WINDOW_SAMPLES = N_FFT + HOP * (N_FRAMES - 1)          # 16400
FMIN, FMAX = 20, SR // 2
LOG_EPS = 1e-6

HANN = (0.5 - 0.5 * np.cos(2 * np.pi * np.arange(N_FFT) / N_FFT)).astype(np.float32)
MEL_BASIS = librosa.filters.mel(sr=SR, n_fft=N_FFT, n_mels=N_MELS, fmin=FMIN, fmax=FMAX).astype(np.float32)


def logmel(x):
    """float waveform -> (40, T) raw log-mel, board stream framing"""
    x = np.asarray(x, dtype=np.float32)
    n_frames = 1 + (len(x) - N_FFT) // HOP
    index = np.arange(N_FFT)[None, :] + HOP * np.arange(n_frames)[:, None]
    spec = np.fft.rfft(x[index] * HANN, axis=1)
    power = (spec.real ** 2 + spec.imag ** 2).astype(np.float32)
    return np.log(power @ MEL_BASIS.T + LOG_EPS).T


class LogMelTorch(torch.nn.Module):
    """(batch, n_samples) float -> (batch, 40, T) raw log-mel, same framing as logmel()"""

    def __init__(self):
        super().__init__()
        self.register_buffer("window", torch.from_numpy(HANN))
        self.register_buffer("mel_basis", torch.from_numpy(MEL_BASIS))

    def forward(self, x):
        spec = torch.stft(x, N_FFT, HOP, N_FFT, self.window, center=False, return_complex=True)
        power = spec.real ** 2 + spec.imag ** 2                          # (batch, 201, T)
        return torch.log(torch.matmul(self.mel_basis, power) + LOG_EPS)


def check():
    """compare with the board tables (kws_feature_tables.h) and the board's own test clip"""
    import re
    import sys
    sys.path.insert(0, str(Path(__file__).resolve().parent.parent))
    import config as C

    def c_array(text, name):
        body = re.search(rf"\b{name}\s*\[\s*\d*\s*\]\s*=\s*\{{(.*?)\}};", text, re.S).group(1)
        body = re.sub(r"/\*.*?\*/", "", body, flags=re.S)
        return np.array([float(v.rstrip("fF")) for v in
                         re.findall(r"[-+]?[0-9.]+(?:[eE][-+]?\d+)?[fF]?", body)])

    kws_c = C.FINAL_MODEL / "kws_c"
    tables = (kws_c / "kws_feature_tables.h").read_text()
    start, length, weights = (c_array(tables, n) for n in ("KWS_MEL_START", "KWS_MEL_LEN", "KWS_MEL_W"))
    board_basis = np.zeros_like(MEL_BASIS)
    k = 0
    for band in range(N_MELS):
        s, n = int(start[band]), int(length[band])
        board_basis[band, s:s + n] = weights[k:k + n]
        k += n
    window_diff = np.abs(c_array(tables, "KWS_WINDOW") - HANN).max()
    basis_diff = np.abs(board_basis - MEL_BASIS).max()

    clip_test = (kws_c / "kws_clip_test.h").read_text()
    pcm = c_array(clip_test, "clip_pcm").astype(np.float32) / 32768
    expected = c_array(clip_test, "expected_logmel").reshape(N_MELS, N_FRAMES)
    padded = np.pad(pcm, (N_FFT // 2, N_FFT // 2))        # the board test clip was made with centre padding
    numpy_diff = np.abs(logmel(padded) - expected).max()

    device = "cuda" if torch.cuda.is_available() else "cpu"
    rng = np.random.default_rng(0)
    batch = np.stack([padded] + [rng.normal(0, 10 ** rng.uniform(-4, -0.5), WINDOW_SAMPLES).astype(np.float32)
                                 for _ in range(31)])
    gpu = LogMelTorch().to(device)(torch.from_numpy(batch).to(device)).cpu().numpy()
    torch_diff = max(np.abs(gpu[i] - logmel(batch[i])).max() for i in range(len(batch)))

    print(f"window vs board {window_diff:.2e} | mel basis vs board {basis_diff:.2e}")
    print(f"board test clip: numpy log-mel vs expected_logmel {numpy_diff:.2e} | shape {logmel(padded).shape}")
    print(f"torch ({device}) vs numpy on 32 windows of {WINDOW_SAMPLES} samples: max diff {torch_diff:.2e} "
          f"| shape {gpu.shape[1:]}")
    ok = window_diff < 1e-6 and basis_diff < 1e-6 and numpy_diff < 1e-3 and torch_diff < 1e-3
    print("CHECK", "PASSED" if ok else "FAILED")


if __name__ == "__main__":
    check()
