#!/usr/bin/python3
# -*- coding: UTF-8 -*-
"""Slice the native-size Riviera: The Promised Land sheet into FE8 16x16 glyphs.

The sheet is GBA-scale (441x34): 40 columns by 3 rows of 11px viewer cells
holding 8px tiles. Copy ink 1:1 into 16x16 cells. Do not resize.

Layout is digits 0-9 and A-J, then K-Z plus a-x, then y z and punctuation.
The two-exclamation cell is not installed as ASCII quotes. `"` and `,`
fall back to vanilla fe8u. `.` and `…` come from the sheet, then shift
down 1px from the letter baseline.
"""

import os
from PIL import Image

SHEET = os.path.join(os.path.dirname(__file__), '..', 'Glyph', 'Riviera', 'sheet.png')
OUT_DIR = os.path.join(os.path.dirname(__file__), '..', 'Glyph', 'Riviera')

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
# Extra blit offset (dx, dy) after LEFT_PAD / baseline.
GLYPH_SHIFT = {
    '.': (0, 1),
    '…': (0, 1),
}

# (char, x0, y0, x1, y1, sheet_baseline) with inclusive box coords.
GLYPHS = [
    # Row 0: 0-9 A-J. Baseline 8.
    ('0', 220, 2, 223, 8, 8),
    ('1', 232, 2, 233, 8, 8),
    ('2', 242, 2, 245, 8, 8),
    ('3', 253, 2, 256, 8, 8),
    ('4', 264, 2, 267, 8, 8),
    ('5', 275, 2, 278, 8, 8),
    ('6', 286, 2, 289, 8, 8),
    ('7', 297, 2, 300, 8, 8),
    ('8', 308, 2, 311, 8, 8),
    ('9', 319, 2, 322, 8, 8),
    ('A', 330, 2, 333, 8, 8),
    ('B', 341, 2, 344, 8, 8),
    ('C', 352, 2, 355, 8, 8),
    ('D', 363, 2, 366, 8, 8),
    ('E', 374, 2, 377, 8, 8),
    ('F', 385, 2, 388, 8, 8),
    ('G', 396, 2, 399, 8, 8),
    ('H', 407, 2, 410, 8, 8),
    ('I', 418, 2, 420, 8, 8),
    ('J', 429, 2, 432, 8, 8),
    # Row 1: K-Z a-x. Baseline 19 (g/j/p/q descenders to 20).
    ('K', 0, 13, 3, 19, 19),
    ('L', 11, 13, 14, 19, 19),
    ('M', 22, 13, 26, 19, 19),
    ('N', 33, 13, 36, 19, 19),
    ('O', 44, 13, 47, 19, 19),
    ('P', 55, 13, 58, 19, 19),
    ('Q', 66, 13, 70, 19, 19),
    ('R', 77, 13, 80, 19, 19),
    ('S', 88, 13, 91, 19, 19),
    ('T', 99, 13, 103, 19, 19),
    ('U', 110, 13, 113, 19, 19),
    ('V', 121, 13, 125, 19, 19),
    ('W', 132, 13, 136, 19, 19),
    ('X', 143, 13, 147, 19, 19),
    ('Y', 154, 13, 158, 19, 19),
    ('Z', 165, 13, 168, 19, 19),
    ('a', 176, 15, 180, 19, 19),
    ('b', 187, 13, 190, 19, 19),
    ('c', 198, 15, 201, 19, 19),
    ('d', 209, 13, 212, 19, 19),
    ('e', 220, 15, 223, 19, 19),
    ('f', 231, 13, 234, 19, 19),
    ('g', 242, 15, 245, 20, 19),
    ('h', 253, 13, 256, 19, 19),
    ('i', 264, 13, 264, 19, 19),
    ('j', 275, 13, 277, 20, 19),
    ('k', 286, 13, 289, 19, 19),
    ('l', 297, 13, 297, 19, 19),
    ('m', 308, 15, 312, 19, 19),
    ('n', 319, 15, 322, 19, 19),
    ('o', 330, 15, 333, 19, 19),
    ('p', 341, 15, 344, 20, 19),
    ('q', 352, 15, 355, 20, 19),
    ('r', 363, 15, 366, 19, 19),
    ('s', 374, 15, 377, 19, 19),
    ('t', 385, 13, 388, 19, 19),
    ('u', 396, 15, 399, 19, 19),
    ('v', 407, 15, 411, 19, 19),
    ('w', 418, 15, 422, 19, 19),
    ('x', 429, 15, 433, 19, 19),
    # Row 2: y z and punctuation. Baseline 30.
    ('y', 0, 26, 3, 31, 30),
    ('z', 11, 26, 14, 30, 30),
    ('-', 25, 27, 26, 27, 30),
    ("'", 99, 24, 99, 25, 30),
    ('!', 111, 24, 112, 30, 30),
    ('?', 121, 24, 124, 30, 30),
    (':', 134, 25, 134, 29, 30),
    ('/', 143, 23, 151, 31, 30),
    ('%', 154, 24, 162, 30, 30),
    ('+', 165, 25, 169, 29, 30),
    ('.', 134, 29, 134, 29, 30),
    ('…', 276, 26, 282, 27, 30),
    (']', 243, 23, 245, 31, 30),
    ('(', 253, 23, 255, 31, 30),
    (')', 264, 23, 266, 31, 30),
]


def is_ink(pixel):
    return pixel[0] < INK_THRESH


def raster_text(sheet, x0, y0, x1, y1, baseline, shift=(0, 0)):
    src = sheet.load()
    img = Image.new('P', (16, 16), 0)
    img.putpalette(PALETTE_TEXT)
    dst = img.load()
    max_x = 0
    found = False
    sx, sy = shift
    for y in range(y0, y1 + 1):
        for x in range(x0, x1 + 1):
            if not is_ink(src[x, y]):
                continue
            dx = LEFT_PAD + (x - x0) + sx
            dy = DEST_BASELINE + (y - baseline) + sy
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
        text_img, width = raster_text(im, x0, y0, x1, y1, baseline, GLYPH_SHIFT.get(ch, (0, 0)))
        hex_id = '{:02X}'.format(ord(ch))
        save_glyph(text_img, to_item(text_img), hex_id)
        entries.append((ch, width, hex_id))
        seen.add(ch)

    entries.sort(key=lambda e: ord(e[0]))

    list_path = os.path.join(out_dir, 'font.fontall.txt')
    with open(list_path, 'w', encoding='utf-8') as f:
        f.write('//char\ttype\tWidth\tFilename\n')
        for ch, width, hex_id in entries:
            f.write('{}\titem\t{}\tFontItem_{}.png\n'.format(ch, width, hex_id))
        for ch, width, hex_id in entries:
            f.write('{}\ttext\t{}\tFontText_{}.png\n'.format(ch, width, hex_id))

    a_width = next(w for ch, w, _ in entries if ch == 'A')
    print('wrote {} glyphs to {} (A advance {})'.format(len(entries), out_dir, a_width))


if __name__ == '__main__':
    main()
