#!/usr/bin/env python3
"""
witch_portrait.py: draws the Hex witch, an original portrait, and writes the talk portrait sheet.

  Data/CustomPortraits/Portraits/hex_witch.png       portrait 0xAE (talk portrait)

The shop portrait 0xAF is not made here: it comes from the supplied illustration, via
Tools/hex/witch_shop_from_art.py, and this script never touches hex_witch_shop.png.

The art is made here, so this script is the source of the talk sheet. The head is shaded as a lit surface (an ellipsoid
with bumps for the brows, eye sockets, nose, cheekbones and chin), the hair as strand texture, the hat and shawl as
flat lit shapes; outlines go where a layer meets one behind it; eyes, brows, mouth and age lines are placed by hand.
Coordinates are "face" coordinates: the face drawing is laid out in a 48x48 box, placed at the talk offset.

Sheet layout (PortraitFormatter, 128x112): face 96x80 at (0,0); minimug 32x32 at (96,16); blink frames 32x16 at
(96,48) half-closed and (96,64) closed; mouth frames 32x16 in the slots (0,80) (32,80) (64,80) (96,80) (0,96) (32,96)
(64,96). Slots: 0/1 talking (half open, open), 4/5 smiling while talking, 2/3/6 the resting mouth.

Talk portrait: the character at offset (24,22); mouth window tile (5,7) = (40,56); eye window tile (4,5) = (32,40).

Usage (from the repo root): python3 Tools/hex/witch_portrait.py [--preview DIR]
"""
import argparse
import math
import os

import numpy as np
from PIL import Image

PAL = [(120, 160, 112), (57, 32, 65), (248, 240, 222), (248, 208, 150), (226, 160, 104), (168, 108, 80),
       (98, 74, 74), (232, 232, 240), (180, 180, 198), (126, 126, 150), (156, 108, 186), (108, 70, 146),
       (66, 42, 98), (139, 98, 57), (232, 184, 80), (96, 168, 120)]
BG, OUT, S1, S2, S3, S4, DK, H1, H2, H3, P1, P2, P3, WD, GLD, GRN = range(16)
OUT_DIR = "Data/CustomPortraits/Portraits"
SLOTS = [(0, 80), (32, 80), (64, 80), (96, 80), (0, 96), (32, 96), (64, 96)]
SS = 4                                  # supersampling for the layer shapes
LIGHT = np.array([-0.42, -0.5, 0.76])
LIGHT /= np.linalg.norm(LIGHT)
FC, FRX, FRY = (23.5, 27.0), 12.0, 16.0


def gauss(x, y, cx, cy, sx, sy):
    return math.exp(-(((x - cx) / sx) ** 2 + ((y - cy) / sy) ** 2))


def lit(hfun, x, y, e=0.25):
    dx = (hfun(x + e, y) - hfun(x - e, y)) / (2 * e)
    dy = (hfun(x, y + e) - hfun(x, y - e)) / (2 * e)
    n = np.array([-dx, -dy, 1.0])
    n /= np.linalg.norm(n)
    return max(0.0, float(n @ LIGHT))


def face_in(x, y):
    v = (y - FC[1]) / FRY
    rx = FRX * (1 - 0.32 * max(0.0, v) ** 1.5)
    return ((x - FC[0]) / rx) ** 2 + v * v <= 1


def face_h(x, y):
    u, v = (x - FC[0]) / FRX, (y - FC[1]) / FRY
    z = math.sqrt(max(0.0, 1 - u * u - v * v)) * 9
    z += 1.5 * gauss(x, y, 20.6, 28.5, 1.3, 3.2) + 1.4 * gauss(x, y, 20.4, 31.6, 1.7, 1.2)     # nose
    z += 1.0 * gauss(x, y, 17.0, 21.2, 3.6, 1.2) + 1.0 * gauss(x, y, 27.5, 21.2, 3.6, 1.2)     # brows
    z -= 1.3 * gauss(x, y, 17.2, 24.2, 2.6, 1.7) + 1.3 * gauss(x, y, 27.6, 24.2, 2.8, 1.7)     # eye sockets
    z += 1.2 * gauss(x, y, 13.8, 29.0, 2.6, 3.0) + 1.3 * gauss(x, y, 30.5, 29.0, 2.8, 3.0)     # cheekbones
    z += 0.9 * gauss(x, y, 22.0, 40.5, 3.0, 1.8)                                               # chin
    z -= 0.7 * gauss(x, y, 16.0, 35.0, 0.7, 3.0) + 0.7 * gauss(x, y, 27.5, 35.0, 0.7, 3.0)     # smile folds
    z -= 0.6 * gauss(x, y, 22.0, 38.5, 4.5, 0.6)                                               # under the lip
    return z


def hair_in(x, y):
    if y < 11 or y > 46:
        return False
    left_edge = 4.5 + 2.2 * math.sin(y * 0.45) + (y - 11) * 0.05
    right_edge = 43.5 - 2.2 * math.sin(y * 0.45 + 1.0) - (y - 11) * 0.05
    half = FRX * math.sqrt(max(0, 1 - ((y - FC[1]) / FRY) ** 2))
    left_inner, right_inner = FC[0] - half + 1.5, FC[0] + half - 1.5
    if left_edge <= x <= max(left_inner, left_edge) or (y < 18 and 6 < x < 20):
        return True
    return min(right_inner, right_edge) <= x <= right_edge or (y < 18 and 28 < x < 42)


def hair_val(x, y):
    side = -1 if x < 24 else 1
    strand = 0.5 + 0.5 * math.sin((x * 1.6 - side * y * 0.55) + 2 * math.sin(y * 0.3))
    shade = 0.75 - 0.35 * (1 if side > 0 else 0) - 0.012 * (y - 11)
    return shade * 0.7 + strand * 0.45


def brim_in(x, y):
    return ((x - 24) / 23.5) ** 2 + ((y - 11) / 5.0) ** 2 <= 1


def crown_in(x, y):
    return y <= 10 and abs(x - (25 + (10 - y) * 0.25)) <= 9.5 - (10 - y) * 0.35


def body_in(x, y):
    return y >= 40 and abs(x - 24) <= 9 + 27 * min(1.0, (y - 40) / 13) ** 0.55


def layer_at(x, y):
    if brim_in(x, y) or crown_in(x, y):
        return 4
    if face_in(x, y):
        return 3
    if hair_in(x, y):
        return 2
    if body_in(x, y):
        return 1
    return 0


def render(w, h, ox, oy, mouth="rest", eyes="open"):
    """The witch on a w x h canvas with the origin at (ox, oy), over a plain background."""
    img = np.zeros((h, w), dtype=np.uint8)
    lay = np.zeros((h, w), dtype=np.uint8)
    for py in range(h):
        for px in range(w):
            acc = {}
            for sy in range(SS):
                for sx in range(SS):
                    l = layer_at(px - ox + (sx + .5) / SS, py - oy + (sy + .5) / SS)
                    acc[l] = acc.get(l, 0) + 1
            l = max(acc, key=acc.get)
            lay[py, px] = l
            x, y = px - ox + .5, py - oy + .5
            if l == 3:
                i = 0.4 + 0.72 * lit(face_h, x, y) - 0.22 * max(0, (x - 29) / 7)
                if y < 14:
                    i -= 0.3
                c = S1 if i > 0.9 else S2 if i > 0.66 else S3 if i > 0.46 else S4
            elif l == 2:
                i = hair_val(x, y)
                c = H1 if i > 0.78 else H2 if i > 0.48 else H3
            elif l == 4:
                if brim_in(x, y) and not crown_in(x, y):
                    top = ((x - 24) / 23.5) ** 2 + ((y - 9.6) / 3.6) ** 2 <= 1
                    c = (P1 if x < 20 else P2) if top else P3
                else:
                    t = (x - (25 + (10 - y) * 0.25)) / (9.5 - (10 - y) * 0.35 + 1e-6)
                    c = P1 if t < -0.35 else P2 if t < 0.45 else P3
                    if 6 <= y <= 8:
                        c = GLD if t < 0.5 else WD
            elif l == 1:
                # plum shawl: lit on the left shoulder, a collar V, a gold clasp
                d = x - 24
                c = P2 if d < -6 else P3
                if d < -14 and (int(y) + int(x)) % 7 == 0:
                    c = P1
                if abs(abs(d) - (y - 40) * 0.6) < 0.8 and y < 52:
                    c = P1
                if abs(d) <= 1.2 and 43 <= y <= 45.5:
                    c = GLD
            else:
                c = BG
            img[py, px] = c
    out = img.copy()
    for py in range(h):
        for px in range(w):
            l = lay[py, px]
            if l == 0:
                continue
            for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)):
                xx, yy = px + dx, py + dy
                if 0 <= xx < w and 0 <= yy < h and lay[yy, xx] < l:
                    out[py, px] = OUT
                    break
    features(out, ox, oy, mouth, eyes)
    return out


def features(img, ox, oy, mouth, eyes):
    def P(x, y, c):
        img[y + oy, x + ox] = c

    def row(y, x0, cs):
        for i, c in enumerate(cs):
            if c is not None:
                P(x0 + i, y, c)

    o, w, g, k = OUT, H1, GRN, P3
    if eyes == "open":
        row(21, 14, [None, o, o, o, o, o])
        row(22, 13, [o, w, g, g, o, w, o])
        row(23, 13, [None, S3, g, k, g, S2])
        row(24, 14, [None, S3, S3, S3])
        row(21, 24, [None, o, o, o, o, o, o])
        row(22, 23, [o, w, w, g, g, o, w, o, o])
        row(23, 23, [None, S2, g, k, k, g, w, S3])
        row(24, 24, [None, S3, g, g, S3, S3])
        row(25, 25, [S3, S3, S3])
        P(25, 22, w)
        P(26, 22, g)
        P(15, 22, w)
    elif eyes == "half":
        row(22, 13, [o, o, o, o, o, o, o])
        row(23, 14, [S3, g, k, g, S3])
        row(22, 23, [o, o, o, o, o, o, o, o, o])
        row(23, 24, [S3, g, k, k, g, S3, S3])
        row(24, 25, [S3, S3, S3])
    else:
        row(23, 13, [None, o, o, o, o, o])
        row(24, 15, [S3, S3, S3])
        row(23, 24, [o, o, o, o, o, o, o])
        row(24, 25, [S3, S3, S3, S3])
    P(31, 21, o)
    P(32, 20, o)
    row(19, 13, [None, H2, H1, H1, H2, H2])
    row(19, 24, [None, H2, H1, H1, H1, H2, H3])
    P(20, 27, S1)
    P(20, 28, S1)
    P(21, 30, S1)
    P(20, 33, S4)
    P(19, 33, OUT)
    P(22, 33, S4)
    smile = mouth.startswith("smile") or mouth == "rest"
    up = 34 if mouth.startswith("smile") else 35
    if mouth == "rest":
        row(36, 19, [o, o, o, o, o, o])
        P(18, 35, o)
        P(25, 35, o)
        row(37, 20, [S2, S2, S2, S2])
        row(38, 21, [S3, S3])
    else:
        row(36, 19, [o, o, o, o, o, o])
        P(18, up, o)
        P(25, up, o)
        if mouth.endswith("half"):
            row(37, 20, [o, DK, DK, o])
            row(38, 21, [S3, S3])
        else:
            row(37, 19, [o, DK, w, w, DK, o])
            row(38, 20, [o, DK, DK, o])
            row(39, 21, [S3, S3])
    if smile:
        P(17, 34, S4)
        P(26, 34, S4)
    P(32, 24, S4)
    P(33, 25, S4)
    P(12, 24, S4)
    row(26, 26, [S4, S4, S4])
    row(25, 15, [S4, S4])
    P(16, 33, S4)
    P(16, 34, S4)
    P(27, 33, S4)
    P(27, 34, S4)


def to_image(a):
    im = Image.new("P", (a.shape[1], a.shape[0]))
    im.putpalette(sum(PAL, ()) + (0,) * (768 - 48))
    im.putdata([int(v) for v in a.flatten()])
    return im


MOUTHS = ["half", "open", "rest", "rest", "smile_half", "smile_open", "rest"]


def build():
    talk_ox, talk_oy = 24, 22
    sheet = np.zeros((112, 128), dtype=np.uint8)
    face = render(96, 80, talk_ox, talk_oy)
    sheet[0:80, 0:96] = face
    mini = to_image(face).convert("RGB").crop((24, 18, 72, 66)).resize((32, 32), Image.LANCZOS)
    sheet[16:48, 96:128] = np.array(mini.quantize(palette=to_image(np.zeros((1, 1), np.uint8)), dither=Image.Dither.NONE))
    for (sx, sy), kind in (((96, 48), "half"), ((96, 64), "closed")):
        sheet[sy:sy + 16, sx:sx + 32] = render(96, 80, talk_ox, talk_oy, eyes=kind)[40:56, 32:64]
    for (sx, sy), kind in zip(SLOTS, MOUTHS):
        sheet[sy:sy + 16, sx:sx + 32] = render(96, 80, talk_ox, talk_oy, mouth=kind)[56:72, 40:72]
    return to_image(sheet)


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("--preview", help="also write a 4x preview of the talk sheet to this directory")
    args = ap.parse_args()
    sheet = build()
    sheet.save(os.path.join(OUT_DIR, "hex_witch.png"))
    print("wrote", os.path.join(OUT_DIR, "hex_witch.png"))
    if args.preview:
        os.makedirs(args.preview, exist_ok=True)
        sheet.convert("RGB").resize((512, 448), Image.NEAREST).save(os.path.join(args.preview, "hex_witch_4x.png"))


if __name__ == "__main__":
    main()
