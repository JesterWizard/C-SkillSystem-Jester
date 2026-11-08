#!/usr/bin/env python3
"""
witch_shop_from_art.py: builds the Hex shop witch's shopkeeper sheet (portrait 0xAF) from a supplied pixel-art illustration.

  python3 Tools/hex/witch_shop_from_art.py Tools/hex/art/hex_witch_shop_source.webp \
      Data/CustomPortraits/Portraits/hex_witch_shop.png [--preview PATH]

Steps, as used for the current art:
  1. The illustration is pixel art on a 64x64 grid (about 19.6 px per art pixel). The cell edges are found from the
     image's own colour edges, so a slightly uneven grid reads the same way (cuts(): the cut positions, 15..25 px apart, that sit on the
     strongest edges), and each cell is read as the median colour of its middle, so the art is recovered pixel for
     pixel, with no resampling.
  2. A 48x48 window of that grid (CROP) is the shopkeeper box: the hat brim with its gold ornament, the face, the hair
     and the earrings, like the vanilla shopkeepers' head-and-shoulders framing.
  3. Every pixel goes to the nearest colour of a fixed 15-colour palette (PAL[1:], on the GBA's 5-bit colour grid),
     fitted to the art with the face weighted up and the outline, white, gold, both eye greens and the lip red kept.
     Warm source pixels (lips, skin shadows) never go to the purple or grey shades. Index 0 is the transparent green
     that every portrait sheet uses.
  4. Mouth frames: the 32x16 mouth window is box (8,24), i.e. sheet tile (4,3) (setMugEntry(0xAF, ..., 4, 3, 3, 2)).
     The resting frame is the art's own smile. Talking closes it to the lip line (half) and opens it one row
     more (open). Slots 0/1 talk (half, open), 4/5 smiling talk, 2/3/6 resting.
  5. The shop draws the shopkeeper box mirrored, so the box and every mouth frame are stored flipped left to right;
     in game she then faces the way she does in the art. The mouth window (box x 8..39) is symmetric about the box
     centre, so mirroring keeps it in place.
The box goes to sheet (24,0); the sheet is 128x112 (PortraitFormatter layout), like the vanilla shopkeepers.
"""
import argparse

import numpy as np
from PIL import Image

CROP = (12, 6)          # top-left art pixel of the 48x48 box
PAL = [(120, 160, 112),
       (56, 32, 64), (104, 56, 112), (176, 96, 176),                      # outline, hat and cloak purples
       (112, 88, 128), (152, 136, 168), (200, 184, 200), (240, 232, 232),  # hair greys, white
       (248, 224, 200), (248, 184, 128), (192, 120, 80),                  # skin
       (128, 80, 72), (232, 176, 72),                                     # brown, gold
       (56, 96, 64), (104, 144, 96), (224, 96, 80)]                       # eye greens, lip red
COOL = [2, 3, 4, 5, 6]
SLOTS = [(0, 80), (32, 80), (64, 80), (96, 80), (0, 96), (32, 96), (64, 96)]
KINDS = ["half", "open", "rest", "rest", "half", "open", "rest"]
MOUTH_WIN = (8, 24)     # box x, y of the 32x16 mouth window


def cuts(d, n, lo=15, hi=25):
    """Cell edges along one axis: 0 = c0 < c1 < ... = n, each gap lo..hi px, on the strongest colour edges."""
    e = np.zeros(n + 1)
    e[1:n] = d[:n - 1]
    best = np.full(n + 1, -np.inf)
    prev = np.zeros(n + 1, int)
    best[0] = 0
    for c in range(1, n + 1):
        for g in range(lo, hi + 1):
            p = c - g
            if p >= 0 and best[p] + e[c] > best[c]:
                best[c], prev[c] = best[p] + e[c], p
    out = [n]
    while out[-1] > 0:
        out.append(prev[out[-1]])
    return out[::-1]


def read_grid(path):
    a = np.array(Image.open(path).convert("RGB")).astype(int)
    ry = cuts(np.abs(np.diff(a, axis=0)).sum(-1).sum(1).astype(float), a.shape[0])
    rx = cuts(np.abs(np.diff(a, axis=1)).sum(-1).sum(0).astype(float), a.shape[1])
    g = np.zeros((len(ry) - 1, len(rx) - 1, 3), int)
    for j in range(len(ry) - 1):
        for i in range(len(rx) - 1):
            y0, y1, x0, x1 = ry[j], ry[j + 1], rx[i], rx[i + 1]
            cell = a[y0 + (y1 - y0) * 3 // 10:y0 + (y1 - y0) * 7 // 10 + 1,
                     x0 + (x1 - x0) * 3 // 10:x0 + (x1 - x0) * 7 // 10 + 1]
            g[j, i] = np.median(cell.reshape(-1, 3), axis=0)
    return g


def to_index(box):
    px = box.reshape(-1, 3)
    p = np.array(PAL[1:])
    weight = np.array([0.3, 0.59, 0.11]) * 3
    d = (((px[:, None, :] - p[None, :, :]) ** 2) * weight).sum(-1)
    warm = (px[:, 0] - px[:, 2]) > 60
    d[np.ix_(warm, [c - 1 for c in COOL])] = 1e12
    return (d.argmin(-1) + 1).reshape(box.shape[:2])


# Mouth, in box coordinates: columns 14..18, rows 28..30. 'O' outline, 'b' brown lip line, 'r' lip red, 'w' skin,
# 'm' skin mid (the lower lip). 'rest' is the art's own smile, unchanged.
CHARS = {"O": 1, "b": 11, "r": 15, "w": 8, "m": 9}
MOUTHS = {
    "rest": [],
    "half": [(14, 28, ["Obbbb",
                       ".mmw.",
                       "....."])],
    "open": [(14, 28, ["Obrrb",
                       ".rrm.",
                       ".mm.."])],
}


def paint(idx, patches):
    for x0, y0, rows in patches:
        for j, r in enumerate(rows):
            for i, ch in enumerate(r):
                if ch != ".":
                    idx[y0 + j, x0 + i] = CHARS[ch]
    return idx


def mouth(base, kind):
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
    x0, y0 = CROP
    base = mouth(to_index(read_grid(args.source)[y0:y0 + 48, x0:x0 + 48]), "rest")
    sheet = np.zeros((112, 128), np.uint8)
    sheet[0:48, 24:72] = base[:, ::-1]
    mx, my = MOUTH_WIN
    assert mx + 32 == 48 - mx, "mouth window must be symmetric for the mirrored box"
    for (sx, sy), kind in zip(SLOTS, KINDS):
        sheet[sy:sy + 16, sx:sx + 32] = mouth(base, kind)[my:my + 16, mx:mx + 32][:, ::-1]
    to_image(sheet).save(args.out)
    print("wrote", args.out)
    if args.preview:
        to_image(base).convert("RGB").resize((288, 288), Image.NEAREST).save(args.preview)


if __name__ == "__main__":
    main()
