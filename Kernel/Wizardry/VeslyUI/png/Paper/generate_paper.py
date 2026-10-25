#!/usr/bin/env python3
"""
Build the Paper (storybook) VeslyUI style from the vanilla UI graphics in
../Vanilla.

The shared redraw lives in ../style_tools.py; this script sets the look:
  * the palettes: sky blue / tomato / grass / plum construction paper and
    cream paper light panels, instead of vanilla's blue / red / green / sepia;
  * the frame: a paper cutout, like a sticker: a thick black outline, a white
    paper border and its cardstock edge, the same on all four sides;
  * rounded corners, and a drop shadow down the right and along the bottom,
    as if each window were a cutout lifted off the page;
  * a faint paper-fibre speckle on the panels.

Outputs (next to this script):
  Paper_*.png             indexed images, read by Png2Dmp from the installer
  Paper_MenuPal.pal       8 menu palettes (4 window colours + 4 guide rows)
  Paper_BattlePal.pal     4 battle UI palettes (one per faction)
  Paper_PrepItemsPal.pal  prep-screen funds/item OBJ palette
  Paper_PlayStatusPal.pal chapter status banner and playtime box OBJ palettes

Rerun this script after changing a colour, then rebuild.
"""
import math
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), ".."))
from style_tools import D, L, M, W, build_style, rgb  # noqa: E402
import style_tools  # noqa: E402

NAME = "Paper"
HERE = os.path.dirname(os.path.abspath(__file__))

# Menu palette entries. White (1) is the paper border (and the battle text);
# 2 and 3 (vanilla's gold) hold the border's cardstock edge and the drop
# shadow; 4 is the black outline.
EDGE, SHADOW = 2, 3
PANEL = 10

TRIM = ["FFFFFF", "D0C8C8", "3C3438", "201818"]     # 1 white, 2 edge, 3 shadow, 4 outline
CREAM = ["FFF8E8", "F8F0D8", "F0E4C4", "E4D4B0", "D8C49C"]   # 5-9

PAPERS = [
    # Sky blue paper: window colour 0, player faction.
    ["4070C8", "3A68BC", "3460B0", "2E58A4", "285098", "22488C"],
    # Tomato paper: window colour 1, enemy faction.
    ["C84838", "BC4032", "B0382C", "A03026", "902820", "802018"],
    # Grass paper: window colour 2, allied (green) faction.
    ["449838", "3C8C30", "348028", "2C7422", "26681C", "205C16"],
    # Plum paper: window colour 3, other (purple) faction.
    ["8848A8", "7E409C", "743890", "6A3084", "602878", "56206C"],
]

# Prep funds/items OBJ palette (vanilla gUnknown_08A1B638): 2-9 dark -> light ramp.
PREP_ITEMS = ["7080A0", "201818", "283C70", "305494", "3860A4", "4878C4",
              "5890E0", "78A8E8", "A0C4F0", "C8DCF8", "181010", "FFFFFF",
              "F8F0D8", "D0C8C8", "3C3438", "000000"]

FIBRE_RAMPS = {"ui": [[5, 6, 7, 8, 9], list(range(10, 16))],
               "prep": [list(range(9, 1, -1))]}

# Paper fibres: a few scattered pixels per 8x8 tile, one shade darker.
FIBRES = {(1, 1), (6, 2), (3, 4), (0, 6), (5, 6), (7, 4)}


def ui_row(paper, dim=1.0):
    row = ([rgb("7080A0")] + [rgb(c) for c in TRIM] + [rgb(c) for c in CREAM]
           + [rgb(c) for c in paper])
    if dim != 1.0:
        row = [row[0]] + [tuple(int(v * dim) for v in c) for c in row[1:]]
    return row


def menu_palette():
    rows = [ui_row(p) for p in PAPERS]
    rows += [ui_row(p, 0.78) for p in PAPERS]
    return rows


def texture(image, kind, skip=frozenset()):
    """Paper fibres: a few pixels per tile one shade darker."""
    px = image.load()
    lookup = {}
    for ramp in FIBRE_RAMPS[kind]:
        for i, index in enumerate(ramp):
            lookup[index] = (ramp, i)

    for y in range(image.height):
        for x in range(image.width):
            index = px[x, y]
            if index not in lookup or (x % 8, y % 8) not in FIBRES or (x, y) in skip:
                continue
            ramp, i = lookup[index]
            px[x, y] = ramp[i + 1] if i + 1 < len(ramp) else ramp[i - 1]


# --- Frame ------------------------------------------------------------------------
# A 4-pixel border on every side, from the outside in: black outline (2
# pixels), white paper, cardstock edge. Top and bottom bars hold it inside
# vanilla's window outline; side edges push it outward (left edges start at
# x 3, right edges end at x 5), so all four sides match. The drop shadow is
# the window's shape moved 2 pixels right and down.
CUTOUT = [D, D, W, EDGE]        # outside -> inside
SHADOW_OFFSET = 2
RADIUS = 4


def beam(n, i, outer):
    """A bar n pixels thick. At a window edge: the cutout border against the
    panel, nothing outside it. Between two panels: a white paper strip in
    black."""
    if outer == 0:
        return D if i in (0, n - 1) or n < 3 else W
    q = n - 1 - i if outer < 0 else i      # distance from the panel side
    return CUTOUT[3 - q] if q < 4 else 0


# Big solid trim areas (patterned plates, the battle item boxes) become panel.
SOLID = {W: 11, L: 12, M: 15, D: 14}

# Each corner tile's window outline: the outer edge lines, as continuous
# coordinates, and which side of them is inside. Left edges start at x 3,
# right edges end at x 5 (so x < 6), top edges at y 4, bottom edges end at
# y 3 (so y < 4).
CORNER_EDGES = {
    "TL": (3.0, 4.0, 1, 1),
    "TR": (6.0, 4.0, -1, 1),
    "BL": (3.0, 4.0, 1, -1),
    "BR": (6.0, 4.0, -1, -1),
}


def depth(corner, x, y):
    """How far pixel centre (x, y) lies inside the rounded window outline of
    a corner tile; negative outside."""
    ex, ey, sx, sy = CORNER_EDGES[corner]
    px, py = x + 0.5, y + 0.5
    dx, dy = (px - ex) * sx, (py - ey) * sy
    if dx < RADIUS and dy < RADIUS:
        return RADIUS - math.hypot(RADIUS - dx, RADIUS - dy)
    return min(dx, dy)


def corner_tile(corner):
    tile = [[0] * 8 for _ in range(8)]
    for y in range(8):
        for x in range(8):
            d = depth(corner, x, y)
            if d >= 0:
                layer = int(d)
                tile[y][x] = CUTOUT[layer] if layer < 4 else PANEL
            elif depth(corner, x - SHADOW_OFFSET, y - SHADOW_OFFSET) >= 0:
                tile[y][x] = SHADOW
    return tile


ART_NAMES = {0: ".", D: "D", W: "W", EDGE: "e", SHADOW: "s", PANEL: "P"}
CORNER_ART = {c: ["".join(ART_NAMES[v] for v in row) for row in corner_tile(c)]
              for c in CORNER_EDGES}
ART_INDEX = {v: k for k, v in ART_NAMES.items()}

# The "Sort" box and similar windows start with this top-left tile.
EXTRA_CORNERS = {0x61: "TL"}

# Window style 1 fills its whole tiles; its edge and corner tiles get the
# cutout border along their outer sides (no room for a shadow there).
FULL_TILE_EDGES = {0x73: "T", 0x75: "L", 0x76: "R", 0x78: "B"}


def full_frame(tile, sides, drawn):
    for y in range(8):
        for x in range(8):
            d = min({"T": y, "B": 7 - y, "L": x, "R": 7 - x}[s] for s in sides)
            tile[y][x] = CUTOUT[d] if d < 4 else PANEL
            if d < 4:
                drawn.add((x, y))


def full_corner(tile, corner, drawn):
    full_frame(tile, corner, drawn)


def push_sides(tile, drawn):
    """Turn each left and right outline pixel into the full border, pushed
    outward so it is 4 pixels thick like the top and bottom bars."""
    lines = [(x, y, dx) for y in range(8) for x in range(8) for dx in (1, -1)
             if tile[y][x] == D and 0 <= x - dx < 8 and tile[y][x - dx] == 0]
    for x, y, dx in lines:
        for q in range(4):                      # panel side -> outside
            px_ = x - dx * q
            if 0 <= px_ < 8:
                tile[y][px_] = CUTOUT[3 - q]
                drawn.add((px_, y))
        inner = x + dx
        if 0 <= inner < 8 and tile[y][inner] == 9:          # vanilla's edge light
            tile[y][inner] = PANEL


def drop_shadow(tile, drawn):
    """The tile's shape moved 2 pixels right and down, on the backdrop. Off
    the tile's top or left edge the shape is taken to carry on straight."""
    shape = [[v != 0 for v in row] for row in tile]
    for y in range(8):
        for x in range(8):
            if shape[y][x]:
                continue
            sx, sy = max(x - SHADOW_OFFSET, 0), max(y - SHADOW_OFFSET, 0)
            if shape[sy][sx]:
                tile[y][x] = SHADOW
                drawn.add((x, y))


def edge_details(tile, t, drawn):
    """Even cutout borders and their drop shadow."""
    if t in FULL_TILE_EDGES:
        full_frame(tile, FULL_TILE_EDGES[t], drawn)
        return
    push_sides(tile, drawn)
    drop_shadow(tile, drawn)


# --- Chapter status banners -------------------------------------------------------
# The banners use the player palette's paper, white, edge, shadow and outline
# (at 1, as the shared code draws letter outlines with 1). They stay off the
# entries that hold the icons' original colours (style_tools.VANILLA_ICON_SLOTS);
# cream reuses the icons' cream.
SIGN_PALETTE = ["7080A0", "201818", "D0C8C8", "3C3438", "FFFFFF",
                None, None, None, None, None, "4070C8", "3A68BC",
                None, None, None, None]
for slot, colour in style_tools.VANILLA_ICON_SLOTS.items():
    SIGN_PALETTE[slot] = colour
MENU_TO_SIGN = {0: 0, D: 1, W: 4, EDGE: 2, SHADOW: 3,
                5: 5, 6: 5, 7: 5, 8: 5, 9: 5,
                10: 10, 11: 11, 12: 11, 13: 11, 14: 10, 15: 10}
# Vanilla PLAY TIME colours (OBJ palette 9) -> SIGN_PALETTE: white face, grey
# shading, black drop shadow.
LETTERS = {11: 4, 12: 4, 13: 2, 14: 1}
# The rest of the sheet (a spare banner with dragons): blue paper ground,
# white paper dragons, black trim.
REST_OF_SHEET = {
    0: 0, 1: 1, 2: 11, 3: 11, 4: 11, 5: 11, 6: 10, 7: 10, 8: 10, 9: 10,
    10: 4, 11: 2, 12: 2, 13: 15, 14: 1, 15: 2,
}
# The two item icons beside the banner keep vanilla's own colours.
ICON_COLOURS = style_tools.VANILLA_ICON_COLOURS


def label_tab(px, left, top, right, bottom):
    """A little paper tab: black outline, white border, blue paper."""
    for y in range(top, bottom + 1):
        for x in range(left, right + 1):
            d = min(x - left, right - x, y - top, bottom - y)
            px[x, y] = 1 if d == 0 else 4 if d == 1 else 10
    for cx, cy in ((left, top), (right, top), (left, bottom), (right, bottom)):
        px[cx, cy] = 0


if __name__ == "__main__":
    build_style(sys.modules[__name__])
