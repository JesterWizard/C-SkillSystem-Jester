# How to Make a Chapter

---

## 📑 Index
- [Introduction](#introduction)
- [Plan](#plan)
- [Code Locations](#code-locations)
- [TODO](#todo)
- [Limitations & Bugs](#limitations--bugs)

---

## 🧩 Introduction

In this build, one chapter lives in one folder under `Data/CustomCampaign/Chapters/NN/`. That folder holds the map scripts, unit groups, ROM chapter data, dialogue, and world-map intro. The player only sees one chapter, but the build still needs matching entries for the event pointer table, the map data, the chapter event group, and the chapter ROM data.

The same instructions live next to the content in [Data/CustomCampaign/README.md](../../Data/CustomCampaign/README.md). Use that file when you are working inside the campaign folder.

If those pieces do not agree, the chapter usually fails in one of a few predictable ways: the wrong map loads, the wrong unit group appears in prep, the chapter has no map changes, or the chapter data points at the wrong event script. This guide shows the structure the build expects so you can replace a vanilla chapter with your own content without fighting the file layout.

---

## 🛠️ Plan

Start from an existing folder under `Chapters/NN/` and keep that shape:

| File | What it owns |
|------|----------------|
| `events/events.c` | `struct ChapterEventGroup` plus unique names like `EventScr_Ch04_Opening` |
| `events/events.h` | Opening, ending, turn, talk, village, and misc scripts |
| `events/units.h` | Unit groups loaded at chapter start |
| `events/redas.h` | Reinforcement movement |
| `events/traps.h` | Startup traps, only if this map has any; otherwise use `TrapData_None` |
| `events/asmc.h` | Custom C helpers, only if this chapter needs them |
| `events/chapter.c` | `ROMChapterData` (fog, BGM, title, camera, goal) |
| `events/worldmap.c` | World-map set-node / travel scripts for this chapter |
| `map/` | TMX, dmp, and generated map event |
| `text/` | `opening.txt`, `map.txt`, `ending.txt` |
| `music/` | Voice-acted intro lines (optional). Add a line event and list it in `music/installer.event` |

Shared empty lists and headers live in `Chapters/_shared/`. Supports, quotes, and guides stay in `Text/`.

In the chapter event group:

- Set `.playerUnitsInNormal` and `.playerUnitsInHard` to a unit group name when the chapter uses prep.
- Leave encounter-only unit pointers as `NULL` unless the chapter actually uses them.
- Point unused select/move/tutorial slots at `EventListScr_Empty` / `EventListScr_Tutorial_None`.
- Keep `.beginningSceneEvents` and `.endingSceneEvents` aligned with `EventScr_ChNN_Opening` / `EventScr_ChNN_Ending`.

### 1. Wire the map event

Export the map as TMX, convert it with TMX2EA, and place the generated files under the chapter's `map/` folder. The generated map event is where the chapter's map metadata gets fixed up.

The generated file needs two things to be correct:

- `SetChapterData(...)` must match the chapter you are replacing in FEBuilder.
- `NewEventPointerTable_Event(mapId, MapData)` must use the same map ID that the chapter data expects.

The build's chapter installer then needs an entry that includes both the event script and the map event. The basic pattern looks like this:

```c
NewEventPointerTable_Event(0x17, Chapter04Event)
#include "../Chapters/04/events/events.lyn.event"
#include "../Chapters/04/map/04_map.event"
```

For a different chapter, replace `0x17`, `Chapter04Event`, and the file paths with the chapter you are actually targeting. The number in `NewEventPointerTable_Event(...)` is the chapter event ID shown in FEBuilder.

### 2. Fill in chapter.c

Keep `chapter.c` in `Chapters/NN/events/`.

- `chapter.c` defines `const struct ROMChapterData ChapterNN`.
- `chapter.lyn.event` emits the chapter pointer entry (included from `Chapters/Chapter_Installer.event`).
- The `.map` block in `chapter.c` controls the object layers, palette, tile config, main map pointer, and change layer pointer.

The map IDs and chapter IDs need to match the event side. If the map event says one thing and the chapter data says another, the chapter will still build but it will load the wrong data at runtime.

### 3. Rebuild and check the obvious failures first

When the chapter does not behave as expected, the first things to verify are:

- The chapter event ID in `Event_Installer.event`.
- The map ID used by `SetChapterData(...)` and `MapChangesData`.
- The chapter's `.playerUnitsInNormal` and `.playerUnitsInHard` bindings.
- The chapter title, prep screen flag, and map pointer values in `Chapters/NN/events/chapter.c`.

If the chapter has no prep screen, the player unit fields should usually be `NULL`. If the chapter does have prep, those fields need to point at a real unit struct name.

---

## 🗂️ Code Locations

| Feature | Location | Description |
|--------|----------|-------------|
| Campaign contributor guide | [Data/CustomCampaign/README.md](../../Data/CustomCampaign/README.md) | Event wiring, text, maps, world map, and voice song IDs next to the content |
| Chapter ROM installer | [Data/CustomCampaign/Chapters/Chapter_Installer.event](../../Data/CustomCampaign/Chapters/Chapter_Installer.event) | Writes each `ROMChapterData` blob into the chapter table |
| Chapter event group | [Data/CustomCampaign/Chapters/04/events/events.c](../../Data/CustomCampaign/Chapters/04/events/events.c) | Binds events, traps, and player unit groups for a chapter |
| Chapter event scripts | [Data/CustomCampaign/Chapters/04/events/events.h](../../Data/CustomCampaign/Chapters/04/events/events.h) | Holds the chapter's scripted event flow |
| Chapter unit groups | [Data/CustomCampaign/Chapters/04/events/units.h](../../Data/CustomCampaign/Chapters/04/events/units.h) | Defines startup unit groups and inventories |
| Chapter reinforcements | [Data/CustomCampaign/Chapters/04/events/redas.h](../../Data/CustomCampaign/Chapters/04/events/redas.h) | Defines reinforcement behavior after units spawn |
| Shared empty lists | [Data/CustomCampaign/Chapters/_shared/empty-event-lists.c](../../Data/CustomCampaign/Chapters/_shared/empty-event-lists.c) | Shared `EventListScr_Empty`, `TrapData_None`, and empty tutorial list |
| Map event data | [Data/CustomCampaign/Chapters/04/map/04_map.event](../../Data/CustomCampaign/Chapters/04/map/04_map.event) | Generated map payload plus `SetChapterData(...)` and the map-change table |
| Chapter ROM data | [Data/CustomCampaign/Chapters/04/events/chapter.c](../../Data/CustomCampaign/Chapters/04/events/chapter.c) | Defines the chapter's `ROMChapterData` entry |
| Chapter dialogue | [Data/CustomCampaign/Chapters/04/text](../../Data/CustomCampaign/Chapters/04/text) | Opening, in-map, and ending text IDs |
| World-map scripts | [Data/CustomCampaign/Chapters/04/events/worldmap.c](../../Data/CustomCampaign/Chapters/04/events/worldmap.c) | Set-node and travel events for this chapter |
| Chapter voice lines | [Data/CustomCampaign/Chapters/04/music](../../Data/CustomCampaign/Chapters/04/music) | Per-chapter voiced intro songs; listed from `music/installer.event` |
| Voice song IDs | [Data/CustomCampaign/Music/assign_voice_song_ids.py](../../Data/CustomCampaign/Music/assign_voice_song_ids.py) | Picks unused vanilla song-table slots; run `make assign_voice_songs` after adding a line with `SongTable(AUTO, ...)` |

---

## 📝 TODO

- Add a worked example that replaces one vanilla chapter end-to-end.
- Add a short checklist for custom maps that do not reuse vanilla tile configs.
- Document how to set up branching encounters if a chapter needs them.
- Add screenshots for the event folder and chapter folder layouts once the asset references are finalized.

---

## 🐛 Limitations & Bugs

- The build is currently limited to the base 0x4E map total, so there is no PList expansion to 0xFF.
- Custom maps and tilesets may need manual adjustment of the `SetChapterData(...)` values even when the folder structure is correct.
- The guide assumes the existing chapter source layout in this repository; if a future refactor changes the folder names, the paths in the examples will need to be updated.
