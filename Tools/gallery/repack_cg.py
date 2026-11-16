#!/usr/bin/env python3
"""Rewrite 224/256-colour background images (IMG.dmp) in the solid-tile format where that is smaller.

    python3 Tools/gallery/repack_cg.py IMG.dmp...

Lossless: each file is decoded to its tiles, re-packed with png2cg.pack_img, and checked to decode to the same tiles.
Files already in the smaller format are left alone. Prints the bytes saved.
"""
import sys

from png2cg import pack_img, unpack_img


def main():
    before = after = 0
    for path in sys.argv[1:]:
        old = open(path, "rb").read()
        tiles = unpack_img(old)
        new = pack_img(tiles)
        if unpack_img(new) != tiles:
            sys.exit("repack_cg.py: %s does not round-trip" % path)
        before += len(old)
        if len(new) < len(old):
            open(path, "wb").write(new)
            after += len(new)
        else:
            after += len(old)
    print("repack_cg.py: %d files, %d -> %d bytes (%d saved)" % (len(sys.argv) - 1, before, after, before - after))


if __name__ == "__main__":
    main()
