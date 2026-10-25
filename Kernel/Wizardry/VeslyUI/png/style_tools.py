"""
Shared tools for building a VeslyUI style from the vanilla UI graphics in
Vanilla/. Each style has its own folder with a generate_<name>.py script that
defines the look and calls build_style().

Every vanilla image keeps its tile layout and window outline, so the game's
tilemaps still line up. What a style redraws on top of that:
  * trim bars, scored by each pixel's position across the bar (style.beam);
  * an outline wherever a window edge meets the transparent backdrop
    (style.OUTLINE);
  * ornament tiles (curls, crests, the battle gem), replaced with the plain
    tile of the same role;
  * corner tiles, from style.CORNER_ART;
  * details on beam and edge tiles (style.edge_details);
  * a texture on the fill colours (style.texture);
  * the chapter status banner and play-time box, built from the finished frame
    tiles so they match the windows.

A style script defines these names (see Wood/generate_wood.py):
  NAME, HERE              file prefix and output folder
  menu_palette()          8 rows: 4 window colours + 4 dimmed guide rows
  PREP_ITEMS              prep funds/items OBJ palette, 16 hex colours
  beam(n, i, outer)       colour across a bar n pixels thick; outer is the
                          side facing the backdrop: -1 before index 0, +1
                          after index n-1, 0 if unknown
  OUTLINE                 colour where a window edge meets the backdrop
                          (optional; D, the black outline, by default)
  SOLID                   trim index -> colour for big solid trim areas
  CORNER_ART, ART_INDEX   8x8 corner tiles ("TL", "TR", "BL", "BR")
  EXTRA_CORNERS           more frame tiles to treat as corners (optional)
  full_corner(tile, corner, drawn)  corner of a window with no margin
  edge_details(tile, t, drawn)      details on a tile's outer edges
  texture(image, kind, skip)        fill texture; kind is "ui" or "prep"
  SIGN_PALETTE, MENU_TO_SIGN        palette and colour map for the banners
  label_tab(px, left, top, right, bottom)  plate behind PLAY TIME
  LETTERS                 vanilla PLAY TIME colours -> SIGN_PALETTE
  REST_OF_SHEET           vanilla colours -> SIGN_PALETTE for the rest of the
                          banner sheet (optional; left as is by default)
  ICON_COLOURS            the same for the icons in SHEET_ICONS (optional)

Png2Dmp maps pixels by colour, so every PNG palette has 16 distinct colours.
"""
import os
import struct

from PIL import Image

VANILLA = os.path.join(os.path.dirname(os.path.abspath(__file__)), "Vanilla")

# Menu palette index roles (vanilla gUiFramePaletteA):
#   0 backdrop, 1 white, 2 light trim, 3 mid trim, 4 dark trim,
#   5-9 light fill ramp (light -> dark), 10-15 frame ramp (light -> dark).
W, L, M, D = 1, 2, 3, 4

# Battle palettes hold the same colours in another order (vanilla gUnknown_08802558).
BATTLE_ORDER = [0, 3, 1, 4, 2] + list(range(5, 16))


def rgb(text):
    return tuple(int(text[i:i + 2], 16) for i in (0, 2, 4))


def to_gba(rows):
    out = bytearray()
    for row in rows:
        for r, g, b in row:
            out += struct.pack("<H", (r >> 3) | (g >> 3) << 5 | (b >> 3) << 10)
    return bytes(out)


def save(image, palette, path):
    assert len(set(palette)) == 16, f"{path}: palette colours must be distinct"
    image.putpalette([c for color in palette for c in color])
    image.save(path, transparency=0)


# --- Tile roles ---------------------------------------------------------------
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

# Corner tiles. Windows with a transparent margin get the style's CORNER_ART;
# style 1 windows fill their whole tile and get style.full_corner().
CORNERS = {
    0x01: "TL", 0x10: "TL", 0x05: "TR", 0x1A: "BL", 0x21: "BR", 0x1F: "BR",
    0x72: "fullTL", 0x74: "fullTR", 0x77: "fullBL", 0x79: "fullBR",
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
# HP panel scrollwork: shaded panel becomes plain.
BATTLE_SCROLLWORK = (
    [battle_tile(c, 2) for c in list(range(3, 8)) + list(range(13, 16))]
    + [battle_tile(c, 3) for c in [0, 1] + list(range(3, 16))]
)

# Interchangeable beam and edge tiles. For styles with edge details, the
# DrawUiFrame replacement in C_Code.c picks among these by position, so the
# details along a long frame don't fall in a regular rhythm. The lists must
# match sFrameTopBeams and friends there.
TOP_BEAMS = [0x03, 0x04, 0x11, 0x12, 0x13, 0x31, 0x32, 0x33]
BOTTOM_BEAMS = [0x1B, 0x1C, 0x1B, 0x1D, 0x1E, 0x1B, 0x20, 0x1C]
LEFT_EDGES = [0x06, 0x07, 0x06, 0x14]

# Plain beam and edge tiles that DrawUiFrame repeats.
DRAWUIFRAME_PLAIN = {0x02, 0x03, 0x11, 0x13, 0x31, 0x33, 0x1B, 0x1E, 0x06, 0x0B,
                     0x0A, 0x0F, 0x73, 0x75, 0x76, 0x78}


def frame_pick(x, y):
    """Same position hash as PickFrameTile in C_Code.c."""
    return (((x * 73 + y * 151) * 2654435761) & 0xFFFFFFFF) >> 24


# --- Tile helpers ---------------------------------------------------------------
def get_tile(px, t, tiles_wide):
    tx, ty = (t % tiles_wide) * 8, (t // tiles_wide) * 8
    return [[px[tx + x, ty + y] for x in range(8)] for y in range(8)]


def put_tile(px, t, tiles_wide, tile):
    tx, ty = (t % tiles_wide) * 8, (t // tiles_wide) * 8
    for y in range(8):
        for x in range(8):
            px[tx + x, ty + y] = tile[y][x]


def noise(*values):
    """Deterministic pseudo-random number in [0, 1000) for detail placement."""
    h = 2166136261
    for v in values:
        h = ((h ^ (v & 0xFFFFFFFF)) * 16777619) & 0xFFFFFFFF
    return h % 1000


def outer_edges(tile, outline=D):
    """Straight outline runs that face the backdrop: (cells, inward step)."""
    edges = []
    for y in range(8):
        for dy in (1, -1):
            outside = y - dy
            if not 0 <= outside < 8:
                continue
            run = [x for x in range(1, 7)
                   if tile[y][x] == outline and tile[outside][x] == 0
                   and 0 <= y + dy < 8 and tile[y + dy][x] not in (0,)]
            if len(run) >= 5:
                edges.append(([(x, y) for x in run], (0, dy)))
    for x in range(8):
        for dx in (1, -1):
            outside = x - dx
            if not 0 <= outside < 8:
                continue
            run = [y for y in range(1, 7)
                   if tile[y][x] == outline and tile[y][outside] == 0
                   and 0 <= x + dx < 8 and tile[y][x + dx] not in (0,)]
            if len(run) >= 5:
                edges.append(([(x, y) for y in run], (dx, 0)))
    return edges


def step(cell, inward, n):
    return cell[0] + inward[0] * n, cell[1] + inward[1] * n


def paint(tile, cell, value, drawn):
    x, y = cell
    if 0 <= x < 8 and 0 <= y < 8 and tile[y][x]:
        tile[y][x] = value
        drawn.add((x, y))


# --- Frame redraw -----------------------------------------------------------------
def redraw_tile(tile, style):
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

    def outer(before, after):
        """Which end of a run faces the backdrop."""
        if before is not None and before == 0:
            return -1
        if after is not None and after == 0:
            return 1
        return 0

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
                out[y][x] = style.SOLID[tile[y][x]]
            elif lv < lh:
                side = outer(tile[y0 - 1][x] if y0 > 0 else None,
                             tile[y1 + 1][x] if y1 < 7 else None)
                out[y][x] = style.beam(lv, y - y0, side)
            else:
                side = outer(tile[y][x0 - 1] if x0 > 0 else None,
                             tile[y][x1 + 1] if x1 < 7 else None)
                out[y][x] = style.beam(lh, x - x0, side)
            drawn.add((x, y))

    # Outline every edge that meets the transparent backdrop.
    outline = getattr(style, "OUTLINE", D)
    for y in range(8):
        for x in range(8):
            if not out[y][x]:
                continue
            for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)):
                nx, ny = x + dx, y + dy
                if 0 <= nx < 8 and 0 <= ny < 8 and tile[ny][nx] == 0:
                    out[y][x] = outline
                    drawn.add((x, y))
                    break
    tile[:] = out
    return drawn


def remove_specks(tile):
    """Clear the last few pixels of a curl that spilled into a fill tile."""
    flat = [v for row in tile for v in row]
    if flat.count(10) >= 56 and all(v >= 10 or v == D for v in flat):
        for row in tile:
            row[:] = [10] * 8


def redraw_frame(image, style, corners=None, copies=None, plain=(), gems=()):
    """Redraw trim and outlines in every tile; returns pixels to keep texture off."""
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
        drawn = redraw_tile(tile, style)
        corner = corners.get(t)
        if corner in style.CORNER_ART:
            tile = [[style.ART_INDEX[ch] for ch in line] for line in style.CORNER_ART[corner]]
            drawn = {(x, y) for y in range(8) for x in range(8)}
        elif corner:
            style.full_corner(tile, corner[4:], drawn)
        else:
            style.edge_details(tile, t, drawn)
        put_tile(px, t, wide, tile)
        tx, ty = (t % wide) * 8, (t // wide) * 8
        keep |= {(tx + x, ty + y) for x, y in drawn}
    return keep


# --- Chapter status banners -------------------------------------------------------
# Img_PlayStatusSprites (32x8 tiles, 2D OBJ layout) holds the chapter title
# banner (OBJ palette 8) and the playtime box (OBJ palette 9). Both are rebuilt
# out of the style's frame tiles, so they share the windows' look. The rest of
# the sheet is unused on that screen and stays as is.
BANNER = (0, 0, 24, 4)        # x, y, width, height in tiles
# The play-time box is 12 tiles wide (vanilla's is 10) so the clock fits;
# StatusScreenSpriteDraw_Loop in C_Code.c draws it with a wider sprite.
PLAYTIME = (16, 4, 12, 4)
PLAYTIME_LABEL = (11, 2, 55, 8)   # "PLAY TIME" letters in vanilla's box, pixels
PLAYTIME_LABEL_SHIFT = 10         # moves the label to the middle of the wider box
SHEET_ICONS = (24, 0, 8, 4)       # two item icons beside the banner, in tiles

# The icons keep vanilla's own colours (OBJ palette 8). A style that wants
# that puts these colours at these SIGN_PALETTE entries, keeps its banners off
# them, and sets ICON_COLOURS = VANILLA_ICON_COLOURS. Only the icons'
# near-black detail colour (3 pixels) shares the outline entry, 1.
VANILLA_ICON_SLOTS = {
    5: "E8D8C8",                      # cream
    6: "B8D8F8",                      # pale blue halo
    7: "205028", 8: "306038", 9: "406840",
    12: "507850", 13: "608860", 14: "98B090",   # greens
    15: "70A8C0",                     # gem
}
VANILLA_ICON_COLOURS = {
    0: 0, 1: 5, 2: 7, 3: 8, 4: 9, 5: 12, 6: 13, 7: 13, 8: 14, 9: 14,
    10: 1, 11: 6, 12: 6, 13: 15, 14: 1, 15: 6,
}


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


def draw_sign(px, frame_px, area, menu_to_sign):
    x0, y0, cols, rows = area
    for ty, row in enumerate(sign_tiles(cols, rows)):
        for tx, t in enumerate(row):
            sx, sy = (t % 32) * 8, (t // 32) * 8
            for y in range(8):
                for x in range(8):
                    v = menu_to_sign[frame_px[sx + x, sy + y]]
                    px[(x0 + tx) * 8 + x, (y0 + ty) * 8 + y] = v


def draw_playtime_label(px, vanilla_px, area, style):
    """A small plate over the box's top edge, carrying vanilla's letters."""
    ax, ay = area[0] * 8, area[1] * 8
    lx, ly, lw, lh = PLAYTIME_LABEL
    shift = PLAYTIME_LABEL_SHIFT
    style.label_tab(px, ax + lx + shift - 2, ay + ly - 2, ax + lx + shift + lw + 1, ay + ly + lh + 1)
    # Vanilla letters, in OBJ palette 9 order: 11 and 12 are the light face,
    # 13 its shading and 14 a mid-tone drop shadow. Their outline is the
    # plate's dark colour, which palette 9 holds twice (1 and 10); the "Y"
    # uses 1. style.LETTERS maps the face, shading and shadow; outline is 1.
    letter = style.LETTERS
    for y in range(ly, ly + lh):
        for x in range(lx, lx + lw):
            v = vanilla_px[ax + x, ay + y]
            if v in letter:
                px[ax + x + shift, ay + y] = letter[v]
            elif any(vanilla_px[ax + x + dx, ay + y + dy] in letter
                     for dx in (-1, 0, 1) for dy in (-1, 0, 1)):
                px[ax + x + shift, ay + y] = 1


def play_status(style):
    vanilla = Image.open(os.path.join(VANILLA, "Vanilla_PlayStatus.png"))
    frame = Image.open(os.path.join(style.HERE, f"{style.NAME}_Main.png"))
    image = vanilla.copy()
    px, vpx, fpx = image.load(), vanilla.load(), frame.load()
    # The rest of the sheet (a spare banner and some icons) keeps vanilla's
    # art; a style can map its colours by role so it suits the new palette.
    rest = getattr(style, "REST_OF_SHEET", None)
    if rest:
        for y in range(image.height):
            for x in range(image.width):
                px[x, y] = rest[vpx[x, y]]
    icons = getattr(style, "ICON_COLOURS", None)
    if icons:
        ix, iy, iw, ih = SHEET_ICONS
        for y in range(iy * 8, (iy + ih) * 8):
            for x in range(ix * 8, (ix + iw) * 8):
                px[x, y] = icons[vpx[x, y]]
    for area in (BANNER, PLAYTIME):
        draw_sign(px, fpx, area, style.MENU_TO_SIGN)
    draw_playtime_label(px, vpx, PLAYTIME, style)

    palette = [rgb(c) for c in style.SIGN_PALETTE]
    save(image, palette, os.path.join(style.HERE, f"{style.NAME}_PlayStatus.png"))
    with open(os.path.join(style.HERE, f"{style.NAME}_PlayStatusPal.pal"), "wb") as f:
        f.write(to_gba([palette, palette]))
    print(f"{style.NAME}_PlayStatus.png")


# --- Build ------------------------------------------------------------------------
def build_style(style):
    menu = style.menu_palette()
    battle = [[row[i] for i in BATTLE_ORDER] for row in menu[:4]]
    prep_items = [rgb(c) for c in style.PREP_ITEMS]

    def out(name):
        return os.path.join(style.HERE, f"{style.NAME}_{name}")

    with open(out("MenuPal.pal"), "wb") as f:
        f.write(to_gba(menu))
    with open(out("BattlePal.pal"), "wb") as f:
        f.write(to_gba(battle))
    with open(out("PrepItemsPal.pal"), "wb") as f:
        f.write(to_gba([prep_items]))

    menu_order = list(range(16))
    jobs = [
        # vanilla name, palette for the PNG, texture kind, index order or None
        # to leave the art as is, then the frame tiles to replace
        ("Main", menu[0], "ui", menu_order, {**CORNERS, **getattr(style, "EXTRA_CORNERS", {})},
         PLAIN_COPIES),
        ("Preps", menu[0], "ui", menu_order),
        ("PrepsUseGfx", menu[0], "ui", menu_order),
        ("PrepPopupWindow", menu[0], "ui", menu_order),
        ("ShopGoldDisplay", menu[0], "ui", menu_order),
        ("PrepItems", prep_items, "prep", None),
        ("Battle", battle[0], "ui", BATTLE_ORDER, BATTLE_CORNERS, BATTLE_COPIES,
         BATTLE_SCROLLWORK, BATTLE_GEM),
        ("Left", battle[0], "ui", BATTLE_ORDER),
        ("LeftItem", battle[0], "ui", BATTLE_ORDER),
        ("Right", battle[0], "ui", BATTLE_ORDER),
        ("RightItem", battle[0], "ui", BATTLE_ORDER),
    ]
    for name, palette, kind, order, *tiles in jobs:
        image = Image.open(os.path.join(VANILLA, f"Vanilla_{name}.png")).copy()
        keep = set()
        if order is not None:
            # Redraw in menu palette order, then map back to the image's own.
            image.putdata([order[i] for i in image.tobytes()])
            keep = redraw_frame(image, style, *tiles)
            image.putdata([order.index(i) for i in image.tobytes()])
        style.texture(image, kind, keep)
        save(image, palette, out(f"{name}.png"))
        print(f"{style.NAME}_{name}.png")

    play_status(style)
