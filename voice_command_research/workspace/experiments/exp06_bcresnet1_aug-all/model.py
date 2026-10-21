"""
model.py - exp06: BC-ResNet-1 (2D, Kim et al. 2021, "Broadcasted Residual Learning for Efficient Keyword
Spotting"), 23 classes, every augmentation on (same recipe as exp01). Question: is the plateau a limit of
the 1D TC-ResNet? About 8k parameters, ~2.3x the operations of the deployed model, but ~189 KB peak
activation memory in float32 (does not fit the board unless quantized; this run only answers the question).
Simplification: sub-spectral normalisation is replaced by plain BatchNorm.

Run from voice_command_research\workspace\:
    C:/Users/ASUS/anaconda3/envs/torch_env/python.exe train.py exp06_bcresnet1_aug-all
"""
import torch
from torch import nn

CONFIG = dict(
    architecture="bcresnet", width=8, depth=12, dropout=0.1,    # width = first stage channels, depth = blocks
    # augmentation switches (the remove-one experiments turn one off)
    stream_positions=True,            # False: word always centred in the window
    noise=True,                       # False: quiet room floor only
    babble=True,                      # False: babble windows get room / indoor noise instead
    reverb=True,                      # False: no room echo
    specaug=True,                     # False: no SpecAugment
    epochs=82,                        # same as exp01 (median best epoch of its 5 folds)
    batch=256, windows_per_epoch=40000, max_epochs=100, patience=12,
    lr=1e-3, weight_decay=1e-2, label_smoothing=0.05, ema_decay=0.999,
    versions=3,                       # fixed augmented versions per clip; one is picked each epoch
    piper_share_unknown=0.45,         # share of Piper clips among _unknown_ draws
    noise_only_share_silence=0.5,     # _silence_ draws: noise-bank window instead of a silence clip
    gain_jitter_db=3.0,
    specaug_share=0.5, time_mask_max=10, freq_mask_max=5,
    seed=0,
)


class Normalize(nn.Module):
    """(x - mean) / std per mel band, stored with the model so the board feeds raw log-mel"""

    def __init__(self, mean, std):
        super().__init__()
        self.register_buffer("mean", mean.reshape(1, -1, 1).float())
        self.register_buffer("std", std.reshape(1, -1, 1).float())

    def forward(self, x):
        return (x - self.mean) / self.std


class BCBlock(nn.Module):
    """frequency-depthwise 2D conv, then a 1D temporal conv on the frequency average, broadcast back"""

    def __init__(self, c_in, c_out, stride, dilation, dropout):
        super().__init__()
        self.transition = nn.Sequential(nn.Conv2d(c_in, c_out, 1, bias=False), nn.BatchNorm2d(c_out), nn.ReLU()) \
            if c_in != c_out else nn.Identity()
        self.freq = nn.Sequential(nn.Conv2d(c_out, c_out, (3, 1), stride=(stride, 1), padding=(1, 0),
                                            groups=c_out, bias=False), nn.BatchNorm2d(c_out))
        self.temporal = nn.Sequential(nn.Conv2d(c_out, c_out, (1, 3), padding=(0, dilation), dilation=(1, dilation),
                                                groups=c_out, bias=False), nn.BatchNorm2d(c_out), nn.SiLU(),
                                      nn.Conv2d(c_out, c_out, 1, bias=False), nn.Dropout2d(dropout))
        self.residual = c_in == c_out and stride == 1

    def forward(self, x):
        x = self.transition(x)
        f = self.freq(x)
        out = f + self.temporal(f.mean(dim=2, keepdim=True))
        if self.residual:
            out = out + x
        return torch.relu(out)


class BCResNet(nn.Module):
    def __init__(self, base=8, dropout=0.1, n_classes=23):
        super().__init__()
        c = [base, base * 3 // 2, base * 2, base * 5 // 2]       # 8, 12, 16, 20 for BC-ResNet-1
        self.stem = nn.Sequential(nn.Conv2d(1, 2 * c[0], 5, stride=(2, 1), padding=2, bias=False),
                                  nn.BatchNorm2d(2 * c[0]), nn.ReLU())
        plan = [(2 * c[0], c[0], 1, 1), (c[0], c[0], 1, 1), (c[0], c[1], 2, 2), (c[1], c[1], 1, 2),
                (c[1], c[2], 2, 4), (c[2], c[2], 1, 4), (c[2], c[2], 1, 4), (c[2], c[2], 1, 4),
                (c[2], c[3], 1, 8), (c[3], c[3], 1, 8), (c[3], c[3], 1, 8), (c[3], c[3], 1, 8)]
        self.blocks = nn.Sequential(*[BCBlock(i, o, s, d, dropout) for i, o, s, d in plan])
        self.head = nn.Sequential(nn.Conv2d(c[3], c[3], (5, 5), groups=c[3], padding=(0, 2), bias=False),
                                  nn.Conv2d(c[3], 32, 1, bias=False), nn.BatchNorm2d(32), nn.ReLU(),
                                  nn.AdaptiveAvgPool2d(1), nn.Flatten(), nn.Linear(32, n_classes))

    def forward(self, x):                        # (batch, 40 mel, 101 frames) -> (batch, 1, 40, 101)
        return self.head(self.blocks(self.stem(x.unsqueeze(1))))


def build_model(mean, std, n_classes):
    """nn.Sequential(Normalize, network): the layout train.py and eval_board.py --model-py expect"""
    return nn.Sequential(Normalize(mean, std), BCResNet(CONFIG["width"], CONFIG["dropout"], n_classes))
