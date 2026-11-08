# Serafew district spec.
#   Generate: python3 Tools/district/tilegen/gen.py Tools/district/tilegen/specs/serafew.py --seed 3
#   Check:    python3 Tools/district/tilegen/gen.py Tools/district/tilegen/specs/serafew.py --seed 3 --check-layout CustomCampaign/Chapters/Serafew/map/Serafew_layout.json
#   Serafew: 24x18. Letters used: . w F (F = terrain 0x0C only).
#   Sketch letters (one per cell, see rules.build_allowed):
#     .  open ground: natural terrain other than sand/desert; water only within 3 cells of an 's'
#     w  walkable natural terrain (road, ground, forest...), no water
#   Stamps are tuples (map id, src x, src y, w, h, dst x, dst y): the src rectangle of vanilla map
#   <map id> is pasted with its top-left corner at (dst x, dst y) and is fixed during WFC.
#   S(mid, sx, sy, sw, sh, ddx, ddy, tx, ty) is the same kind of stamp, written from the door: the
#   door at (ddx, ddy) in map mid lands on the target cell (tx, ty). DOORS names the target cells.
#   Keys of SPEC: 'out' = file prefix for build/; 'train' = vanilla map ids the metatile model is
#   learned from; 'terrain_weight' = WFC odds per terrain id; 'start' = gate cell the reachability
#   flood starts from; 'reach' = cells that must be reachable (the doors); 'seed' = approved seed;
#   'layout' = installed layout JSON (repo path), the target of --write-layout / --check-layout.
#
W, H = 24, 18
sk = [['.'] * W for _ in range(H)]
def fill(x, y, w, h, c):
    for j in range(y, y + h):
        for i in range(x, x + w):
            sk[j][i] = c
def S(mid, sx, sy, sw, sh, ddx, ddy, tx, ty):
    """stamp rect (sx,sy,sw,sh) of map mid so that its door (ddx,ddy) lands on (tx,ty)"""
    return (mid, sx, sy, sw, sh, tx - (ddx - sx), ty - (ddy - sy))
fill(0, 4, W, 2, 'w')          # main street under the top row of buildings
fill(10, 4, 3, 14, 'w')        # street down to the gate
fill(0, 11, W, 2, 'w')         # middle street
fill(0, 7, 3, 3, 'F'); fill(21, 13, 3, 3, 'F'); fill(13, 0, 2, 2, 'F')

DOORS = {'armory': (3, 3), 'vendor': (8, 3), 'village1': (15, 3), 'arena': (21, 3),
         'village2': (4, 10), 'hexshop': (19, 10), 'house': (8, 15), 'village3': (16, 15)}
SPEC = {
    'out': 'ser5',
    'seed': 3,
    'layout': 'CustomCampaign/Chapters/Serafew/map/Serafew_layout.json',
    'w': W, 'h': H, 'tileset': (0x0E, 0x0F, 0x10),
    'train': [0x11, 0x1f, 0x2b, 0x73, 0xd2, 0xeb],
    'stamps': [S(0x1f, 1, 0, 3, 2, 2, 1, *DOORS['armory']),
               S(0x1f, 5, 9, 3, 2, 6, 10, *DOORS['vendor']),
               S(0x1f, 4, 0, 5, 2, 5, 1, *DOORS['village1']),
               S(0x1f, 11, 5, 3, 2, 12, 6, *DOORS['arena']),
               S(0x1f, 11, 9, 4, 2, 12, 10, *DOORS['village2']),
               S(0x1f, 5, 9, 3, 2, 6, 10, *DOORS['hexshop']),
               S(0x1f, 9, 18, 5, 2, 12, 19, *DOORS['house']),
               S(0x1f, 4, 5, 5, 2, 5, 6, *DOORS['village3'])],
    'sketch': [''.join(r) for r in sk],
    'terrain_weight': {0x01: 1.0, 0x02: 6.0, 0x0C: 3.0},
    'start': (11, 17),
    'reach': list(DOORS.values()),
}
