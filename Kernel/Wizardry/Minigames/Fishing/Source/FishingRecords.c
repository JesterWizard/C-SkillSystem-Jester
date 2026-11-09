#include "common-chax.h"
#include "kernel-lib.h"
#include "fishing.h"

#ifdef CONFIG_DISTRICT_FISHING

/*
 * Fishing records: how many of each fish were landed and the best payout for each, shown in the prize table (START on
 * the fishing screen). gFishingRecords lives in RAM (include/link/config-memmap.s) and is saved as one 0x20-byte chunk
 * in each save list (Kernel/Wizardry/SaveData/data.event). FishingRecords_NewGame is a new-game hook.
 */

#define FISHING_RECORDS_MAGIC 0x4653 /* "FS" */
#define FISHING_CAUGHT_MAX 9999

typedef char FishingRecords_SizeCheck[(sizeof(struct FishingRecords) == 0x20) ? 1 : -1];
typedef char FishingState_SizeCheck[(sizeof(struct FishingState) == 0x1A0) ? 1 : -1];

static void FishingRecords_Clear(void)
{
	CpuFill16(0, &gFishingRecords, sizeof(gFishingRecords));
	gFishingRecords.magic = FISHING_RECORDS_MAGIC;
}

void FishingRecords_Add(int type, u32 payout)
{
	if (type < 0 || type >= FISH_TYPES)
		return;
	if (gFishingRecords.magic != FISHING_RECORDS_MAGIC)
		FishingRecords_Clear();
	if (gFishingRecords.caught[type] < FISHING_CAUGHT_MAX)
		gFishingRecords.caught[type]++;
	if (payout > gFishingRecords.best[type])
		gFishingRecords.best[type] = payout;
}

/* New-game hook (gNewSaveHooks) */
void FishingRecords_NewGame(void)
{
	FishingRecords_Clear();
}

/* Game and suspend save chunk (0x20 bytes) */
void FishingRecords_Save(u8 *dst, const u32 size)
{
	gFishingRecords.magic = FISHING_RECORDS_MAGIC;
	WriteAndVerifySramFast(&gFishingRecords, dst, sizeof(gFishingRecords));
}

/* Saves made before this chunk existed have no magic: they load empty records */
void FishingRecords_Load(u8 *src, const u32 size)
{
	ReadSramFast(src, &gFishingRecords, sizeof(gFishingRecords));

	if (gFishingRecords.magic != FISHING_RECORDS_MAGIC)
		FishingRecords_Clear();
}

#endif /* CONFIG_DISTRICT_FISHING */
