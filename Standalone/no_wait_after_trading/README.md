# No Wait After Trading

After a unit trades (or uses Supply, Take, Give, or enters/exits a ballista), cancelling its action menu with **B** sends it back to movement selection instead of forcing it to **Wait**.

In vanilla, once a unit has traded it may still act from the same tile, but pressing B on the action menu ends its turn. With this patch the unit can trade, cancel, move somewhere else, and act there (trade again, attack, etc.).

How it works: `PlayerPhase_PrepareAction` keeps a "taken action" mask in `gBmSt.taken_action` that forces Wait on cancel and hides Trade/Take/Give/ballista commands that were already used. This patch clears that mask every time the player phase prepares an action, which is exactly what the integrated `no_wait_after_trading` flag does.

This matches the integrated C Skill System behaviour for the `no_wait_after_trading` flag, but installs on **clean FE8U** without the kernel.

## Target ROM

- **FE8U (USA)** clean ROM
- Hook: the `PROC_CALL_2(PlayerPhase_PrepareAction)` entry of `gProcScr_PlayerPhase`: the function pointer at `0x0859ABD4`. Vanilla `PlayerPhase_PrepareAction` (`0x0801D084`) itself is not modified.
- Free space: `$1000000`

## Build

No build step is required to install the patch. `Source/PlayerPhase_PrepareAction_NoWait.lyn.event` is checked in, so you can install `Installer.event` as-is.

Run `make` only if you edit `Source/PlayerPhase_PrepareAction_NoWait.c`:

```bash
make -C Standalone/no_wait_after_trading
```

That requires [devkitARM](https://devkitpro.org/wiki/Getting_Started) and this repo's [FE-CLib](https://github.com/MokhaLeee/FE-CLib-Mokha) / [Event Assembler](https://github.com/MokhaLeee/EventAssembler/tree/mokha-fix) tools. See [Documentation/Setup.md](../../Documentation/Setup.md) for full setup.

## Installation

This folder is self-contained. It does not include `EAstdlib.event` or `Hack Installation.txt`. FEBuilder’s bundled Event Assembler is enough.

### FEBuilderGBA

1. Copy the `no_wait_after_trading` folder (or download this standalone package).
2. Open a clean FE8U ROM in FEBuilder.
3. Go to **Advanced Editors → Insert EA**.
4. Click **Select File**, choose `Installer.event`, then click **Load Script**.

### Event Assembler

From a copy of clean `fe8.gba`, with this folder as the working directory:

```bash
ColorzCore A FE8 -input:Installer.event -output:/path/to/fe8-no-wait-after-trading.gba
```

## Files

| File | Purpose |
|------|---------|
| `Installer.event` | EA installer, proc-script pointer, and free-space placement |
| `Source/PlayerPhase_PrepareAction_NoWait.c` | Wrapper: clears `gBmSt.taken_action`, then calls vanilla `PlayerPhase_PrepareAction` |
| `Source/PlayerPhase_PrepareAction_NoWait.lyn.event` | Checked-in lyn output (rebuild with `make` after editing `.c`) |
| `makefile` | Compiles C to `.lyn.event` |

## Hooks & Free Space

| Site | `ORG` | Overwritten |
|------|-------|-------------|
| `gProcScr_PlayerPhase` + `0xFC` (argument of `PROC_CALL_2(PlayerPhase_PrepareAction)`, its only caller) | `$59ABD4` | 4 bytes (`POIN`) |

**Free space:** `$1000000`, 36 bytes. No RAM.

Because the vanilla function is untouched, a patch that hooks the body of `PlayerPhase_PrepareAction` with `jumpToHack` still works together with this one (the wrapper calls whatever is at `0x0801D084`).

## Conflicts

- Any patch that repoints the same `gProcScr_PlayerPhase` entry (`$59ABD4`) or replaces the whole player-phase proc script.
- A full C Skill System build (it replaces `PlayerPhase_PrepareAction` with its own copy that has the same flag).
- Free space at `$1000000` overlaps with other standalone patches from this repo. Install only one body at `$1000000`, or move this patch's `ORG` after any already-installed standalone code.

`Installer.event` uses `PROTECT` on both the pointer (`$59ABD4`, 4 bytes) and the free-space body. If another patch overlaps those ranges, Event Assembler should report the conflicting write location.

## Limitations

- As in the integrated version, the mask is cleared for every action, not just trading: Supply, Take, Give and ballista actions can also be followed by a cancel and a new move, and the Trade/Take/Give commands are offered again after that move.
- After a cancel, the unit goes back to the tile it started its turn on (vanilla `PlayerPhase_BackToMove`) and moves with its full movement again.

## Credits

Extracted from [C Skill System](https://github.com/JesterWizard/C-SkillSystem-Jester).
