# Custom Campaign

---

## 📑 Index
- [Introduction](#introduction)
- [Plan](#plan)
- [Code Locations](#code-locations)
- [TODO](#todo)
- [Limitations & Bugs](#limitations--bugs)

---

## 🧩 Introduction

`Data/CustomCampaign` is the custom story pack: chapters, events, maps, dialogue, world-map intros, and voice lines. Kernel wizardry stays outside this folder. If a chapter does not show the right map, units, text, or music, the usual cause is a missing wire between the chapter folder and one of the installers listed below.

Gated by `CONFIG_FE8_REWRITE` in `Kernel/Wizardry/custom_wizardry.event`, which includes `FE8Rewrite_Installer.event`.

---

## 🛠️ Plan

### Folder layout

```
CustomCampaign/
  FE8Rewrite_Installer.event   # Event + chapters + music
  Chapters/NN/                 # One folder per chapter
    events/                    # events.c/h, units.h, redas.h, chapter.c, worldmap.c
    map/                       # TMX / dmp / generated map event
    text/                      # opening.txt, map.txt, ending.txt
    music/                     # Optional voice lines
  Chapters/_shared/            # Headers, empty lists, world-map helpers
  Event/                       # Event pointer table, lyn jumps, extra maps
  Music/                       # Song table macros + voice ID assigner
  Text/                        # Supports, quotes, guides, heroes cards
```

Copy an existing `Chapters/NN/` folder. Keep unique names (`EventScr_Ch04_Opening`, `Chapter04Event`, `Chapter04`). Shared empties live in `Chapters/_shared/` (`EventListScr_Empty`, `TrapData_None`, `EventListScr_Tutorial_None`). Only add `traps.h` / `asmc.h` when that chapter needs them.

### 1. Make the events compile and register

Chapter scripts are C (`events.c`, `events.h`, `worldmap.c`, `chapter.c`). `make` builds `*.lyn.event` next to the `.c` files.

`events.c` must export a `ChapterEventGroup` whose pointers match the scripts in `events.h`:

- `.beginningSceneEvents` / `.endingSceneEvents` → `EventScr_ChNN_Opening` / `EventScr_ChNN_Ending`
- Unused select/move/tutorial slots → `EventListScr_Empty` / `EventListScr_Tutorial_None`
- Prep chapters: `.playerUnitsInNormal` and `.playerUnitsInHard` point at a real unit group in `units.h`
- No prep: those fields are `NULL`

Include `Chapters/_shared/headers.h` from event `.c` files. That pulls `EAstdlib.h`, `jester_headers/macros.h` (`TEXT`, `LOAD_WAIT`, `HIGHLIGHT_CHARACTER`, …), text IDs, and voice song names.

Then wire three places:

| Wire | File | What to add |
|------|------|-------------|
| Event pointer + lyn + map | `Event/Event_Installer.event` | `NewEventPointerTable_Event(id, ChapterNNEvent)` plus `#include` of `events.lyn.event` and `map/*.event` |
| Chapter ROM blob | `Chapters/Chapter_Installer.event` | `ORG` the vanilla chapter slot, `#include "NN/events/chapter.lyn.event"` |
| World-map scripts | `Event/Event_Installer.event` | `#include "../Chapters/NN/events/worldmap.lyn.event"` |

`NewEventPointerTable_Event(...)` and `chapter.c`'s `.mapEventDataId` must use the same chapter event ID FEBuilder shows for that slot. `.map.mainLayerId` / `.changeLayerId` must match `SetChapterData(...)` in the generated map event.

### 2. Write event scripts

Put flow in `events.h` as `static const EventScr ...[]`. Call text labels from `text/*.txt` (`TEXT(Chapter_04_Scene_01_Convo_01)`). Call vanilla songs with `SONG_*` from `constants/songs.h`. Call voice lines with `SOUN(SONG_VOICE_CHNN_LINE_0001)` after assigning IDs (below).

World-map intros live in `events/worldmap.c`. Prologue-style `TEXTCONT` / `SOUN` pairs belong there, not in the chapter opening unless you want them on the map.

Text commands (`[A]`, `[LoadFace_Eirika]`, box types) are listed in `Notes/text_commands.txt`.

### 3. Add dialogue

Chapter lines go in `Chapters/NN/text/` (`opening.txt`, `map.txt`, `ending.txt`). Each block starts with `## LabelName`. Add the three files to `Text/Text.txt` or the label never gets an ID.

Supports, quotes, guides, and heroes cards stay under `Text/` and are already indexed from `Text/Text.txt`. Chapter titles are `Chapters/titles.txt`.

After editing text, rebuild so `constants/texts.h` picks up new `MSG_` / label enums.

### 4. Add or replace a map

Export TMX, convert with TMX2EA, keep output in `Chapters/NN/map/`. Confirm:

- `SetChapterData(...)` targets the chapter you are replacing
- `NewEventPointerTable_Event` / `.mapEventDataId` agree
- Tile changes exist only if the map actually has them (prologue / ch1 currently skip extra change data)

### 5. Add voice lines without picking song IDs by hand

Vanilla `gSongTable` is 1000 slots (`0x000`–`0x3E7`). Unused IDs are scattered. Do not scan FEBuilder.

1. Copy `Music/Audio_Insert_Event.event` into `Chapters/NN/music/`.
2. Keep `SongTable(AUTO, YourLabel, 0)`.
3. `#include` the file from `music/installer.event`. New chapters also need an include in `Music/Music_Installer.event`.
4. Run `make assign_voice_songs` (needs `fe8.gba` at repo root).
5. Play with the generated name from `include/jester_headers/voice-songs.h`, e.g. `SOUN(SONG_VOICE_CH01_LINE_0001)`.

`python3 Music/assign_voice_song_ids.py --next` / `--list-free` inspect the pool. Existing numeric IDs are kept; `AUTO` takes the next unused vanilla hole that is not claimed by other hacks (unit-select quotes, dragon vein SFX, …).

Convert mp3 → `.s` with `Music/compress_mp3_to_s.bat`. `make` builds `.dmp` from `.s`. The `.event` is the song-table header; the `.dmp` is the audio bytes.

### 6. Rebuild

From repo root: `make -j`. Confirm the new `*.lyn.event` files exist before assuming EA included them. If a new `.c` was added, it must be pulled in by an installer `#include` of its `.lyn.event`.

---

## 🗂️ Code Locations

| Feature | Location | Description |
|--------|----------|-------------|
| Campaign entry | `FE8Rewrite_Installer.event` | Includes event, chapter, and music installers |
| Event pointer table | `Event/Event_Installer.event` | Chapter event IDs, map events, world-map lyn files |
| Engine hooks | `Event/LynJump.event` | World-map intro / prep hooks |
| Chapter ROM table | `Chapters/Chapter_Installer.event` | Writes each `ROMChapterData` into the vanilla chapter slots |
| Shared event headers | `Chapters/_shared/headers.h` | Macros, text IDs, voice song names |
| Empty lists | `Chapters/_shared/empty-event-lists.c` | Shared empty event / trap / tutorial lists |
| Example chapter group | `Chapters/04/events/events.c` | Binds scripts, units, and traps for one chapter |
| Example scripts | `Chapters/04/events/events.h` | Opening / turn / talk / village flow |
| World-map example | `Chapters/00/events/worldmap.c` | Voice `SOUN` names during the prologue intro |
| Text index | `Text/Text.txt` | Includes every chapter `text/*.txt` plus global text |
| Text commands | `Notes/text_commands.txt` | Face / box / font control codes |
| Voice ID assigner | `Music/assign_voice_song_ids.py` | Fills `SongTable(AUTO, ...)` from unused vanilla slots |
| Voice name table | `Music/voice-songs.event` and `include/jester_headers/voice-songs.h` | Generated `SONG_VOICE_*` defines |
| Kernel docs | [HowToMakeAChapter.md](../../Documentation/Features/HowToMakeAChapter.md), [VoiceActedIntros.md](../../Documentation/Features/VoiceActedIntros.md) | Longer walkthroughs with the same layout |

---

## 📝 TODO

- Document TMX2EA command line used for maps in this repo.
- Document how to add a chapter that is not replacing a vanilla slot (needs a free `ORG` and event ID).
- Drop leftover `Music/Prologue` and `Music/Chapter_*` copies once nothing points at them.

---

## 🐛 Limitations & Bugs

- Map count is still the vanilla 0x4E table; there is no PList expansion to 0xFF.
- Vanilla song table ends at `0x3E7`. Writing past that overwrites sample data unless the table is relocated. `assign_voice_song_ids.py` only uses empty slots inside that range.
- `Chapter_Installer.event` `ORG` addresses are vanilla chapter data slots. Pointing two includes at the same `ORG` will clobber a chapter.
- Intermission chapter data is commented out in `Chapter_Installer.event` because enabling it currently crashes.
- Custom tilesets often need hand-edited `SetChapterData(...)` even when the folder layout is correct.
- Report issues with a chapter ID, the installer lines you added, and whether `*.lyn.event` was generated.
