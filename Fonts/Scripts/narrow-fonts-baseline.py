#!/usr/bin/python3
"""Copy Glyph/NarrowFonts with its glyphs moved up, so their bottom row meets another font's.

    python3 Fonts/Scripts/narrow-fonts-baseline.py OUT_DIR TEXT_SHIFT ITEM_SHIFT

The narrow glyphs are drawn for vanilla's baseline (their bottom row is 13, text and item). A font that sits higher
gets its own copy, listed in Fonts/FontList.txt under its CONFIG_FONT_* before the generic NarrowFonts (the first list
with a character wins). Each FontText_* moves up TEXT_SHIFT rows and each FontItem_* ITEM_SHIFT rows; a glyph whose
ink would leave the cell stays where it is.
"""

import os
import shutil
import sys
from PIL import Image

SRC = os.path.join(os.path.dirname(__file__), '..', 'Glyph', 'NarrowFonts')


def shift_up(path, out, rows):
    img = Image.open(path)
    px = img.load()
    w, h = img.size
    bg = px[0, 0]
    top = min((y for y in range(h) for x in range(w) if px[x, y] != bg), default=h)
    if rows <= 0 or top < rows:
        img.save(out)
        return
    moved = Image.new(img.mode, img.size, bg)
    if img.mode == 'P':
        moved.putpalette(img.getpalette())
    moved.paste(img.crop((0, rows, w, h)), (0, 0))
    moved.save(out)


def main():
    out_dir, text_shift, item_shift = sys.argv[1], int(sys.argv[2]), int(sys.argv[3])
    src = os.path.abspath(SRC)
    os.makedirs(out_dir, exist_ok=True)
    shutil.copy(os.path.join(src, 'font.fontall.txt'), os.path.join(out_dir, 'font.fontall.txt'))
    n = 0
    for name in sorted(os.listdir(src)):
        if not name.endswith('.png'):
            continue
        rows = text_shift if name.startswith('FontText_') else item_shift
        shift_up(os.path.join(src, name), os.path.join(out_dir, name), rows)
        n += 1
    print('wrote {} glyphs to {} (text up {}, item up {})'.format(n, out_dir, text_shift, item_shift))


if __name__ == '__main__':
    main()
