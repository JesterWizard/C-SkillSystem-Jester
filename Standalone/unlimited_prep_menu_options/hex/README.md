# Hex (standalone add-on: the Hex minigame in the Preparations menu)

The Hex card game for FE8U, as an add-on to the [Unlimited Prep Menu Options](../README.md) patch. A clean FE8U ROM with both patches has a **Hex** row in the Preparations menu. It opens the Hex screen: Play (pick one of your living units as the opponent, pick a hand of 7 cards, play a match) or Cards (your collection). Winning a match can give you a card of the opponent's, and losing can cost you one. Cards are ordinary vanilla items in the convoy and the units' inventories.

Integrated documentation: [Documentation/Features/Minigames/Hex.md](../../../Documentation/Features/Minigames/Hex.md) (rules, card list and the kernel's version).

## Behaviour

### The Hex row

- Title `Hex`, help text "Play Hex with your army / and view your cards." (the help is a C string; the line break is a `\n` in `Source/HexPrep.c`). It is always shown and never grey.
- A on the row plays the window select sound, then the Hex menu opens: Play and Cards, on the rune mural with the blue list window (the prep sub-screen look).
- B on the Hex menu leaves for the Preparations menu. The cursor is back on the Hex row, with no popup.

### The Hex menu

- **Play** opens "Choose an opponent": the living player units (name and level). The highlighted unit's deck is shown in the red column (on the left). A starts the hand picker. B goes back.
- **Cards** opens the collection: "Cards N/32" and the 32 card types in table order. Owned types show the copies; a type never owned shows `???`. The highlighted owned card is previewed. B goes back.
- The first time the screen opens with no cards, the player gets the 7 starter cards in the convoy (`HexCol_GiveStarters`: Ross 2, Amelia 2, Ewan 1, Franz 1, Neimi 1).

### The hand picker and the match

- The picker lists your owned types with the copies still available. A adds a copy to the hand (7 cards), B removes the last one. With 7 cards, "Accept this hand?" appears: Accept starts the reveal (the opponent's hand is shown; A skips it) and then the match. Change goes back to the picker.
- A match is 14 placements on the 14-cell board, blue and red in turns. Blue chooses a card (UP/DOWN, A) and a cell (D-pad, A places, B goes back). The first player is chosen by the game clock. Red's AI level is set by the opponent's effective level (the unit's level, plus 20 if promoted): 1 below 10, 2 from 10 on. Red never plays at level 0 (random moves). Level 1 takes the best value (the score difference right after the move); level 2 looks one reply ahead. Levels 1 and 2 break a tie between equally valued moves by the damage done to blue's cards (a captured card counts its max HP), then by blue's cards next to the placed cell, then by the game clock seed.
- Layout: the player's hand (blue) and score are on the right, the opponent's hand (red) and score on the left. The arrows point at the choice: a right arrow beside the list row or the highlighted blue card, and a down arrow on the top edge of the cell being chosen (the same place in every cell). In TAKE a left arrow sits just right of the opponent's card.
- RESOLVE: 66 frames (`HEX_RESOLVE_FRAMES` = 30 + 16 + 20); the name bar shows the card just placed. For the first 30 frames (`HEX_ATTACK_DELAY`) the board and both scores are the ones from before the placement, with the placed card on its cell at full HP. At frame 30 the attack shows: the flipped cells in their new colour, the lost HP, and a hit sound (`HEX_SE_HIT` = 0xD4, the battle hit sound) if anything changed. For 16 frames from there the cards that lost HP, changed owner, or (the placed card) were counterattacked shake sideways (`sHexShake`). Then 20 frames of the settled board. The hands always show the real state.
- At the end, the banner says Victory, Defeat or Draw. A goes to the stakes:
  - **Win:** if the opponent's starting cards can be taken (you hold fewer than 3 copies of their type), Take shows them, the opponent's cards on the left, with a left arrow just right of the chosen one. UP/DOWN chooses and A takes one into the convoy. A held type of 3 copies, or a full convoy, is refused with a message. With a full convoy, B skips the take.
  - **Loss:** if you hold more than 7 cards, your strongest card of the hand is taken. Otherwise "Your cards are safe this time."
  - **Draw:** nothing changes.
- The stake banner stays for 120 frames (or until A), then the menu comes back.

### Cards as items

Unchanged from the card add-on: a card is one vanilla item slot `$C1` (a "Dummy" item) whose uses byte is the card type (0 to 31). Its name and description are the character's, it has the Hex card icon (see "Card icon" below), and it cannot be used. The Supply list shows it with `--` uses. R on a card shows the character's description. See the card section of the test notes in [test/README.md](test/README.md).

### Card icon

The card has its own 16x16 icon: a small hexagonal card (dark outline, gold border, blue face, white hexagon pip). It replaces nothing else; it is written into a vanilla icon slot that the clean ROM does not use.

- **How it is added:** reuse of a free slot (option (a) in the task notes), not a hook. The item icon sheet is the table at `0x085926F4` (`$5926F4` in the file), 0x80 bytes per icon id, 0xE0 ids (`0x00` to `0xDF`). `DrawIcon` (`0x080036BC`) takes the id from `GetItemIconId` (`0x08017700`) and copies `table + id * 0x80` to VRAM through `GetIconTileIndex` (`0x08003650`, the only reader of that table; `LoadIconObjectGraphics` `0x0800372C` has no caller). The installer writes the 128 bytes of `Source/HexCardIcon.bin` into slot `HEX_CARD_ICON` = `$DF` (file `$599674` to `$5996F3`, protected). No code is hooked for the icon.
- **HEX_CARD_ICON:** `#define HEX_CARD_ICON $DF` in `Installer.event` (no C side: the C code does not use the icon id). The item row's icon byte (`gItemData[0xC1]` +`0x1D`, file `$80B651`) is set to it.
- **Why slot $DF is free (evidence from the clean ROM):**
  - No item row's icon byte is `$DF` (the 0xCE rows, scanned with the file offsets from `$809B10`).
  - The 40 `DrawIcon` call sites take the id from: the item rows (`GetItemIconId` or a direct `ldrb` of +`0x1D`), `GetUnitAidIconId` (`$80`, `$81`, `$82` or `-1`), the affinity icons (`$79` plus the character's affinity byte; the character table's byte at +9 is at most 29 in rows `0..0x1FF`), or the weapon-type icons (`$6F`/`$70` plus a small index). None gives `$DF`.
  - In the vanilla icon sheet, `$DE` and `$DF` are the red-cross placeholder tiles (checked on the rendered sheet).
- **Palette:** the item icons use OBJ bank 4 (`DrawIcon` with `0x4000`, from `LoadIconPalettes(4)`), palette 0 of the icon table at `$5996F4` (16 BGR555 colours, index 0 transparent). The colours used are index 4 (outline), 5 (gold), 7 (blue) and 1 (white). The generator prints the mapping.
- **Regenerate:** `make` in this folder rebuilds `Source/HexCardIcon.bin` when `tools/gen_card_icon.py` changes (`python3 tools/gen_card_icon.py` by hand does the same). It writes `tools/card_icon_preview.png` (8x) as well when Pillow is installed. Edit the shape or the colours in the script (the `PIP_R`, `R` and `WANTED` values), then run it. The 4bpp layout is GBA 1D OBJ: tile 0 top-left, 1 top-right, 2 bottom-left, 3 bottom-right, 32 bytes each, low nibble left.
- **Conflicts:** another patch that writes icon slot `$DF`, or that reads it as an item icon (any item row with icon `$DF`), gets the card's picture. Any patch that replaces the icon sheet at `$5926F4` conflicts with this one. If `$DF` is taken, pick another free slot in `Installer.event` (`HEX_CARD_ICON`) and check it against the list above.
- **Not proven:** that no other add-on in the repo writes `$DF`, and that the weapon-type index in the `$6F`/`$70` callers stays below `$10`. The clean ROM checks are static (BL and literal-pool scans); a patch that computes ids differently is not covered.

## Target ROM

- **FE8U (USA)**, clean ROM. The header reads `FIREEMBLEM2EBE8E`.
- Not for the integrated C Skill System kernel. Do not install over `Kernel/Wizardry/Minigames/Hex` or `Kernel/Wizardry/SkillSys/PrepSkill`.

## Needs the parent patch

Install [Unlimited Prep Menu Options](../README.md) **first, in the same Insert EA run** (one ColorzCore run with both installers, or FEBuilder's Insert EA on the parent and then on this folder's `Installer.event` on the same ROM). The Hex row is written into the parent's extension slot. Without the parent's `PMX1` header the slot write goes into unformatted ROM and nothing shows in the menu.

## Install

The folder is self-contained (it does not need `EAstdlib.event` or `Hack Installation.txt`; FEBuilder provides `jumpToHack`).

### FEBuilderGBA

1. Open a clean FE8U ROM.
2. **Advanced Editors → Insert EA** on `../Installer.event` (the parent).
3. On the same ROM, **Advanced Editors → Insert EA** on this folder's `Installer.event`.

### Event Assembler (command line)

Both steps write the same ROM. The parent's step is first, then this one. The parent's README has the same commands for the parent alone.

```bash
EA_DIR=<repo>/Tools/EventAssembler
ROM=<somewhere>/fe8-hex.gba
cp <clean fe8.gba> "$ROM"
cd <repo>/Standalone/unlimited_prep_menu_options
{ echo '#include "Extensions/Hack Installation.txt"'; echo '#include "Installer.event"'; } | "$EA_DIR/ColorzCore" A FE8 -I:"$EA_DIR" -output:"$ROM"
cd hex
{ echo '#include "Extensions/Hack Installation.txt"'; echo '#include "Installer.event"'; } | "$EA_DIR/ColorzCore" A FE8 -I:"$EA_DIR" -output:"$ROM"
```

The tested path is one run, as the parent's test does (`test/make_test_rom.sh`). Two runs on one ROM should also work, because the parent's protection does not cover `$1100000` or the slot word, but the two-run order was not tested with the Hex row.

## Build

No build step is needed to install. `Source/Hex.lyn.event` is checked in. To rebuild after editing a C file in `Source/` or `Source/RamAlloc.s`:

```bash
make -C Standalone/unlimited_prep_menu_options/hex
```

This needs devkitARM, [FE-CLib](https://github.com/MokhaLeee/FE-CLib-Mokha) (`Tools/FE-CLib-Mokha`) and the repo's `lyn` (`Tools/EventAssembler/Tools/lyn`). The build uses `-Wall -Wextra -Werror -O2`. It needs the parent's `Source/PrepMenuExt.h` (included by relative path from `Source/HexPrep.c`). The makefile never runs `make -B` or touches `Tools/FE-CLib-Mokha/reference`.

## Files

| File | Purpose |
|------|---------|
| `Installer.event` | Vanilla addresses, `HEX_CARD_ITEM`, `HEX_CARD_ICON`, `HEX_PREP_SLOT`, the item row, the card icon (`#incbin`), the two item hooks, the body at `$1100000`, the slot write |
| `Source/Hex.h` | `HEX_CARD_ITEM`, `HEX_PREP_SLOT`, `struct HexCardType`, the card and art declarations, `gHexScreenState` |
| `Source/HexCards.c` | Card item tests, `MakeHexCardItem`, convoy and inventory collection |
| `Source/HexCardTypes.c` | The 32 card types (`gHexCardTypes`) |
| `Source/HexItem.c` | `HexItem_GetItemName` and `HexItem_GetItemDescId`, the two item hooks |
| `Source/HexCore.h`, `Source/HexCore.c` | The rules engine and the AI (`Hex_Init`, `Hex_Place`, `Hex_AiChoose`, ...). Plain C, no game globals |
| `Source/HexArt.c` | The card art, UI tiles and palettes (data only). Its generated block is from `Tools/hex/gen_hex_art.py` in the repo |
| `Source/HexScreen.c` | The Hex screen: menu, opponents, hand picker, match, stakes, the Cards viewer, the windows and the sprites |
| `Source/HexPrep.c` | The Preparations row (`HexPrepRow`, a `struct PrepMenuExtRow`) |
| `Source/RamAlloc.s` | `gHexScreenState` at `0x0203C100` (0x200 bytes) |
| `Source/Hex.lyn.event` | Checked-in lyn output of all the C files and `RamAlloc.s`, included by the installer |
| `include/configs/configs.h` | Empty stand-in for the kernel config header that FE-CLib's `bmunit.h` includes |
| `Source/HexCardIcon.bin` | The card icon: 128 bytes of 4bpp tiles (16x16, OBJ order), incbin'd by `Installer.event`. Made by `tools/gen_card_icon.py` |
| `tools/gen_card_icon.py` | Draws the card icon, maps its colours to the item icon palette (prints the mapping), writes `Source/HexCardIcon.bin` and the preview |
| `tools/card_icon_preview.png` | The icon at 8x (made by the script) |
| `makefile` | Builds `Source/Hex.lyn.event`, and `Source/HexCardIcon.bin` from `tools/gen_card_icon.py` |
| `test/` | Test-only route, script and ROM builder. Do not install (see `test/README.md`) |

## Hooks and free space

| Vanilla function | ROM offset | Overwritten | Replacement |
|------------------|------------|-------------|-------------|
| `GetItemName` (0x080174F4) | `$174F4` | 8 bytes | `HexItem_GetItemName` |
| `GetItemDescId` (0x08017518) | `$17518` | 8 bytes | `HexItem_GetItemDescId` |

The Hex row adds no hook. It uses the parent's extension slot. The card icon adds no hook either: it writes the 0x80 bytes of vanilla icon slot `$DF` (see "Card icon").

| What | ROM offset | Size | Note |
|------|------------|------|------|
| Item row `gItemData[0xC1]` | `$80B634` | 36 bytes | Protected. The card's fields (icon byte `$DF` at `$80B651`) |
| Card icon, icon slot `$DF` | `$599674` | 128 bytes | Protected. Overwrites the vanilla icon slot `$DF` (a placeholder) with `Source/HexCardIcon.bin` |
| Extension slot `HEX_PREP_SLOT` | `$1000004` (slot 0) | 4 bytes | Written with `POIN HexPrepRow`. Not protected (the parent leaves the slots open) |
| Body (free space) | `$1100000` to `$110CC0B` | **52236 bytes (0xCC0C)** | Protected from `$1100000` to the end of the row. Holds the card code and table, the engine, the art, the screen, the row |

The body is at `FE8U_HexFreeSpace` = `$1100000`, far from the parent's `$1000000` to `$1001060` and its `$1000024` body. The ROM grows to 17878028 bytes (the 16 MiB ROM plus the body).

## Item slot and how to change it

The card uses vanilla item `0xC1`.

- Entry `0xC1` is named by the text `0x0403` ("Dummy") and described by `0x04AB`. Its weapon type is 9 (`ITYPE_ITEM`).
- Not proven: that no vanilla shop, chest, unit or event table uses `0xC1`.

To use another slot: pick an index from `0x00` to `0xCD` that no other patch uses, set `#define HEX_CARD_ITEM` in **both** `Installer.event` and `Source/Hex.h`, and rebuild with `make`. Any index past `0xCD` writes into terrain data.

## Preparations row and slot

- **Slot:** `HEX_PREP_SLOT` (0 to 7) in `Installer.event` is the extension slot of the Hex row. The row sits after the vanilla rows, in slot order. The key and state are `0x40 + slot` (`0x40` for slot 0).
- **To change the slot:** set `#define HEX_PREP_SLOT n` in **both** `Installer.event` and `Source/Hex.h`, then rebuild with `make`. The C code uses the same value for the cursor restore (`cur_cmd`) and the state, so the two must match.
- **Another add-on in the same slot** overwrites this row. Pick a slot that is 0 in the parent's header (`$1000004 + 4*n`) before this install.
- The row's text is in `Source/HexPrep.c` (`"Hex"` and the help). The help uses `\n` for the second line.

## RAM

| Symbol | Address | Size | Use |
|--------|---------|------|-----|
| `gHexScreenState` | `0x0203C100` to `0x0203C2FF` | 0x200 bytes | `struct HexScreenState` (the match, the lists, the text objects). Only used while the Hex screen is open |

Set in `Source/RamAlloc.s`. The parent uses `0x0203C000` to `0x0203C013`. The area is inside the free EWRAM the standalones use (`FreeRamSpace2`, `0x0203AAA4` to `0x0203DDE0`). The compile-time check `HEX_STATE_FITS` in `HexScreen.c` fails if the state outgrows 0x200 bytes. To move it, change the address in `RamAlloc.s` and rebuild. Other add-ons in this range: none in the tree (`portrait_formats` uses `0x0203B000` to `0x0203B89F`, `language_select` uses `0x0203BC00`, `alpha_blend_movement_sprites` uses `0x0203AAA4`).

While it is open the screen uses OBJ tiles 0 to 567 from `0x06010000` and the OBJ palette banks 0 to 15, clears the four BG tilemaps, and draws the BG0 text, the BG1 windows and the BG3 mural (the same layout as the other prep sub-screens).

## Conflicts

- Any patch that hooks `GetItemName` or `GetItemDescId`, or uses item `0xC1`, the parent's hook sites (see the parent's table), or the Preparations menu's extension slot `HEX_PREP_SLOT`.
- The kernel's Hex (`Kernel/Wizardry/Minigames/Hex`, `PrepSkill`), which uses the same card item and the same prep menu. Do not install both.
- Free space `$1100000` to `$110CC0B`: another standalone body there would overlap.
- EWRAM `0x0203C100` to `0x0203C2FF`: another patch that uses that range.

## Verification status

Checked in session S2b with the parent and this folder in one Event Assembler run (`test/make_test_rom.sh`, output `No errors. Please continue being awesome.`):

- `make` in this folder: passes (`-Wall -Wextra -Werror -O2`). `make clean && make` rebuilds it.
- Undefined symbols: every external name of the eight objects (seven C files and `RamAlloc.s`) is defined either by the FE8 reference object (`Tools/FE-CLib-Mokha/reference/fireemblem8.ref.o`) or by this folder's objects. None is left unresolved.
- The 12 parent hooks hold `00 4B 18 47` and a pointer into the parent's body (`$1000000` area). The two item hooks hold `00 4B 18 47` and a pointer into `$1100000`. Slot 0 holds `0x0910CBF4`, the address of `HexPrepRow` (`$110CBF4`), whose title is `Hex` and help `Play Hex with your army\nand view your cards.`, with `isVisible` NULL.
- Emulator (mGBA via gbarun, route in `test/README.md` and `test/hex_screen.gbarun`): the Preparations menu scrolled to the Hex row with its help text; the Hex menu on the mural with blue windows; "Choose an opponent" with the chapter's units and the highlighted unit's deck; the hand picker and the accepted hand; mid-match board; the result banner; the Cards viewer; the convoy's Supply list with the starter cards; B from the Hex menu returns to the Preparations menu with the cursor on Hex and no popup.
- A win (the Take step and "Gained Ewan!") was checked in an earlier run of the same route; the screenshots kept are of a loss run (the match result is not the same on every run, see `test/README.md`)

Checked in session S3 (the card icon, `make` and `test/make_test_rom.sh` as above):

- `make` rebuilt nothing in `Source/` (the C is unchanged) and ran the icon rule when `tools/gen_card_icon.py` was touched: the same 128 bytes. `make` passes.
- The test ROM builds with `No errors. Please continue being awesome.`. Against the previous build (the same route with the Member Card icon), the only changed bytes are the item row's icon byte (`$80B651`: `$84` to `$DF`) and the icon slot `$599674` to `$5996F3`.
- The 12 parent hooks hold `00 4B 18 47` (the PMX1 header at `$1000000` is the 13th site, not a hook). The two item hooks and slot 0 are as above.
- Emulator, `test/cards_supply.gbarun`: the card rows show the hex icon next to the names; `Lightning`, `Vulnerary`, the weapon-type tabs at the top of the Supply list and the other icons are pixel-identical to the previous build (pixel diff: only the card icon cells change).
.

## Credits

The Hex minigame, its rules and its AI come from [C Skill System](https://github.com/JesterWizard/C-SkillSystem-Jester) (`Kernel/Wizardry/Minigames/Hex`, `include/kernel/hex-core.h`). The card table's stats are copied from the Minish Cap mod's `gHexTiles` (`src_custom/hexArt.c`), as the kernel's table says. The card art is generated from the Minish Cap mod's art by `Tools/hex/gen_hex_art.py`. The card icon is drawn by `tools/gen_card_icon.py` and uses the item icon palette of FE8U (the vanilla Member Card icon is no longer used). The Preparations row and its extension contract come from [Unlimited Prep Menu Options](../README.md).
