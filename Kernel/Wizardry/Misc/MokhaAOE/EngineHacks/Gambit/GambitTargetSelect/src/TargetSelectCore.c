#include "common-chax.h"
#include "weapon-range.h"
#include "mokha-aoe.h"
#include "constants/texts.h"

STATIC_DECLAR void ForEachUnitInAoeMovementMap(void (*func)(struct Unit *unit))
{
	int x, y;

	if (gBmMapSize.x <= 0 || gBmMapSize.y <= 0)
		return;

	for (y = 0; y < gBmMapSize.y; y++) {
		for (x = 0; x < gBmMapSize.x; x++) {
			if ((s8)gBmMapMovement[y][x] < 0)
				continue;

			if (gBmMapUnit[y][x] == 0)
				continue;

			func(GetUnit(gBmMapUnit[y][x]));
		}
	}
}

void MakeTargetListFor_SubGambitMenu(struct Unit *unit, u8 range)
{
	InitTargets(unit->xPos, unit->yPos);
	BmMapFill(gBmMapRange, 0);
	MapAddInBoundedRange(unit->xPos, unit->yPos, 1, range);
	ForEachUnitInRange(AddUnitToTargetListIfNotAllied);
}

void MakeTargetListFor_AfterSelectAPressed(u8 x, u8 y, u8 gambitIndex)
{
	InitTargets(x, y);
	BmMapFill(gBmMapMovement, -1);
	gWorkingBmMap = gBmMapMovement;
	FillAOEEffectMap_OnChangeTarget(x, y, gambitIndex);
	ForEachUnitInAoeMovementMap(AddUnitToTargetListIfNotAllied);
}

STATIC_DECLAR u8 GambitTarget_OnSwitchIn(ProcPtr proc, struct SelectTarget *target)
{
	u8 attackIndex = sGambitSelectedAttack;
	const struct MokhaAoeAttackInfo *info = GetMokhaAoeAttackInfo(attackIndex);
	u8 range = info ? info->range : 2;

	(void)proc;

	HideMoveRangeGraphics();
	BmMapFill(gBmMapMovement, -1);
	BmMapFill(gBmMapRange, 0);
	BmMapFill(gBmMapOther, 0);
	gWorkingBmMap = gBmMapMovement;
	FillAOEEffectMap_OnChangeTarget(target->x, target->y, attackIndex);

	gWorkingBmMap = gBmMapRange;
	FillRangeMapForHover(gActiveUnit, range);
	DisplayMoveRangeGraphics(MOVLIMITV_MMAP_RED | MOVLIMITV_RMAP_GREEN);
	return 0;
}

STATIC_DECLAR u8 GambitTarget_OnSelect(ProcPtr proc, struct SelectTarget *target)
{
	(void)proc;

	gActionData.unitActionType = CONFIG_UNIT_ACTION_EXPA_Gambit;
	gActionData.targetIndex = target->uid;
	gActionData.unk08 = sGambitSelectedAttack;

	Proc_EndEach(gProcScr_BKSEL);
	MakeTargetListFor_AfterSelectAPressed(target->x, target->y, sGambitSelectedAttack);
	SaveTarget_PostGambitTargetSelection();

	GambitResetMaps();

	return TARGETSELECTION_ACTION_ENDFAST | TARGETSELECTION_ACTION_END
		| TARGETSELECTION_ACTION_SE_6A | TARGETSELECTION_ACTION_CLEARBGS;
}

STATIC_DECLAR u8 GambitTarget_OnCancel(ProcPtr proc, struct SelectTarget *target)
{
	(void)target;

	ClearTarget_CommonFlagSaveSu();
	GambitResetMaps();
	return GenericSelection_BackToUM(proc, target);
}

STATIC_DECLAR void GambitTarget_OnEnd(ProcPtr proc)
{
	(void)proc;
	GambitResetMaps();
}

STATIC_DECLAR u8 GambitTarget_OnHelp(ProcPtr proc, struct SelectTarget *target)
{
	(void)proc;
	(void)target;
	return 0;
}

const struct SelectInfo gSelectInfo_Gambit = {
	.onInit = NULL,
	.onEnd = GambitTarget_OnEnd,
	.onUnk08 = NULL,
	.onSwitchIn = GambitTarget_OnSwitchIn,
	.onSwitchOut = NULL,
	.onSelect = GambitTarget_OnSelect,
	.onCancel = GambitTarget_OnCancel,
	.onHelp = GambitTarget_OnHelp,
};
