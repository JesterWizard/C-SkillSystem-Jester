#!/usr/bin/env python3
"""Generate a FE8 map (Tiled TMX, compressed .dmp, map .event) from a layout JSON.

Usage:
  python3 Tools/district/district_map.py <layout.json> <out_dir> <basename> \
      --chapter 0x3B --map-id 0xCA --label MapData_Serafew [--check]

Layout JSON:
  {"w": 24, "h": 18, "tileset": [obj, palette, tileconfig], "tiles": [w*h metatile indices]}
  Tiles are row-major. Each value is a metatile index (0..1023).

Writes into out_dir:
  <basename>.tmx        Tiled map, gid = metatile index + 1
  <basename>_data.dmp   uncompressed (u8 w, u8 h, u16 tile*4 ...), LZ77 type 0x10
  <basename>.event      ALIGN/label/#incbin plus NewEventPointerTable_Maps
"""

import argparse
import json
import os
import subprocess
import sys
import tempfile
from pathlib import Path

TILE_COUNT = 1024
REPO_ROOT = Path(__file__).resolve().parents[2]
COMPRESS = REPO_ROOT / "Tools" / "EventAssembler" / "Tools" / "compress"


def load_layout(path):
    with open(path, "r", encoding="utf-8") as f:
        layout = json.load(f)
    w, h = layout["w"], layout["h"]
    tileset = layout["tileset"]
    tiles = layout["tiles"]
    if not (1 <= w <= 255 and 1 <= h <= 255):
        raise ValueError("w and h must be 1..255")
    if len(tileset) != 3 or any(not (0 <= v <= 255) for v in tileset):
        raise ValueError("tileset must be [obj, palette, tileconfig], each 0..255")
    if len(tiles) != w * h:
        raise ValueError("tiles has %d entries, expected %d" % (len(tiles), w * h))
    for t in tiles:
        if not (0 <= t < TILE_COUNT):
            raise ValueError("metatile index %r out of range 0..%d" % (t, TILE_COUNT - 1))
    return w, h, tileset, tiles


def raw_bytes(w, h, tiles):
    out = bytearray([w, h])
    for t in tiles:
        out += (t * 4).to_bytes(2, "little")
    return bytes(out)


def write_tmx(path, w, h, tileset, tiles, chapter, map_id):
    obj, palette, tileconfig = tileset
    name = "%02X%02X%02X%02X" % (obj, 0, palette, tileconfig)
    rows = []
    for y in range(h):
        row = tiles[y * w:(y + 1) * w]
        rows.append("  " + ",".join(str(t + 1) for t in row))
    csv = ",\n".join(rows)
    text = (
        '<?xml version="1.0" encoding="UTF-8"?>\n'
        '<map version="1.10" tiledversion="1.11.0" orientation="orthogonal" renderorder="right-down" '
        'width="%d" height="%d" tilewidth="16" tileheight="16" infinite="0" nextlayerid="2" nextobjectid="1">\n'
        ' <tileset firstgid="1" name="%s" tilewidth="16" tileheight="16" tilecount="%d" columns="32">\n'
        '  <image source="%s.png" trans="ffffff" width="512" height="512"/>\n'
        ' </tileset>\n'
        ' <layer id="1" name="Tile Layer 1" width="%d" height="%d">\n'
        '  <properties>\n'
        '   <property name="ChapterID" value="0x%02X"/>\n'
        '   <property name="Main" value=""/>\n'
        '   <property name="MapID" value="0x%02X"/>\n'
        '   <property name="ObjectType" value="0x%02X"/>\n'
        '   <property name="PaletteID" value="0x%02X"/>\n'
        '   <property name="TileConfig" value="0x%02X"/>\n'
        '  </properties>\n'
        '  <data encoding="csv">\n'
        '%s\n'
        '  </data>\n'
        ' </layer>\n'
        '</map>\n'
    ) % (w, h, name, TILE_COUNT, name, w, h, chapter, map_id, obj, palette, tileconfig, csv)
    with open(path, "w", encoding="utf-8", newline="\n") as f:
        f.write(text)


def compress_to(raw, out_path):
    if not COMPRESS.is_file():
        raise FileNotFoundError("compress tool not found: %s" % COMPRESS)
    with tempfile.TemporaryDirectory() as tmp:
        in_path = os.path.join(tmp, "raw.bin")
        with open(in_path, "wb") as f:
            f.write(raw)
        subprocess.run([str(COMPRESS), in_path, out_path], check=True)
    with open(out_path, "rb") as f:
        data = f.read()
    if not data or data[0] != 0x10:
        raise RuntimeError("compressed output does not start with 0x10: %s" % out_path)


def lz77_decode(data):
    """Decode GBA LZ77 type 0x10 (the format written by the compress tool)."""
    if data[0] != 0x10:
        raise ValueError("not LZ77 type 0x10")
    size = data[1] | (data[2] << 8) | (data[3] << 16)
    out = bytearray()
    pos = 4
    while len(out) < size:
        flags = data[pos]
        pos += 1
        for bit in range(8):
            if len(out) >= size:
                break
            if flags & (0x80 >> bit):
                b0 = data[pos]
                b1 = data[pos + 1]
                pos += 2
                length = (b0 >> 4) + 3
                disp = (((b0 & 0x0F) << 8) | b1) + 1
                start = len(out) - disp
                if start < 0:
                    raise ValueError("back-reference before start of output")
                for i in range(length):
                    out.append(out[start + i])
            else:
                out.append(data[pos])
                pos += 1
    return bytes(out[:size])


def write_event(path, layout_name, label, map_id, basename):
    text = (
        "// Generated by Tools/district/district_map.py from %s. Edit the layout, not this file.\n"
        "{\n"
        "ALIGN 4\n"
        "%s:\n"
        "  #incbin \"%s_data.dmp\"\n"
        "\n"
        "NewEventPointerTable_Maps(%s, %s)\n"
        "}\n"
    ) % (layout_name, label, basename, map_id, label)
    with open(path, "w", encoding="utf-8", newline="\n") as f:
        f.write(text)


def main(argv=None):
    ap = argparse.ArgumentParser(description="Layout JSON -> FE8 map TMX/.dmp/.event")
    ap.add_argument("layout")
    ap.add_argument("out_dir")
    ap.add_argument("basename")
    ap.add_argument("--chapter", required=True, type=lambda s: int(s, 0), help="ChapterID, e.g. 0x3B")
    ap.add_argument("--map-id", required=True, type=lambda s: int(s, 0), help="MapID, e.g. 0xCA")
    ap.add_argument("--label", required=True, help="event label for the map data, e.g. MapData_Serafew")
    ap.add_argument("--check", action="store_true", help="decode the written .dmp and compare to the raw bytes")
    args = ap.parse_args(argv)

    w, h, tileset, tiles = load_layout(args.layout)
    os.makedirs(args.out_dir, exist_ok=True)
    base = args.basename
    tmx_path = os.path.join(args.out_dir, base + ".tmx")
    dmp_path = os.path.join(args.out_dir, base + "_data.dmp")
    event_path = os.path.join(args.out_dir, base + ".event")

    raw = raw_bytes(w, h, tiles)
    write_tmx(tmx_path, w, h, tileset, tiles, args.chapter, args.map_id)
    compress_to(raw, dmp_path)
    write_event(event_path, os.path.basename(args.layout), args.label, "0x%X" % args.map_id, base)

    print("wrote %s" % tmx_path)
    print("wrote %s" % dmp_path)
    print("wrote %s" % event_path)

    if args.check:
        with open(dmp_path, "rb") as f:
            packed = f.read()
        decoded = lz77_decode(packed)
        if decoded != raw:
            print("FAIL: decoded .dmp does not match raw bytes", file=sys.stderr)
            return 1
        print("OK %dx%d %d tiles" % (w, h, w * h))
    return 0


if __name__ == "__main__":
    sys.exit(main())
