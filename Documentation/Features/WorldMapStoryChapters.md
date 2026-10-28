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
- `gWmStoryChapter` is saved in both the normal and the suspend save. Saves made before this feature, and chapters missing from the table, keep the vanilla behaviour.

## Adding a chapter

Add a row to `gWmStoryChapterNodes` in `CustomCampaign/Event/World_Map_Events.c`:

```c
{ CHAPTER_11, WM_NODE_Serafew },
```

- The chapter's own world map script (`EventScrWM_SET_NODE[gmapEventId]`) should still reveal that node as the destination (`WM_REVEAL_DEST` / `WM_SETDESTINATION`). The player can only travel there along an open path from the lord's node.
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
