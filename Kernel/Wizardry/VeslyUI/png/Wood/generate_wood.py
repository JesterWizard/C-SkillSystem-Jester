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
    window corners get square iron caps with a rivet instead;
  * a wood-grain pattern on the fill colours. The pattern repeats every 8x8
    tile, so tiles the game repeats side by side join into continuous grain.

Outputs (next to this script):
  Wood_*.png            indexed images, read by Png2Dmp from the installer
  Wood_MenuPal.pal      8 menu palettes (4 window colours + 4 guide rows)
  Wood_BattlePal.pal    4 battle UI palettes (one per faction)
  Wood_PrepItemsPal.pal prep-screen funds/item OBJ palette

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
# Wood uses 2 and 3 only for iron (corner caps), and 4 as the outline.
TRIM = ["7080A0", "F8F0D8", "B4ACA0", "706860", "24140A"]
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

# Corner tiles: where the 7x7 iron cap goes in the tile, and whether the tile
# is cleared first (corners with a transparent margin are redrawn as just the
# cap; full-tile corners keep their beam under it).
CORNERS = {
    0x01: (1, 1, True), 0x10: (1, 1, True),          # top left
    0x05: (0, 1, True),                              # top right
    0x1A: (1, 0, True),                              # bottom left
    0x21: (0, 0, True), 0x1F: (0, 0, True),          # bottom right
    0x72: (0, 0, False), 0x74: (1, 0, False),        # style 1, no margin
    0x77: (0, 1, False), 0x79: (1, 1, False),
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
BATTLE_CORNERS = {
    battle_tile(6, 0): (1, 1, True),                 # name box corners
    battle_tile(3, 0): (0, 1, True),
    battle_tile(11, 0): (1, 0, True),
    battle_tile(15, 0): (0, 0, True),
}
# Gem between the two item boxes: keep only the bar under it.
BATTLE_GEM = [battle_tile(8, 1), battle_tile(9, 1)]
# HP panel scrollwork: shaded wood becomes plain board.
BATTLE_SCROLLWORK = (
    [battle_tile(c, 2) for c in list(range(3, 8)) + list(range(13, 16))]
    + [battle_tile(c, 3) for c in [0, 1] + list(range(3, 16))]
)

CAP = [
    "DDDDDDD",
    "DLLLLMD",
    "DLMLMMD",
    "DLLWMMD",
    "DLMMDMD",
    "DMMMMMD",
    "DDDDDDD",
]
CAP_INDEX = {"D": D, "L": L, "M": M, "W": W}


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
        if t in corners:
            cx, cy, clear = corners[t]
            if clear:
                tile = [[0] * 8 for _ in range(8)]
                drawn = set()
            for y, line in enumerate(CAP):
                for x, ch in enumerate(line):
                    tile[cy + y][cx + x] = CAP_INDEX[ch]
                    drawn.add((cx + x, cy + y))
        put_tile(px, t, wide, tile)
        tx, ty = (t % wide) * 8, (t // wide) * 8
        keep |= {(tx + x, ty + y) for x, y in drawn}
    return keep


def save(image, palette, name):
    assert len(set(palette)) == 16, f"{name}: palette colours must be distinct"
    image.putpalette([c for color in palette for c in color])
    image.save(os.path.join(HERE, name), transparency=0)


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


if __name__ == "__main__":
    main()
