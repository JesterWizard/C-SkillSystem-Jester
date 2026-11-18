#!/usr/bin/env python3
"""Convert a FEBuilder-style battle-animation script (a .txt and 248x160 frame PNGs) into Event Assembler files.

Input folder (SRC_DIR):
  <Name>.txt   The script, as FEBuilderGBA exports it:
                 /// - Mode N ...   starts mode N (1, 3, 5-12; modes 2 and 4 are made from 1 and 3)
                 Cxx                command 0x850000xx (xx in hex)
                 D p- file.png      show file.png for D frames
                 ~~~                ends the mode
  *.png        The frames: 248x160 mode-P images, at most 16 colours, index 0 transparent, every frame with the
               same palette. The animation's origin is pixel (148, 88), as in FEditor's and FEBuilder's frames.

Usage:
  python3 Tools/banim/febuilder_txt.py [--lz77] [--sheets DIR] SRC_DIR OUT_DIR PREFIX ABBR
  python3 Tools/banim/febuilder_txt.py [--lz77] [--sheets DIR] SCRIPT.txt OUT_DIR PREFIX ABBR

SCRIPT.txt picks one script when the folder holds several (the frames are read from its folder).

--sheets DIR also saves the packed sheets there as PNGs, to look at.

Each frame is cut into OBJs (8x8 tiles; empty tiles are left out), the OBJs are packed into 256x64 sheets in the
order the script first shows the frames, and an OBJ whose pixels are already on its sheet reuses them. Modes 2 and 4
(the layer behind the unit) are modes 1 and 3 with every frame empty, as FEditor makes them. The four palettes are
the frames' palette. The output is what Tools/banim/feditor_dump.py writes (see its docstring and
Documentation/Features/CustomBattleAnimations.md).
"""
import glob
import os
import re
import struct
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

from PIL import Image  # noqa: E402

import feditor_dump as fd  # noqa: E402

FRAME_W, FRAME_H = 248, 160
ORIGIN_X, ORIGIN_Y = 148, 88
SHEET_TW, SHEET_TH = 32, 8
# (width, height) in tiles -> (shape, size) of attr0/attr1
OBJ_SHAPES = {(8, 8): (0, 3), (8, 4): (1, 3), (4, 8): (2, 3), (4, 4): (0, 2), (4, 2): (1, 2), (2, 4): (2, 2),
              (4, 1): (1, 1), (1, 4): (2, 1), (2, 2): (0, 1), (2, 1): (1, 0), (1, 2): (2, 0), (1, 1): (0, 0)}
# An OBJ tile that is empty costs this much against a tile it covers: fewer, bigger OBJs at some room on the sheet.
WASTE = 0.6
OBJ_COST = 1.5
TERM = struct.pack('<HHHhhH', 1, 0, 0, 0, 0, 0)


def fail(msg):
    sys.exit('febuilder_txt: error: ' + msg)


def parse_script(path):
    """Return {mode: [('C', n) | ('F', file, duration)]}."""
    modes, cur = {}, None
    with open(path, encoding='utf-8-sig') as f:
        for ln, line in enumerate(f, 1):
            s = line.strip()
            m = re.match(r'^/// - Mode (\d+)', s)
            if m:
                cur = int(m.group(1))
                if cur in modes or cur in (2, 4) or not 1 <= cur <= 12:
                    fail('%s:%d: unexpected mode %d' % (path, ln, cur))
                modes[cur] = []
                continue
            if not s or s.startswith('///') or s.startswith('#'):
                continue
            if s == '~~~':
                cur = None
                continue
            if cur is None:
                fail('%s:%d: "%s" is outside a mode' % (path, ln, s))
            m = re.match(r'^C([0-9A-Fa-f]{1,2})$', s)
            if m:
                modes[cur].append(('C', int(m.group(1), 16)))
                continue
            m = re.match(r'^(\d+)\s+p-\s+(.+)$', s)
            if m:
                modes[cur].append(('F', m.group(2).strip(), int(m.group(1))))
                continue
            fail('%s:%d: cannot read "%s"' % (path, ln, s))
    missing = [n for n in (1, 3, 5, 6, 7, 8, 9, 10, 11, 12) if n not in modes]
    if missing:
        fail('modes %s are missing' % missing)
    return modes


def load_frame(path):
    im = Image.open(path)
    if im.mode != 'P' or im.size != (FRAME_W, FRAME_H):
        fail('%s: need a %dx%d mode-P image, got %s %s' % (path, FRAME_W, FRAME_H, im.mode, im.size))
    raw = im.tobytes()
    if max(raw) > 15:
        fail('%s: uses palette entries past 15' % path)
    pal = im.getpalette()[:48]
    return raw, pal


def tile_at(raw, x0, y0):
    """The 8x8 tile at pixel (x0, y0) as 64 indices; outside the frame is transparent."""
    out = bytearray(64)
    for y in range(8):
        yy = y0 + y
        if not 0 <= yy < FRAME_H:
            continue
        for x in range(8):
            xx = x0 + x
            if 0 <= xx < FRAME_W:
                out[y * 8 + x] = raw[yy * FRAME_W + xx]
    return bytes(out)


def cut_frame(raw):
    """Cut a frame into OBJs. Returns [(tx, ty, tw, th, tiles)], tile coordinates relative to the origin's tile grid
    shifted by the best (dx, dy), and the shift."""
    xs = [i % FRAME_W for i in range(len(raw)) if raw[i]]
    ys = [i // FRAME_W for i in range(len(raw)) if raw[i]]
    if not xs:
        return [], (0, 0)
    grids = []
    for dy in range(8):
        for dx in range(8):
            grid = {}
            for ty in range((min(ys) - dy) // 8, (max(ys) - dy) // 8 + 1):
                for tx in range((min(xs) - dx) // 8, (max(xs) - dx) // 8 + 1):
                    t = tile_at(raw, tx * 8 + dx, ty * 8 + dy)
                    if any(t):
                        grid[(tx, ty)] = t
            grids.append((len(grid), (dx, dy), grid))
    grids.sort(key=lambda g: g[0])
    best = None
    for _, shift, grid in grids[:6]:
        objs = cover(grid)
        cost = sum(w * h for _, _, w, h in objs) + OBJ_COST * len(objs)
        if best is None or cost < best[0]:
            best = (cost, objs, grid, shift)
    _, objs, grid, shift = best
    out = []
    for tx, ty, w, h in objs:
        tiles = [grid.get((tx + i, ty + j), bytes(64)) for j in range(h) for i in range(w)]
        out.append((tx, ty, w, h, tiles))
    return out, shift


def cover(grid):
    """Greedy cover of the non-empty tiles with OBJ rectangles."""
    left = set(grid)
    objs = []
    while left:
        best = None
        for (w, h) in OBJ_SHAPES:
            area = w * h
            if best is not None and area - OBJ_COST <= best[0]:
                continue
            starts = {(x - i, y - j) for (x, y) in left for i in range(w) for j in range(h)}
            for (sx, sy) in starts:
                got = sum((sx + i, sy + j) in left for j in range(h) for i in range(w))
                score = got - WASTE * (area - got) - OBJ_COST
                if best is None or score > best[0]:
                    best = (score, sx, sy, w, h)
        _, sx, sy, w, h = best
        objs.append((sx, sy, w, h))
        for i in range(w):
            for j in range(h):
                left.discard((sx + i, sy + j))
    return objs


class Sheet:
    def __init__(self):
        self.used = [[False] * SHEET_TW for _ in range(SHEET_TH)]
        self.tiles = [[bytes(64)] * SHEET_TW for _ in range(SHEET_TH)]
        self.blocks = {}  # (w, h, tiles) -> (x, y)

    def free_spot(self, w, h, used):
        for y in range(SHEET_TH - h + 1):
            for x in range(SHEET_TW - w + 1):
                if all(not used[y + j][x + i] for j in range(h) for i in range(w)):
                    return x, y
        return None

    def place_frame(self, objs):
        """Place a frame's OBJs; returns their tile numbers, or None (the sheet is left as it was) if they don't
        fit."""
        used = [row[:] for row in self.used]
        new_blocks = {}
        spots = [None] * len(objs)
        order = sorted(range(len(objs)), key=lambda k: -objs[k][2] * objs[k][3])
        for k in order:
            _, _, w, h, tiles = objs[k]
            key = (w, h, tuple(tiles))
            if key in self.blocks or key in new_blocks:
                spots[k] = self.blocks.get(key) or new_blocks[key]
                continue
            spot = self.free_spot(w, h, used)
            if spot is None:
                return None
            x, y = spot
            for j in range(h):
                for i in range(w):
                    used[y + j][x + i] = True
            new_blocks[key] = spot
            spots[k] = spot
        self.used = used
        for (w, h, tiles), (x, y) in new_blocks.items():
            for j in range(h):
                for i in range(w):
                    self.tiles[y + j][x + i] = tiles[j * w + i]
        self.blocks.update(new_blocks)
        return [y * SHEET_TW + x for x, y in spots]

    def rows(self):
        rows = []
        for ty in range(SHEET_TH):
            for r in range(8):
                rows.append(bytes(b for tx in range(SHEET_TW) for b in self.tiles[ty][tx][r * 8:r * 8 + 8]))
        return rows

    def png(self, pal, path):
        im = Image.new('P', (SHEET_TW * 8, SHEET_TH * 8))
        im.putpalette(pal)
        im.frombytes(b''.join(self.rows()))
        im.save(path)


def frame_oam(objs, shift, tile_nums):
    out = bytearray()
    for (tx, ty, w, h, _), tile in zip(objs, tile_nums):
        shape, size = OBJ_SHAPES[(w, h)]
        x = tx * 8 + shift[0] - ORIGIN_X
        y = ty * 8 + shift[1] - ORIGIN_Y
        out += struct.pack('<HHHhhH', shape << 14, size << 14, tile, x, y, 0)
    return bytes(out) + TERM


def main():
    args = sys.argv[1:]
    lz77_sheets = '--lz77' in args
    args = [a for a in args if a != '--lz77']
    sheet_dir = None
    if '--sheets' in args:
        k = args.index('--sheets')
        sheet_dir = args[k + 1]
        del args[k:k + 2]
    if len(args) != 4:
        sys.exit('usage: febuilder_txt.py [--lz77] [--sheets DIR] SRC_DIR|SCRIPT.txt OUT_DIR PREFIX ABBR')
    src_dir, out_dir, prefix, abbr = args
    if src_dir.endswith('.txt'):
        txts = [src_dir]
        src_dir = os.path.dirname(src_dir) or '.'
    else:
        txts = glob.glob(os.path.join(glob.escape(src_dir), '*.txt'))
    if len(txts) != 1:
        fail('expected exactly one *.txt in %s, found %d' % (src_dir, len(txts)))
    script = parse_script(txts[0])

    # The frames, in the order the script first shows them.
    order = []
    for n in sorted(script):
        for op in script[n]:
            if op[0] == 'F' and op[1] not in order:
                order.append(op[1])
    if len(order) > 256:
        fail('%d frames; at most 256' % len(order))
    frames, pal = {}, None
    for i, fn in enumerate(order):
        raw, p = load_frame(os.path.join(src_dir, fn))
        if pal is None:
            pal = p
        elif p != pal:
            fail('%s: its palette differs from %s\'s' % (fn, order[0]))
        objs, shift = cut_frame(raw)
        frames[fn] = {'num': i, 'objs': objs, 'shift': shift}

    sheets = [Sheet()]
    oam = bytearray()
    for fn in order:
        f = frames[fn]
        nums = sheets[-1].place_frame(f['objs'])
        if nums is None:
            sheets.append(Sheet())
            nums = sheets[-1].place_frame(f['objs'])
            if nums is None:
                fail('%s does not fit on one sheet' % fn)
        f['sheet'] = len(sheets) - 1
        f['oam'] = len(oam)
        oam += frame_oam(f['objs'], f['shift'], nums)
    empty_oam = len(oam)
    oam += TERM

    def mode_words(ops, empty):
        w = []
        for op in ops:
            if op[0] == 'C':
                w.append(0x85000000 | op[1])
            else:
                f = frames[op[1]]
                if not 1 <= op[2] <= 0xFFFF:
                    fail('%s: duration %d' % (op[1], op[2]))
                w += [0x86000000 | f['num'] << 16 | op[2], f['sheet'], empty_oam if empty else f['oam']]
        return w + [0x80000000]

    words, offsets = [], []
    for n in range(1, 13):
        offsets.append(len(words) * 4)
        if n in (2, 4):
            words += mode_words(script[n - 1], True)
        else:
            words += mode_words(script[n], False)
    modes = struct.pack('<12I', *offsets)
    pal555 = bytearray()
    for k in range(16):
        r, g, b = pal[k * 3:k * 3 + 3]
        pal555 += struct.pack('<H', (r >> 3) | (g >> 3) << 5 | (b >> 3) << 10)
    pal555 = bytes(pal555) * 4
    oam_r = bytes(oam)
    oam_l = fd.mirror_oam(oam_r)
    if oam_l is None:
        fail('cannot mirror the OAM')

    os.makedirs(out_dir, exist_ok=True)
    rows = [s.rows() for s in sheets]
    nobj = [len(frames[fn]['objs']) for fn in order]
    print('%s: %d frames, %d sheets, %d to %d OBJs a frame' % (os.path.basename(txts[0]), len(order), len(sheets),
                                                                 min(nobj), max(nobj)))
    if sheet_dir:
        os.makedirs(sheet_dir, exist_ok=True)
        for k, s in enumerate(sheets, 1):
            s.png(pal, os.path.join(sheet_dir, 'Sheet %d.png' % k))
    fd.convert(os.path.basename(txts[0]), modes, oam_r, oam_l, pal555, words, rows, out_dir, prefix, abbr,
               lz77_sheets, tool='febuilder_txt.py')


if __name__ == '__main__':
    main()
