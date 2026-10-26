# Fast Map Animations

Every moving map sprite (MU) walks at the engine's top speed: **one tile per frame**. This covers player units, enemy and NPC movement during their phases, and units moved by events/cutscenes.

Vanilla walks at 1–2 pixels per frame, faster with the Game Speed option or (after the first playthrough) while holding A, and event moves can set their own speed. This patch makes `GetMuQ4MovementSpeed` always return `0x200`. The MU movement code caps each step at one tile, so `0x200` behaves as "one tile per frame", the same top speed vanilla uses for its fast-walk mode.

This matches the integrated C Skill System behaviour for the `fast_map_animations` flag, but installs on **clean FE8U** without the kernel.

## Target ROM

- **FE8U (USA)** clean ROM
- Hook: vanilla `GetMuQ4MovementSpeed` at `0x0807947C`, rewritten in place (6 bytes)
- Free space: none

## Build

There is nothing to build: the patch is three Thumb instructions written directly by `Installer.event`.

## Installation

This folder is self-contained. It does not include `EAstdlib.event` or `Hack Installation.txt`. FEBuilder’s bundled Event Assembler is enough.

### FEBuilderGBA

1. Copy the `fast_map_animations` folder (or download this standalone package).
2. Open a clean FE8U ROM in FEBuilder.
3. Go to **Advanced Editors → Insert EA**.
4. Click **Select File**, choose `Installer.event`, then click **Load Script**.

### Event Assembler

From a copy of clean `fe8.gba`, with this folder as the working directory:

```bash
ColorzCore A FE8 -input:Installer.event -output:/path/to/fe8-fast-map-animations.gba
```

## Files

| File | Purpose |
|------|---------|
| `Installer.event` | EA installer: replaces the start of `GetMuQ4MovementSpeed` with `movs r0, #0x80; lsls r0, r0, #2; bx lr` |

## Hooks & Free Space

| Function | Hook (`ORG`) | Overwritten |
|----------|--------------|-------------|
| `GetMuQ4MovementSpeed` | `$7947C` | 6 bytes (whole function is bypassed) |

**Free space:** none. No RAM.

## Conflicts

- Any patch that replaces or hooks vanilla `GetMuQ4MovementSpeed` (`0x0807947C`), including the full C Skill System.
- Free-movement patches that drive MUs themselves (the integrated kernel notes that this flag does not work well with its free-movement mode).

`Installer.event` uses `PROTECT` on the 6 rewritten bytes. If another patch writes there, Event Assembler should report the conflicting write location.

## Limitations

- It is always on: the Game Speed option and holding A no longer change map walking speed, and event-scripted walking speeds are ignored.
- Only movement speed changes. Battle animations, map battle animations and other map effects run at their normal speed.

## Credits

Extracted from [C Skill System](https://github.com/JesterWizard/C-SkillSystem-Jester).
