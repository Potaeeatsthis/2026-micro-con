r"""
export_model.py - one experiment's final model -> files for X-CUBE-AI (STM32CubeIDE), the same
package layout as RA\final_model\prefer (made with export_kws_onnx.py + make_kws_feature_data.py).
<out> = voice_command_research\final_model\<expNN_arch-size>\ (tracked in git: released models only)

    <out>\onnx_out\kws.onnx              model for X-CUBE-AI: input 1 x 40 x 101 RAW log-mel (normaliser
                                         inside), output 1 x 23 probabilities (softmax inside)
    <out>\onnx_out\kws_test_vector.h     one test input (both memory orders) + the 23 PyTorch probabilities
    <out>\kws_c\kws_clip_test.h          power-up self-test: the same clip as prefer (train00001.wav),
                                         its log-mel and the 23 probabilities of this model
    <out>\kws_c\kws_classes.h            N_CLASSES, CLS_UNKNOWN, CLS_SILENCE, CLASS_NAMES from config.py

Unchanged from prefer (same features): kws_features.c/.h, kws_stream.c/.h, kws_feature_tables.h, kissfft.

Run from voice_command_research\workspace\:
    C:/Users/ASUS/anaconda3/envs/torch_env/python.exe export_model.py exp04_tcresnet-w16d3_aug-all
"""
import argparse
import importlib.util
import re
from pathlib import Path

import numpy as np
import torch
from torch import nn

import eval_board as eb

EXPERIMENTS = Path(__file__).resolve().parent / "experiments"
OUT_ROOT = eb.C.HERE / "final_model"               # voice_command_research\final_model
PREFER = eb.C.FINAL_MODEL                          # the deployed package (self-test clip, old headers)
TEST_CLIP = PREFER / "train00001.wav"
OPSET = 13
N_MELS, N_FRAMES = 40, 101


class WithSoftmax(nn.Module):
    """board gets probabilities directly"""

    def __init__(self, net):
        super().__init__()
        self.net = net

    def forward(self, x):
        return torch.softmax(self.net(x), dim=1)


def load_model(exp):
    spec = importlib.util.spec_from_file_location("experiment_model", exp / "model.py")
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    state = torch.load(exp / "runs" / "final.pth", map_location="cpu")
    net = module.build_model(state["0.mean"].flatten(), state["0.std"].flatten(), len(eb.C.CLASSES))
    net.load_state_dict(state)
    return WithSoftmax(net).eval(), module.CONFIG


def c_floats(name, a, per_line=8):
    v = [f"{float(x):.9e}f" for x in np.asarray(a, np.float32).ravel()]
    rows = [", ".join(v[i:i + per_line]) for i in range(0, len(v), per_line)]
    return f"static const float {name}[{len(v)}] = {{\n    " + ",\n    ".join(rows) + "\n};\n"


def c_ints(name, ctype, a, per_line=12):
    v = [str(int(x)) for x in np.asarray(a).ravel()]
    rows = [", ".join(v[i:i + per_line]) for i in range(0, len(v), per_line)]
    return f"static const {ctype} {name}[{len(v)}] = {{\n    " + ",\n    ".join(rows) + "\n};\n"


def write_test_vector(path, x, y):
    """x: (40, 101). X-CUBE-AI may want either memory order; the one that reproduces test_expected is right"""
    path.write_text(
        "/* kws_test_vector.h - made by export_model.py, do not edit\n"
        " *   test_input_band_major  [band][frame]  index = band * N_FRAMES + frame   (channel first)\n"
        " *   test_input_frame_major [frame][band]  index = frame * N_MELS + band     (channel last)\n"
        " */\n#pragma once\n\n"
        f"#define TEST_N_FRAMES  {N_FRAMES}\n#define TEST_N_MELS    {N_MELS}\n"
        f"#define TEST_N_CLASSES {len(y)}\n\n"
        + c_floats("test_input_band_major", x) + "\n"
        + c_floats("test_input_frame_major", x.T) + "\n"
        + c_floats("test_expected", y))


def write_clip_test(path, model, exp_name):
    """same clip and log-mel as prefer\\kws_c\\kws_clip_test.h, probabilities of this model"""
    pcm = eb.read_wav(TEST_CLIP)[:, 0][:eb.SR]
    logmel = eb.LogMel("numpy").clip(pcm.astype(np.float32) / 32768)
    old = (PREFER / "kws_c" / "kws_clip_test.h").read_text()
    old_logmel = np.array([float(v.strip().rstrip("f")) for v in
                           re.search(r"expected_logmel\[\d+\] = \{(.*?)\};", old, re.S).group(1).split(",")])
    with torch.no_grad():
        probs = model(torch.from_numpy(logmel)[None]).numpy()[0]
    top = int(probs.argmax())
    path.write_text(
        "/* kws_clip_test.h - made by export_model.py, do not edit\n"
        f" * clip: {TEST_CLIP.name}\n"
        " * clip_pcm: 16000 int16 samples. expected_logmel: band-major [band * 101 + frame].\n"
        f" * expected_probs: {exp_name} on that log-mel (top class {top} = {eb.C.CLASSES[top]})\n"
        " */\n#pragma once\n#include <stdint.h>\n\n"
        + c_ints("clip_pcm", "int16_t", pcm) + "\n"
        + c_floats("expected_logmel", logmel) + "\n"
        + c_floats("expected_probs", probs))
    return top, float(probs[top]), float(np.abs(logmel.ravel() - old_logmel).max())


def write_classes(path, exp_name):
    names = ", ".join(f'"{c}"' for c in eb.C.CLASSES)
    path.write_text(
        "/* kws_classes.h - made by export_model.py from voice_command_research\\config.py, do not edit\n"
        f" * model: {exp_name}. Replaces N_CLASSES / CLS_UNKNOWN / CLS_SILENCE / CLASS_NAMES in app_x-cube-ai.c\n"
        " */\n#pragma once\n\n"
        f"#define N_CLASSES   {len(eb.C.CLASSES)}\n"
        f"#define CLS_UNKNOWN {eb.C.CLASSES.index(eb.C.UNKNOWN)}\n"
        f"#define CLS_SILENCE {eb.C.CLASSES.index(eb.C.SILENCE)}\n\n"
        f"static const char *const CLASS_NAMES[N_CLASSES] = {{\n  {names}\n}};\n")


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("experiment")
    args = ap.parse_args()
    exp = EXPERIMENTS / args.experiment
    cfg_name = re.match(r"exp\d+_(\w+-w\d+d\d+)", args.experiment)
    out = OUT_ROOT / (args.experiment.split("_")[0] + "_" + (cfg_name.group(1) if cfg_name else "model"))
    (out / "onnx_out").mkdir(parents=True, exist_ok=True)
    (out / "kws_c").mkdir(parents=True, exist_ok=True)

    model, cfg = load_model(exp)
    norm = model.net[0]
    x = norm.mean + norm.std * torch.randn(1, N_MELS, N_FRAMES, generator=torch.Generator().manual_seed(0))
    with torch.no_grad():
        y = model(x)
    onnx_path = out / "onnx_out" / "kws.onnx"
    torch.onnx.export(model, (x,), str(onnx_path), input_names=["logmel"], output_names=["probs"],
                      opset_version=OPSET, dynamo=False)

    import onnxruntime as ort
    sess = ort.InferenceSession(str(onnx_path), providers=["CPUExecutionProvider"])
    y_onnx = sess.run(None, {"logmel": x.numpy()})[0]
    diff = float(np.abs(y_onnx - y.numpy()).max())
    xa, ya = x.numpy()[0], y.numpy()[0]
    write_test_vector(out / "onnx_out" / "kws_test_vector.h", xa, ya)
    top, p_top, logmel_diff = write_clip_test(out / "kws_c" / "kws_clip_test.h", model, args.experiment)
    write_classes(out / "kws_c" / "kws_classes.h", args.experiment)

    n_params = sum(p.numel() for p in model.parameters())
    print(f"model      {args.experiment}: {cfg['architecture']} w{cfg['width']} d{cfg['depth']}, "
          f"{n_params:,} params (~{n_params * 4 / 1024:.0f} KB float32)")
    print(f"ONNX       {onnx_path} ({onnx_path.stat().st_size / 1024:.0f} KB, opset {OPSET}), "
          f"input {list(x.shape)}, output {list(y.shape)}")
    print(f"check      max |PyTorch - ONNX| = {diff:.2e} ({'OK' if diff < 1e-5 else 'MISMATCH'})")
    print(f"self-test  {TEST_CLIP.name}: top class {top} = {eb.C.CLASSES[top]} ({p_top:.4f}); "
          f"log-mel vs prefer header max diff {logmel_diff:.2e}")
    print(f"classes    {len(eb.C.CLASSES)}: CLS_UNKNOWN {eb.C.CLASSES.index(eb.C.UNKNOWN)}, "
          f"CLS_SILENCE {eb.C.CLASSES.index(eb.C.SILENCE)}")
    print(f"-> {out}")


if __name__ == "__main__":
    main()
