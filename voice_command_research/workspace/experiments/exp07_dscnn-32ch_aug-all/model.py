"""
model.py - exp07: DS-CNN with 32 channels (2D, Zhang et al. 2017, "Hello Edge: Keyword Spotting on
Microcontrollers"), first conv stride (2, 4) to keep memory down, 23 classes, every augmentation on (same
recipe as exp01). Question: is the plateau a limit of the 1D TC-ResNet? About 8k parameters, ~3x the
operations of the deployed model, ~128 KB peak activation memory in float32 (needs quantization to fit).

Run from voice_command_research\workspace\:
    C:/Users/ASUS/anaconda3/envs/torch_env/python.exe train.py exp07_dscnn-32ch_aug-all
"""
from torch import nn

CONFIG = dict(
    architecture="dscnn", width=32, depth=4, dropout=0.2,       # width = channels, depth = separable blocks
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


class DSCNN(nn.Module):
    def __init__(self, channels=32, blocks=4, dropout=0.2, n_classes=23):
        super().__init__()
        layers = [nn.Conv2d(1, channels, (10, 4), stride=(2, 4), padding=(5, 1), bias=False),
                  nn.BatchNorm2d(channels), nn.ReLU()]
        for _ in range(blocks):
            layers += [nn.Conv2d(channels, channels, 3, padding=1, groups=channels, bias=False),
                       nn.BatchNorm2d(channels), nn.ReLU(),
                       nn.Conv2d(channels, channels, 1, bias=False), nn.BatchNorm2d(channels), nn.ReLU()]
        self.body = nn.Sequential(*layers)
        self.head = nn.Sequential(nn.AdaptiveAvgPool2d(1), nn.Flatten(), nn.Dropout(dropout),
                                  nn.Linear(channels, n_classes))

    def forward(self, x):                        # (batch, 40 mel, 101 frames) -> (batch, 1, 101 time, 40 mel)
        return self.head(self.body(x.transpose(1, 2).unsqueeze(1)))


def build_model(mean, std, n_classes):
    """nn.Sequential(Normalize, network): the layout train.py and eval_board.py --model-py expect"""
    return nn.Sequential(Normalize(mean, std), DSCNN(CONFIG["width"], CONFIG["depth"], CONFIG["dropout"], n_classes))
