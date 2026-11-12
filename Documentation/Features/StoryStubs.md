# Story Stubs

## 📑 Index
- [Introduction](#introduction)
- [The Stub Chapters](#the-stub-chapters)
- [What a Stub Does](#what-a-stub-does)
- [Replacing a Stub with a Real Chapter](#replacing-a-stub-with-a-real-chapter)
- [Testing](#testing)
- [Code Locations](#code-locations)
- [Limitations](#limitations)

---

## 🧩 Introduction

The campaign's chapters are written up to Chapter 9. Every story chapter after that, to the end of the Eirika route, is a
**stub**: it uses its vanilla chapter's map and title card, says it is a placeholder, and moves the story on. So the
world map can be played from start to end, and everything that follows the story (district unlocks, featured quests,
the time of day, the townsfolk, the rival) has its later chapters to work with.

---

## 📚 The Stub Chapters

| Chapter | Id | Vanilla title | Node | Event slot | World-map event id |
|---|---|---|---|---|---|
| 10 | `0x0B` | Revolt at Carcino | Teraz Plateau | `0x33` | `0x0C` |
| 11 | `0x3D` | Creeping Darkness | Caer Pelyn | `0xD1` | `0x18` (vanilla `0x39`) |
| 12 | `0x0C` | Village of Silence | Caer Pelyn | `0x37` | `0x0D` |
| 13 | `0x0D` | Hamill Canyon | Hamill Canyon | `0x3B` | `0x0E` |
| 14 | `0x0E` | Queen of White Dunes | Jehanna Hall | `0x41` | `0x0F` |
| 15 | `0x0F` | Scorched Sand | Jehanna Hall | `0x47` | `0x10` |
| 16 | `0x10` | Ruled by Madness | Renais Castle | `0x4B` | `0x11` |
| 17 | `0x11` | River of Regrets | Narube River | `0x4F` | `0x12` |
| 18 | `0x12` | Two Faces of Evil | Neleras Peak | `0x56` | `0x13` |
| 19 | `0x13` | Last Hope | Rausten Court | `0x5A` | `0x14` |
| 20 | `0x14` | Darkling Woods | Black Temple | `0x5E` | `0x15` |
| 21 | `0x15` | Sacred Stone (1) | (no world map) | `0x64` | |
| 22 | `0x16` | Sacred Stone (2) | (no world map) | `0x67` | |

The ids and names are `jester_headers/maps.h`'s (`CHAPTER_10` to `CHAPTER_22`). The titles are the ones on the vanilla
chapters' title cards. The nodes follow vanilla's world map:
- Chapters 11 and 12 share Caer Pelyn, and 14 and 15 share Jehanna Hall.
- Chapter 15 doesn't use vanilla's second Jehanna Hall node (19), because that node is drawn on top of the first.
- Chapters 16 and 17 start the lord on node 19, the neighbour of Renais Castle and Narube River.

Caer Pelyn's district opens after Chapter 12 and Jehanna Hall's after Chapter 15, their places in the story.

---

## 🎬 What a Stub Does

- **The world map before it.** The lord is put on the last chapter's node. The stub's node is revealed and set as the
  destination, and a road is drawn to it (`WM_REVEAL_DEST`). Chapter 10 uses its own world-map scripts, in `Chapters/10`.
- **The trip.** The lord is put on the node, with no scene. The written chapters walk the lord there in their own
  travel scenes.
- **The chapter.** The vanilla title card plays, then the map. A text box reads "This chapter has not been written yet.
  The story moves on to the next one."
- **The battle (#700).** The first few enemies of vanilla's own army for the chapter appear on their starting tiles:
  usually the boss first, at vanilla's levels, 5 to 7 of them (fewer where vanilla's list is shorter: 4 in Chapter 17, 2 in
  Chapter 19, the final boss alone in Chapter 22). The preparations screen deploys the party on vanilla's starting
  tiles. The goal is "Defeat all enemies" (`Stubs.event` sets it on the chapter rows). Winning earns EXP, items and the
  chapter awards like any battle.
  - Vanilla's unit lists are read in place from the ROM: `STUB_CHAPTER` names vanilla's player list (the event group's
    `playerUnitsInNormal`) and enemy list (the largest one its opening loads), and how many enemies to load.
    `StubChapter_LoadEnemies` (an ASMC) loads them with `LoadUnit`, so vanilla's movement after loading (REDAs) is
    skipped.
- **The win.** `DEFEAT_ALL` runs the win script: `NEXT_CHAPTER_WITH_MAP` to the next story chapter, through the save
  menu.
- **Chapters 20 to 22.** They follow each other without the world map, as in vanilla. Chapter 22's win runs `MNC4`, and
  the game's ending plays. The ending plays Chapter 22's map again with its opening, so the opening checks for a complete
  game first (`StubChapter_CheckGameComplete`) and then goes straight on, with no battle. The save menu before it writes the Creature Campaign save, and after "Fin" the game goes on into the postgame
  ([World Map Story Chapters](./WorldMapStoryChapters.md#the-postgame)).
- **The world map before Chapter 20** also shows the Lagdou Ruins and Melkaen Coast with their roads, as vanilla's does
  (`STUB_SET_NODE_WITH(..., WM_SHOW_POSTGAME_DUNGEONS)`). `new_chapter.py` keeps that line's extra events.

---

## ✍️ Replacing a Stub with a Real Chapter

### With the tool

```bash
python3 Tools/chapter/new_chapter.py 12
```

It turns the stub into a chapter folder, `CustomCampaign/Chapters/12/events/`:
- `events.c`: `Chapter12Event`. It starts as a copy of the stub (the placeholder line, then the same next chapter), so
  the game plays as before. Its comments list what to change.
- `units.c`: an empty party list.
- `worldmap.c`: the stub's `SET_NODE` and `TRAVEL_TO_NODE` scripts, written out under the same names, so the world-map
  script tables need no change. Chapter 10 already has its own, and keeps them.

It also wires the folder in:
- The chapter's event slot moves from `Stubs/Stubs.event` to `CustomCampaign/Event/Event_Installer.event`, with the new
  `.lyn.event` includes.
- Its `STUB_CHAPTER` and `STUB_SET_NODE` lines go.

`--dry-run` prints the changes first. It stops when the folder already has an `events.c` (Chapter 10 has its old
vanilla one): move it away, or pass `--force`. Then build (`Tools/cloud/build.sh` or `make` builds the new
`.lyn.event` files) and run `python3 Tools/cloud/smoketest.py --only stubs`, which should still pass. Move the
chapter's row out of the table above.

The new `events.c` keeps the stub's battle (vanilla's tiles and first enemies). To write the chapter, follow its
comments:
1. Fill in `units.c`, point `playerUnitsInNormal`/`InHard` at it, and load the enemies in the opening in place of the
   `SVAL`s and `StubChapter_LoadEnemies`.
2. Take the placeholder `TEXT` out of the opening and write its scene.
3. Keep a win condition in the misc events (`DEFEAT_ALL`, a boss, a seize) that runs the win script.
4. Add `chapter.c` and a map folder, like Chapter 9, if it changes the chapter row or the map. The stub's goal patch
   in `Stubs.event` stays until then.

### By hand

1. Write the chapter in its own folder under `CustomCampaign/Chapters/`, like Chapter 9:
   - `events/events.c` with its `ChapterEventGroup`;
   - `chapter.c` if it changes the chapter row;
   - a map, if it isn't the vanilla one;
   - world-map scripts.
2. Point the chapter's event slot at the new group: in `CustomCampaign/Chapters/Stubs/Stubs.event`, change its
   `NewEventPointerTable_Event` line, or move the line to `CustomCampaign/Event/Event_Installer.event`.
3. Delete the chapter's `STUB_CHAPTER` line in `Stubs/events/events.c`.
4. For its world map, either keep the stub's `SET_NODE` and `TRAVEL_TO_NODE` scripts (`Stubs/events/worldmap.c`) or
   replace them in `EventScrWM_SET_NODE` and `EventScrWM_TRAVEL_TO_NODE` (`CustomCampaign/Event/World_Map_Events.c`,
   indexed by the chapter's `gmapEventId`).
   - The `SET_NODE` script must reveal the node as the destination (`WM_REVEAL_DEST` or `WM_SETDESTINATION`). A node
     it never shows can't be travelled to. `WmStoryChapter_KeepDestination` keeps a shown story node "next", so
     arriving starts the chapter even with a monster on it.
5. End the chapter with `NEXT_CHAPTER_WITH_MAP` to the next story chapter. That chapter can be a stub.

---

## 🧪 Testing

`python3 Tools/cloud/smoketest.py --only stubs` (about 20 seconds) runs the whole route on the world map:
- It leaves a test district with the story at Chapter 10 and the lord on Port Kiris.
- For each story chapter, it puts the cursor on the chapter's node and presses A, then skips the scenes.
- In each battle it checks the enemies loaded, then wins: it takes them off the map, sets the defeat-all flag
  (`EVFLAG_DEFEAT_ALL`, in `gChapterFlagBits`), and Eirika waits.
- After each chapter, it checks the next story chapter and where the lord starts.
- After Chapter 20, it checks that Chapters 21 and 22 lead to the ending (`PLAY_FLAG_COMPLETE`).
- Before Chapter 10, it puts a monster on Teraz Plateau and clears the node's "next" bit: the trip must still start the
  chapter (#690, [World Map Story Chapters](./WorldMapStoryChapters.md#monsters-and-the-story-node)).

---

## 🗂️ Code Locations

| Part | Location |
|---|---|
| The stubs | `CustomCampaign/Chapters/Stubs/events/events.c` (`STUB_CHAPTER`) |
| World-map scripts | `CustomCampaign/Chapters/Stubs/events/worldmap.c` (`STUB_SET_NODE`), Chapter 10's in `Chapters/10/events/worldmap.c` |
| Event slots, Chapter 11's world-map id | `CustomCampaign/Chapters/Stubs/Stubs.event` |
| Story table, world-map script tables | `CustomCampaign/Event/World_Map_Events.c` |
| Story order | `GetWmStoryChapterRank`, `WmStoryReached` in `Kernel/Wizardry/WorldMap/WmStoryChapter.c` |
| The placeholder line | `MSG_STORY_STUB_CHAPTER`, `Contents/Texts/Source/texts/Districts.txt` |
| Turning a stub into a chapter folder | `Tools/chapter/new_chapter.py` |

---

## ⚠️ Limitations

- A stub's battle is small and generic: vanilla's first enemies, no scenes, no reinforcements, no villages or chests.
- A vanilla list doesn't always start with its boss, so a stub may load none. Change a chapter's count, or its list,
  in `STUB_CHAPTER`.
- The trip to a stub's node is a jump, with no walking scene.
- Chapter 10 had vanilla's events until now (the old event group was commented out). Its world-map scripts in
  `Chapters/10` stay. Its `SET_NODE` script now uses `WM_REVEAL_DEST`: with `WM_CREATENEXTDESTINATION` alone, arriving on
  Teraz Plateau didn't start the chapter.
