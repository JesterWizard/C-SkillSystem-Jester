# district_map.py

Generates an FE8 map from a layout JSON: a Tiled `.tmx`, the compressed map data
`.dmp`, and a map `.event` that registers the data in the map pointer table.

Maps are designed as layout JSON and regenerated with this tool. Edit the layout
JSON, then rerun the tool. Do not hand-edit the `.tmx`, `.dmp` or `.event` output.

Command line (standard library only, Python 3):

    python3 Tools/district/district_map.py <layout.json> <out_dir> <basename> \
        --chapter 0x3B --map-id 0xCA --label MapData_Serafew [--check]

This writes `<basename>.tmx`, `<basename>_data.dmp` and `<basename>.event` into `<out_dir>`.
`--check` decodes the written `.dmp` and compares it with the raw bytes.

Layout JSON format:

    {"w": 24, "h": 18, "tileset": [14, 15, 16], "tiles": [w*h metatile indices, row-major]}

`tileset` is `[obj, palette, tileconfig]`. Each tile is a metatile index (0..1023).

The `.dmp` is `u8 w, u8 h`, then one little-endian u16 per tile (metatile index * 4),
compressed with `Tools/EventAssembler/Tools/compress` (LZ77 type 0x10).

## compose.py (installed district maps)

The installed district maps are now made with `compose.py` from the vanilla map of the same town:
Serafew = map 0x1F, Port Kiris = 0x2B, Caer Pelyn = 0x31, Jehanna = 0x45. The spec in `compose/<Name>.json`
crops the vanilla map, sets the edited cells, and writes the layout JSON.

    python3 Tools/district/compose.py Tools/district/compose/Serafew.json --write [--check] [--png PATH]

`--check` prints `MATCH` when the layout file equals the composed tiles, or `DIFF n` and exits 1. `--png` renders the
composed map with the vanilla preview tileset (Pillow needed).

Run order for an installed district map:

1. `compose.py <spec> --write` writes the layout JSON.
2. `hexshop_building.py` (below).
3. `district_map.py` (below) writes the `.tmx`, `.dmp` and `.event`.

## Generating a layout (tilegen)

`tilegen/` is kept as an experimental generator. Its specs no longer match the installed layouts, so do not use
it for the installed district maps. It makes the layout JSON from a short spec: it learns the tile joins from vanilla maps,
fills the natural terrain with Wave Function Collapse, pastes building stamps and checks the doors.
The specs in `tilegen/specs/` keep the seeds of the old generated layouts. Those layouts are no longer installed,
so `--check-layout` against the current layout files reports `DIFF`. The command below shows the check:

    python3 Tools/district/tilegen/gen.py Tools/district/tilegen/specs/serafew.py --seed 3 \
        --check-layout CustomCampaign/Chapters/Serafew/map/Serafew_layout.json

See `tilegen/README.md` for the algorithm, the sketch letters, the spec format, how to make a
new district map, and the limits.

## Hex shop building (hexshop_building.py)

`hexshop_building.py` replaces the Hex shop building (a vanilla house or hut in the Hex shop slot) with a drawn shop in
a copy of each district tileset. In the town tileset it is a market stall from a supplied illustration, read pixel for
pixel (33x25) and made three rows taller (one wall row above the windows, two through the door; 33x28): a blue hipped
roof, a cream stone wall with two green windows, a red-and-white striped awning and a wooden door. The small Caer Pelyn
and Jehanna version is a tower in one tile: a grey cone roof from above with a gold band over a short cream wall.
The art is in `hexshop_art/`: `stall_town.txt` (48x32, the 3x2 town block, on a
yard copied from the block's own left strip) and `tower_small.txt` (16x16, one metatile, for the caer and jeh tilesets,
on a two-colour ground). The colours of each set are in `SETS`. The town set also has a palette copy, `town_pal.bin`
(palette 0xEE, the vanilla 0x0F with bank 5 redefined to the stall's 12 colours and 3 yard colours), because the
stall's colours are not in the town palette; every 8x8 tile that holds stall pixels uses bank 5. The door metatile gets
terrain 0x03, so the door's VISIT location event runs when the player stands on it.

Per tileset copy it writes `<key>_obj.bin` (raw 4bpp tiles) and `<key>_cfg.bin` (raw tile config, 0x2400 bytes).
With `--write-layouts` it also rewrites the building cells of each district layout JSON to the new metatiles.
With `--png DIR` it saves each district with the new tiles (`<Name>.png`) and the building block at 8x (`<key>_block.png`).

| Key | Source tileset (obj / pal / cfg) | Districts (block top-left / door cell) | New obj | New cfg |
| --- | --- | --- | --- | --- |
| `town` | 0x0E / 0x0F / 0x10 | Serafew (4,18) / (5,19), PortKiris (10,14) / (11,15) | 0xEB | 0x8B |
| `caer` | 0x2E / 0x34 / 0x30 | CaerPelyn (13,8) / (13,8) | 0xEC | 0x8C |
| `jeh`  | 0x42 / 0x43 / 0x44 | Jehanna (17,6) / (17,6) | 0xED | 0x8D |

The palette id stays the vanilla one. The new ids are registered in `CustomCampaign/Chapters/_hexshop/HexShopTiles.event`
(`NewEventPointerTable_ObjPal` and `NewEventPointerTable_TileConfig`), which `Event_Installer.event` includes.

Run order for an installed district map:

1. `compose.py <spec> --write` writes the layout JSON (see above).
2. `python3 Tools/district/hexshop_building.py --chapters CustomCampaign/Chapters --out-dir CustomCampaign/Chapters/_hexshop --write-layouts`.
   Run it once, on the layout from step 1. A second run stops with `<Name>: cell (x,y) holds N, expected M`, because the
   tower cells are already in the layout. The script only knows the districts in its `SETS`. A new district needs an entry
   there, with its tileset, its building metatiles, its door cell and its art.
3. Compress each `.bin` to `.lz` with `Tools/EventAssembler/Tools/compress INPUT OUTPUT` (the script does not compress). There
   are six files: `<key>_obj.bin` and `<key>_cfg.bin` for `town`, `caer` and `jeh`. New copies also need their ids in
   `_hexshop/HexShopTiles.event`.
4. Run `district_map.py` (same arguments as before). The compose layout already names the new tileset ids. Do not edit the outputs.
