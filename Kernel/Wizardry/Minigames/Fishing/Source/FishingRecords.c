#include "common-chax.h"
#include "kernel-lib.h"
#include "fishing.h"

#ifdef CONFIG_DISTRICT_FISHING

/*
 * The fish journal: how many of each species were landed and the biggest of each, shown in the journal (START on the
 * fishing screen) and on the catch card. gFishingRecords lives in RAM (include/link/config-memmap.s) and is saved as
 * one 0x2C-byte chunk in each save list (Kernel/Wizardry/SaveData/data.event). FishingRecords_NewGame is a new-game
 * hook. The magic changed with the journal ("FS" was the prize table's records), so older records load empty.
 */

#define FISHING_RECORDS_MAGIC 0x4A46 /* "FJ" */
#define FISHING_CAUGHT_MAX 255

typedef char FishingRecords_SizeCheck[(sizeof(struct FishingRecords) == 0x2C) ? 1 : -1];
typedef char FishingState_SizeCheck[(sizeof(struct FishingState) == 0x1A0) ? 1 : -1];

static void FishingRecords_Clear(void)
{
	CpuFill16(0, &gFishingRecords, sizeof(gFishingRecords));
	gFishingRecords.magic = FISHING_RECORDS_MAGIC;
}

/* Records a catch of `cm` centimetres; returns FISHING_JOURNAL_* */
int FishingRecords_Add(int species, int cm)
{
	int result = FISHING_JOURNAL_SAME;

	if (species < 0 || species >= FISHING_JOURNAL_SPECIES)
		return result;
	if (gFishingRecords.magic != FISHING_RECORDS_MAGIC)
		FishingRecords_Clear();
	if (gFishingRecords.caught[species] == 0)
		result = FISHING_JOURNAL_NEW;
	else if (cm > gFishingRecords.best[species])
		result = FISHING_JOURNAL_RECORD;
	if (gFishingRecords.caught[species] < FISHING_CAUGHT_MAX)
		gFishingRecords.caught[species]++;
	if (cm > gFishingRecords.best[species])
		gFishingRecords.best[species] = cm > 255 ? 255 : cm;
	return result;
}

/* How many of a species were landed (0: not found yet), and the biggest, in centimetres */
int FishingRecords_Caught(int species)
{
	if (gFishingRecords.magic != FISHING_RECORDS_MAGIC || species < 0 || species >= FISHING_JOURNAL_SPECIES)
		return 0;
	return gFishingRecords.caught[species];
}

int FishingRecords_Best(int species)
{
	if (gFishingRecords.magic != FISHING_RECORDS_MAGIC || species < 0 || species >= FISHING_JOURNAL_SPECIES)
		return 0;
	return gFishingRecords.best[species];
}

/* How many species have been found */
int FishingRecords_Found(void)
{
	int i, n = 0;

	if (gFishingRecords.magic != FISHING_RECORDS_MAGIC)
		return 0;
	for (i = 0; i < FISHING_JOURNAL_SPECIES; i++)
		if (gFishingRecords.caught[i] > 0)
			n++;
	return n;
}

/* New-game hook (gNewSaveHooks) */
void FishingRecords_NewGame(void)
{
	FishingRecords_Clear();
}

/* Game and suspend save chunk (0x2C bytes) */
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

/* ---- The angler's progress: rank points, legends, bottles and requests ---- */

#define FISHING_PROGRESS_MAGIC 0x5046 /* "FP" */
#define FISHING_POINTS_MAX 9999

typedef char FishingProgress_SizeCheck[(sizeof(struct FishingProgress) == 0x20) ? 1 : -1];

static const u16 sRankPoints[FISHING_RANKS] = { 0, 25, 75, 175, 350 };

static void FishingProgress_Clear(void)
{
	CpuFill16(0, &gFishingProgress, sizeof(gFishingProgress));
	gFishingProgress.magic = FISHING_PROGRESS_MAGIC;
}

/* Clears the progress if it was never set up (a save from before it existed, or none loaded) */
void FishingProgress_Validate(void)
{
	if (gFishingProgress.magic != FISHING_PROGRESS_MAGIC)
		FishingProgress_Clear();
}

int FishingProgress_RankPoints(int rank)
{
	return rank < FISHING_RANKS ? sRankPoints[rank] : 0xFFFF;
}

int FishingProgress_Rank(void)
{
	int rank = FISHING_RANKS - 1;

	FishingProgress_Validate();
	while (rank > 0 && gFishingProgress.points < sRankPoints[rank])
		rank--;
	return rank;
}

void FishingProgress_AddPoints(int n)
{
	int p;

	FishingProgress_Validate();
	p = gFishingProgress.points + n;
	gFishingProgress.points = p > FISHING_POINTS_MAX ? FISHING_POINTS_MAX : p;
}

/* New-game hook (gNewSaveHooks) */
void FishingProgress_NewGame(void)
{
	FishingProgress_Clear();
}

/* Game and suspend save chunk (0x20 bytes) */
void FishingProgress_Save(u8 *dst, const u32 size)
{
	FishingProgress_Validate();
	WriteAndVerifySramFast(&gFishingProgress, dst, sizeof(gFishingProgress));
}

void FishingProgress_Load(u8 *src, const u32 size)
{
	ReadSramFast(src, &gFishingProgress, sizeof(gFishingProgress));
	FishingProgress_Validate();
}

#endif /* CONFIG_DISTRICT_FISHING */
