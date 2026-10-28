#include "common-chax.h"
#include "wm-story-chapter.h"

/*
 * Story chapters started from the world map.
 *
 * Vanilla works the next story chapter out from the world map itself: the
 * lord's node, then that node's "next" node (gWMNodeData[].unk_08, switched
 * to its second pair by the event flag in unk_06), then that node's single
 * chapter id. So a node can only ever start one chapter, and a chapter only
 * plays when the lord stands where the vanilla graph expects.
 *
 * Here the chapter named by the last NEXT_CHAPTER_WITH_MAP (MNCH) is the
 * story chapter, and gWmStoryChapterNodes says which node it starts from, so
 * any number of chapters can share a node. Chapters missing from the table
 * keep the vanilla behaviour, and so do the other nodes (skirmishes).
 */

/* Node the chapter starts from, or -1 if it is not in the campaign table */
int GetWmStoryChapterNode(int chapter)
{
	const struct WmStoryChapterNode *it;

	if (chapter < 0 || chapter >= WM_STORY_CHAPTER_NONE)
		return -1;

	for (it = gWmStoryChapterNodes; it->chapter != WM_STORY_CHAPTER_NONE; it++)
		if (it->chapter == chapter)
			return it->node;

	return -1;
}

/* WorldMap_CallBeginningEvent: chapter is the MNCH target (gPlaySt.chapterIndex) */
void WmStoryChapter_Begin(int chapter)
{
	if (GetWmStoryChapterNode(chapter) >= 0)
		gWmStoryChapter = chapter;
	else
		gWmStoryChapter = WM_STORY_CHAPTER_NONE;
}

/*
 * WMLoc_GetChapterId: the story chapter at its own node, and nowhere else
 * (vanilla's chapter-less nodes also use 0xFF); vanilla ids elsewhere.
 */
int GetWmLocChapter(int node)
{
	int story = GetWmStoryChapterNode(gWmStoryChapter);
	int chapter;

	if (gPlaySt.chapterModeIndex == CHAPTER_MODE_EPHRAIM)
		chapter = gWMNodeData[node].chapteridx_ephram;
	else
		chapter = gWMNodeData[node].chapteridx_eirika;

	if (story < 0)
		return chapter;

	if (node == story)
		return gWmStoryChapter;

	if (chapter == gWmStoryChapter)
		return WM_STORY_CHAPTER_NONE;

	return chapter;
}

//! FE8U = 0x080BB5E4
LYN_REPLACE_CHECK(WMLoc_GetNextLocId);
int WMLoc_GetNextLocId(int node)
{
	const s8 *next = gWMNodeData[node].unk_08;
	int story = GetWmStoryChapterNode(gWmStoryChapter);

	/* The story chapter's node comes next, wherever the lord is */
	if (story >= 0)
		return story;

	if (CheckFlag(gWMNodeData[node].unk_06))
		next += 2;

	if (gPlaySt.chapterModeIndex == CHAPTER_MODE_EPHRAIM)
		return next[1];

	return next[0];
}

void WmStoryChapter_NewGame(void)
{
	gWmStoryChapter = WM_STORY_CHAPTER_NONE;
}

#define WM_STORY_CHAPTER_MAGIC 0x5753

void WmStoryChapter_Save(u8 *dst, const u32 size)
{
	u16 buffer[2];

	buffer[0] = gWmStoryChapter;
	buffer[1] = WM_STORY_CHAPTER_MAGIC;
	WriteAndVerifySramFast(buffer, dst, sizeof(buffer));
}

/* Saves made before this chunk existed fall back to the vanilla graph */
void WmStoryChapter_Load(u8 *src, const u32 size)
{
	u16 buffer[2];

	ReadSramFast(src, buffer, sizeof(buffer));

	if (buffer[1] == WM_STORY_CHAPTER_MAGIC)
		gWmStoryChapter = buffer[0];
	else
		gWmStoryChapter = WM_STORY_CHAPTER_NONE;
}
