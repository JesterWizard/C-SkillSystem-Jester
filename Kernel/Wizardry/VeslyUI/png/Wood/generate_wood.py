#!/usr/bin/env python3
"""
Build the Wood VeslyUI style from the vanilla UI graphics in ../Vanilla.

Each vanilla image keeps its tile layout and window outline, so the game's
tilemaps still line up. Everything drawn on top of that changes:
  * the palettes: oak / mahogany / moss-stained / weathered wood instead of
    vanilla's blue / red / green / sepia;
  * the frame: vanilla's gold bars with a white stripe become dark wooden
    beams with a black outline, and every window edge gets that outline;
  * the ornaments: the gold scroll curls and gem crests are removed, and
    chips and cracks break up the edges, and at window corners the planks
    run past the window and end in a broken, splintered tip;
  * a wood-grain pattern on the fill colours. The pattern repeats every 8x8
    tile, so tiles the game repeats side by side join into continuous grain.

Outputs (next to this script):
  Wood_*.png            indexed images, read by Png2Dmp from the installer
  Wood_MenuPal.pal      8 menu palettes (4 window colours + 4 guide rows)
  Wood_BattlePal.pal    4 battle UI palettes (one per faction)
  Wood_PrepItemsPal.pal prep-screen funds/item OBJ palette
  Wood_PlayStatusPal.pal chapter status banner and playtime box OBJ palettes

Png2Dmp maps pixels by colour, so every PNG palette here has 16 distinct
colours. Rerun this script after changing a colour, then rebuild.
"""
import os
import struct

from PIL import Image

HERE = os.path.dirname(os.path.abspath(__file__))
VANILLA = os.path.join(HERE, "..", "Vanilla")


def rgb(text):
    return tuple(int(text[i:i + 2], 16) for i in (0, 2, 4))


# Menu palette index roles (vanilla gUiFramePaletteA):
#   0 backdrop, 1 white, 2 light trim, 3 mid trim, 4 dark trim,
#   5-9 light fill ramp (light -> dark), 10-15 frame ramp (light -> dark).
# The redrawn art no longer uses 2 and 3 (vanilla's gold); they stay a light
# and mid wood for any other graphics that share this palette. 4 is the outline.
TRIM = ["7080A0", "F8F0D8", "E2C490", "A07040", "24140A"]
W, L, M, D = 1, 2, 3, 4

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

# Battle palettes hold the same colours in another order (vanilla gUnknown_08802558).
BATTLE_ORDER = [0, 3, 1, 4, 2] + list(range(5, 16))

# Prep funds/items OBJ palette (vanilla gUnknown_08A1B638): 2-9 dark -> light ramp.
PREP_ITEMS = ["7080A0", "3A2614", "4A2E16", "5C3A1C", "6E4824", "80582E",
              "94683A", "A87C48", "BC9058", "CEA46A", "382818", "F8F0D8",
              "EAD8B4", "C0A47C", "8C7454", "000000"]

GRAIN_RAMPS_UI = [list(range(5, 10)), list(range(10, 16))]
GRAIN_RAMPS_PREP = [list(range(9, 1, -1))]

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


def to_gba(rows):
    out = bytearray()
    for row in rows:
        for r, g, b in row:
            out += struct.pack("<H", (r >> 3) | (g >> 3) << 5 | (b >> 3) << 10)
    return bytes(out)


def is_grain_line(x, y):
    return (y - GRAIN_WAVE[x % 8]) % 4 == 1


def add_grain(image, ramps, skip=frozenset()):
    px = image.load()
    lookup = {}
    for ramp in ramps:
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


# --- Frame redraw -----------------------------------------------------------
# Tile numbers below are positions in the 32x4 vanilla frame sheet
# (gUiFrameImage); DrawUiFrame's models in the decomp's uiutils.c name the
# corners and edges.

# Ornament tiles become a plain tile with the same role.
PLAIN_COPIES = {
    0x02: 0x03, 0x07: 0x06, 0x08: 0x09,              # window top-left, style 0
    0x0C: 0x0B, 0x0D: 0x0E,                          # window top-left, style 2
    0x11: 0x03, 0x12: 0x03, 0x13: 0x03,              # title plate crest
    0x14: 0x06, 0x15: 0x09, 0x16: 0x09,              # title plate curl, dark
    0x17: 0x0B, 0x18: 0x0E, 0x19: 0x0E,              # title plate curl, light
    0x31: 0x03, 0x32: 0x03, 0x33: 0x03,              # crest
    0x1D: 0x1C, 0x1E: 0x1C,                          # bottom bar curl
}

# Corner tiles. Windows with a transparent margin get CORNER_ART: the top and
# bottom planks run past the window and end in a broken, splintered tip.
# Style 1 windows fill their whole tile, so their corner is chipped off.
CORNERS = {
    0x01: "TL", 0x10: "TL", 0x05: "TR", 0x1A: "BL", 0x21: "BR", 0x1F: "BR",
    0x72: "chipTL", 0x74: "chipTR", 0x77: "chipBL", 0x79: "chipBR",
}

# The battle frame sheet (gUnknown_08801C14, 17 tiles wide) reuses the
# window art: a name box with a crest, and scrollwork engraved on the HP panel.
def battle_tile(col, row):
    return row * 17 + col


BATTLE_COPIES = {
    battle_tile(7, 0): battle_tile(1, 0),            # crest -> top bar
    battle_tile(8, 0): battle_tile(1, 0),
    battle_tile(9, 0): battle_tile(1, 0),
    battle_tile(10, 0): battle_tile(4, 0),           # curl -> left edge
}
BATTLE_CORNERS = {                                   # name box corners
    battle_tile(6, 0): "TL", battle_tile(3, 0): "TR",
    battle_tile(11, 0): "BL", battle_tile(15, 0): "BR",
}
# Gem between the two item boxes: keep only the bar under it.
BATTLE_GEM = [battle_tile(8, 1), battle_tile(9, 1)]
# HP panel scrollwork: shaded wood becomes plain board.
BATTLE_SCROLLWORK = (
    [battle_tile(c, 2) for c in list(range(3, 8)) + list(range(13, 16))]
    + [battle_tile(c, 3) for c in [0, 1] + list(range(3, 16))]
)

# Plank shading matches beam(): top planks are 4 pixels thick (rows 4-7),
# bottom planks 5 (rows 0-4). p and q are pale raw wood where it broke.
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


def beam(n, i):
    """Colour of position i across a beam n pixels thick: outlined dark wood."""
    if i == 0 or i == n - 1:
        return D
    inner = {3: [12], 4: [12, 14], 5: [12, 13, 14]}.get(n)
    if inner is None:
        inner = [12, 13] + [14] * (n - 5) + [15]
    return inner[i - 1]


# Big solid trim areas (patterned plates) keep their pattern in wood tones.
SOLID = {W: 11, L: 12, M: 15, D: 14}


def redraw_tile(tile):
    """Recolour one 8x8 tile (list of 8 lists). Returns the set of pixels drawn."""
    def trim(x, y):
        v = tile[y][x]
        if v in (L, M, D):
            return True
        if v != W:
            return False
        # White counts as trim only as a stripe between trim pixels.
        vert = 0 < y < 7 and tile[y - 1][x] in (L, M, D) and tile[y + 1][x] in (L, M, D)
        horz = 0 < x < 7 and tile[y][x - 1] in (L, M, D) and tile[y][x + 1] in (L, M, D)
        return vert or horz

    mask = [[trim(x, y) for x in range(8)] for y in range(8)]
    out = [row[:] for row in tile]
    drawn = set()
    for y in range(8):
        for x in range(8):
            if not mask[y][x]:
                continue
            y0 = y
            while y0 > 0 and mask[y0 - 1][x]:
                y0 -= 1
            y1 = y
            while y1 < 7 and mask[y1 + 1][x]:
                y1 += 1
            x0 = x
            while x0 > 0 and mask[y][x0 - 1]:
                x0 -= 1
            x1 = x
            while x1 < 7 and mask[y][x1 + 1]:
                x1 += 1
            lv, lh = y1 - y0 + 1, x1 - x0 + 1
            if lv >= 6 and lh >= 6:
                out[y][x] = SOLID[tile[y][x]]
            elif lv < lh:
                out[y][x] = beam(lv, y - y0)
            else:
                out[y][x] = beam(lh, x - x0)
            drawn.add((x, y))

    # Outline every edge that meets the transparent backdrop.
    for y in range(8):
        for x in range(8):
            if not out[y][x]:
                continue
            for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)):
                nx, ny = x + dx, y + dy
                if 0 <= nx < 8 and 0 <= ny < 8 and tile[ny][nx] == 0:
                    out[y][x] = D
                    drawn.add((x, y))
                    break
    tile[:] = out
    return drawn


def get_tile(px, t, tiles_wide):
    tx, ty = (t % tiles_wide) * 8, (t // tiles_wide) * 8
    return [[px[tx + x, ty + y] for x in range(8)] for y in range(8)]


def put_tile(px, t, tiles_wide, tile):
    tx, ty = (t % tiles_wide) * 8, (t // tiles_wide) * 8
    for y in range(8):
        for x in range(8):
            px[tx + x, ty + y] = tile[y][x]


def noise(*values):
    """Deterministic pseudo-random number in [0, 1000) for chip placement."""
    h = 2166136261
    for v in values:
        h = ((h ^ (v & 0xFFFFFFFF)) * 16777619) & 0xFFFFFFFF
    return h % 1000


def outer_edges(tile):
    """Straight outline runs that face the backdrop: (cells, inward step)."""
    edges = []
    for y in range(8):
        for dy in (1, -1):
            outside = y - dy
            if not 0 <= outside < 8:
                continue
            run = [x for x in range(1, 7)
                   if tile[y][x] == D and tile[outside][x] == 0
                   and 0 <= y + dy < 8 and tile[y + dy][x] not in (0,)]
            if len(run) >= 5:
                edges.append(([(x, y) for x in run], (0, dy)))
    for x in range(8):
        for dx in (1, -1):
            outside = x - dx
            if not 0 <= outside < 8:
                continue
            run = [y for y in range(1, 7)
                   if tile[y][x] == D and tile[y][outside] == 0
                   and 0 <= x + dx < 8 and tile[y][x + dx] not in (0,)]
            if len(run) >= 5:
                edges.append(([(x, y) for y in run], (dx, 0)))
    return edges


# Chips drawn on the interchangeable beam and edge tiles. In wood mode the
# DrawUiFrame replacement in C_Code.c picks among these tiles by position, so
# a long frame mixes plain and chipped stretches without a regular rhythm.
# Each chip: (kind, cell along the edge), cells counting from 0 at pixel 1.
# A streak is a grain line in the beam, not damage.
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


def step(cell, inward, n):
    return cell[0] + inward[0] * n, cell[1] + inward[1] * n


def paint(tile, cell, value, drawn):
    x, y = cell
    if 0 <= x < 8 and 0 <= y < 8 and tile[y][x]:
        tile[y][x] = value
        drawn.add((x, y))


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


def chip_edges(tile, t, drawn):
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
    if t in DRAWUIFRAME_PLAIN:
        return
    for n, (cells, inward) in enumerate(edges):
        roll = noise(t, n)
        if roll < 700:
            continue
        kind = "dent" if roll < 850 else "nick"
        chip(tile, cells, inward, kind, roll % (len(cells) - 1), drawn)


# Plain beam and edge tiles that DrawUiFrame repeats; they stay unchipped.
DRAWUIFRAME_PLAIN = {0x02, 0x03, 0x11, 0x13, 0x31, 0x33, 0x1B, 0x1E, 0x06, 0x0B,
                     0x0A, 0x0F, 0x73, 0x75, 0x76, 0x78}


def remove_specks(tile):
    """Clear the last few pixels of a curl that spilled into a fill tile."""
    flat = [v for row in tile for v in row]
    if flat.count(10) >= 56 and all(v >= 10 or v == D for v in flat):
        for row in tile:
            row[:] = [10] * 8


def redraw_frame(image, corners=None, copies=None, plain=(), gems=()):
    """Redraw trim and outlines in every tile; returns pixels to keep grain off."""
    corners = corners or {}
    copies = copies or {}
    px = image.load()
    wide = image.width // 8
    count = wide * (image.height // 8)
    original = [get_tile(px, t, wide) for t in range(count)]
    keep = set()
    for t in range(count):
        tile = [row[:] for row in original[copies.get(t, t)]]
        remove_specks(tile)
        if t in plain:
            tile = [[10 if v > 10 else v for v in row] for row in tile]
        if t in gems:
            tile = [[0] * 8 for _ in range(6)] + [[D] * 8, [W] * 8]
        drawn = redraw_tile(tile)
        corner = corners.get(t)
        if corner in CORNER_ART:
            tile = [[ART_INDEX[ch] for ch in line] for line in CORNER_ART[corner]]
            drawn = {(x, y) for y in range(8) for x in range(8)}
        elif corner:
            flip_x, flip_y = corner.endswith("R"), corner[-2] == "B"
            cut = [(7 - x if flip_x else x, 7 - y if flip_y else y) for x, y in CORNER_CHIP]
            for x, y in cut:
                tile[y][x] = 0
            for x, y in cut:
                for nx, ny in ((x + 1, y), (x - 1, y), (x, y + 1), (x, y - 1)):
                    if 0 <= nx < 8 and 0 <= ny < 8 and tile[ny][nx]:
                        tile[ny][nx] = D
                        drawn.add((nx, ny))
        else:
            chip_edges(tile, t, drawn)
        put_tile(px, t, wide, tile)
        tx, ty = (t % wide) * 8, (t // wide) * 8
        keep |= {(tx + x, ty + y) for x, y in drawn}
    return keep


def save(image, palette, name):
    assert len(set(palette)) == 16, f"{name}: palette colours must be distinct"
    image.putpalette([c for color in palette for c in color])
    image.save(os.path.join(HERE, name), transparency=0)


# --- Chapter status banners ---------------------------------------------------
# Img_PlayStatusSprites (32x8 tiles, 2D OBJ layout) holds the chapter title
# banner (OBJ palette 8) and the playtime box (OBJ palette 9). Both are rebuilt
# as wooden signs out of the frame tiles in Wood_Main.png, so they share the
# windows' planks, chips and broken corners. The rest of the sheet is unused on
# that screen and stays as is.
BANNER = (0, 0, 24, 4)        # x, y, width, height in tiles
# Wood's play-time box is 12 tiles wide (vanilla's is 10) so the clock fits;
# StatusScreenSpriteDraw_Loop in C_Code.c draws it with a wider sprite.
PLAYTIME = (16, 4, 12, 4)
PLAYTIME_LABEL = (11, 2, 55, 8)   # "PLAY TIME" letters in vanilla's box, pixels
PLAYTIME_LABEL_SHIFT = 10         # moves the label to the middle of the wider box

# Interchangeable tiles, as in the DrawUiFrame replacement in C_Code.c.
TOP_BEAMS = [0x03, 0x04, 0x11, 0x12, 0x13, 0x31, 0x32, 0x33]
BOTTOM_BEAMS = [0x1B, 0x1C, 0x1B, 0x1D, 0x1E, 0x1B, 0x20, 0x1C]
LEFT_EDGES = [0x06, 0x07, 0x06, 0x14]

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


def frame_pick(x, y):
    return (((x * 73 + y * 151) * 2654435761) & 0xFFFFFFFF) >> 24


def sign_tiles(cols, rows):
    grid = []
    for ty in range(rows):
        row = []
        for tx in range(cols):
            pick = frame_pick(tx, ty)
            if ty == 0:
                t = 0x01 if tx == 0 else 0x05 if tx == cols - 1 else TOP_BEAMS[pick & 7]
            elif ty == rows - 1:
                t = 0x1A if tx == 0 else 0x21 if tx == cols - 1 else BOTTOM_BEAMS[pick & 7]
            else:
                t = LEFT_EDGES[pick & 3] if tx == 0 else 0x0A if tx == cols - 1 else 0x09
            row.append(t)
        grid.append(row)
    return grid


def draw_sign(px, frame_px, area):
    x0, y0, cols, rows = area
    for ty, row in enumerate(sign_tiles(cols, rows)):
        for tx, t in enumerate(row):
            sx, sy = (t % 32) * 8, (t // 32) * 8
            for y in range(8):
                for x in range(8):
                    v = MENU_TO_SIGN[frame_px[sx + x, sy + y]]
                    px[(x0 + tx) * 8 + x, (y0 + ty) * 8 + y] = v


def draw_playtime_label(px, vanilla_px, area):
    """A small plank tab over the box's top beam, carrying vanilla's letters."""
    ax, ay = area[0] * 8, area[1] * 8
    lx, ly, lw, lh = PLAYTIME_LABEL
    shift = PLAYTIME_LABEL_SHIFT
    left, top, right, bottom = lx + shift - 2, ly - 2, lx + shift + lw + 1, ly + lh + 1
    for y in range(top, bottom + 1):
        for x in range(left, right + 1):
            edge = x in (left, right) or y in (top, bottom)
            px[ax + x, ay + y] = 1 if edge else (5 if y == top + 1 else 4)
    # Broken corners on the tab.
    for x, y in ((left, top), (right, top), (left, bottom), (right, bottom)):
        px[ax + x, ay + y] = 0
    px[ax + left + 1, ay + top + 1] = 1
    px[ax + right - 1, ay + bottom - 1] = 1
    px[ax + right - 2, ay + top + 1] = 9
    # Vanilla letters, in OBJ palette 9 order: 11 and 12 are the light face,
    # 13 its shading and 14 a mid-tone drop shadow. Their outline is the
    # plate's dark colour, which palette 9 holds twice (1 and 10); the "Y"
    # uses 1.
    letter = {11: 10, 12: 10, 13: 11, 14: 15}
    for y in range(ly, ly + lh):
        for x in range(lx, lx + lw):
            v = vanilla_px[ax + x, ay + y]
            if v in letter:
                px[ax + x + shift, ay + y] = letter[v]
            elif any(vanilla_px[ax + x + dx, ay + y + dy] in letter
                     for dx in (-1, 0, 1) for dy in (-1, 0, 1)):
                px[ax + x + shift, ay + y] = 1


def play_status():
    vanilla = Image.open(os.path.join(VANILLA, "Vanilla_PlayStatus.png"))
    frame = Image.open(os.path.join(HERE, "Wood_Main.png"))
    image = vanilla.copy()
    px, vpx, fpx = image.load(), vanilla.load(), frame.load()
    for area in (BANNER, PLAYTIME):
        draw_sign(px, fpx, area)
    draw_playtime_label(px, vpx, PLAYTIME)

    palette = [rgb(c) for c in SIGN_PALETTE]
    save(image, palette, "Wood_PlayStatus.png")
    open(os.path.join(HERE, "Wood_PlayStatusPal.pal"), "wb").write(to_gba([palette, palette]))
    print("Wood_PlayStatus.png")


def main():
    menu = menu_palette()
    battle = [[row[i] for i in BATTLE_ORDER] for row in menu[:4]]
    prep_items = [rgb(c) for c in PREP_ITEMS]

    open(os.path.join(HERE, "Wood_MenuPal.pal"), "wb").write(to_gba(menu))
    open(os.path.join(HERE, "Wood_BattlePal.pal"), "wb").write(to_gba(battle))
    open(os.path.join(HERE, "Wood_PrepItemsPal.pal"), "wb").write(to_gba([prep_items]))

    menu_order = list(range(16))
    jobs = [
        # vanilla name, palette for the PNG, grain ramps, index order or None
        # to leave the art as is, then the frame tiles to replace
        ("Main", menu[0], GRAIN_RAMPS_UI, menu_order, CORNERS, PLAIN_COPIES),
        ("Preps", menu[0], GRAIN_RAMPS_UI, menu_order),
        ("PrepsUseGfx", menu[0], GRAIN_RAMPS_UI, menu_order),
        ("PrepPopupWindow", menu[0], GRAIN_RAMPS_UI, menu_order),
        ("ShopGoldDisplay", menu[0], GRAIN_RAMPS_UI, menu_order),
        ("PrepItems", prep_items, GRAIN_RAMPS_PREP, None),
        ("Battle", battle[0], GRAIN_RAMPS_UI, BATTLE_ORDER, BATTLE_CORNERS, BATTLE_COPIES,
         BATTLE_SCROLLWORK, BATTLE_GEM),
        ("Left", battle[0], GRAIN_RAMPS_UI, BATTLE_ORDER),
        ("LeftItem", battle[0], GRAIN_RAMPS_UI, BATTLE_ORDER),
        ("Right", battle[0], GRAIN_RAMPS_UI, BATTLE_ORDER),
        ("RightItem", battle[0], GRAIN_RAMPS_UI, BATTLE_ORDER),
    ]
    for name, palette, ramps, order, *tiles in jobs:
        image = Image.open(os.path.join(VANILLA, f"Vanilla_{name}.png")).copy()
        keep = set()
        if order is not None:
            # Redraw in menu palette order, then map back to the image's own.
            image.putdata([order[i] for i in image.tobytes()])
            keep = redraw_frame(image, *tiles)
            image.putdata([order.index(i) for i in image.tobytes()])
        add_grain(image, ramps, keep)
        save(image, palette, f"Wood_{name}.png")
        print(f"Wood_{name}.png")

    play_status()


if __name__ == "__main__":
    main()
