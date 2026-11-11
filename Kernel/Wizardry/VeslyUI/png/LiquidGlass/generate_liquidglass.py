#!/usr/bin/env python3
"""
Build the Liquid Glass VeslyUI style from the vanilla UI graphics in ../Vanilla.

The shared redraw lives in ../style_tools.py; this script sets the look, a
take on "liquid glass": windows are tinted glass the map shows through (the
see-through part is alpha blending, set in C_Code.c, not in these images):
  * the palettes: smooth tinted glass in blue / rose / green / violet, with
    light frosted glass for the light panels (title plates, help boxes);
  * the frame: every window edge is one thin rim, bright (a specular
    highlight) on the top and left edges and dimmer on the bottom and right,
    with no black outline and no glow; vanilla's thick top and bottom bars
    become panel inside that rim;
  * rounded corners;
  * no texture: the glass is smooth.

Outputs (next to this script):
  LiquidGlass_*.png             indexed images, read by Png2Dmp from the installer
  LiquidGlass_MenuPal.pal       8 menu palettes (4 window colours + 4 guide rows)
  LiquidGlass_BattlePal.pal     4 battle UI palettes (one per faction)
  LiquidGlass_PrepItemsPal.pal  prep-screen funds/item OBJ palette
  LiquidGlass_PlayStatusPal.pal chapter status banner and playtime box OBJ palettes

Rerun this script after changing a colour, then rebuild.
"""
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), ".."))
from style_tools import D, L, M, W, build_style, rgb  # noqa: E402
import style_tools  # noqa: E402

NAME = "LiquidGlass"
HERE = os.path.dirname(os.path.abspath(__file__))

NEON, GLOW = 2, 3      # the rim: bright (top, left) and dim (bottom, right); vanilla's gold trim entries
RAIL = 15              # darkest panel colour, behind a rim between two panels

# Each window colour: white, bright rim, dim rim, outline, then the light frosted
# glass ramp (5-9, light -> dark) and the tinted glass ramp (10-15, light -> dark).
COLOURS = [
    # Blue: window colour 0, player faction.
    ["FFFFFF", "E4F0FF", "7C98C8", "0A1020",
     "EEF4FC", "E2ECF8", "D4E2F2", "C6D8EC", "B8CEE6",
     "3A5078", "344A70", "2E4468", "2A3E60", "263858", "203250"],
    # Rose: window colour 1, enemy faction.
    ["FFFFFF", "FFE8EE", "C88094", "200A10",
     "FCF0F2", "F8E4E8", "F2D6DC", "ECC8D0", "E6BAC4",
     "6A3448", "622E42", "5A2A3C", "522636", "4A2230", "421E2A"],
    # Green: window colour 2, allied (green) faction.
    ["FFFFFF", "E4FFF0", "78B898", "0A1A12",
     "EEFAF4", "E2F6EA", "D4F0E0", "C6EAD6", "B8E4CC",
     "2E5A48", "2A5442", "264C3C", "224636", "1E4030", "1A382A"],
    # Violet: window colour 3, other (purple) faction.
    ["FFFFFF", "F0E8FF", "9C88C8", "100A20",
     "F4F0FC", "ECE4F8", "E2D8F2", "D8CCEC", "CEC0E6",
     "4C3C74", "46366C", "403264", "3A2E5C", "342854", "2E244C"],
]

# Prep funds/items OBJ palette (vanilla gUnknown_08A1B638): 2-9 dark -> light ramp.
PREP_ITEMS = ["7080A0", "1A2840", "203250", "263858", "2C4062", "344A70",
              "3E5680", "4C6690", "5C78A0", "7090B4", "0A1020", "FFFFFF",
              "E4F0FF", "B8CEE6", "7C98C8", "000000"]

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
    """No texture: the glass is smooth."""


# --- Frame ------------------------------------------------------------------------
# Every window edge is one thin rim, the same thickness on all four sides, with no
# black outline and no glow. It is bright on the top and left edges (the light
# catching the glass) and dimmer on the bottom and right. Vanilla's thick top and
# bottom bars are shared by dark and light panels, so the rim sits just outside a
# 2-pixel band of panel on all four sides, and the rest of the bar is left
# transparent. Side rims sit one pixel outside vanilla's window outline and top
# and bottom rims one pixel inside it, which leaves the corner tile room for a
# rounded corner (the same geometry as the Tech style).
OUTLINE = NEON
PANEL = 10


def beam(n, i, outer):
    """A bar n pixels thick. At a window edge: the rim (bright on top, dim at the
    bottom) and its band next to the panel, nothing outside them. Between two
    panels: a bright rim on a dark rail."""
    if outer == 0:
        return NEON if i == n // 2 else RAIL
    if n == 1:
        return NEON if outer < 0 else GLOW
    p = i if outer < 0 else n - 1 - i      # distance from the outer side
    if p >= n - 2:
        return PANEL
    if p == n - 3:
        return NEON if outer < 0 else GLOW
    return 0


# Big solid trim areas (patterned plates, the battle item boxes) become panel.
SOLID = {W: 11, L: 12, M: 15, D: 14}

# Rounded corners: top rims are row 5 (band rows 6-7), bottom rims row 2 (band
# rows 0-1), left rims x 5 (band x 6-7), right rims x 3 (band x 1-2). R is the
# bright rim, D the dim one; each corner turns from one to the other.
CORNER_ART = {
    "TL": ["........",
           "........",
           "........",
           "........",
           "........",
           "......RR",
           ".....RPP",
           ".....RPP"],
    "TR": ["........",
           "........",
           "........",
           "........",
           "........",
           "RR......",
           "PPD.....",
           "PPPD...."],
    "BL": [".....RPP",
           ".....RPP",
           "......DD",
           "........",
           "........",
           "........",
           "........",
           "........"],
    "BR": ["PPPD....",
           "PPD.....",
           "DD......",
           "........",
           "........",
           "........",
           "........",
           "........"],
}
# The "Sort" box and similar windows start with this top-left tile.
EXTRA_CORNERS = {0x61: "TL"}
ART_INDEX = {".": 0, "R": NEON, "D": GLOW, "P": PANEL}

# Pixels cut off a full-tile corner, as (x, y) for the top-left case.
CORNER_CUT = [(0, 0), (1, 0), (2, 0), (0, 1), (1, 1), (0, 2)]


# Window style 1 fills its whole tiles, so nothing marks its edges as facing
# the backdrop. Its edge and corner tiles are drawn as plain panel with the
# rim on their outer sides, matching the other windows.
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
    """Round off the corner of a window that fills its whole tile."""
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


def shade_rims(tile):
    """A rim pixel with the backdrop below it or to its right faces down or right:
    it takes the dim rim colour; the rest stay bright."""
    for y in range(8):
        for x in range(8):
            if tile[y][x] not in (NEON, GLOW):
                continue
            below = y + 1 < 8 and tile[y + 1][x] == 0
            right = x + 1 < 8 and tile[y][x + 1] == 0
            above = y > 0 and tile[y - 1][x] == 0
            left = x > 0 and tile[y][x - 1] == 0
            if (below or right) and not (above or left):
                tile[y][x] = GLOW
            elif (above or left) and not (below or right):
                tile[y][x] = NEON


def edge_details(tile, t, drawn):
    """Plain, even edges: the rim, bright on top and left, dim on bottom and right."""
    if t in FULL_TILE_EDGES:
        flat_panel(tile, FULL_TILE_EDGES[t], drawn)
        return
    push_sides(tile, drawn)
    shade_rims(tile)


# --- Chapter status banners -------------------------------------------------------
# The banners use the player palette's glass, rims and white, with the outline at
# 1 and white at 4 (the shared code draws letter outlines with 1). They are kept
# to entries 0-4, 6, 10 and 11, so the other eight hold the icons' colours.
SIGN_PALETTE = [
    "7080A0",  # 0 transparent
    "0A1020",  # 1 outline
    "E4F0FF",  # 2 bright rim
    "7C98C8",  # 3 dim rim
    "FFFFFF",  # 4 white
    "E8D8C8",  # 5 icon cream (vanilla)
    "D4E2F2",  # 6 frosted glass: letter shading, icon halo
    "205028",  # 7-9 icon greens (vanilla)
    "306038",
    "406840",
    "3A5078",  # 10 glass
    "2E4468",  # 11 glass, darker
    "507850",  # 12-14 icon greens (vanilla)
    "608860",
    "98B090",
    "70A8C0",  # 15 icon gem (vanilla)
]
# Frame tile colours -> SIGN_PALETTE: the tinted glass ramp folds into the two
# glass shades, frosted glass into 6.
MENU_TO_SIGN = {0: 0, D: 1, W: 4, NEON: 2, GLOW: 3,
                5: 6, 6: 6, 7: 6, 8: 6, 9: 6,
                10: 10, 11: 11, 12: 11, 13: 11, 14: 10, 15: 10}
# Vanilla PLAY TIME colours (OBJ palette 9) -> SIGN_PALETTE: white face,
# frosted shading, dim rim drop shadow.
LETTERS = {11: 4, 12: 4, 13: 6, 14: 3}
# The rest of the sheet (a spare banner with dragons), from vanilla's OBJ
# palette 8 roles to SIGN_PALETTE: the green ramp (2-9) becomes glass, the dark
# artwork and outlines (10) the dim rim, the cream trim the bright rim, the greys
# frosted glass and the blue gem white.
REST_OF_SHEET = {
    0: 0, 1: 2, 2: 11, 3: 11, 4: 11, 5: 11, 6: 10, 7: 10, 8: 10, 9: 10,
    10: 3, 11: 6, 12: 6, 13: 4, 14: 1, 15: 6,
}
# The two item icons beside the banner keep vanilla's own colours.
ICON_COLOURS = style_tools.VANILLA_ICON_COLOURS


def label_tab(px, left, top, right, bottom):
    """A glass plate with a rim, bright on top and left and dim on the bottom
    and right, corners rounded."""
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
            if not edge:
                px[x, y] = 10
            elif x == right or y == bottom or (x, y) in {(right - 1, top + 1), (left + 1, bottom - 1),
                                                        (right - 1, bottom - 1)}:
                px[x, y] = 3
            else:
                px[x, y] = 2


if __name__ == "__main__":
    build_style(sys.modules[__name__])
