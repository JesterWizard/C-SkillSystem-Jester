# Hex Cards (standalone add-on)

Hex cards as vanilla items, for FE8U. A card is one item in a dummy vanilla item slot. Its name is the name of the character it is named after, and it has no uses number. This add-on also holds the card collection helpers that the Hex screen will use. The Hex screen itself (the minigame) is not in this add-on; it comes in the next task. Integrated documentation: [Documentation/Features/Minigames/Hex.md](../../../Documentation/Features/Minigames/Hex.md) (see "Cards as Items" and "Card List").

## Behaviour

- **Card item.** One vanilla item slot, `HEX_CARD_ITEM` = `$C1` (a "Dummy" item, see [Item slot](#item-slot-and-how-to-change-it)). The item word is `(type << 8) | 0xC1`, so the card type (0 to 31, the row of the card table) is the uses byte. A type past 31 is shown as type 0 (Ross).
- **Name.** The card's name is the name of its character: `GetCharacterData(pid)->nameTextId` (for example "Ross", "L'Arachel").
- **Help text.** R on a card in a list shows the character's description: `GetCharacterData(pid)->descTextId`. Test: "A young villager. He longs to be a warrior, like his father." for Ross.
- **Icon.** The vanilla "Member Card" icon (`$84`), a card picture.
- **Uses.** The item is unbreakable (`IA_UNBREAKABLE`), so vanilla `GetItemUses` returns `0xFF` and no uses number is drawn. The Supply list shows `--`. Max uses is 1, cost is 0, and there is no use effect (a card cannot be used).
- **Other items.** The two hooks keep the vanilla result for every other item (see [Hooks](#hooks-and-free-space)).
- **Collection helpers** (`Source/Hex.h`, all non-static): `IsHexCardItem`, `MakeHexCardItem`, `HexCard_Type`, `HexCard_NameId`, `HexCard_DescId`, `HexCol_Count`, `HexCol_Total`, `HexCol_GiveToConvoy`, `HexCol_TakeOne` (convoy first, then a living unit's inventory), `HexCol_GiveStarters` (7 starter cards into the convoy, only when the player owns none). They use the vanilla convoy (100 slots) and `gUnitArrayBlue`.
- **Card table.** `gHexCardTypes[32]` (attack, max HP, reach, tier, starter count, character `pid`), ported from `Data/Hex/HexCardTypes.c`.

Tested in an emulator: a convoy with card items lists them by character name, R shows the help text, and taking them moves them into a unit's inventory (see `test/README.md`).

## Target ROM

- **FE8U (USA)**, clean ROM. The header reads `FIREEMBLEM2EBE8E`.
- Not for the integrated C Skill System kernel. Do not install over `Kernel/Wizardry/Minigames/Hex` or `ItemInfoRemap`.

## Needs the parent patch

Install [Unlimited Prep Menu Options](../README.md) first, in its own Insert EA run. The Hex row and the Hex screen (next task) live in the prep menu that the parent builds. The two patches do not overlap (see [Hooks](#hooks-and-free-space)). This add-on does not call any code of the parent, and `Installer.event` also assembles on its own, so the order is a design requirement, not an assembler one.

## Build

No build step is needed to install. `Source/Hex.lyn.event` is checked in. To rebuild after editing a C file in `Source/`:

```bash
make -C Standalone/unlimited_prep_menu_options/hex
```

This needs devkitARM, [FE-CLib](https://github.com/MokhaLeee/FE-CLib-Mokha) (`Tools/FE-CLib-Mokha`) and the repo's Event Assembler `lyn` (`Tools/EventAssembler/Tools/lyn`, see [Documentation/Setup.md](../../../Documentation/Setup.md)). The makefile uses `include/configs/configs.h`, an empty stand-in in this folder (FE-CLib's `bmunit.h` includes it), so the build reads no kernel header.

## Install

The folder is self-contained. It does not need `EAstdlib.event` or `Hack Installation.txt` (FEBuilder provides `jumpToHack`).

### FEBuilderGBA

1. Open a clean FE8U ROM and install the parent patch: **Advanced Editors → Insert EA** on `../Installer.event`.
2. On the same ROM, **Advanced Editors → Insert EA** on this folder's `Installer.event`.

### Event Assembler (command line)

From a copy of the clean ROM, run the parent step first, then this step on that output. Run both in their folders (the `Source/` paths are relative):

```bash
EA_DIR=<repo>/Tools/EventAssembler
ROM=<somewhere>/fe8-hex-cards.gba
cp <clean fe8.gba> "$ROM"
cd <repo>/Standalone/unlimited_prep_menu_options
{ echo '#include "Extensions/Hack Installation.txt"'; echo '#include "Installer.event"'; } | "$EA_DIR/ColorzCore" A FE8 -I:"$EA_DIR" -output:"$ROM"
cd hex
{ echo '#include "Extensions/Hack Installation.txt"'; echo '#include "Installer.event"'; } | "$EA_DIR/ColorzCore" A FE8 -I:"$EA_DIR" -output:"$ROM"
```

Both steps write the same `$ROM`, so the parent's patch is in it before the hex patch. The parent's own README gives the parent's step, which is the first command pair above.

## Files

| File | Purpose |
|------|---------|
| `Installer.event` | Vanilla addresses, `HEX_CARD_ITEM`, `ICON_CARD`, the item row, the two hooks, the free-space body |
| `Source/Hex.h` | `HEX_CARD_ITEM`, `struct HexCardType`, the exported helpers |
| `Source/HexCards.c` | Card item tests, `MakeHexCardItem`, convoy and inventory collection |
| `Source/HexCardTypes.c` | The 32 card types (`gHexCardTypes`) |
| `Source/HexItem.c` | `HexItem_GetItemName` and `HexItem_GetItemDescId`, the two hook functions |
| `Source/Hex.lyn.event` | Checked-in lyn output of the three `.c` files, included by the installer |
| `include/configs/configs.h` | Empty stand-in for the kernel config header that FE-CLib includes |
| `makefile` | Builds `Source/Hex.lyn.event` from the `.c` files with `-Wall -Wextra -Werror` |
| `test/` | Test-only route and script. Do not install (see `test/README.md`) |

## Hooks and free space

| Vanilla function | ROM offset | Overwritten | Replacement |
|------------------|------------|-------------|-------------|
| `GetItemName` (0x080174F4) | `$174F4` | 8 bytes | `HexItem_GetItemName` |
| `GetItemDescId` (0x08017518) | `$17518` | 8 bytes | `HexItem_GetItemDescId` |

Each hook is `jumpToHack` (`00 4B 18 47` = `ldr r3,[pc,#0]; bx r3`, then a pointer). The vanilla function starts at a 4-byte-aligned address, so the literal is aligned.

- `HexItem_GetItemName`: a card returns `GetStringFromIndex(HexCard_NameId(item))`. Any other item does what vanilla does: `GetStringFromIndex(GetItemData(item & 0xFF)->nameTextId)`, then `StrInsertTact()`, and returns that value (vanilla 0x080174F4).
- `HexItem_GetItemDescId`: a card returns `HexCard_DescId(item)`. Any other item returns `GetItemData(item & 0xFF)->descTextId` (vanilla 0x08017518).

Other data written:

| What | ROM offset | Size | Note |
|------|------------|------|------|
| Item row `gItemData[0xC1]` (`$809B10 + $C1 * 36`) | `$80B634` | 36 bytes | Protected. The card's fields; see the item slot section |
| Free space (body) | `$1100000` to `$11004A3` | 1188 bytes | Code and `gHexCardTypes`. Protected from `$1100000` to `CURRENTOFFSET` |

**Free space** starts at `$1100000` (`FE8U_HexFreeSpace`), not at `$1000000`, which the parent uses (`$1000000` to `$1001060`). The standalone checklist says later patches continue after the earlier body with `CURRENTOFFSET`. This add-on uses a fixed `$1100000` by decision, so it stays clear of the parent's body and the parent's extension slots. The ROM file grows to about 17.8 MB, as with the parent's `$1000000`.

## Item slot and how to change it

The card uses vanilla item `0xC1`. Its row was checked as follows (`Contents/Texts/Source/vanilla.txt`, FE-CLib `constants/items.h`, the kernel's `include/configs/configs.h`):

- The vanilla item table `gItemData` is at `$809B10`, 36 bytes per item. In this ROM the item data ends at index `0xCD`: the terrain tables start at `$80B808`, which is `gItemData + 0xCE * 36`, and the bytes there are not item rows. So the kernel's card index `0xEC` is past the end of the vanilla table and cannot be used by this add-on.
- Entry `0xC1` is named by the text `0x0403`, which is "Dummy" in the vanilla text (`MSG_0403`). Its description is `0x04AB`, "DUMMY". Its weapon type is 9 (`ITYPE_ITEM`), and it has no attributes, no uses, no cost and no icon. FE-CLib's header names it `ITEM_STAFF_ACUITY`, a staff from cut content, but the vanilla entry is a dummy.
- No kernel config claims `0xC1`. The kernel's `include/configs/configs.h` reserves `0x0A` and `0xBD`, `0xC9`, `0xCA` (skill scrolls), `0xBC` (magic booster), `0xBE` (skill stealer) and `0xDA` (arms scroll). The kernel's item table is at the same address, `$809B10` (`CONFIG_PR_ITEM_TABLE`).
- **Not proven:** that no vanilla shop, chest, unit or event table uses `0xC1`. The table entry, the text and the constants were checked. A raw search of the ROM for the index is too noisy to prove absence.

To use another slot: pick a dummy index from `0x00` to `0xCD` that is not used by another patch, set `#define HEX_CARD_ITEM` to it in **both** `Installer.event` and `Source/Hex.h`, and rebuild with `make`. Any index past `0xCD` writes into terrain data.

## RAM

None. This add-on uses no EWRAM or IWRAM. The convoy and the unit list are read through vanilla functions and `gUnitArrayBlue`. The test route writes RAM in the emulator only (`test/README.md`); that is not part of the install.

## Conflicts

- Any patch that hooks `GetItemName` or `GetItemDescId`, including the kernel's `ItemInfoRemap` (`DuraItem.c`). Do not install both.
- Any patch that uses vanilla item `0xC1` (a dummy-item or staff patch). Both write the same item row, and the one installed last wins.
- Free space `$1100000` to `$11004A3`. Another standalone body there would overlap.
- The parent's hook sites and free space do not overlap this add-on.
- Keep the parent's prep-menu rows and this add-on separate in the installer: nothing here changes the prep menu. The Hex row arrives with the next task.

## Verification status

- `make` in this folder: passes (`-Wall -Wextra -Werror -O2`). A clean rebuild gives a `Source/Hex.lyn.event` byte-identical to the checked-in file.
- Event Assembler: the parent and this add-on in one run, with `test/make_test_rom.sh`: `No errors`. The parent then this add-on as two runs on one ROM: `No errors` for both. This add-on alone: `No errors`.
- Hook sites hold `00 4B 18 47` followed by a pointer into `$1100000` (`0x09100419` for `GetItemName`, `0x09100469` for `GetItemDescId`). The item row at `$80B634` holds `c1`, `09`, `08 00 00 00` (`IA_UNBREAKABLE`), `01` (max uses), icon `84` at `+0x1D`.
- Emulator, test ROM (see `test/README.md`): the Supply list shows the seven test cards with their character names and `--`; R on Ross shows Ross's description; taking Ross and Amelia moves them into a unit's inventory with their names; the vanilla Iron Sword keeps its name and `46` uses.

## Credits

The Hex minigame and its card rules come from [C Skill System](https://github.com/JesterWizard/C-SkillSystem-Jester) (`Kernel/Wizardry/Minigames/Hex`). The card table's stats are copied from the Minish Cap mod's `gHexTiles` (`src_custom/hexArt.c`), as the kernel's table says. The card icon is FE8's vanilla "Member Card" icon.
