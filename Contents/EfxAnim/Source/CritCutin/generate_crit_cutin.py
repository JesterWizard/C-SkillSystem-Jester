#!/usr/bin/env python3
"""
Generate the critical cut-in band frames (240x160, 16 colours, colour 0 clear).

The band is 64px tall across the middle of the screen (y 40-103, clear of
the battle UI panels, which start at y 108): a dark navy strip with
crackling lightning and speed lines. It stays within the 256 tiles a battle
spell BG frame can hold (30x8 band tiles + one blank).

    python3 generate_crit_cutin.py

Writes CritCutin_*.png next to this script. CritCutin.efx.txt lists them.
"""
import os
import random

from PIL import Image, ImageDraw

OUT_DIR = os.path.dirname(os.path.abspath(__file__))

W, H = 240, 160
BAND_Y = 40
BAND_H = 64

PALETTE = [
    (0, 0, 0),          # 0 clear
    (8, 8, 32),         # 1 darkest navy
    (16, 24, 64),       # 2 navy
    (24, 48, 112),      # 3 deep blue
    (40, 96, 216),      # 4 electric blue (glow)
    (88, 160, 248),     # 5 light blue
    (120, 232, 248),    # 6 cyan
    (200, 248, 248),    # 7 pale cyan
    (248, 248, 248),    # 8 white
    (112, 64, 208),     # 9 violet glow
    (184, 144, 248),    # 10 light violet
] + [(0, 0, 0)] * 5

DARKEST, NAVY, DEEP, GLOW, LIGHT, CYAN, PALE, WHITE, VIOLET, LILAC = range(1, 11)


def new_frame():
    img = Image.new('P', (W, H), 0)
    flat = []
    for rgb in PALETTE:
        flat.extend(rgb)
    img.putpalette(flat)
    return img


def band_rect(height):
    top = BAND_Y + (BAND_H - height) // 2
    return top, top + height - 1


def draw_base(draw, rng, top, bottom):
    """Dark strip, lighter towards the edges, with faint horizontal grain."""
    mid = (top + bottom) / 2
    half = max(1, (bottom - top) / 2)
    for y in range(top, bottom + 1):
        edge = abs(y - mid) / half
        colour = DARKEST if edge < 0.45 else NAVY if edge < 0.8 else DEEP
        draw.line([(0, y), (W - 1, y)], fill=colour)

    for _ in range(18):
        y = rng.randint(top + 2, bottom - 2)
        x = rng.randint(-40, W)
        length = rng.randint(20, 90)
        draw.line([(x, y), (x + length, y)], fill=rng.choice([NAVY, DEEP]))


def draw_speed_lines(draw, rng, top, bottom, count):
    for _ in range(count):
        y = rng.randint(top + 3, bottom - 3)
        x = rng.randint(-60, W)
        length = rng.randint(24, 120)
        draw.line([(x, y), (x + length, y)], fill=rng.choice([DEEP, GLOW, GLOW, LIGHT]))


def bolt_points(rng, x0, x1, top, bottom, step=(6, 12), wander=7):
    """A jagged path from x0 to x1 that stays inside the band."""
    points = []
    y = rng.randint(top + 6, bottom - 6)
    x = x0
    direction = 1 if x1 >= x0 else -1
    while (x - x1) * direction < 0:
        points.append((x, y))
        x += direction * rng.randint(*step)
        y = max(top + 3, min(bottom - 3, y + rng.randint(-wander, wander)))
    points.append((x1, y))
    return points


def draw_bolt(draw, points, glow, core, centre):
    draw.line(points, fill=glow, width=5)
    draw.line(points, fill=core, width=3)
    draw.line(points, fill=centre, width=1)


def draw_branches(draw, rng, points, top, bottom, glow, core):
    for _ in range(rng.randint(2, 4)):
        x, y = rng.choice(points[1:-1] or points)
        sign = rng.choice([-1, 1])
        branch = [(x, y)]
        for _ in range(rng.randint(2, 4)):
            x += rng.randint(-8, 8)
            y = max(top + 2, min(bottom - 2, y + sign * rng.randint(4, 9)))
            branch.append((x, y))
        draw.line(branch, fill=glow, width=3)
        draw.line(branch, fill=core, width=1)


def draw_arc(draw, rng, top, bottom):
    """A short vertical crackle from one band edge to the other."""
    x = rng.randint(10, W - 10)
    points = []
    for y in range(top, bottom + 1, rng.randint(5, 8)):
        points.append((x + rng.randint(-5, 5), y))
    points.append((x + rng.randint(-5, 5), bottom))
    draw.line(points, fill=GLOW, width=3)
    draw.line(points, fill=PALE, width=1)


def draw_edges(draw, top, bottom):
    draw.line([(0, top), (W - 1, top)], fill=WHITE)
    draw.line([(0, top + 1), (W - 1, top + 1)], fill=CYAN)
    draw.line([(0, top + 2), (W - 1, top + 2)], fill=GLOW)
    draw.line([(0, bottom), (W - 1, bottom)], fill=WHITE)
    draw.line([(0, bottom - 1), (W - 1, bottom - 1)], fill=CYAN)
    draw.line([(0, bottom - 2), (W - 1, bottom - 2)], fill=GLOW)


def lightning_frame(seed, height=BAND_H, bolts=3):
    rng = random.Random(seed)
    img = new_frame()
    draw = ImageDraw.Draw(img)
    top, bottom = band_rect(height)

    draw_base(draw, rng, top, bottom)
    draw_speed_lines(draw, rng, top, bottom, 10)

    if height >= 32:
        for _ in range(rng.randint(1, 2)):
            draw_arc(draw, rng, top + 3, bottom - 3)

    for i in range(bolts):
        violet = i == bolts - 1 and rng.random() < 0.6
        glow, core = (VIOLET, LILAC) if violet else (GLOW, CYAN)
        x0, x1 = (-8, W + 8) if rng.random() < 0.5 else (W + 8, -8)
        points = bolt_points(rng, x0, x1, top, bottom)
        draw_bolt(draw, points, glow, core, WHITE if i == 0 else PALE)
        if height >= 32:
            draw_branches(draw, rng, points, top, bottom, glow, core)

    draw_edges(draw, top, bottom)
    return img


def flash_frame(height):
    img = new_frame()
    draw = ImageDraw.Draw(img)
    top, bottom = band_rect(height)
    draw.rectangle([(0, top), (W - 1, bottom)], fill=WHITE)
    draw.line([(0, top), (W - 1, top)], fill=PALE)
    draw.line([(0, bottom), (W - 1, bottom)], fill=PALE)
    return img


def main():
    frames = {
        'CritCutin_open0': flash_frame(8),
        'CritCutin_open1': lightning_frame(101, height=32, bolts=1),
        'CritCutin_open2': lightning_frame(102, height=56, bolts=2),
        'CritCutin_close0': lightning_frame(201, height=32, bolts=1),
        'CritCutin_close1': flash_frame(8),
    }
    for i in range(6):
        frames['CritCutin_loop{0}'.format(i)] = lightning_frame(1 + i)

    for name, img in frames.items():
        img.save(os.path.join(OUT_DIR, name + '.png'), optimize=True)
        print('wrote', name + '.png')


if __name__ == '__main__':
    main()
