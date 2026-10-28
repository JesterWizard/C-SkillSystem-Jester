#!/usr/bin/env python3
"""
Render hand-drawn skill icon glyphs into 16x16 skill icons.

    python3 render.py OUTDIR SHEET.png glyphs_a.txt [glyphs_b.txt ...]

Glyph file format (blank lines between entries, '#' comment lines ignored):

    @ SkillName BG
    ............
    ...12 rows of 12 chars...

BG is the background colour code (R B T C Y N D S V O G). Each glyph row is
up to 12 characters (shorter rows are padded with '.'), drawn at icon pixels
x 1..12, y 1..12. '.' is empty. Letters are palette colours:
    W white  L light grey  G grey  K outline  Y yellow  R red  B blue
    S slate  V lavender  T teal  C light teal  N brown  A tan  D dark brown
    O orange
A 1px K outline is added around the drawn cells (8 neighbours), inside the
icon's interior only. The border is the standard skill icon border
(border.txt): pixels outside its interior can't be drawn on.

Checks: every icon must differ from every other output and from every PNG
in --existing (and from their mirror images); pairs that differ in fewer
than 20 pixels are reported as near-duplicates.
"""
import os, sys, glob
from PIL import Image, ImageDraw

HERE = os.path.dirname(os.path.abspath(__file__))
PAL = [(192,248,200),(248,248,248),(200,192,184),(144,144,128),(40,56,32),(216,208,32),(160,8,8),(56,80,240),
       (112,120,144),(176,176,208),(40,128,96),(104,200,184),(112,80,48),(152,128,112),(80,56,64),(192,96,0)]
LET = {'W':1,'L':2,'G':3,'K':4,'Y':5,'R':6,'B':7,'S':8,'V':9,'T':10,'C':11,'N':12,'A':13,'D':14,'O':15}
BG = {'R':(6,14),'B':(7,8),'T':(10,11),'C':(11,10),'Y':(5,15),'N':(12,14),'D':(14,12),'S':(8,9),'V':(9,8),'O':(15,12),'G':(3,8)}
EXISTING = os.path.normpath(os.path.join(HERE, '..', '..', 'Sources', 'SkillIcon'))

border = [l.rstrip('\n') for l in open(os.path.join(HERE, 'border.txt'))][:16]
interior = {(x, y) for y in range(16) for x in range(16) if border[y][x] == '.'}


def parse(path):
    out, cur = [], None
    for n, l in enumerate(open(path), 1):
        l = l.rstrip('\n')
        if l.startswith('#'):
            continue
        if l.startswith('@'):
            f = l[1:].split()
            if len(f) != 2 or f[1] not in BG:
                sys.exit('%s:%d: want "@ Name BG"' % (path, n))
            cur = [f[0], f[1], [], path, n]
            out.append(cur)
        elif l.strip() and cur is not None:
            if len(l) > 12 or any(c not in LET and c != '.' for c in l):
                sys.exit('%s:%d: bad glyph row %r' % (path, n, l))
            cur[2].append(l.ljust(12, '.'))
    for name, bg, rows, p, n in out:
        if len(rows) != 12:
            sys.exit('%s:%d: %s has %d rows, want 12' % (p, n, name, len(rows)))
    return out


def render(bg, rows):
    main, shade = BG[bg]
    g = [[int(border[y][x], 16) if (x, y) not in interior else (shade if x + y >= 23 else main)
          for x in range(16)] for y in range(16)]
    cells = {(x + 1, y + 1): LET[c] for y, r in enumerate(rows) for x, c in enumerate(r) if c != '.'}
    for (x, y) in cells:
        for dx in (-1, 0, 1):
            for dy in (-1, 0, 1):
                p = (x + dx, y + dy)
                if p not in cells and p in interior:
                    g[p[1]][p[0]] = 4
    for (x, y), c in cells.items():
        if (x, y) in interior:
            g[y][x] = c
    return g


def save(g, path):
    im = Image.new('P', (16, 16))
    im.putpalette([c for p in PAL for c in p] + [0] * (768 - 48))
    im.putdata([v for r in g for v in r])
    im.save(path)


def snap(im):
    im = im.convert('RGB')
    return tuple(min(range(16), key=lambda i: sum((a - b) ** 2 for a, b in zip(c, PAL[i]))) for c in im.getdata())


def mirror(t):
    return tuple(t[y * 16 + 15 - x] for y in range(16) for x in range(16))


def main():
    outdir, sheet = sys.argv[1], sys.argv[2]
    files = sys.argv[3:]
    os.makedirs(outdir, exist_ok=True)
    entries = []
    for f in files:
        entries += parse(f)
    names = [e[0] for e in entries]
    dup = {n for n in names if names.count(n) > 1}
    if dup:
        sys.exit('duplicate names: %s' % ' '.join(sorted(dup)))
    grids = {}
    for name, bg, rows, p, n in entries:
        g = render(bg, rows)
        save(g, os.path.join(outdir, name + '.png'))
        grids[name] = tuple(v for r in g for v in r)
    known = {}
    for p in glob.glob(os.path.join(EXISTING, '*.png')):
        n = os.path.basename(p)[:-4]
        if n in grids:
            continue
        im = Image.open(p)
        if im.size == (16, 16):
            known['existing:' + n] = snap(im)
    warns = 0
    allg = list(known.items()) + list(grids.items())
    for name, t in grids.items():
        for other, u in allg:
            if other == name:
                continue
            if not other.startswith('existing:') and other < name:
                continue
            for tag, uu in (('', u), (' (mirrored)', mirror(u))):
                d = sum(1 for a, b in zip(t, uu) if a != b)
                if d == 0:
                    print('DUPLICATE: %s == %s%s' % (name, other, tag)); warns += 1
                elif d < 20:
                    print('NEAR: %s ~ %s%s (%d px differ)' % (name, other, tag, d)); warns += 1
    print('rendered %d icons, %d warnings' % (len(grids), warns))
    cols, cw, ch = 10, 120, 92
    rowsn = (len(entries) + cols - 1) // cols
    im = Image.new('RGB', (cols * cw, rowsn * ch), (45, 45, 45))
    d = ImageDraw.Draw(im)
    for i, (name, bg, rows, p, n) in enumerate(entries):
        ic = Image.open(os.path.join(outdir, name + '.png')).convert('RGB').resize((64, 64), Image.NEAREST)
        x, y = (i % cols) * cw, (i // cols) * ch
        im.paste(ic, (x + 28, y + 4))
        d.text((x + 3, y + 72), name[:19], fill=(255, 255, 255))
    im.save(sheet)


if __name__ == '__main__':
    main()
