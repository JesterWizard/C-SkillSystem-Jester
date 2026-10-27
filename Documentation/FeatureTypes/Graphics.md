# Graphics

> Code: `Contents/Gfx/`, `Contents/EfxAnim/`, `Fonts/`, `Data/CustomPortraits/`, `Kernel/Wizardry/MapAnims/`, `Kernel/Wizardry/Backgrounds_224_256_Colors/`, and feature-owned `Gfx/` folders (e.g. `Kernel/Wizardry/LanguageSelect/Gfx/`)  ·  Entry points: `Preload/Reloc.event` (`gp*` pointers), `gpEfxSkillAnims` (`Kernel/Wizardry/SkillSys/EfxSkill/EfxSkill.c:89`), `IconGetters` (`Kernel/Wizardry/IconDisplay/Source/data.c:6`), `bgTable` (`Kernel/Wizardry/Backgrounds_224_256_Colors/gfx/BG/BG.event`)  ·  Existing docs: `Documentation/Features/Fonts.md`, `Documentation/Features/Portrait32Color.md`, `Documentation/Features/Halfbodies.md`, `Documentation/Features/MaxColorBackgrounds.md`, `Documentation/Features/SkillIconPalettes.md`, `Documentation/SkillTypes/Common.md`, `Documentation/SkillTypes/BattleProc.md`

## What it covers

Non-skill image assets: 4bpp tile sheets, LZ77-packed images, GBA palettes, the font glyph set, portrait and halfbody mugs, multi-colour backgrounds, and the battle efx and map sprite animations that consume them. Examples: the boot flag grid in `Kernel/Wizardry/LanguageSelect/`, the battle-forecast arrow in `Data/CombatArt/Gfx_BKSEL/`, and the NinianPray map anim in `Kernel/Wizardry/MapAnims/NinianPray/`. Skill and combat-art icons are covered in the skill docs; this doc covers the asset pipeline itself.

## How the engine calls it

1. **PNG to binary (build time).** The top-level makefile has pattern rules for each PNG in the hack dirs: `%.4bpp` (grit, no palette, no map), `%.lz77` (grit with `-gzl`) and `%.gbapal` (palette only), `makefile:279-295`. `%.img.bin %.map.bin %.pal.bin` (`makefile:302-304`) is the variant used by efx and popups. The outputs sit beside the PNG and are committed (the `# Spritans` block in `.gitignore` is commented out). The final ROM depends on the files `ea-dep --add-missings` reports (`makefile:137`); whether that alone regenerates an absent `.lz77` was not tested (unverified).

2. **Icon sheet to `GFX_*` symbols.** `make gfx` runs `Contents/Gfx/makefile` (`makefile:339-351`), which runs `Contents/Gfx/Scripts/gfx-installer.sh` over every `Sources/**/*.png`. It writes `GfxInstaller.event` (one `ALIGN 4` label plus `#incbin "Sources/<Group>/<Name>.4bpp"` per PNG) and `GfxDefs.h` (one `extern const unsigned char GFX_<Group>_<Name>[]` per PNG), `gfx-installer.sh:21-24`.

3. **Inclusion.** `main.event:59` includes `Contents/Contents.event` inside `ORG FreeSpaceDEMO` (`main.event:57-59`). That includes `Contents/Gfx/Gfx.event` (`Contents/Contents.event:8`), which includes `GfxInstaller.event` (`Contents/Gfx/Gfx.event:6`). The UsrData copy of that include is commented out (`main.event:34`).

4. **C access, two forms.**
   - Direct name in a static initializer, via `include/constants/gfx.h` (`Data/Misc/IconTable.c:1-7`).
   - Pointer variable: `Preload/Reloc.event:47` declares `gpGFX_SkillIcon_WIP: POIN GFX_SkillIcon_WIP`, and `Kernel/Wizardry/SkillSys/kernel/Infos.c` (lines 9 and 16) reads it as a variable. `Documentation/CHAX.md:22` gives the reason: one fixed pointer list that C hacks and FEBuilder patches can share. The split between the two forms is an observed pattern, not traced further.
   - Feature-owned assets use either a direct `extern` (`Kernel/Wizardry/LanguageSelect/Source/LanguageSelect.c:18`) or a `gp` pointer (BKSEL: `Data/CombatArt/CombatArt.event:9-14`, `Preload/Reloc.event:174-175`, `include/kernel/combat-art.h:132-133`, `Kernel/Wizardry/CombatArt/BKSELfx/Source/BattleForcast.c:125-128`).

5. **Icon dispatch.** `IconGetters[ICON_SHEET_AMT]` (`Kernel/Wizardry/IconDisplay/Source/data.c:6-8`, `include/kernel/icon-rework.h:17`) maps a sheet id to a getter. `Kernel/Wizardry/IconDisplay/Source/Icon.c:6-7` calls it. `GetAffinIcon` returns `gpAffinIcons[index]` (`Kernel/Wizardry/IconDisplay/Source/UsrFunc.c:9-11`). Skill icons: `_GetSkillIconExt` reads `gpSkillInfos[...].icon` and falls back to `gpGFX_SkillIcon_WIP` (`Kernel/Wizardry/SkillSys/kernel/Infos.c:8-16`). Each skill sets `.icon = GFX_SkillIcon_<Name>` in `Data/SkillSys/Source/SkillInfo.c` (line 17 is the first).

6. **Palettes.** `ApplyPalette(src, palId)` (`Tools/FE-CLib-Mokha/include/hardware.h:257`) expands to `ApplyPalettes` (`:256`), which calls `CopyToPaletteBuffer(src, 0x20 * palId, 0x20 * count)` (`hardware.h:345`). The destination is `gPaletteBuffer` (`hardware.h:177`). One bank is 16 colours (0x20 bytes). OBJ banks start at `palId` 0x10, which is u16 index 0x100 of `gPaletteBuffer` (`Kernel/Wizardry/LanguageSelect/Source/LanguageSelect.c:146`; `Kernel/Wizardry/CombatArt/BKSELfx/Source/BattleForcast.c:128` uses `OBJ_ARROW_PAL + 0x10`). After writing `gPaletteBuffer` directly, the code calls `EnablePaletteSync()` (`hardware.h:410`, used at `LanguageSelect.c:165`).

7. **Tiles.** `Decompress(src, dst)` (`Tools/FE-CLib-Mokha/include/bmlib.h:78`) takes an `.lz77` stream. `OBJ_CHR_ADDR(n)` is `OBJ_VRAM0 + CHR_SIZE * n` (`hardware.h:217`, `CHR_SIZE` at `:214`). `Copy2dChr(src, dst, w, h)` (`bmlib.h:84`) copies a rectangle of 8x8 tiles. BKSEL decompresses into `gGenericBuffer` (`Tools/FE-CLib-Mokha/include/variables.h:54`, 0x2000 bytes) and then calls `Copy2dChr(..., 7, 2)` (`BattleForcast.c:125-126`). Raw `.4bpp` data is indexed directly instead: `Gfx_BKSEL[12][0x80]` (`Kernel/Wizardry/BattleSys/Source/BattleForcast.c` (lines 7 and 99)) and the EfxAnimNumber digits (`Kernel/Wizardry/BanimHack/AnimNumber/Source/AnimNumber.c:39`).

8. **Drawing an OBJ.** `PutSpriteExt(layer, OAM1_X(x), OAM0_Y(y), gObject_64x64, oam2)` with `oam2 = OAM2_CHR | OAM2_PAL | OAM2_LAYER` (`Tools/FE-CLib-Mokha/include/ctc.h` (lines 54-56, 67, 87)). `LanguageSelect_Animate` (`LanguageSelect.c:204-240`) draws every flag, and it is called from `PROC_REPEAT` steps (`LanguageSelect.c:358-361`), so the draw call belongs in a per-frame proc.

9. **Battle efx animations.** Each `*.efx.txt` line is `<duration> <png>` and is turned into `*.efx.event` by `make efx` (`makefile:309-333`) using `Contents/EfxAnim/Scripts/efx-anim-creator.py`. The script emits `IMG_`, `TSA_` and `PAL_` labels per frame (`efx-anim-creator.py:75-87`), a `FRAMEs` table ending in `SHORT 0xFFFF` (`:149-153`), and `IMGs`/`PALs`/`TSAs` pointer tables (`:157-170`). The result is a `struct EfxAnimConf` (`include/kernel/efx-anim.h:5-16`). At runtime `NewEfxSkill` reads `gpEfxSkillAnims[sid]` (`Kernel/Wizardry/SkillSys/EfxSkill/EfxSkill.c:89`) and falls back to `gpEfxSkillVanilla` when the entry is not ROM data (`:94-95`). The table is `Data/SkillSys/Source/SkillAnimInfo.c:352`. Named labels live in `Contents/EfxAnim/EfxAnim.event`.

10. **Map sprite animations.** `StartNinianPrayfx` (`Kernel/Wizardry/MapAnims/NinianPray/Source/NinianPray.c:4-16`) starts `ProcScr_EventSpriteAnim` with `priv` set to `&EventSpriteAnimConf_NinianPray` (`Kernel/Wizardry/MapAnims/NinianPray/Source/SpriteAnimConf.c:4-12`). That struct holds `pal`, `img`, `ap_conf` (an animation script), `oam0`, `oam2`, `pal_bank` and `pal_size` (`Tools/FE-CLib-Mokha/include/sprite-animfx.h:6-14`). Declarations are in `include/kernel/map-anims.h`.

11. **Portraits.** `setMugEntry(id, label, mouthX, mouthY, eyeX, eyeY)` is an EA macro in `Tools/EventAssembler/Tools/` (file `Tool Helpers.txt`, lines 39-40). It writes a 0x1C-byte entry into the vanilla `PortraitTable` at `id*0x1C`. `PortraitTable` is not defined anywhere in this repo. The image comes from the `PortraitFormatter` EA tool via `#incext` (`Data/CustomPortraits/CustomPortraits.event:16-17`). `Data/Data.event:38` includes that file, which `main.event:32` pulls into the usr data block.

12. **Fonts.** `main.event:39-40` installs `Fonts/Fonts.event` at `FreeSpaceFont`, which includes the generated `Fonts/GlyphInstaller.event`. `make font` builds it (`makefile:367-383`). The generator (`Fonts/Scripts/glyph-installer-generator.py:70-80`) writes one record per glyph, labelled `FontText_<hex>` or `FontItem_<hex>`: a `POIN` to the previously written glyph with the same low byte (or `WORD 0`), a `BYTE` with the high byte and width, then a `#incbin` of a 16x16 `.img.bin`. What reads these records is not traced here (unverified).

13. **Backgrounds.** `Kernel/Wizardry/Backgrounds_224_256_Colors/gfx/BG/BG.event` repoints the vanilla background table (`ORG 0xE894`, `0xECF4`, `0xEDF8`, `0x10E44`) and appends entries as `POIN <IMG>; WORD <flag>; POIN <PAL>`, starting at index 0x38. The middle word is the slot the README calls the TSA pointer: 1 for 224-colour, 0 for 256-colour backgrounds (`Kernel/Wizardry/Backgrounds_224_256_Colors/README.md`). The runtime toggle is `max_color_backgrounds` (`Data/DesignerConfig/designer-config.c:93`), read by `Kernel/Wizardry/Backgrounds_224_256_Colors/src/C/MaxColorBackgroundsEnabled.c:5-7`. These backgrounds are converted by the external Sommie tool into `.dmp` files, not by the grit rules.

## Where to look

| File | Role |
|---|---|
| `Contents/Gfx/Sources/<Group>/<Name>.png` | Icon source PNG. The `.4bpp` beside it is committed. |
| `Contents/Gfx/Scripts/gfx-installer.sh` | Writes `GfxInstaller.event` and `GfxDefs.h` (`GFX_<Group>_<Name>`). |
| `Contents/Gfx/makefile` | `GFX_HEADER` rule; depends on every Sources PNG. Its `clean` recipe is in the Gotchas. |
| `Contents/Gfx/Gfx.event` | Includes `GfxInstaller.event`. Reached via `Contents/Contents.event`. |
| `include/constants/gfx.h` | Wrapper that C includes to get the `GFX_*` names. |
| `makefile` (`:279-306`) | Pattern rules: `.4bpp`, `.gbapal`, `.lz77`, `.img.bin` / `.map.bin` / `.pal.bin`. |
| `makefile` (`:309-351`, `:367-383`) | `efx`, `gfx` and `font` PRE_BUILD targets. |
| `Preload/Reloc.event` | `gp*` pointer table for gfx symbols (`:47`, `:77-80`, `:130-131`, `:174-175`). |
| `Kernel/fake-nonsec-reloc.event` | Generated mirror of `Preload/Reloc.event` (`fake_reloc`, `makefile:511-525`). Do not edit. |
| `include/kernel/icon-rework.h` | `ICON_SHEET_*` ids and `ICON_SHEET_AMT`. |
| `Data/Misc/IconTable.c` | Per-id tables of `GFX_*` icon pointers (affinity, mount, weapon, tonic). |
| `Kernel/Wizardry/SkillSys/kernel/Infos.c` | Skill icon getters and the `gpGFX_SkillIcon_WIP` fallback. |
| `Data/SkillSys/Source/SkillInfo.c` | Per-skill `.icon = GFX_SkillIcon_*`. |
| `Kernel/Wizardry/LanguageSelect/` | Example feature folder: `Gfx/`, installer, `Source/`. |
| `Data/CombatArt/CombatArt.event` | `Img_BkselObjArrow` / `Pal_BkselObjArrow` labels and their `#incbin` lines. |
| `Data/Anims/Anims.event` | `Img_EfxAnimNumber` (raw 4bpp) and `Pal_EfxAnimNumber`. |
| `Data/Data.event` | Usr-data include list (includes `CombatArt`, `Anims`, `CustomPortraits`). |
| `Kernel/Wizardry/wizardry.event` | Section switches: `WIZARDRY_DATA` (`:1-39`), `WIZARDRY_TEXT` (`:41-127`), `WIZARDRY_CUSTOM` (`:129-191`). |
| `main.event` | `Preload/Reloc.event` (`:27`), `Data/Data.event` (`:32`), `Fonts/Fonts.event` (`:40`), `Contents/Contents.event` (`:59`, FreeSpaceDEMO). |
| `Contents/EfxAnim/EfxAnim.event` | Named battle-efx labels (`EfxSkillVanilla`, `EfxCritCutin`, ...). |
| `Contents/EfxAnim/Scripts/efx-anim-creator.py` | `*.efx.txt` to `*.efx.event` generator. |
| `Contents/EfxAnim/Source/` | Battle efx sources (`*.efx.txt`, PNGs). |
| `include/kernel/efx-anim.h` | `struct EfxAnimConf`. |
| `Kernel/Wizardry/SkillSys/EfxSkill/EfxSkill.c` | `NewEfxSkill` / `NewEfxCombatArt` (`:85`, `:120`). |
| `Kernel/Wizardry/MapAnims/` | Map anims: `data.event` (labels), `<Name>/data.event`, `<Name>/Source/`. |
| `include/kernel/map-anims.h` | `CallMapAnim_*`, `StartNinianPrayfx`, and the map-anim asset externs. |
| `Tools/FE-CLib-Mokha/include/sprite-animfx.h` | `struct EventSpriteAnimConf` and `ProcScr_EventSpriteAnim`. |
| `Tools/FE-CLib-Mokha/include/hardware.h` | `ApplyPalette`, `CopyToPaletteBuffer`, `gPaletteBuffer`, `OBJ_CHR_ADDR`, `CHR_SIZE`. |
| `Tools/FE-CLib-Mokha/include/bmlib.h` | `Decompress`, `Copy2dChr`. |
| `Tools/FE-CLib-Mokha/include/ctc.h` | `PutSpriteExt`, `OAM*` macros, `gObject_64x64`. |
| `Fonts/FontList.txt` | Glyph set install order, one `CONFIG_FONT_*` block per font. |
| `include/configs/configs.h` | `CONFIG_FONT_*` switches (`:35-44`). |
| `Fonts/Glyph/<Font>/font.fontall.txt` | Glyph table: `char  type  width  filename`. |
| `Fonts/Scripts/glyph-installer-generator.py` | Writes `GlyphInstaller.event` and the glyph records. |
| `Data/CustomPortraits/CustomPortraits.event` | `#incext PortraitFormatter` blocks and `setMugEntry` calls. |
| `Data/CustomPortraits/Portraits/` | Portrait source PNGs. |
| `Tools/EventAssembler/Tools/` | `PortraitFormatter`, `PortraitFormatter32`, `HalfbodyFormatter`, and the `Tool Helpers.txt` macros. |
| `Data/DesignerConfig/designer-config.c` | Runtime flags: `.half_body_portraits` (`:71`), `.max_color_backgrounds` (`:93`), `.portrait_32_color` (`:108`). |
| `include/kernel/kernel-lib.h` | `KernelDesigerConfig` fields for those flags (`:65`, `:178`, `:182`). |
| `Kernel/Wizardry/Backgrounds_224_256_Colors/gfx/BG/BG.event` | Background table repoint and custom entries. |
| `Kernel/Wizardry/Backgrounds_224_256_Colors/README.md` | Sommie conversion and 224/256 flag rules. |

## Add one: steps

1. Do the common steps in [Common.md](Common.md).
2. Pick the path that matches the asset.
   - Icon for a sheet or a skill: PNG under `Contents/Gfx/Sources/<Group>/`, then `make gfx` (step 4).
   - Image plus palette used by one feature: feature `Gfx/` folder, installer and C (Template below).
   - Battle efx: `Contents/EfxAnim/Source/<Name>/<Name>.efx.txt`, a label in `Contents/EfxAnim/EfxAnim.event`, and `&<label>` in `Data/SkillSys/Source/SkillAnimInfo.c:352`. See [BattleProc.md](../SkillTypes/BattleProc.md).
   - Map sprite animation: copy the NinianPray layout (`data.event`, `SpriteAnimConf.c`, a `Start...` function).
   - Font, portrait, or background: follow the existing Features doc (`Fonts.md`, `Portrait32Color.md`, `MaxColorBackgrounds.md`). Do not re-derive them here.
3. Author the PNG as indexed colour with colour 0 transparent. LanguageSelect's comment on its palette says index 0 is transparent (`LanguageSelect.c:62`). Keep the image to one 16-colour bank per palette.
4. Keep the PNG and its generated `.lz77` / `.gbapal` / `.4bpp` together and commit them (rules at `makefile:279-295`). For icon-sheet entries, run `make gfx`. Never edit `Contents/Gfx/GfxDefs.h` or `GfxInstaller.event`.
5. Write `Kernel/Wizardry/<Feature>/<Feature>_Installer.event` with a guard, the C `.lyn.event` include, and the `Img_`/`Pal_` labels with their `#incbin` lines (Template).
6. Add one `#include "<Feature>/<Feature>_Installer.event"` line inside the `#ifdef WIZARDRY_CUSTOM` block of `Kernel/Wizardry/wizardry.event` (`:129-191`; LanguageSelect is at `:153`).
7. In C, declare the labels, `Decompress` the tiles into `OBJ_CHR_ADDR(n)`, `ApplyPalette` the bank to `0x10 + bank`, and draw with `PutSpriteExt` every frame.
8. Only if the label must be reached through a pointer from another section, use the `gp` form: a `gp<Name>: POIN <Name>` line in `Preload/Reloc.event` and an `extern` pointer in a header (BKSEL pattern). Each entry costs a word in the reloc table.
9. Run `make`. The PRE_BUILD targets `gfx`, `font` and `efx` run first (`makefile:330`, `:345`, `:382`). Testing in an emulator is not covered here.

## Template

Adapted from `Kernel/Wizardry/LanguageSelect/` (installer at `LanguageSelect_Installer.event:1-5`, C at `Source/LanguageSelect.c:18` and `:242-255`, draw at `:189`). Names are placeholders. The slot numbers are placeholders too; nothing in this doc checks that they are free.

**`Example_Installer.event`** (in `Kernel/Wizardry/<Feature>/`):

```
#ifndef EXAMPLE_INSTALLED
#define EXAMPLE_INSTALLED

#include "Source/Example.lyn.event"

ALIGN 4
Img_Example:
    #incbin "Gfx/Img_Example.lz77"

ALIGN 4
Pal_Example:
    #incbin "Gfx/Img_Example.gbapal"

#endif /* EXAMPLE_INSTALLED */
```

**`Example.c`** (in `Kernel/Wizardry/<Feature>/Source/`):

```c
#include "common-chax.h"
#include "kernel-lib.h"

/* Labels from Example_Installer.event */
extern const u8 Img_Example[];
extern const u16 Pal_Example[];

#define EXAMPLE_OBJ_CHR 0x3C0 /* 64 tiles, 64x64 4bpp, placeholder slot */
#define EXAMPLE_OBJ_PAL 0xF   /* OBJ bank 15, placeholder slot */

void Example_LoadGraphics(void)
{
	Decompress(Img_Example, OBJ_CHR_ADDR(EXAMPLE_OBJ_CHR));
	ApplyPalette(Pal_Example, 0x10 + EXAMPLE_OBJ_PAL);
}

/* Call every frame while the sprite is on screen. */
void Example_Draw(int x, int y)
{
	int oam2 = OAM2_CHR(EXAMPLE_OBJ_CHR) | OAM2_PAL(EXAMPLE_OBJ_PAL) | OAM2_LAYER(1);

	PutSpriteExt(4, OAM1_X(x - 32), OAM0_Y(y - 32), gObject_64x64, oam2);
}
```

Registration line in `Kernel/Wizardry/wizardry.event`, inside the `#ifdef WIZARDRY_CUSTOM` block:

```
#include "Example/Example_Installer.event"
```

The PNG goes in the feature's `Gfx/` folder as `Img_Example.png`. The make rules then produce `Img_Example.lz77` and `Img_Example.gbapal` in the same folder.

## Worked examples

- `Kernel/Wizardry/LanguageSelect/`: the flag grid. An `.lz77` sheet with 64x64 OBJ cells, its own palette written per frame, and an installer in the `WIZARDRY_CUSTOM` block (`LanguageSelect_Installer.event`).
- `Data/CombatArt/Gfx_BKSEL/` with `Kernel/Wizardry/CombatArt/BKSELfx/Source/BattleForcast.c`: the battle-forecast arrow. An `.lz77` plus `.gbapal` pair reached through `gp` pointers, decompressed into `gGenericBuffer`, then `Copy2dChr` into OBJ VRAM.
- `Kernel/Wizardry/MapAnims/NinianPray/`: a map sprite animation. `.lz77` and `.gbapal` labels in `data.event`, an `EventSpriteAnimConf` in `Source/SpriteAnimConf.c`, and `StartNinianPrayfx` in `Source/NinianPray.c`.
- `Contents/Gfx/Sources/SkillIcon/` with `Data/SkillSys/Source/SkillInfo.c`: the icon-sheet path. Raw `.4bpp` icons are installed by `GfxInstaller.event` as `GFX_SkillIcon_<Name>`, and each skill refers to one by name.

## Gotchas

- **Generated files.** `Contents/Gfx/GfxDefs.h` and `GfxInstaller.event` are gitignored (`Contents/Gfx/.gitignore:1-2`) and rewritten by `gfx-installer.sh`. Edit the PNG instead.
- **The symbol is the path.** `GFX_<Group>_<Name>` is built from the folder and file name (`gfx-installer.sh:21-24`). Renaming a folder or PNG breaks every C user (`Data/Misc/IconTable.c`, `Data/SkillSys/Source/SkillInfo.c`).
- **`include/constants/gfx.h` names a stale target.** Its header says "Auto generated by: make icon", but no `icon` target exists in `makefile`, `Contents/Gfx/makefile` or `Fonts/makefile`.
- **Kernel-only builds drop these assets.** `main.event:59-62` is the only place that includes `Contents/` and defines `WIZARDRY_CUSTOM`. `main-kernel.event` includes neither. Assets and installers in those blocks are therefore absent in `CONFIG_KERNEL_ONLY` builds. This follows from the include graph; no kernel-only build was run.
- **Put `#incbin` in data, not in code.** The kernel text section is nearly full (`CLAUDE.md`). Keep `#incbin` in installers (`WIZARDRY_CUSTOM`, FreeSpaceDEMO) or in the data `.event` files, not in files compiled into kernel text.
- **LZ77 versus raw.** `Decompress` is used on `.lz77` streams. Raw `.4bpp` is indexed directly (`Gfx_BKSEL[12][0x80]`, `BattleForcast.c:7,99`). Passing a raw block to `Decompress` was not tested (unverified).
- **One palette bank per `ApplyPalette`.** The macro copies `0x20 * count` bytes with `count = 1` (`hardware.h:257`). Whether a `.gbapal` with more than 16 colours is handled some other way is unverified.
- **OBJ palette and tile slots.** OBJ banks are 0x10 to 0x1F. The tile range must be free in OBJ VRAM. Other users of `OBJ_CHR_ADDR` and `OAM2_PAL` were not all checked (unverified).
- **2D OBJ mapping.** `LanguageSelect.c:253` sets `obj1dMap = 0`. Its 64x64 cells on a 256-pixel sheet are addressed as `row * 0x100 + col * 8` tile units (`LanguageSelect.c:175`).
- **Affine cost.** A double-size affine 64x64 costs about four times a plain OBJ line time. Six of them overflow the scanline budget (`LanguageSelect.c:183-186` comment).
- **Palette sync.** After writing `gPaletteBuffer` directly, call `EnablePaletteSync()` (`LanguageSelect.c:165`). Whether `ApplyPalette` needs it too is unverified.
- **Reloc table cost.** Each `gp*` entry adds a word to `Preload/Reloc.event`, which is sized by `UsrRelocSize` (`main.event:27-29`). Remaining room is unverified. `Kernel/fake-nonsec-reloc.event` is generated from it (`makefile:511-525`).
- **Efx output is not tracked.** `git ls-files` lists no `*.efx.event`; `make efx` regenerates them (`makefile:309-333`). The `_unused_header` (0x150 bytes) in `struct EfxAnimConf` is the FEBuilder identification header (`include/kernel/efx-anim.h:6-10`); the generator writes its own copy in `gen_header` (`Contents/EfxAnim/Scripts/efx-anim-creator.py:90`).
- **Portrait modes cannot share a face.** `Documentation/Features/Portrait32Color.md` says not to enable `portrait_32_color` together with `half_body_portraits` on the same face. Halfbodies must not use `setMugEntry_32Color`.
- **Doc conflict on the halfbody default.** `Documentation/Features/Halfbodies.md` says the default is `true`. `Data/DesignerConfig/designer-config.c:71` sets `.half_body_portraits = false`. This doc follows the code. Which one is intended is unverified.
- **One font at a time.** Exactly one `CONFIG_FONT_*` is active (`include/configs/configs.h:35-44`). After changing it, follow `Documentation/Features/Fonts.md`: remove `Fonts/GlyphInstaller.event` and run `make -C Fonts`. The font block has an `ASSERT` against `FreeSpaceFontSize` (`main.event:39-43`); remaining room is unverified.
- **Background table indices.** New backgrounds start at index 0x38 (`BG.event`, last lines). The conversion is done by the external Sommie tool, not by the grit rules (`Kernel/Wizardry/Backgrounds_224_256_Colors/README.md`).
- **Runtime toggles.** The runtime flag `max_color_backgrounds` (default `true`, `designer-config.c:93`) gates the path (`MaxColorBackgroundsEnabled.c:7`). What the vanilla path does with multi-colour art was not tested.
