#!/usr/bin/python3
# -*- coding: UTF-8 -*-
"""Slice the native-size Advance Wars 2 sheet into FE8 16x16 2bpp glyphs.

The sheet is already GBA-scale (102x75). Copy ink 1:1 into 16x16 cells.
Do not resize.
"""

import os
from PIL import Image

SHEET = os.path.join(os.path.dirname(__file__), '..', 'Glyph', 'AdvanceWars2', 'sheet.png')
OUT_DIR = os.path.join(os.path.dirname(__file__), '..', 'Glyph', 'AdvanceWars2')

# Index 0 bg, 1 unused, 2 white, 3 dark. Item uses the menu-blue bg like fe8u.
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
INK_THRESH = 80
SPACE_WIDTH = 3

# (char, x0, y0, x1, y1, sheet_baseline) with inclusive box coords.
# Each row of the sheet shares a baseline so caps, x-height, and
# descenders line up in the 16x16 cell.
GLYPHS = [
    # Row 0: A-Q, y=1-9
    ('A', 1, 1, 5, 9, 9),
    ('B', 7, 1, 11, 9, 9),
    ('C', 13, 1, 17, 9, 9),
    ('D', 19, 1, 23, 9, 9),
    ('E', 25, 1, 29, 9, 9),
    ('F', 31, 1, 35, 9, 9),
    ('G', 37, 1, 41, 9, 9),
    ('H', 43, 1, 47, 9, 9),
    ('I', 49, 1, 49, 9, 9),
    ('J', 51, 1, 55, 9, 9),
    ('K', 57, 1, 61, 9, 9),
    ('L', 63, 1, 67, 9, 9),
    ('M', 69, 1, 75, 9, 9),
    ('N', 77, 1, 81, 9, 9),
    ('O', 83, 1, 87, 9, 9),
    ('P', 89, 1, 93, 9, 9),
    ('Q', 95, 1, 99, 9, 9),
    # Row 1: R-Z and a-i (g includes its descender)
    ('R', 1, 11, 5, 19, 19),
    ('S', 7, 11, 11, 19, 19),
    ('T', 13, 11, 17, 19, 19),
    ('U', 19, 11, 23, 19, 19),
    ('V', 25, 11, 29, 19, 19),
    ('W', 31, 11, 37, 19, 19),
    ('X', 39, 11, 43, 19, 19),
    ('Y', 45, 11, 49, 19, 19),
    ('Z', 51, 11, 55, 19, 19),
    ('a', 57, 15, 61, 19, 19),
    ('b', 63, 12, 66, 19, 19),
    ('c', 68, 15, 71, 19, 19),
    ('d', 73, 12, 77, 19, 19),
    ('e', 79, 15, 82, 19, 19),
    ('f', 84, 12, 87, 19, 19),
    ('g', 89, 15, 92, 21, 19),
    ('h', 94, 12, 97, 19, 19),
    ('i', 99, 13, 99, 19, 19),
    # Row 2: j-z
    ('j', 1, 22, 3, 30, 28),
    ('k', 5, 21, 8, 28, 28),
    ('l', 10, 21, 10, 28, 28),
    ('m', 12, 24, 16, 28, 28),
    ('n', 18, 24, 21, 28, 28),
    ('o', 23, 24, 26, 28, 28),
    ('p', 28, 24, 31, 30, 28),
    ('q', 33, 24, 36, 30, 28),
    ('r', 38, 24, 41, 28, 28),
    ('s', 43, 24, 46, 28, 28),
    ('t', 48, 21, 50, 28, 28),
    ('u', 52, 24, 55, 28, 28),
    ('v', 57, 24, 61, 28, 28),
    ('w', 63, 24, 67, 28, 28),
    ('x', 69, 24, 73, 28, 28),
    ('y', 75, 24, 78, 30, 28),
    ('z', 80, 24, 84, 28, 28),
    # Row 3: digits and quotes
    ('0', 1, 32, 6, 41, 41),
    ('1', 9, 32, 11, 41, 41),
    ('2', 14, 32, 19, 41, 41),
    ('3', 21, 32, 26, 41, 41),
    ('4', 28, 32, 33, 41, 41),
    ('5', 35, 32, 40, 41, 41),
    ('6', 42, 32, 47, 41, 41),
    ('7', 49, 32, 54, 41, 41),
    ('8', 56, 32, 61, 41, 41),
    ('9', 63, 32, 68, 41, 41),
    ('.', 70, 41, 70, 41, 41),
    (',', 72, 40, 73, 41, 41),
    ("'", 75, 33, 76, 35, 41),
    ('"', 81, 33, 85, 35, 41),
    ('`', 81, 33, 82, 35, 41),
    # Row 4: punctuation
    ('?', 1, 43, 5, 51, 51),
    ('!', 7, 43, 8, 51, 51),
    ('@', 13, 44, 21, 52, 51),
    ('_', 23, 52, 25, 52, 51),
    ('*', 27, 45, 31, 49, 51),
    ('#', 33, 45, 38, 50, 51),
    ('$', 40, 44, 44, 50, 51),
    ('%', 46, 44, 52, 51, 51),
    ('&', 54, 45, 60, 51, 51),
    ('(', 62, 44, 64, 52, 51),
    (')', 66, 44, 68, 52, 51),
    ('+', 70, 45, 74, 49, 51),
    ('-', 76, 47, 78, 47, 51),
    ('/', 80, 45, 86, 51, 51),
    (':', 88, 46, 88, 50, 51),
    (';', 90, 46, 91, 52, 51),
    # Row 5: more punctuation
    ('<', 3, 55, 6, 61, 62),
    ('=', 8, 56, 11, 58, 62),
    ('>', 13, 55, 16, 61, 62),
    ('[', 18, 54, 20, 62, 62),
    (']', 22, 54, 24, 62, 62),
    ('{', 26, 54, 29, 62, 62),
    ('|', 31, 54, 31, 62, 62),
    ('}', 33, 54, 36, 62, 62),
    ('~', 38, 56, 43, 57, 62),
    ('\\', 45, 55, 51, 61, 62),
    # Row 6: caret
    ('^', 45, 65, 47, 67, 73),
]


def is_ink(pixel):
    r = pixel[0]
    return r < INK_THRESH


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
    # 1px left pad plus 1px right bearing keeps letters tight without touching.
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
        hex_id = '{:02X}'.format(ord(ch))
        save_glyph(text_img, to_item(text_img), hex_id)
        entries.append((ch, width, hex_id))
        seen.add(ch)

    list_path = os.path.join(out_dir, 'font.fontall.txt')
    with open(list_path, 'w', encoding='utf-8') as f:
        f.write('//char\ttype\tWidth\tFilename\n')
        for ch, width, hex_id in entries:
            f.write('{}\titem\t{}\tFontItem_{}.png\n'.format(ch, width, hex_id))
        for ch, width, hex_id in entries:
            f.write('{}\ttext\t{}\tFontText_{}.png\n'.format(ch, width, hex_id))

    print('wrote {} glyphs to {}'.format(len(entries), out_dir))


if __name__ == '__main__':
    main()
