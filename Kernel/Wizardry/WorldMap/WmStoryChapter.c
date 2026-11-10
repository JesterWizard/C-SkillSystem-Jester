#include "common-chax.h"
#include "wm-story-chapter.h"
#include "district.h"
#include "save-data.h"

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

/*
 * The chapter a side map hands back to the world map with (gPlaySt.chapterIndex after a skirmish or a district is
 * left: the retreat's next chapter). Vanilla skips the save menu for it (GameControl_CallPostChapterSaveMenu).
 */
#define WM_CHAPTER_SIDE_MAP_RETURN 0x38

int GetWmStoryChapterRank(int chapter)
{
	const struct WmStoryChapterNode *it;
	int rank = 0;

	if (chapter < 0 || chapter >= WM_STORY_CHAPTER_NONE)
		return -1;

	for (it = gWmStoryChapterNodes; it->chapter != WM_STORY_CHAPTER_NONE; it++, rank++)
		if (it->chapter == chapter)
			return rank;

	return -1;
}

int WmStoryRank(void)
{
	int rank = GetWmStoryChapterRank(gWmStoryChapter);

	return rank < 0 ? 0 : rank;
}

/* A chapter outside the story table counts from the start */
bool WmStoryReached(int chapter)
{
	return GetWmStoryChapterRank(chapter) <= WmStoryRank();
}

/*
 * The story chapter's node on the world map, or -1. In the postgame (after the ending, from Extras > Map) the story is
 * over: the last story chapter stays in gWmStoryChapter for WmStoryReached (district life, unlocks), but its node is no
 * longer the next one, and every node starts its vanilla chapter, as in vanilla's postgame.
 */
static int WmStoryNodeNow(void)
{
	if (gPlaySt.chapterStateBits & PLAY_FLAG_POSTGAME)
		return -1;
	return GetWmStoryChapterNode(gWmStoryChapter);
}

/*
 * The idle world map (WorldMap_LoopExt): the story chapter's node, once shown, is the "next" node (state bit 2), so a
 * trip there starts the chapter (vanilla's travel loop checks the bit, and only then ignores a monster on the node). A
 * chapter's SET_NODE script sets it with WM_SETDESTINATION; without that (Chapter 10's script had only
 * WM_CREATENEXTDESTINATION), the trip ended on the node, a monster there started a skirmish, and A opened the node menu.
 * Only for a trip: with the lord already on the node, the bit would make A start the chapter instead of the node menu.
 */
void WmStoryChapter_KeepDestination(void)
{
	int node = WmStoryNodeNow();

	if (node < 0 || node >= NODE_MAX || gGMData.units[0].location == node)
		return;
	if ((gGMData.nodes[node].state & 1) && !(gGMData.nodes[node].state & 2))
		gGMData.nodes[node].state |= 2;
}

/* WorldMap_CallBeginningEvent: chapter is the MNCH target (gPlaySt.chapterIndex) */
void WmStoryChapter_Begin(int chapter)
{
	/*
	 * Back from a district (or a skirmish): no chapter was named, so the story stays where it was. Without this, every
	 * district visit cleared it, and the world map, district unlocks, featured quests, the time of day and the rival
	 * all fell back to their no-story state.
	 */
	if (chapter == WM_CHAPTER_SIDE_MAP_RETURN || District_IsDistrictChapter(chapter))
		return;

	/* The story chapter targeted before this one has just been cleared */
	if (gWmStoryChapter != WM_STORY_CHAPTER_NONE && gWmStoryChapter != chapter &&
	    GetWmStoryChapterNode(chapter) >= 0)
		District_MarkStoryChapterCleared(gWmStoryChapter);

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
	int story = WmStoryNodeNow();
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
	int story = WmStoryNodeNow();

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

/*
 * The story chapter in a game save slot, or WM_STORY_CHAPTER_NONE, without loading the save: its chunk is found by
 * its loader in gEmsSavChunks, which lays the chunks out in order. The save menu titles slots with it (#705).
 */
int WmStoryChapter_ReadFromSave(int slot)
{
	const struct EmsChunk *cur;
	u32 offset = 0;
	u16 buffer[2];

	for (cur = gEmsSavChunks; cur->_identifier_ != EMS_CHUNK_INVALID_OFFSET; cur++) {
		if (cur->load == WmStoryChapter_Load) {
			ReadSramFast((const u8 *)GetSaveReadAddr(slot) + offset, buffer, sizeof(buffer));
			return buffer[1] == WM_STORY_CHAPTER_MAGIC ? buffer[0] : WM_STORY_CHAPTER_NONE;
		}
		offset += cur->size;
	}
	return WM_STORY_CHAPTER_NONE;
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
