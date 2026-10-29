#!/usr/bin/env python3
"""Format a half-body portrait sheet for FE8U (portrait_formats patch).

A Python stand-in for the C Skill System's HalfbodyFormatter.exe (Kirb's
closed-source Windows tool).  The output works with setMugEntry_Halfbody.

Input: a 160x160 PNG, at most 16 colours, laid out like HalfbodyTemplate.png
(the top-left pixel's colour is transparent):

  x 16..112, y   0..48   head (the top corners are not drawn)
  x  0..128, y  48..128  shoulders, chest and body
  x128..160, y  64..96   32x32 chibi (map menu / status screen mini portrait)
  x128..160, y  96..112  eyes half closed   } kept for the stat screen layout;
  x128..160, y 112..128  eyes closed        } half-bodies don't blink
  x  0..128, y 128..160  seven 32x16 mouth blocks, left to right, top row
                         first: blocks 1-6 are the talking frames, block 7
                         (x 64..96, y 144..160) is the closed mouth the stat
                         screen shows at the mouth position

Output (raw, not compressed; offsets from the start):
  +0x0000  header 00 04 20 00, then 0x2000 bytes: a 32x8-tile OBJ sheet
  +0x2004  0x600 bytes: six 4x2-tile mouth frames
  +0x2604  0x40 bytes: palette (16 colours, then 16 unused)
  +0x2644  LZ77-compressed 4x4-tile chibi

Usage: HalfbodyFormatter.py <sheet.png> [-o out.dmp] [--to-stdout]
"""
from __future__ import annotations

import struct
import sys
from pathlib import Path

from PIL import Image

SHEET_W, SHEET_H = 160, 160

# Half-body sprite pieces: (x, y, width, height, first tile), x relative to
# the portrait's centre (sheet x 64).  Same as the patch's sHalfBodyRight.
OBJECTS = [
    (-32, 0, 64, 32, 0x00),
    (-32, 32, 64, 32, 0x08),
    (-48, 0, 16, 16, 0x9A),
    (32, 0, 16, 16, 0xDA),
    (-32, 64, 32, 16, 0x10),
    (0, 64, 32, 16, 0x50),
    (-48, 48, 16, 32, 0x14),
    (32, 48, 16, 32, 0x16),
    (48, 48, 16, 32, 0x96),
    (-64, 48, 16, 32, 0x98),
    (-48, 16, 16, 32, 0x92),
    (32, 16, 16, 32, 0x94),
    (-64, 80, 16, 32, 0x80),
    (-48, 80, 32, 32, 0x82),
    (-16, 80, 32, 32, 0x86),
    (16, 80, 32, 32, 0x8A),
    (48, 80, 16, 32, 0x8E),
    (-64, 112, 16, 16, 0x90),
    (-48, 112, 32, 16, 0x5C),
    (-16, 112, 32, 16, 0x9C),
    (16, 112, 32, 16, 0xDC),
    (48, 112, 16, 16, 0xD0),
]

# Spare tiles in the sheet: (sheet x, sheet y, first tile), 32x16 each.
EXTRA_BLOCKS = [
    (128, 96, 0x18),   # eyes half closed (vanilla mug: tile 0x18)
    (128, 112, 0x58),  # eyes closed (vanilla mug: tile 0x58)
    (64, 144, 0x1C),   # closed mouth (vanilla mug: tile 0x1C)
]

MOUTH_FRAMES = [(0, 128), (32, 128), (64, 128), (96, 128), (0, 144), (32, 144)]
CHIBI = (128, 64)


def rgb_to_gba(rgb):
    return (rgb[0] >> 3) | ((rgb[1] >> 3) << 5) | ((rgb[2] >> 3) << 10)


def load_sheet(path):
    im = Image.open(path)
    if im.size != (SHEET_W, SHEET_H):
        raise RuntimeError("%s is %dx%d; a half-body sheet is %dx%d"
                           % (path, im.size[0], im.size[1], SHEET_W, SHEET_H))
    if im.mode != "P":
        im = im.convert("RGB").quantize(colors=16, method=Image.Quantize.MEDIANCUT)
    pal = im.getpalette() or []
    colors = [tuple(pal[i * 3:i * 3 + 3]) for i in range(len(pal) // 3)]
    px = im.load()
    grid = [[px[x, y] for x in range(SHEET_W)] for y in range(SHEET_H)]

    used = sorted({i for row in grid for i in row})
    trans = grid[0][0]
    order = [trans] + [i for i in used if i != trans]
    if len(order) > 16:
        raise RuntimeError("%s uses %d colours; a half-body has 16 (one is transparent)"
                           % (path, len(order)))
    remap = {old: new for new, old in enumerate(order)}
    grid = [[remap[i] for i in row] for row in grid]
    palette = [colors[i] if i < len(colors) else (0, 0, 0) for i in order]
    palette += [(0, 0, 0)] * (16 - len(palette))
    return grid, palette


def tile_bytes(grid, x0, y0):
    out = bytearray()
    for y in range(8):
        row = grid[y0 + y]
        for x in range(0, 8, 2):
            out.append((row[x0 + x] & 0xF) | ((row[x0 + x + 1] & 0xF) << 4))
    return bytes(out)


def put_block(sheet, grid, sx, sy, w, h, tile):
    for j in range(h // 8):
        for i in range(w // 8):
            t = tile + i + 32 * j
            sheet[t * 32:(t + 1) * 32] = tile_bytes(grid, sx + 8 * i, sy + 8 * j)


def lz77_compress(data):
    """GBA LZ77 (type 0x10), VRAM-safe (no copies from 1 byte back)."""
    out = bytearray(struct.pack("<I", 0x10 | (len(data) << 8)))
    i = 0
    n = len(data)
    while i < n:
        flag_pos = len(out)
        out.append(0)
        flags = 0
        for bit in range(8):
            if i >= n:
                break
            best_len, best_disp = 0, 0
            for disp in range(2, min(i, 0x1000) + 1):
                length = 0
                while (length < 18 and i + length < n
                       and data[i + length] == data[i - disp + length]):
                    length += 1
                if length > best_len:
                    best_len, best_disp = length, disp
                    if length == 18:
                        break
            if best_len >= 3:
                flags |= 0x80 >> bit
                v = ((best_len - 3) << 12) | (best_disp - 1)
                out += bytes([v >> 8, v & 0xFF])
                i += best_len
            else:
                out.append(data[i])
                i += 1
        out[flag_pos] = flags
    while len(out) % 4:
        out.append(0)
    return bytes(out)


def format_halfbody(path):
    grid, palette = load_sheet(path)

    sheet = bytearray(0x2000)
    for x, y, w, h, tile in OBJECTS:
        put_block(sheet, grid, 64 + x, y, w, h, tile)
    for sx, sy, tile in EXTRA_BLOCKS:
        put_block(sheet, grid, sx, sy, 32, 16, tile)

    frames = bytearray()
    for sx, sy in MOUTH_FRAMES:
        for j in range(2):
            for i in range(4):
                frames += tile_bytes(grid, sx + 8 * i, sy + 8 * j)

    pal = b"".join(struct.pack("<H", rgb_to_gba(c)) for c in palette)
    pal += b"\x00" * 0x20

    chibi = bytearray()
    for j in range(4):
        for i in range(4):
            chibi += tile_bytes(grid, CHIBI[0] + 8 * i, CHIBI[1] + 8 * j)

    out = bytes([0x00, 0x04, 0x20, 0x00]) + sheet + frames + pal + lz77_compress(bytes(chibi))
    assert len(out) >= 0x2644 and out[0x2644] == 0x10
    return out


def main(argv):
    args = argv[1:]
    to_stdout = "--to-stdout" in args
    args = [a for a in args if a != "--to-stdout"]
    out_path = None
    if "-o" in args:
        i = args.index("-o")
        if i + 1 >= len(args):
            sys.stderr.write("No output file specified.\n")
            return 1
        out_path = args[i + 1]
        del args[i:i + 2]
    if len(args) != 1:
        sys.stderr.write("usage: HalfbodyFormatter.py <sheet.png> [-o out.dmp] [--to-stdout]\n")
        return 1
    try:
        data = format_halfbody(args[0])
    except (RuntimeError, OSError) as e:
        sys.stderr.write(str(e) + "\n")
        return 1
    if to_stdout:
        sys.stdout.buffer.write(data)
    else:
        Path(out_path or (str(Path(args[0]).with_suffix("")) + "_halfbody.dmp")).write_bytes(data)
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
