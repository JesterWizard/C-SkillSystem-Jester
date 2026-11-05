#!/usr/bin/env python3
"""Generate the card art of the Hex screen (Kernel/Wizardry/Minigames/Hex/Source/HexArt.c).

Run from anywhere:

    python3 Tools/hex/gen_hex_art.py            rewrites the generated block in HexArt.c
    python3 Tools/hex/gen_hex_art.py --check    exits 1 if the block is out of date

The block holds three 32x32 4bpp images (a 4bpp nibble is a palette index; 0 is transparent; tiles are 8x8, row-major,
left pixel = low nibble, 1D OBJ order):

- gHexFrameTiles[64][128]: the ring and the reach dots for each reach mask (bit d = side d has a dot). Drawn IN FRONT of the
  portrait. The ring is 2 px wide along the hexagon edge: index 1 (dark) on the outer 1 px, index 2 (owner colour) just inside
  it, index 3 (owner highlight) on the inner pixels of the top-left facing sides UL and L. A reach dot is a 3x3 index 4 (white)
  square with a 1 px index 1 border (5x5), centred on the ring at the middle of its side.
- gHexCardBackTiles[128]: the owner colour fill of the hexagon interior (index 2, a 1 px index 3 line under the top edges).
  Drawn BEHIND the portrait, so the transparent parts of the chibi show the owner colour.
- gHexPortraitKeep[128]: the portrait keep mask (nibble 0xF keeps the portrait pixel). It keeps every pixel inside the
  hexagon and inside the ring (the 2 px band of the ring is covered by the frame anyway).

Hexagon (continuous box coordinates, clockwise from the top point): top (16, 0), upper-right (30, 8), lower-right (30, 24),
bottom (16, 32), lower-left (2, 24), upper-left (2, 8). Direction d (0 UL, 1 UR, 2 R, 3 DR, 4 DL, 5 L) is side SIDE[d].
Pixel (x, y) is tested at its centre (x + 0.5, y + 0.5). The geometry is shared with the Minish Cap mod (tools/custom/gen_hex_ui.py).
"""

import math
import os
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
HEX_ART_C = os.path.join(ROOT, "Kernel", "Wizardry", "Minigames", "Hex", "Source", "HexArt.c")

SIZE = 32
CENTRE = (16.0, 16.0)
VERTS = [(16.0, 0.0), (30.0, 8.0), (30.0, 24.0), (16.0, 32.0), (2.0, 24.0), (2.0, 8.0)]
SIDE = {0: (5, 0), 1: (0, 1), 2: (1, 2), 3: (2, 3), 4: (3, 4), 5: (4, 5)}

RING_W = 2.0                 # the ring is the 2 px band inside the hexagon edge
RING_DARK = 1                # palette index of the outer px of the ring
RING_FILL = 2                # palette index of the owner colour inside the ring
RING_LIGHT = 3               # palette index of the highlight (top-left facing sides UL and L)
LIGHT_SIDES = (0, 5)
BACK_FILL = 2
BACK_LINE = 3                # 1 px line under the top edges of the back tile
BACK_LINE_SIDES = (0, 1)
DOT_DEPTH = 1.0              # the dot centre is 1 px inside the edge: the middle of the ring
DOT_WHITE = 4
DOT_DARK = 1
DOT_HALF = 2                 # the dark border is 2 px from the dot centre (5x5 total), the white square 1 px (3x3)
BEGIN = "/* BEGIN GENERATED: Tools/hex/gen_hex_art.py (card frame, card back and portrait keep) */"
END = "/* END GENERATED: Tools/hex/gen_hex_art.py */"


def fail(msg):
    sys.stderr.write("gen_hex_art: %s\n" % msg)
    sys.exit(1)


def inward_normal(a, b):
    dx, dy = b[0] - a[0], b[1] - a[1]
    n = math.hypot(dx, dy)
    nx, ny = -dy / n, dx / n
    if (CENTRE[0] - a[0]) * nx + (CENTRE[1] - a[1]) * ny < 0:
        nx, ny = -nx, -ny
    return nx, ny


SIDE_NORMAL = {d: inward_normal(VERTS[a], VERTS[b]) for d, (a, b) in SIDE.items()}


def side_dist(px, py, d):
    """Signed distance from (px, py) to side d's line (> 0 inside)."""
    a, _ = SIDE[d]
    nx, ny = SIDE_NORMAL[d]
    return (px - VERTS[a][0]) * nx + (py - VERTS[a][1]) * ny


def border_dist(px, py):
    """Signed distance from (px, py) to the nearest hexagon side (> 0 inside)."""
    return min(side_dist(px, py, d) for d in SIDE)


def nearest_side(px, py):
    return min(SIDE, key=lambda d: side_dist(px, py, d))


def dot_pixel(d):
    """Pixel of the centre of the reach dot of side d: DOT_DEPTH inside the middle of the side, rounded to the pixel
    whose centre is nearest (the pixel containing the point)."""
    a, b = SIDE[d]
    mid = ((VERTS[a][0] + VERTS[b][0]) / 2.0, (VERTS[a][1] + VERTS[b][1]) / 2.0)
    nx, ny = SIDE_NORMAL[d]
    px, py = mid[0] + DOT_DEPTH * nx, mid[1] + DOT_DEPTH * ny
    return int(math.floor(px)), int(math.floor(py))


DOT_PIX = {d: dot_pixel(d) for d in range(6)}


def frame_grid(mask):
    """Indices of the frame for one reach mask: ring first, then the dots of the mask on top."""
    dots = {}
    for d in range(6):
        if not (mask >> d) & 1:
            continue
        cx, cy = DOT_PIX[d]
        for x in range(cx - DOT_HALF, cx + DOT_HALF + 1):
            for y in range(cy - DOT_HALF, cy + DOT_HALF + 1):
                dots[(x, y)] = DOT_DARK
        for x in range(cx - 1, cx + 2):
            for y in range(cy - 1, cy + 2):
                dots[(x, y)] = DOT_WHITE
    grid = []
    for y in range(SIZE):
        row = []
        for x in range(SIZE):
            px, py = x + 0.5, y + 0.5
            dist = border_dist(px, py)
            v = 0
            if 0.0 <= dist < RING_W:
                if dist < 1.0:
                    v = RING_DARK
                elif nearest_side(px, py) in LIGHT_SIDES:
                    v = RING_LIGHT
                else:
                    v = RING_FILL
            if (x, y) in dots:
                v = dots[(x, y)]
            row.append(v)
        grid.append(row)
    return grid


def back_grid():
    grid = []
    for y in range(SIZE):
        row = []
        for x in range(SIZE):
            px, py = x + 0.5, y + 0.5
            dist = border_dist(px, py)
            v = 0
            if dist >= 0.0:
                v = BACK_FILL
                if RING_W <= dist < RING_W + 1.0 and nearest_side(px, py) in BACK_LINE_SIDES:
                    v = BACK_LINE
            row.append(v)
        grid.append(row)
    return grid


def keep_grid():
    """0xF keeps the portrait pixel: every pixel inside the hexagon and inside the ring."""
    return [[0xF if border_dist(x + 0.5, y + 0.5) >= RING_W else 0 for x in range(SIZE)] for y in range(SIZE)]


def pack(grid):
    """4bpp words in tile order (8x8 tiles row-major, left pixel = low nibble)."""
    words = []
    for ty in range(SIZE // 8):
        for tx in range(SIZE // 8):
            for row in range(8):
                v = 0
                for col in range(8):
                    v |= (grid[ty * 8 + row][tx * 8 + col] & 0xF) << (4 * col)
                words.append(v)
    return words


def emit_words(words, per_line=8):
    lines = []
    for t in range(0, len(words), per_line):
        chunk = words[t:t + per_line]
        last = t + per_line >= len(words)
        lines.append("\t" + ", ".join("0x%08X" % w for w in chunk) + ("" if last else ","))
    return lines


def generated_body():
    lines = []
    lines.append("/* Hex frame for each reach mask (bit d = side d has a reach dot; UL=0 UR=1 R=2 DR=3 DL=4 L=5): 32x32 4bpp,")
    lines.append(" * 16 tiles, 1D OBJ order. The ring (2 px, drawn in front of the portrait): index 1 dark outer px, 2 owner")
    lines.append(" * fill, 3 highlight on the top-left facing sides; the reach dots (3x3 index 4 with a 1 px index 1 border)")
    lines.append(" * sit on the ring at the middle of their side. Index 0 is transparent. The same tiles serve gHexFramePalRed")
    lines.append(" * and gHexFramePalBlue.")
    lines.append(" */")
    lines.append("const u32 gHexFrameTiles[64][128] = {")
    for mask in range(64):
        lines.append("\t{ /* mask %02X */" % mask)
        lines.extend(emit_words(pack(frame_grid(mask))))
        lines.append("\t}," if mask < 63 else "\t}")
    lines.append("};")
    lines.append("")
    lines.append("/* The card back: the owner colour fill of the hexagon (index 2, index 3 under the top edges), drawn behind the")
    lines.append(" * portrait in the same frame palette bank as the ring (HexScreen_CardBody). 32x32 4bpp, 16 tiles.")
    lines.append(" */")
    lines.append("const u32 gHexCardBackTiles[128] = {")
    lines.extend(emit_words(pack(back_grid())))
    lines.append("};")
    lines.append("")
    lines.append("/* 32x32 4bpp keep mask for a card portrait (512 bytes): nibble 0xF keeps the pixel, 0 clears it. */")
    lines.append("const u32 gHexPortraitKeep[128] = {")
    lines.extend(emit_words(pack(keep_grid())))
    lines.append("};")
    return "\n".join(lines) + "\n"


def block_text():
    return BEGIN + "\n" + generated_body() + END


def main(argv):
    check = "--check" in argv
    with open(HEX_ART_C, encoding="utf-8") as f:
        text = f.read()
    if BEGIN not in text or END not in text:
        fail("%s: no GENERATED markers" % HEX_ART_C)
    head, rest = text.split(BEGIN, 1)
    _, tail = rest.split(END, 1)
    want = head + block_text() + tail
    if check:
        if want != text:
            fail("card art is out of date; run Tools/hex/gen_hex_art.py")
        print("gen_hex_art: card art is up to date")
        return 0
    with open(HEX_ART_C, "w", encoding="utf-8") as f:
        f.write(want)
    print("gen_hex_art: wrote the card art into %s" % HEX_ART_C)
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
