#include "common-chax.h"
#include "district.h"
#include "wm-story-chapter.h"
#include "map-colors.h"

/*
 * District life (include/kernel/district.h): the time-of-day tint on a district map's palettes, and the townsfolk's
 * places by the story chapter. The tables are in Data/District/DistrictLife.c.
 */

/* Free movement (Kernel/Wizardry/FreeMovement/cFreeMU/src/FreeMU.h, not on the include path) */
extern bool FMU_CanUnitBeOnPos(struct Unit *unit, s8 x, s8 y);

/* The chapters' map palettes, moved out of gChapterDataAssetTable (CustomCampaign/Event/PListSplit.event) */
extern const void *const SplitedChapterData_ObjPal[];

bool District_IsDistrictChapter(int chapter)
{
	const struct DistrictNode *it;

	for (it = gDistrictNodes; it->node != DISTRICT_NODE_END; it++)
		if (it->chapter == chapter)
			return TRUE;
	return FALSE;
}

int District_TimeOfDay(void)
{
	const struct DistrictTime *it;
	int time = DISTRICT_TIME_DAY;

	for (it = gDistrictTimes; it->storyChapter != DISTRICT_LIFE_END; it++)
		if (WmStoryReached(it->storyChapter))
			time = it->time;
	return time < DISTRICT_TIMES ? time : DISTRICT_TIME_DAY;
}

static int District_TintChannel(int c, int mul, int add)
{
	c = ((c * mul) >> 4) + add;
	return c > 31 ? 31 : c;
}

/* The map's ten palettes (BG palettes 6 to 15), as UnpackChapterMapPalette loads them */
void District_TintMapPalettes(void)
{
	const struct DistrictTint *t;
	u16 *pal = gPaletteBuffer + 0x10 * 6;
	int i, c;

	if (!District_IsDistrictChapter(gPlaySt.chapterIndex))
		return;
	t = &gDistrictTints[District_TimeOfDay()];
	if (t->mul[0] == 16 && t->mul[1] == 16 && t->mul[2] == 16 && !t->add[0] && !t->add[1] && !t->add[2])
		return;
	for (i = 0; i < 0x10 * 10; i++) {
		c = pal[i];
		pal[i] = District_TintChannel(c & 0x1F, t->mul[0], t->add[0]) |
			 (District_TintChannel((c >> 5) & 0x1F, t->mul[1], t->add[1]) << 5) |
			 (District_TintChannel((c >> 10) & 0x1F, t->mul[2], t->add[2]) << 10);
	}
	EnablePaletteSync();
}

/* Vanilla, but for the palette table PListSplit.event moved, the map colours and the tint */
LYN_REPLACE_CHECK(UnpackChapterMapPalette);
void UnpackChapterMapPalette(void)
{
	ApplyPalettes(SplitedChapterData_ObjPal[GetROMChapterStruct(gPlaySt.chapterIndex)->map.paletteId], 6, 10);
	MapColors_ApplyRange(gPaletteBuffer + 0x10 * 6, 0x10 * 10);
	District_TintMapPalettes();
}

/*
 * Vanilla, but for the table PListSplit.event moved, the map colours and the tint: the map is redrawn this way after
 * the Options and stat screens and battles
 */
LYN_REPLACE_CHECK(UnpackChapterMapGraphics);
void UnpackChapterMapGraphics(int chapterId)
{
	const struct ROMChapterData *ch = GetROMChapterStruct(chapterId);

	Decompress(SplitedChapterData_ObjPal[ch->map.obj1Id], (void *)0x06008000);
	if (SplitedChapterData_ObjPal[ch->map.obj2Id])
		Decompress(SplitedChapterData_ObjPal[ch->map.obj2Id], (void *)0x0600C000);

	ApplyPalettes(SplitedChapterData_ObjPal[ch->map.paletteId], 6, 10);
	MapColors_ApplyRange(gPaletteBuffer + 0x10 * 6, 0x10 * 10);
	District_TintMapPalettes();
}

/* Called as the district's NPCs start (StartDistrictNpcs), before they take their places as home */
void District_PlaceNpcs(void)
{
	const struct DistrictNpcSpot *it, *spot;
	struct Unit *unit;
	int id, chapter = gPlaySt.chapterIndex;
	bool moved = FALSE;

	for (id = 0x41; id <= 0x7F; id++) {
		unit = GetUnit(id);
		if (!unit || !unit->pCharacterData)
			continue;
		spot = NULL;
		for (it = gDistrictNpcSpots; it->chapter != DISTRICT_LIFE_END; it++)
			if (it->chapter == chapter && it->pid == unit->pCharacterData->number &&
			    WmStoryReached(it->storyChapter))
				spot = it;
		if (!spot || (spot->x == unit->xPos && spot->y == unit->yPos))
			continue;
		if (!FMU_CanUnitBeOnPos(unit, spot->x, spot->y))
			continue;
		gBmMapUnit[unit->yPos][unit->xPos] = 0;
		unit->xPos = spot->x;
		unit->yPos = spot->y;
		gBmMapUnit[unit->yPos][unit->xPos] = id;
		moved = TRUE;
	}
	if (moved) {
		RefreshEntityBmMaps();
		RefreshUnitSprites();
	}
}
