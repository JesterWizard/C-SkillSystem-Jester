# Alpha Blend Movement Sprites

<p align="center">
  <img src="./Feature - Alpha Blend Movement Sprites.gif" alt="Alpha Blend Movement Sprites" width="600"/>
</p>

While drawing a move path, this shows a Lex Talionis-style faded copy of the active unit's moving-map sprite at the cursor tip. The live MU faces the last path step. At the origin tile, the ghost is hidden and the selected bounce facing is restored.

## RAM

This patch uses **50 bytes** of EWRAM for `sPathfindingGhostObjBuf`, a copy of the MU sprite OBJ list passed to `PutSpriteExt`. The buffer must live in RAM because `PutSpriteExt` only stores a pointer and OAM is flushed later.

### Why 50 bytes?

The size is fixed by the OBJ list layout in `DisplayBlendedMuAp`:

| Field | Size |
|-------|------|
| Sprite count (`u16`) | 2 bytes |
| Up to 8 OAM entries × 3 `u16` each (ATTR0, ATTR1, ATTR2) | 48 bytes |
| **Total** | **50 bytes** |

The C code caps entries at 8 (`count > 8` early-outs), matching the largest moving-map sprite OBJ list vanilla uses. The integrated kernel reserves the same size in `include/link/config-memmap.s` (`_kernel_malloc sPathfindingGhostObjBuf, 50`).

`RamAlloc.s` only sets the **base address** of that buffer; the 50-byte span is implied by the C layout above. Nothing in the asm file allocates or pads bytes — you must keep the chosen address range free.

### Default allocation

| Symbol | Address range | Size | Region |
|--------|---------------|------|--------|
| `sPathfindingGhostObjBuf` | `0x0203AAA4` .. `0x0203AAD5` | 50 bytes | FreeRamSpace2 base |

### Clean FE8U free EWRAM pools

| Region | Address range | Notes |
|--------|---------------|-------|
| **FreeRamSpace2** | `0x0203AAA4` .. `0x0203DDE0` | ~13 KB; default for this patch; grows upward from base |
| **FreeRamSpace** | `0x02026E30` .. `0x02028E58` | Vanilla debug-print buffer; often claimed by skillsys stacks |
| **FreeRamSpace3** | `0x02026AD0` .. `0x02026E30` | ~864 B; grows downward from top |

To use a different address, pick an unused 50-byte slice inside one of these regions, edit `Source/RamAlloc.s`, then run `make` and reinstall.

Integrated CHAX builds already allocate this symbol in `include/link/config-memmap.s` — do not install this standalone patch on top of the full kernel with `alpha_blend_movement_sprites` enabled.

## Target ROM

- **FE8U (USA)** clean ROM
- Hook: vanilla `DrawUpdatedPathArrow` at `0x08033249` (installer ORG `$33248`)
- Free space: `$1000000`

## Build

No build step is required to install the patch. `Source/AlphaBlendMovementSprites.lyn.event` is checked in, so you can install `Installer.event` as-is.

Run `make` if you edit `Source/AlphaBlendMovementSprites.c` or `Source/RamAlloc.s`:

```bash
make -C Standalone/alpha_blend_movement_sprites
```

That requires [devkitARM](https://devkitpro.org/wiki/Getting_Started) and this repo's [FE-CLib](https://github.com/MokhaLeee/FE-CLib-Mokha) / [Event Assembler](https://github.com/MokhaLeee/EventAssembler/tree/mokha-fix) tools. See [Documentation/Setup.md](../../Documentation/Setup.md) for full setup.

## Installation

### FEBuilderGBA

1. Download [FEBuilderGBA (Laqieer branch)](https://nightly.link/laqieer/FEBuilderGBA/workflows/msbuild/master).
2. In **Settings → Options → Path**, set **Event Assembler** to this repo's [ColorzCore](https://github.com/MokhaLeee/EventAssembler/tree/mokha-fix) executable (see [Setup.md](../../Documentation/Setup.md)).
3. Open your project ROM in FEBuilder.
4. Go to **Advanced Editors → Insert EA**.
5. Click **Select File**, choose `Standalone/alpha_blend_movement_sprites/Installer.event`, then click **Load Script**.

### Event Assembler

From a copy of clean `fe8.gba`:

```bash
cp /path/to/fe8.gba /path/to/fe8-alpha-blend-movement-sprites.gba
Tools/EventAssembler/ColorzCore A FE8 \
  -input:Standalone/alpha_blend_movement_sprites/Installer.event \
  -output:/path/to/fe8-alpha-blend-movement-sprites.gba
```

Run from the repository root so Event Assembler can resolve `EAstdlib.event`.

## Files

| File | Purpose |
|------|---------|
| `Installer.event` | EA installer, hook, and free-space placement |
| `Source/AlphaBlendMovementSprites.c` | Ghost draw + blend logic |
| `Source/RamAlloc.s` | EWRAM address for `sPathfindingGhostObjBuf` |
| `Source/AlphaBlendMovementSprites.lyn.event` | Checked-in lyn output |
| `makefile` | Compiles C + RAM object to `.lyn.event` |

## Hooks & Free Space

| Function | Hook (`ORG`) | Overwritten |
|----------|--------------|-------------|
| `DrawUpdatedPathArrow` | `$33248` | 8 bytes |

**Free space:** `$1000000` (body continues at `CURRENTOFFSET` after install).

## Conflicts

- Any patch that hooks vanilla `DrawUpdatedPathArrow` (`$33248`), including C Skill System `RemoveMovePath`.
- **EWRAM** `0x0203AAA4` .. `0x0203AAD5` (50 bytes) overlaps with other hacks using FreeRamSpace2 (`0x0203AAA4` .. `0x0203DDE0`). Change `Source/RamAlloc.s` if needed.
- Free space at `$1000000` overlaps with other standalone patches from this repo. Install only one body at `$1000000`, or move this patch's `ORG` after any already-installed standalone code.
- Patches that disable path arrows (`remove_move_path`) prevent the ghost from drawing.

`Installer.event` uses `PROTECT` on the hook site and free-space body. Overlapping ROM writes should fail assembly with a clear EA error.

## Limitations

- Ghost only appears while the vanilla path-arrow UI is active.
- Blend setup is tuned for BG2 range squares + BG3 map; alternate range rendering may need different blend targets.
- Standing / hidden MUs skip the ghost (`MU_FACING_STANDING`, `hidden_b`).

## Credits

Extracted from [C Skill System](https://github.com/JesterWizard/C-SkillSystem-Jester). See [TranslucentUnitSprite.md](../../Documentation/Features/TranslucentUnitSprite.md) for integrated documentation.
