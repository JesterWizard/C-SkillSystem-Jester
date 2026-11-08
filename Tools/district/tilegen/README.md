# tilegen

Generates district map layouts (metatile grids) from a short spec, so that the result joins
the vanilla tiles the way a hand-made map would. Output is layout JSON, which
`Tools/district/district_map.py` turns into the map data.

Needs Python 3, numpy and Pillow. Reads `fe8.gba` and `CustomCampaign/Event/PListBins/*.bin`
from the repo root (the ROM must be the clean build ROM). Run everything from the repo root.

## Files

| File | What it does |
| --- | --- |
| `fe8map.py` | Reads the ROM: LZ77 decode, map and tileset lookup from the plist bins, metatile rendering to PNG. |
| `wfc.py` | Learns adjacency from vanilla maps (`train`) and runs the bitset Wave Function Collapse (`WFC`). |
| `pixedge.py` | Pixel-edge test: adds neighbour pairs whose shared edge is as smooth as a vanilla join (`extend_model`). |
| `rules.py` | Terrain classes (`WALK`, `NATURAL`, `CLASSES`), sketch-to-domain rules (`build_allowed`), water detection, reachability (`connected`). |
| `repair.py` | `patch_repair`: re-solves small patches around bad joins. `total_conflicts` counts bad joins. |
| `gen.py` | CLI. Runs seeds, pastes stamps, repairs, checks doors, writes outputs, `--write-layout`, `--check-layout`. |
| `seams.py` | CLI. Counts the neighbour pairs of a finished layout that the learned model rejects. `--png` draws them in red. |
| `specs/*.py` | One spec per district (`serafew`, `portkiris`, `caerpelyn`, `jehanna`). Each has `SPEC`, the approved `seed` and the installed `layout`. |
| `specs/build/` | Default output of `gen.py` (not tracked; it holds its own `.gitignore`). |

## Commands

Generate and compare with the installed layout (the approved seed):

    python3 Tools/district/tilegen/gen.py Tools/district/tilegen/specs/serafew.py --seed 3 \
        --check-layout CustomCampaign/Chapters/Serafew/map/Serafew_layout.json

Prints `MATCH` (exit 0) or `DIFF n tiles` (exit 1). Without `--seed`, `gen.py` tries seeds
`0..N-1` (`--seeds N`, default 4) and ranks them.

Options: `--seed S` (only seed S), `--seeds N`, `--out-dir DIR` (default `specs/build/`),
`--write-layout PATH` (best result as `{"w", "h", "tileset", "tiles"}`).

Count the rejected joins of a layout, optionally with a picture. Use the `gen.py` output, which has not been through
`hexshop_building.py`:

    python3 Tools/district/tilegen/seams.py Tools/district/tilegen/specs/build/ser5_0.json \
        0x11 0x1f 0x2b 0x73 0xd2 0xeb --png ser_seams.png

The ids after the layout are the vanilla maps to learn from (use the spec's `train` list). After `hexshop_building.py`, a
layout uses the Hex shop tileset ids (`0xEB` to `0xED`). `fe8.gba` does not have them, so `seams.py` stops with an
`IndexError` in `fe8map.lz77` when it is given an installed layout.

Outputs of `gen.py` for each of the best three seeds: `<out>_<k>.json` (debug data with seed and
conflicts), `<out>_<k>.png` (grid overlay, 5-cell lines and cell coordinates) and
`<out>_<k>_clean.png` (the plain render).

## Algorithm

1. **Learn** (`wfc.train`). Every horizontal and vertical neighbour pair in the vanilla maps
   listed in `train` becomes allowed. The 2x2 windows are recorded too, for scoring. The maps
   must use the spec's tileset.
2. **Extend** (`pixedge.extend_model`). A pair not seen in vanilla is added if its shared
   16-pixel edge is no more than `thresh` (default 3.0) rougher than the tiles' own texture.
   Such added pairs never join water to land (`rules.water_tiles`).
3. **Domains** (`rules.build_allowed`). Each sketch cell gets the set of metatiles its letter
   allows (table below).
4. **Base WFC** (`WFC`). Collapses the whole map with those domains. Cell choice takes the
   lowest-entropy cell; the value is drawn with weight = vanilla frequency times the
   spec's `terrain_weight`. Contradictions are undone by backtracking (3000 steps).
5. **Stamps.** Building stamps cut from vanilla maps and the spec's `tiles` are pasted and
   fixed. The `buildings` key can cut a building around a door automatically.
6. **Patch repair** (`repair.patch_repair`). For each bad joint, re-solve a patch of radius
   1 to 5 with the rest fixed, up to 400 rounds.
7. **Check** (`rules.connected`). Flood fill from `start` over walkable terrain. Every cell in
   `reach` (the doors) must be reached.
8. **Rank.** Seeds are sorted by (unreachable doors, conflicts). The ranking is deterministic:
   the same seed and spec give the same tiles.

## Sketch letters

Each spec has a `sketch` of rows, one letter per cell. `.` is the default.

| Letter | Allowed metatiles |
| --- | --- |
| `.` | natural terrain other than sand/desert, no water; water only within 3 cells of an `s` |
| `w` | walkable natural terrain (`WALK` and `NATURAL`), no water |
| `g` | terrain 0x01 |
| `r` | terrain 0x02 (road) |
| `f` | terrain 0x01 or 0x0C |
| `F` | terrain 0x0C |
| `d` | terrain 0x0E or 0x0F |
| `x` | terrain 0x0F |
| `m` | terrain 0x11 or 0x12 (mountain/peak) |
| `s` | water, or terrain 0x10, 0x15, 0x16 (sand/desert) |

## Spec format

    SPEC = {
        'out': 'ser5',                  # file prefix in the output folder
        'seed': 3,                      # approved seed (used by the check, not by the code)
        'layout': 'CustomCampaign/.../Serafew_layout.json',   # installed layout (repo path)
        'w': 24, 'h': 18,
        'tileset': (0x0E, 0x0F, 0x10),  # obj palette, palette, tile config (PListBins indices)
        'train': [0x11, 0x1f, ...],     # vanilla map ids to learn from
        'stamps': [(map, sx, sy, sw, sh, dx, dy), ...],   # or S(...) from a door, see below
        'sketch': [...],                # h strings of w letters
        'terrain_weight': {0x02: 6.0},  # WFC odds per terrain id (default 1.0)
        'start': (11, 17),              # gate cell
        'reach': [(3, 3), ...],         # door cells that must be reachable
        'tiles': {(x, y): metatile},    # optional fixed cells
        'buildings': [(map, ddx, ddy, tx, ty)],  # optional: auto-cut building around a door
        'thresh': 3.0,                  # optional pixel-edge threshold
    }

A stamp `(map, sx, sy, sw, sh, dx, dy)` copies the vanilla rectangle at `(sx, sy)` of size
`sw x sh` to `(dx, dy)`. The spec helper `S(map, sx, sy, sw, sh, ddx, ddy, tx, ty)` writes the
same stamp from a door: the door `(ddx, ddy)` in the source map lands on `(tx, ty)`.

## Making a new district map

1. Write `specs/<name>.py` with a `SPEC` dict. Pick the vanilla maps of the same tileset for
   `train`, and the building stamps from maps that show the building you want. Sketch the
   streets (`w`), the water (`s`) and the mountains (`m`), and list the doors in `reach`.
2. Run `gen.py` with a few seeds (`--seeds 8`). Open `build/<out>_0.png` and the other two.
   Pick a seed by eye and by the printed `unreachable` and `conflicts` numbers.
3. Run `seams.py` on the best `<out>_<k>.json` to see the rejected joins. Use `--png` and look
   at the red lines. Edit the spec or the sketch and go back to 2 if the joins are bad.
4. `gen.py ... --seed S --write-layout <layout path>` writes the layout JSON. Set `layout` in
   the spec to the same path.
5. Add the Hex shop building and compress its copies, as in [Tools/district/README.md](../README.md) (Hex shop building).
   Do this on this layout, before step 6.
6. Set the layout `tileset` to the Hex shop ids. Then build the map with `Tools/district/district_map.py <layout.json> <out_dir> <basename> --chapter 0x.. --map-id 0x.. --label ...`.
   Do not edit its output by hand.
7. Check the result again with `gen.py ... --check-layout <layout path>`. The Hex shop cells show as `DIFF`. That is expected.

## Limits

- **Small closed ponds can't be generated.** In the vanilla maps water only touches the map
  edges, so no learned pair puts water inside a closed shape. A pond pinned with the `tiles`
  key will have seams at its edge, or draw it by hand.
- **Stamps cut from a source map's edge** have no neighbours from the source on the side that
  touches the map edge, because the source has no tile there to learn from. The WFC can join
  that side only through the other training maps or the pixel-edge rule. Expect seams there.
  `seams.py` shows them.
- **Jehanna's tileset learns from one vanilla map only** (`train: [0x45]`), so the learned
  joins are those of that map plus the pixel-edge rule. Its 0 rejected pairs hold for this
  layout, but the variety is limited to what that map shows.
- **The installed CaerPelyn layout has one unreachable reach cell, (11, 7)** (`gen.py` prints
  `unreachable [(11, 7)]` for seed 0). It is kept as approved. A new seed or sketch change may
  fix it.
- Before the Hex shop step, Serafew, PortKiris and CaerPelyn keep 16, 17 and 16 rejected pairs, and Jehanna keeps 0. These
  equal the `conflicts` that `gen.py` reports for the approved seeds: the joins that patch repair could not fix with the
  learned pairs.
- The Hex shop cells add more rejected pairs, because their metatiles are not in the learned model.

The installed layouts differ from the spec output in the Hex shop cells: `hexshop_building.py --write-layouts` rewrites 1 to 4 building cells per district, so `--check-layout` reports `DIFF` for those cells.
