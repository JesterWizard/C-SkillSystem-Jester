#!/usr/bin/python3
# -*- coding: UTF-8 -*-
"""Slice the native-size Red Rescue Team sheet into FE8 16x16 2bpp glyphs.

The sheet is already GBA-scale (176x75): packed Latin rows, white ink on
gray. Copy 1:1 into 16x16 cells. Do not resize.

Layout is a-m, n-z, A-M, N-Z, then 1-9 0 with colon/ellipsis, then
punctuation, then parentheses. A 6-like cell before `1` is not ASCII.
Missing punctuation falls back to vanilla fe8u.
"""

import os
from PIL import Image

SHEET = os.path.join(os.path.dirname(__file__), '..', 'Glyph', 'RedRescueTeam', 'sheet.png')
OUT_DIR = os.path.join(os.path.dirname(__file__), '..', 'Glyph', 'RedRescueTeam')

PALETTE_TEXT = [
    224, 224, 224,
    168, 168, 167,
    248, 248, 248,
    40, 40, 40,
] + [0, 0, 0] * (256 - 4)

PALETTE_ITEM = [
    104, 136, 168,
    168, 168, 167,
    248, 248, 248,
    40, 40, 40,
] + [0, 0, 0] * (256 - 4)

OUTLINE_DIRS = ((-1, 0), (1, 0), (0, -1), (0, 1))

LEFT_PAD = 1
DEST_BASELINE = 11
INK_THRESH = 200
SPACE_WIDTH = 3

# (char, x0, y0, x1, y1, sheet_baseline) with inclusive box coords.
GLYPHS = [
    # Row 0: a-m. Baseline 7 (g/j descenders to 8).
    ('a', 0, 2, 3, 7, 7),
    ('b', 14, 0, 17, 7, 7),
    ('c', 28, 2, 31, 7, 7),
    ('d', 42, 0, 45, 7, 7),
    ('e', 56, 2, 59, 7, 7),
    ('f', 70, 0, 73, 7, 7),
    ('g', 84, 2, 87, 8, 7),
    ('h', 98, 0, 101, 7, 7),
    ('i', 113, 0, 114, 7, 7),
    ('j', 127, 0, 128, 8, 7),
    ('k', 140, 0, 143, 7, 7),
    ('l', 155, 0, 156, 7, 7),
    ('m', 167, 2, 173, 7, 7),
    # Row 1: n-z. Baseline 18 (p/q/y descenders to 19).
    ('n', 0, 13, 3, 18, 18),
    ('o', 14, 13, 17, 18, 18),
    ('p', 28, 13, 31, 19, 18),
    ('q', 42, 13, 45, 19, 18),
    ('r', 56, 13, 59, 18, 18),
    ('s', 70, 13, 73, 18, 18),
    ('t', 85, 11, 87, 18, 18),
    ('u', 98, 13, 101, 18, 18),
    ('v', 112, 13, 116, 18, 18),
    ('w', 125, 13, 131, 18, 18),
    ('x', 140, 13, 144, 18, 18),
    ('y', 154, 13, 157, 19, 18),
    ('z', 168, 13, 171, 18, 18),
    # Row 2: A-M. Baseline 29.
    ('A', 0, 22, 4, 29, 29),
    ('B', 14, 22, 18, 29, 29),
    ('C', 28, 22, 32, 29, 29),
    ('D', 42, 22, 46, 29, 29),
    ('E', 56, 22, 59, 29, 29),
    ('F', 70, 22, 73, 29, 29),
    ('G', 84, 22, 88, 29, 29),
    ('H', 98, 22, 102, 29, 29),
    ('I', 113, 22, 115, 29, 29),
    ('J', 126, 22, 130, 29, 29),
    ('K', 140, 22, 144, 29, 29),
    ('L', 154, 22, 157, 29, 29),
    ('M', 167, 22, 173, 29, 29),
    # Row 3: N-Z. Baseline 40 (Q descender to 41).
    ('N', 0, 33, 4, 40, 40),
    ('O', 14, 33, 18, 40, 40),
    ('P', 28, 33, 32, 40, 40),
    ('Q', 42, 33, 46, 41, 40),
    ('R', 56, 33, 60, 40, 40),
    ('S', 70, 33, 74, 40, 40),
    ('T', 84, 33, 88, 40, 40),
    ('U', 98, 33, 102, 40, 40),
    ('V', 112, 33, 116, 40, 40),
    ('W', 124, 33, 132, 40, 40),
    ('X', 140, 33, 144, 40, 40),
    ('Y', 154, 33, 158, 40, 40),
    ('Z', 168, 33, 172, 40, 40),
    # Row 4: 1-9, 0, colon, ellipsis. Baseline 51.
    ('1', 15, 44, 16, 51, 51),
    ('2', 28, 44, 32, 51, 51),
    ('3', 42, 44, 46, 51, 51),
    ('4', 56, 44, 60, 51, 51),
    ('5', 70, 44, 74, 51, 51),
    ('6', 84, 44, 88, 51, 51),
    ('7', 98, 44, 102, 51, 51),
    ('8', 112, 44, 116, 51, 51),
    ('9', 126, 44, 130, 51, 51),
    ('0', 140, 44, 144, 51, 51),
    (':', 156, 47, 156, 51, 51),
    ('…', 166, 47, 173, 48, 48),
    # Row 5: punctuation. Baseline 62 (period / ! dot).
    ('+', 0, 57, 4, 61, 62),
    ('-', 14, 59, 17, 59, 62),
    (',', 29, 61, 30, 63, 62),
    ('.', 44, 62, 44, 62, 62),
    ('!', 57, 55, 59, 62, 62),
    ('?', 70, 55, 74, 62, 62),
    ('`', 86, 54, 87, 56, 62),
    ("'", 100, 54, 101, 56, 62),
    ('"', 112, 54, 116, 56, 62),
    ('♂', 139, 55, 145, 61, 62),
    ('♀', 154, 55, 158, 62, 62),
    ('_', 166, 61, 175, 63, 62),
    # Row 6: parentheses. Baseline 74.
    ('(', 2, 66, 4, 74, 74),
    (')', 15, 66, 17, 74, 74),
]


def is_ink(pixel):
    return pixel[0] >= INK_THRESH


def raster_text(sheet, x0, y0, x1, y1, baseline):
    src = sheet.load()
    img = Image.new('P', (16, 16), 0)
    img.putpalette(PALETTE_TEXT)
    dst = img.load()
    max_x = 0
    found = False
    for y in range(y0, y1 + 1):
        for x in range(x0, x1 + 1):
            if not is_ink(src[x, y]):
                continue
            dx = LEFT_PAD + (x - x0)
            dy = DEST_BASELINE + (y - baseline)
            if 0 <= dx < 16 and 0 <= dy < 16:
                dst[dx, dy] = 3
                max_x = max(max_x, dx)
                found = True
    if not found:
        return img, SPACE_WIDTH
    return img, min(16, max(2, max_x + 1))


def to_item(text_img):
    """White fill with a 4-connected dark outline, matching vanilla FontItem."""
    item = Image.new('P', (16, 16), 0)
    item.putpalette(PALETTE_ITEM)
    src = text_img.load()
    dst = item.load()
    whites = []
    for y in range(16):
        for x in range(16):
            if src[x, y] != 3:
                continue
            dst[x, y] = 2
            whites.append((x, y))
    for x, y in whites:
        for ox, oy in OUTLINE_DIRS:
            nx, ny = x + ox, y + oy
            if 0 <= nx < 16 and 0 <= ny < 16 and dst[nx, ny] == 0:
                dst[nx, ny] = 3
    return item


def save_glyph(text_img, item_img, hex_id):
    item_img.save(os.path.join(OUT_DIR, 'FontItem_{}.png'.format(hex_id)))
    text_img.save(os.path.join(OUT_DIR, 'FontText_{}.png'.format(hex_id)))


def hex_id_for(ch):
    return '{:02X}'.format(ord(ch))


def main():
    sheet_path = os.path.abspath(SHEET)
    out_dir = os.path.abspath(OUT_DIR)
    im = Image.open(sheet_path).convert('RGB')
    os.makedirs(out_dir, exist_ok=True)

    entries = []
    space_text = Image.new('P', (16, 16), 0)
    space_text.putpalette(PALETTE_TEXT)
    space_item = Image.new('P', (16, 16), 0)
    space_item.putpalette(PALETTE_ITEM)
    save_glyph(space_text, space_item, '20')
    entries.append((' ', SPACE_WIDTH, '20'))

    seen = set()
    for ch, x0, y0, x1, y1, baseline in GLYPHS:
        if ch in seen:
            raise SystemExit('duplicate glyph {}'.format(repr(ch)))
        text_img, width = raster_text(im, x0, y0, x1, y1, baseline)
        hid = hex_id_for(ch)
        save_glyph(text_img, to_item(text_img), hid)
        entries.append((ch, width, hid))
        seen.add(ch)

    entries.sort(key=lambda e: ord(e[0]))

    list_path = os.path.join(out_dir, 'font.fontall.txt')
    with open(list_path, 'w', encoding='utf-8') as f:
        f.write('//char\ttype\tWidth\tFilename\n')
        for ch, width, hid in entries:
            f.write('{}\titem\t{}\tFontItem_{}.png\n'.format(ch, width, hid))
        for ch, width, hid in entries:
            f.write('{}\ttext\t{}\tFontText_{}.png\n'.format(ch, width, hid))

    a_width = next(w for ch, w, _ in entries if ch == 'A')
    print('wrote {} glyphs to {} (A advance {})'.format(len(entries), out_dir, a_width))


if __name__ == '__main__':
    main()
