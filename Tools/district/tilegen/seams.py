#!/usr/bin/env python3
"""Count the neighbour pairs of a finished layout that the learned model rejects.

Usage:
  python3 Tools/district/tilegen/seams.py <layout.json> <train map ids...> [--png OUT]

The model is learned from the given vanilla maps (the layout's tileset), then extended with the
pixel-edge rule at thresh 3.0, the same way gen.py builds it. Every horizontal and vertical pair
the model rejects is counted. With --png OUT, a red line is drawn on the shared edge of each
rejected pair and the picture is written to OUT (nothing is written otherwise).

Example (Serafew, trained on its own vanilla source maps):
  python3 Tools/district/tilegen/seams.py CustomCampaign/Chapters/Serafew/map/Serafew_layout.json \
      0x11 0x1f 0x2b 0x73 0xd2 0xeb --png ser_seams.png
"""

import argparse
import json

from PIL import Image, ImageDraw

from fe8map import render, tileset
from pixedge import extend_model
from rules import water_tiles
from wfc import train


def rejected_pairs(w, h, t, model):
    """Yield (x, y, direction) for each neighbour pair not in the model. direction 0 = right, 2 = down."""
    order, idx, comp, pats, freq = model
    for y in range(h):
        for x in range(w):
            a = idx.get(t[y * w + x])
            for d, (dx, dy) in ((0, (1, 0)), (2, (0, 1))):
                nx, ny = x + dx, y + dy
                if nx < w and ny < h:
                    b = idx.get(t[ny * w + nx])
                    if a is None or b is None or not (comp[d][a] >> b) & 1:
                        yield x, y, d


def main(argv=None):
    ap = argparse.ArgumentParser(description="Count neighbour pairs in a layout that the learned model rejects")
    ap.add_argument("layout")
    ap.add_argument("train", nargs="+", type=lambda s: int(s, 0), help="vanilla map ids to learn from")
    ap.add_argument("--png", metavar="OUT", help="write the layout with red lines on rejected edges")
    args = ap.parse_args(argv)

    with open(args.layout, "r", encoding="utf-8") as f:
        lay = json.load(f)
    w, h, t = lay["w"], lay["h"], lay["tiles"]
    ts = tileset(*lay["tileset"])
    model = train(args.train)
    extend_model(model, ts, thresh=3.0, same_terrain_only=False, exclude=water_tiles(model[0], ts))

    bad = list(rejected_pairs(w, h, t, model))
    if args.png:
        render(w, h, t, ts, args.png, 2)
        im = Image.open(args.png)
        dr = ImageDraw.Draw(im)
        for x, y, d in bad:
            if d == 0:
                nx = x + 1
                dr.line([(nx * 32, y * 32), (nx * 32, y * 32 + 31)], fill=(255, 0, 0), width=3)
            else:
                ny = y + 1
                dr.line([(x * 32, ny * 32), (x * 32 + 31, ny * 32)], fill=(255, 0, 0), width=3)
        im.save(args.png)
    print(args.layout, "rejected pairs:", len(bad))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
