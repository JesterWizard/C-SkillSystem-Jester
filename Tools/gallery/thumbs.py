#!/usr/bin/env python3
"""The Gallery's thumbnails (Documentation/Features/Gallery.md), made at build time.

    python3 Tools/gallery/thumbs.py ROM GALLERY_C BG_EVENT OUT

For each row of gGalleryEntries (GALLERY_C, up to the GALLERY_END row) it decodes that background from the background
table (BG_EVENT): a vanilla one from the clean ROM (an LZ77 4bpp image, a TSA, 8 palettes), a 224- or 256-colour one
from its .dmp files (an LZ77 8bpp image of 32x20 tiles in order, a palette). It shrinks the 240x160 picture to 48x32
(the average of each 5x5 block), picks 15 colours for it (median cut) and writes, per row:
    768 bytes: 4bpp tiles in the sprites' order (1D): the 32x32 part (4x4 tiles), then the 16x32 part (2x4 tiles)
     32 bytes: its 16-colour palette (colour 0 unused: transparent)
The output is build output (gitignored): it is made from the ROM, so the repo holds none of vanilla's art.
"""
import os
import re
import struct
import sys

W, H, TW, TH = 240, 160, 48, 32


def lz77(data, off):
    size = struct.unpack_from("<I", data, off)[0] >> 8
    off += 4
    out = bytearray()
    while len(out) < size:
        flags = data[off]
        off += 1
        for bit in range(8):
            if len(out) >= size:
                break
            if flags & (0x80 >> bit):
                x = data[off] << 8 | data[off + 1]
                off += 2
                n, d = (x >> 12) + 3, (x & 0xFFF) + 1
                for _ in range(n):
                    out.append(out[-d])
            else:
                out.append(data[off])
                off += 1
    return bytes(out)


def rgb(c):
    return (c & 31, (c >> 5) & 31, (c >> 10) & 31)


def parse_entries(path):
    """gGalleryEntries' background indices, in order."""
    text = open(path).read()
    body = text[text.index("gGalleryEntries"):]
    out = []
    for m in re.finditer(r"\{\s*(GALLERY_END|0x[0-9A-Fa-f]+|\d+)\s*,", body):
        if m.group(1) == "GALLERY_END":
            break
        out.append(int(m.group(1), 0))
    return out


def parse_bg_table(path):
    """The background table: [(img, tsa, pal)], each an address (int) or a label, and the labels' files."""
    text = open(path).read()
    labels = {m.group(1): os.path.join(os.path.dirname(path), m.group(2))
              for m in re.finditer(r"^(\w+):\s*#incbin\s*\"([^\"]+)\"", text, re.M)}
    table = []
    for line in text[text.index("bgTable:"):].splitlines()[1:]:
        line = line.split("//")[0].strip()
        if not line:
            continue
        if line.startswith("WORD") and ";" not in line:
            nums = [int(v, 0) for v in line.split()[1:]]
            table += [tuple(nums[i:i + 3]) for i in range(0, len(nums), 3)]
            continue
        m = re.match(r"POIN\s+(\w+);\s*WORD\s+(\w+);\s*POIN\s+(\w+)", line)
        if m:
            table.append((m.group(1), int(m.group(2), 0), m.group(3)))
            continue
        if line.startswith(("ALIGN", "#", "POIN", "WORD")):
            continue
        break
    return table, labels


def render(entry, rom, labels):
    """The background as 240x160 RGB555 triples (0-31)."""
    img, tsa, pal = entry
    black = [(0, 0, 0)] * (W * H)
    if isinstance(img, int) and img == 0:
        return black
    if isinstance(img, str):
        # 224 (TSA 1) or 256 (TSA 0) colours: 8bpp tiles in order, 32 to a row
        gfx = lz77(open(labels[img], "rb").read(), 0)
        raw = open(labels[pal], "rb").read()
        colors = [struct.unpack_from("<H", raw, 2 * i)[0] for i in range(len(raw) // 2)]
        if tsa == 1:  # banks 2-3 are left for text: colours 0-31, then 64-255
            colors = colors[:32] + [0] * 32 + colors[32:]
        out = []
        for y in range(H):
            for x in range(W):
                t = (y >> 3) * 32 + (x >> 3)
                i = t * 64 + (y & 7) * 8 + (x & 7)
                ci = gfx[i] if i < len(gfx) else 0
                out.append(rgb(colors[ci] if ci < len(colors) else 0))
        return out
    # Vanilla: 4bpp tiles, a TSA (width-1, height-1, then the rows bottom to top), 8 palettes
    gfx = lz77(rom, img - 0x08000000)
    t = tsa - 0x08000000
    w, h = rom[t] + 1, rom[t + 1] + 1
    ents = struct.unpack_from("<%dH" % (w * h), rom, t + 2)
    colors = [struct.unpack_from("<H", rom, pal - 0x08000000 + 2 * i)[0] for i in range(128)]
    out = []
    for y in range(H):
        for x in range(W):
            tx, ty = x >> 3, y >> 3
            if tx >= w or ty >= h:
                out.append((0, 0, 0))
                continue
            e = ents[(h - 1 - ty) * w + tx]
            px, py = x & 7, y & 7
            if e & 0x400:
                px = 7 - px
            if e & 0x800:
                py = 7 - py
            i = (e & 0x3FF) * 32 + py * 4 + (px >> 1)
            ci = (gfx[i] >> ((px & 1) * 4)) & 15 if i < len(gfx) else 0
            out.append(rgb(colors[((e >> 12) & 7) * 16 + ci]) if ci else (0, 0, 0))
    return out


def shrink(pixels):
    out = []
    for ty in range(TH):
        for tx in range(TW):
            acc = [0, 0, 0]
            for y in range(ty * 5, ty * 5 + 5):
                for x in range(tx * 5, tx * 5 + 5):
                    p = pixels[y * W + x]
                    for k in range(3):
                        acc[k] += p[k]
            out.append(tuple((v + 12) // 25 for v in acc))
    return out


def median_cut(pixels, n):
    boxes = [list(pixels)]
    while len(boxes) < n:
        spans = [(max(max(p[k] for p in b) - min(p[k] for p in b) for k in range(3)) if len(b) > 1 else -1, i)
                 for i, b in enumerate(boxes)]
        span, i = max(spans)
        if span <= 0:
            break
        box = boxes.pop(i)
        k = max(range(3), key=lambda c: max(p[c] for p in box) - min(p[c] for p in box))
        box.sort(key=lambda p: p[k])
        boxes += [box[:len(box) // 2], box[len(box) // 2:]]
    return [tuple((sum(p[k] for p in b) + len(b) // 2) // len(b) for k in range(3)) for b in boxes]


def thumbnail(pixels):
    small = shrink(pixels)
    palette = median_cut(small, 15)
    cache = {}

    def nearest(p):
        if p not in cache:
            cache[p] = min(range(len(palette)),
                           key=lambda i: sum((p[k] - palette[i][k]) ** 2 for k in range(3))) + 1
        return cache[p]
    idx = [nearest(p) for p in small]
    tiles = bytearray()
    for x0, cols in ((0, 4), (32, 2)):  # the 32x32 sprite, then the 16x32 one
        for row in range(4):
            for col in range(cols):
                for y in range(8):
                    for x in range(0, 8, 2):
                        px = x0 + col * 8 + x
                        py = row * 8 + y
                        tiles.append(idx[py * TW + px] | idx[py * TW + px + 1] << 4)
    pal = [0] + [c[0] | c[1] << 5 | c[2] << 10 for c in palette]
    pal += [0] * (16 - len(pal))
    return bytes(tiles) + struct.pack("<16H", *pal)


def main():
    rom_path, list_path, bg_path, out_path = sys.argv[1:5]
    rom = open(rom_path, "rb").read()
    table, labels = parse_bg_table(bg_path)
    out = bytearray()
    for bg in parse_entries(list_path):
        if bg >= len(table):
            sys.exit("thumbs.py: background %#x is not in %s" % (bg, bg_path))
        out += thumbnail(render(table[bg], rom, labels))
    with open(out_path, "wb") as f:
        f.write(out)


if __name__ == "__main__":
    main()
