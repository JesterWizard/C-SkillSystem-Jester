# Promote Enemy On Kill

<p align="center">
  <img src="./Feature - Enemies Promote When Killing Your Units.gif" alt="Enemies Promote When Killing Your Units" width="600"/>
</p>

When an unpromoted enemy unit kills a player (or other non-enemy) unit in battle, that enemy is immediately promoted using its class's default promotion and receives a flat stat boost on every growth stat.

This is a difficulty scaler: enemies grow stronger as they score kills.

## Configuration

Edit `PROMOTE_ENEMY_ON_KILL` and `PROMOTE_ENEMY_BOOST` at the top of `Installer.event`, then reinstall the patch. No C recompile is needed.

| Constant | Default | Meaning |
|----------|---------|---------|
| `PROMOTE_ENEMY_ON_KILL` | `1` | Set to `0` to disable the feature |
| `PROMOTE_ENEMY_BOOST` | `3` | Flat bonus added to HP, Str, Mag, Skl, Spd, Lck, Def, and Res after promotion |

Integrated defaults match `Data/DesignerConfig/designer-config.c` (`true` and `3`).

## Target ROM

- **FE8U (USA)** clean ROM
- Hook: vanilla `BattleGenerateHit` at `0x0802B83D` (Thumb entry `0x0802B83D`)
- Free space: `$1000000`

## Build

No build step is required to install the patch. `Source/PromoteEnemyOnKill.lyn.event` is checked in, so you can install `Installer.event` as-is.

Run `make` only if you edit `Source/PromoteEnemyOnKill.c`:

```bash
make -C Standalone/promote_enemy_on_kill
```

That requires [devkitARM](https://devkitpro.org/wiki/Getting_Started) and this repo's [FE-CLib](https://github.com/MokhaLeee/FE-CLib-Mokha) / [Event Assembler](https://github.com/MokhaLeee/EventAssembler/tree/mokha-fix) tools. See [Documentation/Setup.md](../../Documentation/Setup.md) for full setup.

## Installation

### FEBuilderGBA

1. Download [FEBuilderGBA (Laqieer branch)](https://nightly.link/laqieer/FEBuilderGBA/workflows/msbuild/master).
2. In **Settings → Options → Path**, set **Event Assembler** to this repo's [ColorzCore](https://github.com/MokhaLeee/EventAssembler/tree/mokha-fix) executable (see [Setup.md](../../Documentation/Setup.md)).
3. Open your project ROM in FEBuilder.
4. Go to **Advanced Editors → Insert EA**.
5. Click **Select File**, choose `Standalone/promote_enemy_on_kill/Installer.event`, then click **Load Script**.

### Event Assembler

From a copy of clean `fe8.gba`:

```bash
cp /path/to/fe8.gba /path/to/fe8-promote-enemy-on-kill.gba
Tools/EventAssembler/ColorzCore A FE8 \
  -input:Standalone/promote_enemy_on_kill/Installer.event \
  -output:/path/to/fe8-promote-enemy-on-kill.gba
```

Run from the repository root so Event Assembler can resolve `EAstdlib.event`.

## Files

| File | Purpose |
|------|---------|
| `Installer.event` | EA installer, addresses, hook, and free-space placement |
| `Source/PromoteEnemyOnKill.c` | C implementation |
| `Source/PromoteEnemyOnKill.lyn.event` | Checked-in lyn output (rebuild with `make` after editing `.c`) |
| `makefile` | Compiles C to `.lyn.event` |

## Conflicts

- Any patch that replaces vanilla `BattleGenerateHit` (`0x0802B83D`), including the full C Skill System battle hit rewrite.
- Free space at `$1000000` overlaps with other standalone patches from this repo. Install only one body at `$1000000`, or move this patch's `ORG` to the next free region after any already-installed standalone code.

`Installer.event` uses `PROTECT` on both the hook site (`$2B83D`, 8 bytes) and the free-space body (`$1000000` through end of install). If another patch overlaps those ranges, Event Assembler should report the conflicting write location.

## Credits

Extracted from [C Skill System](https://github.com/JesterWizard/C-SkillSystem-Jester).
