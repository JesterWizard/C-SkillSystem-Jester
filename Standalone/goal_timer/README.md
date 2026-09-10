# Goal Timer

Adds a real-time countdown goal. Chapters with a non-zero table entry must be cleared before the clock hits zero, or the game ends. The goal window shows `Remaining: HH:MM:SS`. At half of the chapter's configured time, the digits turn gold and map-sprite animations speed up.

Chapters set to **0 seconds** keep their vanilla objective.

## Target ROM

- **FE8U (USA)** clean ROM
- Free space: `$1000000`

## Configuration

Edit `ChapterTimers.event` without recompiling C. Each entry is `chapter ID, seconds`. End the table with chapter ID `$FFFFFFFF`.

Defaults match the integrated C Skill System table (prologue 30s, chapter 1 off, other listed chapters 60s). Add Ephraim or extra chapters the same way:

```
ChapterTimerEntry($17, 90) // Ephraim Ch9, 90 seconds
```

## Remaining time / save

Remaining seconds are stored in vanilla **PlaySt+0x48** (`unk48`, 2 bytes). That field is already included in suspend and chapter saves, so the countdown survives suspend/resume without changing the save format or allocating extra EWRAM.

Fresh chapter start (the `UndeployEveryone` path on `gProc_BMapMain`) reseeds from the table. Suspend resume does not.

## Build

No build step is required to install the patch. `Source/GoalTimer.lyn.event` is checked in, so you can install `Installer.event` as-is.

Run `make` only if you edit `Source/GoalTimer.c`:

```bash
make -C Standalone/goal_timer
```

That requires [devkitARM](https://devkitpro.org/wiki/Getting_Started) and this repo's [FE-CLib](https://github.com/MokhaLeee/FE-CLib-Mokha) / [Event Assembler](https://github.com/MokhaLeee/EventAssembler/tree/mokha-fix) tools. See [Documentation/Setup.md](../../Documentation/Setup.md) for full setup.

Table-only edits in `ChapterTimers.event` do not need `make`.

## Installation

This folder is self-contained. It does not include `EAstdlib.event` or `Hack Installation.txt`. FEBuilder’s bundled Event Assembler is enough.

### FEBuilderGBA

1. Copy the `goal_timer` folder (or download this standalone package).
2. Open a clean FE8U ROM in FEBuilder.
3. Go to **Advanced Editors → Insert EA**.
4. Click **Select File**, choose `Installer.event`, then click **Load Script**.

### Event Assembler

From a copy of clean `fe8.gba`, with this folder as the working directory:

```bash
ColorzCore A FE8 -input:Installer.event -output:/path/to/fe8-goal-timer.gba
```

## Files

| File | Purpose |
|------|---------|
| `Installer.event` | EA installer, addresses, hooks, and free-space placement |
| `ChapterTimers.event` | Per-chapter countdown table (edit without compiling C) |
| `Source/GoalTimer.c` | C implementation |
| `Source/GoalTimer.lyn.event` | Checked-in lyn output (rebuild with `make` after editing `.c`) |
| `makefile` | Compiles C to `.lyn.event` |

## Hooks & Free Space

| Function | Hook (`ORG`) | Overwritten |
|----------|--------------|-------------|
| `GoalDisplay_Init` | `$8D288` | 8 bytes |
| `GoalDisplay_Loop_Display` | `$8D784` | 8 bytes |
| `Mu_OnLoop` | `$79030` | 8 bytes |
| `SyncUnitSpriteSheet` | `$26F2C` | 8 bytes |
| `gProc_BMapMain` `UndeployEveryone` | `$59A254` | 4 bytes (pointer) |

**Free space:** `$1000000` (body continues at `CURRENTOFFSET` after install).

**PlaySt:** `+0x48` .. `+0x49` (2 bytes) for remaining seconds.

## Conflicts

- Any patch that replaces vanilla `GoalDisplay_Init` (`$8D288`), `GoalDisplay_Loop_Display` (`$8D784`), `Mu_OnLoop` (`$79030`), or `SyncUnitSpriteSheet` (`$26F2C`), including the full C Skill System Goals rewrite.
- Any patch that replaces the `UndeployEveryone` pointer in `gProc_BMapMain` at `$59A254`.
- Any patch that uses **PlaySt+0x48**.
- Free space at `$1000000` overlaps with other standalone patches from this repo. Install only one body at `$1000000`, or move this patch's `ORG` after any already-installed standalone code.
- Do not install this on a ROM that already has the C Skill System kernel with `goal_timer` enabled.

`Installer.event` uses `PROTECT` on the hook sites and free-space body. Overlapping ROM writes should fail assembly with a clear EA error.

## Limitations

- The clock does not pause during events (same as the integrated feature).
- Battle animations freeze the countdown; other map animations do not.
- Some HUD graphics can glitch briefly after suspend resume; opening the map again clears it.
- The `Remaining:` label is hardcoded English, not a text-ID entry.

## Credits

Extracted from [C Skill System](https://github.com/JesterWizard/C-SkillSystem-Jester). See [Timer.md](../../Documentation/Features/ChapterGoals/Timer.md) for integrated documentation.
