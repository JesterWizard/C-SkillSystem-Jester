#!/usr/bin/python3
# -*- coding: UTF-8 -*-
"""Slice the native-size Yu-Gi-Oh! GX Duel Academy sheet into FE8 16x16 2bpp glyphs.

The sheet is already GBA-scale (217x72): 8x16 cells, white ink on black with
gray inner shade and a drop shadow. Copy white 1:1 into 16x16 cells. Do not
resize. Ignore gray; FontItem outline is added after raster.

Layout is A-Z, a-z, then 0-9 with !?"'()*+,-./:;_, then <>[]`{|}~ and ♪.
An unidentified punct cell and a 2px tick are left out; missing punctuation
falls back to vanilla fe8u.
"""

import os
from PIL import Image

SHEET = os.path.join(os.path.dirname(__file__), '..', 'Glyph', 'DuelAcademy', 'sheet.png')
OUT_DIR = os.path.join(os.path.dirname(__file__), '..', 'Glyph', 'DuelAcademy')

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
    # Row 0: A-Z. Baseline 13.
    ('A', 1, 4, 5, 13, 13),
    ('B', 9, 4, 13, 13, 13),
    ('C', 17, 4, 21, 13, 13),
    ('D', 25, 4, 29, 13, 13),
    ('E', 33, 4, 37, 13, 13),
    ('F', 41, 4, 45, 13, 13),
    ('G', 49, 4, 53, 13, 13),
    ('H', 57, 4, 61, 13, 13),
    ('I', 67, 4, 67, 13, 13),
    ('J', 73, 4, 77, 13, 13),
    ('K', 81, 4, 85, 13, 13),
    ('L', 89, 4, 93, 13, 13),
    ('M', 97, 4, 101, 13, 13),
    ('N', 105, 4, 109, 13, 13),
    ('O', 113, 4, 117, 13, 13),
    ('P', 121, 4, 125, 13, 13),
    ('Q', 129, 4, 133, 13, 13),
    ('R', 137, 4, 141, 13, 13),
    ('S', 145, 4, 149, 13, 13),
    ('T', 153, 4, 157, 13, 13),
    ('U', 161, 4, 165, 13, 13),
    ('V', 169, 4, 173, 13, 13),
    ('W', 177, 4, 181, 13, 13),
    ('X', 185, 4, 189, 13, 13),
    ('Y', 193, 4, 197, 13, 13),
    ('Z', 201, 4, 205, 13, 13),
    # Row 1: a-z. Baseline 29 (g/j/p/q/y descenders to 30).
    ('a', 1, 23, 5, 29, 29),
    ('b', 9, 20, 13, 29, 29),
    ('c', 17, 23, 21, 29, 29),
    ('d', 25, 20, 29, 29, 29),
    ('e', 33, 23, 37, 29, 29),
    ('f', 41, 20, 45, 29, 29),
    ('g', 49, 23, 53, 30, 29),
    ('h', 57, 20, 61, 29, 29),
    ('i', 67, 21, 67, 29, 29),
    ('j', 73, 21, 76, 30, 29),
    ('k', 81, 20, 85, 29, 29),
    ('l', 90, 20, 91, 29, 29),
    ('m', 97, 23, 101, 29, 29),
    ('n', 105, 23, 109, 29, 29),
    ('o', 113, 23, 117, 29, 29),
    ('p', 121, 23, 125, 30, 29),
    ('q', 129, 23, 133, 30, 29),
    ('r', 137, 23, 140, 29, 29),
    ('s', 145, 23, 149, 29, 29),
    ('t', 153, 21, 156, 29, 29),
    ('u', 161, 23, 165, 29, 29),
    ('v', 169, 23, 173, 29, 29),
    ('w', 177, 23, 181, 29, 29),
    ('x', 185, 23, 189, 29, 29),
    ('y', 193, 23, 197, 30, 29),
    ('z', 201, 23, 205, 29, 29),
    # Row 2: 0-9 and punctuation. Baseline 45.
    ('0', 1, 36, 5, 45, 45),
    ('1', 10, 36, 11, 45, 45),
    ('2', 17, 36, 21, 45, 45),
    ('3', 25, 36, 29, 45, 45),
    ('4', 33, 36, 37, 45, 45),
    ('5', 41, 36, 45, 45, 45),
    ('6', 49, 36, 53, 45, 45),
    ('7', 57, 36, 61, 45, 45),
    ('8', 65, 36, 69, 45, 45),
    ('9', 73, 36, 77, 45, 45),
    ('!', 83, 36, 83, 45, 45),
    ('?', 89, 36, 93, 45, 45),
    ('"', 97, 36, 101, 39, 45),
    ("'", 113, 36, 114, 39, 45),
    (')', 123, 36, 125, 46, 45),
    ('(', 129, 36, 131, 46, 45),
    ('*', 137, 37, 141, 45, 45),
    ('+', 145, 37, 149, 45, 45),
    (',', 153, 43, 154, 46, 45),
    ('-', 161, 41, 165, 41, 45),
    ('.', 169, 44, 170, 45, 45),
    ('/', 177, 36, 181, 46, 45),
    (':', 186, 38, 187, 44, 45),
    (';', 194, 38, 195, 45, 45),
    ('_', 201, 46, 205, 46, 45),
    # Row 3: more punctuation. Baseline 61 ([ ] { | } ♪ to 62).
    ('<', 1, 53, 5, 61, 61),
    ('=', 9, 55, 13, 58, 61),
    ('>', 17, 53, 21, 61, 61),
    (']', 27, 52, 29, 62, 61),
    ('[', 33, 52, 35, 62, 61),
    ('`', 41, 52, 44, 53, 61),
    ('{', 58, 52, 61, 62, 61),
    ('|', 67, 52, 67, 62, 61),
    ('}', 73, 52, 76, 62, 61),
    ('~', 81, 53, 85, 54, 61),
    ('♪', 93, 52, 99, 62, 61),
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
