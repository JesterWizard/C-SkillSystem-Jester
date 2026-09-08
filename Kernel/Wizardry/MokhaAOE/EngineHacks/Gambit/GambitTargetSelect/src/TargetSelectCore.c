#include "common-chax.h"
#include "weapon-range.h"
#include "mokha-aoe.h"
#include "bksel.h"
#include "bm.h"
#include "constants/items.h"
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

STATIC_DECLAR void Gambit_SetupBattleForecast(struct Unit *target)
{
	const struct MokhaAoeAttackInfo *info = GetMokhaAoeAttackInfo(sGambitSelectedAttack);
	int damage = info ? info->damage : 0;
	int item = GetUnitEquippedWeapon(gActiveUnit);

	if (item == 0)
		item = ITEM_SWORD_IRON;

	InitBattleUnit(&gBattleActor, gActiveUnit);
	InitBattleUnit(&gBattleTarget, target);

	/*
	 * Synthetic one-shot preview: fixed gambit damage, always hits, no
	 * crit, no counter, and no follow-up marker on the BKSEL panel.
	 */
	gBattleActor.weapon = item;
	gBattleActor.weaponBefore = item;
	gBattleActor.weaponAttributes = 0;
	gBattleActor.weaponType = GetItemType(item);
	gBattleActor.canCounter = true;
	gBattleActor.battleAttack = damage;
	gBattleActor.battleDefense = 0;
	gBattleActor.battleEffectiveHitRate = 100;
	gBattleActor.battleEffectiveCritRate = 0;
	gBattleActor.battleSpeed = 0;

	gBattleTarget.weapon = 0;
	gBattleTarget.weaponBefore = 0;
	gBattleTarget.weaponAttributes = 0;
	gBattleTarget.weaponBroke = false;
	gBattleTarget.canCounter = false;
	gBattleTarget.battleAttack = 0;
	gBattleTarget.battleDefense = 0;
	gBattleTarget.battleEffectiveHitRate = 0xFF;
	gBattleTarget.battleEffectiveCritRate = 0xFF;
	/* CheckCanTwiceAttackOrder early-outs when defender speed > 250. */
	gBattleTarget.battleSpeed = 251;

	gBattleStats.range = 1;
}

STATIC_DECLAR void GambitTarget_OnInit(ProcPtr proc)
{
	(void)proc;
	NewBattleForecast(proc);
}

STATIC_DECLAR u8 GambitTarget_OnSwitchIn(ProcPtr proc, struct SelectTarget *target)
{
	u8 attackIndex = sGambitSelectedAttack;
	const struct MokhaAoeAttackInfo *info = GetMokhaAoeAttackInfo(attackIndex);
	u8 range = info ? info->range : 2;
	struct Unit *unit = GetUnit(target->uid);

	(void)proc;

	ChangeActiveUnitFacing(target->x, target->y);

	HideMoveRangeGraphics();
	BmMapFill(gBmMapMovement, -1);
	BmMapFill(gBmMapRange, 0);
	BmMapFill(gBmMapOther, 0);
	gWorkingBmMap = gBmMapMovement;
	FillAOEEffectMap_OnChangeTarget(target->x, target->y, attackIndex);

	gWorkingBmMap = gBmMapRange;
	FillRangeMapForHover(gActiveUnit, range);
	DisplayMoveRangeGraphics(MOVLIMITV_MMAP_RED | MOVLIMITV_RMAP_GREEN);

	if (UNIT_IS_VALID(unit)) {
		Gambit_SetupBattleForecast(unit);
		UpdateBattleForecastContents();
	}

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

STATIC_DECLAR void RebuildGambitSelectMenuAfterForecast(void)
{
	RebuildGambitSelectMenu();
}

STATIC_DECLAR const struct ProcCmd ProcScr_PostGambitSelectTarget[] = {
	PROC_NAME("PostGambitSelectTarget"),
	PROC_CALL(LockGame),
	PROC_CALL(sub_8022E38),
	PROC_WHILE_EXISTS(gProcScr_BKSEL),
	PROC_WHILE_EXISTS(ProcScr_CamMove),
	PROC_CALL(RebuildGambitSelectMenuAfterForecast),
	PROC_CALL(UnlockGame),
	PROC_END,
};

STATIC_DECLAR u8 GambitTarget_OnCancel(ProcPtr proc, struct SelectTarget *target)
{
	(void)proc;
	(void)target;

	if (EventEngineExists())
		return 0;

	ClearTarget_CommonFlagSaveSu();
	GambitResetMaps();
	Proc_Start(ProcScr_PostGambitSelectTarget, PROC_TREE_3);

	return TARGETSELECTION_ACTION_ENDFAST | TARGETSELECTION_ACTION_END
		| TARGETSELECTION_ACTION_SE_6B;
}

STATIC_DECLAR void GambitTarget_OnEnd(ProcPtr proc)
{
	(void)proc;

	BG_Fill(gBG2TilemapBuffer, 0);
	BG_EnableSyncByMask(BG2_SYNC_BIT);
	HideMoveRangeGraphics();
	CloseBattleForecast();
	GambitResetMaps();
}

STATIC_DECLAR u8 GambitTarget_OnHelp(ProcPtr proc, struct SelectTarget *target)
{
	return StartBattleForecastHelpBox(proc, target);
}

const struct SelectInfo gSelectInfo_Gambit = {
	.onInit = GambitTarget_OnInit,
	.onEnd = GambitTarget_OnEnd,
	.onUnk08 = NULL,
	.onSwitchIn = GambitTarget_OnSwitchIn,
	.onSwitchOut = NULL,
	.onSelect = GambitTarget_OnSelect,
	.onCancel = GambitTarget_OnCancel,
	.onHelp = GambitTarget_OnHelp,
};
