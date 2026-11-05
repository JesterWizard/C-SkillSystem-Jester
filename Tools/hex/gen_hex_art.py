#!/usr/bin/env python3
"""Generate the portrait keep mask of the Hex card art (Kernel/Wizardry/Minigames/Hex/Source/HexArt.c).

Run from anywhere:

    python3 Tools/hex/gen_hex_art.py            rewrites the mask block in HexArt.c
    python3 Tools/hex/gen_hex_art.py --check    exits 1 if the block is out of date

The mask is the 32x32 4bpp tile (512 bytes) that a card portrait is ANDed with before it is copied to OBJ VRAM. A pixel
nibble 0xF keeps the portrait pixel, 0 clears it (transparent). A portrait keeps only the pixels inside the hexagon and
clear of the 2 px outline, and not in the six reach-dot blocks (3x3 plus a 1 px margin). Port of hex_portrait_keep() and its
geometry from the Minish Cap mod (tools/custom/gen_hex_ui.py there), so the frames in HexArt.c and the mask share one layout.

Hexagon (continuous box coordinates, clockwise from the top point): top (16, 0), upper-right (30, 8), lower-right (30, 24),
bottom (16, 32), lower-left (2, 24), upper-left (2, 8). Direction d (0 UL, 1 UR, 2 R, 3 DR, 4 DL, 5 L) is side SIDE[d].
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
OUTLINE_W = 2.0
DOT_DEPTH = 4.0
DOT_MARGIN = 1
BEGIN = "/* BEGIN GENERATED: Tools/hex/gen_hex_art.py (portrait keep mask) */"
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


def border_dist(px, py):
    """Signed distance from (px, py) to the nearest hexagon side (> 0 inside)."""
    best = None
    for d, (a, b) in SIDE.items():
        nx, ny = SIDE_NORMAL[d]
        v = (px - VERTS[a][0]) * nx + (py - VERTS[a][1]) * ny
        best = v if best is None or v < best else best
    return best


def dot_pixel(d):
    """Pixel at the centre of the reach dot of direction d (same rule as gen_hex_ui.py dot_pixel)."""
    a, b = SIDE[d]
    mid = ((VERTS[a][0] + VERTS[b][0]) / 2.0, (VERTS[a][1] + VERTS[b][1]) / 2.0)
    nx, ny = SIDE_NORMAL[d]
    px, py = mid[0] + DOT_DEPTH * nx, mid[1] + DOT_DEPTH * ny
    cx = math.floor(px) if nx < 0 else math.ceil(px) - 1
    cy = math.floor(py) if ny <= 0 else math.ceil(py) - 1
    return int(cx), int(cy)


DOT_PIX = {d: dot_pixel(d) for d in range(6)}


def dot_block(d, margin=0):
    cx, cy = DOT_PIX[d]
    r = 1 + margin
    return {(x, y) for x in range(cx - r, cx + r + 1) for y in range(cy - r, cy + r + 1)}


def keep_grid():
    cleared = set()
    for d in range(6):
        cleared |= dot_block(d, DOT_MARGIN)
    return [[border_dist(x + 0.5, y + 0.5) >= OUTLINE_W and (x, y) not in cleared for x in range(SIZE)]
            for y in range(SIZE)]


def pack_keep(g):
    """4bpp words, tile order as gHexFrameTiles (8x8 tiles row-major, left pixel = low nibble). 0xF keeps a pixel."""
    words = []
    for ty in range(SIZE // 8):
        for tx in range(SIZE // 8):
            for row in range(8):
                v = 0
                for col in range(8):
                    if g[ty * 8 + row][tx * 8 + col]:
                        v |= 0xF << (4 * col)
                words.append(v)
    return words


def mask_block():
    words = pack_keep(keep_grid())
    lines = [BEGIN]
    lines.append("/* 32x32 4bpp keep mask for a card portrait (512 bytes): nibble 0xF keeps the pixel, 0 clears it. */")
    lines.append("const u32 gHexPortraitKeep[128] = {")
    for t in range(len(words) // 8):
        chunk = words[t * 8:(t + 1) * 8]
        lines.append("\t" + ", ".join("0x%08X" % w for w in chunk) + ("," if t < len(words) // 8 - 1 else ""))
    lines.append("};")
    lines.append(END)
    return "\n".join(lines) + "\n"


def main(argv):
    check = "--check" in argv
    with open(HEX_ART_C, encoding="utf-8") as f:
        text = f.read()
    if BEGIN not in text or END not in text:
        fail("%s: no GENERATED markers" % HEX_ART_C)
    head, rest = text.split(BEGIN, 1)
    _, tail = rest.split(END, 1)
    want = head + mask_block().rstrip("\n") + tail
    want = want.rstrip("\n") + "\n" if not text.endswith("\n") else want
    if check:
        if want != text:
            fail("portrait keep mask is out of date; run Tools/hex/gen_hex_art.py")
        print("gen_hex_art: portrait keep mask is up to date")
        return 0
    with open(HEX_ART_C, "w", encoding="utf-8") as f:
        f.write(want)
    print("gen_hex_art: wrote the portrait keep mask into %s" % HEX_ART_C)
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
