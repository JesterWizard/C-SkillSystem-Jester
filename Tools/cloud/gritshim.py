#!/usr/bin/env python3
import warnings; warnings.filterwarnings("ignore")
"""Minimal stand-in for devkitPro grit / gbalzss, covering only the argument
sets used by C-SkillSystem's makefiles (bin output, no headers)."""
import os
import struct
import sys

from PIL import Image


def lz77(data):
    out = bytearray(struct.pack('<I', 0x10 | (len(data) << 8)))
    i, n = 0, len(data)
    chains = {}

    def add(pos):
        if pos + 3 <= n:
            chains.setdefault(data[pos:pos + 3], []).append(pos)

    while i < n:
        flag_pos = len(out)
        out.append(0)
        flags = 0
        for bit in range(8):
            if i >= n:
                break
            best_len, best_disp = 0, 0
            for p in reversed(chains.get(data[i:i + 3], ())[-256:]):
                disp = i - p
                if disp > 0x1000:
                    break
                if disp < 2:
                    continue
                l = 0
                while l < 18 and i + l < n and data[p + l] == data[i + l]:
                    l += 1
                if l > best_len:
                    best_len, best_disp = l, disp
                    if l == 18:
                        break
            if best_len >= 3:
                flags |= 0x80 >> bit
                v = ((best_len - 3) << 12) | (best_disp - 1)
                out += bytes([v >> 8, v & 0xFF])
                for k in range(best_len):
                    add(i + k)
                i += best_len
            else:
                out.append(data[i])
                add(i)
                i += 1
        out[flag_pos] = flags
    while len(out) % 4:
        out.append(0)
    return bytes(out)


def unlz77(src):
    size = struct.unpack('<I', src[:4])[0] >> 8
    out = bytearray()
    i = 4
    while len(out) < size:
        f = src[i]
        i += 1
        for bit in range(8):
            if len(out) >= size:
                break
            if f & (0x80 >> bit):
                v = (src[i] << 8) | src[i + 1]
                i += 2
                for _ in range((v >> 12) + 3):
                    out.append(out[-((v & 0xFFF) + 1)])
            else:
                out.append(src[i])
                i += 1
    return bytes(out)


def load_indexed(path):
    im = Image.open(path)
    if im.mode == 'P':
        pal = im.getpalette() or []
        bits = 4 if len(pal) // 3 <= 16 and max(im.getdata(), default=0) < 16 \
            and im.info.get('bits', 8) <= 4 else 8
        return im, pal, bits
    im = im.convert('RGB')
    colors, idx = [], {}
    for px in im.getdata():
        if px not in idx:
            idx[px] = len(colors)
            colors.append(px)
    pim = Image.new('P', im.size)
    pim.putdata([idx[p] for p in im.getdata()])
    pal = [c for col in colors for c in col]
    pim.putpalette(pal + [0] * (768 - len(pal)))
    return pim, pal, 8


def gba_pal(pal, count):
    out = bytearray()
    for k in range(count):
        if 3 * k + 2 < len(pal):
            r, g, b = pal[3 * k:3 * k + 3]
        else:
            r = g = b = 0
        out += struct.pack('<H', (r >> 3) | ((g >> 3) << 5) | ((b >> 3) << 10))
    return bytes(out)


def tile_bytes(px, bpp):
    out = bytearray()
    per = 8 // bpp
    mask = (1 << bpp) - 1
    for j in range(0, len(px), per):
        v = 0
        for k in range(per):
            v |= (px[j + k] & mask) << (k * bpp)
        out.append(v)
    return bytes(out)


def tiles_of(im, tw=8, th=8):
    w, h = im.size
    data = list(im.getdata())
    res = []
    for my in range(0, h // th):
        for mx in range(0, w // tw):
            x0 = mx * tw
            y0 = my * th
            # a grit "tile" of tw x th is stored as linear rows of tw pixels
            res.append([data[(y0 + y) * w + x0 + x]
                        for y in range(th) for x in range(tw)])
    return res


def main(argv):
    src = argv[0]
    args = argv[1:]
    bpp, gfx, pal_on, map_on = 8, True, True, False
    gz = mz = False
    pn = None
    tw = th = 8
    outname = None
    i = 0
    while i < len(args):
        a = args[i]
        if a == '-gB':
            i += 1
            bpp = int(args[i])
        elif a.startswith('-gB'):
            bpp = int(a[3:])
        elif a == '-g!':
            gfx = False
        elif a == '-gzl':
            gz = True
        elif a == '-p!':
            pal_on = False
        elif a == '-p':
            pal_on = True
        elif a == '-pn':
            i += 1
            pn = int(args[i])
        elif a == '-m':
            map_on = True
        elif a == '-m!':
            map_on = False
        elif a == '-mzl':
            mz = True
        elif a.startswith('-tw'):
            tw = int(a[3:])
        elif a.startswith('-th'):
            th = int(a[3:])
        elif a == '-o':
            i += 1
            outname = args[i]
        elif a in ('-gb', '-gu', '-ft', '-fh!', '-ftb', '-mLf', '-mR4'):
            if a in ('-gu', '-ft'):
                i += 1
        i += 1

    if outname is None:
        base = os.path.splitext(os.path.basename(src))[0]
    else:
        base = outname
        for ext in ('.bin', '.img', '.map', '.pal'):
            if base.endswith(ext):
                base = base[:-len(ext)]
        if base.endswith('.img') or base.endswith('.map') or base.endswith('.pal'):
            base = base[:-4]

    im, pal, srcbits = load_indexed(src)

    if gfx:
        tiles = tiles_of(im, tw, th)
        if map_on:
            mask = (1 << bpp) - 1
            uniq, lut, mp = [], {}, []
            for t in tiles:
                bank = (max(t) >> bpp) if bpp == 4 else 0
                lo = tuple(p & mask for p in t)
                cand = [
                    (lo, 0),
                    (tuple(lo[y * 8 + 7 - x] for y in range(8) for x in range(8)), 1),
                    (tuple(lo[(7 - y) * 8 + x] for y in range(8) for x in range(8)), 2),
                    (tuple(lo[(7 - y) * 8 + 7 - x] for y in range(8) for x in range(8)), 3),
                ]
                hit = None
                for c, f in cand:
                    if c in lut:
                        hit = (lut[c], f)
                        break
                if hit is None:
                    lut[lo] = len(uniq)
                    uniq.append(lo)
                    hit = (lut[lo], 0)
                mp.append(hit[0] | (hit[1] << 10) | (bank << 12))
            img = b''.join(tile_bytes(t, bpp) for t in uniq)
            mapb = b''.join(struct.pack('<H', e) for e in mp)
            open(base + '.map.bin', 'wb').write(lz77(mapb) if mz else mapb)
        else:
            img = b''.join(tile_bytes(t, bpp) for t in tiles)
        open(base + '.img.bin', 'wb').write(lz77(img) if gz else img)

    if pal_on:
        count = pn if pn else (16 if srcbits <= 4 else max(16, (len(pal) // 3 + 15) // 16 * 16))
        open(base + '.pal.bin', 'wb').write(gba_pal(pal, count))


if __name__ == '__main__':
    if os.path.basename(sys.argv[0]).startswith('gbalzss'):
        assert sys.argv[1] == 'e'
        open(sys.argv[3], 'wb').write(lz77(open(sys.argv[2], 'rb').read()))
    else:
        main(sys.argv[1:])
