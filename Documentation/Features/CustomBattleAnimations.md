# Custom Battle Animations

---

## Index
- [Introduction](#introduction)
- [Switches](#switches)
- [Adding an Animation](#adding-an-animation)
- [IDs and Free Room](#ids-and-free-room)
- [Palettes](#palettes)
- [How It Works](#how-it-works)
- [Compression](#compression)
- [Testing](#testing)
- [Code Locations](#code-locations)

---

## Introduction

Battle animations can be added to the game from FEditor Advance dumps. A character uses one in place of their class's
animation (see [Credits](../Credits.md#battle-animations) for the authors):

| ID | Character, class | Weapon | Animation | Author |
|----|------------------|--------|-----------|--------|
| 0xCA | Gerik, Mercenary | swords | "[Mercenary-Variant] SALVAGED Gerik [M]" (1. Sword) | UltraFenix |
| 0xCB | Tana, Pegasus Knight | lances | "Peg T1 Reskin F Tana" (2. Lance) | RedBean |

Their other weapon types (unarmed) keep the class's vanilla animation.

The custom animations are in `Contents/BattleAnims/`. Vanilla's animation table is not changed: a new animation
gets an ID after vanilla's, and a character or a class is pointed at it through `CustomAnimeTable`.

---

## Switches

- `CONFIG_CUSTOM_BATTLE_ANIMS` (`include/configs/configs.h`) is the compile-time switch. It includes
  `Contents/BattleAnims/BattleAnims.event`; off, that file builds nothing.
- `KernelDesigerConfig.custom_character_animations` (`Data/DesignerConfig/designer-config.c`) is the runtime
  toggle (struct spelled as in the code). Off, a character keeps its class's animations.

Both are needed for Gerik's sword to show.

---

## Adding an Animation

1. Export the animation from FEditor Advance. The dump is three kinds of file: `<Name>.bin` (a Java dump of the
   modes, OAM and palettes), `<Name> Frame Data.dmp` (the script) and `<Name> Sheet N.png` (264x64, one per sheet).
2. Put the dump in `Contents/BattleAnims/<Char>/Source/`, as Gerik's is in `Contents/BattleAnims/Gerik/Source/`.
   It is kept there as the source.
3. Convert it:
   ```
   python3 Tools/banim/feditor_dump.py Contents/BattleAnims/<Char>/Source Contents/BattleAnims/<Char> <Prefix> <abbr>
   ```
   `<Prefix>` names the output (`<Prefix>.event`, `<Prefix>_sheet_<i>.lz`, `<Prefix>_oam_r.lz`, `<Prefix>_oam_l.lz`,
   `<Prefix>_pal.lz`, `<Prefix>_modes.bin`). `<abbr>` is the short name written into the entry (ASCII, at most 11
   characters; Gerik's is `gerik_sw1`). The converter's `<Prefix>_BANIM_ENTRY` macro holds the entry.
4. In `Contents/BattleAnims/BattleAnims.event`, add the `#include`, then the entry at the next free ID:
   ```
   #include "<Char>/<Prefix>.event"
   PUSH
   BanimEntry(<ID>)
   <Prefix>_BANIM_ENTRY
   POP
   ```
   and an animation list for the character, if one is needed (see the next step). Define the ID as a `#define`
   next to the others (`BANIM_GERIK_SWORD`).
5. Add a row to `CustomAnimeTable` in `Contents/External/Individual_animation_ea_2/data.event`:
   `BYTE <character> BYTE <class> BYTE 0 BYTE 0 WORD <list>`, where `<list>` is an animation list like
   `gBanimDef_GerikMercenary` in `BattleAnims.event`. Its rows are `BYTE <weapon type> 0x01; SHORT <ID>` (kind 1 is
   a weapon type; kind 0 would be an item id), and the list ends with `WORD 0`. Gerik's two rows: swords (weapon type
   0x00) use ID 0xCA, unarmed (0x09) keeps vanilla Mercenary's 0x0C.
6. Add a row to the Credits (`Documentation/Credits.md`, "Battle Animations"), with the author and a link
   to the source.
7. Rebuild with `Tools/cloud/build.sh`, then run `python3 Tools/cloud/smoketest.py --only banim`.

The converter's output is committed (`Contents/BattleAnims/<Char>/*.event`, `*.lz`). Run it again only when the
dump changes.

---

## IDs and Free Room

Vanilla's `banim_data` (ROM `0xC00008`, 0x20 bytes an entry) has 201 animations, IDs 0x01-0xC9. The zero bytes after
it, up to `0xC02000`, hold the new entries: IDs **0xCA-0xFF**, 54 in all. An entry goes at
`0xC00008 + 0x20 * (ID - 1)`; `BanimEntry(id)` in `BattleAnims.event` writes it there.

Take the next free ID after the highest one in use in `BattleAnims.event`: Gerik's sword is 0xCA, Tana's lance
0xCB, so the next is 0xCC.

---

## Palettes

A character's palettes (`gAnimCharaPalConfig`) apply only to its class's own animations. A custom animation uses
the four palettes in its own entry. So the battle's unique palette for that side is -1 (no character palette), and
the sprite's colours are the ones the dump gave.

---

## How It Works

- `GetUnitBattleAnimDef(unit)` (`Kernel/Wizardry/FebCompat/IndividualAnim/Source/IndividualAnim.c`) returns the
  unit's row of `CustomAnimeTable` if it has one, else its class's row. It reads the table directly; before, it went
  through a hook that was disabled, and read garbage.
- Battle setup (`PrepareBattleGraphicsMaybe`, `Kernel/Wizardry/HPBarPalettes/HPBarPalettes.c`) loads the unit's
  animation from that lookup. Combo attacks use the same lookup
  (`Kernel/Wizardry/ComboAttack/Source/efx-combo.c`, `Kernel/Wizardry/BanimSync/Source/utils.c`).
- The loaded index is in `gBanimIdx[2]` (per side, the player on the right). It holds the ID minus 1, so Gerik's
  sword is 0xC9 there. `gBanimUniquePal[2]` is the side's unique palette, -1 for a custom animation.

## Compression

The converter writes Gerik's sword in 43.9 KB and Tana's lance in 39.2 KB, against 58.6 KB and 53.1 KB inserted the
usual way (every part in the BIOS's LZ77, both OAMs):

| Part | Gerik's sword | Tana's lance | How |
|------|---------------|--------------|-----|
| Sheets | 37.1 KB (12) | 34.5 KB (11) | Huffman-coded, one pair of trees per animation (below) |
| Right OAM | 4.8 KB | 3.6 KB | LZ77, optimal parse |
| Left OAM | 0 | 0 | none: AutoGenLeftOAM makes it |
| Script | 1.8 KB | 1.0 KB | LZ77, optimal parse, sheet pointers filled in by EA |
| Palettes, modes | 0.2 KB | 0.2 KB | |

- **The sheets** (`Kernel/Wizardry/BanimCodec`): deflate's symbols (literals and length/distance pairs) with one pair
  of canonical Huffman trees for all the animation's sheets, stored once in ROM with lookup tables
  (`struct BanimHuffTrees`, `include/kernel/banim-codec.h`). A sheet starts with `(size << 8) | 0xD1` and a pointer to
  the trees. No RAM.
- **Where it's decoded.** Every vanilla call that can decompress an animation's sheet, script or OAM goes to
  `BanimUnComp` (`BanimCodec.event`), which decodes that tag and passes everything else (every vanilla animation) to
  the BIOS: the sheet loader (`RegisterAISSheetGraphics`), `UpdateBanimFrame`, the mini animations
  (`InitMainMiniAnim`, `sub_805A7B4`, `sub_805AA68`), and the unit zooming in from the map and back out
  (`UnitKakudaiPrepareAnimScript`). A call left out decodes a Huffman sheet as LZ77 into garbage: the zoom was, and the
  battle opened and closed on a garbled sprite.
- **Speed.** The decoding loop is ARM code (`Source/BanimHuffArm.s`, generated from `Tools/banim/huffdec.c` by
  `Tools/banim/gen_huffdec_asm.sh`) copied into IWRAM for each sheet, into `gMovMapFillStPool2` (0x03004C50, 0x200
  bytes: the movement flood fill's scratch, which VoiceStream also borrows, see
  [VoiceStream.md](VoiceStream.md#the-iwram-overlay)). `BanimCodec.event` checks that it fits. A sheet takes up to 1.3
  frames, against about a third of a frame for the BIOS's LZ77 (Thumb code from ROM took 7.6), so a sheet change can
  drop a frame: the test battle runs 776 frames against 764 with LZ77 sheets. The sheet is still copied to VRAM at the
  VBlank after its frame's sprites are set, as vanilla does, so the two stay in step.
- **The left OAM.** AutoGenLeftOAM (`Contents/External/AutoGenLeftOAM`, 7743) mirrors the right OAM when both
  pointers are the same. The converter checks that the mirror gives the dump's left OAM exactly; if not, it stores the
  left OAM.
- **The script** holds the sheets' ROM addresses, which only EA knows. The converter compresses it with each pointer
  byte as a symbol of its own (a match only copies a pointer byte from the same byte of the same sheet's pointer) and
  writes those bytes as `BANIM_PTR_BYTE(label, k)` expressions.
- **`--lz77`** (`feditor_dump.py --lz77 ...`) writes the sheets in LZ77 instead: decoded by the BIOS, no dropped
  frame, about a fifth bigger (Gerik's sword 53.4 KB).
- **Measured but not used:** compressing the sheets together (LZMA, 26.6 KB for Gerik's) needs every sheet in RAM at
  once (96 KB); repacking frames into fewer sheets frees at most one (Gerik's sheets are 89% full and only 551 of
  2,749 tiles repeat, mostly on other sheets); a bit-packed LZ without Huffman coding gives 40.5 KB.

---

## Testing

`python3 Tools/cloud/smoketest.py --only banim` (the test is `test_banim`, `Tools/cloud/smoketest.py`). It starts
from the prologue's turn 2 and fights twice: Eirika's unit made Gerik as a Mercenary with an Iron Sword, then Tana as
a Pegasus Knight with an Iron Lance (the unit gets the weapon rank). Each time it checks:

- the loaded animation is the custom one (`gBanimIdx` the ID minus 1: 0xC9 for Gerik's 0xCA, 0xCA for Tana's 0xCB);
- that side has no character palette (`gBanimUniquePal` -1).

It saves `gerik.png` and `tana.png` (the battles) in the test's folder.

---

## Code Locations

| What | Where |
|------|-------|
| Compile-time switch | `include/configs/configs.h` (`CONFIG_CUSTOM_BATTLE_ANIMS`) |
| Runtime toggle | `Data/DesignerConfig/designer-config.c` (`custom_character_animations`) |
| Entry point and lists | `Contents/BattleAnims/BattleAnims.event` |
| The animations (source and converted) | `Contents/BattleAnims/Gerik/`, `Contents/BattleAnims/Tana/` |
| Converter | `Tools/banim/feditor_dump.py`, `Tools/banim/banimpack.py` (the compressors) |
| Sheet decoder | `Kernel/Wizardry/BanimCodec/` (`BanimUnComp`); the ARM loop from `Tools/banim/huffdec.c` |
| `CustomAnimeTable` rows | `Contents/External/Individual_animation_ea_2/data.event` |
| Lookup | `Kernel/Wizardry/FebCompat/IndividualAnim/Source/IndividualAnim.c` (`GetUnitBattleAnimDef`) |
| Battle setup | `Kernel/Wizardry/HPBarPalettes/HPBarPalettes.c` (`PrepareBattleGraphicsMaybe`) |
| Combo attacks | `Kernel/Wizardry/ComboAttack/Source/efx-combo.c`, `Kernel/Wizardry/BanimSync/Source/utils.c` |
| Test | `Tools/cloud/smoketest.py` (`test_banim`) |
