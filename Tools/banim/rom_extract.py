#!/usr/bin/env python3
"""Extract a vanilla battle animation from a ROM as the Source files that feditor_dump.py --rom reads.

Usage:
  python3 Tools/banim/rom_extract.py [--origin X,Y] --rom ROM ID OUT_DIR [--frames PREVIEW_DIR]

ID is the animation's ID in the ROM's banim_data (vanilla: 0x01-0xC9). The entry is at 0xC00008 + 0x20 * (ID - 1)
(struct BattleAnim, Tools/FE-CLib-Mokha/include/banim_data.h): the abbr, then the pointers to the modes, the script, the
right OAM, the left OAM and the palettes, all LZ77 except the modes.

Writes to OUT_DIR, the files feditor_dump.py --rom reads for its sheets and palette (the modes, OAMs and script come from
the ROM itself, so they are not written here):
  Sheet N.png   The sheets the script uses, N from 1 in the order the script first uses them (the order
                feditor_dump.py reads them in). 264x64 mode-P: x 0..255 is the sheet, one palette index (0..15) per
                pixel, in 8x8 tiles of 4bpp, 32 tiles to a row, as the sheet is in VRAM; x 256..263 is a swatch
                with row y showing entry y of the four palettes (entries 0..63). Only x 0..255 is read back.
  Palette.bin   The four 16-colour palettes, RGB555, 128 bytes, as the ROM has them.

With --frames PREVIEW_DIR, also draws every distinct frame of the script (a frame command is [0x86 word, sheet pointer,
OAM offset]; distinct means distinct sheet and OAM offset) for preview. Each is the right OAM's sprites from the OAM
offset to its end code, in palette 0, on a 240x160 canvas: the OAM's x and y are added to --origin (default 148,88, the
right side's position as the brief gives it; not checked against the battle engine, so pass --origin to move it), with
index 0 transparent. Sprites use 2D OBJ tile mapping (a sprite's tile rows are 32 tiles apart, as the sheet is laid
out; 1D mapping draws the vanilla frames broken). H- and V-flip are applied per OBJ; the palette is the OBJ's attr2 bank
(of the four palettes); affine OBJs (attr0 bit 8) are not drawn and are counted. Writes:
  PREVIEW_DIR/frame_NN.png  one frame, NN its number in the list below
  PREVIEW_DIR/frames.png    a contact sheet, each frame labelled with its number, its frame ID (bits 16-23 of the 0x86
                            word, the animation's own frame number), its script word index (the first command that
                            uses it) and the modes whose script section holds it (the modes table, ekrbattle.h's
                            banim_mode_index: NA normal attack, CRIT critical attack, RNG ranged, DODGE, STAND, MISS...)
The modes table and the list of frames (every script word index, the modes, the notes) are printed to stdout.

Round trip: feditor_dump.py --rom ROM ID SRC_DIR OUT_DIR PREFIX ABBR on the files written here gives sheets that decode
to the ROM's sheets, and a palette equal to the ROM's, byte for byte.
"""
import argparse
import os
import re
import struct
import sys
import textwrap

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', 'gallery'))

from PIL import Image, ImageDraw  # noqa: E402

from png2cg import lz77_decompress  # noqa: E402
from banimpack import HUFF_TAG, huff_unpack  # noqa: E402

BANIM_TABLE = 0xC00008
ENTRY_SIZE = 0x20
ROM_BASE = 0x08000000
SHEET_BYTES = 256 * 64 // 2
SHEET_W, SHEET_H, PNG_W = 256, 64, 264
TILES_PER_ROW = 32
PAL_BYTES = 128
SCREEN_W, SCREEN_H = 240, 160
DEFAULT_ORIGIN = (148, 88)
BACKDROP = (48, 48, 64)
# (shape, size) -> (width, height) in pixels, from attr0 bits 14-15 and attr1 bits 14-15
OBJ_SIZE = {
    (0, 0): (8, 8), (0, 1): (16, 16), (0, 2): (32, 32), (0, 3): (64, 64),
    (1, 0): (16, 8), (1, 1): (32, 8), (1, 2): (32, 16), (1, 3): (64, 32),
    (2, 0): (8, 16), (2, 1): (8, 32), (2, 2): (16, 32), (2, 3): (32, 64),
}


# banim_mode_index (Tools/FE-CLib-Mokha/include/ekrbattle.h): the modes table's 12 entries, in this order
MODE_NAMES = ['NA', 'NAp', 'CRIT', 'CRITp', 'RNG', 'RNGC', 'DODGEc', 'DODGEr', 'STAND', 'STAND2', 'RSTAND', 'MISS']
MODE_LONG = ['normal attack', 'normal attack (priority L)', 'critical attack', 'critical (priority L)',
             'ranged attack', 'ranged critical', 'close dodge', 'ranged dodge', 'standing', 'standing 2',
             'ranged standing', 'missed attack']


def modes_at(modes, byte_off):
    """The modes whose script section holds byte_off: those that start at the largest start not after it."""
    starts = [s for s in modes if s <= byte_off]
    if not starts:
        return []
    best = max(starts)
    return [i for i, s in enumerate(modes) if s == best]


def fail(msg):
    sys.exit('rom_extract: error: ' + msg)


def rgb555_to_rgb(v):
    return tuple((c << 3) | (c >> 2) for c in (v & 31, (v >> 5) & 31, (v >> 10) & 31))


def palettes_rgb(pal):
    """The 64 colours of the four palettes, as 8-bit RGB."""
    out = []
    for k in range(0, PAL_BYTES, 2):
        out.append(rgb555_to_rgb(struct.unpack_from('<H', pal, k)[0]))
    return out


def read_entry(rom, anim_id):
    if not 1 <= anim_id <= 0xFF:
        fail('ID %#x is not an animation ID' % anim_id)
    ent = BANIM_TABLE + ENTRY_SIZE * (anim_id - 1)
    if ent + ENTRY_SIZE > len(rom):
        fail('ID %#x: its entry is past the end of the ROM' % anim_id)
    ptrs = struct.unpack_from('<5I', rom, ent + 12)
    for p in ptrs:
        if not ROM_BASE <= p < ROM_BASE + len(rom):
            fail('ID %#x has no animation (entry at %#x, pointer %#x)' % (anim_id, ent, p))
    return ptrs  # modes, script, oam right, oam left, palettes


def read_sheet(rom, p):
    """A sheet's bytes: LZ77 (the vanilla ones), or the Huffman coding of Kernel/Wizardry/BanimCodec (tag 0xD1: the
    header is (size << 8) | 0xD1 and the trees' pointer, then the stream; feditor_dump.py writes it so)."""
    hdr, trees_p = struct.unpack_from('<II', rom, p - ROM_BASE)
    if hdr & 0xFF != HUFF_TAG:
        return lz77_decompress(rom, p - ROM_BASE)
    size = hdr >> 8
    if size != SHEET_BYTES:
        fail('Huffman sheet at %#x declares %d bytes, expected %d' % (p, size, SHEET_BYTES))
    out = huff_unpack(rom[trees_p - ROM_BASE:], rom[p - ROM_BASE + 8:])
    if len(out) != size:
        fail('Huffman sheet at %#x decodes to %d bytes, expected %d' % (p, len(out), size))
    return out


def read_script(rom, script_p):
    """The script's words, and the sheet pointers in the order the script first uses them."""
    raw = lz77_decompress(rom, script_p - ROM_BASE)
    if len(raw) % 4:
        fail('script length %d is not a multiple of 4' % len(raw))
    words = list(struct.unpack('<%dI' % (len(raw) // 4), raw))
    sheet_ptrs, cmds, i = [], [], 0
    while i < len(words):
        if words[i] >> 24 == 0x86:
            if i + 2 >= len(words):
                fail('frame command at word %d runs past the end of the script' % i)
            if words[i + 1] not in sheet_ptrs:
                sheet_ptrs.append(words[i + 1])
            cmds.append({'pos': i, 'word': words[i], 'sheet': sheet_ptrs.index(words[i + 1]),
                         'oam': words[i + 2]})
            i += 3
        else:
            i += 1
    return words, sheet_ptrs, cmds


def sheet_pixels(raw):
    """A 4bpp sheet (8192 bytes, tiles in order) as rows of 256 palette indices."""
    rows = [bytearray(SHEET_W) for _ in range(SHEET_H)]
    for y in range(SHEET_H):
        ty, r = divmod(y, 8)
        for x in range(SHEET_W):
            tx, c = divmod(x, 8)
            b = raw[((ty * TILES_PER_ROW + tx) * 32) + r * 4 + c // 2]
            rows[y][x] = (b & 0xF) if c % 2 == 0 else (b >> 4)
    return rows


def sheet_png(raw, rgb64):
    rows = sheet_pixels(raw)
    data = bytearray()
    for y in range(SHEET_H):
        data += rows[y]
        data += bytes([y] * 8)  # swatch: row y is palette entry y
    im = Image.frombytes('P', (PNG_W, SHEET_H), bytes(data))
    flat = bytearray()
    for r, g, b in rgb64:
        flat += bytes((r, g, b))
    im.putpalette(bytes(flat))
    return im


def tile_pixel(raw, tile, x, y):
    b = raw[tile * 32 + y * 4 + x // 2]
    return (b & 0xF) if x % 2 == 0 else (b >> 4)


def render_frame(raw, oam, off, rgb64, origin):
    """The right OAM's sprites from OFF to the end code, drawn as the GBA would (first entry on top).

    Tiles use 2D OBJ mapping: a sprite's tile rows are 32 tiles apart (the sheet's own row length), which is the
    layout the sheets are drawn in (1D mapping drew the vanilla frames as broken pieces). H- and V-flip are
    applied per OBJ. Affine OBJs (attr0 bit 8: rotation or scaling from the OAM matrices) are not drawn: they are
    counted in the stats. An OBJ's palette is its attr2 bank (bits 12-15) of the four palettes."""
    entries = []
    k = off
    while k + 12 <= len(oam) and oam[k] != 1:
        a0, a1, a2 = struct.unpack_from('<HHH', oam, k)
        x, y = struct.unpack_from('<hh', oam, k + 6)
        entries.append((a0, a1, a2, x, y))
        k += 12
    canvas = Image.new('RGB', (SCREEN_W, SCREEN_H), BACKDROP)
    px = canvas.load()
    stats = {'affine': 0, 'outside': 0, 'banks': set(), 'objs': len(entries)}
    for a0, a1, a2, x, y in reversed(entries):
        if (a0 >> 8) & 1:
            stats['affine'] += 1
            continue
        size = OBJ_SIZE.get((a0 >> 14, a1 >> 14))
        if size is None:
            stats['outside'] += 1
            continue
        w, h = size
        hflip, vflip = (a1 >> 12) & 1, (a1 >> 13) & 1
        base = a2 & 0x3FF
        bank = a2 >> 12
        stats['banks'].add(bank)
        pal = rgb64[(bank % 4) * 16:(bank % 4) * 16 + 16]
        ox, oy = origin[0] + x, origin[1] + y
        for j in range(h):
            sj = h - 1 - j if vflip else j
            for i in range(w):
                si = w - 1 - i if hflip else i
                tile = base + (sj // 8) * TILES_PER_ROW + si // 8
                if (tile + 1) * 32 > len(raw):
                    stats['outside'] += 1
                    continue
                idx = tile_pixel(raw, tile, si % 8, sj % 8)
                cx, cy = ox + i, oy + j
                if idx and 0 <= cx < SCREEN_W and 0 <= cy < SCREEN_H:
                    px[cx, cy] = pal[idx]
    return canvas, stats


def contact_sheet(frames, cols=6):
    """frames: list of (label lines, image). Labels above each frame."""
    label_h = 11 * max(len(label) for label, _ in frames) + 4
    cw, ch = SCREEN_W + 4, SCREEN_H + label_h + 4
    rows = (len(frames) + cols - 1) // cols
    sheet = Image.new('RGB', (cols * cw, rows * ch), (16, 16, 16))
    draw = ImageDraw.Draw(sheet)
    for n, (label, im) in enumerate(frames):
        cx, cy = (n % cols) * cw + 2, (n // cols) * ch + 2
        for k, line in enumerate(label):
            draw.text((cx + 2, cy + 1 + 11 * k), line, fill=(255, 255, 255))
        sheet.paste(im, (cx, cy + label_h))
    return sheet


def main():
    ap = argparse.ArgumentParser(description='Extract a vanilla battle animation as feditor_dump.py --rom sources.')
    ap.add_argument('--rom', required=True)
    ap.add_argument('--origin', default='%d,%d' % DEFAULT_ORIGIN, help='X,Y of the right side for --frames')
    ap.add_argument('--frames', metavar='PREVIEW_DIR')
    ap.add_argument('id')
    ap.add_argument('out_dir')
    args = ap.parse_args()

    anim_id = int(args.id, 0)
    origin = tuple(int(v, 0) for v in args.origin.split(','))
    if len(origin) != 2:
        fail('--origin is X,Y')
    with open(args.rom, 'rb') as f:
        rom = f.read()

    modes_p, script_p, oam_r_p, oam_l_p, pal_p = read_entry(rom, anim_id)
    words, sheet_ptrs, cmds = read_script(rom, script_p)
    if not sheet_ptrs:
        fail('ID %#x: the script has no frames' % anim_id)

    sheets = []
    for p in sheet_ptrs:
        raw = read_sheet(rom, p)
        if len(raw) != SHEET_BYTES:
            fail('sheet at %#x is %d bytes, expected %d (256x64 4bpp)' % (p, len(raw), SHEET_BYTES))
        sheets.append(raw)
    pal = lz77_decompress(rom, pal_p - ROM_BASE)
    if len(pal) != PAL_BYTES:
        fail('palettes are %d bytes, expected %d' % (len(pal), PAL_BYTES))
    rgb64 = palettes_rgb(pal)

    os.makedirs(args.out_dir, exist_ok=True)
    for old in os.listdir(args.out_dir):
        if re.fullmatch(r'Sheet \d+\.png', old) and int(re.search(r'\d+', old).group()) > len(sheets):
            os.remove(os.path.join(args.out_dir, old))
    for n, raw in enumerate(sheets, 1):
        sheet_png(raw, rgb64).save(os.path.join(args.out_dir, 'Sheet %d.png' % n))
    with open(os.path.join(args.out_dir, 'Palette.bin'), 'wb') as f:
        f.write(pal)
    print('ID %#x: %d sheets, %d frame commands, script %d words -> %s' % (
        anim_id, len(sheets), len(cmds), len(words), args.out_dir))

    if args.frames:
        oam_r = lz77_decompress(rom, oam_r_p - ROM_BASE)
        modes = struct.unpack_from('<12I', rom, modes_p - ROM_BASE)
        print('modes (script byte offset each mode starts at; script word = offset / 4):')
        for i, (name, start) in enumerate(zip(MODE_NAMES, modes)):
            print('  %2d %-22s %-9s offset %#06x, word %d' % (i, MODE_LONG[i], name, start, start // 4))
        frames = {}
        order = []
        for c in cmds:
            key = (c['sheet'], c['oam'])
            if key not in frames:
                frames[key] = {'n': len(order), 'ids': [], 'pos': []}
                order.append(key)
            f = frames[key]
            fid = (c['word'] >> 16) & 0xFF
            if fid not in f['ids']:
                f['ids'].append(fid)
            f['pos'].append(c['pos'])
        os.makedirs(args.frames, exist_ok=True)
        tiles = []
        for key in order:
            sheet_i, off = key
            im, st = render_frame(sheets[sheet_i], oam_r, off, rgb64, origin)
            f = frames[key]
            im.save(os.path.join(args.frames, 'frame_%02d.png' % f['n']))
            ids = ' '.join('%#x' % i for i in f['ids'])
            modes_of = sorted({m for p in f['pos'] for m in modes_at(modes, 4 * p)})
            mtxt = ' '.join(MODE_NAMES[m] for m in modes_of) or '-'
            label = ['#%d id %s w%d' % (f['n'], ids, f['pos'][0])] + textwrap.wrap('modes ' + mtxt, 34)
            tiles.append((label, im))
            notes = []
            if st['affine']:
                notes.append('%d affine OBJ(s) not drawn' % st['affine'])
            if st['outside']:
                notes.append('%d OBJ tile(s) outside the sheet' % st['outside'])
            if st['banks'] - {0}:
                notes.append('palette banks %s' % sorted(st['banks']))
            print('frame %2d: sheet %d, OAM offset %#x, frame ID %s, modes %s, script words %s%s' % (
                f['n'], sheet_i + 1, off, ids, mtxt, ' '.join(str(p) for p in f['pos']),
                ('; ' + ', '.join(notes)) if notes else ''))
        contact_sheet(tiles).save(os.path.join(args.frames, 'frames.png'))
        print('contact sheet: %s' % os.path.join(args.frames, 'frames.png'))


if __name__ == '__main__':
    main()
