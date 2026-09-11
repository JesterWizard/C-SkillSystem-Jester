#!/usr/bin/python3
# -*- coding: UTF-8 -*-
"""Slice the native-size Mother 3 sheet into FE8 16x16 2bpp glyphs.

The sheet is already GBA-scale (217x75). Copy ink 1:1 into 16x16 cells.
Do not resize. Ink is black (0,0,0) on teal (50,78,91).

Layout is three 14px-pitch groups of five columns. Caps A-E, F-J, K-O,
P-T, U-Y, then Z with brackets. Lowercase and digits sit in the other
two groups. A filled blob and a hollow oval on the sheet are not ASCII;
vanilla fe8u fills missing punctuation.
"""

import os
from PIL import Image

SHEET = os.path.join(os.path.dirname(__file__), '..', 'Glyph', 'Mother3', 'sheet.png')
OUT_DIR = os.path.join(os.path.dirname(__file__), '..', 'Glyph', 'Mother3')

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
SPACE_WIDTH = 3

INK_RGB = {
    (0, 0, 0),
}

# (char, x0, y0, x1, y1, sheet_baseline) with inclusive box coords.
# Each row of the sheet shares a baseline so caps, x-height, and
# descenders line up in the 16x16 cell.
GLYPHS = [
    # Row 0: A-E, a-e, 0-4. Baseline 7.
    ('A', 0, 0, 5, 7, 7),
    ('B', 14, 0, 18, 7, 7),
    ('C', 28, 0, 32, 7, 7),
    ('D', 42, 0, 46, 7, 7),
    ('E', 56, 0, 59, 7, 7),
    ('a', 76, 2, 80, 7, 7),
    ('b', 90, 0, 93, 7, 7),
    ('c', 104, 2, 107, 7, 7),
    ('d', 118, 0, 121, 7, 7),
    ('e', 132, 2, 135, 7, 7),
    ('0', 152, 0, 155, 7, 7),
    ('1', 167, 0, 168, 7, 7),
    ('2', 180, 0, 183, 7, 7),
    ('3', 194, 0, 197, 7, 7),
    ('4', 207, 0, 211, 7, 7),
    # Row 1: F-J, f-j, 5-9. Baseline 20 (g/j descenders to 23).
    ('F', 0, 13, 3, 20, 20),
    ('G', 14, 13, 18, 20, 20),
    ('H', 28, 13, 32, 20, 20),
    ('I', 44, 13, 44, 20, 20),
    ('J', 56, 13, 59, 20, 20),
    ('f', 76, 13, 78, 20, 20),
    ('g', 90, 15, 93, 23, 20),
    ('h', 104, 13, 107, 20, 20),
    ('i', 119, 13, 119, 20, 20),
    ('j', 132, 13, 133, 23, 20),
    ('5', 152, 13, 155, 20, 20),
    ('6', 166, 13, 169, 20, 20),
    ('7', 180, 13, 183, 20, 20),
    ('8', 194, 13, 197, 20, 20),
    ('9', 208, 13, 211, 20, 20),
    # Row 2: K-O, k-o, * ~ " .
    ('K', 0, 26, 4, 33, 33),
    ('L', 14, 26, 17, 33, 33),
    ('M', 28, 26, 34, 33, 33),
    ('N', 42, 26, 46, 33, 33),
    ('O', 56, 26, 60, 33, 33),
    ('k', 76, 26, 79, 33, 33),
    ('l', 91, 26, 91, 33, 33),
    ('m', 104, 28, 110, 33, 33),
    ('n', 118, 28, 121, 33, 33),
    ('o', 132, 28, 135, 33, 33),
    ('*', 152, 26, 154, 28, 33),
    ('~', 166, 30, 171, 31, 33),
    ('"', 180, 26, 182, 27, 33),
    # Period sits on the letter baseline, not this row's cap line.
    ('.', 208, 29, 209, 30, 30),
    # Row 3: P-T, p-t, ' = /
    ('P', 0, 39, 4, 46, 46),
    ('Q', 14, 39, 18, 46, 46),
    ('R', 28, 39, 32, 46, 46),
    ('S', 42, 39, 46, 46, 46),
    ('T', 56, 39, 60, 46, 46),
    ('p', 76, 41, 79, 49, 46),
    ('q', 90, 41, 93, 49, 46),
    ('r', 104, 41, 106, 46, 46),
    ('s', 118, 41, 121, 46, 46),
    ('t', 132, 39, 134, 46, 46),
    ("'", 180, 39, 181, 41, 46),
    ('=', 194, 43, 198, 45, 46),
    ('/', 208, 39, 211, 46, 46),
    # Row 4: U-Y, u-y, + - $ %
    ('U', 0, 52, 4, 59, 59),
    ('V', 14, 52, 19, 59, 59),
    ('W', 28, 52, 34, 59, 59),
    ('X', 42, 52, 46, 59, 59),
    ('Y', 56, 52, 60, 59, 59),
    ('u', 76, 54, 79, 59, 59),
    ('v', 90, 54, 94, 59, 59),
    ('w', 104, 54, 110, 59, 59),
    ('x', 118, 54, 121, 59, 59),
    ('y', 132, 54, 135, 62, 59),
    ('+', 152, 54, 156, 58, 59),
    ('-', 166, 56, 168, 56, 59),
    ('$', 194, 52, 198, 60, 59),
    ('%', 208, 53, 216, 59, 59),
    # Row 5: Z < > [ ], z ! ? , : ; &
    ('Z', 0, 65, 3, 72, 72),
    ('<', 15, 64, 17, 74, 72),
    ('>', 29, 64, 31, 74, 72),
    ('[', 43, 64, 44, 74, 72),
    (']', 57, 64, 58, 74, 72),
    ('z', 76, 67, 79, 72, 72),
    ('!', 92, 65, 92, 72, 72),
    ('?', 104, 65, 107, 72, 72),
    (',', 132, 72, 133, 74, 72),
    (':', 167, 68, 167, 72, 72),
    (';', 180, 68, 181, 74, 72),
    ('&', 194, 65, 200, 72, 72),
]


def is_ink(pixel):
    return pixel[:3] in INK_RGB


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
