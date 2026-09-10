# World Map Thought Bubbles

<p align="center">
  <img src="./World_Map_Thoughts.gif" alt="Timer Demo" width="600"/>
</p>


Shows chapter-specific thought bubbles on the world map when a supported unit opens a node menu. Press **R** on the world map to cycle the active unit through the prep roster. Press **A** on the node the active unit currently occupies to center the camera there.

Bubbles are one **128x64** PNG per unit and chapter, drawn as two 64x64 sprites. Units without an entry for the current chapter show no bubble.

## Target ROM

- **FE8U (USA)** clean ROM
- Free space: `$1000000`

## Build

No build step is required to install the patch. `Source/WorldMapThoughts.lyn.event` and the `.dmp` graphics are checked in, so you can install `Installer.event` as-is.

Run `make` only if you edit `Source/WorldMapThoughts.c`. Run `make gfx` if you edit a PNG:

```bash
make -C Standalone/world_map_thought_bubbles
make -C Standalone/world_map_thought_bubbles gfx
```

That requires [devkitARM](https://devkitpro.org/wiki/Getting_Started) and this repo's [FE-CLib](https://github.com/MokhaLeee/FE-CLib-Mokha) / [Event Assembler](https://github.com/MokhaLeee/EventAssembler/tree/mokha-fix) tools. See [Documentation/Setup.md](../../Documentation/Setup.md) for full setup.

## Installation

This folder is self-contained. It does not include `EAstdlib.event`, `Hack Installation.txt`, or a Png2Dmp step. FEBuilder’s bundled Event Assembler is enough.

### FEBuilderGBA

1. Copy the `world_map_thought_bubbles` folder (or download this standalone package).
2. Open a clean FE8U ROM in FEBuilder.
3. Go to **Advanced Editors → Insert EA**.
4. Click **Select File**, choose `Installer.event`, then click **Load Script**.

### Event Assembler

From a copy of clean `fe8.gba`, with this folder as the working directory:

```bash
ColorzCore A FE8 -input:Installer.event -output:/path/to/fe8-world-map-thoughts.gba
```

## Files

| File | Purpose |
|------|---------|
| `Installer.event` | EA installer, addresses, hooks, and free-space placement |
| `Gfx.event` | `#incbin` of preconverted bubble graphics |
| `Gfx/<Unit>/*.png` | Source art (`make gfx` rebuilds the matching `.dmp`) |
| `Gfx/<Unit>/*.dmp` | LZ77 graphics used at install time |
| `Source/WorldMapThoughts.c` | C implementation |
| `Source/thought_bubbles.h` | Graphics symbol declarations |
| `Source/WorldMapThoughts.lyn.event` | Checked-in lyn output (rebuild with `make` after editing `.c`) |
| `makefile` | Compiles C to `.lyn.event` and converts PNG to DMP |

## Hooks & Free Space

| Function | Hook (`ORG`) | Overwritten |
|----------|--------------|-------------|
| `WorldMap_LoopExt` | `$B96F8` | 8 bytes |
| `StartWMNodeMenu` | `$BC5B4` | 8 bytes |

**Free space:** `$1000000` (body and graphics continue at `CURRENTOFFSET` after install).

## Adding new thoughts

Create the bubble as a **128x64** PNG using the **secondary icon palette** as a base. Existing character folders under `Gfx/` can be copied as a template.

1. Add `Gfx/<Unit>/Chapter_<NN>_Thought_Bubble_<Unit>.png` and run `make gfx`.
2. Add an `ALIGN 4` / label / `#incbin` block in `Gfx.event`.
3. Declare `extern u8 Gfx_Chapter_<NN>_Thought_Bubble_<Unit>[];` in `Source/thought_bubbles.h`.
4. Add a `WMTB_ENTRY` in that unit's table in `Source/WorldMapThoughts.c`, and a `switch` case in `GetWorldMapThoughtBubbleForUnit` if the unit is new.
5. Run `make` to rebuild `Source/WorldMapThoughts.lyn.event`.

`WMTB_ENTRY` names use story chapter numbers, but they index `gPlaySt.chapterIndex`:

| Macro | `chapterIndex` | Chapter |
|-------|----------------|---------|
| `02`–`04` | 2–4 | Ch2–Ch4 |
| `05X` | 5 | Ch5x |
| `05` | 6 | Ch5 |
| `06` | 7 | Ch6 |
| `07` | 8 | Ch7 |
| `08` | 9 | Ch8 |
| `09` | 10 | Eirika Ch9 |

Leave unused chapter slots empty so the feature skips them.

## Conflicts

- Any patch that replaces vanilla `WorldMap_LoopExt` (`$B96F8`) or `StartWMNodeMenu` (`$BC5B4`), including the full C Skill System world map rewrite and Enter Town.
- Free space at `$1000000` overlaps with other standalone patches from this repo. Install only one body at `$1000000`, or move this patch's `ORG` after any already-installed standalone code.
- Do not install this on a ROM that already has the C Skill System kernel with `world_map_thought_bubbles` enabled.

`Installer.event` uses `PROTECT` on the hook sites and free-space body. Overlapping ROM writes should fail assembly with a clear EA error.

## Limitations

- Bubbles only appear while a world map node menu is open.
- `R` unit cycling uses the prep roster; it needs a valid world map unit list.
- Camera centering only runs when the selected node is the active unit's current location.
- Default tables cover early Eirika-route chapters. Ephraim-route chapter IDs need their own entries.

## Credits

Extracted from [C Skill System](https://github.com/JesterWizard/C-SkillSystem-Jester). See [WorldMapThoughts.md](../../Documentation/Features/WorldMapThoughts.md) for integrated documentation.
