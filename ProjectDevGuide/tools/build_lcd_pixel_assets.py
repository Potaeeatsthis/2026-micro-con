#!/usr/bin/env python3
"""build_lcd_pixel_assets.py - 8-bit pixel-art look for the touch-screen UI (replaces the pictures only).

    python3 ProjectDevGuide/tools/build_lcd_pixel_assets.py RoboticArmMain/App/lcd [preview_dir]
    (needs: pip install pillow. no font files: the 5x7 pixel font is inside this script)

contract with lcd.c (unchanged): lcd_assets.h is NOT rewritten. every picture keeps its id, size and
position from lcd_assets.h, so touch zones, page code and the digit fields stay the same. this script
only rewrites lcd_assets.c and lcd_assets_arm.c (same RLE format as build_lcd_assets.py).

colours lcd.c uses at run time and that the pictures must match:
    page background  (8, 11, 16)    LCD_BG
    X/Y/Z row fill   (13, 17, 23)   LCD_ROW_BG, the digits are drawn on it (field 50 x 14 in each row)
    slider track     3 px white line drawn by lcd.c at slider y + 9, knob 12 x 12 centred on it

style: everything sits on whole pixels, no anti-aliasing. text = 5x7 pixel font at 1x / 2x / 3x.
theme orange / white like the real arm: white text, one orange accent for what you can press
next or what is selected.

RLE format (one byte header, then pixels as big-endian RGB565 = the order the panel wants):
    0..127   : run of (header + 3) identical pixels, 2 bytes follow
    128..255 : literal block of (header - 127) pixels, 2 bytes each follow
"""
import re
import sys
from pathlib import Path
from PIL import Image, ImageDraw

# ---------------------------------------------------------------- palette
BG = (8, 11, 16)            # = LCD_BG
ROW = (13, 17, 23)          # = LCD_ROW_BG
PANEL = ROW
EDGE = (58, 66, 80)
EDGE_HI = (96, 106, 122)
TEXT = (236, 238, 230)
DIM = (128, 136, 148)
DARK = (8, 11, 16)
# orange ramp (light -> deep), used for 8-bit shading: highlight on top, shadow below
OR_HI = (255, 186, 110)
AMBER = (255, 122, 26)       # accent: the arm's orange
OR_MID = (232, 96, 14)
AMBER_DK = (176, 64, 6)
OR_DEEP = (96, 34, 4)
ARM_HI = (255, 255, 255)     # arm body: white
ARM_MID = (226, 228, 232)
ARM_LO = (255, 122, 26)      # arm covers / base: orange
ARM_JOINT = (96, 34, 4)
BOXES = [("RED", (226, 52, 44)), ("GREEN", (72, 196, 80)), ("BLACK", (70, 72, 80)), ("BLUE", (52, 120, 255))]
AXIS = [(236, 64, 56), (80, 210, 90), (70, 140, 255)]

# ---------------------------------------------------------------- 5x7 pixel font
F = {
    "A": "01110 10001 10001 11111 10001 10001 10001", "B": "11110 10001 10001 11110 10001 10001 11110",
    "C": "01110 10001 10000 10000 10000 10001 01110", "D": "11110 10001 10001 10001 10001 10001 11110",
    "E": "11111 10000 10000 11110 10000 10000 11111", "F": "11111 10000 10000 11110 10000 10000 10000",
    "G": "01110 10001 10000 10111 10001 10001 01111", "H": "10001 10001 10001 11111 10001 10001 10001",
    "I": "01110 00100 00100 00100 00100 00100 01110", "J": "00111 00010 00010 00010 00010 10010 01100",
    "K": "10001 10010 10100 11000 10100 10010 10001", "L": "10000 10000 10000 10000 10000 10000 11111",
    "M": "10001 11011 10101 10101 10001 10001 10001", "N": "10001 10001 11001 10101 10011 10001 10001",
    "O": "01110 10001 10001 10001 10001 10001 01110", "P": "11110 10001 10001 11110 10000 10000 10000",
    "Q": "01110 10001 10001 10001 10101 10010 01101", "R": "11110 10001 10001 11110 10100 10010 10001",
    "S": "01111 10000 10000 01110 00001 00001 11110", "T": "11111 00100 00100 00100 00100 00100 00100",
    "U": "10001 10001 10001 10001 10001 10001 01110", "V": "10001 10001 10001 10001 10001 01010 00100",
    "W": "10001 10001 10001 10101 10101 10101 01010", "X": "10001 10001 01010 00100 01010 10001 10001",
    "Y": "10001 10001 01010 00100 00100 00100 00100", "Z": "11111 00001 00010 00100 01000 10000 11111",
    "0": "01110 10001 10011 10101 11001 10001 01110", "1": "00100 01100 00100 00100 00100 00100 01110",
    "2": "01110 10001 00001 00010 00100 01000 11111", "3": "11111 00010 00100 00010 00001 10001 01110",
    "4": "00010 00110 01010 10010 11111 00010 00010", "5": "11111 10000 11110 00001 00001 10001 01110",
    "6": "00110 01000 10000 11110 10001 10001 01110", "7": "11111 00001 00010 00100 01000 01000 01000",
    "8": "01110 10001 10001 01110 10001 10001 01110", "9": "01110 10001 10001 01111 00001 00010 01100",
    "-": "00000 00000 00000 11111 00000 00000 00000", "+": "00000 00100 00100 11111 00100 00100 00000",
    ".": "00000 00000 00000 00000 00000 01100 01100", ",": "00000 00000 00000 00000 01100 00100 01000",
    ":": "00000 01100 01100 00000 01100 01100 00000", "?": "01110 10001 00001 00010 00100 00000 00100",
    "!": "00100 00100 00100 00100 00100 00000 00100", "/": "00000 00001 00010 00100 01000 10000 00000",
    "<": "00010 00100 01000 10000 01000 00100 00010", ">": "01000 00100 00010 00001 00010 00100 01000",
    '"': "01010 01010 01010 00000 00000 00000 00000", "'": "00100 00100 01000 00000 00000 00000 00000",
    "(": "00010 00100 01000 01000 01000 00100 00010", ")": "01000 00100 00010 00010 00010 00100 01000",
    "=": "00000 00000 11111 00000 11111 00000 00000", "%": "11000 11001 00010 00100 01000 10011 00011",
}
SPACE_COLS = 3   # a space is narrower than a letter (keeps labels short)


def text_w(s, sc):
    return sum((SPACE_COLS if c == " " else 6) * sc for c in s) - sc


def draw_text(im, x, y, s, sc, col):
    """top-left at x, y; 5x7 glyphs, 1 column gap, every font pixel = sc x sc screen pixels"""
    px = im.load()
    for c in s:
        if c == " ":
            x += SPACE_COLS * sc
            continue
        rows = F[c].split()
        for r, bits in enumerate(rows):
            for k, b in enumerate(bits):
                if b == "1":
                    for dy in range(sc):
                        for dx in range(sc):
                            xx, yy = x + k * sc + dx, y + r * sc + dy
                            if 0 <= xx < im.width and 0 <= yy < im.height:
                                px[xx, yy] = col
        x += 6 * sc


def text_c(im, cx, y, s, sc, col):
    draw_text(im, cx - text_w(s, sc) // 2, y, s, sc, col)


# ---------------------------------------------------------------- 8x8 icons (drawn at 2x = 16x16)
ICONS = {
    "servo":  ["00111100", "01000010", "10011001", "10111101", "10111101", "10011001", "01000010", "00111100"],
    "group":  ["00011000", "01100110", "10000001", "01100110", "10011001", "01100110", "00011000", "00000000"],
    "axes":   ["10000000", "10000010", "10000111", "10000010", "10000000", "10001000", "11111110", "00001000"],
    "mic":    ["00111000", "00111000", "00111000", "10111010", "10111010", "01000100", "00111000", "00010000"],
    "camera": ["00000000", "00111000", "11111111", "10001001", "10111101", "10001001", "11111111", "00000000"],
    "pin":    ["00111100", "01111110", "01100110", "01111110", "00111100", "00011000", "00011000", "00000000"],
    "down":   ["00011000", "00011000", "00011000", "00011000", "11011011", "01111110", "00111100", "00011000"],
    "slider": ["01000000", "11111111", "01000000", "00000100", "11111111", "00000100", "00100000", "11111111"],
    "play":   ["11000000", "11110000", "11111100", "11111111", "11111111", "11111100", "11110000", "11000000"],
    "check":  ["00000001", "00000011", "00000110", "10001100", "11011000", "01110000", "00100000", "00000000"],
    "dots":   ["00000000", "00000000", "00000000", "11011011", "11011011", "00000000", "00000000", "00000000"],
    "home":   ["00011000", "00111100", "01111110", "11111111", "01100110", "01100110", "01111110", "00000000"],
    "box":    ["00111100", "01111110", "11111111", "11111111", "11111111", "11111111", "01111110", "00111100"],
}


def draw_icon(im, x, y, name, sc, col):
    px = im.load()
    for r, bits in enumerate(ICONS[name]):
        for k, b in enumerate(bits):
            if b == "1":
                for dy in range(sc):
                    for dx in range(sc):
                        xx, yy = x + k * sc + dx, y + r * sc + dy
                        if 0 <= xx < im.width and 0 <= yy < im.height:
                            px[xx, yy] = col


# ---------------------------------------------------------------- pixel boxes
def canvas(w, h, fill=BG):
    im = Image.new("RGB", (w, h), fill)
    return im, ImageDraw.Draw(im)


def pbox(d, x0, y0, x1, y1, fill, edge, t=2, notch=2, outside=BG):
    """pixel panel: t px border, corners notched by `notch` px (the 8-bit rounded corner)"""
    d.rectangle((x0, y0, x1, y1), fill=edge)
    d.rectangle((x0 + t, y0 + t, x1 - t, y1 - t), fill=fill)
    for cx, cy in ((x0, y0), (x1 - notch + 1, y0), (x0, y1 - notch + 1), (x1 - notch + 1, y1 - notch + 1)):
        d.rectangle((cx, cy, cx + notch - 1, cy + notch - 1), fill=outside)


def obtn(d, x0, y0, x1, y1):
    """orange 8-bit button: deep outline, light top bevel, dark bottom bevel"""
    pbox(d, x0, y0, x1, y1, AMBER, OR_DEEP)
    d.rectangle((x0 + 2, y0 + 2, x1 - 2, y0 + 3), fill=OR_HI)
    d.rectangle((x0 + 2, y1 - 3, x1 - 2, y1 - 2), fill=OR_MID)


def draw_icon2(im, x, y, name, sc):
    """orange icon with a one-pixel dark drop shadow"""
    draw_icon(im, x + sc, y + sc, name, sc, AMBER_DK)
    draw_icon(im, x, y, name, sc, AMBER)


def header(icon, title):
    im, d = canvas(320, 40)
    draw_icon2(im, 14, 12, icon, 2)
    draw_text(im, 40, 13, title, 2, TEXT)
    d.rectangle((0, 38, 319, 39), fill=EDGE)
    d.rectangle((0, 38, 40 + text_w(title, 2), 39), fill=AMBER)   # orange underline under the title
    d.rectangle((0, 38, 30, 39), fill=OR_HI)
    return im


def btn_back():
    im, d = canvas(78, 28)
    pbox(d, 0, 0, 77, 27, PANEL, EDGE)
    text_c(im, 39, 7, "< BACK", 2, TEXT)
    return im


def btn_next():
    im, d = canvas(78, 28)
    obtn(d, 0, 0, 77, 27)
    text_c(im, 39, 7, "NEXT >", 2, DARK)
    return im


def row(lab):
    """X/Y/Z row. the digit field (local x 36..86, y 6..20) must stay plain ROW: lcd.c draws there"""
    im, d = canvas(133, 27)
    pbox(d, 0, 0, 132, 26, ROW, EDGE)
    draw_text(im, 10, 7, lab, 2, AMBER)
    draw_text(im, 107, 10, "MM", 1, DIM)
    return im


def selector(w):
    im, d = canvas(w, 28)
    pbox(d, 0, 0, w - 1, 27, PANEL, EDGE)
    draw_text(im, 8, 7, "<", 2, AMBER)
    draw_text(im, w - 18, 7, ">", 2, AMBER)
    return im


def tile(w, h, s, sc, col, fill=PANEL):
    im, _ = canvas(w, h, fill)
    text_c(im, w // 2, (h - 7 * sc) // 2, s, sc, col)
    return im


def tile_left(w, h, s, sc, col, fill=BG, x=2):
    im, _ = canvas(w, h, fill)
    draw_text(im, x, (h - 7 * sc) // 2, s, sc, col)
    return im


# ---------------------------------------------------------------- pixel-art arm
# drawn on a half-resolution grid (every arm pixel = 2x2 screen pixels): base, turret, upper arm,
# forearm, wrist, gripper. joint centres in half-res coords of an 84 x 77 sprite.
JOINTS = [(38, 64), (38, 52), (38, 30), (62, 22), (70, 36), (70, 44)]   # J1 base .. J6 gripper
JOINT_NAMES = ["BASE", "SHOULDER", "ELBOW", "WRIST", "ROLL", "GRIP"]


def thick_line(d, a, b, w, col):
    d.line([a, b], fill=col, width=w)


def arm_sprite(hl=()):
    W, H = 84, 77
    im, d = canvas(W, H)
    # base plate + turret
    d.rectangle((22, 68, 54, 72), fill=AMBER)            # base plate: light top, mid, deep bottom
    d.rectangle((22, 68, 54, 68), fill=OR_HI)
    d.rectangle((22, 72, 54, 72), fill=AMBER_DK)
    d.rectangle((24, 66, 52, 67), fill=OR_DEEP)
    d.rectangle((30, 56, 46, 65), fill=ARM_MID)          # turret: white with an orange band
    d.rectangle((30, 56, 46, 57), fill=ARM_HI)
    d.rectangle((30, 60, 46, 62), fill=AMBER)
    d.rectangle((44, 58, 46, 65), fill=(190, 194, 200))
    # upper arm (shoulder -> elbow), forearm (elbow -> wrist), wrist link
    for (a, b, w) in ((JOINTS[1], JOINTS[2], 7), (JOINTS[2], JOINTS[3], 6), (JOINTS[3], JOINTS[4], 4)):
        thick_line(d, (a[0] + 1, a[1] + 1), (b[0] + 1, b[1] + 1), w, (176, 180, 188))   # shadow side
        thick_line(d, a, b, w, ARM_MID)
        thick_line(d, (a[0] - 1, a[1] - 1), (b[0] - 1, b[1] - 1), 2, ARM_HI)
    # orange covers in the middle of the upper arm and the forearm
    for (a, b) in ((JOINTS[1], JOINTS[2]), (JOINTS[2], JOINTS[3])):
        mx, my = (a[0] + b[0]) // 2, (a[1] + b[1]) // 2
        d.rectangle((mx - 2, my - 2, mx + 2, my + 2), fill=AMBER)
        d.rectangle((mx - 2, my - 2, mx + 2, my - 2), fill=OR_HI)
        d.rectangle((mx - 2, my + 2, mx + 2, my + 2), fill=AMBER_DK)
    # gripper: palm + two fingers
    gx, gy = JOINTS[5]
    d.rectangle((gx - 5, gy - 2, gx + 5, gy + 1), fill=AMBER)
    d.rectangle((gx - 5, gy - 2, gx + 5, gy - 2), fill=OR_HI)
    d.rectangle((gx - 5, gy + 1, gx + 5, gy + 1), fill=AMBER_DK)
    d.rectangle((gx - 5, gy + 2, gx - 3, gy + 9), fill=ARM_HI)
    d.rectangle((gx + 3, gy + 2, gx + 5, gy + 9), fill=ARM_HI)
    # joint hubs
    for i, (x, y) in enumerate(JOINTS[1:5], start=1):
        d.rectangle((x - 2, y - 2, x + 2, y + 2), fill=ARM_JOINT)
        d.rectangle((x - 1, y - 1, x + 1, y + 1), fill=OR_MID)
    # highlight: amber square ring around the chosen joints
    for k in hl:
        x, y = JOINTS[k]
        r = 5 if k else 7
        d.rectangle((x - r, y - r, x + r, y + r), outline=AMBER)
        d.rectangle((x - r + 1, y - r + 1, x + r - 1, y + r - 1), outline=ARM_HI)
    return im.resize((W * 2, H * 2), Image.NEAREST)


def arm_tile(w, h, hl=(), axes=False, tags=()):
    im, d = canvas(w, h)
    spr = arm_sprite(hl)
    ox, oy = w - spr.width, h - spr.height
    im.paste(spr, (ox, oy))
    for k in tags:                                  # "J1" tag up-left of the highlighted joint
        x, y = JOINTS[k]
        r = 7 if k == 0 else 5
        lab = f"J{k + 1}"
        tx, ty = ox + (x - r) * 2 - text_w(lab, 1) - 6, oy + (y - r) * 2 - 4
        d.rectangle((tx - 2, ty - 2, tx + text_w(lab, 1) + 1, ty + 8), fill=AMBER)
        draw_text(im, tx, ty, lab, 1, DARK)
    if axes:                                        # X / Y / Z arrows at the tool, labels at the tips
        cx, cy = ox + JOINTS[5][0] * 2 - 24, oy + JOINTS[5][1] * 2 + 22
        for (dx, dy), lab, col in zip(((28, 0), (-18, 14), (0, -28)), "XYZ", AXIS):
            d.line([(cx, cy), (cx + dx, cy + dy)], fill=col, width=2)
            d.rectangle((cx + dx - 2, cy + dy - 2, cx + dx + 1, cy + dy + 1), fill=col)
            draw_text(im, cx + dx + (5 if dx > 0 else -9 if dx < 0 else -2), cy + dy - (12 if dy < 0 else 3), lab, 1, col)
        d.rectangle((cx - 2, cy - 2, cx + 1, cy + 1), fill=TEXT)
    return im


# ---------------------------------------------------------------- pages
def page_home():
    im, d = canvas(320, 240)
    draw_text(im, 20, 40, "ROBOTIC", 3, TEXT)
    draw_text(im, 23, 69, "ARM", 3, OR_DEEP)
    draw_text(im, 20, 66, "ARM", 3, AMBER)
    draw_text(im, 20, 96, "6-DOF CONTROLLER", 1, DIM)
    im.paste(arm_sprite(), (152, 72))                           # whole pixels: the sprite at 2x, as on the mode pages
    obtn(d, 19, 139, 190, 186)               # touch zone 19,139,172,48
    draw_icon(im, 44, 155, "play", 2, DARK)
    draw_text(im, 70, 156, "START", 2, DARK)
    d.rectangle((0, 224, 319, 225), fill=EDGE)
    return im


def page_select():
    im, d = canvas(320, 240)
    cards = [((21, 16), "servo", "SINGLE", "ONE SERVO"), ((170, 16), "group", "GROUP", "SERVO PAIRS"),
             ((21, 129), "axes", "CARTESIAN", "MOVE IN X Y Z"), ((170, 129), "mic", "VOICE", "SAY A COMMAND")]
    for (x, y), icon, title, sub in cards:                     # touch zones 132 x 93
        pbox(d, x, y, x + 131, y + 92, PANEL, EDGE)
        draw_icon(im, x + 52, y + 16, icon, 4, OR_DEEP)
        draw_icon(im, x + 50, y + 14, icon, 4, AMBER)
        text_c(im, x + 66, y + 54, title, 2, TEXT)
        text_c(im, x + 66, y + 74, sub, 1, DIM)
    return im


def panel_cam():
    im, d = canvas(289, 103)
    pbox(d, 0, 0, 288, 102, BG, EDGE)
    draw_icon(im, 8, 6, "camera", 1, AMBER)
    draw_text(im, 22, 7, "CAMERA SEES", 1, DIM)
    return im


def card(name, col):
    im, d = canvas(131, 35)
    pbox(d, 0, 0, 130, 34, PANEL, EDGE, outside=BG)
    draw_icon(im, 8, 9, "box", 2, col)
    draw_text(im, 32, 11, name, 2, TEXT)
    return im


def check():
    im, _ = canvas(15, 15, PANEL)
    draw_icon(im, -1, 0, "check", 2, AMBER)
    return im


def panel_pick():
    im, d = canvas(289, 40)
    pbox(d, 0, 0, 288, 39, PANEL, EDGE)
    draw_icon(im, 8, 4, "mic", 1, AMBER)
    draw_text(im, 22, 5, "PICK COLOR", 1, DIM)
    return im


def dest_head(name):
    im, _ = canvas(200, 34)
    draw_text(im, 0, 2, "DESTINATION", 2, TEXT)
    draw_text(im, 0, 22, f"HOLDING: {name} BOX", 1, DIM)
    return im


def dest_body():
    im, d = canvas(320, 84)
    # touch zones on the page: (8,95,100,75) (113,95,94,75) (219,110,92,33); this picture starts at y 92
    for (x, y, w, h), icon, l1, l2 in (((8, 3, 100, 75), "pin", "DROP AT", "PLACE"),
                                       ((113, 3, 94, 75), "down", "DROP", "NOW")):
        pbox(d, x, y, x + w - 1, y + h - 1, PANEL, EDGE)
        draw_icon2(im, x + w // 2 - 8, y + 10, icon, 2)
        text_c(im, x + w // 2, y + 34, l1, 2, TEXT)
        text_c(im, x + w // 2, y + 52, l2, 2, TEXT)
    pbox(d, 219, 18, 310, 50, PANEL, AMBER_DK)
    draw_icon2(im, 227, 26, "slider", 2)
    draw_text(im, 248, 28, "CUSTOM", 1, TEXT)
    return im


def panel_a():
    im, d = canvas(176, 84)
    pbox(d, 0, 0, 175, 83, PANEL, EDGE)
    draw_text(im, 8, 8, "CUSTOM", 2, TEXT)
    draw_text(im, 8, 33, "STEP", 1, DIM)
    return im


def dist(n):
    im, _ = canvas(50, 16, PANEL)
    draw_text(im, 0, 1, f"{n}CM", 2, AMBER)
    return im


def slider():
    """lcd.c fills the chosen part white (3 px at y 9, from x 6) and puts the 12 x 12 knob on it"""
    im, d = canvas(160, 34, PANEL)
    d.rectangle((6, 9, 151, 11), fill=EDGE)
    for cm in (0, 5, 10, 15, 20):
        x = 6 + cm * 29 // 4
        d.rectangle((x, 14, x + 1, 16), fill=DIM)
        text_c(im, x + 1, 20, str(cm), 1, DIM)
    return im


def knob():
    im, d = canvas(12, 12, PANEL)
    d.rectangle((1, 1, 10, 10), fill=OR_DEEP)
    d.rectangle((2, 2, 9, 9), fill=AMBER)
    d.rectangle((2, 2, 9, 3), fill=OR_HI)
    d.rectangle((2, 8, 9, 9), fill=OR_MID)
    return im


def panel_say():
    im, d = canvas(176, 37)
    pbox(d, 0, 0, 175, 36, PANEL, EDGE)
    draw_icon2(im, 8, 10, "mic", 2)
    draw_text(im, 30, 8, "SAY A DIRECTION", 1, TEXT)
    draw_text(im, 30, 21, "FWD/BACK/LEFT/RIGHT", 1, DIM)
    return im


def panel_queue():
    im, d = canvas(114, 131)
    pbox(d, 0, 0, 113, 130, PANEL, EDGE)
    draw_text(im, 8, 8, "QUEUE", 2, TEXT)
    draw_text(im, 98, 11, "3", 1, AMBER)
    for i, s in enumerate(("FWD 2CM", "LEFT 5CM", "RIGHT 20CM")):
        y = 32 + i * 24
        pbox(d, 6, y, 107, y + 19, BG, EDGE, outside=PANEL)
        draw_text(im, 12, y + 6, s, 1, TEXT)
    return im


def overlay_frame(w, h):
    im, d = canvas(w, h)
    pbox(d, 0, 0, w - 1, h - 1, BG, AMBER, t=2, notch=4)
    d.rectangle((4, 2, w - 5, 3), fill=OR_HI)                  # light top edge
    d.rectangle((4, h - 4, w - 5, h - 3), fill=AMBER_DK)       # dark bottom edge
    return im, d


def ov_conf():
    im, d = overlay_frame(260, 160)
    text_c(im, 130, 30, "CONFIRM?", 3, TEXT)
    # No / Yes touch areas (local): y 103..137, No x 16..122, Yes x 136..245
    pbox(d, 16, 103, 121, 136, PANEL, EDGE)
    text_c(im, 69, 113, "NO", 2, TEXT)
    obtn(d, 136, 103, 244, 136)
    text_c(im, 190, 113, "YES", 2, DARK)
    return im


CONF_TEXTS = ["START SINGLE?", "START GROUP?", "START CARTESIAN?",
              "PICK RED BOX?", "PICK GREEN BOX?", "PICK BLACK BOX?", "PICK BLUE BOX?",
              "DROP AT PLACE?", "DROP NOW?", "RUN QUEUE?"]


def say(im, d, y, word):
    s = f'SAY "{word}"'
    w = text_w(s, 2) + 24
    x = (280 - w) // 2
    draw_icon2(im, x, y - 1, "mic", 2)
    draw_text(im, x + 24, y, s, 2, AMBER)


def ov_status(title, lines, word, dots=False):
    im, d = overlay_frame(280, 200)
    text_c(im, 140, 22, title, 3, TEXT)
    if dots:
        draw_icon(im, 132, 48, "dots", 2, AMBER)
    d.rectangle((16, 86, 263, 87), fill=EDGE)
    for i, s in enumerate(lines):
        text_c(im, 140, 98 + i * 14, s, 1, DIM)
    d.rectangle((16, 140, 263, 141), fill=EDGE)
    say(im, d, 156, word)
    return im


def ov_proc_frame():
    im, d = overlay_frame(280, 200)
    text_c(im, 140, 22, "WORKING", 3, TEXT)
    draw_icon(im, 132, 54, "dots", 2, AMBER)
    d.rectangle((16, 86, 263, 87), fill=EDGE)
    d.rectangle((16, 140, 263, 141), fill=EDGE)
    say(im, d, 156, "STOP")
    return im


def ov_stoph_frame():
    im, d = overlay_frame(280, 200)
    text_c(im, 140, 18, "STOPPED", 3, TEXT)
    d.rectangle((16, 86, 263, 87), fill=EDGE)
    text_c(im, 140, 100, "PLACE THE BLOCK", 1, DIM)
    text_c(im, 140, 114, "BEFORE GOING HOME", 1, DIM)
    d.rectangle((16, 140, 263, 141), fill=EDGE)
    say(im, d, 156, "PLACE")
    return im


def holding(w, h, name, sc_y):
    im, _ = canvas(w, h)
    s = f"HOLDING: {name}"
    x = (w - text_w(s, 2)) // 2
    draw_text(im, x, (h - 14) // 2, "HOLDING:", 2, DIM)
    draw_text(im, x + text_w("HOLDING: ", 2), (h - 14) // 2, name, 2, dict(BOXES)[name])
    return im


# ---------------------------------------------------------------- digits for lcd_port_digits (alpha)
def digit_font():
    out = []
    for ch in "0123456789- ":
        im = Image.new("L", (10, 14), 0)
        if ch != " ":
            draw_text(im, 0, 0, ch, 2, 255)
        out.append(list(im.getdata()))
    return out


# ---------------------------------------------------------------- build
def build():
    a = {}
    a["HOME"] = page_home()
    a["SELECT"] = page_select()
    a["HDR_SINGLE"] = header("servo", "SINGLE MODE")
    a["HDR_GROUP"] = header("group", "GROUP MODE")
    a["HDR_CART"] = header("axes", "CARTESIAN MODE")
    a["BTN_BACK"] = btn_back()
    a["BTN_PROCEED"] = btn_next()
    for lab in "XYZ":
        a[f"ROW_{lab}"] = row(lab)
    a["SEL_NARROW"] = selector(133)
    a["SEL_WIDE"] = selector(164)
    for n in range(6):
        a[f"SINGLE_LABEL_{n}"] = tile(76, 26, f"SERVO {n + 1}", 2, TEXT)
    for g, sub in enumerate(("BASE + SHOULDER", "ELBOW + WRIST", "ROLL + GRIPPER")):
        a[f"GROUP_LABEL_{g}"] = tile(116, 26, f"GROUP {g + 1}", 2, TEXT)
        a[f"GROUP_SUB_{g}"] = tile_left(172, 10, sub, 1, DIM)
    for k in range(6):
        a[f"SINGLE_ARM_{k}"] = arm_tile(168, 152, hl=(k,), tags=(k,))
    for g in range(3):
        a[f"GROUP_ARM_{g}"] = arm_tile(168, 154, hl=(2 * g, 2 * g + 1), tags=(2 * g, 2 * g + 1))
    a["CART_ARM"] = arm_tile(168, 154, axes=True)
    a["HDR_VOICE"] = header("mic", "VOICE MODE")
    a["PANEL_CAM"] = panel_cam()
    for i, (nm, col) in enumerate(BOXES):
        a[f"CARD_{i}"] = card(nm, col)
    a["CHECK"] = check()
    a["PANEL_PICK"] = panel_pick()
    for i, (nm, col) in enumerate(BOXES):
        a[f"PICK_TXT_{i}"] = tile_left(255, 16, f'"PICK THE {nm} BOX"', 2, TEXT, fill=PANEL, x=0)
    a["PICK_TXT_4"] = tile_left(255, 16, "SAY: PICK A COLOR", 2, DIM, fill=PANEL, x=0)
    for i, (nm, _) in enumerate(BOXES):
        a[f"DEST_HEAD_{i}"] = dest_head(nm)
    a["DEST_BODY"] = dest_body()
    a["PANEL_A"] = panel_a()
    for n in range(1, 21):
        a[f"DIST_{n}"] = dist(n)
    a["SLIDER"] = slider()
    a["KNOB"] = knob()
    a["PANEL_SAY"] = panel_say()
    a["PANEL_QUEUE"] = panel_queue()
    a["OV_CONF_FRAME"] = ov_conf()
    for i, s in enumerate(CONF_TEXTS):
        a[f"OV_CONF_SUB_{i}"] = tile(220, 14, s, 2, AMBER, fill=BG)
    a["OV_PROC_FRAME"] = ov_proc_frame()
    for i, (nm, _) in enumerate(BOXES):
        a[f"OV_PROC_LINE_{i}"] = holding(200, 16, nm, 0)
    a["OV_STOPH_FRAME"] = ov_stoph_frame()
    for i, (nm, _) in enumerate(BOXES):
        a[f"OV_STOPH_LINE_{i}"] = holding(200, 18, nm, 0)
    a["OV_STOPE"] = ov_status("STOPPED", ["GRIPPER: EMPTY", "READY TO GO HOME"], "HOME")
    a["OV_COMPLETE"] = ov_status("DONE!", ["GRIPPER: EMPTY", "READY TO GO HOME"], "HOME")
    return a


def rgb565_be(px):
    r, g, b = px
    v = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3)
    return bytes((v >> 8, v & 0xFF))


def rle(im):
    pix = [rgb565_be(p) for p in im.convert("RGB").getdata()]
    out, i, lit = bytearray(), 0, []

    def flush():
        while lit:
            chunk = lit[:128]
            del lit[:128]
            out.append(127 + len(chunk))
            for p in chunk:
                out.extend(p)
    while i < len(pix):
        j = i
        while j < len(pix) and pix[j] == pix[i] and j - i < 130:
            j += 1
        if j - i >= 3:
            flush()
            out.append(j - i - 3)
            out.extend(pix[i])
            i = j
        else:
            lit.append(pix[i])
            i += 1
    flush()
    return bytes(out)


def c_array(name, data):
    lines = [", ".join(f"0x{b:02X}" for b in data[k:k + 20]) + "," for k in range(0, len(data), 20)]
    return f"const uint8_t {name}[{len(data)}] = {{\n" + "\n".join("  " + l for l in lines) + "\n};\n"


def main():
    lcd_dir = Path(sys.argv[1])
    preview = Path(sys.argv[2]) if len(sys.argv) > 2 else None
    hdr = (lcd_dir / "lcd_assets.h").read_text()
    ids = re.findall(r"^\s+LCD_A_(\w+),", hdr.split("typedef enum")[1].split("}")[0], re.M)
    old = (lcd_dir / "lcd_assets.c").read_text()
    sizes = re.findall(r"\{\s*(\d+),\s*(\d+),\s*lcd_d_\w+\s*\}", old.split("lcd_assets[LCD_A_COUNT]")[1])
    assert len(ids) == len(sizes), "lcd_assets.h and lcd_assets.c disagree"

    pics = build()
    missing = [i for i in ids if i not in pics]
    assert not missing, f"no picture for {missing}"
    big = {i for i in ids if i.startswith(("SINGLE_ARM", "GROUP_ARM", "CART_ARM"))}

    main_c = ["/* generated by ProjectDevGuide/tools/build_lcd_pixel_assets.py - do not edit */\n",
              '#include "lcd/lcd_assets.h"\n\n']
    arm_c = ["/* generated by ProjectDevGuide/tools/build_lcd_pixel_assets.py - do not edit */\n",
             '#include "lcd/lcd_assets.h"\n\n']
    for i in ids:
        if i in big:
            main_c.append(f"extern const uint8_t lcd_d_{i.lower()}[{len(rle(pics[i]))}];\n")
    main_c.append("\n")
    table = []
    for i, (w, h) in zip(ids, sizes):
        im = pics[i]
        assert im.size == (int(w), int(h)), f"{i}: {im.size} != {w}x{h}"
        data = rle(im)
        (arm_c if i in big else main_c).append(c_array(f"lcd_d_{i.lower()}", data) + "\n")
        table.append(f"  {{ {w}, {h}, lcd_d_{i.lower()} }},")
        if preview:
            im.save(preview / f"LCD_A_{i}.png")
    main_c.append("const lcd_asset_t lcd_assets[LCD_A_COUNT] = {\n" + "\n".join(table) + "\n};\n\n")
    glyphs = digit_font()
    rows = [("  { " + ", ".join(str(v) for v in g) + " },") for g in glyphs]
    main_c.append("const uint8_t lcd_digit_font[LCD_DIGIT_COUNT][LCD_DIGIT_W * LCD_DIGIT_H] = {\n"
                  + "\n".join(rows) + "\n};\n")
    (lcd_dir / "lcd_assets.c").write_text("".join(main_c))
    (lcd_dir / "lcd_assets_arm.c").write_text("".join(arm_c))
    total = sum(len(rle(p)) for k, p in pics.items() if k in ids)
    print(f"{len(ids)} pictures, {total // 1024} KB packed")


if __name__ == "__main__":
    main()
