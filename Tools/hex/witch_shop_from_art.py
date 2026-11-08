#!/usr/bin/env python3
"""
witch_shop_from_art.py: builds the Hex shop witch's shopkeeper sheet (portrait 0xAF) from a supplied illustration.

  python3 Tools/hex/witch_shop_from_art.py Tools/hex/art/hex_witch_shop_source.webp \
      Data/CustomPortraits/Portraits/hex_witch_shop.png [--preview PATH]

Steps, as used for the current art:
  1. Crop the illustration to its non-white area and sample it down to the 48x48 shopkeeper box (the median colour of
     the middle of each cell; the source is pixel art that is not on an exact grid, so it is resampled, not read).
  2. Map every pixel to a fixed 15-colour palette picked from the source (the face skin has its own cream, FC) (snapped to the GBA's 5-bit colour grid);
     index 0 is the transparent green that every portrait sheet uses.
  3. The eyes, nose and mouth are transcribed cell by cell from the source at the 48x48 grid (FEATURES): the source
     draws them with details half a box pixel wide, which no resample keeps.
  4. Mouth frames: the mouth sits on box row 32, so the 32x16 window is box (0,32), i.e. sheet tile (3,4)
     (setMugEntry(0xAF, ..., 3, 4, 3, 2)). Slots 0/1 talk (half, open), 4/5 smiling talk, 2/3/6 resting.
The box goes to sheet (24,0); the sheet is 128x112 (PortraitFormatter layout), like the vanilla shopkeepers.
"""
import argparse

import numpy as np
from PIL import Image

PAL = [(120, 160, 112), (56, 40, 72), (80, 56, 96), (112, 88, 136), (144, 96, 160), (248, 232, 200), (152, 104, 72),
       (216, 160, 88), (248, 200, 120), (248, 216, 168), (232, 232, 240), (152, 152, 176), (184, 184, 192),
       (120, 128, 152), (96, 136, 72), (184, 136, 80)]
O, DP, PM, PL, FC, BR, GO, SK2, SK1, WH, HM, HL, HD, GR, SK3 = range(1, 16)
SLOTS = [(0, 80), (32, 80), (64, 80), (96, 80), (0, 96), (32, 96), (64, 96)]
KINDS = ["half", "open", "rest", "rest", "smile_half", "smile_open", "rest"]


def sample_box(path):
    im = Image.open(path).convert("RGB")
    a = np.array(im).astype(int)
    ys, xs = np.nonzero(a.min(-1) < 235)
    a = a[ys.min():ys.max() + 1, xs.min():xs.max() + 1]
    h, w = a.shape[:2]
    box = np.zeros((48, 48, 3), int)
    for j in range(48):
        for i in range(48):
            x0, x1 = int((i + .3) * w / 48), int((i + .7) * w / 48)
            y0, y1 = int((j + .3) * h / 48), int((j + .7) * h / 48)
            box[j, i] = np.median(a[y0:y1, x0:x1].reshape(-1, 3), axis=0)
    p = np.array(PAL[1:])
    weight = np.array([0.3, 0.59, 0.11]) * 3
    d = (((box[:, :, None, :] - p[None, None, :, :]) ** 2) * weight).sum(-1)
    return d.argmin(-1) + 1


# The eyes, nose and mouth, transcribed cell by cell from the source at the 48x48 grid: the source draws them with
# details half a box pixel wide, which no resample keeps. 'w' = face skin, 'W' = white of the eye, '.' = keep.
FEATURES = [
    (22, 20, ["wwwwwww",
              "wOOOOOO",
              "OWWOOWO",
              "wwWOgWw",
              "wwwggww",
              "www22ww"]),      # near eye, cols 22..28, rows 20..25: one lash row, then a 3-row eye
    (14, 22, ["2www2",
              "OOOOb",
              "OWOO2",
              "wWOgw",
              "wwggw",
              "www2w"]),        # far eye, cols 14..18, rows 22..27: lash row 23, then a 3-row eye
    (18, 28, ["www",
              "w3w",
              "2Ow"]),          # nose, cols 18..20, rows 28..30
    (19, 31, ["wwboobw",
              "wwO33ww",
              "www22ww"]),      # mouth, cols 19..25, rows 31..33 (row 31 is the upper lip, outside the mouth window)
]
CHARS = {"O": O, "W": WH, "w": FC, "g": GR, "1": SK1, "2": SK2, "3": SK3, "b": BR, "o": GO, "d": DP}


def paint(idx, patches):
    for x0, y0, rows in patches:
        for j, r in enumerate(rows):
            for i, ch in enumerate(r):
                if ch != ".":
                    idx[y0 + j, x0 + i] = CHARS[ch]
    return idx


def paint_features(idx):
    return paint(idx, FEATURES)


# Mouth frames: the lower lip opens under the fixed upper lip. Rows 32..35 are inside the 32x16 window at box (0,32).
MOUTHS = {
    "half":       [(19, 32, ["wwObbww",
                             "www22ww"])],
    "open":       [(19, 32, ["wwObbOw",
                             "wwwbbww",
                             "www22ww"])],
}
MOUTHS["smile_half"] = [(19, 31, ["w3"]), (25, 31, ["3"])] + MOUTHS["half"]
MOUTHS["smile_open"] = [(19, 31, ["w3"]), (25, 31, ["3"])] + MOUTHS["open"]


def mouth(base, kind):
    if kind == "rest":
        return base.copy()
    return paint(base.copy(), MOUTHS[kind])


def to_image(a):
    im = Image.new("P", (a.shape[1], a.shape[0]))
    im.putpalette(sum(PAL, ()) + (0,) * (768 - 48))
    im.putdata([int(v) for v in a.flatten()])
    return im


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("source")
    ap.add_argument("out")
    ap.add_argument("--preview", help="also write the 48x48 box at 6x to this PNG")
    args = ap.parse_args()
    base = paint_features(sample_box(args.source))
    sheet = np.zeros((112, 128), np.uint8)
    sheet[0:48, 24:72] = base
    for (sx, sy), kind in zip(SLOTS, KINDS):
        sheet[sy:sy + 16, sx:sx + 32] = mouth(base, kind)[32:48, 0:32]
    to_image(sheet).save(args.out)
    print("wrote", args.out)
    if args.preview:
        to_image(base).convert("RGB").resize((288, 288), Image.NEAREST).save(args.preview)


if __name__ == "__main__":
    main()
