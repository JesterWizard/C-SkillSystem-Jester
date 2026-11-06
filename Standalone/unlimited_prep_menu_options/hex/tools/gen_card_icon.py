#!/usr/bin/env python3
"""Draw the Hex card item icon (16x16, 4bpp) and write it for the installer.

Output:
  Source/HexCardIcon.bin        128 bytes, the icon tiles in OBJ order (incbin'd by ../Installer.event)
  tools/card_icon_preview.png   8x preview (needs Pillow; skipped with a note if missing)

The icon goes into vanilla icon slot HEX_CARD_ICON (0xDF) of the item icon sheet, which uses palette 0
(OBJ bank 4, the palette of GetItemIconId icons). The palette is read from the clean FE8U ROM at file
offset 0x5996F4 (16 BGR555 colours, the same 16 values as PALETTE below). Only those colours are used:
each wanted colour is mapped to the nearest palette index and the mapping is printed.

Layout of the icon (16x16 pixels, origin top-left):
  - a pointy-top hexagon (vertical left and right sides), 14 px tall (rows 1..14), about 12 px wide
  - outline: the 1 px outer ring (dark), gold: the next ring, blue: the face
  - a white hexagon pip in the centre
  - index 0 (transparent) everywhere else

Tile order (GBA 1D OBJ mapping for a 16x16 sprite): tile 0 top-left, tile 1 top-right, tile 2 bottom-left,
tile 3 bottom-right. Each 8x8 tile is 32 bytes, 8 rows of 4 bytes, the left pixel in the low nibble.

Usage: python3 tools/gen_card_icon.py          (run from the hex folder, or any folder)
"""
import math
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
HEX = os.path.dirname(HERE)
BIN_OUT = os.path.join(HEX, 'Source', 'HexCardIcon.bin')
PNG_OUT = os.path.join(HERE, 'card_icon_preview.png')

# FE8U item icon palette 0 (file offset 0x5996F4, 16 x BGR555). Index 0 is transparent.
PALETTE = [0x67F8, 0x7FFF, 0x5F19, 0x4252, 0x10E5, 0x135B, 0x0434, 0x7947,
           0x49EE, 0x6AD6, 0x3205, 0x5F2D, 0x194E, 0x3A13, 0x20EA, 0x0198]

# The colours wanted for the card (RGB 0..255), mapped to the nearest palette index 1..15.
WANTED = {
    'outline': (24, 24, 32),     # 1 px dark outline
    'gold': (240, 192, 48),      # border inside the outline
    'blue': (48, 96, 232),       # face
    'white': (255, 255, 255),    # centre pip
}

W = 16
H = 16
R = 7.0                 # hexagon: centre (8, 8) in pixel-corner units, vertex distance R (top vertex at y=1)
A = R * math.sqrt(3) / 2  # apothem (half width)
PIP_R = 2.5


def bgr555_to_rgb(c):
    return ((c & 31) * 255 // 31, ((c >> 5) & 31) * 255 // 31, ((c >> 10) & 31) * 255 // 31)


def nearest_index(rgb):
    best = None
    for i in range(1, 16):
        r, g, b = bgr555_to_rgb(PALETTE[i])
        d = (r - rgb[0]) ** 2 + (g - rgb[1]) ** 2 + (b - rgb[2]) ** 2
        if best is None or d < best[0]:
            best = (d, i)
    return best[1]


def in_hex(cx, cy, radius, apothem):
    """Point (cx, cy) inside a pointy-top hexagon centred at (8, 8)."""
    dx = abs(cx - 8.0)
    dy = abs(cy - 8.0)
    if dx > apothem or dy > radius:
        return False
    return (radius / 2) * dx + apothem * dy <= apothem * radius


def erode4(cells):
    out = set()
    for (x, y) in cells:
        if all(((x + ax, y + ay) in cells) for ax, ay in ((1, 0), (-1, 0), (0, 1), (0, -1))):
            out.add((x, y))
    return out


def build_grid():
    outer = set()
    for y in range(H):
        for x in range(W):
            if in_hex(x + 0.5, y + 0.5, R, A):
                outer.add((x, y))
    pip = set()
    for y in range(H):
        for x in range(W):
            if in_hex(x + 0.5, y + 0.5, PIP_R, PIP_R * math.sqrt(3) / 2):
                pip.add((x, y))
    ring1 = erode4(outer)          # inside the outline
    ring2 = erode4(ring1)          # inside the gold border: face
    grid = [[None] * W for _ in range(H)]
    for (x, y) in outer:
        grid[y][x] = 'outline'
    for (x, y) in ring1:
        grid[y][x] = 'gold'
    for (x, y) in ring2:
        grid[y][x] = 'blue'
    for (x, y) in pip:
        if grid[y][x]:
            grid[y][x] = 'white'
    return grid


def encode(grid, index):
    """4bpp, GBA 1D OBJ order: TL, TR, BL, BR 8x8 tiles, 32 bytes each."""
    out = bytearray()
    for ty in (0, 8):
        for tx in (0, 8):
            for y in range(8):
                for x in range(0, 8, 2):
                    lo = index(grid[ty + y][tx + x])
                    hi = index(grid[ty + y][tx + x + 1])
                    out.append(lo | (hi << 4))
    assert len(out) == 0x80
    return bytes(out)


def main():
    mapping = {name: nearest_index(rgb) for name, rgb in WANTED.items()}
    print('colour mapping (wanted RGB -> palette index, palette 0 of the item icons):')
    for name, rgb in WANTED.items():
        i = mapping[name]
        pr, pg, pb = bgr555_to_rgb(PALETTE[i])
        print('  %-8s %-16s -> index %2d  BGR555 $%04X  (%d,%d,%d)' % (
            name, str(rgb), i, PALETTE[i], pr, pg, pb))
    index_of = {None: 0, 0: 0}
    index_of.update(mapping)

    def index(cell):
        return index_of[cell] if cell else 0

    grid = build_grid()
    data = encode(grid, index)
    os.makedirs(os.path.dirname(BIN_OUT), exist_ok=True)
    with open(BIN_OUT, 'wb') as f:
        f.write(data)
    print('wrote %s (%d bytes)' % (BIN_OUT, len(data)))

    chars = {None: '.', 'outline': 'k', 'gold': 'g', 'blue': 'b', 'white': 'w'}
    print('pixel map (. transparent, k outline, g gold, b blue, w white):')
    for row in grid:
        print('  ' + ''.join(chars[c] for c in row))

    try:
        from PIL import Image
    except ImportError:
        print('Pillow not installed: preview not written')
        return 0
    scale = 8
    img = Image.new('RGB', (W * scale, H * scale), (60, 60, 60))
    for y in range(H):
        for x in range(W):
            cell = grid[y][x]
            if not cell:
                continue
            rgb = bgr555_to_rgb(PALETTE[mapping[cell]])
            for dy in range(scale):
                for dx in range(scale):
                    img.putpixel((x * scale + dx, y * scale + dy), rgb)
    img.save(PNG_OUT)
    print('wrote %s (%dx%d, %dx scale)' % (PNG_OUT, W * scale, H * scale, scale))
    return 0


if __name__ == '__main__':
    sys.exit(main())
