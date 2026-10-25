#!/usr/bin/env python3
"""
Build the Wood VeslyUI style from the vanilla UI graphics in ../Vanilla.

The shared redraw lives in ../style_tools.py; this script sets the look:
  * the palettes: oak / mahogany / moss-stained / weathered wood instead of
    vanilla's blue / red / green / sepia;
  * the frame: vanilla's gold bars with a white stripe become dark wooden
    beams with a black outline, and every window edge gets that outline;
  * chips and cracks break up the edges, and at window corners the planks
    run past the window and end in a broken, splintered tip;
  * a wood-grain pattern on the fill colours. The pattern repeats every 8x8
    tile, so tiles the game repeats side by side join into continuous grain.

Outputs (next to this script):
  Wood_*.png             indexed images, read by Png2Dmp from the installer
  Wood_MenuPal.pal       8 menu palettes (4 window colours + 4 guide rows)
  Wood_BattlePal.pal     4 battle UI palettes (one per faction)
  Wood_PrepItemsPal.pal  prep-screen funds/item OBJ palette
  Wood_PlayStatusPal.pal chapter status banner and playtime box OBJ palettes

Rerun this script after changing a colour, then rebuild.
"""
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), ".."))
from style_tools import D, L, M, W, build_style, noise, outer_edges, paint, rgb, step  # noqa: E402
import style_tools  # noqa: E402

NAME = "Wood"
HERE = os.path.dirname(os.path.abspath(__file__))

# The redrawn art no longer uses 2 and 3 (vanilla's gold); they stay a light
# and mid wood for any other graphics that share this palette. 4 is the outline.
TRIM = ["7080A0", "F8F0D8", "E2C490", "A07040", "24140A"]

WOODS = [
    # Oak: window colour 0, player faction.
    ["F4DCA8", "ECCC94", "E2BC80", "D6A86A", "C89456",
     "AC7038", "9C6030", "8C522A", "7C4624", "6C3A1C", "5C3016"],
    # Mahogany: window colour 1, enemy faction.
    ["F2D2BC", "EAC2A6", "E0B092", "D49A7C", "C68466",
     "A44C2C", "944026", "843620", "742E1C", "642616", "541E12"],
    # Moss-stained: window colour 2, allied (green) faction.
    ["E6E2B4", "DCD8A2", "CECC90", "C0BE7E", "B0AE6C",
     "727C3C", "646E34", "56602C", "4A5226", "3E4420", "323618"],
    # Weathered: window colour 3, other (purple) faction.
    ["EAE2D2", "DFD6C4", "D2C8B6", "C4BAA8", "B6AC9A",
     "8E8272", "7E7264", "6E6456", "60564A", "52483E", "443C34"],
]

# Prep funds/items OBJ palette (vanilla gUnknown_08A1B638): 2-9 dark -> light ramp.
PREP_ITEMS = ["7080A0", "3A2614", "4A2E16", "5C3A1C", "6E4824", "80582E",
              "94683A", "A87C48", "BC9058", "CEA46A", "382818", "F8F0D8",
              "EAD8B4", "C0A47C", "8C7454", "000000"]

GRAIN_RAMPS = {"ui": [list(range(5, 10)), list(range(10, 16))],
               "prep": [list(range(9, 1, -1))]}

# Tile-local grain: two wavy lines per 8x8 tile.
GRAIN_WAVE = [0, 0, 1, 1, 1, 1, 0, 0]


def ui_row(wood, dim=1.0):
    row = [rgb(c) for c in TRIM] + [rgb(c) for c in wood]
    if dim != 1.0:
        row = [row[0]] + [tuple(int(v * dim) for v in c) for c in row[1:]]
    return row


def menu_palette():
    rows = [ui_row(w) for w in WOODS]
    rows += [ui_row(w, 0.78) for w in WOODS]
    return rows


def is_grain_line(x, y):
    return (y - GRAIN_WAVE[x % 8]) % 4 == 1


def texture(image, kind, skip=frozenset()):
    """Wood grain: one step darker along two wavy lines per tile."""
    px = image.load()
    lookup = {}
    for ramp in GRAIN_RAMPS[kind]:
        for i, index in enumerate(ramp):
            lookup[index] = (ramp, i)

    for y in range(image.height):
        for x in range(image.width):
            index = px[x, y]
            if index not in lookup or not is_grain_line(x, y) or (x, y) in skip:
                continue
            ramp, i = lookup[index]
            # One step darker; the darkest shade steps lighter instead.
            px[x, y] = ramp[i + 1] if i + 1 < len(ramp) else ramp[i - 1]


# --- Frame ------------------------------------------------------------------------
def beam(n, i, outer):
    """Colour of position i across a beam n pixels thick: outlined dark wood."""
    if i == 0 or i == n - 1:
        return D
    inner = {3: [12], 4: [12, 14], 5: [12, 13, 14]}.get(n)
    if inner is None:
        inner = [12, 13] + [14] * (n - 5) + [15]
    return inner[i - 1]


# Big solid trim areas (patterned plates) keep their pattern in wood tones.
SOLID = {W: 11, L: 12, M: 15, D: 14}

# The top and bottom planks run past the window and end in a broken,
# splintered tip. Plank shading matches beam(): top planks are 4 pixels thick
# (rows 4-7), bottom planks 5 (rows 0-4). p and q are pale raw wood.
CORNER_ART = {
    "TL": ["........",
           "........",
           "........",
           "........",
           "...DDDDD",
           ".DDppHHH",
           "..DqJJJJ",
           ".DDDDDDD"],
    "TR": ["........",
           "........",
           "........",
           "........",
           "DDDDDD..",
           "HHHHpD..",
           "JJJppDD.",
           "DDDDD..."],
    "BL": ["...DDDDD",
           "..DpHHHH",
           ".DqpIIII",
           "DDDqJJJJ",
           "...DDDDD",
           "........",
           "........",
           "........"],
    "BR": ["DDDDD...",
           "HHHHpD..",
           "IIIIqpD.",
           "JJJqDD..",
           "DDDDD...",
           "........",
           "........",
           "........"],
}
ART_INDEX = {".": 0, "D": D, "H": 12, "I": 13, "J": 14, "p": 7, "q": 8}
PALE = 7

# Pixels knocked off a full-tile corner, as (x, y) for the top-left case.
CORNER_CHIP = [(0, 0), (1, 0), (2, 0), (0, 1), (1, 1), (0, 2)]


def full_corner(tile, corner, drawn):
    """Chip the corner off a window that fills its whole tile."""
    flip_x, flip_y = corner.endswith("R"), corner[0] == "B"
    cut = [(7 - x if flip_x else x, 7 - y if flip_y else y) for x, y in CORNER_CHIP]
    for x, y in cut:
        tile[y][x] = 0
    for x, y in cut:
        for nx, ny in ((x + 1, y), (x - 1, y), (x, y + 1), (x, y - 1)):
            if 0 <= nx < 8 and 0 <= ny < 8 and tile[ny][nx]:
                tile[ny][nx] = D
                drawn.add((nx, ny))


# Chips drawn on the interchangeable beam and edge tiles (style_tools.TOP_BEAMS
# and friends). Each chip: (kind, cell along the edge), cells counting from 0
# at pixel 1. A streak is a grain line in the beam, not damage.
EDGE_CHIPS = {
    # top beams
    0x03: [("streak", 1)],
    0x04: [("dent", 1), ("streak", 4)],
    0x11: [("nick", 4)],
    0x12: [("chunk", 2)],
    0x13: [("streak", 0), ("nick", 5)],
    0x31: [("dent", 3)],
    0x32: [("crack", 3)],
    0x33: [("streak", 2)],
    # bottom beams
    0x1B: [("streak", 2)],
    0x1C: [("chunk", 1)],
    0x1D: [("dent", 3), ("streak", 0)],
    0x1E: [("nick", 1), ("streak", 3)],
    0x20: [("crack", 1), ("nick", 5)],
    # left edges
    0x07: [("dent", 1)],
    0x14: [("nick", 4)],
    0x0C: [("dent", 3)],
    0x17: [("nick", 1)],
}


def chip(tile, cells, inward, kind, at, drawn):
    """Draw one chip on an outer edge: a dent, a deeper chunk, or a crack."""
    if kind in ("dent", "nick"):
        width = 2 if kind == "dent" else 1
        hole = cells[at:at + width]
        for cell in hole:
            tile[cell[1]][cell[0]] = 0
        for cell in hole:
            paint(tile, step(cell, inward, 1), D, drawn)
        # Raw wood shows on the wall where the piece broke away.
        if kind == "dent" and at + width < len(cells):
            paint(tile, step(cells[at + width], inward, 1), PALE, drawn)
    elif kind == "chunk":
        a, b, c = cells[at:at + 3]
        for cell in (a, b, c, step(b, inward, 1)):
            tile[cell[1]][cell[0]] = 0
        paint(tile, step(a, inward, 1), D, drawn)
        paint(tile, step(c, inward, 1), D, drawn)
        paint(tile, step(b, inward, 2), D, drawn)
        paint(tile, step(a, inward, 2), PALE, drawn)
        paint(tile, step(c, inward, 2), PALE + 1, drawn)
    elif kind == "crack":
        paint(tile, step(cells[at], inward, 1), D, drawn)
        paint(tile, step(cells[at + 1], inward, 2), D, drawn)
    elif kind == "streak":
        for cell in cells[at:at + 3]:
            paint(tile, step(cell, inward, 1), 13, drawn)


def edge_details(tile, t, drawn):
    """Chip a tile's outer edge: a fixed chip for EDGE_CHIPS tiles, otherwise
    an occasional small dent so plates and boxes look worn too."""
    edges = outer_edges(tile)
    if not edges:
        return
    if t in EDGE_CHIPS:
        cells, inward = edges[0]
        for kind, at in EDGE_CHIPS[t]:
            chip(tile, cells, inward, kind, at, drawn)
        return
    if t in style_tools.DRAWUIFRAME_PLAIN:
        return
    for n, (cells, inward) in enumerate(edges):
        roll = noise(t, n)
        if roll < 700:
            continue
        kind = "dent" if roll < 850 else "nick"
        chip(tile, cells, inward, kind, roll % (len(cells) - 1), drawn)


# --- Chapter status banners -------------------------------------------------------
SIGN_PALETTE = [
    "7080A0",  # 0 transparent
    "24140A",  # 1 outline
    "4A2812",  # 2 darkest wood
    "5C3016",  # 3 plank, dark
    "6C3A1C",  # 4 plank, mid
    "7C4624",  # 5 plank, light
    "8C522A",  # 6 board
    "7E4826",  # 7 board grain
    "C08A50",  # 8 raw wood, dark
    "DCB278",  # 9 raw wood
    "F8F0D8",  # 10 lettering
    "C8B090",  # 11 lettering shade
    "A86C3A",  # 12 edge highlight
    "E8C890",  # 13 raw wood, light
    "3A200E",  # 14 unused
    "9A6438",  # 15 lettering drop shadow
]
# Frame tile colours (menu palette indices) -> SIGN_PALETTE.
MENU_TO_SIGN = {0: 0, D: 1, 1: 10, 5: 13, 6: 13, 7: 9, 8: 8, 9: 12,
                10: 6, 11: 7, 12: 5, 13: 4, 14: 3, 15: 2}
# Vanilla PLAY TIME colours (OBJ palette 9) -> SIGN_PALETTE.
LETTERS = {11: 10, 12: 10, 13: 11, 14: 15}


def label_tab(px, left, top, right, bottom):
    """A small plank over the box's top beam, with broken corners."""
    for y in range(top, bottom + 1):
        for x in range(left, right + 1):
            edge = x in (left, right) or y in (top, bottom)
            px[x, y] = 1 if edge else (5 if y == top + 1 else 4)
    for x, y in ((left, top), (right, top), (left, bottom), (right, bottom)):
        px[x, y] = 0
    px[left + 1, top + 1] = 1
    px[right - 1, bottom - 1] = 1
    px[right - 2, top + 1] = 9


if __name__ == "__main__":
    build_style(sys.modules[__name__])
