#pragma once

#include "common-chax.h"

/*
 * Story chapters started from the world map: see
 * Kernel/Wizardry/WorldMap/WmStoryChapter.c.
 */

#define WM_STORY_CHAPTER_NONE 0xFF

struct WmStoryChapterNode {
	u8 chapter;
	u8 node;
};

/* Campaign table (CustomCampaign/Event/World_Map_Events.c), ends with { WM_STORY_CHAPTER_NONE, 0 } */
extern const struct WmStoryChapterNode gWmStoryChapterNodes[];

/* Chapter named by the last NEXT_CHAPTER_WITH_MAP, or WM_STORY_CHAPTER_NONE (include/link/config-memmap.s) */
extern u8 gWmStoryChapter;

int GetWmStoryChapterNode(int chapter);

/* Idle world map: the story chapter's node, once shown, stays the "next" node (WorldMap_LoopExt) */
void WmStoryChapter_KeepDestination(void);

/*
 * Where a chapter comes in the story: its row in gWmStoryChapterNodes (0: the prologue), or -1. Story chapters are
 * compared by this, not by id: CHAPTER_11 is 0x3D, after every other story chapter's id.
 */
int GetWmStoryChapterRank(int chapter);

/* How far the story is: the rank of gWmStoryChapter, 0 before the first story chapter */
int WmStoryRank(void);

/* A row that counts from story chapter `chapter` on: true once the story has reached it */
bool WmStoryReached(int chapter);

/* Synthesize and Infuse unlock at a story chapter (Data/DesignerConfig/designer-config.c); 0xFF: not yet */
#define PREP_MENU_LOCKED 0xFF
extern const u8 gSkillSynthUnlockChapter;
extern const u8 gInfuseUnlockChapter;
extern const u8 gValniFloorUnlockChapter[8];
int ValniFloors_NodeChapter(int chapter);
bool PrepMenu_SkillSynthUnlocked(void);
bool PrepMenu_InfuseUnlocked(void);
void WmStoryChapter_Begin(int chapter);
int GetWmLocChapter(int node);

void WmStoryChapter_NewGame(void);
void WmStoryChapter_Save(u8 *dst, const u32 size);
void WmStoryChapter_Load(u8 *src, const u32 size);
int WmStoryChapter_ReadFromSave(int slot);
