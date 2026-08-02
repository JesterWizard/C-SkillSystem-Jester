#include "common-chax.h"
#include "weapon-range.h"
#include "mokha-aoe.h"

void FillRangeMapForHover(struct Unit *unit, u8 range)
{
	MapAddInRange(unit->xPos, unit->yPos, range, 1);
	MapAddInRange(unit->xPos, unit->yPos, 0, -1);
}

void FillAOEEffectMap_OnChangeTarget(u8 x, u8 y, u8 gambitIndex)
{
	u8 direc;
	GambitAoeMapFunc func;

	if (gambitIndex >= MOKHA_AOE_ATK_COUNT)
		gambitIndex = 0;

	direc = GetFacingDirection(gActiveUnit->xPos, gActiveUnit->yPos, x, y);
	func = GambitEffectMap_DrawMapRoutineTable[gambitIndex];

	if (func) {
		gWorkingBmMap = gBmMapMovement;
		func(x, y, direc);
	}
}

void GambitResetMaps(void)
{
	HideMoveRangeGraphics();
	BG_Fill(gBG2TilemapBuffer, 0);
	BG_EnableSyncByMask(BG2_SYNC_BIT);
	BmMapFill(gBmMapMovement, -1);
	BmMapFill(gBmMapRange, 0);
	BmMapFill(gBmMapOther, 0);
	gWorkingBmMap = gBmMapMovement;
	RefreshEntityBmMaps();
	RefreshUnitSprites();
	RenderBmMap();
}
