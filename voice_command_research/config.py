"""
config.py - paths, audio settings and the class list for voice_command_research.

This is the ONLY copy of the class order. Every script imports it; firmware class arrays
are generated from it or checked against it.

    import config as C
    C.CLASSES   # 23 classes: 21 commands, then _unknown_, _silence_
"""
from pathlib import Path

# ---------------------------------------------------------------- paths
HERE = Path(__file__).resolve().parent               # voice_command_research\
DATASET = HERE / "dataset"
RECORDINGS = DATASET / "recordings"                 # board recordings (recorder.py writes here)
SYNTHETIC = DATASET / "synthetic"                   # Piper voices + text-to-speech clips
BUILT = DATASET / "built"                           # generated training datasets
WORKSPACE = HERE / "workspace"

FINAL_MODEL = Path(r"D:\Microcon69\ProjectRoboticArm\final_model\prefer")     # deployed model + C export
DA_DATA = Path(r"D:\data_analytics\DA_Project\voice_command_classification\dataset")  # open data, read-only
OPEN_NOISE = Path(r"D:\Microcon69\ProjectRoboticArm\open_noise")  # OpenSLR 28 (RIRS_NOISES) + ESC-50, outside git

# ---------------------------------------------------------------- audio
SR = 16000

# ---------------------------------------------------------------- vocabulary (locked 7 Oct, undo added 20 Oct 2026)
# token -> meaning group used by the state machine (synonyms share a group)
TOKENS = {
    "move": "trigger", "pick": "trigger", "grab": "trigger",
    "blue box": "object", "green box": "object", "red box": "object", "black box": "object",
    "hand": "destination", "station": "destination", "base": "destination",
    "forward": "jog", "backward": "jog", "left": "jog", "right": "jog",
    "place": "release", "drop": "release",
    "yes": "confirm", "no": "reject",
    "home": "home",
    "stop": "stop",
    "undo": "undo",
}
UNKNOWN, SILENCE = "_unknown_", "_silence_"
CLASSES = list(TOKENS) + [UNKNOWN, SILENCE]   # 23 classes: undo 20, _unknown_ 21, _silence_ 22

# the deployed model (final_model.pth / kws.onnx) has the 22 outputs from before undo
CLASSES_22 = [c for c in CLASSES if c != "undo"]


def token_dir(token):
    """folder-safe name: 'blue box' -> 'blue_box'"""
    return token.replace(" ", "_")
