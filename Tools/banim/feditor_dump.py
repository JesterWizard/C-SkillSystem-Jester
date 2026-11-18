#!/usr/bin/env python3
"""Convert an FEditor Advance battle-animation dump into Event Assembler files.

Input dump folder (DUMP_DIR):
  <Name>.bin             Java-serialized object. After the file names come five arrays: "[I" (the
                         12-byte name, ignored), then four "[B": modes (48 bytes), OAM right, OAM left,
                         palettes (4 x 16 RGB555).
  <Name> Frame Data.dmp  The script: little-endian u32 words. A word with top byte 0x86 starts a frame
                         command of 3 words [0x86xxxxxx, SHEET, OAM_OFFSET]; other words are 1-word commands.
  <Name> Sheet N.png     264x64 mode-P sheets, N from 1. x 0..255 is the sheet, x 256..263 a swatch.

Usage:
  python3 Tools/banim/feditor_dump.py [--lz77] DUMP_DIR OUT_DIR PREFIX ABBR
  python3 Tools/banim/feditor_dump.py [--lz77] --rom ROM ID SRC_DIR OUT_DIR PREFIX ABBR

--rom reskins a vanilla animation: the modes, the OAMs and the script come from animation ID of ROM (a clean fe8.gba),
the sheets and the palettes from SRC_DIR: "Sheet N.png" (264x64 mode-P, as above; one per sheet the script uses, in
the order it first uses them) and "Palette.bin" (4 x 16 RGB555, 128 bytes).

The sheets are Huffman-coded for Kernel/Wizardry/BanimCodec (about a fifth smaller than LZ77; a sheet takes up to 1.3
frames to decode, so a sheet change can drop a frame). --lz77 writes them in the BIOS's LZ77 instead
(PREFIX_sheet_<i>.lz): bigger, decoded at BIOS speed. See Documentation/Features/CustomBattleAnimations.md,
"Compression".

Writes PREFIX_trees.bin and PREFIX_sheet_<i>.huff (the sheets, Huffman-coded with one shared pair of trees for
Kernel/Wizardry/BanimCodec; with --lz77, PREFIX_sheet_<i>.lz instead), PREFIX_modes.bin, PREFIX_oam_r.lz, PREFIX_pal.lz (LZ77, optimal parse), PREFIX_oam_l.lz
only when the left OAM isn't the right one mirrored (else AutoGenLeftOAM makes it), and PREFIX.event (the labels, the
LZ77 script with the sheet pointers' bytes filled in by EA, and the PREFIX_BANIM_ENTRY table macro).
"""
import glob
import os
import re
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', 'gallery'))

from PIL import Image  # noqa: E402

from png2cg import lz77_decompress  # noqa: E402
from banimpack import HUFF_TAG, huff_pack, huff_unpack, lz77_compress_optimal, lz77_parse, lz77_stream  # noqa: E402

SHEET_W, SHEET_H, PNG_W = 256, 64, 264
TILES_PER_ROW = 32
PTR_BASE = 0x08000000
PTR_MACRO = '#define BANIM_PTR_BYTE(l, k) "BYTE (((l)|0x08000000)>>(8*(k)))&0xFF"'


def fail(msg):
    sys.exit('feditor_dump: error: ' + msg)


def _read_desc(data, p):
    """Parse a TC_CLASSDESC for an array class. Returns (name, position after the descriptor)."""
    nlen = struct.unpack_from('>H', data, p)[0]
    name = data[p + 2:p + 2 + nlen]
    p += 2 + nlen + 8 + 1
    nfields = struct.unpack_from('>H', data, p)[0]
    p += 2
    if nfields != 0 or data[p:p + 2] != b'\x78\x70':
        raise ValueError('unexpected class descriptor')
    if name not in (b'[I', b'[B'):
        raise ValueError('not an array class')
    return name.decode(), p + 2


def parse_java_arrays(data):
    """Find the "[I" and "[B" arrays in file order. Stops after the fourth "[B" array.

    A back-reference (0x71) takes the class of the most recent class descriptor (the arrays after the
    first "[B" descriptor are all "[B"). Element counts and the 0x78 0x70 layout are checked."""
    arrays, b_count, last_kind, pos = [], 0, None, 0
    while pos + 2 < len(data) and b_count < 4:
        if data[pos] != 0x75 or data[pos + 1] not in (0x72, 0x71):
            pos += 1
            continue
        try:
            if data[pos + 1] == 0x72:
                kind, p = _read_desc(data, pos + 2)
            else:
                if last_kind is None:
                    raise ValueError('back-reference before any descriptor')
                kind, p = last_kind, pos + 6
            count = struct.unpack_from('>i', data, p)[0]
            p += 4
            esize = 4 if kind == '[I' else 1
            if count < 0 or p + count * esize > len(data):
                raise ValueError('bad element count')
        except (ValueError, struct.error):
            pos += 1
            continue
        if kind == '[I':
            elems = [struct.unpack_from('>I', data, p + 4 * k)[0] for k in range(count)]
        else:
            elems = bytes(data[p:p + count])
            b_count += 1
        arrays.append((kind, elems))
        last_kind = kind
        pos = p + count * esize
    return arrays


def read_dump(dump_dir):
    bins = sorted(glob.glob(os.path.join(dump_dir, '*.bin')))
    if len(bins) != 1:
        fail('expected exactly one *.bin in %s, found %d' % (dump_dir, len(bins)))
    name = os.path.splitext(os.path.basename(bins[0]))[0]
    with open(bins[0], 'rb') as f:
        arrays = parse_java_arrays(f.read())
    bs = [a for k, a in arrays if k == '[B']
    if len(bs) != 4:
        fail('expected 4 "[B" arrays, found %d' % len(bs))
    modes, oam_r, oam_l, pal = bs
    if len(modes) != 48:
        fail('modes is %d bytes, expected 48' % len(modes))
    if len(pal) != 128:
        fail('palettes is %d bytes, expected 128' % len(pal))
    if len(oam_r) != len(oam_l):
        fail('OAM right and left differ in length')
    if len(oam_r) % 12:
        fail('OAM length %d is not a multiple of 12' % len(oam_r))

    with open(os.path.join(dump_dir, name + ' Frame Data.dmp'), 'rb') as f:
        raw = f.read()
    if len(raw) % 4:
        fail('frame data length %d is not a multiple of 4' % len(raw))
    words = list(struct.unpack('<%dI' % (len(raw) // 4), raw))

    sheets = []
    for path in glob.glob(os.path.join(dump_dir, name + ' Sheet *.png')):
        m = re.search(r'Sheet (\d+)\.png$', path)
        if m:
            sheets.append((int(m.group(1)), path))
    sheets.sort()
    if [n for n, _ in sheets] != list(range(1, len(sheets) + 1)):
        fail('sheet PNGs are not numbered 1..N: %s' % [n for n, _ in sheets])
    if not sheets:
        fail('no "%s Sheet N.png" files' % name)
    return name, modes, oam_r, oam_l, pal, words, [p for _, p in sheets]


def walk_script(words, nsheets):
    """Return (indices of SHEET words, frame count). Frames are [0x86xxxxxx, SHEET, OAM_OFFSET]."""
    sheet_pos, frames, i = [], 0, 0
    while i < len(words):
        if words[i] >> 24 == 0x86:
            if i + 2 >= len(words):
                fail('frame at word %d runs past the end of the script' % i)
            sheet = words[i + 1]
            if sheet >= nsheets:
                fail('frame at word %d uses sheet %d, but there are %d sheets' % (i, sheet, nsheets))
            sheet_pos.append(i + 1)
            frames += 1
            i += 3
        else:
            i += 1
    return sheet_pos, frames


def load_sheet(path):
    im = Image.open(path)
    if im.mode != 'P' or im.size != (PNG_W, 64):
        fail('%s: need a 264x64 mode-P image, got %s %s' % (path, im.mode, im.size))
    raw = im.tobytes()
    rows = [raw[y * PNG_W:(y + 1) * PNG_W] for y in range(64)]
    used = set()
    for row in rows:
        used.update(row[:SHEET_W])
    if len(used) > 16 or max(used) > 15:
        fail('%s: more than 16 colours in x 0..255' % path)
    return rows


def sheet_4bpp(rows):
    out = bytearray(SHEET_W * SHEET_H // 2)
    for ty in range(SHEET_H // 8):
        for tx in range(TILES_PER_ROW):
            base = (ty * TILES_PER_ROW + tx) * 32
            for r in range(8):
                row = rows[ty * 8 + r]
                for c in range(4):
                    x = tx * 8 + c * 2
                    out[base + r * 4 + c] = (row[x] & 0xF) | ((row[x + 1] & 0xF) << 4)
    return bytes(out)


def ea_bytes(bs):
    return 'BYTE ' + ' '.join('0x%02X' % b for b in bs)


def abbr_bytes(abbr):
    try:
        raw = abbr.encode('ascii')
    except UnicodeEncodeError:
        fail('ABBR must be ASCII')
    if len(raw) > 11:
        fail('ABBR "%s" is %d chars; the maximum is 11' % (abbr, len(raw)))
    return raw + bytes(12 - len(raw))


# OBJ widths by shape (attr0 bits 14-15) and size (attr1 bits 14-15)
OBJ_WIDTH = {(0, 0): 8, (0, 1): 16, (0, 2): 32, (0, 3): 64, (1, 0): 16, (1, 1): 32, (1, 2): 32, (1, 3): 64,
             (2, 0): 8, (2, 1): 8, (2, 2): 16, (2, 3): 32}


def mirror_oam(oam_r):
    """The left OAM as AutoGenLeftOAM (Contents/External/AutoGenLeftOAM) makes it from the right one: every entry but
    the end codes (attr0 1) and the special ones (attr1 0xFFFF) gets the h-flip bit and x = -x - width."""
    out = bytearray(oam_r)
    for k in range(0, len(out), 12):
        a0, a1 = struct.unpack_from('<HH', out, k)
        if out[k] == 1 or a1 == 0xFFFF:
            continue
        x = struct.unpack_from('<h', out, k + 6)[0]
        w = OBJ_WIDTH.get((a0 >> 14, a1 >> 14))
        if w is None:
            return None
        struct.pack_into('<H', out, k + 2, a1 | 0x1000)
        struct.pack_into('<h', out, k + 6, -x - w)
    return bytes(out)


def script_tokens(words, sheet_pos):
    """The script as tokens: bytes, and ('P', sheet, k) for byte k of a sheet's pointer."""
    pos = set(sheet_pos)
    toks = []
    for i, w in enumerate(words):
        if i in pos:
            toks += [('P', w, k) for k in range(4)]
        else:
            toks += list(struct.pack('<I', w))
    return toks


def read_rom(rom_path, anim_id, src_dir):
    """A vanilla animation's modes, OAMs and script (its sheet pointers made indices) with SRC_DIR's sheets and
    palettes, as read_dump returns them."""
    with open(rom_path, 'rb') as f:
        rom = f.read()
    ent = 0xC00008 + 0x20 * (anim_id - 1)
    modes_p, script_p, oam_r_p, oam_l_p, _pal_p = struct.unpack_from('<5I', rom, ent + 12)
    modes = rom[modes_p - 0x08000000:][:48]
    raw = lz77_decompress(rom, script_p - 0x08000000)
    words = list(struct.unpack('<%dI' % (len(raw) // 4), raw))
    oam_r = lz77_decompress(rom, oam_r_p - 0x08000000)
    oam_l = lz77_decompress(rom, oam_l_p - 0x08000000)
    ptrs, i = [], 0
    while i < len(words):
        if words[i] >> 24 == 0x86:
            if words[i + 1] not in ptrs:
                ptrs.append(words[i + 1])
            words[i + 1] = ptrs.index(words[i + 1])
            i += 3
        else:
            i += 1
    paths = [os.path.join(src_dir, 'Sheet %d.png' % (n + 1)) for n in range(len(ptrs))]
    for p in paths:
        if not os.path.exists(p):
            fail('%s: missing (the script uses %d sheets)' % (p, len(ptrs)))
    with open(os.path.join(src_dir, 'Palette.bin'), 'rb') as f:
        pal = f.read()
    if len(pal) != 128:
        fail('Palette.bin is %d bytes, expected 128' % len(pal))
    return 'banim %#x of %s' % (anim_id, os.path.basename(rom_path)), modes, oam_r, oam_l, pal, words, paths


def main():
    args = [a for a in sys.argv[1:] if a != '--lz77']
    lz77_sheets = len(args) != len(sys.argv) - 1
    if args and args[0] == '--rom' and len(args) == 7:
        rom_path, anim_id, src_dir, out_dir, prefix, abbr = args[1:]
        name, modes, oam_r, oam_l, pal, words, sheet_paths = read_rom(rom_path, int(anim_id, 0), src_dir)
    elif len(args) == 4:
        dump_dir, out_dir, prefix, abbr = args
        name, modes, oam_r, oam_l, pal, words, sheet_paths = read_dump(dump_dir)
    else:
        sys.exit('usage: feditor_dump.py [--lz77] DUMP_DIR OUT_DIR PREFIX ABBR\n'
                 '       feditor_dump.py [--lz77] --rom ROM ID SRC_DIR OUT_DIR PREFIX ABBR')
    convert(name, modes, oam_r, oam_l, pal, words, [load_sheet(p) for p in sheet_paths], out_dir, prefix, abbr,
            lz77_sheets)


def convert(name, modes, oam_r, oam_l, pal, words, rows_by_sheet, out_dir, prefix, abbr, lz77_sheets=False,
            tool='feditor_dump.py'):
    """Write the EA files for an animation. rows_by_sheet holds each sheet as 64 rows of 256+ palette indices."""
    nsheets = len(rows_by_sheet)
    sheet_pos, frames = walk_script(words, nsheets)
    oam_r, oam_l, pal = bytes(oam_r), bytes(oam_l), bytes(pal)

    # Build every output as bytes first, so the self-checks run before anything is written.
    files = {}
    sheet_raw = [sheet_4bpp(rows) for rows in rows_by_sheet]
    if lz77_sheets:
        for i, raw in enumerate(sheet_raw, 1):
            files['%s_sheet_%d.lz' % (prefix, i)] = lz77_compress_optimal(raw)
    else:
        trees, streams = huff_pack(sheet_raw)
        files['%s_trees.bin' % prefix] = trees
        for i, st in enumerate(streams, 1):
            files['%s_sheet_%d.huff' % (prefix, i)] = st
    files['%s_modes.bin' % prefix] = bytes(modes) + bytes(48)
    files['%s_oam_r.lz' % prefix] = lz77_compress_optimal(oam_r)
    # The left OAM: AutoGenLeftOAM makes it from the right one when both pointers are the same.
    left_auto = mirror_oam(oam_r) == oam_l
    if not left_auto:
        files['%s_oam_l.lz' % prefix] = lz77_compress_optimal(oam_l)
    files['%s_pal.lz' % prefix] = lz77_compress_optimal(pal)

    # Self-checks: every part decodes back to its input.
    for i, raw in enumerate(sheet_raw, 1):
        if lz77_sheets:
            ok = lz77_decompress(files['%s_sheet_%d.lz' % (prefix, i)]) == raw
        else:
            ok = huff_unpack(trees, streams[i - 1]) == raw
        if not ok:
            fail('self-check: sheet %d does not decode back to its pixels' % i)
    checks = {'%s_oam_r.lz' % prefix: oam_r, '%s_pal.lz' % prefix: pal}
    if not left_auto:
        checks['%s_oam_l.lz' % prefix] = oam_l
    for fn, raw in checks.items():
        if lz77_decompress(files[fn]) != raw:
            fail('self-check: %s does not decompress back to its input' % fn)

    # The script: LZ77 over tokens, the sheet pointers' bytes as literals that EA fills in.
    script_len = len(words) * 4
    stream = lz77_stream(lz77_parse(script_tokens(words, sheet_pos)), script_len)
    as_index = bytes(t[1] >> (8 * t[2]) & 0xFF if isinstance(t, tuple) else t for t in stream)
    if lz77_decompress(as_index) != struct.pack('<%dI' % len(words), *words):
        fail('self-check: the script stream does not reproduce the dump')
    n_ptr = len(sheet_pos)

    ea = []
    ea.append('// Generated by Tools/banim/%s from %s. Do not edit.' % (tool, name))
    ea.append('#ifndef BANIM_PTR_BYTE')
    ea.append(PTR_MACRO)
    ea.append('#endif')
    ea.append('')
    entry = abbr_bytes(abbr)
    ea.append('#define %s_BANIM_ENTRY "%s; POIN %s_Modes %s_Script %s_OamR %s_OamL %s_Pal"' % (
        prefix, ea_bytes(entry), prefix, prefix, prefix, prefix, prefix))
    ea.append('')
    if not lz77_sheets:
        ea.append('ALIGN 4')
        ea.append('%s_Trees: // struct BanimHuffTrees (include/kernel/banim-codec.h)' % prefix)
        ea.append('#incbin "%s_trees.bin"' % prefix)
    for i, raw in enumerate(sheet_raw, 1):
        ea.append('ALIGN 4')
        ea.append('%s_Sheet%d:' % (prefix, i))
        if lz77_sheets:
            ea.append('#incbin "%s_sheet_%d.lz"' % (prefix, i))
        else:
            ea.append('WORD 0x%08X; POIN %s_Trees' % (len(raw) << 8 | HUFF_TAG, prefix))
            ea.append('#incbin "%s_sheet_%d.huff"' % (prefix, i))
    ea.append('ALIGN 4')
    ea.append('%s_Modes:' % prefix)
    ea.append('#incbin "%s_modes.bin"' % prefix)
    ea.append('ALIGN 4')
    ea.append('%s_OamR:' % prefix)
    if left_auto:
        ea.append('%s_OamL: // the same pointer: AutoGenLeftOAM mirrors the right OAM' % prefix)
    ea.append('#incbin "%s_oam_r.lz"' % prefix)
    if not left_auto:
        ea.append('ALIGN 4')
        ea.append('%s_OamL:' % prefix)
        ea.append('#incbin "%s_oam_l.lz"' % prefix)
    ea.append('ALIGN 4')
    ea.append('%s_Pal:' % prefix)
    ea.append('#incbin "%s_pal.lz"' % prefix)
    ea.append('ALIGN 4')
    ea.append('%s_Script:' % prefix)
    line = []
    for t in stream:
        if isinstance(t, tuple):
            if line:
                ea.append(ea_bytes(line))
                line = []
            ea.append('BANIM_PTR_BYTE(%s_Sheet%d, %d)' % (prefix, t[1] + 1, t[2]))
        else:
            line.append(t)
            if len(line) == 16:
                ea.append(ea_bytes(line))
                line = []
    if line:
        ea.append(ea_bytes(line))
    ea_text = '\n'.join(ea) + '\n'
    files['%s.event' % prefix] = ea_text.encode('ascii')

    os.makedirs(out_dir, exist_ok=True)
    for old in glob.glob(os.path.join(glob.escape(out_dir), '%s_*' % prefix)):
        if os.path.basename(old) not in files and os.path.isfile(old):
            os.remove(old)
    total = 0
    for fn, data in files.items():
        with open(os.path.join(out_dir, fn), 'wb') as f:
            f.write(data)
        total += len(data)
    rom = sum(len(d) for fn, d in files.items() if not fn.endswith('.event'))
    rom += (0 if lz77_sheets else 8 * nsheets) + len(stream)
    print('%s: %d sheets, %d frames, script %d bytes (%d sheet pointers) packed to %d, left OAM %s; about %d bytes'
          ' in the ROM' % (name, nsheets, frames, script_len, n_ptr, len(stream),
                            'made by AutoGenLeftOAM' if left_auto else 'stored', rom))


if __name__ == '__main__':
    main()
