# Stat Page Promotions

From Chapter 10 onward, the stat screen gains a fourth page. It lists up to three promotion classes for the unit, with a map sprite on a platform beside each name. Press R on a class name to read that class's description.

Units with no entry show **No promotions**. The checked-in list has Eirika only, matching the integrated table: Great Lord (Eirika), Great Lord (Ephraim), and Paladin. Edit `sPromoPages` in `Source/StatPagePromotions.c` and run `make` to change it.

The page counter shows **4/4**. Vanilla's page-name graphics only cover the first three pages, so the title banner on this page is blank.

Skill icons and skill R-text from the integrated page are not included. Those need the skill system. Integrated `designer-config.c` defaults this flag to `true`.

## Configuration

Edit `STAT_PAGE_PROMOTIONS` at the top of `Installer.event`, then reinstall the patch. No C recompile is needed.

| Constant | Default | Meaning |
|----------|---------|---------|
| `STAT_PAGE_PROMOTIONS` | `1` | Set to `0` to keep the vanilla 3-page stat screen |

The page stays hidden before chapter id `0x0B` (Chapter 10), same as the integrated page lock.

## Target ROM

- **FE8U (USA)** clean ROM
- Hooks (8 bytes each):
  - `DisplayPage` at `0x080878CD` (installer ORG `$878CC`)
  - `DisplayPageNameSprite` at `0x08087EB9` (installer ORG `$87EB8`)
  - `PageNumCtrl_DisplayMuPlatform` at `0x08088355` (installer ORG `$88354`)
  - page-count store inside `StatScreen_Display` at `0x08088690` (installer ORG `$88690`)
  - `StartStatScreenHelp` at `0x080889A1` (installer ORG `$889A0`)
- Free space: `$1000000`

## Build

No build step is required to install the patch. `Source/StatPagePromotions.lyn.event` is checked in, so you can install `Installer.event` as-is.

Run `make` only if you edit `Source/StatPagePromotions.c`:

```bash
make -C Standalone/stat_page_promotions
```

That requires [devkitARM](https://devkitpro.org/wiki/Getting_Started) and this repo's [FE-CLib](https://github.com/MokhaLeee/FE-CLib-Mokha) / [Event Assembler](https://github.com/MokhaLeee/EventAssembler/tree/mokha-fix) tools. See [Documentation/Setup.md](../../Documentation/Setup.md) for full setup.

## Installation

This folder is self-contained. It does not include `EAstdlib.event` or `Hack Installation.txt`. FEBuilder’s bundled Event Assembler is enough.

### FEBuilderGBA

1. Copy the `stat_page_promotions` folder (or download this standalone package).
2. Open a clean FE8U ROM in FEBuilder.
3. Go to **Advanced Editors → Insert EA**.
4. Click **Select File**, choose `Installer.event`, then click **Load Script**.

### Event Assembler

From a copy of clean `fe8.gba`, with this folder as the working directory:

```bash
ColorzCore A FE8 -input:Installer.event -output:/path/to/fe8-stat-page-promotions.gba
```

## Files

| File | Purpose |
|------|---------|
| `Installer.event` | EA installer, addresses, hooks, and free-space placement |
| `Source/StatPagePromotions.c` | Page draw, sprites, R-text, and the promotion list |
| `Source/StatPagePromotions.lyn.event` | Checked-in lyn output (rebuild with `make` after editing `.c`) |
| `makefile` | Compiles C to `.lyn.event` |

## Conflicts

- Any patch that replaces `DisplayPage`, `DisplayPageNameSprite`, `PageNumCtrl_DisplayMuPlatform`, `StartStatScreenHelp`, or the page-count store at `$88690`, including the full C Skill System stat screen.
- Another patch that adds its own fourth stat screen page.
- Free space at `$1000000` overlaps with other standalone patches from this repo. Install only one body at `$1000000`, or move this patch's `ORG` to the next free region after any already-installed standalone code.

`Installer.event` uses `PROTECT` on each hook site (8 bytes) and the free-space body (`$1000000` through end of install). If another patch overlaps those ranges, Event Assembler should report the conflicting write location.

## Credits

Extracted from [C Skill System](https://github.com/JesterWizard/C-SkillSystem-Jester).
