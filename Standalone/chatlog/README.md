# Conversation Log

The C Skill System's conversation log (`chatlog_enabled`) for vanilla FE8U dialogue: press **SELECT** during a conversation to open a scrollback log of the chapter's dialogue.

- The log lists the chapter's dialogue so far, newest at the bottom: each speaker's mini portrait and name, then their lines, rewrapped to the log's width. The line being printed shows as far as it has got.
- **UP** / **DOWN** scroll (a scroll bar shows when there's more than fits), **SELECT** closes it. The dialogue waits while the log is open (A and B do nothing) and carries on where it was when you close it.
- The log keeps the last 17 lines of the chapter. It starts again at each chapter start (including restarting a chapter).
- The scene behind the log is dimmed. The log draws on BG2 with tiles and palette slots the scene isn't using, so it shows fewer lines (or doesn't open) when a scene leaves little VRAM free. It doesn't open over world map narration, which prints its text as sprites.

If you use [`Standalone/text_engine`](../text_engine/), don't install this package: turn on that package's `TEXT_ENGINE_CHATLOG` setting instead, which installs the same log for the text engine.

## Target ROM

- **FE8U (USA)** clean ROM
- Free space: `$1000000`. The code takes about 6.5 KB (`$1000000` .. `$1001964`).
- EWRAM: `0x0203B000` .. `0x0203B5FF` (see [RAM](#ram)).

## Build

No build step is required to install the patch. `Source/Chatlog.lyn.event` is checked in.

Run `make` if you edit `Source/` or `Source/ChatlogRamAlloc.s`:

```bash
make -C Standalone/chatlog
```

That requires [devkitARM](https://devkitpro.org/wiki/Getting_Started) and this repo's [FE-CLib](https://github.com/MokhaLeee/FE-CLib-Mokha) / [Event Assembler](https://github.com/MokhaLeee/EventAssembler/tree/mokha-fix) tools. See [Documentation/Setup.md](../../Documentation/Setup.md) for full setup.

## Installation

This folder is self-contained. It does not include `EAstdlib.event` or `Hack Installation.txt`; it uses `jumpToHack`, which FEBuilder's Event Assembler already defines.

### FEBuilderGBA

1. Copy the `chatlog` folder (or download this standalone package).
2. Open a clean FE8U ROM in FEBuilder.
3. Go to **Advanced Editors → Insert EA**.
4. Click **Select File**, choose `Installer.event`, then click **Load Script**.

### Event Assembler

From a copy of clean `fe8.gba`, with this folder as the working directory:

```bash
EA_DIR=<repo>/Tools/EventAssembler
{
  echo '#include "Extensions/Hack Installation.txt"'
  echo '#include "Installer.event"'
} > _install.event
"$EA_DIR/ColorzCore" A FE8 -input:_install.event -I:"$EA_DIR" -output:/path/to/fe8-chatlog.gba
```

## RAM

This patch uses **0x600 bytes** of EWRAM, set in `Source/ChatlogRamAlloc.s`:

| Symbol | Address range | Size | Use |
|--------|---------------|------|-----|
| `sChatLogState` | `0x0203B000` .. `0x0203B443` | `0x444` | The logged lines (17 × 64 bytes) and scroll position |
| `sChatLogStamp` | `0x0203B444` .. `0x0203B44B` | 8 | Which chapter the log belongs to: a marker and `gPlaySt`'s chapter start time |
| `sChatlogUiState` | `0x0203B44C` .. `0x0203B5FF` | `0x1AC` used | Drawing state while the log is open |

The block sits in FreeRamSpace2 (`0x0203AAA4` .. `0x0203DDE0`). To move it, change `ChatlogRamBase` in `Source/ChatlogRamAlloc.s` and run `make`.

Nothing is added to the save file. The log clears itself when `gPlaySt`'s chapter start time changes (it is set when a chapter starts or restarts), and it isn't kept in a suspend save.

## Files

| File | Purpose |
|------|---------|
| `Installer.event` | EA installer: hooks and free-space placement |
| `Source/Chatlog.c` | The log: capture, drawing, scrolling, and the two talk procs it holds back. The same file as `Standalone/text_engine/Source/Chatlog.c` |
| `Source/Chatlog.h` | Log structures and the calls between the two C files |
| `Source/ChatlogTalk.c` | Vanilla `Talk_OnIdle` that feeds the log each letter and line or page break, and the speaker lookup |
| `Source/ChatlogRamAlloc.s` | EWRAM addresses |
| `Source/Chatlog.lyn.event` | Checked-in lyn output |
| `makefile` | Compiles the C and RAM objects to `Source/Chatlog.lyn.event` |

## Hooks & Free Space

| Function | Hook (`ORG`) | Overwritten | Change |
|----------|--------------|-------------|--------|
| `TalkSkipListener_OnIdle` | `$6B4C` | 8 bytes | Vanilla, but B / START don't skip or speed up the dialogue while the log is open |
| `Talk_OnIdle` | `$6C34` | 8 bytes | Vanilla, plus: starts the log's session, passes it each letter and each `[NL]` / `[2NL]` / `[A]`, and holds the text while the log is open |
| `TalkWaitForInput_OnIdle` | `$7C4C` | 8 bytes | Vanilla, but the `[A]` arrow hides and doesn't move on while the log is open |

**Free space:** `$1000000`.

## Conflicts

- [`Standalone/text_engine`](../text_engine/) (it hooks `Talk_OnIdle` too) and any other dialogue text engine, including Tequila's assembly text engine. With the standalone text engine, use its `TEXT_ENGINE_CHATLOG` setting instead.
- Any patch that hooks the three functions above, or uses EWRAM `0x0203B000` .. `0x0203B5FF`.
- Free space at `$1000000` overlaps with other standalone patches from this repo. Install only one body at `$1000000`, or move this patch's `ORG` after any already-installed standalone code.
- Patches that put their own graphics on BG2 during dialogue: the log then doesn't open (it leaves a BG2 that's already in use alone).

`Installer.event` uses `PROTECT` on every hook site and the free-space body, so overlapping ROM writes fail assembly with a clear EA error.

## Differences from the C Skill System

- Text is one byte per character (the C Skill System decodes UTF-8).
- Letters are passed to the log by the `Talk_OnIdle` replacement, and line and page breaks are read from the text before vanilla `TalkInterpret` acts on them, instead of hooks in the C Skill System's text engine.
- The log isn't kept in the suspend save, and it is cleared when `gPlaySt`'s chapter start time changes instead of by a chapter-start hook.
- The log's proc runs under the talk proc, so it closes with the conversation even when an event skip freezes it.
- Map sprites aren't hidden while the log is open (they are dimmed with the map behind it).
- It doesn't open over world map narration, where its text came out garbled.
- Each redraw copies its tilemap to VRAM straight away, which cuts down the half-drawn frames the integrated log shows while scrolling.

## Limitations

- Redrawing the log takes most of a frame, so while you scroll quickly the frame being redrawn can show a few letters out of place (as in the C Skill System).
- START still skips the whole scene while the log is open (the event skip isn't blocked, as in the C Skill System); the log closes cleanly.
- The speaker of each line is guessed from the active portrait, so generic portraits show as `?` and may take the name of a character who shares the portrait.
- Only text printed by the dialogue system (`Talk_OnIdle`) is logged; menus, help boxes and battle quotes shown another way are not.

## Credits

Conversation log from [C Skill System](https://github.com/JesterWizard/C-SkillSystem-Jester).
