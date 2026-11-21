#!/usr/bin/env python3
"""Draw the Growth Radar chart's static grid: Img_GrowthRadarGrid.png (96x96, 4bpp indices of GrowthRadar.c's palette).

    python3 Kernel/Wizardry/GrowthRadar/Gfx/make-grid.py

The page copies this image (built to .4bpp) under the two layers every frame, so only the layers are drawn at run
time. The geometry is GrowthRadar.c's, with the same integer maths: centre 47.5 pixels (Q8), radius 44 pixels, axes
clockwise from the top with 45-degree steps (Q8 181 = 256 / sqrt 2); rings at 25%, 50% and 75% and the spokes in
colour 1, the 100% ring in colour 2. Run it again after changing GR_RADIUS or GR_CENTER there.
"""
import os

from PIL import Image

SIZE = 96
RADIUS = 44
CENTER = (47 << 8) + 0x80
DIR_X = [0, 181, 256, 181, 0, -181, -256, -181]
DIR_Y = [-256, -181, 0, 181, 256, 181, 0, -181]
COL_GRID, COL_RING = 1, 2

# GrowthRadar.c's sGrowthRadarPal, as 8-bit RGB (the build reads only the indices)
PAL5 = [(0, 0, 0), (9, 10, 15), (18, 19, 24), (5, 13, 6), (9, 18, 10), (5, 8, 18), (9, 12, 23), (4, 12, 14),
        (8, 17, 19), (21, 30, 18), (18, 25, 31)]


def vertices(share):
    r = (RADIUS * share * 0x100) >> 8
    return [(CENTER + ((DIR_X[k] * r) >> 8), CENTER + ((DIR_Y[k] * r) >> 8)) for k in range(8)]


def line(px, x0, y0, x1, y1, color):
    dx, sx = abs(x1 - x0), 1 if x0 < x1 else -1
    dy, sy = -abs(y1 - y0), 1 if y0 < y1 else -1
    err = dx + dy
    while True:
        px[x0, y0] = color
        if x0 == x1 and y0 == y1:
            break
        e2 = 2 * err
        if e2 >= dy:
            err += dy
            x0 += sx
        if e2 <= dx:
            err += dx
            y0 += sy


def outline(px, vs, color):
    for k in range(8):
        (ax, ay), (bx, by) = vs[k], vs[(k + 1) & 7]
        line(px, ax >> 8, ay >> 8, bx >> 8, by >> 8, color)


def main():
    im = Image.new('P', (SIZE, SIZE), 0)
    pal = []
    for r, g, b in PAL5 + [(0, 0, 0)] * (16 - len(PAL5)):
        pal += [r * 8, g * 8, b * 8]
    im.putpalette(pal + [0] * (768 - len(pal)))
    px = im.load()
    for ring in (1, 2, 3):
        outline(px, vertices(ring * 64), COL_GRID)
    full = vertices(0x100)
    for vx, vy in full:
        line(px, CENTER >> 8, CENTER >> 8, vx >> 8, vy >> 8, COL_GRID)
    outline(px, full, COL_RING)
    im.save(os.path.join(os.path.dirname(os.path.abspath(__file__)), 'Img_GrowthRadarGrid.png'))


if __name__ == '__main__':
    main()
