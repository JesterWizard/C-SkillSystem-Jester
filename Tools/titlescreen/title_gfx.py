#!/usr/bin/env python3
"""Turn the title-screen art into GBA graphics data (part 1: BG1 background and BG0 emblem).

    python3 Tools/titlescreen/title_gfx.py

Reads Kernel/Wizardry/TitleScreenGfx/src/title_background.png (RGB) and
Kernel/Wizardry/TitleScreenGfx/src/title_logo.png (RGBA), and writes:

    Kernel/Wizardry/TitleScreenGfx/dmp/   the data the game loads
        bg_tiles_a.lz, bg_tiles_b.lz  background tiles 0-383 and 384-639 (4bpp, LZ77)
        bg_map.lz                     background tilemap, 32x20 u16 (LZ77)
        bg_pal.bin                    background palette, 16 x RGB555
        emblem_tiles.lz               emblem tiles, 4bpp, tile 0 blank (LZ77)
        emblem_map.lz                 emblem tilemap, 32x20 u16 (LZ77)
        emblem_pal.bin                emblem palette, 16 x RGB555
    Kernel/Wizardry/TitleScreenGfx/gfx/   pictures to look at (not used by the game)
        background.png, emblem.png    indexed; index 0 is shown as magenta
        preview_bg.png                background with the emblem over it, x2

Part 2 (OBJ logo, intro zoom) adds:

    Kernel/Wizardry/TitleScreenGfx/dmp/
        obj_tiles.lz                  OBJ tiles, 4bpp, 352 tiles (11 rows of 32) (LZ77)
        obj_pal.bin                   OBJ palette, 16 x RGB555 (banks 0-1 from the ROM, 2 and 3 new)
        zoom_tiles.lz                 zoom tiles, 8bpp, 8x8, tile 0 blank (LZ77)
        zoom_map.lz                   zoom tilemap, 32x32 bytes (LZ77)
        zoom_pal.bin                  zoom palette, 16 x RGB555
    Kernel/Wizardry/TitleScreenGfx/gfx/   obj_sheet.png, zoom.png, preview.png (x2)
    Kernel/Wizardry/TitleScreenGfx/TitleScreenGfx_Data.event
                                      the data above as #incbin labels and sprite layouts
"""
import sys
from pathlib import Path

import numpy as np
from PIL import Image

REPO = Path(__file__).resolve().parent.parent.parent
GFX_DIR = REPO / "Kernel" / "Wizardry" / "TitleScreenGfx"
DMP_DIR = GFX_DIR / "dmp"
PICS_DIR = GFX_DIR / "gfx"
SRC_DIR = GFX_DIR / "src"
sys.path.insert(0, str(REPO / "Tools" / "gallery"))
from png2cg import lz77_compress  # noqa: E402

W, H = 240, 160
TILES_W, TILES_H = 30, 20
MAP_W = 32
MAP_BYTES = MAP_W * TILES_H * 2
BG_MAX_TILES = 640
BG_SPLIT = 384
EMBLEM_MAX_TILES = 192
# The emblem is centred on x=120 (11 + 218/2): the tile boundary the mirror axis sits on.
EMBLEM_X, EMBLEM_Y = 11, 8
EMBLEM_W = 218
EMBLEM_H = round(464 * EMBLEM_W / 1128)
FLIPS = ((0, 0), (1, 0), (0, 1), (1, 1))  # (hflip, vflip), in the order they are tried
MAGENTA = (255, 0, 255)

# Part 2: OBJ logo sprites and the intro zoom image. Crop boxes are (x0, y0, x1, y1) in the source.
LOGO_BOX, LOGO_W, LOGO_XY = (280, 537, 1257, 688), 232, (4, 46)  # 232x36
BANNER_BOX, BANNER_W, BANNER_XY = (326, 697, 1211, 832), 208, (16, 85)  # 208x32
EXCEED_BOX, EXCEED_W, EXCEED_XY = (372, 894, 1163, 987), 118, (61, 112)  # 118x14
SOURCE_ROM = REPO / "fe8.gba"  # vanilla FE8U ROM
ROM_OBJ_PAL_OFF = 0xAADB68  # OBJ banks 0-1 ("Press START" and copyright palettes)
ROM_OBJ_PAL_BYTES = 64
ZOOM_BACKDROP = 0x4290  # vanilla zoom palette entry 0 (sets the backdrop)
OBJ_TILE_ROWS, OBJ_TILE_COLS = 11, 32  # 256 px wide sheet, 88 px high
OBJ_SHEET_W, OBJ_SHEET_H = 256, 88
OBJ_TILES_BYTES = 0x2C00  # 352 tiles; the game allows 0x3000 before its OBJ tiles at 0x06013000
OBJ_LOGO_ROW = 0  # sheet pixel rows: logo at 0 (rows 0-4), banner at 40 (rows 5-8), exceed at 72 (rows 9-10)
OBJ_BANNER_ROW = 40
OBJ_EXCEED_ROW = 72
OBJ_PAL_LOGO = 2  # OBJ palette bank of the logo and exceed text
OBJ_PAL_BANNER = 3


def tile_base(sheet_row):
    """The first OBJ tile of a sheet row (32 tiles per row, the 2D mapping of the sheet)."""
    assert sheet_row % 8 == 0, sheet_row
    return sheet_row // 8 * OBJ_TILE_COLS


LOGO_TILE_BASE = tile_base(OBJ_LOGO_ROW)  # 0x000
BANNER_TILE_BASE = tile_base(OBJ_BANNER_ROW)  # 0x0A0
EXCEED_TILE_BASE = tile_base(OBJ_EXCEED_ROW)  # 0x120
# attr2 base (palette << 12 | tile base) the installer gives the banner and exceed calls
TITLE_BANNER_ATTR2 = OBJ_PAL_BANNER << 12 | BANNER_TILE_BASE  # 0x30A0
TITLE_EXCEED_ATTR2 = OBJ_PAL_LOGO << 12 | EXCEED_TILE_BASE  # 0x2120
ZOOM_SIZE = 256
# Zoom tiles are 8bpp and sit at char base 0x06008000; the map is at 0x0600B000, so 192 tiles.
ZOOM_TILES_MAX = 192
SPRITE_SHAPES = {(64, 32): (1, 3), (32, 16): (1, 2), (32, 8): (1, 1), (32, 32): (0, 2)}
EVENT_PATH = GFX_DIR / "TitleScreenGfx_Data.event"
# (width, height, x, y, tile offset) for each sprite of an element
LOGO_SPRITES = ([(64, 32, 0, 0, 0), (64, 32, 64, 0, 8), (64, 32, 128, 0, 16), (64, 32, 192, 0, 24)]
                # row 4 of the sheet is 4 tile rows (128 tiles) below the base
                + [(32, 8, 32 * k, 32, 128 + 4 * k) for k in range(8)])
BANNER_SPRITES = [(64, 32, 0, 0, 0), (64, 32, 64, 0, 8), (64, 32, 128, 0, 16), (32, 32, 192, 0, 24)]
EXCEED_SPRITES = [(32, 16, 0, 0, 0), (32, 16, 32, 0, 4), (32, 16, 64, 0, 8), (32, 16, 96, 0, 12)]


def to555(r, g, b):
    """8-bit RGB to a GBA RGB555 value."""
    r5, g5, b5 = ((c * 31 + 127) // 255 for c in (r, g, b))
    return r5 | g5 << 5 | b5 << 10


def from555(value):
    """GBA RGB555 value to 8-bit RGB (the 5-bit values are spread over 8 bits)."""
    out = []
    for shift in (0, 5, 10):
        c5 = (value >> shift) & 31
        out.append(c5 << 3 | c5 >> 2)
    return tuple(out)


def snap(rgb):
    """The 8-bit colour as the GBA can show it."""
    return from555(to555(*rgb))


def luminance(rgb):
    r, g, b = rgb
    return 0.299 * r + 0.587 * g + 0.114 * b


def pal_bytes(colours):
    """Up to 16 RGB555 values (u16), padded with 0, as 32 little-endian bytes."""
    assert len(colours) <= 16, len(colours)
    values = list(colours) + [0] * (16 - len(colours))
    return b"".join(v.to_bytes(2, "little") for v in values)


def pack4(block):
    """An 8x8 array of palette indices as 32 bytes of 4bpp (low nibble = left pixel)."""
    lo = block[:, 0::2] & 15
    hi = block[:, 1::2] & 15
    return (lo | (hi << 4)).astype(np.uint8).tobytes()


def unpack4(tile):
    """32 bytes of 4bpp as an 8x8 array of palette indices."""
    packed = np.frombuffer(tile, dtype=np.uint8).reshape(8, 4)
    block = np.empty((8, 8), dtype=np.uint8)
    block[:, 0::2] = packed & 15
    block[:, 1::2] = packed >> 4
    return block


def tile4(indices, tx, ty):
    """The 8x8 tile at tile coordinates (tx, ty) of an index image, as 32 bytes of 4bpp."""
    block = indices[ty * 8:ty * 8 + 8, tx * 8:tx * 8 + 8]
    return pack4(block)


def flip_tile(block, hflip, vflip):
    if hflip:
        block = block[:, ::-1]
    if vflip:
        block = block[::-1, :]
    return block


def dedupe(tiles_list):
    """Unique tiles from a list of 32-byte tiles. The first tile is tile 0.

    Each later tile is matched against the stored tiles in all four flips. Returns
    (unique_tiles, entries): unique_tiles are 32-byte tiles, entries are map words
    (index | hflip << 10 | vflip << 11), one per input tile.
    """
    unique = []
    entries = []
    lookup = {}  # tile bytes as shown -> (index, hflip, vflip) that shows it
    for tile in tiles_list:
        if tile in lookup:
            index, hflip, vflip = lookup[tile]
        else:
            index = len(unique)
            unique.append(tile)
            _add_variants(lookup, unpack4(tile), index)
            hflip, vflip = 0, 0
        entries.append(index | hflip << 10 | vflip << 11)
    return unique, entries


def _add_variants(lookup, block, index):
    """Record every flip of a new stored tile; the first stored tile to show a tile wins."""
    for hflip, vflip in FLIPS:
        shown = pack4(flip_tile(block, hflip, vflip))
        lookup.setdefault(shown, (index, hflip, vflip))


def nearest(rgb_pixels, palette):
    """Index of the nearest palette colour (squared RGB distance) for each pixel."""
    pixels = rgb_pixels.astype(np.int32)
    pal = np.array(palette, dtype=np.int32)
    dist = ((pixels[:, None, :] - pal[None, :, :]) ** 2).sum(axis=2)
    return dist.argmin(axis=1)


def quantize_opaque(rgb_pixels, n):
    """Up to n colours (snapped to 15 bits) for an Nx3 uint8 array, darkest first."""
    count = len(rgb_pixels)
    strip = Image.fromarray(rgb_pixels.reshape(1, count, 3).astype(np.uint8))
    quant = strip.quantize(colors=n, method=Image.Quantize.MEDIANCUT, kmeans=4,
                           dither=Image.Dither.NONE)
    raw = quant.getpalette()[:3 * n]
    colours = []
    for i in range(0, len(raw) - 2, 3):
        rgb = snap(tuple(raw[i:i + 3]))
        if rgb not in colours:
            colours.append(rgb)
    colours.sort(key=luminance)
    return colours


def background_tiles(resized):
    """Steps for BG1: 16 colours, rarest colour made transparent, tiles, map."""
    quant = resized.quantize(colors=16, method=Image.Quantize.MEDIANCUT, kmeans=4,
                             dither=Image.Dither.FLOYDSTEINBERG)
    palette = [snap(tuple(quant.getpalette()[i:i + 3])) for i in range(0, 48, 3)]
    indices = np.array(quant, dtype=np.uint8)
    counts = np.bincount(indices.ravel(), minlength=16)
    rarest = int(np.argmin(counts))
    print(f"background: least used colour index {rarest} ({palette[rarest]}), "
          f"{counts[rarest]} pixels, moved to the nearest other colour")
    indices = _drop_colour(indices, palette, rarest)
    order = [rarest] + sorted((i for i in range(16) if i != rarest),
                              key=lambda i: luminance(palette[i]))
    remap = np.zeros(16, dtype=np.uint8)
    for new, old in enumerate(order):
        remap[old] = new
    indices = remap[indices]
    assert not (indices == 0).any(), "background pixel uses transparent index 0"
    ordered = [palette[i] for i in order]
    return indices, ordered


def _drop_colour(indices, palette, drop):
    """Give every pixel of colour `drop` its nearest other palette colour."""
    flat = indices.ravel()
    mask = flat == drop
    if not mask.any():
        return indices
    others = [i for i in range(len(palette)) if i != drop]
    colour = np.array([palette[drop]], dtype=np.uint8)
    best = nearest(colour, [palette[i] for i in others])[0]
    flat[mask] = others[best]
    return flat.reshape(indices.shape)


def tilemap(entries, cols=TILES_W):
    """Tilemap words (row-major, 32 per row) with the 2 right-hand columns left at 0."""
    words = np.zeros((TILES_H, MAP_W), dtype="<u2")
    words[:, :cols] = np.array(entries, dtype="<u2").reshape(TILES_H, cols)
    return words.tobytes()


def write_bytes(path, data):
    path.write_bytes(data)
    print(f"  wrote {path.relative_to(REPO)} ({len(data)} bytes)")


def write_indexed_png(path, indices, palette):
    """Save an index image as a PNG with its palette; index 0 is shown as magenta."""
    shown = list(palette)
    shown[0] = MAGENTA
    flat = [c for rgb in shown for c in rgb]
    flat += [0] * (768 - len(flat))
    img = Image.fromarray(indices.astype(np.uint8), mode="P")
    img.putpalette(flat)
    img.save(path)
    print(f"  wrote {path.relative_to(REPO)}")


def build_background(src_path):
    """Part A: returns (indices 240x160, palette of 16 RGB, tile entries, unique tiles)."""
    img = Image.open(src_path).convert("RGB").resize((W, H), Image.LANCZOS)
    indices, palette = background_tiles(img)
    tiles = [tile4(indices, tx, ty) for ty in range(TILES_H) for tx in range(TILES_W)]
    unique, entries = dedupe(tiles)
    print(f"background: {len(unique)} unique tiles (max {BG_MAX_TILES})")
    assert len(unique) <= BG_MAX_TILES, len(unique)
    return indices, palette, unique, entries


def write_background(indices, palette, unique, entries):
    head = unique[:min(BG_SPLIT, len(unique))]
    tail = unique[BG_SPLIT:]
    tail_data = b"".join(tail) if tail else bytes(32)
    write_bytes(DMP_DIR / "bg_tiles_a.lz", lz77_compress(b"".join(head)))
    write_bytes(DMP_DIR / "bg_tiles_b.lz", lz77_compress(tail_data))
    map_data = tilemap(entries)
    assert len(map_data) == MAP_BYTES
    write_bytes(DMP_DIR / "bg_map.lz", lz77_compress(map_data))
    write_bytes(DMP_DIR / "bg_pal.bin", pal_bytes([to555(*c) for c in palette]))
    write_indexed_png(PICS_DIR / "background.png", indices, palette)


def emblem_source(src_path):
    """Part B step 1: the emblem as an EMBLEM_W-wide index block (0 = transparent) and palette."""
    img = Image.open(src_path).convert("RGBA")
    img = img.crop((204, 14, 1332, 478))
    img = img.resize((EMBLEM_W, EMBLEM_H), Image.LANCZOS)
    arr = np.array(img)
    # Make the emblem left-right symmetric: the right half is the mirror of the left half.
    # Unmirrored the emblem needs 249 tiles, but only about 127 mirrored, because the tile
    # dedupe matches flipped tiles. The limit is EMBLEM_MAX_TILES (192).
    assert EMBLEM_W % 2 == 0, EMBLEM_W
    half = EMBLEM_W // 2
    arr[:, half:] = arr[:, :half][:, ::-1].copy()
    opaque = arr[:, :, 3] >= 128
    rgb = arr[:, :, :3][opaque]
    palette = quantize_opaque(rgb, 15)
    indices = np.zeros((EMBLEM_H, EMBLEM_W), dtype=np.uint8)
    idx = nearest(rgb, palette) + 1
    indices[opaque] = idx
    full = [(0, 0, 0)] + palette
    return indices, full


def build_emblem(src_path):
    """Part B: returns (indices 240x160, palette of 16 RGB, tile entries, unique tiles)."""
    block, palette = emblem_source(src_path)
    print(f"emblem: {EMBLEM_W}x{EMBLEM_H} at ({EMBLEM_X},{EMBLEM_Y}), "
          f"{len(palette) - 1} colours")
    canvas = np.zeros((H, W), dtype=np.uint8)
    canvas[EMBLEM_Y:EMBLEM_Y + EMBLEM_H, EMBLEM_X:EMBLEM_X + EMBLEM_W] = block
    tiles = [tile4(canvas, tx, ty) for ty in range(TILES_H) for tx in range(TILES_W)]
    unique, entries = dedupe([bytes(32)] + tiles)
    entries = entries[1:]
    unique_count = len(unique)
    print(f"emblem: {unique_count} unique tiles incl. blank (max {EMBLEM_MAX_TILES})")
    assert unique_count <= EMBLEM_MAX_TILES, unique_count
    return canvas, palette, unique, entries


def write_emblem(canvas, palette, unique, entries):
    write_bytes(DMP_DIR / "emblem_tiles.lz", lz77_compress(b"".join(unique)))
    map_data = tilemap(entries)
    assert len(map_data) == MAP_BYTES
    write_bytes(DMP_DIR / "emblem_map.lz", lz77_compress(map_data))
    write_bytes(DMP_DIR / "emblem_pal.bin", pal_bytes([to555(*c) for c in palette]))
    write_indexed_png(PICS_DIR / "emblem.png", canvas, palette)


def write_preview(bg_indices, bg_palette, emb_canvas, emb_palette):
    """The background with the emblem over it (emblem index 0 is see-through), x2."""
    bg = np.array(bg_palette, dtype=np.uint8)[bg_indices]
    emb = np.array(emb_palette, dtype=np.uint8)[emb_canvas]
    visible = emb_canvas != 0
    out = bg.copy()
    out[visible] = emb[visible]
    img = Image.fromarray(out, mode="RGB").resize((W * 2, H * 2), Image.NEAREST)
    img.save(PICS_DIR / "preview_bg.png")
    print(f"  wrote {(PICS_DIR / 'preview_bg.png').relative_to(REPO)}")


def element_source(img, box, width):
    """One crop of the RGBA source, resized to `width`: (rgb array, opaque mask)."""
    x0, y0, x1, y1 = box
    height = round((y1 - y0) * width / (x1 - x0))
    arr = np.array(img.crop(box).resize((width, height), Image.LANCZOS))
    return arr[:, :, :3], arr[:, :, 3] >= 128


def element_indices(rgb, opaque, palette):
    """Palette indices (1-based, 0 = transparent) of an element's opaque pixels."""
    idx = np.zeros(opaque.shape, dtype=np.uint8)
    idx[opaque] = nearest(rgb[opaque], palette) + 1
    return idx


def paste(canvas, block, x, y):
    h, w = block.shape
    canvas[y:y + h, x:x + w] = block


def paint(out, block, x, y, palette):
    """Paint the non-zero pixels of an index block onto an RGB array.

    `palette` is the list of colours for indices 1..n (index 0 is not in it).
    """
    h, w = block.shape
    region = out[y:y + h, x:x + w]
    mask = block != 0
    region[mask] = np.array(palette, dtype=np.uint8)[block[mask] - 1]


def obj_entry(width, height, x, y, tile):
    """One sprite's (attr0, attr1, attr2)."""
    shape, size = SPRITE_SHAPES[(width, height)]
    attr0 = (y & 0xFF) | shape << 14
    attr1 = (x & 0x1FF) | size << 14
    return (attr0, attr1, tile)


def obj_layout(sprites, elem_w, elem_h, name):
    """Sprite layout words (count, then attr0/attr1/attr2 per sprite); asserts the cover."""
    right = max(x + w for w, h, x, y, t in sprites)
    bottom = max(y + h for w, h, x, y, t in sprites)
    assert right >= elem_w and bottom >= elem_h, (name, right, bottom, elem_w, elem_h)
    words = [len(sprites)]
    for w, h, x, y, tile in sprites:
        words.extend(obj_entry(w, h, x, y, tile))
    return words


def tile8_bytes(canvas, tx, ty):
    """The 8x8 tile at tile coordinates (tx, ty) as 64 bytes, one byte per pixel."""
    return canvas[ty * 8:ty * 8 + 8, tx * 8:tx * 8 + 8].astype(np.uint8).tobytes()


def build_obj(src_path):
    """Part 2a: the OBJ sheet, palettes and the element palette indices."""
    img = Image.open(src_path).convert("RGBA")
    logo_rgb, logo_op = element_source(img, LOGO_BOX, LOGO_W)
    ban_rgb, ban_op = element_source(img, BANNER_BOX, BANNER_W)
    exc_rgb, exc_op = element_source(img, EXCEED_BOX, EXCEED_W)
    # bank 2 is shared by the logo and the exceed text, bank 3 is the banner alone
    pal2 = quantize_opaque(np.concatenate([logo_rgb[logo_op], exc_rgb[exc_op]]), 15)
    pal3 = quantize_opaque(ban_rgb[ban_op], 15)
    assert len(pal2) <= 15 and len(pal3) <= 15, (len(pal2), len(pal3))
    logo_idx = element_indices(logo_rgb, logo_op, pal2)
    ban_idx = element_indices(ban_rgb, ban_op, pal3)
    exc_idx = element_indices(exc_rgb, exc_op, pal2)
    print(f"obj: logo {logo_idx.shape[1]}x{logo_idx.shape[0]}, "
          f"banner {ban_idx.shape[1]}x{ban_idx.shape[0]}, "
          f"exceed {exc_idx.shape[1]}x{exc_idx.shape[0]}; "
          f"bank 2 {len(pal2)} colours, bank 3 {len(pal3)} colours")

    sheet = np.zeros((OBJ_SHEET_H, OBJ_SHEET_W), dtype=np.uint8)
    paste(sheet, logo_idx, 0, OBJ_LOGO_ROW)
    paste(sheet, ban_idx, 0, OBJ_BANNER_ROW)
    paste(sheet, exc_idx, 0, OBJ_EXCEED_ROW)
    tiles = [tile4(sheet, c, r) for r in range(OBJ_TILE_ROWS) for c in range(OBJ_TILE_COLS)]
    obj_tiles = b"".join(tiles)
    assert len(obj_tiles) == OBJ_TILES_BYTES, len(obj_tiles)

    with open(SOURCE_ROM, "rb") as rom:
        rom.seek(ROM_OBJ_PAL_OFF)
        banks01 = rom.read(ROM_OBJ_PAL_BYTES)
    assert len(banks01) == ROM_OBJ_PAL_BYTES
    # entry 0 of banks 2 and 3 is 0 (transparent), indices 1..15 are the colours
    obj_pal = (banks01
               + pal_bytes([0] + [to555(*c) for c in pal2])
               + pal_bytes([0] + [to555(*c) for c in pal3]))
    assert len(obj_pal) == 128
    return {
        "sheet": sheet, "tiles": obj_tiles, "pal": obj_pal,
        "pal2": pal2, "pal3": pal3,
        "logo": logo_idx, "banner": ban_idx, "exceed": exc_idx,
    }


def build_zoom(obj):
    """Part 2b: the 256x256 intro zoom image (logo and exceed, bank 2 indices)."""
    canvas = np.zeros((ZOOM_SIZE, ZOOM_SIZE), dtype=np.uint8)
    paste(canvas, obj["logo"], *LOGO_XY)
    paste(canvas, obj["exceed"], *EXCEED_XY)
    unique = [bytes(64)]  # tile 0 is all zeros, forced first
    lookup = {bytes(64): 0}
    entries = []
    for ty in range(ZOOM_SIZE // 8):
        for tx in range(ZOOM_SIZE // 8):
            data = tile8_bytes(canvas, tx, ty)
            if data not in lookup:
                lookup[data] = len(unique)
                unique.append(data)
            entries.append(lookup[data])
    print(f"zoom: {len(unique)} unique 8bpp tiles incl. blank (max {ZOOM_TILES_MAX})")
    # char base 0x06008000, map at 0x0600B000: 192 tiles of 64 bytes fill that gap
    assert len(unique) <= ZOOM_TILES_MAX, len(unique)
    map_data = np.array(entries, dtype=np.uint8).reshape(32, 32).tobytes()
    assert len(map_data) == 1024
    pal = pal_bytes([ZOOM_BACKDROP] + [to555(*c) for c in obj["pal2"]])
    assert len(pal) == 32
    return {"canvas": canvas, "tiles": b"".join(unique), "map": map_data, "pal": pal}


def write_obj(obj):
    write_bytes(DMP_DIR / "obj_tiles.lz", lz77_compress(obj["tiles"]))
    write_bytes(DMP_DIR / "obj_pal.bin", obj["pal"])
    tiles = [obj["tiles"][i:i + 32] for i in range(0, len(obj["tiles"]), 32)]
    print(f"obj: {len(tiles)} tiles ({len(obj['tiles'])} bytes), "
          f"{len(set(tiles))} unique (incl. blank)")


def write_zoom(zoom):
    write_bytes(DMP_DIR / "zoom_tiles.lz", lz77_compress(zoom["tiles"]))
    write_bytes(DMP_DIR / "zoom_map.lz", lz77_compress(zoom["map"]))
    write_bytes(DMP_DIR / "zoom_pal.bin", zoom["pal"])
    print(f"zoom: {len(zoom['tiles']) // 64} tiles ({len(zoom['tiles'])} bytes), "
          f"map {len(zoom['map'])} bytes")


def write_obj_sheet_png(obj):
    """The OBJ sheet as RGB: logo and exceed in bank 2, banner in bank 3, index 0 magenta."""
    sheet = np.zeros((OBJ_SHEET_H, OBJ_SHEET_W, 3), dtype=np.uint8)
    sheet[:] = MAGENTA
    paint(sheet, obj["logo"], 0, 0, obj["pal2"])
    paint(sheet, obj["banner"], 0, OBJ_BANNER_ROW, obj["pal3"])
    paint(sheet, obj["exceed"], 0, OBJ_EXCEED_ROW, obj["pal2"])
    Image.fromarray(sheet).save(PICS_DIR / "obj_sheet.png")
    print(f"  wrote {(PICS_DIR / 'obj_sheet.png').relative_to(REPO)}")


def write_zoom_png(zoom, obj):
    rgb = np.zeros((ZOOM_SIZE, ZOOM_SIZE, 3), dtype=np.uint8)
    rgb[:] = MAGENTA
    paint(rgb, zoom["canvas"], 0, 0, obj["pal2"])
    Image.fromarray(rgb).save(PICS_DIR / "zoom.png")
    print(f"  wrote {(PICS_DIR / 'zoom.png').relative_to(REPO)}")


def write_preview_full(bg_indices, bg_palette, emb_canvas, emb_palette, obj):
    """Background, emblem (index 0 see-through), then logo, banner and exceed; x2."""
    bg = np.array(bg_palette, dtype=np.uint8)[bg_indices]
    emb = np.array(emb_palette, dtype=np.uint8)[emb_canvas]
    visible = emb_canvas != 0
    out = bg.copy()
    out[visible] = emb[visible]
    paint(out, obj["logo"], *LOGO_XY, obj["pal2"])
    paint(out, obj["banner"], *BANNER_XY, obj["pal3"])
    paint(out, obj["exceed"], *EXCEED_XY, obj["pal2"])
    img = Image.fromarray(out).resize((W * 2, H * 2), Image.NEAREST)
    img.save(PICS_DIR / "preview.png")
    print(f"  wrote {(PICS_DIR / 'preview.png').relative_to(REPO)}")


def write_event(obj):
    """TitleScreenGfx_Data.event: positions, every binary as #incbin, the sprite layouts."""
    lines = [
        "// Generated by Tools/titlescreen/title_gfx.py. Do not edit.",
        f"#define TITLE_LOGO_X {LOGO_XY[0]}",
        f"#define TITLE_LOGO_Y {LOGO_XY[1]}",
        f"#define TITLE_BANNER_X {BANNER_XY[0]}",
        f"#define TITLE_BANNER_Y {BANNER_XY[1]}",
        f"#define TITLE_EXCEED_X {EXCEED_XY[0]}",
        f"#define TITLE_EXCEED_Y {EXCEED_XY[1]}",
        f"#define TITLE_BANNER_ATTR2 0x{TITLE_BANNER_ATTR2:X}",
        f"#define TITLE_EXCEED_ATTR2 0x{TITLE_EXCEED_ATTR2:X}",
    ]
    binaries = [
        ("TitleBg_TilesA", "bg_tiles_a.lz"),
        ("TitleBg_TilesB", "bg_tiles_b.lz"),
        ("TitleBg_Map", "bg_map.lz"),
        ("TitleBg_Pal", "bg_pal.bin"),
        ("TitleEmblem_Tiles", "emblem_tiles.lz"),
        ("TitleEmblem_Map", "emblem_map.lz"),
        ("TitleEmblem_Pal", "emblem_pal.bin"),
        ("TitleObj_Tiles", "obj_tiles.lz"),
        ("TitleObj_Pal", "obj_pal.bin"),
        ("TitleZoom_Tiles", "zoom_tiles.lz"),
        ("TitleZoom_Map", "zoom_map.lz"),
        ("TitleZoom_Pal", "zoom_pal.bin"),
    ]
    for label, name in binaries:
        lines += ["ALIGN 4", f"{label}:", f'#incbin "dmp/{name}"']
    layouts = [
        ("TitleLogo_Layout", obj_layout(LOGO_SPRITES, obj["logo"].shape[1],
                                        obj["logo"].shape[0], "logo")),
        ("TitleBanner_Layout", obj_layout(BANNER_SPRITES, obj["banner"].shape[1],
                                          obj["banner"].shape[0], "banner")),
        ("TitleExceed_Layout", obj_layout(EXCEED_SPRITES, obj["exceed"].shape[1],
                                          obj["exceed"].shape[0], "exceed")),
    ]
    for label, words in layouts:
        lines += ["ALIGN 4", f"{label}:", "SHORT " + " ".join(f"0x{w:X}" for w in words)]
    write_bytes(EVENT_PATH, ("\n".join(lines) + "\n").encode())


def main():
    DMP_DIR.mkdir(parents=True, exist_ok=True)
    PICS_DIR.mkdir(parents=True, exist_ok=True)
    bg_indices, bg_palette, bg_unique, bg_entries = build_background(
        SRC_DIR / "title_background.png")
    emb_canvas, emb_palette, emb_unique, emb_entries = build_emblem(SRC_DIR / "title_logo.png")
    write_background(bg_indices, bg_palette, bg_unique, bg_entries)
    write_emblem(emb_canvas, emb_palette, emb_unique, emb_entries)
    write_preview(bg_indices, bg_palette, emb_canvas, emb_palette)
    obj = build_obj(SRC_DIR / "title_logo.png")
    write_obj(obj)
    zoom = build_zoom(obj)
    write_zoom(zoom)
    write_obj_sheet_png(obj)
    write_zoom_png(zoom, obj)
    write_preview_full(bg_indices, bg_palette, emb_canvas, emb_palette, obj)
    write_event(obj)
    print("done")


if __name__ == "__main__":
    main()
