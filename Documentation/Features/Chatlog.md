# Scrollback for Sacred Stones

---

## Index
- [Introduction](#introduction)
- [Plan](#plan)
- [Code Locations](#code-locations)
- [TODO](#todo)
- [Limitations & Bugs](#limitations--bugs)

---

## Introduction

Chapter dialogue in FE8 is ephemeral: once a page advances, earlier lines are gone. Players who miss a name, joke, or plot beat have no in-scene way to re-read without restarting the conversation (or using prep Event Replay after the fact).

This feature adds a **Talk-scene chatlog**: an overlay that stores the last **17** dialogue lines of the current chapter, toggled with **SELECT**, scrolled with **DPAD_UP / DPAD_DOWN**, and kept alive across **suspend** (not normal save files).

### Runtime configuration

`KernelDesigerConfig::chatlog_enabled` (`true` by default) turns the whole feature on and off in [`designer-config.c`](../../Data/DesignerConfig/designer-config.c). With it disabled, no session proc starts, no glyphs are captured, and SELECT behaves exactly as it does in vanilla; the suspend chunk is still written so save layout does not change between builds.

---

## Plan

### Player controls

| Input | Context | Behavior |
|-------|---------|----------|
| **SELECT** | Talk open, log closed | Open chatlog overlay |
| **SELECT** | Talk open, log open | Close overlay and restore layer priorities |
| **DPAD_UP / DOWN** | Log open | Scroll through stored entries |
| **A / B / DPAD** | Log open | Ignored for Talk advance / skip |
| Map **SELECT** | Player phase | Unchanged (not wired to chatlog) |

### Capture model

| Event | Action |
|-------|--------|
| Glyph printed in `Talk_OnIdle` | Append UTF-8 character to the current entry, wrapping to a continuation entry once the panel width is full |
| `[N]` | Treated as a word separator: the log re-wraps the message to its own width instead of splitting it |
| `[2NL]` | Commit the entry: speaker id, name text id, portrait, text → ring buffer |
| `[A]` | Commit the current entry, then wait for player input |
| Speaker change mid-page | Commit the current entry first |
| Talk end | Commit any leftover line, tear down UI proc |
| Chapter init | Clear the ring buffer |

### Storage

| Region | Symbol | Size | Purpose |
|--------|--------|------|---------|
| FreeRamSpace2 | `sChatLogState` | `0x444` | 17 line entry ring + view cursor (SUS-persisted) |
| FreeRamSpace2 | `sChatlogUiState` | `0x600` | Font/texts, chibi palette + priority/blend backups, tile allocation, name/draw scratch |
| SuspendSave EMS | `SaveChatLogSuspendState` | `0x444` | Survive soft reset / resume mid-chapter |
| NormalSave | — | — | Not used |

UI visibility (`CHATLOG_FLAG_VISIBLE`) is cleared on suspend load.

### Overlay drawing

- The log draws on **BG2**, the one layer a Talk scene leaves empty, so the box (BG1), the dialogue text (BG0) and the map or scene art all keep their own layers and stay visible. Priorities become log → text → box → scene while it is open and are restored on close; BG2 is force-enabled in `DISPCNT` for scenes that had it off.
- Nothing is filled in: cells the log does not draw stay on blank tile 0 and the frozen scene shows through. It is darkened with the hardware brightness effect (`SetBlendDarken(CHATLOG_DIM)`, every layer except BG2 targeted) purely for legibility — set `CHATLOG_DIM` to `0` for full brightness.
- Tiles and palettes are claimed at open time, not fixed. BG0/BG1/BG2 share a character base and how much is spare depends on the scene: a map talk leaves roughly 400 unreferenced tiles, a world-map narration about 45. `Chatlog_ScanUsedTiles` walks the four live tilemaps plus the range the talk font has reserved for glyphs it has not printed yet, `Chatlog_AllocTiles` claims the longest free run, and the log shows as many rows as fit (`0x3E` tiles per row: a 16-tile chibi plus the name and message glyphs).
- Because of that, no scene graphics are ever overwritten and nothing has to be stashed. The only saved state is the layer priorities, the blend registers and the handful of palette slots the chibis borrow, which are likewise picked from slots no live tilemap references.
- Text uses the dialogue's own font palette (`gActiveFont->palid`), so gold names and white messages match the talk exactly.
- Nameplate, floating glyph and wave procs in the text engine are gated on `Chatlog_IsVisible()` so the frozen dialogue is not redrawn under the log.

```
[chibi] Name
        Message line
        (continuation line, no name or chibi)
```

---

## Code Locations

| Feature | Location | Description |
|--------|----------|-------------|
| Ring buffer + overlay UI | `Chatlog_*` in [`Chatlog.c`](../../Kernel/Wizardry/Misc/Chatlog/Source/Chatlog.c) | Capture API, SELECT toggle, blend/draw, DPAD scroll, SUS save/load |
| Talk input gates | `TalkWaitForInput_OnIdle`, `TalkSkipListener_OnIdle` in [`Chatlog.c`](../../Kernel/Wizardry/Misc/Chatlog/Source/Chatlog.c) | Block A/B/DPAD Talk advance while the log is open |
| Glyph capture | `Talk_OnIdle` in [`MiscFunctions.c`](../../Kernel/Wizardry/Misc/MiscFunctions/Source/MiscFunctions.c) | Pause printing while visible; append each printed character |
| Page commit hooks | `TalkInterpret` / `Talk_OnInit_C` / `Talk_OnEnd_C` in [`TextEngineRework.c`](../../Kernel/Wizardry/Misc/TextEngineRework/Source/TextEngineRework.c) | `[N]` / `[A]` capture, start/end chatlog session |
| EWRAM reservation | [`config-memmap.s`](../../include/link/config-memmap.s) | `_kernel_malloc2` for `sChatLogState` and `sChatlogUiState` |
| Suspend chunk | `gEmsSusChunks` in [`data.event`](../../Kernel/Wizardry/Common/SaveData/data.event) | SUS-only `0x444` EMS chunk |
| Chapter clear | `ChapterInit_ResetChatlog` via [`ChapterInitHook/data.event`](../../Kernel/Wizardry/Common/ChapterInitHook/data.event) | Wipe history on chapter start |
| Public API | [`chatlog.h`](../../include/kernel/chatlog.h) | Caps, structs, and hook entry points |
| Runtime toggle | `chatlog_enabled` in [`kernel-lib.h`](../../include/kernel/kernel-lib.h) and [`designer-config.c`](../../Data/DesignerConfig/designer-config.c) | Gates `Chatlog_StartSession` and glyph capture |

---

## TODO

- Optionally filter prep Event Replay / world-map Talk from the chapter ring.
- Consider a longer message field if English rewrites routinely truncate at 56 bytes per wrapped line.
- Suspend RAM is at 97% with the log's `0x444` chunk in place; shrink `CHATLOG_CAP` if another feature needs suspend space.

---

## Limitations & Bugs

- Chatlog is **Talk-only**; map SELECT is intentionally untouched.
- History does **not** persist in normal save files—only suspend.
- Opening the overlay owns BG2's tilemap; scenes that already draw on BG2, or that leave fewer than `0x3E` spare tiles (world-map narration, for instance), simply do not open the log rather than corrupt their graphics.
- A scene with room for only some rows shows a shorter log; the scroll range follows the row count actually drawn.
- Minimugs are the portrait chibi (16 colors), not the 32-color half-body sprite. Portraits without a chibi show the engine's `?` mug, exactly as they do on the map.
- Only 17 entries are kept; older lines fall out of the ring.
- Portrait→name resolution uses stored `portraitId` plus character-table fallback; some variant faces may show a blank name.
- B-skipped (`instantScroll`) pages are still logged, which is intentional for completeness.

Please report issues in the repository’s **Issues** tab.
