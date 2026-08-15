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
| Glyph printed in `Talk_OnIdle` | Append UTF-8 character to the current line buffer |
| `[N]` / `[2NL]` | Commit that line immediately: face ID, name, text → ring buffer |
| `[A]` | Commit the current line, then wait for player input |
| Talk end | Commit any leftover line, tear down UI proc |
| Chapter init | Clear the ring buffer |

### Storage

| Region | Symbol | Size | Purpose |
|--------|--------|------|---------|
| FreeRamSpace2 | `sChatLogState` | `0x478` | 15-entry ring + view cursor (SUS-persisted) |
| FreeRamSpace2 | `sChatlogUiState` | `0x400` | Font/texts, page accumulate buffer, BG0 panel backup |
| SuspendSave EMS | `SaveChatLogSuspendState` | `0x478` | Survive soft reset / resume mid-chapter |
| NormalSave | — | — | Not used |

UI visibility (`CHATLOG_FLAG_VISIBLE`) is cleared on suspend load.

### Overlay drawing

- Full-screen dim on **BG0** (alpha-blended over BG1/BG2/BG3/OBJ).
- Each visible row on **BG0**: **chibi**, gold name, white line text (Talk glyphs at BG chr `0x80`).
- Opening snapshots BG0; closing restores it and reselects the Talk text font.

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
| Suspend chunk | `gEmsSusChunks` in [`data.event`](../../Kernel/Wizardry/Common/SaveData/data.event) | SUS-only `0x478` EMS chunk |
| Chapter clear | `ChapterInit_ResetChatlog` via [`ChapterInitHook/data.event`](../../Kernel/Wizardry/Common/ChapterInitHook/data.event) | Wipe history on chapter start |
| Public API | [`chatlog.h`](../../include/kernel/chatlog.h) | Caps, structs, and hook entry points |

---

## TODO

- Tune blend coefficients / panel width against half-body portrait layouts.
- Optionally filter prep Event Replay / world-map Talk from the chapter ring.
- Consider a longer page text field if English rewrites routinely truncate at 72 bytes.

---

## Limitations & Bugs

- Chatlog is **Talk-only**; map SELECT is intentionally untouched.
- History does **not** persist in normal save files—only suspend.
- Opening the overlay temporarily owns the left BG0 region (Talk glyphs there are restored from a backup on close).
- Chibis use portrait mini-faces, not half-body OBJ sprites.
- Portrait→name resolution uses an exact `portraitId` character-table match; some variant faces may show a blank name.
- B-skipped (`instantScroll`) pages are still logged, which is intentional for completeness.

Please report issues in the repository’s **Issues** tab.
