#include "common-chax.h"
#include "kernel-lib.h"
#include "crossing.h"

#ifdef CONFIG_DISTRICT_CROSSING

/*
 * River Crossing records: the best score of each district's course, shown on the status line and the pause screen.
 * gCrossingRecords lives in RAM (include/link/config-memmap.s) and is saved as one 0x0C-byte chunk in each save list
 * (Kernel/Wizardry/SaveData/data.event). CrossingRecords_NewGame is a new-game hook.
 *
 * gCrossingProgress (fastest clears, milestones, challenges) is one more 0x10-byte save chunk in both save lists,
 * with its own magic. CrossingProgress_NewGame is a new-game hook. gCrossingExtra is RAM only.
 */

#define CROSSING_RECORDS_MAGIC 0x5243 /* "CR" */
#define CROSSING_PROGRESS_MAGIC 0x5043 /* "CP" */

typedef char CrossingRecords_SizeCheck[(sizeof(struct CrossingRecords) == 0x0C) ? 1 : -1];
typedef char CrossingState_SizeCheck[(sizeof(struct CrossingState) == 0x100) ? 1 : -1];
typedef char CrossingProgress_SizeCheck[(sizeof(struct CrossingProgress) == 0x10) ? 1 : -1];
typedef char CrossingExtra_SizeCheck[(sizeof(struct CrossingExtra) == 0x80) ? 1 : -1];

static void CrossingRecords_Clear(void)
{
	CpuFill16(0, &gCrossingRecords, sizeof(gCrossingRecords));
	gCrossingRecords.magic = CROSSING_RECORDS_MAGIC;
}

int CrossingRecords_Best(int level)
{
	if (level < 0 || level >= CROSSING_LEVELS || gCrossingRecords.magic != CROSSING_RECORDS_MAGIC)
		return 0;
	return gCrossingRecords.best[level];
}

void CrossingRecords_Add(int level, u32 score)
{
	if (level < 0 || level >= CROSSING_LEVELS)
		return;
	if (gCrossingRecords.magic != CROSSING_RECORDS_MAGIC)
		CrossingRecords_Clear();
	if (score > 0xFFFF)
		score = 0xFFFF;
	if (score > gCrossingRecords.best[level])
		gCrossingRecords.best[level] = score;
}

/* New-game hook (gNewSaveHooks) */
void CrossingRecords_NewGame(void)
{
	CrossingRecords_Clear();
}

/* Game and suspend save chunk (0x0C bytes) */
void CrossingRecords_Save(u8 *dst, const u32 size)
{
	gCrossingRecords.magic = CROSSING_RECORDS_MAGIC;
	WriteAndVerifySramFast(&gCrossingRecords, dst, sizeof(gCrossingRecords));
}

/* Saves made before this chunk existed have no magic: they load empty records */
void CrossingRecords_Load(u8 *src, const u32 size)
{
	ReadSramFast(src, &gCrossingRecords, sizeof(gCrossingRecords));

	if (gCrossingRecords.magic != CROSSING_RECORDS_MAGIC)
		CrossingRecords_Clear();
}

int CrossingRecords_Medal(int level)
{
	int b = CrossingRecords_Best(level);

	if (b >= CROSSING_MEDAL_GOLD)
		return 3;
	if (b >= CROSSING_MEDAL_SILVER)
		return 2;
	if (b >= CROSSING_MEDAL_BRONZE)
		return 1;
	return 0;
}

static void CrossingProgress_Clear(void)
{
	int i;

	CpuFill16(0, &gCrossingProgress, sizeof(gCrossingProgress));
	for (i = 0; i < CROSSING_LEVELS; i++)
		gCrossingProgress.chalStory[i] = 0xFF;
	gCrossingProgress.magic = CROSSING_PROGRESS_MAGIC;
}

static void CrossingProgress_Check(void)
{
	if (gCrossingProgress.magic != CROSSING_PROGRESS_MAGIC)
		CrossingProgress_Clear();
}

/* The fastest round 1 clear in seconds, 0: none */
int CrossingRecords_Fastest(int level)
{
	if (level < 0 || level >= CROSSING_LEVELS || gCrossingProgress.magic != CROSSING_PROGRESS_MAGIC)
		return 0;
	return gCrossingProgress.fastest[level];
}

void CrossingRecords_SetFastest(int level, int seconds)
{
	if (level < 0 || level >= CROSSING_LEVELS)
		return;
	CrossingProgress_Check();
	if (seconds < 1)
		seconds = 1;
	if (seconds > 0xFFFF)
		seconds = 0xFFFF;
	if (gCrossingProgress.fastest[level] == 0 || seconds < gCrossingProgress.fastest[level])
		gCrossingProgress.fastest[level] = seconds;
}

/* which: 0 the first clear, 1 the first round 5 */
bool CrossingRecords_Milestone(int level, int which)
{
	if (level < 0 || level >= CROSSING_LEVELS || which < 0 || which > 1)
		return false;
	if (gCrossingProgress.magic != CROSSING_PROGRESS_MAGIC)
		return false;
	return (gCrossingProgress.milestones >> (level + 4 * which)) & 1;
}

void CrossingRecords_SetMilestone(int level, int which)
{
	if (level < 0 || level >= CROSSING_LEVELS || which < 0 || which > 1)
		return;
	CrossingProgress_Check();
	gCrossingProgress.milestones |= 1 << (level + 4 * which);
}

/* chapter: the story chapter whose challenge was met */
bool CrossingRecords_ChalDone(int level, int chapter)
{
	if (level < 0 || level >= CROSSING_LEVELS)
		return false;
	if (gCrossingProgress.magic != CROSSING_PROGRESS_MAGIC)
		return false;
	return gCrossingProgress.chalStory[level] == chapter;
}

void CrossingRecords_SetChalDone(int level, int chapter)
{
	if (level < 0 || level >= CROSSING_LEVELS)
		return;
	CrossingProgress_Check();
	gCrossingProgress.chalStory[level] = chapter;
}

/* New-game hook (gNewSaveHooks) */
void CrossingProgress_NewGame(void)
{
	CrossingProgress_Clear();
}

/* Game and suspend save chunk (0x10 bytes) */
void CrossingProgress_Save(u8 *dst, const u32 size)
{
	CrossingProgress_Check();
	WriteAndVerifySramFast(&gCrossingProgress, dst, sizeof(gCrossingProgress));
}

/* Saves made before this chunk existed have no magic: they load empty progress */
void CrossingProgress_Load(u8 *src, const u32 size)
{
	ReadSramFast(src, &gCrossingProgress, sizeof(gCrossingProgress));
	CrossingProgress_Check();
}

#endif /* CONFIG_DISTRICT_CROSSING */
