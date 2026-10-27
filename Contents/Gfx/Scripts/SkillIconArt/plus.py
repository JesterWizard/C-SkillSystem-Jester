#!/usr/bin/env python3
"""
Make the "...Plus" icons: the base skill's icon with the gold border and red
plus badge that the repo's hand-made Plus icons use (plus_template.txt is
the part all 25 of them share; positions they don't share come from
CantoPlus.png).

    python3 plus.py OUTDIR plus.txt BASEDIR [BASEDIR ...]

plus.txt: "PlusName BaseName" per line. The base is looked up in the
BASEDIRs in order (new art first, then the repo's icons).
"""
import os, sys
from PIL import Image
from render import PAL, save, snap, border

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.normpath(os.path.join(HERE, '..', '..', 'Sources', 'SkillIcon'))
tpl = [l.rstrip('\n') for l in open(os.path.join(HERE, 'plus_template.txt'))][:16]
canto = snap(Image.open(os.path.join(REPO, 'CantoPlus.png')))


def main():
    outdir, listfile, bases = sys.argv[1], sys.argv[2], sys.argv[3:]
    os.makedirs(outdir, exist_ok=True)
    n = 0
    for l in open(listfile):
        if not l.strip():
            continue
        name, base = l.split()
        src = next((os.path.join(d, base + '.png') for d in bases if os.path.exists(os.path.join(d, base + '.png'))), None)
        if not src:
            sys.exit('no base icon for %s (%s)' % (name, base))
        t = snap(Image.open(src))
        g = [[t[y * 16 + x] for x in range(16)] for y in range(16)]
        for y in range(16):
            for x in range(16):
                c = tpl[y][x]
                if c != '.':
                    g[y][x] = int(c, 16)
                elif border[y][x] != '.':
                    g[y][x] = canto[y * 16 + x]
        save(g, os.path.join(outdir, name + '.png'))
        n += 1
    print('made %d plus icons' % n)


if __name__ == '__main__':
    main()
