# Deny Stat Screen Access

Lets you list classes whose units cannot be inspected on the stat screen. Pressing **R** on such a unit on the map does nothing, and when browsing the stat screen with **Up/Down** those units are skipped.

Vanilla FE8U already does this for the two Gorgon Egg classes only. This package turns that into an editable class list in `Installer.event`.

This matches the integrated C Skill System `deny_stat_screen_access` config flag, but installs on **clean FE8U** without the kernel.

## Configuration

Edit the `StatScreenDenyClasses` list in `Installer.event` before installing: one class ID per `BYTE`, ended by `BYTE 0x00`.

```
StatScreenDenyClasses:
    BYTE 0x62 // CLASS_GORGONEGG
    BYTE 0x34 // CLASS_GORGONEGG2
    // BYTE 0x51 // example: CLASS_PHANTOM
    BYTE 0x00 // end of list
```

The default list is the integrated build's list (the two Gorgon Egg classes). Vanilla already denies those, so **out of the box the patch behaves like vanilla**; add your own classes to make it do something. Class ID `0x00` cannot be listed (it ends the list).

## Target ROM

- **FE8U (USA)** clean ROM
- Hooks:
  - `CanShowUnitStatScreen` at `0x0801C928` (installer ORG `$1C928`)
  - `FindNextUnit` at `0x08087920` (installer ORG `$87920`)
- Free space: `$1000000`

## Build

No build step is required to install the patch. `Source/DenyStatScreenAccess.lyn.event` is checked in, and the class list lives in `Installer.event`, so you can edit the list and install `Installer.event` as-is.

Run `make` only if you edit `Source/DenyStatScreenAccess.c`:

```bash
make -C Standalone/deny_stat_screen_access
```

That requires [devkitARM](https://devkitpro.org/wiki/Getting_Started) and this repo's [FE-CLib](https://github.com/MokhaLeee/FE-CLib-Mokha) / [Event Assembler](https://github.com/MokhaLeee/EventAssembler/tree/mokha-fix) tools. See [Documentation/Setup.md](../../Documentation/Setup.md) for full setup.

## Installation

This folder is self-contained. It does not include `EAstdlib.event` or `Hack Installation.txt`. FEBuilder’s bundled Event Assembler is enough.

### FEBuilderGBA

1. Copy the `deny_stat_screen_access` folder (or download this standalone package).
2. Edit `StatScreenDenyClasses` in `Installer.event`.
3. Open a clean FE8U ROM in FEBuilder.
4. Go to **Advanced Editors → Insert EA**.
5. Click **Select File**, choose `Installer.event`, then click **Load Script**.

### Event Assembler

From a copy of clean `fe8.gba`, with this folder as the working directory:

```bash
ColorzCore A FE8 -input:Installer.event -output:/path/to/fe8-deny-stat-screen.gba
```

## Files

| File | Purpose |
|------|---------|
| `Installer.event` | EA installer, hooks, free-space placement, and the class list |
| `Source/DenyStatScreenAccess.c` | Replacements for `CanShowUnitStatScreen` and `FindNextUnit` |
| `Source/DenyStatScreenAccess.lyn.event` | Checked-in lyn output (rebuild with `make` after editing `.c`) |
| `makefile` | Compiles C to `.lyn.event` |

## Hooks & Free Space

| Function | Hook (`ORG`) | Overwritten |
|----------|--------------|-------------|
| `CanShowUnitStatScreen` | `$1C928` | 8 bytes |
| `FindNextUnit` | `$87920` | 8 bytes |

**Free space:** `$1000000` (about 260 bytes plus the class list; continues at `CURRENTOFFSET`).

No RAM is used.

## Conflicts

- Any patch that hooks or replaces `CanShowUnitStatScreen` (`$1C928`) or `FindNextUnit` (`$87920`), including the full C Skill System stat screen access code and fog patches that hide units from the stat screen.
- No other package in `Standalone/` hooks these addresses.
- Free space at `$1000000` overlaps with other standalone patches from this repo. Install only one body at `$1000000`, or move this patch's `ORG` after any already-installed standalone code.

`Installer.event` uses `PROTECT` on both hook sites and the free-space body. Overlapping ROM writes should fail assembly with a clear EA error.

## Differences from the integrated build

- **Up/Down browsing is fixed.** In the integrated `FindNextUnit`, the class check's `continue` only continues its inner `for` loop, so denied classes could still be reached by pressing Up/Down from another unit's stat screen. This package really skips them.
- If every other unit of the faction is skipped, `FindNextUnit` returns the current unit instead of looping forever.
- The integrated `multiple_fog_stages` check (another config flag) is not included; vanilla fog behaviour is kept.

## Limitations

- `CanShowUnitStatScreen` is only used by the map (player phase and prep-screen map). Stat screens opened from prep-screen unit lists or other menus are not blocked, but Up/Down browsing there still skips denied classes.

## Credits

Extracted from [C Skill System](https://github.com/JesterWizard/C-SkillSystem-Jester) (`Data/StatScreen/Source/AccessStatScreen.c`).
