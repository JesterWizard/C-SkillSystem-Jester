#!/usr/bin/env python3
"""Portrait tool: export, check, compare, animate and recolour FE8 portrait sheets.

A portrait sheet is the 128x112 PNG that PortraitFormatter (Tools/EventAssembler/Tools) turns into game data. Its parts,
in pixels:

    face      96x80 at (0, 0); its top corners (x 0-15 and 80-95, y 0-47) are not used
    minimug   32x32 at (96, 16)
    eyes      two 32x16 blink frames at (96, 48) and (96, 64): half shut and shut, drawn at the eye coordinates
    mouths    six 32x16 frames at (0, 80), (32, 80), (64, 80), (0, 96), (32, 96), (64, 96): talking, then smiling
    rest      32x16 at (96, 80): the mouth at rest, a copy of the face at the mouth coordinates
    (96, 96) is not used. setMugEntry's coordinates are 8-pixel tiles from the face's top-left corner.

The pixel at (0, 0) is the transparent colour. The layout is PortraitFormatter.hs's (EAFormattingSuite sources).

Commands:
  export ROM ID OUT.png [--table ADDR]   a portrait from a ROM, as an indexed sheet with its own palette
  check SHEET.png [--base BASE.png]       the format, and against a base: off-palette colours, stray pixels,
                                          and eye or mouth edits whose animation frames were not edited too
  diff BASE.png EDIT.png OUT.png          base, edit and the changed pixels side by side (4x)
  animate SHEET.png OUT.gif --mouth X,Y --eyes X,Y [--rom ROM --id ID]
                                          talking and blinking, as the game draws them (coords from setMugEntry)
  recolor SHEET.png OUT.png OLD=NEW ...   swaps colours (RRGGBB hex), keeping every pixel
  sync-frames BASE.png EDIT.png OUT.png --mouth X,Y --eyes X,Y
                                          copies the face's edits into the blink, mouth and resting-mouth frames,
                                          wherever a frame still shows the base's face there
  format SHEET.png OUT.bin                the data PortraitFormatter writes (mug, frames, palette, minimug)

Run with `python3 -I` from anywhere; it needs Pillow.
"""
import argparse
import struct
import sys

from PIL import Image

W, H = 128, 112
TILE = 8
VANILLA_TABLE = 0x08ACBC4
ENTRY = 0x1C

# (src tile x, src tile y, dst tile x, dst tile y, w, h): the sheet -> the 256x32 mug image (PortraitFormatter.hs)
MUG_MAP = [
    (2, 0, 0, 0, 8, 4), (2, 4, 8, 0, 8, 4), (2, 8, 16, 0, 4, 2), (6, 8, 16, 2, 4, 2),
    (0, 6, 20, 0, 2, 4), (10, 6, 22, 0, 2, 4), (12, 6, 24, 0, 4, 2), (12, 8, 24, 2, 4, 2),
    (12, 10, 28, 0, 4, 2), (12, 12, 28, 2, 4, 2),
]
# the six mouth frames -> the 384x8 frame strip, one 4x1-tile row at a time
FRAME_MAP = [
    (0, 10, 0), (0, 11, 4), (4, 10, 8), (4, 11, 12), (8, 10, 16), (8, 11, 20),
    (0, 12, 24), (0, 13, 28), (4, 12, 32), (4, 13, 36), (8, 12, 40), (8, 13, 44),
]
MINIMUG = (12, 2, 4, 4)

EYE_FRAMES = [(96, 48), (96, 64)]
MOUTH_FRAMES = [(0, 80), (32, 80), (64, 80), (0, 96), (32, 96), (64, 96)]
MOUTH_REST = (96, 80)


def used_mask():
    """The sheet pixels the formatter reads."""
    m = [[False] * W for _ in range(H)]

    def mark(tx, ty, w, h):
        for y in range(ty * TILE, (ty + h) * TILE):
            for x in range(tx * TILE, (tx + w) * TILE):
                m[y][x] = True
    for sx, sy, _, _, w, h in MUG_MAP:
        mark(sx, sy, w, h)
    for sx, sy, _ in FRAME_MAP:
        mark(sx, sy, 4, 1)
    mark(*MINIMUG)
    return m


# ---- GBA data ----

def lz77_decompress(data, off=0):
    if data[off] != 0x10:
        raise ValueError("not LZ77 data at 0x%X" % off)
    size = data[off + 1] | (data[off + 2] << 8) | (data[off + 3] << 16)
    out, i = bytearray(), off + 4
    while len(out) < size:
        flags = data[i]
        i += 1
        for b in range(8):
            if len(out) >= size:
                break
            if flags & (0x80 >> b):
                n = (data[i] >> 4) + 3
                disp = (((data[i] & 0xF) << 8) | data[i + 1]) + 1
                i += 2
                for _ in range(n):
                    out.append(out[-disp])
            else:
                out.append(data[i])
                i += 1
    return bytes(out[:size])


def lz77_compress(data):
    """A plain greedy GBA LZ77 (type 0x10) compressor."""
    out = bytearray([0x10, len(data) & 0xFF, (len(data) >> 8) & 0xFF, (len(data) >> 16) & 0xFF])
    i = 0
    while i < len(data):
        flag_pos = len(out)
        out.append(0)
        flags = 0
        for b in range(8):
            if i >= len(data):
                break
            best_len, best_disp = 0, 0
            for disp in range(2, min(i, 0x1000) + 1):  # disp 1 trips some VRAM decompressors
                n = 0
                while n < 18 and i + n < len(data) and data[i + n] == data[i - disp + n]:
                    n += 1
                if n > best_len:
                    best_len, best_disp = n, disp
                    if n == 18:
                        break
            if best_len >= 3:
                flags |= 0x80 >> b
                out.append(((best_len - 3) << 4) | ((best_disp - 1) >> 8))
                out.append((best_disp - 1) & 0xFF)
                i += best_len
            else:
                out.append(data[i])
                i += 1
        out[flag_pos] = flags
    while len(out) & 3:
        out.append(0)
    return bytes(out)


def tiles_to_pixels(raw, wt, ht):
    """4bpp tiles, row-major, wt x ht tiles -> rows of colour indices."""
    px = [[0] * (wt * TILE) for _ in range(ht * TILE)]
    for t in range(wt * ht):
        tx, ty = t % wt, t // wt
        for r in range(TILE):
            for c in range(TILE // 2):
                b = raw[t * 32 + r * 4 + c]
                px[ty * TILE + r][tx * TILE + c * 2] = b & 0xF
                px[ty * TILE + r][tx * TILE + c * 2 + 1] = b >> 4
    return px


def pixels_to_tiles(px, wt, ht):
    out = bytearray()
    for t in range(wt * ht):
        tx, ty = t % wt, t // wt
        for r in range(TILE):
            for c in range(TILE // 2):
                lo = px[ty * TILE + r][tx * TILE + c * 2]
                hi = px[ty * TILE + r][tx * TILE + c * 2 + 1]
                out.append(lo | (hi << 4))
    return bytes(out)


def gba_to_rgb(c):
    return tuple(((c >> s) & 0x1F) * 255 // 31 for s in (0, 5, 10))


def rgb_to_gba(rgb):
    r, g, b = rgb[:3]
    return (r >> 3) | ((g >> 3) << 5) | ((b >> 3) << 10)


# ---- Sheets ----

def blank_sheet():
    return [[0] * W for _ in range(H)]


def copy_tiles(src, sx, sy, dst, dx, dy, w, h):
    for y in range(h * TILE):
        for x in range(w * TILE):
            dst[dy * TILE + y][dx * TILE + x] = src[sy * TILE + y][sx * TILE + x]


def sheet_from_rom(rom, fid, table):
    """The sheet of portrait `fid`: (rows of indices, 16 RGB colours, (mouthX, mouthY, eyeX, eyeY))."""
    e = (table & 0x1FFFFFF) + fid * ENTRY
    img, chibi, pal, mouth, card = struct.unpack_from("<5I", rom, e)
    coords = tuple(rom[e + 0x14:e + 0x18])
    if img == 0:
        raise ValueError("portrait 0x%X has no image" % fid)
    img &= 0x1FFFFFF
    if rom[img] == 0x10:
        mug = lz77_decompress(rom, img)
    else:
        mug = rom[img + 4:img + 4 + 0x1000]
    mug_px = tiles_to_pixels(mug, 32, 4)
    mouth &= 0x1FFFFFF
    frames = lz77_decompress(rom, mouth) if rom[mouth] == 0x10 else rom[mouth:mouth + 0x600]
    frames_px = tiles_to_pixels(frames, 48, 1)
    chibi_px = tiles_to_pixels(lz77_decompress(rom, chibi & 0x1FFFFFF), 4, 4) if chibi else None
    colours = [gba_to_rgb(v) for v in struct.unpack_from("<16H", rom, pal & 0x1FFFFFF)]

    sheet = blank_sheet()
    for sx, sy, dx, dy, w, h in MUG_MAP:
        copy_tiles(mug_px, dx, dy, sheet, sx, sy, w, h)
    for sx, sy, dx in FRAME_MAP:
        copy_tiles(frames_px, dx, 0, sheet, sx, sy, 4, 1)
    if chibi_px:
        copy_tiles(chibi_px, 0, 0, sheet, MINIMUG[0], MINIMUG[1], 4, 4)
    return sheet, colours, coords


def save_sheet(path, sheet, colours):
    im = Image.new("P", (W, H))
    flat = []
    for c in colours:
        flat += list(c)
    im.putpalette(flat + [0] * (768 - len(flat)))
    im.putdata([v for row in sheet for v in row])
    im.save(path)


def load_sheet(path):
    """(rows of RGB tuples, Image) from any PNG."""
    im = Image.open(path).convert("RGB")
    if im.size != (W, H):
        raise ValueError("%s is %dx%d, not %dx%d" % (path, im.size[0], im.size[1], W, H))
    px = im.load()
    return [[px[x, y] for x in range(W)] for y in range(H)], im


def palettize(rgb_rows):
    """As PortraitFormatter: index 0 is the colour of pixel (0, 0); other colours in first-seen order."""
    trans = rgb_rows[0][0]
    colours = [trans]
    mask = used_mask()
    for y in range(H):
        for x in range(W):
            c = rgb_rows[y][x]
            if mask[y][x] and c not in colours:
                colours.append(c)
    index = {c: i for i, c in enumerate(colours)}
    return [[index.get(rgb_rows[y][x], 0) for x in range(W)] for y in range(H)], colours


def format_sheet(rgb_rows):
    """The bytes PortraitFormatter writes: mug (with header), frames, palette, minimug (LZ77)."""
    idx, colours = palettize(rgb_rows)
    if len(colours) > 16:
        raise ValueError("%d colours; a portrait has 16 at most" % len(colours))
    mug = [[0] * 256 for _ in range(32)]
    for sx, sy, dx, dy, w, h in MUG_MAP:
        copy_tiles(idx, sx, sy, mug, dx, dy, w, h)
    frames = [[0] * 384 for _ in range(8)]
    for sx, sy, dx in FRAME_MAP:
        copy_tiles(idx, sx, sy, frames, dx, 0, 4, 1)
    chibi = [[0] * 32 for _ in range(32)]
    copy_tiles(idx, MINIMUG[0], MINIMUG[1], chibi, 0, 0, 4, 4)
    pal = b"".join(struct.pack("<H", rgb_to_gba(c)) for c in colours + [(0, 0, 0)] * (16 - len(colours)))
    return (bytes([0x00, 0x04, 0x10, 0x00]) + pixels_to_tiles(mug, 32, 4) + pixels_to_tiles(frames, 48, 1) + pal +
            lz77_compress(pixels_to_tiles(chibi, 4, 4)))


# ---- Checks ----

def region_of(x, y):
    if y < 80 and x < 96:
        return "face"
    for i, (fx, fy) in enumerate(EYE_FRAMES):
        if fx <= x < fx + 32 and fy <= y < fy + 16:
            return "eyes%d" % i
    for i, (fx, fy) in enumerate(MOUTH_FRAMES):
        if fx <= x < fx + 32 and fy <= y < fy + 16:
            return "mouth%d" % i
    if MOUTH_REST[0] <= x and MOUTH_REST[1] <= y < MOUTH_REST[1] + 16:
        return "rest"
    if 96 <= x and 16 <= y < 48:
        return "minimug"
    return "unused"


def best_match(rows, fx, fy, w, h):
    """Where the w x h block at (fx, fy) best matches the face: (x, y, differing pixels)."""
    best = None
    for oy in range(0, 80 - h + 1):
        for ox in range(0, 96 - w + 1):
            d = 0
            for y in range(h):
                ra, rb = rows[oy + y], rows[fy + y]
                for x in range(w):
                    if ra[ox + x] != rb[fx + x]:
                        d += 1
                if best and d >= best[2]:
                    break
            if best is None or d < best[2]:
                best = (ox, oy, d)
    return best


def lum(c):
    return (c[0] * 299 + c[1] * 587 + c[2] * 114) // 1000


def stray_pixels(rows, trans, area, gap=48):
    """Specks: opaque pixels in `area` unlike all 8 neighbours and at least `gap` brighter or darker than each.

    A shaded line (a mouth, a lash) has close tones beside it and is not a speck; a dot of hair colour on skin is."""
    out = []
    for (x, y) in area:
        c = rows[y][x]
        if c == trans or not (0 < x < W - 1 and 0 < y < H - 1):
            continue
        ns = [rows[y + dy][x + dx] for dy in (-1, 0, 1) for dx in (-1, 0, 1) if dx or dy]
        if all(n != c and abs(lum(n) - lum(c)) >= gap for n in ns):
            out.append((x, y))
    return sorted(out, key=lambda p: (p[1], p[0]))


def cmd_check(args):
    rows, _ = load_sheet(args.sheet)
    trans = rows[0][0]
    mask = used_mask()
    problems, warnings, notes = [], [], []
    colours = {rows[y][x] for y in range(H) for x in range(W) if mask[y][x]} | {trans}
    if len(colours) > 16:
        problems.append("%d colours: a portrait has 16 at most (the transparent one included)" % len(colours))
    unused = [(x, y) for y in range(H) for x in range(W) if not mask[y][x] and rows[y][x] != trans]
    if unused:
        notes.append("%d pixels in unused areas are ignored (first at %s)" % (len(unused), unused[0]))
    if args.base:
        base, _ = load_sheet(args.base)
        base_colours = {base[y][x] for y in range(H) for x in range(W) if mask[y][x]} | {base[0][0]}
        changed = {(x, y) for y in range(H) for x in range(W) if mask[y][x] and rows[y][x] != base[y][x]}
        new = sorted({rows[y][x] for (x, y) in changed} - base_colours)
        if new:
            # fine for a recolour, usually an accident for an expression
            warnings.append("colours not in the base palette: %s" % ", ".join("#%02X%02X%02X" % c for c in new))
        regions = {}
        for (x, y) in changed:
            regions.setdefault(region_of(x, y), []).append((x, y))
        for r in sorted(regions):
            xs = [p[0] for p in regions[r]]
            ys = [p[1] for p in regions[r]]
            notes.append("%-8s %4d pixels changed, in x %d-%d, y %d-%d" %
                         (r, len(regions[r]), min(xs), max(xs), min(ys), max(ys)))
        # Eye and mouth edits on the face need matching frames, or blinking and talking snap back to the base
        for kind, (fx, fy) in (("eyes", EYE_FRAMES[0]), ("mouth", MOUTH_REST)):
            ox, oy, _ = best_match(base, fx, fy, 32, 16)
            face_hit = [p for p in regions.get("face", []) if ox <= p[0] < ox + 32 and oy <= p[1] < oy + 16]
            frames_hit = [r for r in regions if r.startswith(kind)]
            if face_hit and not frames_hit:
                problems.append("the face's %s area (x %d-%d, y %d-%d) changed but no %s frame did: the %s will "
                                "snap back to the base's" % (kind, ox, ox + 31, oy, oy + 15, kind,
                                                            "blink" if kind == "eyes" else "talking"))
        # The resting mouth must stay a copy of the face where the mouth is drawn
        ox, oy, _ = best_match(base, MOUTH_REST[0], MOUTH_REST[1], 32, 16)
        def rest_off(r):
            return sum(1 for y in range(16) for x in range(32)
                       if r[oy + y][ox + x] != r[MOUTH_REST[1] + y][MOUTH_REST[0] + x])
        # vanilla sheets differ by a few pixels here already: only more than the base's is a problem
        if rest_off(rows) > rest_off(base):
            problems.append("the resting mouth at (96, 80) differs from the face at (%d, %d) in %d pixels (the base: "
                            "%d): copy the face's 32x16 there" % (ox, oy, rest_off(rows), rest_off(base)))
        # Specks among the edited pixels: shaded pixel art has a few (an eye highlight); a scatter of them is the
        # usual tell of a noisy or AI-made edit
        specks = stray_pixels(rows, trans, changed)
        notes.append("specks among the edited pixels: %d of %d" % (len(specks), len(changed)))
        if len(specks) > max(3, len(changed) // 8):
            problems.append("the edit is speckled: %d lone pixels unlike their neighbours, e.g. %s" %
                            (len(specks), " ".join("(%d,%d)" % p for p in specks[:8])))
    print("%s: %d colours" % (args.sheet, len(colours)))
    for n in notes:
        print("  " + n)
    for w in warnings:
        print("  WARNING: " + w)
    for p in problems:
        print("  PROBLEM: " + p)
    if not problems:
        print("  OK")
    return 1 if problems else 0


def cmd_export(args):
    rom = open(args.rom, "rb").read()
    table = int(args.table, 0) if args.table else VANILLA_TABLE
    sheet, colours, coords = sheet_from_rom(rom, int(args.id, 0), table)
    save_sheet(args.out, sheet, colours)
    print("%s: portrait 0x%X; setMugEntry(ID, LABEL, %d, %d, %d, %d)" % ((args.out, int(args.id, 0)) + coords))
    return 0


def cmd_format(args):
    rows, _ = load_sheet(args.sheet)
    data = format_sheet(rows)
    open(args.out, "wb").write(data)
    print("%s: %d bytes (mug +0, frames +0x1004, palette +0x1604, minimug +0x1624)" % (args.out, len(data)))
    return 0


def cmd_diff(args):
    a, ia = load_sheet(args.base)
    b, ib = load_sheet(args.edit)
    grey = ia.convert("L").convert("RGB")
    hl = grey.copy()
    px = hl.load()
    n = 0
    for y in range(H):
        for x in range(W):
            if a[y][x] != b[y][x]:
                px[x, y] = (255, 0, 255)
                n += 1
    out = Image.new("RGB", (W * 3 + 8, H), (40, 40, 40))
    out.paste(ia, (0, 0))
    out.paste(ib, (W + 4, 0))
    out.paste(hl, (W * 2 + 8, 0))
    out = out.resize((out.width * 4, out.height * 4), Image.NEAREST)
    out.save(args.out)
    print("%s: %d pixels differ" % (args.out, n))
    return 0


def cmd_animate(args):
    rows, im = load_sheet(args.sheet)
    trans = rows[0][0]
    mx, my = [int(v) for v in args.mouth.split(",")]
    ex, ey = [int(v) for v in args.eyes.split(",")]
    face = im.crop((0, 0, 96, 80))

    def frame(mouth, eyes):
        f = face.copy()
        if eyes is not None:
            fx, fy = EYE_FRAMES[eyes]
            f.paste(im.crop((fx, fy, fx + 32, fy + 16)), (ex * 8, ey * 8))
        if mouth is not None:
            fx, fy = MOUTH_REST if mouth == "rest" else MOUTH_FRAMES[mouth]
            f.paste(im.crop((fx, fy, fx + 32, fy + 16)), (mx * 8, my * 8))
        bg = Image.new("RGB", f.size, trans)
        bg.paste(f)
        return bg.resize((96 * 3, 80 * 3), Image.NEAREST)
    # talk (mouth frames cycle), then blink (half, shut, half), then the smile mouths
    seq = [("rest", None, 400)]
    for m in (0, 1, 2, 1, 0, 1, 2, 1, 0):
        seq.append((m, None, 90))
    seq += [(None, 0, 60), (None, 1, 90), (None, 0, 60), (None, None, 300)]
    for m in (3, 4, 5, 4, 3):
        seq.append((m, None, 90))
    seq.append((None, None, 300))
    frames = [frame(m, e) for m, e, _ in seq]
    frames[0].save(args.out, save_all=True, append_images=frames[1:], duration=[d for _, _, d in seq], loop=0)
    print("%s: %d frames (mouth at tile %d,%d, eyes at tile %d,%d)" % (args.out, len(frames), mx, my, ex, ey))
    return 0


def cmd_sync_frames(args):
    base, _ = load_sheet(args.base)
    edit, im = load_sheet(args.edit)
    mx, my = [int(v) * 8 for v in args.mouth.split(",")]
    ex, ey = [int(v) * 8 for v in args.eyes.split(",")]
    out = [r[:] for r in edit]
    n = 0
    blocks = [(f, ex, ey) for f in EYE_FRAMES] + [(f, mx, my) for f in MOUTH_FRAMES + [MOUTH_REST]]
    for (fx, fy), ox, oy in blocks:
        for y in range(16):
            for x in range(32):
                fc, bc, ec = base[fy + y][fx + x], base[oy + y][ox + x], edit[oy + y][ox + x]
                # the frame shows the base face here, and the face was edited: carry the edit over
                if ec != bc and fc == bc and edit[fy + y][fx + x] == fc:
                    out[fy + y][fx + x] = ec
                    n += 1
    res = Image.new("RGB", (W, H))
    res.putdata([c for r in out for c in r])
    if Image.open(args.edit).mode == "P":
        res = res.quantize(palette=Image.open(args.edit), dither=Image.Dither.NONE)
    res.save(args.out)
    print("%s: %d frame pixels updated" % (args.out, n))
    return 0


def cmd_recolor(args):
    im = Image.open(args.sheet)
    rows, rgb = load_sheet(args.sheet)
    swaps = {}
    for s in args.swaps:
        old, new = s.split("=")
        swaps[tuple(int(old[i:i + 2], 16) for i in (0, 2, 4))] = tuple(int(new[i:i + 2], 16) for i in (0, 2, 4))
    if im.mode == "P":
        pal = im.getpalette()[:48]
        cols = [tuple(pal[i:i + 3]) for i in range(0, 48, 3)]
        missing = [k for k in swaps if k not in cols]
        cols = [swaps.get(c, c) for c in cols]
        out = im.copy()
        out.putpalette([v for c in cols for v in c] + [0] * (768 - 48))
    else:
        missing = [k for k in swaps if not any(k in r for r in rows)]
        out = Image.new("RGB", (W, H))
        out.putdata([swaps.get(c, c) for r in rows for c in r])
    for k in missing:
        print("warning: #%02X%02X%02X is not in the sheet" % k)
    out.save(args.out)
    print("%s: %d colours swapped" % (args.out, len(swaps) - len(missing)))
    return 0


def main():
    p = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = p.add_subparsers(dest="cmd", required=True)
    s = sub.add_parser("export")
    s.add_argument("rom")
    s.add_argument("id")
    s.add_argument("out")
    s.add_argument("--table", help="the portrait table's address (default: vanilla FE8U 0x8ACBC4)")
    s = sub.add_parser("check")
    s.add_argument("sheet")
    s.add_argument("--base")
    s = sub.add_parser("diff")
    s.add_argument("base")
    s.add_argument("edit")
    s.add_argument("out")
    s = sub.add_parser("animate")
    s.add_argument("sheet")
    s.add_argument("out")
    s.add_argument("--mouth", required=True, help="X,Y tiles, setMugEntry's mouth coordinates")
    s.add_argument("--eyes", required=True, help="X,Y tiles, setMugEntry's eye coordinates")
    s = sub.add_parser("recolor")
    s.add_argument("sheet")
    s.add_argument("out")
    s.add_argument("swaps", nargs="+", help="OLD=NEW, RRGGBB hex")
    s = sub.add_parser("sync-frames")
    s.add_argument("base")
    s.add_argument("edit")
    s.add_argument("out")
    s.add_argument("--mouth", required=True)
    s.add_argument("--eyes", required=True)
    s = sub.add_parser("format")
    s.add_argument("sheet")
    s.add_argument("out")
    args = p.parse_args()
    return {"export": cmd_export, "check": cmd_check, "diff": cmd_diff, "animate": cmd_animate,
            "recolor": cmd_recolor, "format": cmd_format, "sync-frames": cmd_sync_frames}[args.cmd](args)


if __name__ == "__main__":
    sys.exit(main())
