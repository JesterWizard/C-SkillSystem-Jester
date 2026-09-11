#!/usr/bin/python3
# -*- coding: UTF-8 -*-
"""Slice the native-size Pokémon FireRed sheet into FE8 16x16 2bpp glyphs.

The sheet is already GBA-scale (256x77): packed Latin rows, white ink on
black. Copy 1:1 into 16x16 cells. Do not resize.

Layout is accented leftovers, then symbols, then 0-9 with !?.,-, then
quotes/♂♀ and A-Z, then a-z. Decorative arrows and unidentified Latin-1
are left out; missing punctuation falls back to vanilla fe8u.
"""

import os
from PIL import Image

SHEET = os.path.join(os.path.dirname(__file__), '..', 'Glyph', 'PokeFireRed', 'sheet.png')
OUT_DIR = os.path.join(os.path.dirname(__file__), '..', 'Glyph', 'PokeFireRed')

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
    # Row 1 leftovers: + = : ; ( ). Baseline 25 (semicolon tail to 27).
    ('+', 68, 20, 72, 24, 25),
    ('=', 87, 21, 91, 24, 25),
    (';', 95, 20, 96, 27, 25),
    (':', 100, 20, 101, 25, 25),
    ('(', 178, 17, 179, 28, 28),
    (')', 183, 17, 184, 28, 28),
    # Row 2: 0-9, !?.,-. Baseline 40.
    ('0', 72, 32, 76, 40, 40),
    ('1', 79, 32, 81, 40, 40),
    ('2', 84, 32, 88, 40, 40),
    ('3', 90, 32, 94, 40, 40),
    ('4', 96, 32, 100, 40, 40),
    ('5', 102, 32, 106, 40, 40),
    ('6', 108, 32, 112, 40, 40),
    ('7', 114, 32, 118, 40, 40),
    ('8', 120, 32, 124, 40, 40),
    ('9', 126, 32, 130, 40, 40),
    ('!', 134, 31, 134, 40, 40),
    ('?', 138, 31, 142, 40, 40),
    ('.', 145, 39, 146, 40, 40),
    ('-', 149, 36, 153, 36, 40),
    # Row 3: quotes, ♂♀, comma, slash, A-Z. Baseline 54.
    ('…', 0, 54, 4, 54, 54),
    ("'", 6, 44, 7, 47, 54),
    ('"', 6, 44, 10, 47, 54),
    ('♂', 24, 45, 28, 54, 54),
    ('♀', 30, 45, 34, 54, 54),
    (',', 45, 53, 46, 56, 54),
    ('/', 58, 45, 62, 54, 54),
    ('A', 64, 47, 68, 54, 54),
    ('B', 70, 47, 74, 54, 54),
    ('C', 76, 47, 80, 54, 54),
    ('D', 82, 47, 86, 54, 54),
    ('E', 88, 47, 92, 54, 54),
    ('F', 94, 47, 98, 54, 54),
    ('G', 100, 47, 104, 54, 54),
    ('H', 106, 47, 110, 54, 54),
    ('I', 112, 47, 116, 54, 54),
    ('J', 118, 47, 122, 54, 54),
    ('K', 124, 47, 128, 54, 54),
    ('L', 130, 47, 134, 54, 54),
    ('M', 136, 47, 140, 54, 54),
    ('N', 142, 47, 146, 54, 54),
    ('O', 148, 47, 152, 54, 54),
    ('P', 154, 47, 158, 54, 54),
    ('Q', 160, 47, 164, 54, 54),
    ('R', 166, 47, 170, 54, 54),
    ('S', 172, 47, 176, 54, 54),
    ('T', 178, 47, 182, 54, 54),
    ('U', 184, 47, 188, 54, 54),
    ('V', 190, 47, 194, 54, 54),
    ('W', 196, 47, 200, 54, 54),
    ('X', 202, 47, 206, 54, 54),
    ('Y', 208, 47, 212, 54, 54),
    ('Z', 214, 47, 218, 54, 54),
    # Row 4: a-z. Baseline 68 (g/j/p/q/y descenders to 70).
    ('a', 0, 64, 4, 68, 68),
    ('b', 6, 61, 10, 68, 68),
    ('c', 12, 64, 16, 68, 68),
    ('d', 18, 61, 22, 68, 68),
    ('e', 24, 64, 28, 68, 68),
    ('f', 30, 61, 33, 68, 68),
    ('g', 35, 64, 39, 70, 68),
    ('h', 41, 61, 45, 68, 68),
    ('i', 48, 61, 48, 68, 68),
    ('j', 51, 61, 54, 70, 68),
    ('k', 57, 61, 60, 68, 68),
    ('l', 63, 61, 64, 68, 68),
    ('m', 67, 64, 71, 68, 68),
    ('n', 73, 64, 76, 68, 68),
    ('o', 78, 64, 82, 68, 68),
    ('p', 84, 64, 88, 70, 68),
    ('q', 90, 64, 94, 70, 68),
    ('r', 96, 64, 99, 68, 68),
    ('s', 101, 64, 104, 68, 68),
    ('t', 106, 62, 109, 68, 68),
    ('u', 111, 64, 115, 68, 68),
    ('v', 117, 64, 121, 68, 68),
    ('w', 123, 64, 127, 68, 68),
    ('x', 129, 64, 133, 68, 68),
    ('y', 135, 64, 139, 70, 68),
    ('z', 142, 64, 146, 68, 68),
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
