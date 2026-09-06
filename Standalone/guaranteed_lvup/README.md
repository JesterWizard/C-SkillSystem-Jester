# Guaranteed Level Up

When a battle level-up would grant no stat gains, the patch retries up to **10 times**, re-rolling all stats each time with a **+10% growth bonus** (on top of Metis/Antlion's +5% when applicable).

Vanilla FE8U only retries twice, rolling stats one at a time without a growth bonus. This matches the integrated C Skill System `guaranteed_lvup` behavior for vanilla growth mode, but installs on **clean FE8U** without the kernel.

## Target ROM

- **FE8U (USA)** clean ROM
- Hook: vanilla `CheckBattleUnitLevelUp` at `0x0802BA28` (Thumb entry `0x0802BA29`)
- Free space: `$1000000`

## Build

No build step is required to install the patch. `Source/CheckBattleUnitLevelUp_Guaranteed.lyn.event` is checked in, so you can install `Installer.event` as-is.

Run `make` only if you edit `Source/CheckBattleUnitLevelUp_Guaranteed.c`:

```bash
make -C Standalone/guaranteed_lvup
```

That requires [devkitARM](https://devkitpro.org/wiki/Getting_Started) and this repo's [FE-CLib](https://github.com/MokhaLeee/FE-CLib-Mokha) / [Event Assembler](https://github.com/MokhaLeee/EventAssembler/tree/mokha-fix) tools. See [Documentation/Setup.md](../../Documentation/Setup.md) for full setup.

## Installation

### FEBuilderGBA

1. Download [FEBuilderGBA (Laqieer branch)](https://nightly.link/laqieer/FEBuilderGBA/workflows/msbuild/master).
2. In **Settings → Options → Path**, set **Event Assembler** to this repo's [ColorzCore](https://github.com/MokhaLeee/EventAssembler/tree/mokha-fix) executable (see [Setup.md](../../Documentation/Setup.md)).
3. Open your project ROM in FEBuilder.
4. Go to **Advanced Editors → Insert EA**.
5. Click **Select File**, choose `Standalone/guaranteed_lvup/Installer.event`, then click **Load Script**.

FEBuilder applies the patch to the open ROM. For more detail on Insert EA, see [Installing ASM / C using Insert EA](https://feuniverse.us/t/installing-asm-c-using-insert-ea/32968).

### Event Assembler

From a copy of clean `fe8.gba`:

```bash
cp /path/to/fe8.gba /path/to/fe8-guaranteed-lvup.gba
Tools/EventAssembler/ColorzCore A FE8 \
  -input:Standalone/guaranteed_lvup/Installer.event \
  -output:/path/to/fe8-guaranteed-lvup.gba
```

Run from the repository root so Event Assembler can resolve `EAstdlib.event`.

## Files

| File | Purpose |
|------|---------|
| `Installer.event` | EA installer, addresses, hook, and free-space placement |
| `Source/CheckBattleUnitLevelUp_Guaranteed.c` | C implementation |
| `Source/CheckBattleUnitLevelUp_Guaranteed.lyn.event` | Checked-in lyn output (rebuild with `make` after editing `.c`) |
| `makefile` | Compiles C to `.lyn.event` |

## Conflicts

- Any patch that replaces vanilla `CheckBattleUnitLevelUp` (`0x0802BA28` / `0x0802BA29`), including the full C Skill System level-up rewrite.
- Installing on a ROM that already modified the first 8 bytes at `0x0802BA28`.
- Free space at `$1000000` overlaps with other standalone patches from this repo (for example `two_random_number_growths` or `custom_fog_sight`). Install only one body at `$1000000`, or move this patch's `ORG` to the next free region after any already-installed standalone code.

`Installer.event` uses `PROTECT` on both the hook site (`$2BA28`, 8 bytes) and the free-space body (`$1000000` through end of install). If another patch overlaps those ranges, Event Assembler should report the conflicting write location.

## Credits

Extracted from [C Skill System](https://github.com/JesterWizard/C-SkillSystem-Jester).
