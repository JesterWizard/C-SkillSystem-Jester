# Jehanna district spec.
#   Generate: python3 Tools/district/tilegen/gen.py Tools/district/tilegen/specs/jehanna.py --seed 0
#   Check:    python3 Tools/district/tilegen/gen.py Tools/district/tilegen/specs/jehanna.py --seed 0 --check-layout CustomCampaign/Chapters/Jehanna/map/Jehanna_layout.json
#   Jehanna: 22x16. Letters used: . w. Its tileset (66, 67, 68) is learned from map 0x45 alone.
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
W, H = 22, 16
sk = [['.'] * W for _ in range(H)]
def fill(x, y, w, h, c):
    for j in range(y, y + h):
        for i in range(x, x + w):
            sk[j][i] = c
def S(mid, sx, sy, sw, sh, ddx, ddy, tx, ty):
    return (mid, sx, sy, sw, sh, tx - (ddx - sx), ty - (ddy - sy))
fill(9, 6, 4, 10, 'w')          # road from the gate up to the oasis
fill(2, 7, 18, 2, 'w')          # cross street
DOORS = {'vendor': (8, 4), 'village': (1, 12), 'hexshop': (15, 10), 'house': (5, 11)}
SPEC = {
    'out': 'jeh2',
    'seed': 0,
    'layout': 'CustomCampaign/Chapters/Jehanna/map/Jehanna_layout.json',
    'w': W, 'h': H, 'tileset': (0x42, 0x43, 0x44),
    'train': [0x45],
    'stamps': [(0x45, 6, 0, 9, 6, 6, 0),             # the oasis and the hall front
               S(0x45, 0, 12, 3, 3, 1, 14, *DOORS['village']),
               (0x45, 19, 15, 3, 3, 18, 11),         # walled house block
               S(0x45, 17, 6, 1, 1, 17, 6, *DOORS['hexshop']),
               S(0x45, 5, 13, 1, 1, 5, 13, *DOORS['house'])],
    'sketch': [''.join(r) for r in sk],
    'terrain_weight': {0x01: 0.15, 0x0F: 3.0, 0x0E: 2.0},
    'start': (11, 15),
    'reach': list(DOORS.values()),
}
