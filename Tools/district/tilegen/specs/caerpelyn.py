# CaerPelyn district spec.
#   Generate: python3 Tools/district/tilegen/gen.py Tools/district/tilegen/specs/caerpelyn.py --seed 0
#   Check:    python3 Tools/district/tilegen/gen.py Tools/district/tilegen/specs/caerpelyn.py --seed 0 --check-layout CustomCampaign/Chapters/CaerPelyn/map/CaerPelyn_layout.json
#   CaerPelyn: 22x18. Letters used: . m (mountain/peak, terrain 0x11/0x12) w.
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
W, H = 22, 18
sk = [['.'] * W for _ in range(H)]
def fill(x, y, w, h, c):
    for j in range(y, y + h):
        for i in range(x, x + w):
            sk[j][i] = c
def S(mid, sx, sy, sw, sh, ddx, ddy, tx, ty):
    return (mid, sx, sy, sw, sh, tx - (ddx - sx), ty - (ddy - sy))
fill(0, 0, 7, 2, 'm')           # northern ridge (west)
fill(17, 0, 5, 2, 'm')          # northern ridge (east)
fill(18, 15, 4, 3, 'm')         # south-east peaks
fill(9, 9, 4, 9, 'w')           # valley floor from the village down to the gate
fill(12, 12, 6, 3, 'w')         # meadow east of the gate road
fill(15, 5, 6, 5, 'w')          # east meadow with the houses and the shop
DOORS = {'hexshop': (18, 8), 'house1': (16, 6), 'house2': (20, 6)}
SPEC = {
    'out': 'caer5',
    'seed': 0,
    'layout': 'CustomCampaign/Chapters/CaerPelyn/map/CaerPelyn_layout.json',
    'w': W, 'h': H, 'tileset': (0x2E, 0x34, 0x30),
    'train': [0x31, 0x35, 0xd8],
    'stamps': [(0x31, 9, 4, 7, 7, 8, 1),            # Ch12 village with its stairs
               (0x31, 1, 13, 9, 8, 0, 9),           # Ch12 fort
               S(0x35, 17, 6, 1, 1, 17, 6, *DOORS['hexshop']),
               S(0x35, 6, 10, 1, 1, 6, 10, *DOORS['house1']),
               S(0x35, 5, 12, 1, 1, 5, 12, *DOORS['house2'])],
    'sketch': [''.join(r) for r in sk],
    'terrain_weight': {0x11: 0.6, 0x12: 0.6},
    'start': (10, 17),
    'reach': [(11, 7), (4, 15), (16, 6), (20, 6), (18, 8), (14, 13), (3, 6)],
}
