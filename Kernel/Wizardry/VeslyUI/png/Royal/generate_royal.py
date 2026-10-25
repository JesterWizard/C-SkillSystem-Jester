#!/usr/bin/env python3
"""
Build the Royal VeslyUI style from the vanilla UI graphics in ../Vanilla.

The shared redraw lives in ../style_tools.py; this script sets the look:
  * the palettes: royal blue / crimson / emerald / purple velvet panels and
    ivory light panels, instead of vanilla's blue / red / green / sepia;
  * the frame: a gilded moulding 4 pixels thick, the same on all four sides,
    from a dark outline through bright and mid gold to a shadowed inner edge;
  * a jewel set in a gold diamond on each window corner;
  * a woven damask pattern on the panels: a small diamond in every tile.

Outputs (next to this script):
  Royal_*.png             indexed images, read by Png2Dmp from the installer
  Royal_MenuPal.pal       8 menu palettes (4 window colours + 4 guide rows)
  Royal_BattlePal.pal     4 battle UI palettes (one per faction)
  Royal_PrepItemsPal.pal  prep-screen funds/item OBJ palette
  Royal_PlayStatusPal.pal chapter status banner and playtime box OBJ palettes

Rerun this script after changing a colour, then rebuild.
"""
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), ".."))
from style_tools import D, L, M, W, build_style, rgb  # noqa: E402
import style_tools  # noqa: E402

NAME = "Royal"
HERE = os.path.dirname(os.path.abspath(__file__))

# Menu palette entries. White (1) stays white: the battle screen draws its
# name and weapon text with it. The two darkest light panel shades hold the
# shadowed gold (8) and the jewel (9); light panels use the three above them.
GOLD_LIGHT, GOLD, GOLD_DARK = 2, 3, 8
JEWEL = 9
IVORY = 5
PANEL = 10

TRIM = ["FFFCF0", "F8DC78", "DCA838", "3C1E08"]   # 1 white, 2 light gold, 3 gold, 4 outline
IVORY_RAMP = ["FFF8E8", "F8ECD0", "F0DEB4"]           # 5-7
DARK_GOLD = "9C6A1C"                                  # 8

COLOURS = [
    # Royal blue velvet: window colour 0, player faction. Ruby jewel.
    ("E02840", ["2C44A8", "263C98", "203488", "1A2C78", "162468", "121C58"]),
    # Crimson velvet: window colour 1, enemy faction. Sapphire jewel.
    ("3878F0", ["A82838", "982030", "881C2A", "781824", "68141E", "581018"]),
    # Emerald velvet: window colour 2, allied (green) faction. Ruby jewel.
    ("E02840", ["1E7848", "1A6A40", "165C38", "124E30", "0E4028", "0A3220"]),
    # Purple velvet: window colour 3, other (purple) faction. Ruby jewel.
    ("E02840", ["6A2C98", "5E2688", "522078", "461A68", "3A1458", "2E0E48"]),
]

# Prep funds/items OBJ palette (vanilla gUnknown_08A1B638): 2-9 dark -> light ramp.
PREP_ITEMS = ["7080A0", "100C30", "141C5A", "1C2A78", "283C98", "3850B0",
              "5068C4", "7084D4", "90A4E0", "B4C4EC", "0C0820", "FFF8E8",
              "F0DEB4", "DCA838", "9C6A1C", "000000"]

DAMASK_RAMPS = {"ui": [[5, 6, 7], list(range(10, 16))],
                "prep": [list(range(9, 1, -1))]}

# One small diamond with a centre dot per 8x8 tile, plus a dot on the tile
# corner, so repeated tiles weave into a lattice.
DAMASK = {(3, 1), (2, 2), (4, 2), (1, 3), (3, 3), (5, 3), (2, 4), (4, 4), (3, 5), (7, 7)}


def ui_row(jewel, velvet, dim=1.0):
    row = ([rgb("7080A0")] + [rgb(c) for c in TRIM] + [rgb(c) for c in IVORY_RAMP]
           + [rgb(DARK_GOLD), rgb(jewel)] + [rgb(c) for c in velvet])
    if dim != 1.0:
        row = [row[0]] + [tuple(int(v * dim) for v in c) for c in row[1:]]
    return row


def menu_palette():
    rows = [ui_row(j, v) for j, v in COLOURS]
    rows += [ui_row(j, v, 0.78) for j, v in COLOURS]
    return rows


def texture(image, kind, skip=frozenset()):
    """Damask: the motif pixels one shade darker."""
    px = image.load()
    lookup = {}
    for ramp in DAMASK_RAMPS[kind]:
        for i, index in enumerate(ramp):
            lookup[index] = (ramp, i)

    for y in range(image.height):
        for x in range(image.width):
            index = px[x, y]
            if index not in lookup or (x % 8, y % 8) not in DAMASK or (x, y) in skip:
                continue
            ramp, i = lookup[index]
            px[x, y] = ramp[i + 1] if i + 1 < len(ramp) else ramp[i - 1]


# --- Frame ------------------------------------------------------------------------
# A 4-pixel gilded moulding on every side: outline, light gold, gold, dark
# gold from the outside in. Top and bottom bars hold it inside vanilla's
# window outline; side edges push it outward (left edges start at x 3, right
# edges end at x 5), so all four sides match.
MOULDING = [D, GOLD_LIGHT, GOLD, GOLD_DARK]     # outside -> inside


def beam(n, i, outer):
    """A bar n pixels thick. At a window edge: the moulding against the
    panel, nothing outside it. Between two panels: a gold rail."""
    if outer == 0:
        if n == 1:
            return GOLD
        return GOLD_LIGHT if i == n // 2 else (GOLD_DARK if i in (0, n - 1) else GOLD)
    q = n - 1 - i if outer < 0 else i      # distance from the panel side
    return MOULDING[3 - q] if q < 4 else 0


# Big solid trim areas (patterned plates, the battle item boxes) become panel.
SOLID = {W: 11, L: 12, M: 15, D: 14}


def corner_tile(outer_x, outer_y, inner_x, inner_y, jewel_x, jewel_y):
    """A mitred moulding corner with a jewel in a gold diamond on top.
    outer_* is the outline's corner, inner_* the direction to the panel."""
    tile = [[0] * 8 for _ in range(8)]
    for y in range(8):
        for x in range(8):
            dx = (x - outer_x) * inner_x
            dy = (y - outer_y) * inner_y
            if dx < 0 or dy < 0:
                continue
            d = min(dx, dy)
            tile[y][x] = MOULDING[d] if d < 4 else PANEL
    for y in range(8):
        for x in range(8):
            r = abs(x - jewel_x) + abs(y - jewel_y)
            if r == 3:
                tile[y][x] = D
            elif r == 2:
                s = (x - jewel_x) + (y - jewel_y)
                tile[y][x] = GOLD_LIGHT if s < 0 else GOLD_DARK if s > 0 else GOLD
            elif r <= 1:
                tile[y][x] = JEWEL
    tile[jewel_y - 1][jewel_x] = IVORY           # glint
    return tile


def corner_art():
    """CORNER_ART strings for the four corners, from corner_tile()."""
    names = {0: ".", D: "D", GOLD_LIGHT: "L", GOLD: "G", GOLD_DARK: "d",
             JEWEL: "j", IVORY: "i", PANEL: "P"}
    tiles = {
        # outline corner, panel direction, jewel centre
        "TL": corner_tile(3, 4, 1, 1, 3, 4),
        "TR": corner_tile(5, 4, -1, 1, 4, 4),
        "BL": corner_tile(3, 3, 1, -1, 3, 3),
        "BR": corner_tile(5, 3, -1, -1, 4, 3),
    }
    return {k: ["".join(names[v] for v in row) for row in t] for k, t in tiles.items()}


CORNER_ART = corner_art()
ART_INDEX = {".": 0, "D": D, "L": GOLD_LIGHT, "G": GOLD, "d": GOLD_DARK,
             "j": JEWEL, "i": IVORY, "P": PANEL}

# The "Sort" box and similar windows start with this top-left tile.
EXTRA_CORNERS = {0x61: "TL"}

# Window style 1 fills its whole tiles; its edge and corner tiles get the
# moulding along their outer sides.
FULL_TILE_EDGES = {0x73: "T", 0x75: "L", 0x76: "R", 0x78: "B"}


def full_frame(tile, sides, drawn):
    for y in range(8):
        for x in range(8):
            ds = [{"T": y, "B": 7 - y, "L": x, "R": 7 - x}[s] for s in sides]
            d = min(ds)
            tile[y][x] = MOULDING[d] if d < 4 else PANEL
            if d < 4:
                drawn.add((x, y))


def full_corner(tile, corner, drawn):
    full_frame(tile, corner, drawn)


def push_sides(tile, drawn):
    """Turn each left and right outline pixel into the full moulding, pushed
    outward so it is 4 pixels thick like the top and bottom bars."""
    lines = [(x, y, dx) for y in range(8) for x in range(8) for dx in (1, -1)
             if tile[y][x] == D and 0 <= x - dx < 8 and tile[y][x - dx] == 0]
    for x, y, dx in lines:
        for k, colour in enumerate(reversed(MOULDING)):    # inside -> outside
            px_ = x - dx * k
            if 0 <= px_ < 8:
                tile[y][px_] = colour
                drawn.add((px_, y))
        inner = x + dx
        if 0 <= inner < 8 and tile[y][inner] == 9:          # vanilla's edge light
            tile[y][inner] = PANEL


def edge_details(tile, t, drawn):
    """Plain, even mouldings, and the panels' two darkest light shades folded
    into ivory (their palette entries hold the dark gold and the jewel)."""
    if t in FULL_TILE_EDGES:
        full_frame(tile, FULL_TILE_EDGES[t], drawn)
        return
    push_sides(tile, drawn)
    for y in range(8):
        for x in range(8):
            if (x, y) in drawn:
                continue
            if tile[y][x] in (GOLD_DARK, JEWEL):
                tile[y][x] = 7


# --- Chapter status banners -------------------------------------------------------
# The banners use velvet, gold, ivory and the outline (at 1, as the shared code
# draws letter outlines with 1). They stay off the entries that hold the
# icons' original colours (style_tools.VANILLA_ICON_SLOTS), so the banner
# jewels use the icons' blue gem and the ivory their cream.
SIGN_PALETTE = ["7080A0", "3C1E08", "F8DC78", "DCA838", "9C6A1C",
                None, None, None, None, None, "2C44A8", "263C98",
                None, None, None, None]
for slot, colour in style_tools.VANILLA_ICON_SLOTS.items():
    SIGN_PALETTE[slot] = colour
MENU_TO_SIGN = {0: 0, D: 1, W: 5, GOLD_LIGHT: 2, GOLD: 3, GOLD_DARK: 4,
                5: 5, 6: 5, 7: 5, JEWEL: 15,
                10: 10, 11: 11, 12: 11, 13: 11, 14: 10, 15: 10}
# Vanilla PLAY TIME colours (OBJ palette 9) -> SIGN_PALETTE: ivory face,
# light gold shading, dark gold drop shadow.
LETTERS = {11: 5, 12: 5, 13: 2, 14: 4}
# The rest of the sheet (a spare banner with dragons): velvet ground, gold
# dragons and trim.
REST_OF_SHEET = {
    0: 0, 1: 2, 2: 11, 3: 11, 4: 11, 5: 11, 6: 10, 7: 10, 8: 10, 9: 10,
    10: 3, 11: 5, 12: 5, 13: 15, 14: 1, 15: 5,
}
# The two item icons beside the banner keep vanilla's own colours.
ICON_COLOURS = style_tools.VANILLA_ICON_COLOURS


def label_tab(px, left, top, right, bottom):
    """A velvet plate in a thin gilded frame."""
    for y in range(top, bottom + 1):
        for x in range(left, right + 1):
            d = min(x - left, right - x, y - top, bottom - y)
            px[x, y] = 1 if d == 0 else 2 if d == 1 else 10
    for cx, cy in ((left, top), (right, top), (left, bottom), (right, bottom)):
        px[cx, cy] = 0


if __name__ == "__main__":
    build_style(sys.modules[__name__])
