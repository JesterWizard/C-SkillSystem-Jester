#!/usr/bin/python3
# -*- coding: UTF-8 -*-
"""Slice the native-size Super Star Saga sheet into FE8 16x16 2bpp glyphs.

The sheet is already GBA-scale (243x135). Copy ink 1:1 into 16x16 cells.
Do not resize. Only the dark outline (48,48,48) is the letter. The off-white
(248,248,248) inside counters is leftover sheet background, not fill.
Ignore that, the purple drop shadow, and the black UI boxes.

Sheet ripped by Nuffe90.
"""

import os
from PIL import Image

SHEET = os.path.join(os.path.dirname(__file__), '..', 'Glyph', 'SuperStarSaga', 'sheet.png')
OUT_DIR = os.path.join(os.path.dirname(__file__), '..', 'Glyph', 'SuperStarSaga')

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
    (48, 48, 48),      # dark outline only
}

# (char, x0, y0, x1, y1, sheet_baseline) with inclusive box coords.
# Each row of the sheet shares a baseline so caps, x-height, and
# descenders line up in the 16x16 cell. q is omitted: the rip has
# none; it is mirrored from p after raster.
GLYPHS = [
    # Lowercase a-z except q, plus ã ä ö. Baseline 17 (descenders to 19).
    ('a', 9, 12, 13, 17, 17),
    ('b', 16, 10, 20, 17, 17),
    ('c', 23, 12, 27, 17, 17),
    ('d', 31, 10, 35, 17, 17),
    ('e', 39, 12, 43, 17, 17),
    ('f', 46, 10, 49, 17, 17),
    ('g', 54, 12, 58, 19, 17),
    ('h', 62, 10, 66, 17, 17),
    ('i', 70, 10, 70, 17, 17),
    ('j', 73, 10, 76, 19, 17),
    ('k', 79, 10, 83, 17, 17),
    ('l', 87, 10, 87, 17, 17),
    ('m', 90, 12, 96, 17, 17),
    ('n', 99, 12, 103, 17, 17),
    ('o', 106, 12, 110, 17, 17),
    ('p', 113, 12, 117, 19, 17),
    ('r', 120, 12, 123, 17, 17),
    ('s', 126, 12, 130, 17, 17),
    ('t', 134, 10, 137, 17, 17),
    ('u', 141, 12, 145, 17, 17),
    ('v', 148, 12, 152, 17, 17),
    ('w', 155, 12, 161, 17, 17),
    ('x', 165, 12, 169, 17, 17),
    ('y', 173, 12, 177, 19, 17),
    ('z', 181, 12, 185, 17, 17),
    ('ã', 189, 8, 193, 17, 17),
    ('ä', 196, 9, 200, 17, 17),
    ('ö', 203, 9, 207, 17, 17),
    # Caps A-Z, y=27-34, baseline 34
    ('A', 8, 27, 14, 34, 34),
    ('B', 17, 27, 22, 34, 34),
    ('C', 25, 27, 30, 34, 34),
    ('D', 33, 27, 38, 34, 34),
    ('E', 41, 27, 45, 34, 34),
    ('F', 49, 27, 53, 34, 34),
    ('G', 56, 27, 61, 34, 34),
    ('H', 64, 27, 68, 34, 34),
    ('I', 72, 27, 74, 34, 34),
    ('J', 77, 27, 81, 34, 34),
    ('K', 85, 27, 91, 34, 34),
    ('L', 95, 27, 99, 34, 34),
    ('M', 103, 27, 109, 34, 34),
    ('N', 112, 27, 117, 34, 34),
    ('O', 120, 27, 126, 34, 34),
    ('P', 130, 27, 135, 34, 34),
    ('Q', 138, 27, 144, 34, 34),
    ('R', 147, 27, 152, 34, 34),
    ('S', 156, 27, 161, 34, 34),
    ('T', 165, 27, 171, 34, 34),
    ('U', 174, 27, 179, 34, 34),
    ('V', 182, 27, 188, 34, 34),
    ('W', 191, 27, 197, 34, 34),
    ('X', 201, 27, 205, 34, 34),
    ('Y', 209, 27, 213, 34, 34),
    ('Z', 215, 27, 220, 34, 34),
    # Punctuation and digit 1. Baseline 51 / 64.
    ('?', 8, 44, 12, 51, 51),
    ('!', 16, 44, 16, 51, 51),
    (',', 20, 50, 21, 53, 51),
    ('.', 26, 50, 27, 51, 51),
    ('-', 34, 47, 38, 47, 51),
    (':', 44, 46, 45, 51, 51),
    (';', 51, 46, 52, 53, 51),
    ('1', 8, 57, 10, 64, 64),
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


def mirror_text(text_img):
    """Horizontal flip of ink, keeping the left pad. Used for q from p."""
    src = text_img.load()
    xs = [x for y in range(16) for x in range(16) if src[x, y] == 3]
    img = Image.new('P', (16, 16), 0)
    img.putpalette(PALETTE_TEXT)
    if not xs:
        return img
    dst = img.load()
    x0, x1 = min(xs), max(xs)
    for y in range(16):
        for x in range(x0, x1 + 1):
            if src[x, y] == 3:
                dst[x0 + (x1 - x), y] = 3
    return img


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
    rasters = {}
    for ch, x0, y0, x1, y1, baseline in GLYPHS:
        if ch in seen:
            raise SystemExit('duplicate glyph {}'.format(repr(ch)))
        text_img, width = raster_text(im, x0, y0, x1, y1, baseline)
        hid = hex_id_for(ch)
        save_glyph(text_img, to_item(text_img), hid)
        entries.append((ch, width, hid))
        rasters[ch] = (text_img, width)
        seen.add(ch)

    # Sheet has no q; Super Star Saga q is a mirrored p.
    if 'q' in seen:
        raise SystemExit('q is already in the sheet; drop the mirror')
    q_text = mirror_text(rasters['p'][0])
    q_width = rasters['p'][1]
    save_glyph(q_text, to_item(q_text), hex_id_for('q'))
    entries.append(('q', q_width, hex_id_for('q')))

    # Keep font.fontall.txt in codepoint order so diffs stay readable.
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
