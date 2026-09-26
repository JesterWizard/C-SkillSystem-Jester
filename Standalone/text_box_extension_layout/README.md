# Text Box Extension Layout

R-text can use more than vanilla's three lines. `TEXT_BOX_EXTENSION_LAYOUT` picks how overflow is shown.

| Value | Box | Overflow |
|-------|-----|----------|
| `0` | 3 lines | Extra lines are cut off |
| `1` | Up to 5 lines | The box grows |
| `2` | 3 lines | Extra lines are later pages. **A** advances and wraps. **B** or **R** closes. Moving the cursor returns to page 1 |

Mode `2` draws a gold `n/m` at the top-right of the help frame when there is more than one page. Integrated `designer-config.c` defaults this field to `2`.

Weapon and staff headers still take their vanilla lines, so a weapon description gets fewer lines per page. Tellius capacity text is not drawn.

## Configuration

Edit `TEXT_BOX_EXTENSION_LAYOUT` at the top of `Installer.event`, then reinstall the patch. No C recompile is needed.

## Target ROM

- **FE8U (USA)** clean ROM
- Hooks (8 bytes each):
  - `LoadHelpBoxGfx` at `$89804`
  - `DisplayHelpBoxObj` at `$89980`
  - `HelpBoxTextScroll_OnLoop` at `$89E58`
  - `HelpBoxIntroDrawTexts` at `$8A00C`
  - `ClearHelpBoxText` at `$8A118`
  - `StartHelpBoxExt` at `$88E9C`
  - `sub_808A200` at `$8A200`
  - `ApplyHelpBoxContentSize` at `$891AC`
  - `HbMoveCtrl_OnIdle` at `$89088`
- Free space: `$1000000`
- EWRAM: `0x0203AAD8` through `0x0203AAEF` (24 bytes). Eight bytes are the page counter. Sixteen bytes are the fourth and fifth text slots, because vanilla `gHelpBoxSt` only has three.

## Build

No build step is required to install the patch. `Source/TextBoxExtensionLayout.lyn.event` is checked in, so you can install `Installer.event` as-is.

Run `make` only if you edit `Source/TextBoxExtensionLayout.c` or `Source/RamAlloc.s`:

```bash
make -C Standalone/text_box_extension_layout
```

That requires [devkitARM](https://devkitpro.org/wiki/Getting_Started) and this repo's [FE-CLib](https://github.com/MokhaLeee/FE-CLib-Mokha) / [Event Assembler](https://github.com/MokhaLeee/EventAssembler/tree/mokha-fix) tools. See [Documentation/Setup.md](../../Documentation/Setup.md) for full setup.

## Installation

This folder is self-contained. It does not include `EAstdlib.event` or `Hack Installation.txt`. FEBuilder’s bundled Event Assembler is enough.

### FEBuilderGBA

1. Copy the `text_box_extension_layout` folder (or download this standalone package).
2. Open a clean FE8U ROM in FEBuilder.
3. Go to **Advanced Editors → Insert EA**.
4. Click **Select File**, choose `Installer.event`, then click **Load Script**.

### Event Assembler

From a copy of clean `fe8.gba`, with this folder as the working directory:

```bash
ColorzCore A FE8 -input:Installer.event -output:/path/to/fe8-text-box-layout.gba
```

## Files

| File | Purpose |
|------|---------|
| `Installer.event` | EA installer, addresses, hooks, and free-space placement |
| `Source/TextBoxExtensionLayout.c` | Help-box layout and paging |
| `Source/RamAlloc.s` | EWRAM address for page state and extra text slots |
| `Source/TextBoxExtensionLayout.lyn.event` | Checked-in lyn output (rebuild with `make` after editing the sources) |
| `makefile` | Compiles C and the RAM file to `.lyn.event` |

## Conflicts

- Any patch that replaces the nine help-box functions listed above, including the full C Skill System help-box rewrite.
- EWRAM `0x0203AAD8` .. `0x0203AAEF` sits just after the ghost buffer used by `Standalone/alpha_blend_movement_sprites/`. Move `Source/RamAlloc.s` if another hack already uses that range, then run `make`.
- Mode `2` decompresses the stat-screen digit sheet into OBJ VRAM at tile `0x240` when a help box opens. On the stat screen that is the same sheet. Elsewhere it can cover other OBJ graphics until the box closes.
- Free space at `$1000000` overlaps with other standalone patches from this repo. Install only one body at `$1000000`, or move this patch's `ORG` to the next free region after any already-installed standalone code.

`Installer.event` uses `PROTECT` on each hook site (8 bytes) and the free-space body (`$1000000` through end of install). If another patch overlaps those ranges, Event Assembler should report the conflicting write location.

## Credits

Extracted from [C Skill System](https://github.com/JesterWizard/C-SkillSystem-Jester).
