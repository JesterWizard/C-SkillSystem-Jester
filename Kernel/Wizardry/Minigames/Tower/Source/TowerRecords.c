#include "common-chax.h"
#include "kernel-lib.h"
#include "tower.h"

#ifdef CONFIG_DISTRICT_TOWER

/*
 * Tower Stack record: the best score, shown on the status line. gTowerRecords lives in RAM
 * (include/link/config-memmap.s) and is saved as one 4-byte chunk in each save list (Kernel/Wizardry/SaveData/data.event).
 * TowerRecords_NewGame is a new-game hook.
 */

#define TOWER_RECORDS_MAGIC 0x5453 /* "TS" */

typedef char TowerRecords_SizeCheck[(sizeof(struct TowerRecords) == 4) ? 1 : -1];
typedef char TowerState_SizeCheck[(sizeof(struct TowerState) == 0x100) ? 1 : -1];

int TowerRecords_Best(void)
{
	return gTowerRecords.magic == TOWER_RECORDS_MAGIC ? gTowerRecords.best : 0;
}

void TowerRecords_Add(u32 score)
{
	if (gTowerRecords.magic != TOWER_RECORDS_MAGIC) {
		gTowerRecords.magic = TOWER_RECORDS_MAGIC;
		gTowerRecords.best = 0;
	}
	if (score > 0xFFFF)
		score = 0xFFFF;
	if (score > gTowerRecords.best)
		gTowerRecords.best = score;
}

/* New-game hook (gNewSaveHooks) */
void TowerRecords_NewGame(void)
{
	gTowerRecords.magic = TOWER_RECORDS_MAGIC;
	gTowerRecords.best = 0;
}

/* Game and suspend save chunk (4 bytes) */
void TowerRecords_Save(u8 *dst, const u32 size)
{
	gTowerRecords.magic = TOWER_RECORDS_MAGIC;
	WriteAndVerifySramFast(&gTowerRecords, dst, sizeof(gTowerRecords));
}

/* Saves made before this chunk existed have no magic: they load no record */
void TowerRecords_Load(u8 *src, const u32 size)
{
	ReadSramFast(src, &gTowerRecords, sizeof(gTowerRecords));

	if (gTowerRecords.magic != TOWER_RECORDS_MAGIC)
		TowerRecords_NewGame();
}

#endif /* CONFIG_DISTRICT_TOWER */
