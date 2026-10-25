#!/usr/bin/env python3
"""
Build the Parchment & Ink VeslyUI style from the vanilla UI graphics in
../Vanilla.

The shared redraw lives in ../style_tools.py; this script sets the look:
  * the palettes: aged paper panels and cream light panels for every window
    colour; the factions are told apart by their ink (blue, red, green,
    sepia), which draws the borders and the seals;
  * the frame: a hand-drawn ink line whose outer edge wobbles, the stroke
    swelling and thinning like a pen, on all four sides;
  * a wax seal on each window corner, in the window's ink colour;
  * a faint paper-fibre speckle on the panels.

Outputs (next to this script):
  Parchment_*.png             indexed images, read by Png2Dmp from the installer
  Parchment_MenuPal.pal       8 menu palettes (4 window colours + 4 guide rows)
  Parchment_BattlePal.pal     4 battle UI palettes (one per faction)
  Parchment_PrepItemsPal.pal  prep-screen funds/item OBJ palette
  Parchment_PlayStatusPal.pal chapter status banner and playtime box OBJ palettes

Rerun this script after changing a colour, then rebuild.
"""
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), ".."))
from style_tools import D, L, M, W, build_style, noise, rgb  # noqa: E402
import style_tools  # noqa: E402

NAME = "Parchment"
HERE = os.path.dirname(os.path.abspath(__file__))

# Menu palette entries. White (1) stays white for text. 2-4 (vanilla's gold)
# hold the window colour's ink, light to dark: it draws the border and the
# wax seals. 5-9 are cream paper, 10-15 aged paper.
INK_LIGHT, INK, INK_DARK = 2, 3, D
PANEL = 10

INKS = [
    # Blue ink: window colour 0, player faction.
    ["6888D0", "2C4CA0", "142450"],
    # Red ink: window colour 1, enemy faction.
    ["D07060", "A02828", "501010"],
    # Green ink: window colour 2, allied (green) faction.
    ["70A868", "2C6C34", "123018"],
    # Sepia ink: window colour 3, other (purple) faction.
    ["B08860", "6A4424", "2E1C0E"],
]
CREAM = ["FFF8E4", "F6ECD2", "EEE0BE", "E4D2A8", "D8C498"]                # 5-9
AGED = ["D8C49C", "CEB890", "C4AC84", "B8A078", "AC946C", "A08860"]       # 10-15

# Prep funds/items OBJ palette (vanilla gUnknown_08A1B638): 2-9 dark -> light ramp.
PREP_ITEMS = ["7080A0", "2E1C0E", "4A3018", "6A4424", "8A6034", "A87E4C",
              "C49C68", "D8B888", "E8D0A8", "F4E6CC", "1E1208", "FFFFFF",
              "F6ECD2", "6888D0", "2C4CA0", "000000"]

FIBRE_RAMPS = {"ui": [list(range(5, 10)), list(range(10, 16))],
               "prep": [list(range(9, 1, -1))]}

# Paper fibres: a few scattered pixels per 8x8 tile, one shade darker.
FIBRES = {(1, 1), (6, 2), (3, 4), (0, 6), (5, 6), (7, 4), (2, 7)}


def ui_row(ink, dim=1.0):
    row = ([rgb("7080A0"), rgb("FFFFFF")] + [rgb(c) for c in ink]
           + [rgb(c) for c in CREAM] + [rgb(c) for c in AGED])
    if dim != 1.0:
        row = [row[0]] + [tuple(int(v * dim) for v in c) for c in row[1:]]
    return row


def menu_palette():
    rows = [ui_row(i) for i in INKS]
    rows += [ui_row(i, 0.78) for i in INKS]
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
# A hand-drawn ink line on every side. Its inner edge runs straight along the
# panel; its outer edge wobbles, the stroke 1 to 3 pixels thick (2 at each
# tile's ends, so tiles join up). Each tile wobbles in its own way, and the
# DrawUiFrame replacement in C_Code.c mixes the interchangeable beam and left
# edge tiles by position, so the wobble along a window is irregular.
# Top lines end on row 7, bottom lines start on row 0, left lines end on
# x 6 and right lines start on x 2.
OUTLINE = None       # this style lays out its own edges
H_STROKE = 100       # + distance from the panel (0-2): placeholder on a top/bottom bar
V_STROKE = 110       # the same on a left/right edge
SIDE = 120           # a 1-pixel vanilla line on a side edge
BAND = 3             # the most the stroke can swell to


def wobble(t):
    """Stroke thickness at each pixel along tile t: 2 with one swelling or
    thinning stretch, placed by noise()."""
    widths = [2] * 8
    roll = noise(t)
    if roll < 200:
        return widths
    start = 1 + roll % 4
    length = 2 + (roll // 4) % 2
    for i in range(start, min(7, start + length)):
        widths[i] = 3 if (roll // 8) % 2 else 1
    return widths


def stroke(q, width):
    """Ink q pixels from the panel on a stroke this wide: the outer pixel
    darkest, nothing beyond it."""
    if q >= width:
        return 0
    return INK_DARK if q == width - 1 else INK


def beam(n, i, outer):
    """A bar n pixels thick. At a window edge: the ink line against the
    panel. Between two panels: a single ink rule. A 1-pixel line is a side
    edge, laid out later."""
    if n == 1:
        return SIDE
    if outer == 0:
        return INK if i == n // 2 else PANEL
    q = n - 1 - i if outer < 0 else i      # distance from the panel side
    return H_STROKE + q if q < BAND else 0


# Big solid trim areas (patterned plates, the battle item boxes) become paper.
SOLID = {W: 11, L: 12, M: 13, D: 14}

# A wax seal: a blob of the window's ink, darker rim, a lit edge, a pressed
# ring and a drip.
SEAL = [
    "..DDD..",
    ".DLLID.",
    "DLIDDID",
    "DLDWIDD",
    "DIIDDID",
    ".DIIDD.",
    "..DD.D.",
]
SEAL_INDEX = {"D": INK_DARK, "I": INK, "L": INK_LIGHT, "W": W}


def corner_tile(outer_x, outer_y, inner_x, inner_y, seal_x, seal_y):
    """The ink line mitred round the corner, with a seal on top. outer_* is
    the corner of the stroke band's outer edge, inner_* the direction to the
    panel."""
    tile = [[0] * 8 for _ in range(8)]
    for y in range(8):
        for x in range(8):
            dx = (x - outer_x) * inner_x
            dy = (y - outer_y) * inner_y
            if dx < 0 or dy < 0:
                continue
            q = BAND - 1 - min(dx, dy)      # distance from the panel
            tile[y][x] = stroke(q, 2) if q >= 0 else PANEL
    for sy, row in enumerate(SEAL):
        for sx, ch in enumerate(row):
            x, y = seal_x - 3 + sx, seal_y - 3 + sy
            if ch != "." and 0 <= x < 8 and 0 <= y < 8:
                tile[y][x] = SEAL_INDEX[ch]
    return tile


ART_NAMES = {0: ".", INK_LIGHT: "L", INK: "I", INK_DARK: "D", W: "W", PANEL: "P"}
ART_INDEX = {v: k for k, v in ART_NAMES.items()}
CORNER_ART = {
    name: ["".join(ART_NAMES[v] for v in row) for row in corner_tile(*args)]
    for name, args in {
        # stroke band corner, panel direction, seal centre
        "TL": (4, 5, 1, 1, 3, 4),
        "TR": (4, 5, -1, 1, 4, 4),
        "BL": (4, 2, 1, -1, 3, 3),
        "BR": (4, 2, -1, -1, 4, 3),
    }.items()
}

# The "Sort" box and similar windows start with this top-left tile.
EXTRA_CORNERS = {0x61: "TL"}

# Window style 1 fills its whole tiles; its edge and corner tiles get the
# ink line along their outer sides.
FULL_TILE_EDGES = {0x73: "T", 0x75: "L", 0x76: "R", 0x78: "B"}


def full_frame(tile, sides, drawn):
    for y in range(8):
        for x in range(8):
            d = min({"T": y, "B": 7 - y, "L": x, "R": 7 - x}[s] for s in sides)
            if d < 2:
                tile[y][x] = INK_DARK if d == 0 else INK
                drawn.add((x, y))
            else:
                tile[y][x] = PANEL


def full_corner(tile, corner, drawn):
    full_frame(tile, corner, drawn)


def is_panel(v):
    return v == SIDE or v == W or 5 <= v <= 15


def lay_out_sides(tile, drawn):
    """Each panel pixel on a left or right edge becomes the inner edge of the
    stroke band, which reaches outward from it."""
    edges = [(x, y, dx) for y in range(8) for x in range(8) for dx in (1, -1)
             if is_panel(tile[y][x]) and 0 <= x - dx < 8 and tile[y][x - dx] == 0]
    for x, y, dx in edges:
        for q in range(BAND):                   # panel side -> outside
            px_ = x - dx * q
            if 0 <= px_ < 8:
                tile[y][px_] = V_STROKE + q
                drawn.add((px_, y))
        inner = x + dx
        if 0 <= inner < 8 and tile[y][inner] == 9:          # vanilla's edge light
            tile[y][inner] = PANEL


def edge_details(tile, t, drawn):
    """Lay out the side bands, ink every band with this tile's wobble, and
    turn the white panels' white into fresh cream paper."""
    if t in FULL_TILE_EDGES:
        full_frame(tile, FULL_TILE_EDGES[t], drawn)
        return
    lay_out_sides(tile, drawn)
    widths = wobble(t)
    for y in range(8):
        for x in range(8):
            v = tile[y][x]
            if H_STROKE <= v < H_STROKE + BAND:
                tile[y][x] = stroke(v - H_STROKE, widths[x])
            elif V_STROKE <= v < V_STROKE + BAND:
                tile[y][x] = stroke(v - V_STROKE, widths[y])
            elif v == SIDE:
                tile[y][x] = INK
            elif (x, y) not in drawn and v == W:
                tile[y][x] = 5
                continue
            else:
                continue
            if not tile[y][x]:
                drawn.discard((x, y))


# --- Chapter status banners -------------------------------------------------------
# The banners use the player palette's blue ink, aged paper and white (the
# darkest ink is the outline at 1, as the shared code draws letter outlines
# with 1). They stay off the entries that hold the icons' original colours
# (style_tools.VANILLA_ICON_SLOTS); cream paper reuses the icons' cream.
SIGN_PALETTE = ["7080A0", "142450", "6888D0", "2C4CA0", "FFFFFF",
                None, None, None, None, None, "D8C49C", "CEB890",
                None, None, None, None]
for slot, colour in style_tools.VANILLA_ICON_SLOTS.items():
    SIGN_PALETTE[slot] = colour
MENU_TO_SIGN = {0: 0, INK_DARK: 1, INK_LIGHT: 2, INK: 3, W: 4,
                5: 5, 6: 5, 7: 5, 8: 5, 9: 5,
                10: 10, 11: 11, 12: 11, 13: 11, 14: 11, 15: 11}
# Vanilla PLAY TIME colours (OBJ palette 9) -> SIGN_PALETTE: blue ink face,
# light ink shading, white drop shadow.
LETTERS = {11: 3, 12: 3, 13: 2, 14: 5}
# The rest of the sheet (a spare banner with dragons): aged paper, inked
# dragons and trim.
REST_OF_SHEET = {
    0: 0, 1: 1, 2: 11, 3: 11, 4: 11, 5: 11, 6: 10, 7: 10, 8: 10, 9: 10,
    10: 3, 11: 5, 12: 5, 13: 2, 14: 1, 15: 5,
}
# The two item icons beside the banner keep vanilla's own colours.
ICON_COLOURS = style_tools.VANILLA_ICON_COLOURS


def label_tab(px, left, top, right, bottom):
    """A cream paper slip ruled in ink."""
    for y in range(top, bottom + 1):
        for x in range(left, right + 1):
            d = min(x - left, right - x, y - top, bottom - y)
            px[x, y] = 1 if d == 0 else 5
    for cx, cy in ((left, top), (right, top), (left, bottom), (right, bottom)):
        px[cx, cy] = 0


if __name__ == "__main__":
    build_style(sys.modules[__name__])
