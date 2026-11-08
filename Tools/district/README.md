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

## Generating a layout (tilegen)

`tilegen/` makes the layout JSON from a short spec: it learns the tile joins from vanilla maps,
fills the natural terrain with Wave Function Collapse, pastes building stamps and checks the doors.
Each district's approved seed and installed layout are in `tilegen/specs/`. Check that a spec still
reproduces its layout:

    python3 Tools/district/tilegen/gen.py Tools/district/tilegen/specs/serafew.py --seed 3 \
        --check-layout CustomCampaign/Chapters/Serafew/map/Serafew_layout.json

See `tilegen/README.md` for the algorithm, the sketch letters, the spec format, how to make a
new district map, and the limits.

## Hex shop building (hexshop_building.py)

`hexshop_building.py` restyles the Hex shop building (a vanilla house or hut in the Hex shop slot) in a copy of
each district tileset: a slate roof and a small gold-rimmed Hex card sign. The door metatile gets terrain 0x03, so
the door's VISIT location event runs when the player stands on it.

Per tileset copy it writes `<key>_obj.bin` (raw 4bpp tiles) and `<key>_cfg.bin` (raw tile config, 0x2400 bytes).
With `--write-layouts` it also rewrites the building cells of each district layout JSON to the new metatiles.

| Key | Source tileset (obj / pal / cfg) | Districts | New obj | New cfg |
| --- | --- | --- | --- | --- |
| `town` | 0x0E / 0x0F / 0x10 | Serafew, PortKiris | 0xEB | 0x8B |
| `caer` | 0x2E / 0x34 / 0x30 | CaerPelyn | 0xEC | 0x8C |
| `jeh`  | 0x42 / 0x43 / 0x44 | Jehanna | 0xED | 0x8D |

The palette id stays the vanilla one. The new ids are registered in `CustomCampaign/Chapters/_hexshop/HexShopTiles.event`
(`NewEventPointerTable_ObjPal` and `NewEventPointerTable_TileConfig`), which `Event_Installer.event` includes.

Run order for a district map:

1. `tilegen/gen.py` writes the layout JSON (see above). Run `tilegen/seams.py` on the result now, before step 2. After step 2,
   `seams.py` cannot read the layout's tileset from `fe8.gba` (see the tilegen README).
2. `python3 Tools/district/hexshop_building.py --chapters CustomCampaign/Chapters --out-dir CustomCampaign/Chapters/_hexshop --write-layouts`.
   Run it once, on the layout from step 1. A second run stops with `<Name>: cell (x,y) holds N, expected M`, because the shop
   cells are already in the layout. The script only knows the districts in its `SETS`. A new district needs an entry there,
   with its tileset, its building metatiles and its door cell.
3. Compress each `.bin` to `.lz` with `Tools/EventAssembler/Tools/compress INPUT OUTPUT` (the script does not compress). There
   are six files: `<key>_obj.bin` and `<key>_cfg.bin` for `town`, `caer` and `jeh`. New copies also need their ids in
   `_hexshop/HexShopTiles.event`.
4. Set the layout `tileset` to the new ids, then `district_map.py` (same arguments as before). Do not edit the outputs.
