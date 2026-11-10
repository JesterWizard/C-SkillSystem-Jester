# World Map Story Chapters

## Introduction

In vanilla FE8 the world map, not your events, decides which chapter plays next:

1. The lord's node (`gGMData.units[0].location`, set by `WM_SPAWNLORD` / `WM_SETUNITONNODE`).
2. That node's "next" node in `gWMNodeData[].unk_08`. An event flag in `unk_06` switches it to a second pair: flag 136 on Castle Frelia and Serafew, 137 on Border Mulan.
3. That next node's single chapter id (`gWMNodeData[].chapteridx_eirika` / `_ephram`).

So a node can only ever start one chapter. A chapter only plays if the lord stands where that graph expects. The chapter given to `NEXT_CHAPTER_WITH_MAP` is overwritten as soon as the world map opens.

With this feature, the chapter named by `NEXT_CHAPTER_WITH_MAP` is the story chapter, and a campaign table says which node it starts from. Several chapters can use the same node.

## How it works

- `WorldMap_CallBeginningEvent` (`CustomCampaign/Event/World_Map_Events.c`) records `gPlaySt.chapterIndex`, the `NEXT_CHAPTER_WITH_MAP` target, as `gWmStoryChapter` when that chapter is in the table.
- `WMLoc_GetNextLocId` returns the story chapter's node, wherever the lord is.
- `WMLoc_GetChapterId` returns the story chapter for its own node, and the vanilla chapter for every other node, so skirmishes are unchanged. The story chapter's id is hidden from other nodes (they report `0xFF`, like vanilla's chapter-less nodes), so `GetBattleMapKind` finds the right node.
- Back on the world map from a district or a skirmish (chapter `0x38`, or a district chapter), the story chapter is kept.
- Story chapters are compared by their place in the table (`GetWmStoryChapterRank`, `WmStoryReached`), not by id:
  `CHAPTER_11` is `0x3D`. District life, the guild rival and anything else that counts "from story chapter N on" use it.
- `gWmStoryChapter` is saved in both the normal and the suspend save. Saves made before this feature, and chapters missing from the table, keep the vanilla behaviour. `smoketest.py --only wmsave` saves from the world map, resets and loads: the story chapter, the lord's node, the unlocked districts and the nodes' states come back, and Enter District still works.

## Monsters and the story node

World-map monsters (`gGMData.units[4..7]`) spawn on and walk between nodes along open paths, and nothing stops one from
standing on the story chapter's node. The vanilla travel loop checks the trip's last node:
- If the node is the "next" one (node state bit 2), the chapter starts, whatever stands there.
- If not, and a monster is there, the trip becomes a skirmish (#690: travelling to Teraz Plateau gave "Skirmish!", a
  Preparations screen and an empty map, and Chapter 10 never started).

So the story node must keep bit 2. Two things lost it:
- Chapter 10's `SET_NODE` script had only `WM_CREATENEXTDESTINATION`, which does not set it (fixed in #686).
- FE-CLib's `struct GMNode` was 1 byte, but the game reads `gGMData.nodes` 4 bytes a node (`nodes` at `+0x30`,
  `openPaths` at `+0xA4`). Every C access to `nodes[n]`, `openPaths`, `current_node` and `unk_cd` hit the wrong byte:
  `Event97_WmInitNextStoryNode`'s writes, the shop rows' "is this node cleared" checks, the skill shop and
  `WmDynamicPath`. `Tools/FE-CLib-Mokha/include/worldmap.h` now pads the struct to 4 bytes.

`WmStoryChapter_KeepDestination` (`Kernel/Wizardry/WorldMap/WmStoryChapter.c`, called by `WorldMap_LoopExt` every
frame of the idle world map) sets bit 2 on the story node once it is shown, so a trip there always starts the chapter.
It leaves the node alone while the lord stands on it: there, bit 2 would make A start the chapter instead of opening
the node menu (and its Enter District row).

A monster on a node *between* the lord and the story node still stops the trip with a skirmish, as in vanilla.

## The story

| Chapter | Id | Node | |
|---|---|---|---|
| Prologue | `0x00` | Border Mulan | |
| Intermission | `0x38` | Castle Frelia | |
| 2 - 9 | `0x02` - `0x0A` | Ide, Borgo Ridge, Za'ha Woods, Serafew, Adlas Plains, Renvall (twice), Port Kiris | |
| 10 | `0x0B` | Teraz Plateau | [stub](./StoryStubs.md) |
| 11, 12 | `0x3D`, `0x0C` | Caer Pelyn | stubs |
| 13 | `0x0D` | Hamill Canyon | stub |
| 14, 15 | `0x0E`, `0x0F` | Jehanna Hall | stubs |
| 16 | `0x10` | Renais Castle | stub |
| 17 | `0x11` | Narube River | stub |
| 18 | `0x12` | Neleras Peak | stub |
| 19 | `0x13` | Rausten Court | stub |
| 20 | `0x14` | Black Temple | stub; 21 and 22 follow without the world map |

## Adding a chapter

Add a row to `gWmStoryChapterNodes` in `CustomCampaign/Event/World_Map_Events.c`:

```c
{ CHAPTER_11, WM_NODE_Serafew },
```

- The chapter's own world map script (`EventScrWM_SET_NODE[gmapEventId]`) should reveal that node as the destination (`WM_REVEAL_DEST` / `WM_SETDESTINATION`). `WmStoryChapter_KeepDestination` now sets the "next" bit on the shown story node too, but a node the script never shows stays hidden (see [Monsters and the story node](#monsters-and-the-story-node)). The player can only travel there along an open path from the lord's node, and `WM_REVEAL_DEST` only draws a vanilla path, between neighbouring nodes.
- Chapters reached with `NEXT_CHAPTER_WITHOUT_MAP` (e.g. Chapter 1 and 5x) need no row.
- Flag 136 no longer decides where the story goes. The events can keep using it, or stop.

## Code locations

| Part | Location |
|---|---|
| Campaign table | `gWmStoryChapterNodes`, `CustomCampaign/Event/World_Map_Events.c` |
| Logic, `WMLoc_GetNextLocId`, save chunk | `Kernel/Wizardry/WorldMap/WmStoryChapter.c` |
| `WMLoc_GetChapterId` | `Kernel/Wizardry/WorldMap/WorldMap.c` |
| Header | `include/kernel/wm-story-chapter.h` |
| RAM | `gWmStoryChapter`, `include/link/config-memmap.s` |
| Save chunks, new-game reset | `Kernel/Wizardry/SaveData/data.event` |
