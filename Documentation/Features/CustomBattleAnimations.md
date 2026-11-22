# Custom Battle Animations

---

## Index
- [Introduction](#introduction)
- [Switches](#switches)
- [Adding an Animation](#adding-an-animation)
- [Reskinning a Vanilla Animation](#reskinning-a-vanilla-animation)
- [From Frames](#from-frames)
- [IDs and Free Room](#ids-and-free-room)
- [Palettes](#palettes)
- [How It Works](#how-it-works)
- [Compression](#compression)
- [Viewing in Game](#viewing-in-game)
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
| 0xCC | Amelia, Recruit (Tier 0) | lances | Amelia, Tier 0 (Lance) | Spud |
| 0xCD | Amelia, Recruit pre-promote (`CLASS_RECRUIT_T1`) | lances | Amelia, PrePromote (Lance) | Spud |
| 0xCE | Amelia, Halberdier (`CLASS_HALBERDIER`) | lances | Amelia, Halberdier (Lance) | Spud |
| 0xCF | Joshua, Assassin | knives | "[Assassin-Reskin] Joshua [M]" (1. Knife) | Seliost1 |
| 0xD0 | Marisa, Assassin (F) | swords | "[Assassin-Reskin] Marisa [F]" (1. Sword, Knife Ranged) | RedBean |
| 0xD1 | Colm, Assassin | swords | "[Assassin-Variant] [M] Colm" (1. Sword, Fixed) | Aruka |
| 0xD2 | Eirika, Master Lord | swords | "[FE8 Eirika-Reskin] T2 Eirika Unmounted [F]" (1. Sword) | RedBean |
| 0xD3 | Ephraim, Master Lord | lances | "[FE8 Ephraim-Custom] T2 Heavy Infantry [M]" (2. Lance, Siegmund) | Nuramon |
| 0xD4 | Amelia, General (F) | swords | "[General-Reskin] Amelia [F]" (1. Sword) | RedBean |
| 0xD5 | Amelia, Armor Knight (F) | lances | "[Knight-Variant] Amelia [F]" (2. Lance) | RedBean |
| 0xD6 | L'Arachel, Mage Knight (F) | anima, light, dark, unarmed | "[MK-Variant] L'Arachel [F]" (6. Magic) | RedBean |
| 0xD7 | L'Arachel, Valkyrie | anima, light, dark, unarmed | "[Valkyrie-Variant] L'Arachel [F]" (6. Magic) | RedBean |
| 0xD8 | L'Arachel, Troubadour | staves | "[Troubadour-Reskin] L'Arachel [F]" (7. Staff) | RedBean |
| 0xD9 | Morva, Manakete (`CLASS_MANAKETE_2`) | everything (Wretched Air, unarmed) | "[Manakete-Base] [U] Vanilla FE8 Undead Morva" (8. Dragonstone) | Unknown |
| 0xDA | Joshua, Myrmidon | swords | "[Myrmidon-Reskin] Joshua Hat Repal [M]" (1. Sword) | Unknown |
| 0xDB | Joshua, Swordmaster | swords | "[Swordmaster-Variant] Joshua [M]" (1. Sword, Alt 2) | Aruka |
| 0xDC | Marisa, Swordmaster (F) | swords | "[Swordmaster-Variant] Marisa Repal [F]" (1. Sword, Alt Crit) | RedBean |
| 0xDD | every Phantom (class-wide) | axes | "[Monster-Custom] [U] Phantom" (3. Axe) | TBA |
| 0xDE | every Gorgon Egg (class-wide, both egg classes) | everything | "[Gorgon-Custom] [U] Gorgon Eggs" (8. Unarmed, Large) | Ryn |

Their other weapon types (mostly unarmed) keep the class's vanilla animation. Each list in `BattleAnims.event` is the
class's vanilla list with the custom ID in place of the animated weapon types' entries.

The Phantom and the Gorgon Eggs had no battle animation (their fights were map animations): their animations are
class-wide, set as the class's `pBattleAnimDef` in `Data/ClassData/ClassData.c` (`BANIM_DEF_PHANTOM`,
`BANIM_DEF_GORGON_EGG`), so `custom_character_animations` does not turn them off. An egg's list covers every weapon
type, so an egg holding a weapon still gets its animation.

Five more animations are for the third-tier classes ([ThirdTierClasses.md](ThirdTierClasses.md)). `BattleAnims.event`
installs them, with IDs, when `CONFIG_THIRD_TIER_CLASSES` is on (it is). Four are class-wide, like the Phantom's: each
third-tier class's `pBattleAnimDef` (`Data/ClassData/ClassData.c`) is a list in `BattleAnims.event`
(`gBanimDef_T3Trickster`, `gBanimDef_T3Marshall`, `gBanimDef_T3Trueblade`, `gBanimDef_T3Archsage`): its promoted
class's vanilla list with the third-tier animation in place of the main weapon's entries. With the switch off, the
classes use their promoted class's vanilla list.

| ID | Third-tier class (from) | Weapon | Animation | Author |
|----|-------------------------|--------|-----------|--------|
| 0xDF | Trueblade, `CLASS_TRUEBLADE` (Swordmaster) | swords (unarmed: Swordmaster's 0x16) | "[Swordmaster-Variant] Trueblade [M]- Joshua" (1. Sword, Default) | Unknown |
| 0xE0 | none (installed, unused) | magic | "[Sage-Variant] Archsage [M]" (6. Magic) | Dinar87 |
| 0xE1 | Archsage, `CLASS_ARCHSAGE` (Sage) | anima, light, dark, unarmed (staves: Sage's 0x6E) | "[T3 Custom] Archsage + Intro [M]" (6. Magic, with a staff and an intro) | Nuramon |
| 0xE2 | Marshall, `CLASS_MARSHALL` (General) | lances (axes, swords, javelins, unarmed: General's) | "[Marshall] Zelgius v3 [M]" (2. Lance) | Nuramon |
| 0xE3 | Trickster, `CLASS_TRICKSTER` (Rogue) | knives (`ITYPE_KNIFE`, 0x0B) and swords (unarmed: Rogue's 0x90) | "[Rogue-Reskin] Trickster [M]" (1. Knife) | Leo_Link |

The Archsage uses Nuramon's animation: it is the one made for the third tier, holds a staff as the Archsage map sprite
does, and has an intro. Dinar87's is a Sage variant (a recolour of the vanilla Sage, closer to the map sprite's blue and
white). To use it instead, change the four `BANIM_T3_ARCHSAGE_NURAMON_MAGIC` entries of `gBanimDef_T3Archsage` to
`BANIM_T3_ARCHSAGE_DINAR87_MAGIC`. The Trickster's animation is the Rogue's sword animation reskinned (the Rogue's
"sword" is a dagger), so it plays for both of the class's weapon types. The Marshall's and the Trueblade's palettes are
their animations' own (Zelgius's red armour, Joshua's dark one), on every team.

The Halberdier (`CLASS_HALBERDIER`, 0x77, vanilla's unused class slot) is a promotion of the pre-promote Recruit
(`CLASS_RECRUIT_T1`), beside Recruit T2: `Data/ClassData/Promotions.event` writes both into vanilla's `gPromoJidLut`
in place of Paladin (F). Its class data is in `Data/ClassData/ClassData.c` (FE10 Halberdier caps, Recruit's map
sprite, Recruit T2's vanilla animations for anyone but Amelia), its texts at the end of `Districts.txt`.

The custom animations are in `Contents/BattleAnims/`. Vanilla's animation table is not changed: a new animation
gets an ID after vanilla's, and a character or a class is pointed at it through `CustomAnimeTable`.

---

## Switches

- `CONFIG_CUSTOM_BATTLE_ANIMS` (`include/configs/configs.h`) is the compile-time switch. It includes
  `Contents/BattleAnims/BattleAnims.event`; off, that file builds nothing.
- `KernelDesigerConfig.custom_character_animations` (`Data/DesignerConfig/designer-config.c`) is the runtime
  toggle (struct spelled as in the code). Off, a character keeps its class's animations.

Both are needed for a character's animation to show; the class-wide ones (Phantom, Gorgon Eggs) need only the
first.

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

## Reskinning a Vanilla Animation

`feditor_dump.py --rom ROM ID SRC_DIR OUT_DIR PREFIX ABBR` makes an animation from vanilla animation ID of a clean
`fe8.gba`: its modes, OAMs and script, with the sheets and palettes from `SRC_DIR` ("Sheet N.png", 264x64 mode-P, in the
order the script first uses them; "Palette.bin", 4 x 16 RGB555). Edit only the sheets' pixels: the OAM still cuts the
same tiles, so a new detail must stay inside the tiles the frame already shows.

For example, to make a character's own Armor Knight from vanilla's 0x40 (lance, three sheets) with the sheets edited
in `Source`:

```
python3 Tools/banim/feditor_dump.py --rom fe8.gba 0x40 Contents/BattleAnims/<Name>/ArmorKnight/Source Contents/BattleAnims/<Name>/ArmorKnight <Name>ArmorKnightLance name_akn
```

---

## From Frames

Some animations come as a FEBuilder-style script with one 248x160 PNG per frame instead of an FEditor dump: a `.txt`
with `/// - Mode N` headers, `Cxx` commands (0x850000xx), `D p- file.png` lines (show the frame for D ticks) and `~~~`
at each mode's end. Put the frames and their script in a `Source` folder and convert each script with (a script's
path picks it when the folder holds several):

```
python3 Tools/banim/febuilder_txt.py Contents/BattleAnims/<Name>/<Class>/Source/<Script>.txt Contents/BattleAnims/<Name>/<Class> <Prefix> <abbr>
```

The output is the same as `feditor_dump.py`'s (Huffman sheets, `<Prefix>_BANIM_ENTRY`), so steps 4-7 of
[Adding an Animation](#adding-an-animation) are the same. What the converter does:

- **Frames.** Every frame is a mode-P image with at most 16 colours, index 0 transparent and the same palette in
  every frame. The animation's origin is pixel (148, 88), as in FEditor's and FEBuilder's frames: the unit's feet
  are at about y 102, on the platform. The four palettes (player, enemy, NPC, other) are all the frames' palette.
- **OBJs and sheets.** Each frame is cut into OBJs over its non-empty 8x8 tiles (a greedy cover that trades empty
  tiles against the number of OBJs), and the OBJs are packed into 256x64 sheets in the order the script first shows
  the frames, so the sheet changes follow the script. An OBJ whose pixels are already on its sheet reuses them.
  `--sheets DIR` also saves the packed sheets as PNGs, to look at. An 87-frame axe swing took 17 sheets and about
  37 KB, a 35-frame throw 7 sheets and about 16 KB, 1-5 OBJs a frame.
- **Modes 2 and 4** (the layer behind the unit) are modes 1 and 3 with every frame empty, as FEditor makes them when
  nothing is drawn behind the unit.
- **The palette strip.** FEBuilder puts the palette in the last 8 columns of each frame (x 240-247, rows 0 and 1:
  entries 7..0, then 15..8). It is not part of the picture, and the converter leaves it out.
- **Back layer.** A 488x160 frame has a second picture at x 240-479, with the same origin: the part drawn behind
  the opponent (modes 2 and 4), such as an axe blade where it bites into them. Its OBJs go on the same sheet as
  the frame's front, because both layers draw from the side's one sheet in VRAM. A 248-wide frame's back layer is
  empty.
- **Missing modes.** Modes 1, 3 and 5-12 must all be in the script.
- **The class's list.** Give the character a copy of the class's vanilla list (`gBanimDef_<Name><Class>` in
  `BattleAnims.event`) with the new IDs in place of the weapon types they animate, and add the character and class to
  `CustomAnimeTable` (`Contents/External/Individual_animation_ea_2/data.event`).

---

## IDs and Free Room

Vanilla's `banim_data` (ROM `0xC00008`, 0x20 bytes an entry) has 201 animations, IDs 0x01-0xC9. The zero bytes after
it, up to `0xC02000`, hold the new entries: IDs **0xCA-0xFF**, 54 in all. An entry goes at
`0xC00008 + 0x20 * (ID - 1)`; `BanimEntry(id)` in `BattleAnims.event` writes it there.

Take a free ID: Gerik's sword is 0xCA, Tana's lance 0xCB, and so on up to the third-tier Trickster's 0xE3. 0xE4-0xE9
(Gilliam's Armor Knight, Dozla's Berserker and Garcia's Fighter, since removed) and 0xEA-0xFF are free: 28 in all.

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
- **Every animation's size** (uncompressed, inserted the usual way, and now) is in
  [CustomBattleAnimationSizes.md](CustomBattleAnimationSizes.md), written by `python3 Tools/banim/sizes.py`: the 30
  animations take 733.8 KB, against 970.5 KB the usual way (24.4% less) and 2512.3 KB uncompressed. Run it again
  after adding or converting an animation.
- **Measured but not used:** compressing the sheets together (LZMA, 26.6 KB for Gerik's) needs every sheet in RAM at
  once (96 KB); repacking frames into fewer sheets frees at most one (Gerik's sheets are 89% full and only 551 of
  2,749 tiles repeat, mostly on other sheets); a bit-packed LZ without Huffman coding gives 40.5 KB.

---

## Viewing in Game

The debugger (B on the map) has **Battle anim** on its page 3. It opens beside the debugger's menu, which stays on the
right and is blocked while the viewer runs: the map shows around both, and a window panel on the left holds one platform
with a battle animation attacking on it. Above it are the animation's ID, the class and its number among the classes
(`6/109`), and the variant (Generic or a character) and its number among the class's variants. Below it are Normal or
Critical and the keys. Every animation faces left, as the player's does in battle (its right-side OAM, the one the
mini engine plays with `state2` set). It opens on the selected unit: its character in its class, else its class.

The list is by class, in class order. Each class has its own animation first (**Generic**, with the class's own
palette), then every character with a `CustomAnimeTable` row for the class (so every custom animation is on it) or a
character palette for the class's own animation.

- **Left/Right** go to the previous or next class (wrapping); **L/R** go ten classes at a time. Holding a direction
  scrolls: the labels follow at once, and the animation loads once the selection has stayed for 6 frames.
- **Up/Down** step through the class's variants (wrapping within the class).
- **A** switches between the normal attack and the critical.
- **B** closes the viewer and goes back to the debugger's menu, as it was.

The attack plays to its end. The engine stops at each wait command (C01, C02 and C18: in battle, the HP bar draining),
and the viewer goes on 24 frames later (`sub_805A990`, as the promotion screen does). Once the mode has ended (the front
layer's `nextRoundId` is `0xFFFF`) it attacks again 40 frames later. Before, the viewer started the attack over at the
first wait, so nothing after it played. An animation that leaves the screen in battle leaves the panel too: the
Wyvern animation flies off the top and dives back, and several custom animations (Marisa's Assassin and Swordmaster,
Joshua's Myrmidon and Swordmaster, the Phantom, the Trueblade and the Trickster) vanish between strikes or flicker in
their criticals. That is in their art: a survey of every entry, frame by frame, found no other gaps.

The animation shown is the first weapon type in the entry's list that isn't unarmed, with the character palette when
the animation is the class's own. Mages and staff users cast with no spell shown: the class reel's spell for their weapon type is not run
(`magicFuncIdx` 0, the class reel's Dummy), so the cast animation plays and the attack goes on. BG1 and the map's BG2
are at priority 3 while the viewer is up, so the panel and the menu window are over the map and the animation and the
platform (priority 2 and 3) are over both. On leaving, the BG priorities and the BG0/BG1 tilemaps are put back.

It runs on the game's mini battle-animation engine, the one the class reel on the title screen uses
(`ClassInfoDisplay_Init`), with that screen's `AnimBuffer`, buffers and platform at the same addresses (battle-animation
work RAM, free outside a battle). The viewer's own data sits in the battle-script buffer (`gBanimScrLeft`, also free on
the map). While the viewer is up the map's unit sprites are hidden: `PutUnitSpritesOam` skips them, as it does while the chatlog
is up (`DebugBanimViewer_IsOpen`, `Kernel/Wizardry/MirrorMapSprites/MirrorSprites.c`), and the moving-unit sprites (the
selected unit's too) are hidden with `HideMu` and shown again with `ShowMu` on the way out. BG0 and BG1 are backed up when it opens and put back when it closes; the font's tile allocator is put back too. BG1 and the
map's BG2 are set to priority 3 while it is up (the panel over the map), and both are restored on the way out.
The code is `Kernel/Wizardry/VeslyDebugger/BanimViewer.c`.

---

## Testing

`python3 Tools/cloud/smoketest.py --only banim` (the test is `test_banim`, `Tools/cloud/smoketest.py`). It starts
from the prologue's turn 2 and fights once per animation, Eirika's unit made the character and class with a weapon of
the type (the unit gets the weapon rank). Each time it checks:

- the loaded animation is the custom one (`gBanimIdx` the ID minus 1: 0xC9 for Gerik's 0xCA, and so on);
- that side has no character palette (`gBanimUniquePal` -1).

Some fights take another route:

- Magic (L'Arachel as a Mage Knight or Valkyrie): tomes are spells here (`IsThreeHousesMagicItem`), so she casts Divine
  from B.Mag; high anima, light and dark ranks unlock it.
- Staff (L'Arachel as a Troubadour): W.Mag heals Seth, hurt for the test.
- Gorgon Egg: the enemies are made eggs with empty inventories, and Eirika attacks one; the egg is on the left.

It also checks the pre-promote Recruit's promotion choices (Recruit T2 and Halberdier), and saves each battle as
`<name>.png` in the test's folder. The third-tier animations aren't in the test (they are off).

---

## Code Locations

| What | Where |
|------|-------|
| Compile-time switch | `include/configs/configs.h` (`CONFIG_CUSTOM_BATTLE_ANIMS`) |
| Runtime toggle | `Data/DesignerConfig/designer-config.c` (`custom_character_animations`) |
| Entry point and lists | `Contents/BattleAnims/BattleAnims.event` |
| The animations (source and converted) | `Contents/BattleAnims/<Character>/<Class>/`, `Classes/` (class-wide), `ThirdTier/` |
| Class-wide animations | `Data/ClassData/ClassData.c` (`BANIM_DEF_PHANTOM`, `BANIM_DEF_GORGON_EGG`) |
| Converter | `Tools/banim/feditor_dump.py`, `Tools/banim/febuilder_txt.py` (frames), `Tools/banim/banimpack.py` (the compressors) |
| Sheet decoder | `Kernel/Wizardry/BanimCodec/` (`BanimUnComp`); the ARM loop from `Tools/banim/huffdec.c` |
| `CustomAnimeTable` rows | `Contents/External/Individual_animation_ea_2/data.event` |
| Lookup | `Kernel/Wizardry/FebCompat/IndividualAnim/Source/IndividualAnim.c` (`GetUnitBattleAnimDef`) |
| Battle setup | `Kernel/Wizardry/HPBarPalettes/HPBarPalettes.c` (`PrepareBattleGraphicsMaybe`) |
| Combo attacks | `Kernel/Wizardry/ComboAttack/Source/efx-combo.c`, `Kernel/Wizardry/BanimSync/Source/utils.c` |
| Test | `Tools/cloud/smoketest.py` (`test_banim`) |
