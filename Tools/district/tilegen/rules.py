# Terrain classes, which metatiles are allowed in each sketch cell, and the reachability check.
from collections import deque

from fe8map import terrain
from pixedge import meta_pixels
from wfc import DIRS

# Terrain ids (the tile-configs terrain byte, see fe8map.terrain). WALK: walkable for the reachability check.
# NATURAL: terrain the base WFC may fill with.
WALK = {0x08, 0x01, 0x02, 0x03, 0x05, 0x06, 0x07, 0x0C, 0x0E, 0x0F, 0x13, 0x14, 0x17, 0x18, 0x2D, 0x0A, 0x0B, 0x1E, 0x38, 0x25, 0x26}
NATURAL = {0x01, 0x02, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x15, 0x16}
CLASSES = {
    'g': {0x01},                 # plain ground
    'r': {0x02},                 # road
    'f': {0x01, 0x0C},           # ground or forest
    'w': None,                   # walkable (filled from WALK)
    'm': {0x11, 0x12},           # mountain / peak
    'd': {0x0E, 0x0F},
    'F': {0x0C},
    's': {0x10, 0x15, 0x16},           # sand / desert
    'x': {0x0F},                 # desert only
}


def water_tiles(order, ts):
    """Metatiles whose pixels are mostly blue (water)."""
    out = set()
    for v in order:
        p = meta_pixels(ts, v)
        r, g, b = p[..., 0], p[..., 1], p[..., 2]
        blue = ((r < 70) & (b > 170)).mean()
        if blue > 0.15:
            out.add(v)
    return out


def build_allowed(model, ts, sketch, w, h):
    order = model[0]
    terr = {v: (terrain(ts, v) or 0) for v in order}
    wet = water_tiles(order, ts)
    allowed = []
    for y in range(h):
        for x in range(w):
            c = sketch[y][x]
            near_water = any(0 <= y + dy < h and 0 <= x + dx < w and sketch[y + dy][x + dx] == 's'
                             for dx in range(-3, 4) for dy in range(-3, 4))
            if c == '.':
                allowed.append({v for v in order if (terr[v] in NATURAL - {0x10, 0x15, 0x16} and v not in wet) or (near_water and v in wet)})
                continue
            if c == 's':
                allowed.append({v for v in order if v in wet or terr[v] in CLASSES['s']})
                continue
            cls = (WALK & NATURAL) if c == 'w' else CLASSES[c]
            allowed.append({v for v in order if terr[v] in cls and v not in wet})
    return allowed, terr


def connected(w, h, t, terr_of, start, goals):
    """Flood-fill walkable tiles from start; return the goals that the fill does not reach."""
    seen = {start}
    q = deque([start])
    while q:
        x, y = q.popleft()
        for dx, dy in DIRS:
            n = (x + dx, y + dy)
            if 0 <= n[0] < w and 0 <= n[1] < h and n not in seen and terr_of(t[n[1] * w + n[0]]) in WALK:
                seen.add(n)
                q.append(n)
    return [g for g in goals if g not in seen]
