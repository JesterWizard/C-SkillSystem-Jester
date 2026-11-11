#!/usr/bin/env python3
"""Draws the EXP bar styles (Kernel/Wizardry/ExpBarStyles) from Tools/expbar/styles.py.

    python3 Tools/expbar/expbar.py              writes Kernel/Wizardry/ExpBarStyles/Gfx/<Name>.png for every style
    python3 Tools/expbar/expbar.py --preview F  also writes a preview sheet of every style to F (no emulator needed)
    python3 Tools/expbar/expbar.py --only Blade only that style

Each PNG is a 64x64 indexed sheet, 8 tiles a row, in the order the game loads it:
  tiles  0-27  the window (Img_EkrExpBar layout): 0 TL, 1 top, 2 TR, 3 left, 4 inside, 5 right, 6 BL, 7 bottom,
               8 BR, 9-12 the label; 13-27 are not loaded by the game (a pre-drawn bar and "10")
  tiles 28-51  the 24 fill steps (Img_EkrExpBarChange layout): left cap 0-6, body tile 7-16, right cap 17-23
  tiles 52-62  digits 0-9 and a blank (Img_BarNumfx layout)
Its palette is the style's 16 colours; colour 0 is transparent in game. The build turns each PNG into a .4bpp and a
.gbapal (makefile grit rules). See Documentation/Features/ExpBarStyles.md.
"""
import argparse
import math
import os
import sys

from PIL import Image

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.normpath(os.path.join(HERE, '..', '..'))
OUT = os.path.join(ROOT, 'Kernel', 'Wizardry', 'ExpBarStyles', 'Gfx')

# Column patterns of the 24 fill steps (F full, e the edge after the fill, . empty), in Img_EkrExpBarChange order.
# They reproduce vanilla's 24 tiles exactly from its full and empty bar.
STEPS = ['........', '.FFe....', '.FFFe...', '.FFFFe..', '.FFFFFe.', '.FFFFFFe', '.FFFFFFF',
         '........', 'e.......', 'Fe......', 'FFe.....', 'FFFe....', 'FFFFe...', 'FFFFFe..', 'FFFFFFe.', 'FFFFFFFe',
         'FFFFFFFF',
         '........', 'e.......', 'Fe......', 'FFe.....', 'FFFe....', 'FFFFe...', 'FFFFFFe.']

LABEL_FONTS = {
    'bold': {'E': ['####', '#...', '###.', '#...', '####'], 'X': ['#..#', '#..#', '.##.', '#..#', '#..#'],
             'P': ['###.', '#..#', '###.', '#...', '#...']},
    'slim': {'E': ['###', '#..', '##.', '#..', '###'], 'X': ['#.#', '#.#', '.#.', '#.#', '#.#'],
             'P': ['##.', '#.#', '##.', '#..', '#..']},
}
DIGIT_FONTS = {
    # 5x6 with 2-pixel strokes, drawn at x=1, y=1: with its outline it fills columns 0-6 and rows 0-7
    'bold': {
        '0': ['.###.', '##.##', '##.##', '##.##', '##.##', '.###.'],
        '1': ['.##..', '###..', '.##..', '.##..', '.##..', '####.'],
        '2': ['####.', '...##', '.###.', '##...', '##...', '#####'],
        '3': ['####.', '...##', '.###.', '...##', '...##', '####.'],
        '4': ['##.##', '##.##', '#####', '...##', '...##', '...##'],
        '5': ['#####', '##...', '####.', '...##', '...##', '####.'],
        '6': ['.###.', '##...', '####.', '##.##', '##.##', '.###.'],
        '7': ['#####', '...##', '..##.', '.##..', '.##..', '.##..'],
        '8': ['.###.', '##.##', '.###.', '##.##', '##.##', '.###.'],
        '9': ['.###.', '##.##', '##.##', '.####', '...##', '.###.'],
    },
}


def grid(w, h, v=None):
    return [[v] * w for _ in range(h)]


def parse(rows, legend):
    """ASCII art to colour names; '.' is None (what is behind: the window, or transparent)."""
    return [[None if ch in '. ' else legend[ch] for ch in r] for r in rows]


def frame(st):
    """The window as a 24x24 picture (3x3 tiles) of colour names; None is transparent."""
    f = st['frame']
    prof = f.get('profile', [])
    inner = f['interior']
    shape = f.get('corner', 'square')
    r = f.get('radius', 3)
    g = grid(24, 24)
    for y in range(24):
        for x in range(24):
            d = min(x, 23 - x, y, 23 - y)
            cx, cy = min(x, 23 - x), min(y, 23 - y)
            if shape == 'round' and cx < r and cy < r:
                dd = r - math.hypot(r - 0.5 - cx, r - 0.5 - cy) + 0.35
                if dd < 0:
                    continue
                d = min(d, int(dd))
            elif shape == 'cut' and cx + cy < r:
                continue
            elif shape == 'cut':
                d = min(d, cx + cy - r)
            g[y][x] = prof[d] if d < len(prof) else inner
    for kind, arg in f.get('deco', []):
        if kind == 'dots':          # a dotted line: (depth from the edge, colour, period, phase)
            depth, col, per, ph = arg
            for y in range(24):
                for x in range(24):
                    cx, cy = min(x, 23 - x), min(y, 23 - y)
                    on = (cy == depth and cx >= depth) or (cx == depth and cy >= depth)
                    if on and g[y][x] is not None and (x if cy == depth else y) % per == ph:
                        g[y][x] = col
        elif kind == 'rivets':      # a 2x2 rivet in each corner: (offset, colour, highlight)
            off, col, hi = arg
            for (x, y) in ((off, off), (22 - off, off), (off, 22 - off), (22 - off, 22 - off)):
                g[y][x], g[y][x + 1], g[y + 1][x], g[y + 1][x + 1] = hi, col, col, col
        elif kind == 'art':         # pixel art over the window: (x, y, rows, legend); '.' keeps, '_' is transparent
            ax, ay, rows, leg = arg
            for j, row in enumerate(rows):
                for i, ch in enumerate(row):
                    if ch != '.':
                        g[ay + j][ax + i] = None if ch == '_' else leg[ch]
    return g


def tile_of(g, tx, ty):
    return [row[tx * 8:tx * 8 + 8] for row in g[ty * 8:ty * 8 + 8]]


def put(g, pts, col, outline=None, shadow=None):
    s = set(pts)
    h, w = len(g), len(g[0])
    if shadow:
        for (px, py) in pts:
            if px + 1 < w and py + 1 < h and (px + 1, py + 1) not in s:
                g[py + 1][px + 1] = shadow
    if outline:
        for (px, py) in pts:
            for dx in (-1, 0, 1):
                for dy in (-1, 0, 1):
                    q = (px + dx, py + dy)
                    if q not in s and 0 <= q[0] < w and 0 <= q[1] < h:
                        g[q[1]][q[0]] = outline
    for (px, py) in pts:
        g[py][px] = col


def label(st, top):
    """Tiles 9-12: the label, drawn on four copies of the top edge tile."""
    lab = st['label']
    g = [r * 4 for r in top]
    if lab.get('plate'):
        x0, x1, y0, y1 = lab['plate']
        cut = lab.get('cut', 0)
        for y in range(y0, y1 + 1):
            for x in range(x0, x1 + 1):
                cx, cy = min(x - x0, x1 - x), min(y - y0, y1 - y)
                if cut and cx + cy < cut:
                    continue
                edge = lab.get('edge') and (cx == 0 or cy == 0 or (cut and cx + cy == cut))
                g[y][x] = lab['edge'] if edge else lab['fill']
    pts = []
    x = lab['x']
    for ch in 'EXP':
        glyph = LABEL_FONTS[lab.get('font', 'bold')][ch]
        pts += [(x + i, lab['y'] + j) for j, row in enumerate(glyph) for i, c in enumerate(row) if c == '#']
        x += len(glyph[0]) + 1
    put(g, pts, lab['col'], lab.get('outline'), lab.get('shadow'))
    for (px, py, col) in lab.get('pixels', []):
        g[py][px] = col
    return [[row[i * 8:i * 8 + 8] for row in g] for i in range(4)]


def bar_tiles(st, bg):
    """The 24 fill steps, from the style's full and empty bar. bg is the inside tile ('.' in the bar art)."""
    b = st['bar']
    leg = b['legend']
    full = {k: parse(b['full'][k], leg) for k in 'LBR'}
    empty = {k: parse(b['empty'][k], leg) for k in 'LBR'}
    edge = b.get('edge', '........')
    # The fill never reaches the left cap's column 0 or the right cap's columns 6-7: draw them the same full and empty
    for k, cols in (('L', [0]), ('R', [6, 7])):
        for x in cols:
            if [full[k][y][x] for y in range(8)] != [empty[k][y][x] for y in range(8)]:
                sys.exit('%s: the %s cap\'s column %d must be the same full and empty' % (st['name'], k, x))
    out = []
    for i, pat in enumerate(STEPS):
        k = 'L' if i < 7 else ('B' if i < 17 else 'R')
        t = grid(8, 8)
        for x, ch in enumerate(pat):
            for y in range(8):
                if ch == 'F' or (ch == 'e' and k == 'R' and x >= 6):
                    v = full[k][y][x]
                elif ch == 'e' and edge[y] != '.':
                    v = leg[edge[y]]
                else:
                    v = empty[k][y][x]
                t[y][x] = bg[y][x] if v is None else v
        out.append(t)
    return out


def digit_tiles(st, bg):
    d = st['digits']
    font = DIGIT_FONTS[d.get('font', 'bold')]
    x0, y0 = d.get('x', 1), d.get('y', 1)
    out = []
    for n in '0123456789':
        t = [row[:] for row in bg]
        pts = [(x0 + i, y0 + j) for j, row in enumerate(font[n]) for i, c in enumerate(row) if c == '#']
        put(t, pts, d['col'], d.get('outline'), d.get('shadow'))
        for (px, py) in pts:            # the lower half of each glyph in the low colours
            if py >= y0 + 3:
                t[py][px] = d.get('low2', d.get('low', d['col'])) if py == y0 + 5 else d.get('low', d['col'])
        out.append(t)
    out.append([row[:] for row in bg])  # blank (a tens digit of 0)
    return out


def build(st):
    """The style's 64 tiles (colour names) and palette (hex strings, colour 0 first)."""
    g = frame(st)
    tiles = [tile_of(g, tx, ty) for ty in range(3) for tx in range(3)]
    tiles += label(st, tile_of(g, 1, 0))
    bg = tiles[4]
    steps = bar_tiles(st, bg)
    digits = digit_tiles(st, bg)
    extra = [steps[6], steps[11]] + [steps[7]] * 10 + [steps[17]] + [digits[1], digits[0]]
    names = [None] + list(st['pal'])
    if len(names) > 16:
        sys.exit('%s: %d colours, at most 15 plus transparent' % (st['name'], len(names) - 1))
    used = {v for t in tiles + steps + digits for row in t for v in row}
    missing = used - set(names)
    if missing:
        sys.exit('%s: colours not in its palette: %s' % (st['name'], sorted(missing)))
    return tiles + extra + steps + digits + [grid(8, 8)], names, [st.get('clear', 'ff00ff')] + list(st['pal'].values())


def write_png(st, path):
    tiles, names, pal = build(st)
    idx = {n: i for i, n in enumerate(names)}
    im = Image.new('P', (64, 64), 0)
    for k, t in enumerate(tiles):
        for y in range(8):
            for x in range(8):
                im.putpixel(((k % 8) * 8 + x, (k // 8) * 8 + y), idx[t[y][x]])
    flat = []
    for h in pal:
        flat += [int(h[0:2], 16), int(h[2:4], 16), int(h[4:6], 16)]
    im.putpalette(flat + [0] * (48 - len(flat)))
    im.save(path, bits=4)


def preview(styles, path, exp=(99, 29)):
    """Each style's map window at two EXP values, drawn from its tiles (the bar fill is approximate)."""
    scale, rows = 2, []
    for st in styles:
        tiles, names, pal = build(st)
        rgb = {n: tuple(int(h[i:i + 2], 16) for i in (0, 2, 4)) for n, h in zip(names, pal)}
        wins = []
        for e in exp:
            fill = e * 104 // 100           # pixels across the left cap, 11 body tiles and the right cap
            tens, ones = (e % 100) // 10, e % 10
            mid = [3, 52 + (tens if tens else 10), 52 + ones]
            for i in range(13):
                px = max(0, min(8, fill - 8 * i))
                if i == 0:
                    step = 0 if px < 2 else min(6, px - 1)
                elif i == 12:
                    step = 17 + (0 if px == 0 else min(6, px + 1))
                else:
                    step = 7 + (0 if px == 0 else min(9, px + 1))
                mid.append(28 + step)
            mid.append(5)
            wins.append([[0, 9, 10, 11, 12] + [1] * 11 + [2], mid, [6] + [7] * 15 + [8]])
        rows.append((tiles, rgb, wins))
    w, h = 17 * 8 * scale, 3 * 8 * scale
    img = Image.new('RGB', ((w + 8) * len(exp) + 8, (h + 8) * len(rows) + 8), (40, 120, 90))
    for r, (tiles, rgb, wins) in enumerate(rows):
        for c, win in enumerate(wins):
            ox, oy = 8 + c * (w + 8), 8 + r * (h + 8)
            for ty, line in enumerate(win):
                for tx, tn in enumerate(line):
                    for y in range(8):
                        for x in range(8):
                            v = tiles[tn][y][x]
                            if v is None:
                                continue
                            x0, y0 = ox + (tx * 8 + x) * scale, oy + (ty * 8 + y) * scale
                            img.paste(rgb[v], (x0, y0, x0 + scale, y0 + scale))
    img.save(path)


def main():
    sys.path.insert(0, HERE)
    import styles as style_defs
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument('--only', action='append', help='only this style (by name); can repeat')
    ap.add_argument('--preview', help='also write a preview sheet to this PNG')
    a = ap.parse_args()
    todo = [s for s in style_defs.STYLES if not a.only or s['name'] in a.only]
    os.makedirs(OUT, exist_ok=True)
    for st in todo:
        path = os.path.join(OUT, st['name'] + '.png')
        write_png(st, path)
        print(os.path.relpath(path, ROOT))
    if a.preview:
        preview(todo, a.preview)
        print(a.preview)


if __name__ == '__main__':
    main()
