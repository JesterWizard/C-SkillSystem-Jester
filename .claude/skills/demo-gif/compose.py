"""Compose recorded scenes into one captioned demo GIF.

    python3 compose.py WORK config.json

config.json (paths relative to WORK; rects and regions in GBA pixels, 240x160):
{
  "title": "ACCESSORY SLOT",
  "scenes": ["s1", "s2", "s3"],
  "out": "demo.gif",
  "stride": 2,          # keep every Nth recorded frame
  "duration": 40,       # ms per GIF frame
  "boxes": {
    "slot5": {"rect": [102, 88, 234, 105],
              "only_if": {"ref": "s1/f0050.png", "region": [108, 118, 112, 166], "max_diff": 8}}
  }
}
A box is drawn on frames whose caption names it. With only_if, only when the frame's
region [y0, y1, x0, x1] matches the same region of the ref frame (mean difference
below max_diff), so the box goes away when the screen it marks closes.
Also writes contact.png (30 frames spread over the GIF) for checking.
"""
import json
import os
import sys

import numpy as np
from PIL import Image, ImageDraw, ImageFont

FONT = '/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf'
FONT_B = '/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf'
W, H, BAR = 480, 320, 70
GOLD, WHITE, BG = (255, 214, 0), (255, 255, 255), (24, 28, 40)


def load(path):
    return np.asarray(Image.open(path).convert('RGB')).astype(int)


def main():
    work, cfg = sys.argv[1], json.load(open(sys.argv[2]))
    font, font_b = ImageFont.truetype(FONT, 17), ImageFont.truetype(FONT_B, 15)
    boxes = cfg.get('boxes', {})
    refs = {}
    for name, b in boxes.items():
        c = b.get('only_if')
        if c:
            y0, y1, x0, x1 = c['region']
            refs[name] = load(os.path.join(work, c['ref']))[y0:y1, x0:x1]
    stride = cfg.get('stride', 2)
    frames = []
    for scene in cfg['scenes']:
        caps = json.load(open(os.path.join(work, scene, 'caps.json')))
        for i, (cap, box) in enumerate(caps):
            if i % stride:
                continue
            path = os.path.join(work, scene, 'f%04d.png' % i)
            im = Image.open(path).convert('RGB')
            out = Image.new('RGB', (W, H + BAR), BG)
            out.paste(im.resize((W, H), Image.NEAREST), (0, 0))
            d = ImageDraw.Draw(out)
            if box is True:  # old caps.json format
                box = next(iter(boxes), None)
            if box and box in boxes:
                show = True
                if box in refs:
                    y0, y1, x0, x1 = boxes[box]['only_if']['region']
                    diff = np.abs(np.asarray(im).astype(int)[y0:y1, x0:x1] - refs[box]).mean()
                    show = diff < boxes[box]['only_if'].get('max_diff', 8)
                if show:
                    x0, y0, x1, y1 = boxes[box]['rect']
                    d.rectangle([2 * x0, 2 * y0, 2 * x1, 2 * y1], outline=GOLD, width=3)
            d.text((10, H + 6), cfg['title'], font=font_b, fill=GOLD)
            for k, line in enumerate(cap.split('|')):
                d.text((10, H + 25 + k * 21), line, font=font, fill=WHITE)
            frames.append(out)
    pal = [f.quantize(colors=255, method=Image.MEDIANCUT, dither=Image.Dither.NONE) for f in frames]
    out_path = os.path.join(work, cfg.get('out', 'demo.gif'))
    pal[0].save(out_path, save_all=True, append_images=pal[1:], duration=cfg.get('duration', 40),
                loop=0, optimize=True, disposal=1)
    n = len(frames)
    idx = list(range(0, n, max(1, n // 30)))[:30]
    sheet = Image.new('RGB', (240 * 6, 195 * 5))
    for k, i in enumerate(idx):
        sheet.paste(frames[i].resize((240, 195)), ((k % 6) * 240, (k // 6) * 195))
    sheet.save(os.path.join(work, 'contact.png'))
    print('%s: %d frames, %d KB (contact.png written)' % (out_path, n, os.path.getsize(out_path) // 1024))


if __name__ == '__main__':
    main()
