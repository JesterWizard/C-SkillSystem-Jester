# Stat Gain Frame Speed

Sets how fast the stat gains appear one after another on the map level-up window (the one shown with battle animations off). Vanilla waits 20 frames between each `+1`; the C Skill System default waits 10, so the window fills in about twice as fast.

This matches the integrated C Skill System `stat_gain_frame_speed` designer-config value (used in `ManimLevelUp_PutStatGainLabels`), but installs on **clean FE8U** without the kernel. It is a one-byte data patch: no code, no free space.

## Configuration

Edit `STAT_GAIN_FRAME_SPEED` at the top of `Installer.event`, then reinstall the patch.

| Constant | Default | Meaning |
|----------|---------|---------|
| `STAT_GAIN_FRAME_SPEED` | `10` | Frames to wait before showing the next stat gain. Vanilla is `20`. Range `0`-`255`; `0` shows a new gain every frame. |

The next gain appears `STAT_GAIN_FRAME_SPEED + 1` frames after the previous one (21 frames in vanilla, 11 with the default).

## Target ROM

- **FE8U (USA)** clean ROM
- Patched byte: the immediate of `movs r0, #20` at `0x0807F464`, inside vanilla `ManimLevelUp_PutStatGainLabels` (`0x0807F39C`)
- Free space: none

## Build

Nothing to build. `Installer.event` writes the setting byte directly.

## Installation

This folder is self-contained. It does not include `EAstdlib.event` or `Hack Installation.txt`. FEBuilder's bundled Event Assembler is enough.

### FEBuilderGBA

1. Copy the `stat_gain_frame_speed` folder (or download this standalone package).
2. Open a clean FE8U ROM in FEBuilder.
3. Go to **Advanced Editors → Insert EA**.
4. Click **Select File**, choose `Installer.event`, then click **Load Script**.

### Event Assembler

From a copy of clean `fe8.gba`, with this folder as the working directory:

```bash
ColorzCore A FE8 -input:Installer.event -output:/path/to/fe8-stat-gain-speed.gba
```

## Files

| File | Purpose |
|------|---------|
| `Installer.event` | EA installer: setting and the patched byte |

## Hooks & Free Space

| Function | Patched (`ORG`) | Overwritten |
|----------|-----------------|-------------|
| `ManimLevelUp_PutStatGainLabels` (`movs r0, #20` → `proc->clock`) | `$7F464` | 1 byte |

**Free space:** none.

## Conflicts

- Any patch that replaces or hooks vanilla `ManimLevelUp_PutStatGainLabels` (`$7F39C` .. `$7F477`), including the full C Skill System map level-up rewrite. A replacement makes this byte unused.
- Works with [talk_on_level_up](../talk_on_level_up/), whose level-up proc calls the vanilla `ManimLevelUp_PutStatGainLabels`, and with patches that hook other parts of the map level-up window such as [promotion_on_max_level](../promotion_on_max_level/) (`ManimLevelUp_ScrollOut`).

`Installer.event` uses `PROTECT` on the patched byte (`$7F464`). If another patch writes it, Event Assembler should report the conflicting write location.

## Limitations

- Only the map level-up window is affected. The battle-animation level-up screen has its own timing.
- The vanilla pauses before the first gain (30 frames) and after the last one (60 frames) are unchanged. The integrated C Skill System level-up proc shortens those to 15 and 45 frames; that is part of its own proc rewrite, not of this setting.

## Credits

Extracted from [C Skill System](https://github.com/JesterWizard/C-SkillSystem-Jester).
