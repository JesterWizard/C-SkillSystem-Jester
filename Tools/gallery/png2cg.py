#!/usr/bin/env python3
"""Turn a 240x160 (or 256x160) PNG into a 224- or 256-colour background (Documentation/Features/MaxColorBackgrounds.md).

    python3 Tools/gallery/png2cg.py COLOURS PNG IMG.dmp PAL.dmp

A stand-in for Huichelaar's Sommie, with the same output:
    IMG.dmp: the picture as 8bpp tiles, 32x20 tiles in order (the last 2 tile columns, x 240-255, are not shown),
             LZ77 compressed (VRAM safe).
    PAL.dmp: its colours, RGB555. For 224 colours, pixel values 32-63 are skipped (banks 2-3 hold the text's
             palettes): palette entries 0-31 are pixel values 0-31 and entries 32-223 are pixel values 64-255.
COLOURS is 224 (portraits and speech bubbles can be drawn over it: TSA word 1 in BG.event) or 256 (TSA word 0).
The picture may have up to COLOURS colours once cut to the GBA's 15 bits; more is an error (reduce it first).
Colour 0 is the backdrop's: the darkest colour goes there.
"""
import struct
import sys

from PIL import Image

W, H, FULL_W = 240, 160, 256


def lz77_compress(data):
    """GBA LZ77 (type 0x10). Matches never start 1 byte back, so the VRAM decompressor can read it."""
    out = bytearray(struct.pack("<I", len(data) << 8 | 0x10))
    i, n = 0, len(data)
    seen = {}  # 3-byte string -> the positions it starts at, oldest first
    added = 0

    def note_up_to(end):
        nonlocal added
        while added < end and added + 3 <= n:
            seen.setdefault(data[added:added + 3], []).append(added)
            added += 1

    while i < n:
        flag_pos = len(out)
        out.append(0)
        flags = 0
        for bit in range(8):
            if i >= n:
                break
            best_len, best_d = 0, 0
            note_up_to(i - 1)  # a match starts at least 2 back
            for tries, p in enumerate(reversed(seen.get(data[i:i + 3], ()))):
                d = i - p
                if d > 0x1000 or tries == 512:
                    break
                length = 0
                while length < 18 and i + length < n and data[i + length] == data[i - d + length]:
                    length += 1
                if length > best_len:
                    best_len, best_d = length, d
                    if length == 18:
                        break
            if best_len >= 3:
                flags |= 0x80 >> bit
                x = (best_len - 3) << 12 | (best_d - 1)
                out += bytes((x >> 8, x & 0xFF))
                i += best_len
            else:
                out.append(data[i])
                i += 1
        out[flag_pos] = flags
    while len(out) % 4:
        out.append(0)
    return bytes(out)


def main():
    colours, png, img_out, pal_out = int(sys.argv[1]), sys.argv[2], sys.argv[3], sys.argv[4]
    if colours not in (224, 256):
        sys.exit("png2cg.py: COLOURS is 224 or 256")
    im = Image.open(png).convert("RGB")
    if im.size not in ((W, H), (FULL_W, H)):
        sys.exit("png2cg.py: %s is %dx%d, not 240x160 or 256x160" % ((png,) + im.size))
    px = [(r >> 3, g >> 3, b >> 3) for r, g, b in im.get_flattened_data()] \
        if hasattr(im, "get_flattened_data") else [(r >> 3, g >> 3, b >> 3) for r, g, b in im.getdata()]
    iw = im.size[0]
    pal = sorted(set(px), key=lambda c: (sum(c), c))
    if len(pal) > colours:
        sys.exit("png2cg.py: %s has %d colours in 15 bits, more than %d" % (png, len(pal), colours))
    index = {c: i for i, c in enumerate(pal)}

    def value(i):
        return i if colours == 256 or i < 32 else i + 32

    tiles = bytearray()
    for ty in range(H // 8):
        for tx in range(FULL_W // 8):
            for y in range(8):
                for x in range(8):
                    sx, sy = tx * 8 + x, ty * 8 + y
                    tiles.append(value(index[px[sy * iw + sx]]) if sx < iw else 0)
    words = [r | g << 5 | b << 10 for r, g, b in pal] + [0] * (colours - len(pal))
    with open(img_out, "wb") as f:
        f.write(lz77_compress(bytes(tiles)))
    with open(pal_out, "wb") as f:
        f.write(struct.pack("<%dH" % colours, *words))


if __name__ == "__main__":
    main()
