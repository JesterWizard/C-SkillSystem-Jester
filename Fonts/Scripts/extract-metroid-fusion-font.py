#!/usr/bin/python3
# -*- coding: UTF-8 -*-
"""Slice the native-size Metroid Fusion sheet into FE8 16x16 2bpp glyphs.

The sheet is already GBA-scale (150x150). Copy ink 1:1 into 16x16 cells.
Do not resize.
"""

import os
from PIL import Image

SHEET = os.path.join(os.path.dirname(__file__), '..', 'Glyph', 'MetroidFusion', 'sheet.png')
OUT_DIR = os.path.join(os.path.dirname(__file__), '..', 'Glyph', 'MetroidFusion')

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
DEST_BASELINE = 12
INK_THRESH = 80
SPACE_WIDTH = 4

# (char, x0, y0, x1, y1, sheet_baseline) with inclusive box coords.
# Each row of the sheet shares a baseline so caps, x-height, and
# descenders line up in the 16x16 cell.
GLYPHS = [
    # Row 0: A-R, y=23-31, baseline 31
    ('A', 3, 23, 9, 31, 31),
    ('B', 11, 23, 16, 31, 31),
    ('C', 19, 23, 24, 31, 31),
    ('D', 27, 23, 32, 31, 31),
    ('E', 35, 23, 40, 31, 31),
    ('F', 43, 23, 48, 31, 31),
    ('G', 51, 23, 56, 31, 31),
    ('H', 59, 23, 64, 31, 31),
    ('I', 67, 23, 69, 31, 31),
    ('J', 72, 23, 77, 31, 31),
    ('K', 80, 23, 85, 31, 31),
    ('L', 88, 23, 93, 31, 31),
    ('M', 96, 23, 102, 31, 31),
    ('N', 105, 23, 110, 31, 31),
    ('O', 113, 23, 118, 31, 31),
    ('P', 121, 23, 126, 31, 31),
    ('Q', 129, 23, 134, 31, 31),
    ('R', 137, 23, 142, 31, 31),
    # Row 1: S-Z and a-j (g/j include descenders)
    ('S', 3, 39, 8, 47, 47),
    ('T', 11, 39, 17, 47, 47),
    ('U', 19, 39, 24, 47, 47),
    ('V', 27, 39, 33, 47, 47),
    ('W', 36, 39, 42, 47, 47),
    ('X', 45, 39, 50, 47, 47),
    ('Y', 53, 39, 59, 47, 47),
    ('Z', 61, 39, 66, 47, 47),
    ('a', 69, 42, 74, 47, 47),
    ('b', 77, 39, 82, 47, 47),
    ('c', 85, 42, 90, 47, 47),
    ('d', 93, 39, 98, 47, 47),
    ('e', 101, 42, 106, 47, 47),
    ('f', 109, 39, 114, 47, 47),
    ('g', 117, 42, 122, 49, 47),
    ('h', 125, 39, 130, 47, 47),
    ('i', 134, 39, 134, 47, 47),
    ('j', 138, 39, 141, 49, 47),
    # Row 2: k-z
    ('k', 3, 55, 8, 63, 63),
    ('l', 12, 55, 12, 63, 63),
    ('m', 14, 58, 20, 63, 63),
    ('n', 23, 58, 28, 63, 63),
    ('o', 31, 58, 36, 63, 63),
    ('p', 39, 58, 44, 65, 63),
    ('q', 47, 58, 52, 65, 63),
    ('r', 55, 58, 60, 63, 63),
    ('s', 63, 58, 68, 63, 63),
    ('t', 71, 56, 76, 63, 63),
    ('u', 79, 58, 84, 63, 63),
    ('v', 87, 58, 92, 63, 63),
    ('w', 94, 58, 100, 63, 63),
    ('x', 103, 58, 108, 63, 63),
    ('y', 111, 58, 116, 65, 63),
    ('z', 119, 58, 123, 63, 63),
    # Row 3: digits and punctuation
    ('0', 3, 71, 7, 79, 79),
    ('1', 10, 71, 12, 79, 79),
    ('2', 15, 71, 19, 79, 79),
    ('3', 22, 71, 26, 79, 79),
    ('4', 29, 71, 33, 79, 79),
    ('5', 36, 71, 40, 79, 79),
    ('6', 43, 71, 47, 79, 79),
    ('7', 50, 71, 54, 79, 79),
    ('8', 57, 71, 61, 79, 79),
    ('9', 64, 71, 68, 79, 79),
    ('.', 71, 78, 72, 79, 79),
    (',', 75, 78, 76, 81, 79),
    ('"', 79, 69, 83, 72, 79),
    ("'", 93, 69, 94, 72, 79),
    ('`', 93, 69, 94, 72, 79),
    ('?', 97, 71, 102, 79, 79),
    ('!', 105, 71, 108, 79, 79),
    ('(', 120, 69, 122, 79, 79),
    (')', 125, 69, 127, 79, 79),
    ('-', 130, 75, 135, 75, 79),
    (':', 139, 73, 140, 78, 79),
    (';', 144, 73, 145, 80, 79),
    # Uppercase accented (9px bodies)
    ('Á', 17, 84, 23, 95, 95),
    ('À', 26, 84, 32, 95, 95),
    ('Â', 35, 83, 41, 95, 95),
    ('Ä', 43, 84, 49, 95, 95),
    ('Ç', 59, 87, 64, 98, 95),
    ('É', 67, 84, 72, 95, 95),
    ('È', 75, 84, 80, 95, 95),
    ('Ê', 83, 83, 88, 95, 95),
    ('Ë', 91, 84, 96, 95, 95),
    ('Í', 99, 84, 101, 95, 95),
    ('Ì', 104, 84, 106, 95, 95),
    ('Î', 108, 83, 112, 95, 95),
    ('Ï', 114, 84, 116, 95, 95),
    ('Ñ', 119, 84, 124, 95, 95),
    ('Ó', 127, 84, 132, 95, 95),
    ('Ò', 135, 84, 140, 95, 95),
    ('Ô', 3, 99, 8, 111, 111),
    ('Ö', 11, 100, 16, 111, 111),
    ('Ú', 27, 100, 32, 111, 111),
    ('Ù', 35, 100, 40, 111, 111),
    ('Û', 43, 99, 48, 111, 111),
    ('Ü', 51, 100, 56, 111, 111),
    ('Æ', 67, 103, 72, 111, 111),
    # Lowercase accented (6px bodies, shared baselines)
    ('á', 75, 103, 80, 111, 111),
    ('à', 83, 103, 88, 111, 111),
    ('â', 91, 102, 96, 111, 111),
    ('ä', 99, 103, 104, 111, 111),
    ('ç', 115, 106, 120, 114, 111),
    ('é', 123, 103, 128, 111, 111),
    ('è', 131, 103, 136, 111, 111),
    ('ê', 139, 102, 144, 111, 111),
    ('ë', 3, 119, 8, 127, 127),
    ('í', 12, 119, 13, 127, 127),
    ('ò', 39, 119, 44, 127, 127),
    ('ó', 47, 119, 52, 127, 127),
    ('ô', 55, 118, 60, 127, 127),
    ('ö', 63, 119, 68, 127, 127),
    ('ù', 79, 119, 84, 127, 127),
    ('ú', 87, 119, 92, 127, 127),
    ('û', 95, 118, 100, 127, 127),
    ('ü', 103, 119, 108, 127, 127),
    ('ý', 111, 119, 116, 129, 127),
    ('ÿ', 119, 119, 124, 129, 127),
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
