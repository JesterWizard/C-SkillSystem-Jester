# Refuge

Adds a **Refuge** command to the unit action menu. An adjacent same-faction ally whose AID is at least this unit's CON can be selected; the actor is then rescued by that ally (reverse Rescue). If the host has not already acted this turn, they remain selectable.

Phantoms, berserk units, and units that are already rescuing or rescued cannot be hosts. The actor cannot use Refuge after moving gray, while mounted on a ballista, or while already rescuing.

## Target ROM

- **FE8U (USA)** clean ROM
- Hooks:
  - `gUnitActionMenuDef.menuItems` pointer at `$59D1F8` (4 bytes)
  - vanilla `PlayerPhase_FinishAction` at `0x0801D344` (8-byte `jumpToHack`; Thumb entry `0x0801D345`)
- Free space: `$1000000`
- The command name is a raw ASCII string (` Refuge`); `nameMsgId` is 0 so the menu draws it without `GetStringFromIndex`.
- R-text uses text ID `$405` (unused vanilla dummy item-name slot) with **vanilla Huffman** bytes: *Retreat into an ally who's / AID is at least this CON.* Do not set the anti-Huffman high bit (`pointer | 0x80000000`); clean FE8U `DecodeString` has no high-bit check and will crash if R-help is uncompressed ASCII. Change `TEXT_REFUGE_DESC` in `Installer.event` and the matching `SHORT` / `setText` in `Menu.event` / `Text.event` if it collides.

## Build

No build step is required to install the patch. `Source/Refuge.lyn.event` is checked in, so you can install `Installer.event` as-is.

Run `make` only if you edit `Source/Refuge.c`:

```bash
make -C Standalone/refuge
```

That requires [devkitARM](https://devkitpro.org/wiki/Getting_Started) and this repo's [FE-CLib](https://github.com/MokhaLeee/FE-CLib-Mokha) / [Event Assembler](https://github.com/MokhaLeee/EventAssembler/tree/mokha-fix) tools. See [Documentation/Setup.md](../../Documentation/Setup.md) for full setup.

## Installation

This folder is self-contained. It does not include `EAstdlib.event` or `Hack Installation.txt`. FEBuilder’s bundled Event Assembler is enough.

### FEBuilderGBA

1. Copy the `refuge` folder (or download this standalone package).
2. Open a clean FE8U ROM in FEBuilder.
3. Go to **Advanced Editors → Insert EA**.
4. Click **Select File**, choose `Installer.event`, then click **Load Script**.

### Event Assembler

From a copy of clean `fe8.gba`, with this folder as the working directory:

```bash
ColorzCore A FE8 -input:Installer.event -output:/path/to/fe8-refuge.gba
```

## Files

| File | Purpose |
|------|---------|
| `Installer.event` | EA installer, addresses, hooks, text IDs, and free-space placement |
| `Menu.event` | Vanilla unit menu commands plus Refuge |
| `Text.event` | Menu name and R-button help strings |
| `Source/Refuge.c` | Usability, target select, action, and finish-action hook |
| `Source/Refuge.lyn.event` | Checked-in lyn output (rebuild with `make` after editing `.c`) |
| `makefile` | Compiles C to `.lyn.event` |

## Conflicts

- Any patch that repoints the unit action menu item table (`$59D1F8`), including the full C Skill System unit menu and Skill System FE8 `UnitMenu`.
- Any patch that replaces vanilla `PlayerPhase_FinishAction` (`0x0801D344` / `0x0801D345`).
- Text ID `$405` (unused vanilla dummy item name). Do not use IDs past the vanilla message table (`$E4B`); that overwrites Huffman data and will crash when the action menu draws.
- Any anti-Huffman / “uncompressed text pointer” patch is unnecessary for this installer. The R-text is already Huffman-compressed for vanilla `GetStringFromIndex`.

`Installer.event` uses `PROTECT` on the menu pointer, the 8-byte finish-action hook, the `$405` text-table slot, and the free-space body (`$1000000` through end of Refuge + menu). If another patch overlaps those ranges, Event Assembler should report the conflicting write location.

## Credits

Extracted from [C Skill System](https://github.com/JesterWizard/C-SkillSystem-Jester).
