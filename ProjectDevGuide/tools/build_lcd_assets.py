#!/usr/bin/env python3
"""build_lcd_assets.py - makes ALL pictures of the touch-screen UI and packs them for the ILI9341.

    python3 ProjectDevGuide/tools/build_lcd_assets.py <white_arm.png> <home.png> <select_mode.png> \
            RoboticArmMain/App/lcd [check_dir]
    (needs: pip install pillow; fonts: DejaVu Sans Mono)

what it does
  * draws the page COMPONENTS once (header + title, back / proceed buttons, X/Y/Z rows, servo/group
    selector, label tiles, arm pictures with joint highlights / axis arrows) in the white theme
  * RLE-compresses every picture (RGB565, ~5x smaller than raw) and writes
        lcd_assets.h        asset ids (LCD_A_...), positions (LCD_POS_...), digit font
        lcd_assets.c        the compressed data + asset table
        lcd_assets_arm.c    the arm pictures (the biggest ones)
  * with check_dir: also writes every asset as raw RGB565 (for the host decoder test) and a PNG preview
    of each page composed exactly the way lcd.c draws it

a NEW PAGE costs only its own pictures: reuse the header builder, buttons and rows, add a title and
(if needed) an arm picture here, then draw them in lcd.c with lcd_port_asset().

RLE format (one byte header, then pixels as big-endian RGB565 = the order the panel wants):
    0..127   : run of (header + 3) identical pixels, 2 bytes follow
    128..255 : literal block of (header - 127) pixels, 2 bytes each follow
"""
import math
import sys
from pathlib import Path
from PIL import Image, ImageDraw, ImageFont

S = 4
BG = (8, 11, 16)
ROW_BG = (13, 17, 23)
LINE = (41, 46, 50)
ROW_BORDER = (52, 58, 61)
WHITE = (245, 247, 251)
SEL_FILL = (24, 27, 32)
SEL_BORDER = (232, 235, 242)
BACK_FILL = (17, 24, 38)
BACK_BORDER = (58, 66, 84)
LABEL_GREY = (125, 132, 142)
UNIT_GREY = (100, 108, 118)
TEXT = (218, 219, 224)
FB = "/usr/share/fonts/truetype/dejavu/DejaVuSansMono-Bold.ttf"
FR = "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf"
SINGLE_ARM_POS = (150, 46)
SINGLE_ARM_W = 143
SINGLE_TILE_ARM = (150, 44, 168, 152)
SINGLE_TILE_LABEL = (42, 58, 76, 26)
SINGLE_JOINTS = [
    (240.5, 156.0, "ring", 26, 281, 148),
    (263.0, 139.5, "arcs", 17, 284, 132),
    (274.0,  92.5, "arcs", 15, 287,  87),
    (249.0,  59.0, "arcs", 14, 285,  53),
    (216.0,  91.0, "arcs", 11, 154,  62),
    (196.0, 112.0, "arcs", 11, 154,  84),
]
DIGIT_W, DIGIT_H = 10, 14
G_LABEL = (36, 49, 116, 26)
G_SUB = (14, 77, 172, 10)
G_PLACE = (157, 46, 139)
C_PLACE = (155, 43, 144)
GROUPS = [("Group 1 (S1+S2)", "Control base and shoulder together"),
          ("Group 2 (S3+S4)", "Control elbow and wrist together"),
          ("Group 3 (S5+S6)", "Control wrist roll and gripper together")]
SUB_GREY = (84, 91, 101)
JOINT = {1: (112, 135), 2: (132, 118), 3: (145, 60), 4: (108, 21), 5: (60, 58), 6: (33, 80)}
GROUP_HI = [
    (((1, "ring", 24), (2, "arcs", 15)), 283, 143),
    (((3, "arcs", 14), (4, "arcs", 12)), 285, 66),
    (((5, "arcs", 10), (6, "arcs", 10)), 168, 150),
]
MODE_TILE_ARM = (152, 42, 168, 154)
def font(path, px):
    return ImageFont.truetype(path, int(round(px * S)))


def rrect(d, box, r, fill=None, outline=None, w=1):
    x0, y0, x1, y1 = box
    d.rounded_rectangle([x0 * S, y0 * S, (x1 + 1) * S - 1, (y1 + 1) * S - 1], radius=r * S,
                        fill=fill, outline=outline, width=int(w * S))


def line(d, pts, fill, w):
    d.line([(x * S, y * S) for x, y in pts], fill=fill, width=int(w * S), joint="curve")


def text(d, xy, s, f, fill, anchor="lm"):
    d.text((xy[0] * S, xy[1] * S), s, font=f, fill=fill, anchor=anchor)


def down(im):
    return im.resize((im.width // S, im.height // S), Image.LANCZOS)


def highlight(k):
    """RGBA overlay (full page size) with the rings / arcs, leader line and tag of joint k"""
    cx, cy, kind, r, tx, ty = SINGLE_JOINTS[k]
    ov = Image.new("RGBA", (320 * S, 240 * S), (0, 0, 0, 0))
    d = ImageDraw.Draw(ov)
    shadow, hi, lead = (8, 11, 16, 210), (240, 244, 250, 255), (170, 182, 200, 255)

    def arcs(color, w):
        if kind == "ring":
            box = [(cx - r) * S, (cy - 7) * S, (cx + r) * S, (cy + 7) * S]
            d.arc(box, 150, 360, fill=color, width=int(w * S))
            d.arc(box, 0, 30, fill=color, width=int(w * S))
        else:
            box = [(cx - r) * S, (cy - r) * S, (cx + r) * S, (cy + r) * S]
            d.arc(box, 150, 210, fill=color, width=int(w * S))
            d.arc(box, -30, 30, fill=color, width=int(w * S))

    # leader line to the tag
    tw, th = 30, 11
    ex = tx if tx > cx else tx + tw
    ey = ty + th / 2
    d.line([(cx * S, cy * S), (ex * S, ey * S)], fill=shadow, width=int(2.4 * S))
    d.line([(cx * S, cy * S), (ex * S, ey * S)], fill=lead, width=int(1.0 * S))
    arcs(shadow, 3.0)
    arcs(hi, 1.6)
    d.ellipse([(cx - 2.6) * S, (cy - 2.6) * S, (cx + 2.6) * S, (cy + 2.6) * S], fill=shadow)
    d.ellipse([(cx - 1.6) * S, (cy - 1.6) * S, (cx + 1.6) * S, (cy + 1.6) * S], fill=hi)
    # tag
    d.rounded_rectangle([tx * S, ty * S, (tx + tw) * S - 1, (ty + th) * S - 1], radius=3 * S,
                        fill=ROW_BG + (255,), outline=(200, 207, 216, 255), width=int(1 * S))
    d.text(((tx + tw / 2) * S, (ty + th / 2) * S), f"Joint {k + 1}", font=font(FB, 6.0), fill=(250, 250, 252, 255), anchor="mm")
    return ov.resize((320, 240), Image.LANCZOS)


def digit_font():
    """glyphs '0'..'9' (0..9), '-' (10), ' ' (11), alpha 0..255, DIGIT_W x DIGIT_H"""
    f = font(FB, 17)
    out = []
    for ch in "0123456789- ":
        im = Image.new("L", (DIGIT_W * S, DIGIT_H * S), 0)
        ImageDraw.Draw(im).text((DIGIT_W * S / 2, 12.6 * S), ch, font=f, fill=255, anchor="ms")
        out.append(list(down(im).getdata()))
    return out


def rgb565_swapped(im):
    out = []
    for r, g, b in im.convert("RGB").getdata():
        v = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)
        out.append(((v & 0xFF) << 8) | (v >> 8))
    return out


def layers_icon(d):
    top = [(29, 10.5), (39.5, 15.5), (29, 20.5), (18.5, 15.5)]
    line(d, top + [top[0]], WHITE, 1.6)
    line(d, [(18.5, 20.2), (29, 25.2), (39.5, 20.2)], WHITE, 1.6)
    line(d, [(18.5, 25), (29, 30), (39.5, 25)], WHITE, 1.6)


def text_tile(w, h, bg, s, size, color, cx, cy_off=0.0, left=None, path=FR):
    im = Image.new("RGB", (w * S, h * S), bg)
    d = ImageDraw.Draw(im)
    if left is None:
        text(d, (cx, h / 2 + cy_off), s, font(path, size), color, "mm")
    else:
        text(d, (left, h / 2 + cy_off), s, font(path, size), color, "lm")
    return down(im)


def joint_xy(j, place):
    ox, oy, w = place
    k = w / 171.0
    return ox + JOINT[j][0] * k, oy + JOINT[j][1] * k


def hl_group(g, place):
    pair, tx, ty = GROUP_HI[g]
    ov = Image.new("RGBA", (320 * S, 240 * S), (0, 0, 0, 0))
    d = ImageDraw.Draw(ov)
    shadow, hi, lead = (8, 11, 16, 210), (240, 244, 250, 255), (170, 182, 200, 255)
    tw, th = 34, 11
    ex = tx + tw / 2
    for (j, kind, r) in pair:
        cx, cy = joint_xy(j, place)
        r *= place[2] / 143.0
        tgt = (tx if tx > cx else tx + tw, ty + th / 2)
        if tx < 200:                                    # tag below-left: aim at its top edge
            tgt = (ex, ty)
        d.line([(cx * S, cy * S), (tgt[0] * S, tgt[1] * S)], fill=shadow, width=int(2.4 * S))
        d.line([(cx * S, cy * S), (tgt[0] * S, tgt[1] * S)], fill=lead, width=int(1.0 * S))
    for (j, kind, r) in pair:
        cx, cy = joint_xy(j, place)
        r *= place[2] / 143.0
        for color, w in ((shadow, 3.0), (hi, 1.6)):
            if kind == "ring":
                box = [(cx - r) * S, (cy - 6.5) * S, (cx + r) * S, (cy + 6.5) * S]
                d.arc(box, 150, 360, fill=color, width=int(w * S))
                d.arc(box, 0, 30, fill=color, width=int(w * S))
            else:
                box = [(cx - r) * S, (cy - r) * S, (cx + r) * S, (cy + r) * S]
                d.arc(box, 150, 210, fill=color, width=int(w * S))
                d.arc(box, -30, 30, fill=color, width=int(w * S))
        d.ellipse([(cx - 2.6) * S, (cy - 2.6) * S, (cx + 2.6) * S, (cy + 2.6) * S], fill=shadow)
        d.ellipse([(cx - 1.6) * S, (cy - 1.6) * S, (cx + 1.6) * S, (cy + 1.6) * S], fill=hi)
    d.rounded_rectangle([tx * S, ty * S, (tx + tw) * S - 1, (ty + th) * S - 1], radius=3 * S,
                        fill=ROW_BG + (255,), outline=(200, 207, 216, 255), width=int(1 * S))
    d.text(((tx + tw / 2) * S, (ty + th / 2) * S), f"Group {g + 1}", font=font(FB, 6.0),
           fill=(250, 250, 252, 255), anchor="mm")
    return ov.resize((320, 240), Image.LANCZOS)


def hl_axes():
    """x / y / z arrows at the tool tip (positions measured on the design)"""
    ov = Image.new("RGBA", (320 * S, 240 * S), (0, 0, 0, 0))
    d = ImageDraw.Draw(ov)
    o = (187.3, 137.3)
    axes = [("Z", (187.3, 106.5), (60, 190, 255, 255)),
            ("Y", (211.0, 123.5), (70, 232, 100, 255)),
            ("X", (176.5, 150.0), (255, 75, 75, 255))]
    for name, tip, col in axes:
        for c, w in (((8, 11, 16, 220), 3.2), (col, 1.5)):
            d.line([(o[0] * S, o[1] * S), (tip[0] * S, tip[1] * S)], fill=c, width=int(w * S))
        ang = math.atan2(tip[1] - o[1], tip[0] - o[0])
        hd = [(tip[0] + 3.4 * math.cos(ang), tip[1] + 3.4 * math.sin(ang)),
              (tip[0] + 1.6 * math.cos(ang + 2.5), tip[1] + 1.6 * math.sin(ang + 2.5)),
              (tip[0] + 1.6 * math.cos(ang - 2.5), tip[1] + 1.6 * math.sin(ang - 2.5))]
        # arrow head
        b = (tip[0] - 3.2 * math.cos(ang), tip[1] - 3.2 * math.sin(ang))
        p1 = (b[0] + 2.2 * math.cos(ang + math.pi / 2), b[1] + 2.2 * math.sin(ang + math.pi / 2))
        p2 = (b[0] + 2.2 * math.cos(ang - math.pi / 2), b[1] + 2.2 * math.sin(ang - math.pi / 2))
        tipx = (tip[0] + 1.2 * math.cos(ang), tip[1] + 1.2 * math.sin(ang))
        poly = [(tipx[0] * S, tipx[1] * S), (p1[0] * S, p1[1] * S), (p2[0] * S, p2[1] * S)]
        d.polygon(poly, fill=col, outline=(8, 11, 16, 220))
        lx, ly = tip[0] + 4.6 * math.cos(ang), tip[1] + 4.6 * math.sin(ang)
        d.text((lx * S, ly * S), name, font=font(FB, 6.5), fill=col, anchor="mm", stroke_width=int(0.8 * S), stroke_fill=(8, 11, 16, 255))
    d.ellipse([(o[0] - 1.5) * S, (o[1] - 1.5) * S, (o[0] + 1.5) * S, (o[1] + 1.5) * S], fill=(240, 244, 250, 255))
    return ov.resize((320, 240), Image.LANCZOS)


# ============================================================================ layout
# single source of truth: written to lcd_assets.h as LCD_POS_*, used by lcd.c and by the previews below
POS = {
    "BTN_BACK": (14, 201), "BTN_PROCEED": (224, 201),
    "SINGLE_SEL": (14, 57), "SINGLE_LABEL": (42, 58), "SINGLE_ARM": (150, 44),
    "GROUP_SEL": (14, 48), "GROUP_LABEL": (36, 49), "GROUP_SUB": (14, 77), "GROUP_ARM": (152, 42),
    "CART_ARM": (152, 42),
}
ROW_X_SINGLE, ROW_Y_SINGLE = 14, (91, 125, 159)
ROW_X_GROUP, ROW_Y_GROUP = 14, (91, 123, 159)
ROW_X_CART, ROW_Y_CART = 17, (73, 110, 147)
ROW_W, ROW_H = 133, 27
BTN_W, BTN_H = 78, 28


# ============================================================================ components
def canvas(w, h, fill=BG):
    im = Image.new("RGB", (w * S, h * S), fill)
    return im, ImageDraw.Draw(im)


def icon_cube(d):
    cx, cy, r = 24.0, 21.5, 10.5
    v = [(cx + r * math.cos(math.radians(a)), cy + r * math.sin(math.radians(a))) for a in (-90, -30, 30, 90, 150, 210)]
    line(d, v + [v[0]], WHITE, 1.6)
    for k in (3, 5, 1):
        line(d, [(cx, cy), v[k]], WHITE, 1.6)


def comp_header(icon, title, title_x):
    """320 x 40: icon + title + the rule under the header"""
    im, d = canvas(320, 40)
    icon(d)
    text(d, (title_x, 21.5), title, font(FB, 16), WHITE)
    d.rectangle([0, 39 * S, 320 * S, 40 * S - 1], fill=LINE)
    return down(im)


def comp_selector(w):
    """servo / group selector frame with both chevrons, w x 28 (the label is a separate tile)"""
    im, d = canvas(w, 28)
    rrect(d, (0, 0, w - 1, 27), 6, SEL_FILL, SEL_BORDER, 1)
    line(d, [(15.5, 8.5), (11, 13.5), (15.5, 18.5)], WHITE, 1.8)
    line(d, [(w - 16.5, 8.5), (w - 12, 13.5), (w - 16.5, 18.5)], WHITE, 1.8)
    return down(im)


def comp_row(lab):
    """one X / Y / Z row (label + unit, the number is drawn at run time)"""
    im, d = canvas(ROW_W, ROW_H)
    rrect(d, (0, 0, ROW_W - 1, ROW_H - 1), 6, ROW_BG, ROW_BORDER, 1)
    text(d, (13, 13.5), lab, font(FR, 13), LABEL_GREY)
    text(d, (103, 14.5), "mm", font(FR, 8), UNIT_GREY)
    return down(im)


def comp_back():
    im, d = canvas(BTN_W, BTN_H)
    rrect(d, (0, 0, BTN_W - 1, BTN_H - 1), 7, BACK_FILL, BACK_BORDER, 1)
    line(d, [(17, 13.5), (10.5, 13.5)], WHITE, 1.4)
    line(d, [(14, 10.2), (10.5, 13.5), (14, 16.8)], WHITE, 1.4)
    text(d, (27, 13.5), "Back", font(FB, 11), WHITE)
    return down(im)


def comp_proceed():
    im, d = canvas(BTN_W, BTN_H)
    rrect(d, (0, 0, BTN_W - 1, BTN_H - 1), 7, WHITE, None, 0)
    text(d, (11, 13.5), "Proceed", font(FB, 11), BG)
    line(d, [(63, 13.5), (70, 13.5)], BG, 1.4)
    line(d, [(66.5, 10.2), (70, 13.5), (66.5, 16.8)], BG, 1.4)
    return down(im)


def arm_tile(arm, place, overlay, box):
    """arm picture + highlight on the page background, cropped to box (x, y, w, h)"""
    ox, oy, w = place
    a = arm.resize((w, round(arm.height * w / arm.width)), Image.LANCZOS)
    page = Image.new("RGBA", (320, 240), BG + (255,))
    page.alpha_composite(a, (ox, oy))
    page.alpha_composite(overlay)
    x, y, bw, bh = box
    return page.convert("RGB").crop((x, y, x + bw, y + bh))


# ============================================================================ voice mode: colors + icons
PANEL_A = (9, 12, 18)
CHIP_FILL = (22, 26, 34)
CHIP_BORDER = (89, 97, 110)
TRACK = (89, 97, 110)
GREY = (156, 163, 175)
DIM = (75, 81, 91)
SUBDIM = (99, 106, 116)
SUBDIM2 = (99, 107, 117)
OV_FILL = (14, 17, 20)
OV_SUB = (116, 125, 136)
OV_GREY = (140, 150, 163)
OV_LINE = (36, 40, 46)
TEXT2 = (235, 238, 244)
BOXES = [("Red", (222, 32, 32)), ("Green", (64, 200, 72)), ("Black", (46, 46, 52)), ("Blue", (40, 112, 255))]
CONF_TEXTS = ["Start Single mode?", "Start Group mode?", "Start Cartesian mode?",
              "Pick up the red box?", "Pick up the green box?", "Pick up the black box?", "Pick up the blue box?",
              "Drop in destination?", "Drop now?", "Run the custom queue?"]
POS.update({
    "PANEL_CAM": (14, 49), "CARD_0": (23, 73), "CARD_1": (162, 73), "CARD_2": (23, 112), "CARD_3": (162, 112),
    "PANEL_PICK": (14, 157), "PICK_TXT": (42, 177),
    "DEST_HEAD": (22, 50), "DEST_BODY": (0, 92),
    "PANEL_A": (12, 51), "DIST_VAL": (82, 81), "SLIDER": (18, 95), "PANEL_SAY": (12, 145), "PANEL_QUEUE": (196, 51),
    "OV_CONF": (30, 40), "OV_CONF_SUB": (50, 112), "OV_STATUS": (20, 20),
    "OV_PROC_LINE": (60, 118), "OV_STOPH_LINE": (60, 66),
})
CHECK_DX, CHECK_DY = 108, 10                        # check mark inside a card
CARD_W, CARD_H = 131, 35
OV_CONF_W, OV_CONF_H = 260, 160
OV_ST_W, OV_ST_H = 280, 200


def mic(d, cx, top, s, color, w=1.5):
    lw = max(1.0, w * s)
    cw, ch = 4.3 * s, 13 * s
    d.rounded_rectangle([(cx - cw) * S, top * S, (cx + cw) * S, (top + ch) * S], radius=cw * S, outline=color, width=int(lw * S))
    r = 8 * s
    cy = top + 9 * s
    d.arc([(cx - r) * S, (cy - r) * S, (cx + r) * S, (cy + r) * S], 10, 170, fill=color, width=int(lw * S))
    line(d, [(cx, cy + r), (cx, top + 20.5 * s)], color, lw)
    line(d, [(cx - 3.5 * s, top + 20.5 * s), (cx + 3.5 * s, top + 20.5 * s)], color, lw)


def icon_mic_header(d):
    mic(d, 24, 9.5, 1.05, WHITE, 1.6)


def icon_camera(d, x, y):
    c = (150, 158, 170)
    rrect(d, (x, y + 3, x + 13, y + 12), 2, None, c, 1.2)
    line(d, [(x + 4, y + 3), (x + 5, y + 0.8), (x + 8, y + 0.8), (x + 9, y + 3)], c, 1.2)
    d.ellipse([(x + 6.5 - 2.7) * S, (y + 7.5 - 2.7) * S, (x + 6.5 + 2.7) * S, (y + 7.5 + 2.7) * S], outline=c, width=int(1.2 * S))


def icon_sliders(d, cx, cy):
    c = GREY
    for dy, kx in ((-5, -3), (0, 3), (5, -1)):
        line(d, [(cx - 7, cy + dy), (cx + 7, cy + dy)], c, 1.3)
        d.ellipse([(cx + kx - 2) * S, (cy + dy - 2) * S, (cx + kx + 2) * S, (cy + dy + 2) * S], fill=c)


def shade(c, k):
    return tuple(max(0, min(255, int(v * k))) for v in c)


def cube(d, cx, cy, r, base):
    def v(a):
        return (cx + r * math.cos(math.radians(a)), cy + r * math.sin(math.radians(a)))
    c = (cx, cy)
    lighter = tuple(min(255, int(x + (255 - x) * 0.38)) for x in base)
    faces = [([v(-90), v(-30), c, v(210)], lighter), ([v(210), c, v(90), v(150)], base), ([c, v(-30), v(30), v(90)], shade(base, 0.62))]
    for pts, col in faces:
        d.polygon([(x * S, y * S) for x, y in pts], fill=col)
    edge = tuple(min(255, int(x + (255 - x) * 0.55)) for x in base)
    hexa = [v(a) for a in (-90, -30, 30, 90, 150, 210)]
    line(d, hexa + [hexa[0]], edge, 0.8)
    line(d, [c, v(-30)], edge, 0.6)
    line(d, [c, v(210)], edge, 0.6)
    line(d, [c, v(90)], edge, 0.6)


# ============================================================================ voice mode: page components
def comp_panel_cam():
    im, d = canvas(289, 103)
    rrect(d, (0, 0, 288, 102), 7, ROW_BG, ROW_BORDER, 1)
    icon_camera(d, 9, 6)
    text(d, (31, 12), "Camera Detection", font(FR, 11), TEXT2)
    return down(im)


def comp_card(label, color):
    im, d = canvas(CARD_W, CARD_H)
    rrect(d, (0, 0, CARD_W - 2, CARD_H - 2), 6, ROW_BG, ROW_BORDER, 1)
    cube(d, 17, 18, 12, color)
    text(d, (33, 17.5), label, font(FR, 11), TEXT2)
    return down(im)


def comp_check():
    im, d = canvas(15, 15, ROW_BG)
    d.ellipse([1 * S, 1 * S, 14 * S, 14 * S], outline=WHITE, width=int(1.2 * S))
    line(d, [(4.4, 7.8), (6.6, 10.1), (10.8, 5.1)], WHITE, 1.3)
    return down(im)


def comp_panel_pick():
    im, d = canvas(289, 40)
    rrect(d, (0, 0, 288, 39), 7, ROW_BG, ROW_BORDER, 1)
    text(d, (11, 12.5), "Pick Color", font(FR, 11), TEXT2)
    mic(d, 15.5, 22, 0.6, TEXT2, 1.4)
    return down(im)


def comp_dest_head(box):
    im, d = canvas(200, 34)
    text(d, (0, 11), "Destination", font(FB, 14), WHITE)
    text(d, (0, 28), f"Holding {box.lower()} box", font(FR, 8), SUBDIM)
    return down(im)


def comp_dest_body():
    im, d = canvas(320, 84)
    for cx in (54, 160):
        mic(d, cx, 6.2, 0.65, GREY, 1.6)
    ctr = font(FB, 11)
    text(d, (57, 31), "drop in", ctr, WHITE, "mm")
    text(d, (57, 47), "destination", ctr, WHITE, "mm")
    text(d, (55, 63), "default/station", font(FR, 7), SUBDIM2, "mm")
    text(d, (55, 76), "/hand", font(FR, 7), SUBDIM2, "mm")
    text(d, (160, 32), "drop now", ctr, WHITE, "mm")
    text(d, (160, 48), "place/drop", font(FR, 7), SUBDIM2, "mm")
    line(d, [(111, 5), (111, 75)], LINE, 1.2)
    line(d, [(208, 2), (208, 73)], LINE, 1.2)
    rrect(d, (219, 18, 310, 50), 7, (7, 9, 15), SEL_BORDER, 1.3)
    icon_sliders(d, 235.5, 34)
    text(d, (255, 34), "custom", font(FR, 12), WHITE)
    return down(im)


def comp_panel_a():
    im, d = canvas(176, 84)
    rrect(d, (0, 0, 175, 83), 7, PANEL_A, SEL_BORDER, 1.2)
    text(d, (10, 19), "Custom", font(FB, 15), WHITE)
    text(d, (10, 38), "Distance", font(FR, 11.8), GREY)
    return down(im)


def comp_dist(n):
    return text_tile(50, 16, PANEL_A, f"{n} CM", 13, (240, 242, 247), 0, left=1)


def comp_slider():
    im, d = canvas(160, 34, PANEL_A)
    d.rounded_rectangle([6 * S, 9 * S, 151 * S, 12 * S - 1], radius=1.5 * S, fill=TRACK)
    for i in range(21):
        x = 6 + 7.25 * i
        if i % 5 == 0:
            line(d, [(x, 14.5), (x, 17.5)], (110, 116, 126), 1.0)
            text(d, (x, 22.5), str(i), font(FR, 6), (124, 127, 132), "mm")
        else:
            line(d, [(x, 14.5), (x, 16.5)], (62, 68, 78), 0.9)
    text(d, (2, 31), "1 cm per step", font(FR, 6.2), (170, 174, 182))
    return down(im)


def comp_knob():
    im, d = canvas(12, 12, PANEL_A)
    d.ellipse([0.6 * S, 0.6 * S, 11.4 * S, 11.4 * S], fill=WHITE)
    return down(im)


def comp_panel_say():
    im, d = canvas(176, 37)
    rrect(d, (0, 0, 175, 36), 7, (10, 13, 19), SEL_BORDER, 1.2)
    mic(d, 13, 8, 0.6, GREY, 1.4)
    text(d, (25, 12.5), "Say a direction", font(FR, 10), GREY)
    text(d, (25, 23), "forward/backward/left/right", font(FR, 6.5), DIM)
    return down(im)


def comp_panel_queue():
    im, d = canvas(114, 131)
    rrect(d, (0, 0, 113, 130), 7, PANEL_A, SEL_BORDER, 1.2)
    text(d, (8, 14), "Queue", font(FR, 12), WHITE)
    text(d, (105, 12), "3", font(FR, 10), GREY, "rm")
    for y, s in ((27, "forward 2cm"), (52, "left 5 cm"), (77, "right 20 cm")):
        rrect(d, (6, y, 107, y + 20), 6, CHIP_FILL, CHIP_BORDER, 1)
        text(d, (12, y + 10.5), s, font(FR, 9), (218, 220, 224))
    return down(im)


# ============================================================================ overlays
def ov_canvas(w, h):
    im, d = canvas(w, h, BG)
    rrect(d, (0, 0, w - 1, h - 1), 12, OV_FILL, WHITE, 1.5)
    return im, d


def comp_conf_frame():
    im, d = ov_canvas(OV_CONF_W, OV_CONF_H)
    text(d, (130, 60), "CONFIRM ACTION ?", font(FB, 14), WHITE, "mm")
    rrect(d, (16, 103, 121, 136), 7, BACK_FILL, BACK_BORDER, 1)
    text(d, (68.5, 120), "No", font(FB, 13), WHITE, "mm")
    rrect(d, (136, 103, 244, 136), 7, WHITE, None, 0)
    text(d, (190, 120), "Yes", font(FB, 13), BG, "mm")
    return down(im)


def comp_conf_sub(s):
    return text_tile(220, 14, OV_FILL, s, 9, OV_SUB, 110)


def holding_tile(w, h, box, size):
    im, d = canvas(w, h, OV_FILL)
    parts = [("Holding", font(FB, size), OV_GREY), (" : ", font(FR, size), (90, 98, 110)), (f"{box} box", font(FR, size), OV_SUB)]
    total = sum(d.textlength(t, font=f) for t, f, _ in parts)
    x = (w * S - total) / 2
    for t, f, c in parts:
        d.text((x, h / 2 * S), t, font=f, fill=c, anchor="lm")
        x += d.textlength(t, font=f)
    return down(im)


def gripper_line(d, y, size):
    parts = [("Gripper", font(FB, size), OV_GREY), (" : ", font(FR, size), (90, 98, 110)), ("Empty", font(FR, size), OV_SUB)]
    total = sum(d.textlength(t, font=f) for t, f, _ in parts)
    x = (OV_ST_W * S - total) / 2
    for t, f, c in parts:
        d.text((x, y * S), t, font=f, fill=c, anchor="lm")
        x += d.textlength(t, font=f)


def say_line(d, mic_cx, text_x, s):
    mic(d, mic_cx, 147, 1.0, WHITE, 1.6)
    text(d, (text_x, 157), s, font(FB, 12), OV_GREY)


def comp_proc_frame():
    im, d = ov_canvas(OV_ST_W, OV_ST_H)
    text(d, (140, 33), "Processing", font(FB, 14), WHITE, "mm")
    for cx in (126, 139.5, 153):
        d.ellipse([(cx - 3.8) * S, (56 - 3.8) * S, (cx + 3.8) * S, (56 + 3.8) * S], fill=WHITE)
    line(d, [(13, 80), (267, 80)], OV_LINE, 1.0)
    line(d, [(15, 130), (271, 130)], OV_LINE, 1.0)
    say_line(d, 80, 98, "Say \u201cStop\u201d")
    return down(im)


def comp_stoph_frame():
    im, d = ov_canvas(OV_ST_W, OV_ST_H)
    text(d, (140, 33), "Stopped", font(FB, 14), WHITE, "mm")
    line(d, [(13, 74), (267, 74)], OV_LINE, 1.0)
    text(d, (140, 96), "Place the block before", font(FB, 10), OV_GREY, "mm")
    text(d, (140, 112), "returning home.", font(FB, 10), OV_GREY, "mm")
    line(d, [(14, 135), (269, 135)], OV_LINE, 1.0)
    say_line(d, 61, 82, "Say \u201cPlace/drop\u201d")
    return down(im)


def comp_empty_frame(title):
    im, d = ov_canvas(OV_ST_W, OV_ST_H)
    text(d, (140, 33), title, font(FB, 14), WHITE, "mm")
    gripper_line(d, 54, 13)
    line(d, [(13, 74), (267, 74)], OV_LINE, 1.0)
    text(d, (140, 105), "Ready to return home", font(FB, 10), OV_GREY, "mm")
    line(d, [(14, 135), (269, 135)], OV_LINE, 1.0)
    say_line(d, 88, 108, "Say \u201cHome\u201d")
    return down(im)


# ============================================================================ packing
def rgb565_words(im):
    out = []
    for r, g, b in im.convert("RGB").getdata():
        out.append(((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3))
    return out


def rle_encode(px):
    out = bytearray()
    i, n = 0, len(px)
    while i < n:
        j = i
        while j + 1 < n and px[j + 1] == px[i] and j - i + 1 < 130:
            j += 1
        run = j - i + 1
        if run >= 3:
            out.append(run - 3)
            out += bytes((px[i] >> 8, px[i] & 255))
            i = j + 1
        else:
            k = i
            while k < n and k - i < 128:
                if k + 2 < n and px[k] == px[k + 1] == px[k + 2]:
                    break
                k += 1
            if k == i:
                k = i + 1
            out.append(127 + (k - i))
            for v in px[i:k]:
                out += bytes((v >> 8, v & 255))
            i = k
    return bytes(out)


def rle_decode(data, n_px):
    out, p = [], 0
    while len(out) < n_px:
        c = data[p]
        p += 1
        if c < 128:
            v = (data[p] << 8) | data[p + 1]
            p += 2
            out += [v] * (c + 3)
        else:
            for _ in range(c - 127):
                out.append((data[p] << 8) | data[p + 1])
                p += 2
    return out[:n_px]


def c_bytes(name, data):
    lines = [",".join(str(b) for b in data[i:i + 24]) + "," for i in range(0, len(data), 24)]
    return f"const uint8_t {name}[{len(data)}] = {{\n" + "\n".join("  " + l for l in lines) + "\n};\n"


def main():
    arm_png, home_png, select_png, outdir = sys.argv[1], sys.argv[2], sys.argv[3], Path(sys.argv[4])
    check = Path(sys.argv[5]) if len(sys.argv) > 5 else None
    arm = Image.open(arm_png).convert("RGBA")

    assets = []                                     # (enum name, image, in arm file?)

    def add(name, im, big=False):
        assets.append((name, im.convert("RGB"), big))

    add("HOME", Image.open(home_png))
    add("SELECT", Image.open(select_png))
    add("HDR_SINGLE", comp_header(icon_cube, "SINGLE MODE", 46))
    add("HDR_GROUP", comp_header(layers_icon, "GROUP MODE", 48))
    add("HDR_CART", comp_header(layers_icon, "CARTESIAN MODE", 53))
    add("BTN_BACK", comp_back())
    add("BTN_PROCEED", comp_proceed())
    for lab in "XYZ":
        add(f"ROW_{lab}", comp_row(lab))
    add("SEL_NARROW", comp_selector(133))
    add("SEL_WIDE", comp_selector(164))
    for n in range(6):
        add(f"SINGLE_LABEL_{n}", text_tile(76, 26, SEL_FILL, f"Servo {n + 1}", 12, TEXT, 38))
    for g, (lab, sub) in enumerate(GROUPS):
        add(f"GROUP_LABEL_{g}", text_tile(116, 26, SEL_FILL, lab, 12, TEXT, 60))
    for g, (lab, sub) in enumerate(GROUPS):
        add(f"GROUP_SUB_{g}", text_tile(172, 10, BG, sub, 7, SUB_GREY, 0, left=3))
    for k in range(6):
        add(f"SINGLE_ARM_{k}", arm_tile(arm, (150, 46, 143), highlight(k), SINGLE_TILE_ARM), True)
    for g in range(3):
        add(f"GROUP_ARM_{g}", arm_tile(arm, G_PLACE, hl_group(g, G_PLACE), MODE_TILE_ARM), True)
    add("CART_ARM", arm_tile(arm, C_PLACE, hl_axes(), MODE_TILE_ARM), True)

    # ---- voice mode pages and overlays
    add("HDR_VOICE", comp_header(icon_mic_header, "VOICE MODE", 39))
    add("PANEL_CAM", comp_panel_cam())
    for i, (nm, col) in enumerate(BOXES):
        add(f"CARD_{i}", comp_card(f"{nm} box", col))
    add("CHECK", comp_check())
    add("PANEL_PICK", comp_panel_pick())
    for i, (nm, _) in enumerate(BOXES):
        add(f"PICK_TXT_{i}", text_tile(255, 16, ROW_BG, f'" Pick the {nm.lower()} box "', 11, TEXT2, 0, left=2))
    add("PICK_TXT_4", text_tile(255, 16, ROW_BG, '" Say: pick the <colour> box "', 11, (120, 128, 140), 0, left=2))
    for i, (nm, _) in enumerate(BOXES):
        add(f"DEST_HEAD_{i}", comp_dest_head(nm))
    add("DEST_BODY", comp_dest_body())
    add("PANEL_A", comp_panel_a())
    for n in range(1, 21):
        add(f"DIST_{n}", comp_dist(n))
    add("SLIDER", comp_slider())
    add("KNOB", comp_knob())
    add("PANEL_SAY", comp_panel_say())
    add("PANEL_QUEUE", comp_panel_queue())
    add("OV_CONF_FRAME", comp_conf_frame())
    for i, s in enumerate(CONF_TEXTS):
        add(f"OV_CONF_SUB_{i}", comp_conf_sub(s))
    add("OV_PROC_FRAME", comp_proc_frame())
    for i, (nm, _) in enumerate(BOXES):
        add(f"OV_PROC_LINE_{i}", holding_tile(200, 16, nm, 12))
    add("OV_STOPH_FRAME", comp_stoph_frame())
    for i, (nm, _) in enumerate(BOXES):
        add(f"OV_STOPH_LINE_{i}", holding_tile(200, 18, nm, 13))
    add("OV_STOPE", comp_empty_frame("Stopped"))
    add("OV_COMPLETE", comp_empty_frame("Complete"))

    enc = []
    for name, im, big in assets:
        px = rgb565_words(im)
        data = rle_encode(px)
        assert rle_decode(data, len(px)) == px, name
        enc.append((name, im.size, data, big))
    total_raw = sum(w * h * 2 for _, (w, h), _, _ in enc)
    total = sum(len(d) for _, _, d, _ in enc)
    print(f"{len(enc)} assets: raw {total_raw} bytes -> packed {total} bytes ({total * 100 // total_raw}%)")

    # ---- lcd_assets.h
    gen = "/* generated by ProjectDevGuide/tools/build_lcd_assets.py - do not edit */\n"
    ids = "".join(f"  LCD_A_{n},\n" for n, _, _, _ in enc)
    pos = "".join(f"#define LCD_POS_{k}_X {v[0]}\n#define LCD_POS_{k}_Y {v[1]}\n" for k, v in POS.items())
    pos += (f"#define LCD_CHECK_DX {CHECK_DX}\n#define LCD_CHECK_DY {CHECK_DY}\n#define LCD_CARD_W {CARD_W}\n#define LCD_CARD_H {CARD_H}\n"
            f"#define LCD_OV_CONF_W {OV_CONF_W}\n#define LCD_OV_CONF_H {OV_CONF_H}\n#define LCD_OV_ST_W {OV_ST_W}\n#define LCD_OV_ST_H {OV_ST_H}\n")
    rows = (f"#define LCD_ROW_W {ROW_W}\n#define LCD_ROW_H {ROW_H}\n"
            f"#define LCD_ROWS_SINGLE_X {ROW_X_SINGLE}\n#define LCD_ROWS_GROUP_X {ROW_X_GROUP}\n#define LCD_ROWS_CART_X {ROW_X_CART}\n"
            f"#define LCD_ROWS_SINGLE_Y {{ {', '.join(map(str, ROW_Y_SINGLE))} }}\n"
            f"#define LCD_ROWS_GROUP_Y {{ {', '.join(map(str, ROW_Y_GROUP))} }}\n"
            f"#define LCD_ROWS_CART_Y {{ {', '.join(map(str, ROW_Y_CART))} }}\n")
    (outdir / "lcd_assets.h").write_text(
        gen + "#ifndef APP_LCD_ASSETS_H\n#define APP_LCD_ASSETS_H\n#include <stdint.h>\n\n"
        "/* draw one with lcd_port_asset(LCD_A_xxx, x, y). arrays: LCD_A_SINGLE_ARM(i) etc. */\n"
        "typedef enum {\n" + ids + "  LCD_A_COUNT\n} lcd_asset_id_t;\n\n"
        "#define LCD_A_SINGLE_LABEL(i) (LCD_A_SINGLE_LABEL_0 + (i))\n#define LCD_A_SINGLE_ARM(i)   (LCD_A_SINGLE_ARM_0 + (i))\n"
        "#define LCD_A_GROUP_LABEL(i)  (LCD_A_GROUP_LABEL_0 + (i))\n#define LCD_A_GROUP_SUB(i)    (LCD_A_GROUP_SUB_0 + (i))\n"
        "#define LCD_A_GROUP_ARM(i)    (LCD_A_GROUP_ARM_0 + (i))\n"
        "#define LCD_A_CARD(i)         (LCD_A_CARD_0 + (i))          /* 0 red, 1 green, 2 black, 3 blue */\n"
        "#define LCD_A_PICK_TXT(i)     (LCD_A_PICK_TXT_0 + (i))      /* 0..3 colours, 4 = nothing picked */\n"
        "#define LCD_A_DEST_HEAD(i)    (LCD_A_DEST_HEAD_0 + (i))\n"
        "#define LCD_A_DIST(cm)        (LCD_A_DIST_1 + (cm) - 1)     /* 1..20 */\n"
        "#define LCD_A_OV_CONF_SUB(i)  (LCD_A_OV_CONF_SUB_0 + (i))\n"
        "#define LCD_A_OV_PROC_LINE(i) (LCD_A_OV_PROC_LINE_0 + (i))\n"
        "#define LCD_A_OV_STOPH_LINE(i) (LCD_A_OV_STOPH_LINE_0 + (i))\n\n"
        "/* where each piece sits on its page (pixels) */\n" + pos + rows + "\n"
        "typedef struct { uint16_t w; uint16_t h; const uint8_t *rle; } lcd_asset_t;\n"
        "extern const lcd_asset_t lcd_assets[LCD_A_COUNT];\n\n"
        f"#define LCD_DIGIT_W {DIGIT_W}\n#define LCD_DIGIT_H {DIGIT_H}\n#define LCD_DIGIT_COUNT 12\n"
        "extern const uint8_t lcd_digit_font[LCD_DIGIT_COUNT][LCD_DIGIT_W * LCD_DIGIT_H];  /* alpha 0..255: 0-9 - space */\n"
        "#endif\n")

    # ---- lcd_assets.c / lcd_assets_arm.c
    def sym(n):
        return "lcd_d_" + n.lower()
    small = "".join(c_bytes(sym(n), d) + "\n" for n, _, d, big in enc if not big)
    bigs = "".join(c_bytes(sym(n), d) + "\n" for n, _, d, big in enc if big)
    decl = "".join(f"extern const uint8_t {sym(n)}[{len(d)}];\n" for n, _, d, big in enc if big)
    table = "".join(f"  {{ {w}, {h}, {sym(n)} }},\n" for n, (w, h), d, big in enc)
    gl = digit_font()
    fontc = ("const uint8_t lcd_digit_font[LCD_DIGIT_COUNT][LCD_DIGIT_W * LCD_DIGIT_H] = {\n"
             + "\n".join("  {\n" + "\n".join("    " + ",".join(f"{v:3d}" for v in g[i:i + DIGIT_W]) + ","
                                              for i in range(0, len(g), DIGIT_W)) + "\n  }," for g in gl) + "\n};\n")
    (outdir / "lcd_assets.c").write_text(
        gen + '#include "lcd/lcd_assets.h"\n\n' + decl + "\n" + small
        + "const lcd_asset_t lcd_assets[LCD_A_COUNT] = {\n" + table + "};\n\n" + fontc)
    (outdir / "lcd_assets_arm.c").write_text(gen + '#include "lcd/lcd_assets.h"\n\n' + bigs)

    # ---- checks and previews
    if check:
        check.mkdir(parents=True, exist_ok=True)
        with open(check / "manifest.txt", "w") as mf:
            for i, (n, (w, h), d, big) in enumerate(enc):
                mf.write(f"{i} {n} {w} {h}\n")
                (check / f"{i}.raw").write_bytes(b"".join(bytes((v >> 8, v & 255)) for v in rgb565_words(assets[i][1])))
        img = {n: im for n, im, _ in assets}

        def page(parts):
            p = Image.new("RGB", (320, 240), BG)
            for name, xy in parts:
                p.paste(img[name], xy)
            return p
        btn = [("BTN_BACK", POS["BTN_BACK"]), ("BTN_PROCEED", POS["BTN_PROCEED"])]
        for k in range(6):
            parts = [("HDR_SINGLE", (0, 0)), (f"SINGLE_ARM_{k}", POS["SINGLE_ARM"]), ("SEL_NARROW", POS["SINGLE_SEL"]),
                     (f"SINGLE_LABEL_{k}", POS["SINGLE_LABEL"])]
            parts += [(f"ROW_{c}", (ROW_X_SINGLE, y)) for c, y in zip("XYZ", ROW_Y_SINGLE)] + btn
            page(parts).save(check / f"single{k + 1}.png")
        for g in range(3):
            parts = [("HDR_GROUP", (0, 0)), (f"GROUP_ARM_{g}", POS["GROUP_ARM"]), ("SEL_WIDE", POS["GROUP_SEL"]),
                     (f"GROUP_LABEL_{g}", POS["GROUP_LABEL"]), (f"GROUP_SUB_{g}", POS["GROUP_SUB"])]
            parts += [(f"ROW_{c}", (ROW_X_GROUP, y)) for c, y in zip("XYZ", ROW_Y_GROUP)] + btn
            page(parts).save(check / f"group{g + 1}.png")
        parts = [("HDR_CART", (0, 0)), ("CART_ARM", POS["CART_ARM"])]
        parts += [(f"ROW_{c}", (ROW_X_CART, y)) for c, y in zip("XYZ", ROW_Y_CART)] + btn
        page(parts).save(check / "cartesian.png")
        # ---- voice pages (composed in the order lcd.c draws them) and the overlays on top of them
        vp = page([("HDR_VOICE", (0, 0)), ("PANEL_CAM", POS["PANEL_CAM"])]
                  + [(f"CARD_{i}", POS[f"CARD_{i}"]) for i in range(4)]
                  + [("PANEL_PICK", POS["PANEL_PICK"]), ("PICK_TXT_3", POS["PICK_TXT"])] + btn)
        for i in range(4):
            vp.paste(img["CHECK"], (POS[f"CARD_{i}"][0] + CHECK_DX, POS[f"CARD_{i}"][1] + CHECK_DY))
        vp.save(check / "voice.png")
        page([("HDR_VOICE", (0, 0)), ("DEST_HEAD_3", POS["DEST_HEAD"]), ("DEST_BODY", POS["DEST_BODY"]),
              ("BTN_BACK", POS["BTN_BACK"])]).save(check / "dest.png")
        cp = page([("HDR_VOICE", (0, 0)), ("PANEL_A", POS["PANEL_A"]), ("DIST_2", POS["DIST_VAL"]), ("SLIDER", POS["SLIDER"]),
                   ("PANEL_SAY", POS["PANEL_SAY"]), ("PANEL_QUEUE", POS["PANEL_QUEUE"])] + btn)
        dr = ImageDraw.Draw(cp)
        dr.rectangle([24, 104, 24 + 2 * 29 // 4 + 6, 106], fill=WHITE)
        cp.paste(img["KNOB"], (24 + 2 * 29 // 4 - 6, 99))
        cp.save(check / "custom.png")

        def over(base_page, parts, name):
            q = base_page.copy()
            for n, xy in parts:
                q.paste(img[n], xy)
            q.save(check / name)
        over(vp, [("OV_CONF_FRAME", POS["OV_CONF"]), ("OV_CONF_SUB_6", POS["OV_CONF_SUB"])], "ov_confirm.png")
        over(vp, [("OV_PROC_FRAME", POS["OV_STATUS"]), ("OV_PROC_LINE_3", POS["OV_PROC_LINE"])], "ov_processing.png")
        over(vp, [("OV_STOPH_FRAME", POS["OV_STATUS"]), ("OV_STOPH_LINE_3", POS["OV_STOPH_LINE"])], "ov_stopped_holding.png")
        over(vp, [("OV_STOPE", POS["OV_STATUS"])], "ov_stopped_empty.png")
        over(vp, [("OV_COMPLETE", POS["OV_STATUS"])], "ov_complete.png")
        img["HOME"].save(check / "home.png")
        img["SELECT"].save(check / "select.png")
    print("ok")


if __name__ == "__main__":
    main()
