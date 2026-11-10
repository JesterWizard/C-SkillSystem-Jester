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
- **The chapter.** The vanilla title card plays, then the map with no units. A text box reads "This chapter has not been
  written yet. The story moves on to the next one." Then `NEXT_CHAPTER_WITH_MAP` goes to the next story chapter, through
  the save menu.
- **Chapters 20 to 22.** They follow each other without the world map, as in vanilla. Chapter 22 ends with `MNC4`, and
  the game's ending plays.

---

## ✍️ Replacing a Stub with a Real Chapter

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

---

## ⚠️ Limitations

- A stub has no units and no battle. Nothing is earned in it: no EXP, items or chapter awards.
- The trip to a stub's node is a jump, with no walking scene.
- Chapter 10 had vanilla's events until now (the old event group was commented out). Its world-map scripts in
  `Chapters/10` stay. Its `SET_NODE` script now uses `WM_REVEAL_DEST`: with `WM_CREATENEXTDESTINATION` alone, arriving on
  Teraz Plateau didn't start the chapter.
