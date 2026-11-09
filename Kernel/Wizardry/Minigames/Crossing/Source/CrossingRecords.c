#include "common-chax.h"
#include "kernel-lib.h"
#include "crossing.h"

#ifdef CONFIG_DISTRICT_CROSSING

/*
 * River Crossing records: the best score of each district's course, shown on the status line and the pause screen.
 * gCrossingRecords lives in RAM (include/link/config-memmap.s) and is saved as one 0x0C-byte chunk in each save list
 * (Kernel/Wizardry/SaveData/data.event). CrossingRecords_NewGame is a new-game hook.
 */

#define CROSSING_RECORDS_MAGIC 0x5243 /* "CR" */

typedef char CrossingRecords_SizeCheck[(sizeof(struct CrossingRecords) == 0x0C) ? 1 : -1];
typedef char CrossingState_SizeCheck[(sizeof(struct CrossingState) == 0x100) ? 1 : -1];

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

#endif /* CONFIG_DISTRICT_CROSSING */
