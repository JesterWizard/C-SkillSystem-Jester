#!/usr/bin/env python3
"""
Build the Forest / Leaf VeslyUI style from the vanilla UI graphics in
../Vanilla. It pairs well with the Wood style.

The shared redraw lives in ../style_tools.py; this script sets the look:
  * the palettes: pine / maple / leaf green / autumn amber panels and pale
    parchment light panels, instead of vanilla's blue / red / green / sepia;
  * the frame: two vines woven over and under each other, 4 pixels thick and
    the same on all four sides;
  * a small leaf on each window corner;
  * a leaf-vein texture on the panels: a midrib and side veins in every tile.

Outputs (next to this script):
  Forest_*.png             indexed images, read by Png2Dmp from the installer
  Forest_MenuPal.pal       8 menu palettes (4 window colours + 4 guide rows)
  Forest_BattlePal.pal     4 battle UI palettes (one per faction)
  Forest_PrepItemsPal.pal  prep-screen funds/item OBJ palette
  Forest_PlayStatusPal.pal chapter status banner and playtime box OBJ palettes

Rerun this script after changing a colour, then rebuild.
"""
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), ".."))
from style_tools import D, L, M, W, build_style, rgb  # noqa: E402
import style_tools  # noqa: E402

NAME = "Forest"
HERE = os.path.dirname(os.path.abspath(__file__))

# Menu palette entries. White (1) stays white for text. 2-4 (vanilla's gold)
# hold the vines, light to dark (the dark one also fills the gaps between
# them). The two darkest light panel shades hold the corner leaves' bright
# green (8) and the bark brown of their stems (9); light panels use 5-7.
VINE_LIGHT, VINE, VINE_DARK = 2, 3, D
LEAF, BARK = 8, 9
PANEL = 10

TRIM = ["FFFFFF", "8CC448", "4E8A2C", "1E3414"]     # 1 white, 2-4 vines
PARCHMENT = ["F4F8E4", "E4ECCC", "D4E0B4"]          # 5-7
CORNER = ["B8E060", "6A4428"]                       # 8 leaf, 9 bark

LEAVES = [
    # Pine: window colour 0, player faction.
    ["2E7A5C", "287050", "226646", "1C5C3C", "165034", "104428"],
    # Maple red: window colour 1, enemy faction.
    ["B84838", "AC4030", "A0382A", "903024", "802820", "70201A"],
    # Leaf green: window colour 2, allied (green) faction.
    ["5CA034", "549430", "4C882C", "447C26", "3C7020", "34641C"],
    # Autumn amber: window colour 3, other (purple) faction.
    ["B88830", "AC7E2C", "A07428", "906824", "805C1E", "705018"],
]

# Prep funds/items OBJ palette (vanilla gUnknown_08A1B638): 2-9 dark -> light ramp.
PREP_ITEMS = ["7080A0", "142408", "104428", "165034", "1C5C3C", "287050",
              "3C8A60", "5CA478", "88C098", "B4DCB8", "0C1808", "FFFFFF",
              "E4ECCC", "8CC448", "4E8A2C", "000000"]

VEIN_RAMPS = {"ui": [[5, 6, 7], list(range(10, 16))],
              "prep": [list(range(9, 1, -1))]}

# Leaf veins per 8x8 tile: a midrib on the diagonal and four side veins.
VEINS = ({(x, 7 - x) for x in range(8)}
         | {(5, 4), (6, 5), (3, 6), (4, 7), (2, 3), (1, 2), (4, 1), (3, 0)})


def ui_row(leaf, dim=1.0):
    row = ([rgb("7080A0")] + [rgb(c) for c in TRIM] + [rgb(c) for c in PARCHMENT]
           + [rgb(c) for c in CORNER] + [rgb(c) for c in leaf])
    if dim != 1.0:
        row = [row[0]] + [tuple(int(v * dim) for v in c) for c in row[1:]]
    return row


def menu_palette():
    rows = [ui_row(l) for l in LEAVES]
    rows += [ui_row(l, 0.78) for l in LEAVES]
    return rows


def texture(image, kind, skip=frozenset()):
    """Leaf veins: the vein pixels one shade darker."""
    px = image.load()
    lookup = {}
    for ramp in VEIN_RAMPS[kind]:
        for i, index in enumerate(ramp):
            lookup[index] = (ramp, i)

    for y in range(image.height):
        for x in range(image.width):
            index = px[x, y]
            if index not in lookup or (x % 8, y % 8) not in VEINS or (x, y) in skip:
                continue
            ramp, i = lookup[index]
            px[x, y] = ramp[i + 1] if i + 1 < len(ramp) else ramp[i - 1]


# --- Frame ------------------------------------------------------------------------
# A 4-pixel band of two vines woven over and under each other, the same on
# all four sides. Top and bottom bars hold it inside vanilla's window
# outline; side edges grow it outward from the panel's edge pixel (left
# edges start at x 3, right edges end at x 5). The weave repeats every 8
# pixels along a side, so repeated tiles join up.
OUTLINE = None       # this style lays out its own edges
H_LAYER = 100        # + layer (0-3, outside -> in): placeholder on a top/bottom bar
V_LAYER = 110        # the same on a left/right edge
SIDE = 120           # a 1-pixel vanilla line on a side edge
THICKNESS = 4


def weave():
    """4 rows x 8 columns of the woven band, outside row first: two vines,
    one light and one darker green, zig-zagging across the band in opposite
    directions so they cross twice per repeat, over a dark gap."""
    a_rows = [0, 0, 1, 2, 3, 3, 2, 1]           # row of the light vine at each x
    rows = [[VINE_DARK] * 8 for _ in range(THICKNESS)]
    for x in range(8):
        rows[a_rows[x]][x] = VINE_LIGHT
        rows[THICKNESS - 1 - a_rows[x]][x] = VINE
    return rows


LAYERS = weave()


def layer_colour(layer, along):
    return LAYERS[layer][along % 8]


def beam(n, i, outer):
    """A bar n pixels thick. At a window edge: the woven band against the
    panel, nothing outside it. Between two panels: a single vine. A 1-pixel
    line is a side edge, laid out later."""
    if n == 1:
        return SIDE
    if outer == 0:
        return H_LAYER + 1 + (i % 2)
    q = n - 1 - i if outer < 0 else i      # distance from the panel side
    return H_LAYER + (THICKNESS - 1 - q) if q < THICKNESS else 0


# Big solid trim areas (patterned plates, the battle item boxes) become panel.
SOLID = {W: 11, L: 12, M: 15, D: 14}

# A leaf pointing out of the top-left corner, stem at the frame corner. Other
# corners mirror it.
LEAF_ART = [
    "lll.....",
    "lLLl....",
    "lLvLl...",
    ".lLvLl..",
    "..lLvb..",
    "...lbb..",
    "........",
    "........",
]
LEAF_INDEX = {"l": VINE_DARK, "L": LEAF, "v": VINE, "b": BARK}


def corner_tile(outer_x, outer_y, inner_x, inner_y, flip_x, flip_y, shift_x, shift_y):
    """The weave mitred round the corner, with a leaf on top. outer_* is the
    outline's corner, inner_* the direction to the panel; the leaf is flipped
    and shifted so it points out of the corner with its stem on the frame."""
    tile = [[0] * 8 for _ in range(8)]
    for y in range(8):
        for x in range(8):
            dx = (x - outer_x) * inner_x
            dy = (y - outer_y) * inner_y
            if dx < 0 or dy < 0:
                continue
            d = min(dx, dy)
            tile[y][x] = layer_colour(d, x + y) if d < THICKNESS else PANEL
    for ly, row in enumerate(LEAF_ART):
        for lx, ch in enumerate(row):
            if ch == ".":
                continue
            x = (7 - lx if flip_x else lx) + shift_x
            y = (7 - ly if flip_y else ly) + shift_y
            if 0 <= x < 8 and 0 <= y < 8:
                tile[y][x] = LEAF_INDEX[ch]
    return tile


ART_NAMES = {0: ".", VINE_LIGHT: "L", VINE: "V", VINE_DARK: "D", LEAF: "f",
             BARK: "b", PANEL: "P"}
ART_INDEX = {v: k for k, v in ART_NAMES.items()}
CORNER_ART = {
    name: ["".join(ART_NAMES[v] for v in row) for row in corner_tile(*args)]
    for name, args in {
        # outline corner, panel direction, leaf flips, leaf shift
        "TL": (3, 4, 1, 1, False, False, 0, 0),
        "TR": (5, 4, -1, 1, True, False, -1, 0),
        "BL": (3, 3, 1, -1, False, True, 0, -1),
        "BR": (5, 3, -1, -1, True, True, -1, -1),
    }.items()
}

# The "Sort" box and similar windows start with this top-left tile.
EXTRA_CORNERS = {0x61: "TL"}

# Window style 1 fills its whole tiles; its edge and corner tiles get the
# band along their outer sides.
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
    full band, which reaches outward from it."""
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


def edge_details(tile, t, drawn):
    """Lay out the side bands, weave the vines along each side, and fold the
    panels' two darkest light shades (their entries hold the leaf and bark)
    and their white into parchment."""
    if t in FULL_TILE_EDGES:
        full_frame(tile, FULL_TILE_EDGES[t], drawn)
        return
    lay_out_sides(tile, drawn)
    for y in range(8):
        for x in range(8):
            v = tile[y][x]
            if H_LAYER <= v < H_LAYER + THICKNESS:
                tile[y][x] = layer_colour(v - H_LAYER, x)
            elif V_LAYER <= v < V_LAYER + THICKNESS:
                tile[y][x] = layer_colour(v - V_LAYER, y)
            elif v == SIDE:
                tile[y][x] = VINE
            elif (x, y) not in drawn and v in (LEAF, BARK):
                tile[y][x] = 7
            elif (x, y) not in drawn and v == W:
                tile[y][x] = 5


# --- Chapter status banners -------------------------------------------------------
# The banners use the player palette's pine, vines and white (the darkest vine
# green is the outline at 1, as the shared code draws letter outlines with 1).
# They stay off the entries that hold the icons' original colours
# (style_tools.VANILLA_ICON_SLOTS): the leaves reuse the icons' palest green,
# their stems the outline.
SIGN_PALETTE = ["7080A0", "1E3414", "8CC448", "4E8A2C", "FFFFFF",
                None, None, None, None, None, "2E7A5C", "287050",
                None, None, None, None]
for slot, colour in style_tools.VANILLA_ICON_SLOTS.items():
    SIGN_PALETTE[slot] = colour
MENU_TO_SIGN = {0: 0, VINE_DARK: 1, VINE_LIGHT: 2, VINE: 3, W: 4,
                5: 5, 6: 5, 7: 5, LEAF: 14, BARK: 1,
                10: 10, 11: 11, 12: 11, 13: 11, 14: 11, 15: 11}
# Vanilla PLAY TIME colours (OBJ palette 9) -> SIGN_PALETTE: white face, cream
# shading, dark green drop shadow.
LETTERS = {11: 4, 12: 4, 13: 5, 14: 1}
# The rest of the sheet (a spare banner with dragons): pine ground, light
# green dragons, vine trim.
REST_OF_SHEET = {
    0: 0, 1: 2, 2: 11, 3: 11, 4: 11, 5: 11, 6: 10, 7: 10, 8: 10, 9: 10,
    10: 2, 11: 5, 12: 5, 13: 14, 14: 1, 15: 5,
}
# The two item icons beside the banner keep vanilla's own colours.
ICON_COLOURS = style_tools.VANILLA_ICON_COLOURS


def label_tab(px, left, top, right, bottom):
    """A pine plate edged in vine."""
    for y in range(top, bottom + 1):
        for x in range(left, right + 1):
            d = min(x - left, right - x, y - top, bottom - y)
            px[x, y] = 1 if d == 0 else 2 if d == 1 else 11
    for cx, cy in ((left, top), (right, top), (left, bottom), (right, bottom)):
        px[cx, cy] = 0


if __name__ == "__main__":
    build_style(sys.modules[__name__])
