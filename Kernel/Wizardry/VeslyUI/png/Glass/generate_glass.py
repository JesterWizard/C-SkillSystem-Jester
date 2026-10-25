#!/usr/bin/env python3
"""
Build the Stained Glass VeslyUI style from the vanilla UI graphics in ../Vanilla.

The shared redraw lives in ../style_tools.py; this script sets the look:
  * the palettes: blue / red / green / purple glass panels and frosted light
    panels, instead of vanilla's blue / red / green / sepia;
  * the frame: a border of amber, ruby, emerald and sapphire glass pieces set
    in lead, 4 pixels thick and the same on all four sides;
  * a square emerald or sapphire pane on each window corner, like the corner
    blocks of a church window;
  * faint diagonal seams across the panels. The panels carry no lead lines,
    so text stays readable.

Outputs (next to this script):
  Glass_*.png             indexed images, read by Png2Dmp from the installer
  Glass_MenuPal.pal       8 menu palettes (4 window colours + 4 guide rows)
  Glass_BattlePal.pal     4 battle UI palettes (one per faction)
  Glass_PrepItemsPal.pal  prep-screen funds/item OBJ palette
  Glass_PlayStatusPal.pal chapter status banner and playtime box OBJ palettes

Rerun this script after changing a colour, then rebuild.
"""
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), ".."))
from style_tools import D, L, M, W, build_style, rgb  # noqa: E402
import style_tools  # noqa: E402

NAME = "Glass"
HERE = os.path.dirname(os.path.abspath(__file__))

# Menu palette entries. White (1) stays white: the battle screen draws its
# name and weapon text with it. 2 and 3 (vanilla's gold) hold the border's
# amber and ruby glass, 4 the lead. The two darkest light panel shades hold
# the corner panes' emerald (8) and sapphire (9); light panels use 5-7.
AMBER, RUBY, LEAD = 2, 3, D
EMERALD, SAPPHIRE = 8, 9
PANEL = 10

TRIM = ["FFFFFF", "F0B030", "D02848", "1C1418"]     # 1 white, 2 amber, 3 ruby, 4 lead
FROST = ["EEF2FF", "DCE4F8", "C8D2EE"]              # 5-7
CORNER_GLASS = ["30B060", "3070E0"]                 # 8 emerald, 9 sapphire

PANELS = [
    # Blue glass: window colour 0, player faction.
    ["3858C8", "3050B8", "2848A8", "204098", "183888", "103078"],
    # Red glass: window colour 1, enemy faction.
    ["C03040", "B02838", "A02030", "901C2C", "801826", "701420"],
    # Green glass: window colour 2, allied (green) faction.
    ["289858", "229050", "1C8048", "187040", "146038", "105030"],
    # Purple glass: window colour 3, other (purple) faction.
    ["7040B0", "6838A0", "603090", "582880", "502070", "481860"],
]

# Prep funds/items OBJ palette (vanilla gUnknown_08A1B638): 2-9 dark -> light ramp.
PREP_ITEMS = ["7080A0", "100C18", "103078", "183888", "2848A8", "3858C8",
              "5070D8", "7090E4", "98B0EC", "C0D0F4", "0C0810", "FFFFFF",
              "DCE4F8", "F0B030", "D02848", "000000"]

SEAM_RAMPS = {"ui": [[5, 6, 7], list(range(10, 16))],
              "prep": [list(range(9, 1, -1))]}


def ui_row(panel, dim=1.0):
    row = ([rgb("7080A0")] + [rgb(c) for c in TRIM] + [rgb(c) for c in FROST]
           + [rgb(c) for c in CORNER_GLASS] + [rgb(c) for c in panel])
    if dim != 1.0:
        row = [row[0]] + [tuple(int(v * dim) for v in c) for c in row[1:]]
    return row


def menu_palette():
    rows = [ui_row(p) for p in PANELS]
    rows += [ui_row(p, 0.78) for p in PANELS]
    return rows


def texture(image, kind, skip=frozenset()):
    """Glass seams: a faint diagonal line, one shade darker, every 8 pixels."""
    px = image.load()
    lookup = {}
    for ramp in SEAM_RAMPS[kind]:
        for i, index in enumerate(ramp):
            lookup[index] = (ramp, i)

    for y in range(image.height):
        for x in range(image.width):
            index = px[x, y]
            if index not in lookup or (x + y) % 8 != 0 or (x, y) in skip:
                continue
            ramp, i = lookup[index]
            px[x, y] = ramp[i + 1] if i + 1 < len(ramp) else ramp[i - 1]


# --- Frame ------------------------------------------------------------------------
# A 4-pixel border on every side: lead, two rows of glass, lead. Each border
# tile holds one glass piece, 7 pixels long, closed by a lead bar on its last
# pixel. The pieces' colours vary from tile to tile; the DrawUiFrame
# replacement in C_Code.c mixes the interchangeable beam and left edge tiles
# by position, so the colours fall irregularly, like real stained glass. Top
# and bottom bars hold the border inside vanilla's window outline; side edges
# push it outward (left edges start at x 3, right edges end at x 5), so all
# four sides match.
H_GLASS, V_GLASS = 100, 101     # glass placeholders, coloured by tile later

# Glass colour of each interchangeable border tile (style_tools.TOP_BEAMS and
# friends); other tiles pick one from noise().
PIECES = {
    0x03: RUBY, 0x04: AMBER, 0x11: SAPPHIRE, 0x12: AMBER,          # top
    0x13: EMERALD, 0x31: RUBY, 0x32: AMBER, 0x33: SAPPHIRE,
    0x1B: AMBER, 0x1C: RUBY, 0x1D: EMERALD, 0x1E: SAPPHIRE,        # bottom
    0x20: AMBER,
    0x06: AMBER, 0x07: RUBY, 0x14: SAPPHIRE,                       # left
    0x0B: AMBER, 0x0C: EMERALD, 0x17: RUBY,
    0x0A: AMBER, 0x0F: RUBY,                                       # right
}
GLASS = [AMBER, RUBY, EMERALD, SAPPHIRE]


def piece_colour(t):
    return PIECES.get(t, GLASS[style_tools.noise(t) % 4])


def piece(along, colour):
    """Glass colour, or the closing lead bar, at a position along a tile."""
    return LEAD if along % 8 == 7 else colour


def border(q, glass):
    """Colour q pixels from the panel side of the border."""
    return LEAD if q in (0, 3) else glass


def beam(n, i, outer):
    """A bar n pixels thick. At a window edge: the border against the panel,
    nothing outside it. Between two panels: a glass rail in lead."""
    if outer == 0:
        return LEAD if i in (0, n - 1) or n < 3 else H_GLASS
    q = n - 1 - i if outer < 0 else i      # distance from the panel side
    return border(q, H_GLASS) if q < 4 else 0


# Big solid trim areas (patterned plates, the battle item boxes) become panel.
SOLID = {W: 11, L: 12, M: 15, D: 14}

# Square corner panes, 6x6, set over the border corner and standing 2 pixels
# proud of it. e is the pane's glass (emerald or sapphire, by corner), w a
# white glint, a the amber piece where the top or bottom border runs in.
CORNER_ART = {
    "TL": ["........",
           "........",
           ".DDDDDD.",
           ".DweeeDD",
           ".DeeeeDD",
           ".DeeeeDD",
           ".DeeeeDD",
           ".DDDDDDD"],
    "TR": ["........",
           "........",
           "..DDDDDD",
           "..DweeeD",
           "DDDeeeeD",
           "aaDeeeeD",
           "aaDeeeeD",
           "DDDDDDDD"],
    "BL": [".DDDDDDD",
           ".DweeeDD",
           ".DeeeeDD",
           ".DeeeeDD",
           ".DeeeeD.",
           ".DDDDDD.",
           "........",
           "........"],
    "BR": ["DDDDDDDD",
           "aaDweeeD",
           "aaDeeeeD",
           "DDDeeeeD",
           "..DeeeeD",
           "..DDDDDD",
           "........",
           "........"],
}
CORNER_PANE = {"TL": EMERALD, "TR": SAPPHIRE, "BL": SAPPHIRE, "BR": EMERALD}


def corner_art():
    out = {}
    for name, rows in CORNER_ART.items():
        pane = {"e": "E" if CORNER_PANE[name] == EMERALD else "S"}
        out[name] = ["".join(pane.get(ch, ch) for ch in row) for row in rows]
    return out


CORNER_ART = corner_art()
ART_INDEX = {".": 0, "D": LEAD, "E": EMERALD, "S": SAPPHIRE, "w": W, "a": AMBER}

# The "Sort" box and similar windows start with this top-left tile.
EXTRA_CORNERS = {0x61: "TL"}

# Window style 1 fills its whole tiles; its edge and corner tiles get the
# border along their outer sides, and a pane where two sides meet.
FULL_TILE_EDGES = {0x73: "T", 0x75: "L", 0x76: "R", 0x78: "B"}


def full_frame(tile, sides, drawn):
    for y in range(8):
        for x in range(8):
            dist = {"T": y, "B": 7 - y, "L": x, "R": 7 - x}
            near = [s for s in sides if dist[s] < 4]
            if not near:
                tile[y][x] = PANEL
                continue
            drawn.add((x, y))
            if len(near) == 2:                  # the corner pane
                d = [dist[s] for s in near]
                tile[y][x] = LEAD if 0 in d or 3 in d else EMERALD
                continue
            s = near[0]
            along = x if s in "TB" else y
            colour = {"T": AMBER, "B": RUBY, "L": RUBY, "R": AMBER}[s]
            tile[y][x] = border(3 - dist[s], piece(along, colour))


def full_corner(tile, corner, drawn):
    full_frame(tile, corner, drawn)


def push_sides(tile, drawn):
    """Turn each left and right outline pixel into the full border, pushed
    outward so it is 4 pixels thick like the top and bottom bars."""
    lines = [(x, y, dx) for y in range(8) for x in range(8) for dx in (1, -1)
             if tile[y][x] == LEAD and 0 <= x - dx < 8 and tile[y][x - dx] == 0]
    for x, y, dx in lines:
        for q in range(4):                      # panel side -> outside
            px_ = x - dx * q
            if 0 <= px_ < 8:
                tile[y][px_] = border(q, V_GLASS)
                drawn.add((px_, y))
        inner = x + dx
        if 0 <= inner < 8 and tile[y][inner] == 9:          # vanilla's edge light
            tile[y][inner] = PANEL


def edge_details(tile, t, drawn):
    """Colour the border glass, frost the light panels (white stays in the
    palette for text), and fold their two darkest shades into frost too
    (those entries hold corner glass)."""
    if t in FULL_TILE_EDGES:
        full_frame(tile, FULL_TILE_EDGES[t], drawn)
        return
    push_sides(tile, drawn)
    for y in range(8):
        for x in range(8):
            v = tile[y][x]
            if v == H_GLASS:
                tile[y][x] = piece(x, piece_colour(t))
            elif v == V_GLASS:
                tile[y][x] = piece(y, piece_colour(t))
            elif (x, y) not in drawn and v in (EMERALD, SAPPHIRE):
                tile[y][x] = 7
            elif (x, y) not in drawn and v == W:
                tile[y][x] = 5


# --- Chapter status banners -------------------------------------------------------
# The banners use the player palette's glass, lead and white (lead at 1, as
# the shared code draws letter outlines with 1). They stay off the entries
# that hold the icons' original colours (style_tools.VANILLA_ICON_SLOTS); the
# corner panes reuse the icons' green and blue gem.
SIGN_PALETTE = ["7080A0", "1C1418", "F0B030", "D02848", "FFFFFF",
                None, None, None, None, None, "3858C8", "3050B8",
                None, None, None, None]
for slot, colour in style_tools.VANILLA_ICON_SLOTS.items():
    SIGN_PALETTE[slot] = colour
MENU_TO_SIGN = {0: 0, LEAD: 1, W: 4, AMBER: 2, RUBY: 3,
                5: 6, 6: 6, 7: 6, EMERALD: 13, SAPPHIRE: 15,
                10: 10, 11: 11, 12: 11, 13: 11, 14: 10, 15: 10}
# Vanilla PLAY TIME colours (OBJ palette 9) -> SIGN_PALETTE: white face, pale
# blue shading, dark glass drop shadow.
LETTERS = {11: 4, 12: 4, 13: 6, 14: 11}
# The rest of the sheet (a spare banner with dragons): blue glass ground,
# amber dragons, ruby trim.
REST_OF_SHEET = {
    0: 0, 1: 3, 2: 11, 3: 11, 4: 11, 5: 11, 6: 10, 7: 10, 8: 10, 9: 10,
    10: 2, 11: 6, 12: 6, 13: 15, 14: 1, 15: 6,
}
# The two item icons beside the banner keep vanilla's own colours.
ICON_COLOURS = style_tools.VANILLA_ICON_COLOURS


def label_tab(px, left, top, right, bottom):
    """A glass plate in lead, with an amber inner ring."""
    for y in range(top, bottom + 1):
        for x in range(left, right + 1):
            d = min(x - left, right - x, y - top, bottom - y)
            px[x, y] = 1 if d == 0 else 2 if d == 1 else 10
    for cx, cy in ((left, top), (right, top), (left, bottom), (right, bottom)):
        px[cx, cy] = 0


if __name__ == "__main__":
    build_style(sys.modules[__name__])
