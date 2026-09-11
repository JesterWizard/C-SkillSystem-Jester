#!/usr/bin/python3
# -*- coding: UTF-8 -*-
"""Slice the native-size Boktai 2 sheet into FE8 16x16 2bpp glyphs.

The sheet is already GBA-scale (483x425): name-entry and password UI.
Copy dark ink 1:1 into 16x16 cells. Do not resize. Ignore the peach
highlight; FontItem outline is added after raster.

Letters A-Z/a-z and .,'-/ come from the player-name grid (complete
alphabet). Digits and remaining ASCII punct come from the password
grid. Missing punctuation falls back to vanilla fe8u.
"""

import os
from PIL import Image

SHEET = os.path.join(os.path.dirname(__file__), '..', 'Glyph', 'Boktai2', 'sheet.png')
OUT_DIR = os.path.join(os.path.dirname(__file__), '..', 'Glyph', 'Boktai2')

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

# Dusty rose body plus the few midtone pixels that complete diagonals.
INK_RGB = {
    (160, 104, 104),
    (224, 152, 128),
}

# (char, x0, y0, x1, y1, sheet_baseline) with inclusive box coords.
GLYPHS = [
    # Player name A-I and punct. Baseline 56.
    ('A', 259, 50, 264, 56, 56),
    ('B', 275, 50, 280, 56, 56),
    ('C', 291, 50, 296, 56, 56),
    ('D', 307, 50, 312, 56, 56),
    ('E', 323, 50, 328, 56, 56),
    ('F', 339, 50, 344, 56, 56),
    ('G', 355, 50, 360, 56, 56),
    ('H', 371, 50, 376, 56, 56),
    ('I', 387, 50, 391, 56, 56),
    ('.', 411, 54, 412, 55, 56),
    (',', 427, 54, 428, 56, 56),
    ("'", 442, 49, 443, 51, 56),
    ('-', 459, 53, 464, 53, 56),
    # Player name J-R and slash. Baseline 72.
    ('J', 259, 66, 264, 72, 72),
    ('K', 275, 66, 280, 72, 72),
    ('L', 291, 66, 296, 72, 72),
    ('M', 306, 66, 312, 72, 72),
    ('N', 323, 66, 328, 72, 72),
    ('O', 339, 66, 344, 72, 72),
    ('P', 355, 66, 360, 72, 72),
    ('Q', 371, 66, 376, 72, 72),
    ('R', 387, 66, 392, 72, 72),
    ('/', 411, 65, 416, 72, 72),
    # Player name S-Z. Baseline 88.
    ('S', 259, 82, 264, 88, 88),
    ('T', 274, 82, 280, 88, 88),
    ('U', 291, 82, 296, 88, 88),
    ('V', 306, 82, 312, 88, 88),
    ('W', 322, 82, 328, 88, 88),
    ('X', 338, 82, 344, 88, 88),
    ('Y', 354, 82, 360, 88, 88),
    ('Z', 371, 82, 376, 88, 88),
    # Player name a-i. Baseline 112.
    ('a', 259, 107, 264, 112, 112),
    ('b', 275, 105, 280, 112, 112),
    ('c', 291, 107, 296, 112, 112),
    ('d', 307, 105, 312, 112, 112),
    ('e', 323, 107, 328, 112, 112),
    ('f', 339, 105, 343, 112, 112),
    ('g', 355, 106, 360, 112, 112),
    ('h', 371, 105, 376, 112, 112),
    ('i', 389, 106, 389, 112, 112),
    # Player name j-r. Baseline 128.
    ('j', 259, 121, 264, 128, 128),
    ('k', 275, 121, 279, 128, 128),
    ('l', 293, 121, 294, 128, 128),
    ('m', 306, 123, 312, 128, 128),
    ('n', 323, 123, 328, 128, 128),
    ('o', 339, 123, 344, 128, 128),
    ('p', 355, 122, 360, 128, 128),
    ('q', 371, 122, 376, 128, 128),
    ('r', 387, 123, 392, 128, 128),
    # Player name s-z. Baseline 144.
    ('s', 259, 139, 264, 144, 144),
    ('t', 275, 138, 280, 144, 144),
    ('u', 291, 139, 296, 144, 144),
    ('v', 307, 139, 312, 144, 144),
    ('w', 322, 139, 328, 144, 144),
    ('x', 339, 139, 344, 144, 144),
    ('y', 355, 138, 360, 144, 144),
    ('z', 371, 139, 376, 144, 144),
    # Password digits 0-5. Baseline 72.
    ('0', 138, 66, 143, 72, 72),
    ('1', 156, 66, 158, 72, 72),
    ('2', 170, 66, 175, 72, 72),
    ('3', 186, 66, 191, 72, 72),
    ('4', 202, 66, 207, 72, 72),
    ('5', 218, 66, 223, 72, 72),
    # Password 6-9+=. Baseline 88.
    ('6', 138, 82, 143, 88, 88),
    ('7', 154, 82, 159, 88, 88),
    ('8', 170, 82, 175, 88, 88),
    ('9', 186, 82, 191, 88, 88),
    ('+', 201, 82, 207, 88, 88),
    ('=', 218, 83, 223, 86, 88),
    # Password remaining punct. Baseline 104 / 120 / 136 / 152.
    (':', 140, 99, 141, 104, 104),
    ('_', 186, 104, 191, 104, 104),
    ('>', 203, 98, 206, 104, 104),
    ('^', 218, 97, 222, 99, 104),
    ('#', 138, 113, 143, 120, 120),
    ('?', 138, 130, 143, 136, 136),
    ('@', 137, 146, 143, 152, 152),
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
