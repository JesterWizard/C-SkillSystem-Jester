#!/usr/bin/python3
# -*- coding: UTF-8 -*-
"""Slice the native-size Minish Cap sheet into FE8 16x16 2bpp glyphs.

The sheet is already GBA-scale (199x144): 13 columns by 9 rows of 16px
cells, cropped 9px on the right. White ink on gray. Copy 1:1 into 16x16
cells. Do not resize.

Layout is a-m, n-z, then Latin-1 lowercase, digits with comma/period,
A-M, N-Z, then Latin-1 capitals. `î` stops above `ß` in the same column.
The last cell is a stray bar, not `Ü` (that sits in the cell before it).
Missing ASCII punctuation falls back to vanilla fe8u.
"""

import os
from PIL import Image

SHEET = os.path.join(os.path.dirname(__file__), '..', 'Glyph', 'MinishCap', 'sheet.png')
OUT_DIR = os.path.join(os.path.dirname(__file__), '..', 'Glyph', 'MinishCap')

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
    # Row 0: a-m. Baseline 10 (g/j descenders to 11).
    ('a', 1, 4, 6, 10, 10),
    ('b', 17, 0, 21, 10, 10),
    ('c', 33, 4, 37, 10, 10),
    ('d', 49, 0, 53, 10, 10),
    ('e', 65, 4, 69, 10, 10),
    ('f', 81, 0, 84, 10, 10),
    ('g', 97, 4, 102, 11, 10),
    ('h', 113, 0, 117, 10, 10),
    ('i', 131, 1, 131, 10, 10),
    ('j', 146, 1, 148, 11, 10),
    ('k', 161, 0, 165, 10, 10),
    ('l', 179, 0, 179, 10, 10),
    ('m', 193, 4, 197, 10, 10),
    # Row 1: n-z. Baseline 26 (p/q/y descenders to 27).
    ('n', 1, 20, 5, 26, 26),
    ('o', 17, 20, 21, 26, 26),
    ('p', 33, 20, 37, 27, 26),
    ('q', 49, 20, 53, 27, 26),
    ('r', 66, 20, 69, 26, 26),
    ('s', 81, 20, 85, 26, 26),
    ('t', 97, 17, 100, 26, 26),
    ('u', 113, 20, 117, 26, 26),
    ('v', 129, 20, 133, 26, 26),
    ('w', 145, 20, 149, 26, 26),
    ('x', 161, 20, 165, 26, 26),
    ('y', 177, 20, 181, 27, 26),
    ('z', 193, 20, 197, 26, 26),
    # Row 2: Latin-1 lowercase. Baseline 42 (ç cedilla to 44).
    # î is clipped to this row so ß in the next cell keeps its cap.
    ('à', 1, 33, 6, 42, 42),
    ('â', 17, 33, 22, 42, 42),
    ('ä', 33, 33, 38, 42, 42),
    ('ã', 49, 34, 54, 42, 42),
    ('æ', 64, 36, 70, 42, 42),
    ('ç', 80, 35, 84, 44, 42),
    ('è', 97, 33, 101, 42, 42),
    ('é', 113, 33, 117, 42, 42),
    ('ê', 129, 33, 133, 42, 42),
    ('ë', 145, 34, 149, 42, 42),
    ('ì', 162, 33, 163, 42, 42),
    ('í', 179, 33, 180, 42, 42),
    ('î', 193, 33, 196, 42, 42),
    # Row 3: more Latin-1 lowercase. Baseline 58.
    ('ï', 2, 50, 4, 58, 58),
    ('á', 17, 49, 21, 58, 58),
    ('ñ', 33, 49, 37, 58, 58),
    ('ò', 49, 49, 53, 58, 58),
    ('ó', 65, 49, 69, 58, 58),
    ('ô', 81, 49, 85, 58, 58),
    ('õ', 97, 50, 101, 58, 58),
    ('œ', 112, 52, 118, 58, 58),
    ('ù', 129, 49, 133, 58, 58),
    ('ú', 145, 49, 149, 58, 58),
    ('û', 161, 49, 165, 58, 58),
    ('ü', 177, 50, 181, 58, 58),
    ('ß', 193, 46, 198, 58, 58),
    # Row 4: digits, comma, period. Baseline 74 (comma to 75).
    ('0', 1, 64, 5, 74, 74),
    ('1', 18, 64, 20, 74, 74),
    ('2', 33, 64, 37, 74, 74),
    ('3', 49, 64, 53, 74, 74),
    ('4', 65, 64, 70, 74, 74),
    ('5', 81, 64, 85, 74, 74),
    ('6', 97, 64, 101, 74, 74),
    ('7', 114, 64, 118, 74, 74),
    ('8', 129, 64, 134, 74, 74),
    ('9', 145, 64, 149, 74, 74),
    (',', 163, 72, 164, 75, 74),
    ('.', 178, 73, 179, 74, 74),
    # Row 5: A-M. Baseline 95.
    ('A', 1, 85, 5, 95, 95),
    ('B', 17, 85, 21, 95, 95),
    ('C', 33, 85, 38, 95, 95),
    ('D', 49, 85, 53, 95, 95),
    ('E', 65, 85, 69, 95, 95),
    ('F', 81, 85, 85, 95, 95),
    ('G', 97, 85, 102, 95, 95),
    ('H', 113, 85, 117, 95, 95),
    ('I', 130, 85, 132, 95, 95),
    ('J', 145, 85, 149, 95, 95),
    ('K', 161, 85, 166, 95, 95),
    ('L', 177, 85, 182, 95, 95),
    ('M', 193, 85, 197, 95, 95),
    # Row 6: N-Z. Baseline 111.
    ('N', 1, 101, 5, 111, 111),
    ('O', 17, 101, 22, 111, 111),
    ('P', 33, 101, 37, 111, 111),
    ('Q', 49, 101, 54, 111, 111),
    ('R', 65, 101, 69, 111, 111),
    ('S', 81, 101, 85, 111, 111),
    ('T', 97, 101, 101, 111, 111),
    ('U', 113, 101, 117, 111, 111),
    ('V', 129, 101, 133, 111, 111),
    ('W', 145, 101, 149, 111, 111),
    ('X', 161, 101, 165, 111, 111),
    ('Y', 177, 101, 181, 111, 111),
    ('Z', 193, 101, 197, 111, 111),
    # Row 7: Latin-1 capitals. Baseline 127.
    ('À', 1, 115, 5, 127, 127),
    ('Â', 17, 115, 21, 127, 127),
    ('Ä', 33, 115, 37, 127, 127),
    ('Ã', 49, 116, 53, 127, 127),
    ('Æ', 64, 117, 70, 127, 127),
    ('Ç', 80, 118, 86, 127, 127),
    ('È', 97, 116, 101, 127, 127),
    ('É', 113, 116, 117, 127, 127),
    ('Ê', 129, 116, 133, 127, 127),
    ('Ë', 145, 117, 149, 127, 127),
    ('Ì', 162, 115, 164, 127, 127),
    ('Í', 178, 115, 180, 127, 127),
    ('Î', 194, 115, 196, 127, 127),
    # Row 8: more Latin-1 capitals. Baseline 143.
    ('Ï', 2, 132, 4, 143, 143),
    ('Ø', 17, 133, 22, 143, 143),
    ('Ñ', 33, 131, 37, 143, 143),
    ('Ò', 49, 131, 54, 143, 143),
    ('Ó', 65, 131, 70, 143, 143),
    ('Õ', 81, 128, 86, 143, 143),
    ('Ö', 97, 132, 102, 143, 143),
    ('Œ', 112, 134, 118, 143, 143),
    ('Ù', 129, 131, 133, 143, 143),
    ('Ú', 145, 131, 149, 143, 143),
    ('Û', 161, 131, 165, 143, 143),
    ('Ü', 177, 132, 181, 143, 143),
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
