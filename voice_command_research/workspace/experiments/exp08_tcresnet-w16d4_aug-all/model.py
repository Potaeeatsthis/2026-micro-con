"""
model.py - exp08: TC-ResNet w16 d4 (two blocks deeper than deployed; ~1.6x operations, ~52 ms on the board),
23 classes, every augmentation on:
stream-like word positions, board level range, noise / babble / events, room echo, mild SpecAugment.

The network is the same as TCResNet in final_model\prefer\final_model_info\export_kws_onnx.py
(keep the two identical so the trained weights export to the board unchanged).

Run from voice_command_research\workspace\:
    C:/Users/ASUS/anaconda3/envs/torch_env/python.exe train.py exp08_tcresnet-w16d4_aug-all
"""
import torch
from torch import nn

CONFIG = dict(
    architecture="tcresnet", width=16, depth=4, dropout=0.2,
    # augmentation switches (the remove-one experiments turn one off)
    stream_positions=True,            # False: word always centred in the window
    noise=True,                       # False: quiet room floor only
    babble=True,                      # False: babble windows get room / indoor noise instead
    reverb=True,                      # False: no room echo
    specaug=True,                     # False: no SpecAugment
    epochs=82,                        # all-data run: median best epoch of exp01's 5 folds (53, 82, 73, 83, 96)
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


class ResBlock(nn.Module):
    def __init__(self, c_in, c_out, kernel=9):
        super().__init__()
        self.main = nn.Sequential(
            nn.Conv1d(c_in, c_out, kernel, stride=2, padding=kernel // 2, bias=False),
            nn.BatchNorm1d(c_out), nn.ReLU(),
            nn.Conv1d(c_out, c_out, kernel, padding=kernel // 2, bias=False),
            nn.BatchNorm1d(c_out))
        self.shortcut = nn.Sequential(nn.Conv1d(c_in, c_out, 1, stride=2, bias=False), nn.BatchNorm1d(c_out))
        self.relu = nn.ReLU()

    def forward(self, x):
        return self.relu(self.main(x) + self.shortcut(x))


class TCResNet(nn.Module):
    def __init__(self, width=16, depth=4, dropout=0.2, n_mels=40, n_classes=22):
        super().__init__()
        channels = [width + i * width // 2 for i in range(depth + 1)]
        self.stem = nn.Conv1d(n_mels, channels[0], 3, padding=1, bias=False)
        self.blocks = nn.Sequential(*[ResBlock(channels[i], channels[i + 1]) for i in range(depth)])
        self.head = nn.Sequential(nn.AdaptiveAvgPool1d(1), nn.Flatten(), nn.Dropout(dropout),
                                  nn.Linear(channels[-1], n_classes))

    def forward(self, x):                       # x: (batch, 40, frames), mel bands = channels
        return self.head(self.blocks(self.stem(x)))


def build_model(mean, std, n_classes):
    """nn.Sequential(Normalize, TCResNet): the layout eval_board.py and the ONNX export expect"""
    net = TCResNet(CONFIG["width"], CONFIG["depth"], CONFIG["dropout"], n_classes=n_classes)
    return nn.Sequential(Normalize(mean, std), net)
