# Custom Fog Sight

Standalone FE8U patch extracted from the C Skill System designer config flag `custom_fog_sight`.

## Behavior

Replaces vanilla fog vision with per-class bonuses on top of the chapter base range (`gPlaySt.chapterVisionRange`) and torch duration. Bonuses are defined in `ClassFogSightBonusTable.event`.

Default table values:

| Bonus | Classes |
|-------|---------|
| +1 | Master Lords, Cavaliers, Paladins, Great Knights, Troubadours, Mage Knights, Valkyries, Rangers, Tarvos, Maelduin |
| +2 | Archers, Snipers, Pegasus/Falcon Knights, Wyvern line, Gargoyles, Draco Zombie, Manaketes, Mogalls, Phantom |
| +5 | Thief, Rogue, Assassin |

All other classes use the chapter base range only (plus torch).

This matches the integrated C Skill System behavior when `custom_fog_sight` is enabled, but installs on **clean FE8U** without the kernel.

**Note:** This patch only replaces `GetUnitFogViewRange`. It does not include stat-screen vision bars or help text from the full kernel stat menu.

## Target ROM

- **FE8U (USA)** clean ROM
- Hook: vanilla `GetUnitFogViewRange` at `0x080178A8` (Thumb entry `0x080178A9`)
- Free space: `$1000000`

## Installation

No build step required — this patch is pure Event Assembler.

### FEBuilderGBA

1. Download [FEBuilderGBA (Laqieer branch)](https://nightly.link/laqieer/FEBuilderGBA/workflows/msbuild/master).
2. In **Settings → Options → Path**, set **Event Assembler** to this repo's [ColorzCore](https://github.com/MokhaLeee/EventAssembler/tree/mokha-fix) executable (see [Setup.md](../../Documentation/Setup.md)).
3. Open your project ROM in FEBuilder.
4. Go to **Advanced Editors → Insert EA**.
5. Click **Select File**, choose `Standalone/custom_fog_sight/Installer.event`, then click **Load Script**.

FEBuilder applies the patch to the open ROM. For more detail on Insert EA, see [Installing ASM / C using Insert EA](https://feuniverse.us/t/installing-asm-c-using-insert-ea/32968).

### Event Assembler

From a copy of clean `fe8.gba`:

```bash
cp /path/to/fe8.gba /path/to/fe8-fog.gba
Tools/EventAssembler/ColorzCore A FE8 \
  -input:Standalone/custom_fog_sight/Installer.event \
  -output:/path/to/fe8-fog.gba
```

Run from the repository root so Event Assembler can resolve `EAstdlib.event`.

## Files

| File | Purpose |
|------|---------|
| `Installer.event` | EA installer, hook, Thumb implementation, and free-space placement |
| `ClassFogSightBonusTable.event` | Editable per-class fog sight bonus table |

## Conflicts

- Any patch that replaces vanilla `GetUnitFogViewRange` (`0x080178A8` / `0x080178A9`), including the full C Skill System fog vision hook.
- Installing on a ROM that already modified the first 8 bytes at `0x080178A8`.
- Free space at `$1000000` overlaps with other standalone patches from this repo (for example `two_random_number_growths`). Install only one body at `$1000000`, or move this patch's `ORG` to the next free region after any already-installed standalone code.

`Installer.event` uses `PROTECT` on both the hook site (`$178A8`, 8 bytes) and the free-space body (`$1000000` through end of install). If another patch overlaps those ranges, Event Assembler should report the conflicting write location.

## Credits

Extracted from [C Skill System](https://github.com/JesterWizard/C-SkillSystem-Jester).
