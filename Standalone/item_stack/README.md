# Item Stack

Adds a **Stack** command to the unit action menu, after Arena. It lists each item id that appears at least twice in the unit's inventory. Choosing one merges those copies into the first slot and adds their uses together.

The command is hidden unless a merge is possible. A merge is possible when:

- At least two slots hold the same item
- The items are not unbreakable
- The items are not gold or a bullion (`ITEM_GOLD`, `1G` through `5000G`)
- The combined uses are from 1 to 255

Selecting the last stackable item closes the submenu. The unit does not spend their turn. Integrated `designer-config.c` defaults this flag to `true`.

## Configuration

Edit `ITEM_STACK` at the top of `Installer.event`, then reinstall the patch. No C recompile is needed.

| Constant | Default | Meaning |
|----------|---------|---------|
| `ITEM_STACK` | `1` | Set to `0` to hide the Stack command |

## Target ROM

- **FE8U (USA)** clean ROM
- Hook: `gUnitActionMenuDef.menuItems` pointer at `$59D1F8` (4 bytes)
- Free space: `$1000000`
- The command name is a raw ASCII string (` Stack`); `nameMsgId` is 0 so the menu draws it without `GetStringFromIndex`.
- R-text uses text ID `$407` (unused vanilla dummy item-name slot) with **vanilla Huffman** bytes: *Combine duplicate items into / one stack.* Do not set the anti-Huffman high bit (`pointer | 0x80000000`); clean FE8U `DecodeString` has no high-bit check and will crash if R-help is uncompressed ASCII. `TEXT_STACK_DESC` in `Installer.event` and `STACK_HELP_TEXT` in `Source/StackCommand.c` must stay the same ID.

## Build

No build step is required to install the patch. `Source/StackCommand.lyn.event` is checked in, so you can install `Installer.event` as-is.

Run `make` only if you edit `Source/StackCommand.c`:

```bash
make -C Standalone/item_stack
```

That requires [devkitARM](https://devkitpro.org/wiki/Getting_Started) and this repo's [FE-CLib](https://github.com/MokhaLeee/FE-CLib-Mokha) / [Event Assembler](https://github.com/MokhaLeee/EventAssembler/tree/mokha-fix) tools. See [Documentation/Setup.md](../../Documentation/Setup.md) for full setup.

## Installation

This folder is self-contained. It does not include `EAstdlib.event` or `Hack Installation.txt`. FEBuilder’s bundled Event Assembler is enough.

### FEBuilderGBA

1. Copy the `item_stack` folder (or download this standalone package).
2. Open a clean FE8U ROM in FEBuilder.
3. Go to **Advanced Editors → Insert EA**.
4. Click **Select File**, choose `Installer.event`, then click **Load Script**.

### Event Assembler

From a copy of clean `fe8.gba`, with this folder as the working directory:

```bash
ColorzCore A FE8 -input:Installer.event -output:/path/to/fe8-item-stack.gba
```

## Files

| File | Purpose |
|------|---------|
| `Installer.event` | EA installer, addresses, text ID, and free-space placement |
| `Menu.event` | Vanilla unit menu commands plus Stack |
| `Text.event` | R-button help string |
| `Source/StackCommand.c` | Usability, submenu, and merge |
| `Source/StackCommand.lyn.event` | Checked-in lyn output (rebuild with `make` after editing `.c`) |
| `makefile` | Compiles C to `.lyn.event` |

## Conflicts

- Any patch that repoints the unit action menu item table (`$59D1F8`), including the full C Skill System unit menu, Skill System FE8 `UnitMenu`, and `Standalone/refuge/`.
- Text ID `$407` (unused vanilla dummy item name). Do not use IDs past the vanilla message table (`$E4B`); that overwrites Huffman data and will crash when the action menu draws.
- Any anti-Huffman / “uncompressed text pointer” patch is unnecessary for this installer. The R-text is already Huffman-compressed for vanilla `GetStringFromIndex`.

`Installer.event` uses `PROTECT` on the menu pointer, the `$407` text-table slot, and the free-space body (`$1000000` through end of install). If another patch overlaps those ranges, Event Assembler should report the conflicting write location.

## Credits

Extracted from [C Skill System](https://github.com/JesterWizard/C-SkillSystem-Jester).
