#!/usr/bin/env python3
"""
Generate the Spin For Luck wheel sprite.

    python3 make_wheel.py OUTDIR

Writes OUTDIR/Wheel.png (64x64), indexed with the 16-colour palette PAL
below (index 0 transparent). The C code loads the same palette
(sSpinForLuckPal in SpinForLuck.c), so keep the two in step. Indices 1-3
must stay the outline and gold colours: the shared arrow
(Contents/Gfx/Sources/SpinForGold/Arrow.png) is drawn with them.

Section i is centred at i * 45 degrees clockwise from the top of the
wheel and gives the status SECTIONS[i] (must match sSpinForLuckStatus).
Good statuses get blue sections, bad ones purple.
"""
import math, os, sys
from PIL import Image

PAL = [
    (0, 0, 0),        # 0 transparent
    (40, 24, 16),     # 1 outline
    (248, 208, 64),   # 2 rim gold
    (176, 120, 32),   # 3 rim dark gold
    (48, 96, 200),    # 4 good section
    (120, 48, 152),   # 5 bad section
    (248, 248, 248),  # 6 white
    (184, 192, 208),  # 7 light grey
    (104, 112, 128),  # 8 grey
    (216, 40, 40),    # 9 red
    (248, 136, 168),  # 10 pink
    (88, 200, 64),    # 11 green
    (32, 112, 40),    # 12 dark green
    (152, 216, 248),  # 13 sky
    (248, 232, 104),  # 14 yellow
    (144, 88, 40),    # 15 brown
]

GOOD, BAD = 4, 5

SECTIONS = ['attack', 'poison', 'defense', 'sleep', 'crit', 'silence', 'avoid', 'recover']
KIND = {'attack': GOOD, 'defense': GOOD, 'crit': GOOD, 'avoid': GOOD, 'recover': GOOD,
        'poison': BAD, 'sleep': BAD, 'silence': BAD}

# 1 outline, 6 white, 7 light grey, 8 grey, 9 red, A pink, B green, C dark green,
# D sky, E yellow, F brown, 2 gold, 3 dark gold
ICONS = {
    'attack': [  # sword, point up
        "....1....",
        "...161...",
        "...171...",
        "...171...",
        "...171...",
        "...171...",
        ".1112111.",
        "..13231..",
        "...1F1...",
        "...121...",
        "....1....",
    ],
    'defense': [  # shield
        "111111111",
        "1D66666D1",
        "1D6DDD6D1",
        "1D6DDD6D1",
        "1D6DDD6D1",
        ".1D6D6D1.",
        ".1DD6DD1.",
        "..1DDD1..",
        "...1D1...",
        "....1....",
    ],
    'crit': [  # starburst
        "....1....",
        "...1E1...",
        "1..1E1..1",
        "1E1EEE1E1",
        ".1EE6EE1.",
        "..1E6E1..",
        ".1EE6EE1.",
        "1E1EEE1E1",
        "1..1E1..1",
        "...1E1...",
        "....1....",
    ],
    'avoid': [  # wing
        "1........",
        "161......",
        "1661.....",
        "16D61111.",
        "16DD66661",
        "16DDDDD61",
        ".16DDDD61",
        "..1666661",
        "...111111",
    ],
    'recover': [  # heart
        ".11...11.",
        "1AA1.1991",
        "1A9919991",
        "199999991",
        "199999991",
        ".1999991.",
        "..19991..",
        "...191...",
        "....1....",
    ],
    'poison': [  # poison drop
        "....1....",
        "...1B1...",
        "...1B1...",
        "..1BBB1..",
        ".1B6BBB1.",
        ".1B6BBC1.",
        "1BBBBBBC1",
        "1BBBBBCC1",
        ".1BBCCC1.",
        "..11111..",
    ],
    'sleep': [  # Z
        "1111111",
        "1666661",
        "1111661",
        "..1661.",
        ".1661..",
        "1661111",
        "1666661",
        "1111111",
    ],
    'silence': [  # speech bubble with a cross
        ".1111111.",
        "166666661",
        "169666961",
        "166969661",
        "166696661",
        "166969661",
        "169666961",
        ".1666661.",
        "..161111.",
        "..11.....",
    ],
}
GLYPH = {'.': None, ' ': 6, '1': 1, '2': 2, '3': 3, '6': 6, '7': 7, '8': 8, '9': 9, 'A': 10,
         'B': 11, 'C': 12, 'D': 13, 'E': 14, 'F': 15}


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
            a = math.degrees(math.atan2(dx, -dy)) % 360
            sec = int(((a + 22.5) % 360) // 45)
            edge = abs(((a + 22.5) % 45) - 22.5)
            if 22.5 - edge < 26.0 / max(r, 1):
                px[x, y] = 1
                continue
            px[x, y] = KIND[SECTIONS[sec]]
    for i, name in enumerate(SECTIONS):
        rows = ICONS[name]
        h = len(rows)
        w = max(len(r) for r in rows)
        a = math.radians(i * 45)
        ix = cx + 17.0 * math.sin(a)
        iy = cy - 17.0 * math.cos(a)
        ox = int(round(ix - w / 2.0 + 0.5))
        oy = int(round(iy - h / 2.0 + 0.5))
        for yy, row in enumerate(rows):
            for xx, ch in enumerate(row):
                c = GLYPH.get(ch)
                if c:
                    px[ox + xx, oy + yy] = c
    return im


def main():
    out = sys.argv[1]
    os.makedirs(out, exist_ok=True)
    make_wheel().save(os.path.join(out, 'Wheel.png'), transparency=0)


if __name__ == '__main__':
    main()
