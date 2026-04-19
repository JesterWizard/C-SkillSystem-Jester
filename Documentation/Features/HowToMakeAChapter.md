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

In this build, a chapter is split across two systems: the event side that drives the map and the chapter data side that tells the engine how to load it. The player only sees one chapter, but the build needs matching entries for the event pointer table, the map data, the chapter event group, and the chapter ROM data.

If those pieces do not agree, the chapter usually fails in one of a few predictable ways: the wrong map loads, the wrong unit group appears in prep, the chapter has no map changes, or the chapter data points at the wrong event script. This guide shows the structure the build expects so you can replace a vanilla chapter with your own content without fighting the file layout.

---

## 🛠️ Plan

Chapters in this repository are built in two layers.

| Layer | What it owns | Main files |
|-------|--------------|------------|
| Events | Pre-chapter scenes, turn events, unit groups, reinforcements, traps, and map data | [Data/FE8_Rewritten_Terper/Event/Source/04/04.c](../../Data/FE8_Rewritten_Terper/Event/Source/04/04.c), [Data/FE8_Rewritten_Terper/Event/Source/04/Source/Events.h](../../Data/FE8_Rewritten_Terper/Event/Source/04/Source/Events.h), [Data/FE8_Rewritten_Terper/Event/Source/04/Map/04_map.event](../../Data/FE8_Rewritten_Terper/Event/Source/04/Map/04_map.event) |
| Chapter data | ROM chapter metadata, map IDs, prep screen settings, title text, BGM, and starting camera | [Data/FE8_Rewritten_Terper/Chapter/Source/04/04.c](../../Data/FE8_Rewritten_Terper/Chapter/Source/04/04.c), [Data/FE8_Rewritten_Terper/Chapter/Source/04/04.lyn.event](../../Data/FE8_Rewritten_Terper/Chapter/Source/04/04.lyn.event) |

### 1. Build the event side

Start from an existing chapter folder and keep the same internal shape:

- `Event/Source/NN/NN.c` defines the `struct ChapterEventGroup` for the chapter.
- `Event/Source/NN/Source/Events.h` holds turn, character, location, misc, select, and ending event scripts.
- `Event/Source/NN/Source/Units.h` holds the unit groups loaded at chapter start.
- `Event/Source/NN/Source/Redas.h` controls reinforcement behavior.
- `Event/Source/NN/Source/Traps.h` defines startup traps.
- `Event/Source/NN/Source/ASMCs.h` holds custom C helpers used by events.

In the chapter event group, the important fields are the ones that bind those pieces together:

- Set `.playerUnitsInNormal` and `.playerUnitsInHard` to a unit group name when the chapter uses prep.
- Leave the encounter-only unit pointers as `NULL` unless the chapter actually uses them.
- Point `.traps` and `.extraTrapsInHard` at the trap arrays that belong to the chapter.
- Keep `.beginningSceneEvents` and `.endingSceneEvents` aligned with the script flow in `Events.h`.

### 2. Wire the map event

Export the map as TMX, convert it with TMX2EA, and place the generated files under the chapter's `Map` folder. The generated map event is where the chapter's map metadata gets fixed up.

The generated file needs two things to be correct:

- `SetChapterData(...)` must match the chapter you are replacing in FEBuilder.
- `NewEventPointerTable_Event(mapId, MapData)` must use the same map ID that the chapter data expects.

The build's chapter installer then needs an entry that includes both the event script and the map event. The basic pattern looks like this:

```c
NewEventPointerTable_Event(0x17, Chapter04Event)
#include "Source/04/04.lyn.event"
#include "Source/04/Map/04_map.event"
```

For a different chapter, replace `0x17`, `Chapter04Event`, and the file paths with the chapter you are actually targeting. The number in `NewEventPointerTable_Event(...)` is the chapter event ID shown in FEBuilder.

### 3. Generate the chapter data side

Create the matching chapter folder under `Chapter/Source/NN/` and keep the naming consistent with the event side.

- `NN.c` defines `const struct ROMChapterData ChapterNN`.
- `NN.lyn.event` emits the chapter pointer entry.
- The `.map` block in `NN.c` controls the object layers, palette, tile config, main map pointer, and change layer pointer.

The map IDs and chapter IDs need to match the event side. If the map event says one thing and the chapter data says another, the chapter will still build but it will load the wrong data at runtime.

### 4. Rebuild and check the obvious failures first

When the chapter does not behave as expected, the first things to verify are:

- The chapter event ID in `Event_Installer.event`.
- The map ID used by `SetChapterData(...)` and `MapChangesData`.
- The chapter's `.playerUnitsInNormal` and `.playerUnitsInHard` bindings.
- The chapter title, prep screen flag, and map pointer values in `Chapter/Source/NN/NN.c`.

If the chapter has no prep screen, the player unit fields should usually be `NULL`. If the chapter does have prep, those fields need to point at a real unit struct name.

---

## 🗂️ Code Locations

| Feature | Location | Description |
|--------|----------|-------------|
| Chapter installer table | [Data/FE8_Rewritten_Terper/Event/Event_Installer.event](../../Data/FE8_Rewritten_Terper/Event/Event_Installer.event) | Registers chapter event pointers and includes each chapter's `.lyn.event` file and map event |
| Chapter event group | [Data/FE8_Rewritten_Terper/Event/Source/04/04.c](../../Data/FE8_Rewritten_Terper/Event/Source/04/04.c) | Binds events, traps, and player unit groups for a chapter |
| Chapter event scripts | [Data/FE8_Rewritten_Terper/Event/Source/04/Source/Events.h](../../Data/FE8_Rewritten_Terper/Event/Source/04/Source/Events.h) | Holds the chapter's scripted event flow |
| Chapter unit groups | [Data/FE8_Rewritten_Terper/Event/Source/04/Source/Units.h](../../Data/FE8_Rewritten_Terper/Event/Source/04/Source/Units.h) | Defines startup unit groups and inventories |
| Chapter reinforcements | [Data/FE8_Rewritten_Terper/Event/Source/04/Source/Redas.h](../../Data/FE8_Rewritten_Terper/Event/Source/04/Source/Redas.h) | Defines reinforcement behavior after units spawn |
| Chapter traps | [Data/FE8_Rewritten_Terper/Event/Source/04/Source/Traps.h](../../Data/FE8_Rewritten_Terper/Event/Source/04/Source/Traps.h) | Defines startup trap arrays for the chapter |
| Chapter ASMC helpers | [Data/FE8_Rewritten_Terper/Event/Source/04/Source/ASMCs.h](../../Data/FE8_Rewritten_Terper/Event/Source/04/Source/ASMCs.h) | Holds custom event helpers used during chapter flow |
| Map event data | [Data/FE8_Rewritten_Terper/Event/Source/04/Map/04_map.event](../../Data/FE8_Rewritten_Terper/Event/Source/04/Map/04_map.event) | Generated map payload plus `SetChapterData(...)` and the map-change table |
| Chapter ROM data | [Data/FE8_Rewritten_Terper/Chapter/Source/04/04.c](../../Data/FE8_Rewritten_Terper/Chapter/Source/04/04.c) | Defines the chapter's `ROMChapterData` entry |
| Chapter pointer output | [Data/FE8_Rewritten_Terper/Chapter/Source/04/04.lyn.event](../../Data/FE8_Rewritten_Terper/Chapter/Source/04/04.lyn.event) | Emits the chapter pointer table entry used by the build |

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
