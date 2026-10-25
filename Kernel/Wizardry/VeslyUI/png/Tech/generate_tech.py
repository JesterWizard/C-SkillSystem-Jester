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
    node; the edges themselves stay plain and even;
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
from style_tools import D, L, M, W, build_style, rgb  # noqa: E402

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
# top and bottom bars are shared by dark and light panels, so the line sits
# just outside a 2-pixel band of panel on all four sides: invisible on dark
# panels, an even inset line on light ones. The rest of the bar is left
# transparent. Side lines sit one pixel outside vanilla's window outline and
# top and bottom lines one pixel inside it, which leaves the corner tile room
# for a rounded corner.
OUTLINE = NEON
PANEL = 10


def beam(n, i, outer):
    """A bar n pixels thick. At a window edge: the neon line and its band
    next to the panel, nothing outside them. Between two panels: a neon line
    on a dark rail."""
    if outer == 0:
        return NEON if i == n // 2 else RAIL
    if n == 1:
        return NEON
    p = i if outer < 0 else n - 1 - i      # distance from the outer side
    if p >= n - 2:
        return PANEL
    return NEON if p == n - 3 else 0


# Big solid trim areas (patterned plates, the battle item boxes) become panel.
SOLID = {W: 11, L: 12, M: 15, D: 14}

# Rounded corners: the lines cut across the corner in two white-hot steps,
# glow around them. Top edges are row 5 (glow row 4, band rows 6-7), bottom
# edges row 2 (glow row 3, band rows 0-1), left edges x 5 (glow x 4, band
# x 6-7), right edges x 3 (glow x 4, band x 1-2).
CORNER_ART = {
    "TL": ["........",
           "........",
           "........",
           "........",
           "........",
           ".......g",
           "......gW",
           ".....gWS"],
    "TR": ["........",
           "........",
           "........",
           "........",
           "g.......",
           "Ng......",
           "PWg.....",
           "SSWg...."],
    "BL": [".....gWP",
           "......gW",
           ".......g",
           "........",
           "........",
           "........",
           "........",
           "........"],
    "BR": ["PPWg....",
           "SWg.....",
           "Ng......",
           "g.......",
           "........",
           "........",
           "........",
           "........"],
}
# The "Sort" box and similar windows start with this top-left tile.
EXTRA_CORNERS = {0x61: "TL"}
# S is panel on an odd row, already in its scanline shade.
ART_INDEX = {".": 0, "N": NEON, "g": GLOW, "W": W, "P": PANEL, "S": PANEL + 1}

# Pixels cut off a full-tile corner, as (x, y) for the top-left case.
CORNER_CUT = [(0, 0), (1, 0), (2, 0), (0, 1), (1, 1), (0, 2)]


# Window style 1 fills its whole tiles, so nothing marks its edges as facing
# the backdrop. Its edge and corner tiles are drawn as plain panel with the
# neon line on their outer sides, matching the other windows.
FULL_TILE_EDGES = {0x73: "T", 0x75: "L", 0x76: "R", 0x78: "B"}


def flat_panel(tile, sides, drawn):
    for y in range(8):
        for x in range(8):
            edge = (("T" in sides and y == 0) or ("B" in sides and y == 7)
                    or ("L" in sides and x == 0) or ("R" in sides and x == 7))
            tile[y][x] = NEON if edge else PANEL
            if edge:
                drawn.add((x, y))


def full_corner(tile, corner, drawn):
    """Chamfer the corner of a window that fills its whole tile."""
    flat_panel(tile, corner, drawn)
    flip_x, flip_y = corner.endswith("R"), corner[0] == "B"
    cut = [(7 - x if flip_x else x, 7 - y if flip_y else y) for x, y in CORNER_CUT]
    for x, y in cut:
        tile[y][x] = 0
    for x, y in cut:
        for nx, ny in ((x + 1, y), (x - 1, y), (x, y + 1), (x, y - 1)):
            if 0 <= nx < 8 and 0 <= ny < 8 and tile[ny][nx]:
                tile[ny][nx] = NEON
                drawn.add((nx, ny))


def push_sides(tile, drawn):
    """Move each left and right edge line one pixel out, leaving a 2-pixel
    band of panel inside it (top and bottom bands come from beam()). The band
    stays out of drawn so the scanlines run through it."""
    lines = [(x, y, dx) for y in range(8) for x in range(8) for dx in (1, -1)
             if tile[y][x] == NEON and 0 <= x - dx < 8 and tile[y][x - dx] == 0]
    for x, y, dx in lines:
        tile[y][x - dx] = NEON
        drawn.add((x - dx, y))
        tile[y][x] = PANEL
        inner = x + dx
        if 0 <= inner < 8 and (tile[y][inner] == W or 5 <= tile[y][inner] <= 15):
            tile[y][inner] = PANEL


def add_glow(tile, drawn):
    """Dim glow on the backdrop just outside every lit edge pixel."""
    lit = [(x, y) for y in range(8) for x in range(8) if tile[y][x] in (NEON, W)]
    for x, y in lit:
        for nx, ny in ((x + 1, y), (x - 1, y), (x, y + 1), (x, y - 1)):
            if 0 <= nx < 8 and 0 <= ny < 8 and tile[ny][nx] == 0:
                tile[ny][nx] = GLOW
                drawn.add((nx, ny))


def edge_details(tile, t, drawn):
    """Plain, even edges: the neon line and its glow, nothing else."""
    if t in FULL_TILE_EDGES:
        flat_panel(tile, FULL_TILE_EDGES[t], drawn)
        return
    push_sides(tile, drawn)
    add_glow(tile, drawn)
    # Panel pixels (the bands) take the scanlines like the rest of the panel.
    drawn -= {(x, y) for y in range(8) for x in range(8) if tile[y][x] == PANEL}


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
# The rest of the sheet (a spare banner with dragons, and some icons), from
# vanilla's OBJ palette 8 roles to SIGN_PALETTE, drawn like a hologram: the
# green ramp (2-9) becomes dark navy panel, the dark artwork and outlines
# (10) glowing cyan, the cream trim neon, the greys glass and the blue gem
# white-hot.
REST_OF_SHEET = {
    0: 0, 1: 2, 2: 15, 3: 15, 4: 14, 5: 14, 6: 13, 7: 12, 8: 11, 9: 10,
    10: 3, 11: 8, 12: 6, 13: 4, 14: 1, 15: 5,
}


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
