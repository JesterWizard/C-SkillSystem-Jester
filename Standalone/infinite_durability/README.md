# Infinite Durability

Weapons, including magic tomes, do not lose uses in combat or from the item menu. The durability number is hidden for weapons in:

- Unit and item menus
- The stat screen
- Prep inventory and trade
- The supply list
- Shops

Staves and other non-weapon items still show uses and still spend them. A weapon that is already damaged keeps the uses it had when the patch was installed; it just stops losing more. Broken weapons are not repaired.

The integrated `infinite_durability` flag only hides those numbers. This patch also stops weapon use loss.

## Configuration

Edit `INFINITE_DURABILITY` at the top of `Installer.event`, then reinstall the patch. No C recompile is needed.

| Constant | Default | Meaning |
|----------|---------|---------|
| `INFINITE_DURABILITY` | `1` | Set to `0` to keep vanilla durability |

## Target ROM

- **FE8U (USA)** clean ROM
- Hooks (8 bytes each, Thumb entry is the address + 1):
  - `DrawItemMenuLine` at `0x08016848`
  - `DrawItemMenuLineLong` at `0x080168E0`
  - `DrawItemMenuLineNoColor` at `0x080169A8`
  - `DrawItemStatScreenLine` at `0x08016A2C`
  - `GetItemAfterUse` at `0x08016AEC`
  - `PrepItemScreen_DrawUnitItems` at `0x08099F7C`
  - `DrawPrepScreenItems` at `0x0809B74C`
  - `PrepItemSupply_DrawItemList` at `0x0809D300`
  - `PrepItemSupply_DrawItemListRow` at `0x0809D47C`
- Free space: `$1000000`

## Build

No build step is required to install the patch. `Source/InfiniteDurability.lyn.event` is checked in, so you can install `Installer.event` as-is.

Run `make` only if you edit `Source/InfiniteDurability.c`:

```bash
make -C Standalone/infinite_durability
```

That requires [devkitARM](https://devkitpro.org/wiki/Getting_Started) and this repo's [FE-CLib](https://github.com/MokhaLeee/FE-CLib-Mokha) / [Event Assembler](https://github.com/MokhaLeee/EventAssembler/tree/mokha-fix) tools. See [Documentation/Setup.md](../../Documentation/Setup.md) for full setup.

## Installation

This folder is self-contained. It does not include `EAstdlib.event` or `Hack Installation.txt`. FEBuilder’s bundled Event Assembler is enough.

### FEBuilderGBA

1. Copy the `infinite_durability` folder (or download this standalone package).
2. Open a clean FE8U ROM in FEBuilder.
3. Go to **Advanced Editors → Insert EA**.
4. Click **Select File**, choose `Installer.event`, then click **Load Script**.

### Event Assembler

From a copy of clean `fe8.gba`, with this folder as the working directory:

```bash
ColorzCore A FE8 -input:Installer.event -output:/path/to/fe8-infinite-durability.gba
```

## Files

| File | Purpose |
|------|---------|
| `Installer.event` | EA installer, addresses, hooks, and free-space placement |
| `Source/InfiniteDurability.c` | C implementation |
| `Source/InfiniteDurability.lyn.event` | Checked-in lyn output (rebuild with `make` after editing `.c`) |
| `makefile` | Compiles C to `.lyn.event` |

## Conflicts

- Any patch that replaces the hooked routines above, including the full C Skill System item-info and prep-screen rewrites.
- Free space at `$1000000` overlaps with other standalone patches from this repo. Install only one body at `$1000000`, or move this patch's `ORG` to the next free region after any already-installed standalone code.

`Installer.event` uses `PROTECT` on each hook site (8 bytes) and on the free-space body (`$1000000` through end of install). If another patch overlaps those ranges, Event Assembler should report the conflicting write location.

## Credits

Extracted from [C Skill System](https://github.com/JesterWizard/C-SkillSystem-Jester).
