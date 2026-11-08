# PortKiris district spec.
#   Generate: python3 Tools/district/tilegen/gen.py Tools/district/tilegen/specs/portkiris.py --seed 1
#   Check:    python3 Tools/district/tilegen/gen.py Tools/district/tilegen/specs/portkiris.py --seed 1 --check-layout CustomCampaign/Chapters/PortKiris/map/PortKiris_layout.json
#   PortKiris: 26x20. Letters used: s (water or sand/desert), w, f (ground or forest).
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
W, H = 26, 20
sk = [['.'] * W for _ in range(H)]
def fill(x, y, w, h, c):
    for j in range(y, y + h):
        for i in range(x, x + w):
            sk[j][i] = c
def S(mid, sx, sy, sw, sh, ddx, ddy, tx, ty):
    return (mid, sx, sy, sw, sh, tx - (ddx - sx), ty - (ddy - sy))
fill(12, 13, 14, 7, 's')
fill(22, 8, 4, 12, 's')         # and the east edge
fill(12, 0, 3, 13, 'w')         # road from the north gate into town
fill(0, 5, 22, 2, 'w')          # upper street
fill(0, 12, 22, 2, 'w')         # quay street
fill(0, 0, 3, 3, 'f'); fill(18, 0, 4, 3, 'f')
DOORS = {'armory': (4, 4), 'vendor': (9, 4), 'village1': (17, 4), 'hexshop': (19, 12),
         'village2': (16, 8), 'house': (8, 8)}
SPEC = {
    'out': 'kiris5',
    'seed': 1,
    'layout': 'CustomCampaign/Chapters/PortKiris/map/PortKiris_layout.json',
    'w': W, 'h': H, 'tileset': (0x0E, 0x0F, 0x10),
    'train': [0x11, 0x1f, 0x2b, 0x73, 0xd2, 0xeb],
    'stamps': [(0x73, 0, 10, 12, 10, 0, 10),
               S(0x1f, 1, 0, 3, 2, 2, 1, *DOORS['armory']),
               S(0x1f, 5, 9, 3, 2, 6, 10, *DOORS['vendor']),
               S(0x73, 4, 4, 5, 2, 5, 5, *DOORS['village1']),
               S(0x1f, 5, 9, 3, 2, 6, 10, *DOORS['hexshop']),
               S(0x1f, 11, 9, 4, 2, 12, 10, *DOORS['village2']),
               S(0x2b, 10, 14, 3, 2, 11, 15, *DOORS['house'])],
    'sketch': [''.join(r) for r in sk],
    'terrain_weight': {0x02: 4.0, 0x0C: 2.0},
    'start': (13, 0),
    'reach': list(DOORS.values()),
}
