#!/usr/bin/env python3
"""
Build the Tech VeslyUI style from the vanilla UI graphics in ../Vanilla.

The shared redraw lives in ../style_tools.py; this script sets the look:
  * the palettes: dark glass panels in cyan / magenta / green / violet
    instead of vanilla's blue / red / green / sepia;
  * the frame: every window edge is one bright neon line, the same thickness
    on all four sides, with a dim glow outside it and no black outline;
    vanilla's thick top and bottom bars become panel behind that line;
  * corners are chamfered, the line cutting across them with a white-hot
    node; along the edges, dim segments, nodes, notches, raised tabs and tick
    marks;
  * scanlines on the fill colours, every other pixel row.

Outputs (next to this script):
  Tech_*.png             indexed images, read by Png2Dmp from the installer
  Tech_MenuPal.pal       8 menu palettes (4 window colours + 4 guide rows)
  Tech_BattlePal.pal     4 battle UI palettes (one per faction)
  Tech_PrepItemsPal.pal  prep-screen funds/item OBJ palette
  Tech_PlayStatusPal.pal chapter status banner and playtime box OBJ palettes

Rerun this script after changing a colour, then rebuild.
"""
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), ".."))
from style_tools import D, L, M, W, build_style, noise, outer_edges, paint, rgb, step  # noqa: E402
import style_tools  # noqa: E402

NAME = "Tech"
HERE = os.path.dirname(os.path.abspath(__file__))

NEON, GLOW = 2, 3      # menu palette entries vanilla used for gold trim
RAIL = 15              # darkest panel colour, behind the neon lines

# Each window colour: white, neon, dim neon, outline, then the light glass
# ramp (5-9, light -> dark) and the dark panel ramp (10-15, light -> dark).
COLOURS = [
    # Cyan: window colour 0, player faction.
    ["F0FCFF", "40F0FF", "2090B8", "040810",
     "D8F4FC", "C4ECF8", "ACE0F0", "94D2E8", "7CC4DC",
     "1C3450", "182C46", "14263C", "102032", "0C1A2A", "081422"],
    # Magenta: window colour 1, enemy faction.
    ["FFF0F8", "FF4C9C", "B02868", "100408",
     "FCE0EC", "F8CCE0", "F0B4D0", "E89CC0", "DC84B0",
     "3C1830", "341428", "2C1022", "240C1C", "1C0816", "140410"],
    # Green: window colour 2, allied (green) faction.
    ["F0FFF6", "48FF98", "20A860", "040C08",
     "D8FCE8", "C0F4D8", "A4ECC4", "88E0B0", "70D49C",
     "143C2C", "103426", "0C2C20", "0A241A", "081C14", "04140E"],
    # Violet: window colour 3, other (purple) faction.
    ["F8F0FF", "B488FF", "7050C0", "08040C",
     "ECE0FC", "DCCCF8", "C8B4F0", "B49CE8", "A084DC",
     "2C2048", "261C40", "201836", "1A142C", "141022", "0E0A18"],
]

# Prep funds/items OBJ palette (vanilla gUnknown_08A1B638): 2-9 dark -> light ramp.
PREP_ITEMS = ["7080A0", "081018", "0A1828", "0E2238", "122C48", "163858",
              "1C4868", "245A7C", "2E7090", "3C88A8", "060C14", "F0FCFF",
              "C8ECF8", "90C8E0", "5890B0", "000000"]

SCAN_RAMPS = {"ui": [list(range(5, 10)), list(range(10, 16))],
              "prep": [list(range(9, 1, -1))]}


def ui_row(colours, dim=1.0):
    row = [rgb("7080A0")] + [rgb(c) for c in colours]
    if dim != 1.0:
        row = [row[0]] + [tuple(int(v * dim) for v in c) for c in row[1:]]
    return row


def menu_palette():
    rows = [ui_row(c) for c in COLOURS]
    rows += [ui_row(c, 0.78) for c in COLOURS]
    return rows


def texture(image, kind, skip=frozenset()):
    """Scanlines: every other pixel row one step darker."""
    px = image.load()
    lookup = {}
    for ramp in SCAN_RAMPS[kind]:
        for i, index in enumerate(ramp):
            lookup[index] = (ramp, i)

    for y in range(1, image.height, 2):
        for x in range(image.width):
            index = px[x, y]
            if index not in lookup or (x, y) in skip:
                continue
            ramp, i = lookup[index]
            px[x, y] = ramp[i + 1] if i + 1 < len(ramp) else ramp[i - 1]


# --- Frame ------------------------------------------------------------------------
# Every window edge is one bright neon line, the same thickness on all four
# sides, with a dim glow just outside it and no black outline. Vanilla's thick
# top and bottom bars become panel behind that line.
OUTLINE = NEON
PANEL = 10


def beam(n, i, outer):
    """A bar n pixels thick. At a window edge: the neon line on the outer
    side, panel behind it. Between two panels: a neon line on a dark rail."""
    if outer == 0:
        return NEON if i == n // 2 else RAIL
    p = i if outer < 0 else n - 1 - i      # distance from the outer side
    return NEON if p == 0 else PANEL


# Big solid trim areas (patterned plates, the battle item boxes) become panel.
SOLID = {W: 11, L: 12, M: 15, D: 14}

# Chamfered corners: the neon line and its glow cut across the corner, with a
# white-hot node on the cut. Rows and columns match the window outline: top
# edges are row 4 (glow row 3), bottom edges row 4 (glow row 5), left edges
# x 6 (glow x 5), right edges x 2 (glow x 3).
CORNER_ART = {
    "TL": ["........",
           "........",
           "........",
           "........",
           ".......g",
           "......gW",
           ".....gWP",
           ".....gNP"],
    "TR": ["........",
           "........",
           "........",
           "g.......",
           "Ng......",
           "PWg.....",
           "PPWg....",
           "PPNg...."],
    "BL": [".....gNP",
           ".....gWP",
           "......gW",
           ".......g",
           "........",
           "........",
           "........",
           "........"],
    "BR": ["PPNg....",
           "PPWg....",
           "PWg.....",
           "Wg......",
           "g.......",
           "........",
           "........",
           "........"],
}
ART_INDEX = {".": 0, "N": NEON, "g": GLOW, "W": W, "P": PANEL}

# Pixels cut off a full-tile corner, as (x, y) for the top-left case.
CORNER_CUT = [(0, 0), (1, 0), (2, 0), (0, 1), (1, 1), (0, 2)]


def full_corner(tile, corner, drawn):
    """Chamfer the corner of a window that fills its whole tile."""
    flip_x, flip_y = corner.endswith("R"), corner[0] == "B"
    cut = [(7 - x if flip_x else x, 7 - y if flip_y else y) for x, y in CORNER_CUT]
    for x, y in cut:
        tile[y][x] = 0
    for x, y in cut:
        for nx, ny in ((x + 1, y), (x - 1, y), (x, y + 1), (x, y - 1)):
            if 0 <= nx < 8 and 0 <= ny < 8 and tile[ny][nx]:
                tile[ny][nx] = NEON
                drawn.add((nx, ny))


def clean_rim(tile, drawn):
    """Vanilla lights the inside of a window's left edge; tech keeps it panel."""
    for y in range(8):
        for x in range(8):
            if tile[y][x] != NEON:
                continue
            for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)):
                ox, oy, ix, iy, jx, jy = x - dx, y - dy, x + dx, y + dy, x + 2 * dx, y + 2 * dy
                if not all(0 <= v < 8 for v in (ox, oy, ix, iy)):
                    continue
                inner_next = tile[jy][jx] if 0 <= jx < 8 and 0 <= jy < 8 else PANEL
                if tile[oy][ox] == 0 and tile[iy][ix] == 9 and inner_next >= 10:
                    tile[iy][ix] = PANEL
                    drawn.add((ix, iy))


def add_glow(tile, drawn):
    """Dim glow on the backdrop just outside every lit edge pixel."""
    lit = [(x, y) for y in range(8) for x in range(8) if tile[y][x] in (NEON, W)]
    for x, y in lit:
        for nx, ny in ((x + 1, y), (x - 1, y), (x, y + 1), (x, y - 1)):
            if 0 <= nx < 8 and 0 <= ny < 8 and tile[ny][nx] == 0:
                tile[ny][nx] = GLOW
                drawn.add((nx, ny))


# Details on the interchangeable beam and edge tiles (style_tools.TOP_BEAMS
# and friends). Each: (kind, cell along the edge), cells counting from 0 at
# pixel 1.
EDGE_DETAILS = {
    # top edges
    0x04: [("notch", 1)],
    0x11: [("node", 4)],
    0x12: [("ticks", 1)],
    0x13: [("dim", 2)],
    0x32: [("tab", 1)],
    0x33: [("node", 2), ("dim", 4)],
    # bottom edges
    0x1C: [("ticks", 2)],
    0x1D: [("tab", 2)],
    0x1E: [("dim", 1)],
    0x20: [("notch", 2)],
    # left edges
    0x07: [("node", 2)],
    0x14: [("dim", 1)],
    0x0C: [("node", 3)],
    0x17: [("tab", 1)],
}


def detail(tile, cells, inward, kind, at, drawn):
    """One detail on the neon edge line: a dim segment, a white-hot node, a
    notch stepping the edge in, a raised tab stepping it out, or tick marks
    inside the panel."""
    if kind == "dim":
        for cell in cells[at:at + 2]:
            paint(tile, cell, GLOW, drawn)
    elif kind == "node":
        paint(tile, cells[at], W, drawn)
    elif kind == "notch":
        for cell in cells[at:at + 3]:
            tile[cell[1]][cell[0]] = 0
            drawn.discard(cell)
            paint(tile, step(cell, inward, 1), NEON, drawn)
    elif kind == "tab":
        for cell in cells[at:at + 3]:
            x, y = step(cell, inward, -1)
            if 0 <= x < 8 and 0 <= y < 8:
                tile[y][x] = NEON
                drawn.add((x, y))
            paint(tile, cell, PANEL, drawn)
    elif kind == "ticks":
        for cell in cells[at:at + 5:2]:
            paint(tile, step(cell, inward, 2), GLOW, drawn)


def edge_details(tile, t, drawn):
    """A fixed detail on EDGE_DETAILS tiles and an occasional node or dim
    segment on other frame tiles, then the glow around every lit edge."""
    clean_rim(tile, drawn)
    edges = outer_edges(tile, NEON)
    if t in EDGE_DETAILS and edges:
        cells, inward = edges[0]
        for kind, at in EDGE_DETAILS[t]:
            detail(tile, cells, inward, kind, at, drawn)
    elif t not in style_tools.DRAWUIFRAME_PLAIN:
        for n, (cells, inward) in enumerate(edges):
            roll = noise(t, n)
            if roll < 750:
                continue
            kind = "node" if roll < 880 else "dim"
            detail(tile, cells, inward, kind, roll % (len(cells) - 1), drawn)
    add_glow(tile, drawn)


# --- Chapter status banners -------------------------------------------------------
# The banners use the player palette, with the outline at 1 and white at 4
# (the shared code draws letter outlines with 1).
SIGN_PALETTE = [
    "7080A0", "040810", "40F0FF", "2090B8", "F0FCFF",
    "D8F4FC", "C4ECF8", "ACE0F0", "94D2E8", "7CC4DC",
    "1C3450", "182C46", "14263C", "102032", "0C1A2A", "081422",
]
MENU_TO_SIGN = {i: i for i in range(16)}
MENU_TO_SIGN[D], MENU_TO_SIGN[W] = 1, 4
# Vanilla PLAY TIME colours (OBJ palette 9) -> SIGN_PALETTE: white face,
# pale cyan shading, dim neon drop shadow.
LETTERS = {11: 4, 12: 4, 13: 6, 14: 3}


def label_tab(px, left, top, right, bottom):
    """A dark plate edged with the neon line and glow, corners chamfered."""
    corners = set()
    for cx, cy, dx, dy in ((left, top, 1, 1), (right, top, -1, 1),
                           (left, bottom, 1, -1), (right, bottom, -1, -1)):
        corners |= {(cx, cy), (cx + dx, cy), (cx, cy + dy)}
    for y in range(top, bottom + 1):
        for x in range(left, right + 1):
            if (x, y) in corners:
                continue
            edge = (x in (left, right) or y in (top, bottom)
                    or (x, y) in {(left + 1, top + 1), (right - 1, top + 1),
                                  (left + 1, bottom - 1), (right - 1, bottom - 1)})
            px[x, y] = 2 if edge else 15
    # Glow on the backdrop around the plate.
    for y in range(top - 1, bottom + 2):
        for x in range(left - 1, right + 2):
            if px[x, y] == 0 and any(px[x + dx, y + dy] == 2
                                     for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1))):
                px[x, y] = 3


if __name__ == "__main__":
    build_style(sys.modules[__name__])
