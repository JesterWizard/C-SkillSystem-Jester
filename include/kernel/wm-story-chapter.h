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
void WmStoryChapter_Begin(int chapter);
int GetWmLocChapter(int node);

void WmStoryChapter_NewGame(void);
void WmStoryChapter_Save(u8 *dst, const u32 size);
void WmStoryChapter_Load(u8 *src, const u32 size);
