#!/usr/bin/env python3
"""
Generate the Spin For Gold wheel and arrow sprites.

    python3 make_wheel.py OUTDIR

Writes OUTDIR/Wheel.png (64x64) and OUTDIR/Arrow.png (16x16), both indexed
with the 16-colour palette PAL below (index 0 transparent). The C code
loads the same palette (sSpinForGoldPal in SpinForGold.c), so keep the two
in step.

Section i is centred at i * 45 degrees clockwise from the top of the
wheel and pays SECTIONS[i] x 100G (must match sSpinForGoldValues).
"""
import math, os, sys
from PIL import Image

PAL = [
    (0, 0, 0),        # 0 transparent
    (40, 24, 16),     # 1 outline
    (248, 208, 64),   # 2 rim gold
    (176, 120, 32),   # 3 rim dark gold
    (200, 56, 48),    # 4 section red
    (48, 96, 200),    # 5 section blue
    (248, 232, 104),  # 6 coin
    (248, 248, 208),  # 7 coin highlight
    (208, 136, 24),   # 8 coin shadow
    (160, 104, 48),   # 9 bag
    (104, 64, 32),    # 10 bag dark
    (248, 248, 248),  # 11 white
    (144, 144, 152),  # 12 grey
    (64, 64, 72),     # 13 dark grey
    (152, 32, 32),    # 14 red dark
    (32, 64, 152),    # 15 blue dark
]

SECTIONS = [5, 0, 1, 2, 0, 1, 3, 0]

ICONS = {
    0: [  # empty: a cross
        "11...11",
        "111.111",
        ".11111.",
        "..111..",
        ".11111.",
        "111.111",
        "11...11",
    ],
    1: [  # one coin
        "..111..",
        ".17661.",
        "1766661",
        "1666661",
        "1666681",
        ".18881.",
        "..111..",
    ],
    2: [  # two coins
        "..111....",
        ".17661...",
        "176666111",
        "16666 7661",
        "16611766661",
        ".1176666661",
        "..1666666681",
        "...16666681",
        "....188881.",
        ".....1111..",
    ],
    3: [  # stack of three coins
        "..11111..",
        ".1766661.",
        ".1888881.",
        ".1111111.",
        ".1766661.",
        ".1888881.",
        ".1111111.",
        ".1766661.",
        ".1888881.",
        "..11111..",
    ],
    5: [  # gold bag
        "..1...1..",
        "..1A1A1..",
        "...1A1...",
        "..19991..",
        ".1996991.",
        "196666691",
        "199669991",
        "199999991",
        ".1AAAAA1.",
        "..11111..",
    ],
}
GLYPH = {'.': None, ' ': 6, '1': 1, '6': 6, '7': 7, '8': 8, '9': 9, 'A': 10}
CROSS = {'1': 13}

DIGITS = {
    0: ["111", "1.1", "1.1", "1.1", "111"],
    1: [".1.", "11.", ".1.", ".1.", "111"],
    2: ["111", "..1", "111", "1..", "111"],
    3: ["111", "..1", "111", "..1", "111"],
    5: ["111", "1..", "111", "..1", "111"],
}


def make_wheel():
    im = Image.new('P', (64, 64), 0)
    im.putpalette([c for rgb in PAL for c in rgb])
    px = im.load()
    cx = cy = 31.5
    for y in range(64):
        for x in range(64):
            dx, dy = x - cx, y - cy
            r = math.hypot(dx, dy)
            if r > 31.6:
                continue
            if r > 30.4:
                px[x, y] = 1
                continue
            if r > 28.0:
                px[x, y] = 2 if (dx + dy) < 0 else 3
                continue
            if r > 27.0:
                px[x, y] = 1
                continue
            if r < 4.5:
                px[x, y] = 1 if r > 3.6 else 2
                continue
            # clockwise angle from up, in sections
            a = math.degrees(math.atan2(dx, -dy)) % 360
            sec = int(((a + 22.5) % 360) // 45)
            edge = abs(((a + 22.5) % 45) - 22.5)
            # divider lines along section borders
            if 22.5 - edge < 26.0 / max(r, 1):
                px[x, y] = 1
                continue
            light = sec % 2 == 0
            px[x, y] = 4 if light else 5
    for i, v in enumerate(SECTIONS):
        rows = ICONS[v]
        h = len(rows)
        w = max(len(r) for r in rows)
        a = math.radians(i * 45)
        ix = cx + 14.0 * math.sin(a)
        iy = cy - 14.0 * math.cos(a)
        ox = int(round(ix - w / 2.0 + 0.5))
        oy = int(round(iy - h / 2.0 + 0.5))
        for yy, row in enumerate(rows):
            for xx, ch in enumerate(row):
                c = (CROSS if v == 0 else GLYPH).get(ch)
                if c:
                    px[ox + xx, oy + yy] = c
        dx_ = cx + 23.0 * math.sin(a)
        dy_ = cy - 23.0 * math.cos(a)
        ox = int(round(dx_ - 1.5 + 0.5))
        oy = int(round(dy_ - 2.5 + 0.5))
        for yy, row in enumerate(DIGITS[v]):
            for xx, ch in enumerate(row):
                if ch == '1':
                    px[ox + xx, oy + yy] = 11
    return im


ARROW = [
    "................",
    "...1111111111...",
    "...1222222221...",
    "...1222222221...",
    "....12222221....",
    "....12222221....",
    ".....122221.....",
    ".....122231.....",
    "......1231......",
    "......1231......",
    ".......11.......",
    "................",
    "................",
    "................",
    "................",
    "................",
]


def make_arrow():
    im = Image.new('P', (16, 16), 0)
    im.putpalette([c for rgb in PAL for c in rgb])
    px = im.load()
    for y, row in enumerate(ARROW):
        for x, ch in enumerate(row):
            if ch != '.':
                px[x, y] = int(ch)
    return im


def main():
    out = sys.argv[1]
    os.makedirs(out, exist_ok=True)
    make_wheel().save(os.path.join(out, 'Wheel.png'), transparency=0)
    make_arrow().save(os.path.join(out, 'Arrow.png'), transparency=0)


if __name__ == '__main__':
    main()
