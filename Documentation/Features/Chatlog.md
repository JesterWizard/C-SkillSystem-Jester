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

This feature adds a **Talk-scene chatlog**: a left-side BG0 overlay that stores the last **15** completed dialogue pages for the current chapter, toggled with **SELECT**, scrolled with **DPAD_UP / DPAD_DOWN**, and kept alive across **suspend** (not normal save files).

---

## Plan

### Player controls

| Input | Context | Behavior |
|-------|---------|----------|
| **SELECT** | Talk open, log closed | Open chatlog overlay |
| **SELECT** | Talk open, log open | Close overlay and restore Talk BG0 |
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
| FreeRamSpace2 | `sChatLogState` | `0x444` | 17-entry ring + view cursor (SUS-persisted) |
| FreeRamSpace2 | `sChatlogUiState` | `0x600` | Font/texts, palette + display backups, name/draw scratch |
| SuspendSave EMS | `SaveChatLogSuspendState` | `0x444` | Survive soft reset / resume mid-chapter |
| NormalSave | — | — | Not used |

UI visibility (`CHATLOG_FLAG_VISIBLE`) is cleared on suspend load.

### Overlay drawing

- The panel owns **BG0**, the layer the dialogue text itself uses; every other layer is switched off, because the map engine keeps writing BG1–BG3 behind our back. The talk glyphs at chr `0xC0–0x16F` and the whole BG0 tilemap are stashed in `gGenericBuffer` while the log is open and copied back on close.
- Each visible row: **BG minimug** drawn with `PutFaceChibi` at chr `0xC0 + row * 0x10` (own 16-color pal), gold name, white message text.
- Names/message text use the Talk glyph set on BG0 at chr `0x100+`, pal 8, `0x3E` tiles per row; chr `0x1F8` is a solid tile that provides the panel backdrop, since the hardware backdrop colour is rewritten by the scene.
- Display registers, blend, window, BG0 scroll and the whole BG palette are saved on open, restated every frame (other procs keep poking them), and restored on close.
- Nameplate, floating glyph, print-shake and wave procs in the text engine are gated on `Chatlog_IsVisible()` so they cannot draw into the layers the log borrows.

```
[chibi] Name
        Page text line 1
        Page text line 2
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

---

## TODO

- Optionally filter prep Event Replay / world-map Talk from the chapter ring.
- Consider a longer message field if English rewrites routinely truncate at 56 bytes per wrapped line.
- Suspend RAM is at 97% with the log's `0x444` chunk in place; shrink `CHATLOG_CAP` if another feature needs suspend space.

---

## Limitations & Bugs

- Chatlog is **Talk-only**; map SELECT is intentionally untouched.
- History does **not** persist in normal save files—only suspend.
- Opening the overlay owns all of BG0 (tilemap and chr `0xC0–0x1FF`) plus the whole BG palette; both are restored on close.
- Minimugs are the portrait chibi (16 colors), not the 32-color half-body sprite. Portraits without a chibi show the engine's `?` mug, exactly as they do on the map.
- Only 17 entries are kept; older lines fall out of the ring.
- Portrait→name resolution uses stored `portraitId` plus character-table fallback; some variant faces may show a blank name.
- B-skipped (`instantScroll`) pages are still logged, which is intentional for completeness.

Please report issues in the repository’s **Issues** tab.
