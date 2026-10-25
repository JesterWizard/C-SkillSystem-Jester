#!/usr/bin/env python3
"""
Build the Ocean / Coral VeslyUI style from the vanilla UI graphics in
../Vanilla.

The shared redraw lives in ../style_tools.py; this script sets the look:
  * the palettes: teal / coral / kelp green / pearl water panels and sand
    light panels, instead of vanilla's blue / red / green / sepia;
  * the frame: a twisted rope edge outside a band of wave crests, 4 pixels
    thick and the same on all four sides;
  * a starfish on each window corner;
  * soft bubbles drifting across the panels.

Outputs (next to this script):
  Ocean_*.png             indexed images, read by Png2Dmp from the installer
  Ocean_MenuPal.pal       8 menu palettes (4 window colours + 4 guide rows)
  Ocean_BattlePal.pal     4 battle UI palettes (one per faction)
  Ocean_PrepItemsPal.pal  prep-screen funds/item OBJ palette
  Ocean_PlayStatusPal.pal chapter status banner and playtime box OBJ palettes

Rerun this script after changing a colour, then rebuild.
"""
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), ".."))
from style_tools import D, L, M, W, build_style, rgb  # noqa: E402
import style_tools  # noqa: E402

NAME = "Ocean"
HERE = os.path.dirname(os.path.abspath(__file__))

# Menu palette entries. White (1) is the sea foam (and the battle text); 2-4
# (vanilla's gold) hold the rope, light to dark. The two darkest light panel
# shades hold the wave band's aqua (8) and deep water (9); light panels use
# the three above them.
ROPE_LIGHT, ROPE, ROPE_DARK = 2, 3, D
FOAM = W
AQUA, DEEP = 8, 9
PANEL = 10

TRIM = ["FFFFFF", "E8C888", "B08040", "4A2E14"]     # 1 foam, 2-4 rope
SAND = ["FCF8F0", "F0EADC", "E2D8C8"]               # 5-7
WAVE = ["58C8D8", "1E6E9C"]                         # 8 aqua, 9 deep water

WATERS = [
    # Teal: window colour 0, player faction.
    ["38A8A8", "309C9C", "289090", "208080", "187070", "106060"],
    # Coral: window colour 1, enemy faction.
    ["E87870", "DC6C64", "D06058", "BC5048", "A84038", "943028"],
    # Kelp green: window colour 2, allied (green) faction.
    ["58A048", "4C9440", "448838", "3C7830", "346828", "2C5820"],
    # Pearl: window colour 3, other (purple) faction.
    ["A8A0C0", "9C94B4", "9088A8", "847C9C", "787090", "6C6484"],
]

# Prep funds/items OBJ palette (vanilla gUnknown_08A1B638): 2-9 dark -> light ramp.
PREP_ITEMS = ["7080A0", "082830", "0E4048", "145058", "1C6468", "247878",
              "308C8C", "44A4A4", "68BCBC", "98D4D4", "061C20", "FFFFFF",
              "F0EADC", "E8C888", "B08040", "000000"]

BUBBLE_RAMPS = {"ui": [[5, 6, 7], list(range(10, 16))],
                "prep": [list(range(9, 1, -1))]}

# Per 8x8 tile: one bubble's ring and a small bubble beside it. Bubble pixels
# keep the panel's lightest shade; the water around them takes the next one.
BUBBLES = {(2, 1), (3, 1), (1, 2), (4, 2), (1, 3), (4, 3), (2, 4), (3, 4), (6, 6)}


def ui_row(water, dim=1.0):
    row = ([rgb("7080A0")] + [rgb(c) for c in TRIM] + [rgb(c) for c in SAND]
           + [rgb(c) for c in WAVE] + [rgb(c) for c in water])
    if dim != 1.0:
        row = [row[0]] + [tuple(int(v * dim) for v in c) for c in row[1:]]
    return row


def menu_palette():
    rows = [ui_row(w) for w in WATERS]
    rows += [ui_row(w, 0.78) for w in WATERS]
    return rows


def texture(image, kind, skip=frozenset()):
    """Soft bubbles: the water one shade darker everywhere except the
    bubbles, which keep the lighter shade."""
    px = image.load()
    lookup = {}
    for ramp in BUBBLE_RAMPS[kind]:
        for i, index in enumerate(ramp):
            lookup[index] = (ramp, i)

    for y in range(image.height):
        for x in range(image.width):
            index = px[x, y]
            if index not in lookup or (x % 8, y % 8) in BUBBLES or (x, y) in skip:
                continue
            ramp, i = lookup[index]
            if i + 1 < len(ramp):
                px[x, y] = ramp[i + 1]


# --- Frame ------------------------------------------------------------------------
# A 5-pixel border on every side, from the outside in: two rows of twisted
# rope, then three rows of waves. Top bars are only 4 pixels tall, so their
# outer rope row sits just above vanilla's window outline; side edges push
# the border outward (left edges start at x 2, right edges end at x 6), so
# all four sides match. The rope and waves are patterned along each side and
# repeat every 8 pixels, so repeated tiles join up.
OUTLINE = None       # this style lays out its own edges
H_LAYER = 100        # + layer (0-4, outside -> in): placeholder on a top/bottom bar
V_LAYER = 110        # the same on a left/right edge
SIDE = 120           # a 1-pixel vanilla line on a side edge
THICKNESS = 5

# Rope: a light strand twisting across a darker one.
ROPE_ROWS = [
    [ROPE_LIGHT, ROPE_LIGHT, ROPE, ROPE_DARK, ROPE_LIGHT, ROPE_LIGHT, ROPE, ROPE_DARK],
    [ROPE, ROPE_DARK, ROPE_LIGHT, ROPE_LIGHT, ROPE, ROPE_DARK, ROPE_LIGHT, ROPE_LIGHT],
]
# Waves: rows of overlapping arcs (seigaiha), foam crests over aqua with deep
# water between them.
WAVE_ROWS = [
    [DEEP, DEEP, FOAM, FOAM, FOAM, FOAM, DEEP, DEEP],
    [DEEP, FOAM, AQUA, AQUA, AQUA, AQUA, FOAM, DEEP],
    [FOAM, AQUA, AQUA, AQUA, AQUA, AQUA, AQUA, FOAM],
]
LAYERS = ROPE_ROWS + WAVE_ROWS      # outside -> inside


def layer_colour(layer, along):
    return LAYERS[layer][along % 8]


def beam(n, i, outer):
    """A bar n pixels thick. At a window edge: the rope and waves against the
    panel, nothing outside them. Between two panels: a rope. A 1-pixel line
    is a side edge, laid out later."""
    if n == 1:
        return SIDE
    if outer == 0:
        return H_LAYER + (i % 2)
    q = n - 1 - i if outer < 0 else i      # distance from the panel side
    return H_LAYER + (THICKNESS - 1 - q) if q < THICKNESS else 0


# Big solid trim areas (patterned plates, the battle item boxes) become panel.
SOLID = {W: 11, L: 12, M: 15, D: 14}

STARFISH = [
    "...D...",
    "..DLD..",
    "DDDLDDD",
    "DLLWLLD",
    ".DLLLD.",
    "DLDDDLD",
    "DD...DD",
]
STAR_INDEX = {"D": ROPE_DARK, "L": ROPE_LIGHT, "W": FOAM}


def corner_tile(outer_x, outer_y, inner_x, inner_y, star_x, star_y):
    """The rope and waves mitred round the corner, with a starfish on top.
    outer_* is the outline's corner, inner_* the direction to the panel."""
    tile = [[0] * 8 for _ in range(8)]
    for y in range(8):
        for x in range(8):
            dx = (x - outer_x) * inner_x
            dy = (y - outer_y) * inner_y
            if dx < 0 or dy < 0:
                continue
            d = min(dx, dy)
            tile[y][x] = layer_colour(d, x + y) if d < THICKNESS else PANEL
    for sy, row in enumerate(STARFISH):
        for sx, ch in enumerate(row):
            x, y = star_x - 3 + sx, star_y - 3 + sy
            if ch != "." and 0 <= x < 8 and 0 <= y < 8:
                tile[y][x] = STAR_INDEX[ch]
    return tile


ART_NAMES = {0: ".", ROPE_LIGHT: "L", ROPE: "R", ROPE_DARK: "D", FOAM: "W",
             AQUA: "a", DEEP: "d", PANEL: "P"}
ART_INDEX = {v: k for k, v in ART_NAMES.items()}
CORNER_ART = {
    name: ["".join(ART_NAMES[v] for v in row) for row in corner_tile(*args)]
    for name, args in {
        # outline corner, panel direction, starfish centre
        "TL": (2, 3, 1, 1, 3, 3),
        "TR": (6, 3, -1, 1, 4, 3),
        "BL": (2, 4, 1, -1, 3, 4),
        "BR": (6, 4, -1, -1, 4, 4),
    }.items()
}

# The "Sort" box and similar windows start with this top-left tile.
EXTRA_CORNERS = {0x61: "TL"}

# Window style 1 fills its whole tiles; its edge and corner tiles get the
# border along their outer sides.
FULL_TILE_EDGES = {0x73: "T", 0x75: "L", 0x76: "R", 0x78: "B"}


def full_frame(tile, sides, drawn):
    for y in range(8):
        for x in range(8):
            dist = {"T": y, "B": 7 - y, "L": x, "R": 7 - x}
            d, side = min((dist[s], s) for s in sides)
            if d < THICKNESS:
                tile[y][x] = layer_colour(d, x if side in "TB" else y)
                drawn.add((x, y))
            else:
                tile[y][x] = PANEL


def full_corner(tile, corner, drawn):
    full_frame(tile, corner, drawn)


def is_panel(v):
    return v == SIDE or v == W or 5 <= v <= 15


def lay_out_sides(tile, drawn):
    """Each panel pixel on a left or right edge becomes the inner edge of the
    full border, which reaches outward from it."""
    edges = [(x, y, dx) for y in range(8) for x in range(8) for dx in (1, -1)
             if is_panel(tile[y][x]) and 0 <= x - dx < 8 and tile[y][x - dx] == 0]
    for x, y, dx in edges:
        for q in range(THICKNESS):              # panel side -> outside
            px_ = x - dx * q
            if 0 <= px_ < 8:
                tile[y][px_] = V_LAYER + (THICKNESS - 1 - q)
                drawn.add((px_, y))
        inner = x + dx
        if 0 <= inner < 8 and tile[y][inner] == 9:          # vanilla's edge light
            tile[y][inner] = PANEL


def add_outer_rope(tile, drawn):
    """Top bars are a row short: their outer rope goes just outside them."""
    for y in range(8):
        for x in range(8):
            if tile[y][x] != H_LAYER + 1:
                continue
            for dy in (-1, 1):
                ny = y + dy
                if 0 <= ny < 8 and tile[ny][x] == 0 and 0 <= y - dy < 8 \
                        and tile[y - dy][x] == H_LAYER + 2:
                    tile[ny][x] = H_LAYER
                    drawn.add((x, ny))


def edge_details(tile, t, drawn):
    """Lay out the side borders, finish the top bars, pattern the rope and
    waves along each side, and fold the panels' two darkest light shades into
    sand (their entries hold the wave colours) and their white too."""
    if t in FULL_TILE_EDGES:
        full_frame(tile, FULL_TILE_EDGES[t], drawn)
        return
    lay_out_sides(tile, drawn)
    add_outer_rope(tile, drawn)
    for y in range(8):
        for x in range(8):
            v = tile[y][x]
            if H_LAYER <= v < H_LAYER + THICKNESS:
                tile[y][x] = layer_colour(v - H_LAYER, x)
            elif V_LAYER <= v < V_LAYER + THICKNESS:
                tile[y][x] = layer_colour(v - V_LAYER, y)
            elif v == SIDE:
                tile[y][x] = ROPE
            elif (x, y) not in drawn and v in (AQUA, DEEP):
                tile[y][x] = 7
            elif (x, y) not in drawn and v == W:
                tile[y][x] = 5


# --- Chapter status banners -------------------------------------------------------
# The banners use the player palette's teal, rope and foam (the rope's dark
# brown is the outline at 1, as the shared code draws letter outlines with
# 1). They stay off the entries that hold the icons' original colours
# (style_tools.VANILLA_ICON_SLOTS): the waves' aqua reuses the icons' blue
# gem, and their deep water the darker teal.
SIGN_PALETTE = ["7080A0", "4A2E14", "E8C888", "B08040", "FFFFFF",
                None, None, None, None, None, "38A8A8", "309C9C",
                None, None, None, None]
for slot, colour in style_tools.VANILLA_ICON_SLOTS.items():
    SIGN_PALETTE[slot] = colour
MENU_TO_SIGN = {0: 0, ROPE_DARK: 1, ROPE_LIGHT: 2, ROPE: 3, FOAM: 4,
                5: 5, 6: 5, 7: 5, AQUA: 15, DEEP: 11,
                10: 10, 11: 11, 12: 11, 13: 11, 14: 11, 15: 11}
# Vanilla PLAY TIME colours (OBJ palette 9) -> SIGN_PALETTE: white face, pale
# blue shading, rope drop shadow.
LETTERS = {11: 4, 12: 4, 13: 6, 14: 1}
# The rest of the sheet (a spare banner with dragons): teal water, foam
# dragons, rope trim.
REST_OF_SHEET = {
    0: 0, 1: 2, 2: 11, 3: 11, 4: 11, 5: 11, 6: 10, 7: 10, 8: 10, 9: 10,
    10: 4, 11: 6, 12: 6, 13: 15, 14: 1, 15: 6,
}
# The two item icons beside the banner keep vanilla's own colours.
ICON_COLOURS = style_tools.VANILLA_ICON_COLOURS


def label_tab(px, left, top, right, bottom):
    """A teal plate edged in rope."""
    for y in range(top, bottom + 1):
        for x in range(left, right + 1):
            d = min(x - left, right - x, y - top, bottom - y)
            px[x, y] = 1 if d == 0 else 2 if d == 1 else 11
    for cx, cy in ((left, top), (right, top), (left, bottom), (right, bottom)):
        px[cx, cy] = 0


if __name__ == "__main__":
    build_style(sys.modules[__name__])
