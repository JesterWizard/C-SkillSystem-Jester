#!/usr/bin/env python3
"""Compose a district layout JSON from a hand-made vanilla map.

District maps are cut from hand-made vanilla maps of the same town (the vanilla
map of that town is the base), then edited cell by cell. A spec names the base
map, the crop rectangle (x, y, w, h) inside it, the layout tileset, and the
metatile edits. This tool loads the base map from the clean fe8.gba, crops it,
sets each edited cell, and writes the layout JSON that district_map.py reads.

Spec JSON:
  {"name": "Serafew",
   "base": {"map": "0x1F", "x": 0, "y": 0, "w": 15, "h": 21},
   "tileset": [235, 15, 139],
   "preview_tileset": ["0x0E", "0x0F", "0x10"],
   "tiles": [[x, y, metatile], ...],          (x, y relative to the crop)
   "layout": "CustomCampaign/Chapters/Serafew/map/Serafew_layout.json"}

Integers may be JSON ints or strings such as "0x1F".

Command line (Python 3, standard library; Pillow only for --png):

  python3 Tools/district/compose.py SPEC.json [--write] [--check] [--png PATH]

  --write     write the composed layout to the spec's "layout" path
  --check     compare the composed layout with the existing layout file;
              prints MATCH and exits 0, or prints DIFF n and exits 1
              (n = differing tile cells, plus 1 if w, h or tileset differ)
  --png PATH  render the composed map with "preview_tileset" (scale 2)

Run order for an installed district map:

  1. compose.py SPEC.json --write
  2. hexshop_building.py (see Tools/district/README.md)
  3. district_map.py <layout.json> <out_dir> <basename> --chapter .. --map-id .. --label ..
"""

import argparse
import json
import os
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(REPO_ROOT / "Tools" / "district" / "tilegen"))


def to_int(v, what):
    if isinstance(v, bool):
        raise ValueError("%s: expected an integer, got %r" % (what, v))
    if isinstance(v, int):
        return v
    if isinstance(v, str):
        try:
            return int(v, 0)
        except ValueError:
            pass
    raise ValueError("%s: expected an integer, got %r" % (what, v))


def repo_path(p):
    path = Path(p)
    return path if path.is_absolute() else REPO_ROOT / path


def load_spec(path):
    with open(path, "r", encoding="utf-8") as f:
        spec = json.load(f)
    name = spec.get("name", os.path.basename(path))
    base = spec["base"]
    b = {k: to_int(base[k], "base." + k) for k in ("x", "y", "w", "h")}
    b["map"] = to_int(base["map"], "base.map")
    tileset = [to_int(v, "tileset") for v in spec["tileset"]]
    if len(tileset) != 3:
        raise ValueError("tileset must be [obj, palette, tileconfig]")
    preview = [to_int(v, "preview_tileset") for v in spec["preview_tileset"]]
    if len(preview) != 3:
        raise ValueError("preview_tileset must be [obj, palette, tileconfig]")
    edits = []
    for i, e in enumerate(spec.get("tiles", [])):
        if len(e) != 3:
            raise ValueError("tiles[%d]: expected [x, y, metatile]" % i)
        edits.append(tuple(to_int(v, "tiles[%d]" % i) for v in e))
    return {
        "name": name,
        "base": b,
        "tileset": tileset,
        "preview": preview,
        "edits": edits,
        "layout": spec["layout"],
    }


def compose(spec):
    import fe8map

    b = spec["base"]
    w, h, base_tiles = fe8map.load_map(b["map"])
    if b["w"] <= 0 or b["h"] <= 0:
        raise ValueError("base crop must be non-empty")
    if b["x"] < 0 or b["y"] < 0 or b["x"] + b["w"] > w or b["y"] + b["h"] > h:
        raise ValueError(
            "%s: crop (%d,%d,%d,%d) is outside base map 0x%X (%dx%d)"
            % (spec["name"], b["x"], b["y"], b["w"], b["h"], b["map"], w, h)
        )
    tiles = []
    for y in range(b["h"]):
        row = (y + b["y"]) * w + b["x"]
        tiles.extend(base_tiles[row:row + b["w"]])
    for x, y, m in spec["edits"]:
        if not (0 <= x < b["w"] and 0 <= y < b["h"]):
            raise ValueError(
                "%s: cell (%d,%d) is outside the %dx%d crop"
                % (spec["name"], x, y, b["w"], b["h"])
            )
        if not (0 <= m < 1024):
            raise ValueError("%s: metatile %d out of range" % (spec["name"], m))
        tiles[y * b["w"] + x] = m
    return b["w"], b["h"], tiles


def layout_text(w, h, tileset, tiles):
    # Same key order and separators as the existing layout files (no trailing newline).
    return json.dumps({"w": w, "h": h, "tileset": tileset, "tiles": tiles})


def check(spec, w, h, tiles):
    path = repo_path(spec["layout"])
    with open(path, "r", encoding="utf-8") as f:
        old = json.load(f)
    diff = 0
    if old.get("w") != w or old.get("h") != h:
        diff += 1
    if list(old.get("tileset", [])) != spec["tileset"]:
        diff += 1
    old_tiles = old.get("tiles", [])
    for i in range(max(len(old_tiles), len(tiles))):
        a = old_tiles[i] if i < len(old_tiles) else None
        b = tiles[i] if i < len(tiles) else None
        if a != b:
            diff += 1
    if diff == 0:
        print("MATCH")
        return 0
    print("DIFF %d" % diff)
    return 1


def main(argv=None):
    ap = argparse.ArgumentParser(description="Compose a district layout JSON from a vanilla map")
    ap.add_argument("spec")
    ap.add_argument("--write", action="store_true", help="write the layout to the spec's layout path")
    ap.add_argument("--check", action="store_true", help="compare with the existing layout file")
    ap.add_argument("--png", metavar="PATH", help="render the composed map as PNG (needs Pillow)")
    args = ap.parse_args(argv)

    spec = load_spec(args.spec)
    w, h, tiles = compose(spec)
    print("%s: %dx%d, %d tiles" % (spec["name"], w, h, len(tiles)))

    rc = 0
    if args.write:
        out = repo_path(spec["layout"])
        os.makedirs(out.parent, exist_ok=True)
        with open(out, "w", encoding="utf-8", newline="\n") as f:
            f.write(layout_text(w, h, spec["tileset"], tiles))
        print("wrote %s" % out)
    if args.check:
        rc = check(spec, w, h, tiles)
    if args.png:
        import fe8map

        ts = fe8map.tileset(*spec["preview"])
        fe8map.render_grid(w, h, tiles, ts, args.png, scale=2)
        print("wrote %s" % args.png)
    return rc


if __name__ == "__main__":
    try:
        sys.exit(main())
    except (ValueError, KeyError, OSError) as e:
        print("error: %s" % e, file=sys.stderr)
        sys.exit(2)
