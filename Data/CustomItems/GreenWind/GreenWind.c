#include "common-chax.h"
#include "item-sys.h"
#include "battle-system.h"
#include "constants/items.h"
#include "constants/texts.h"
#include "jester_headers/custom-functions.h"
#include "mapanim.h"
#include "kernel/map-anims.h"
#include "proc.h"
#include "vanilla.h"
#include "bwl.h"

static void TryAddUnitToGreenWindTargetList(struct Unit *unit)
{
	if (!UNIT_IS_VALID(unit))
		return;

	if (!AreUnitsAllied(gSubjectUnit->index, unit->index))
		return;

	if (unit->state & (US_RESCUED | US_HIDDEN | US_DEAD | US_BIT16))
		return;

	if (GetUnitCurrentMP(unit) >= GetUnitMaxMP(unit))
		return;

	AddTarget(unit->xPos, unit->yPos, unit->index, 0);
}

static void MakeTargetListForGreenWind(struct Unit *unit)
{
	gSubjectUnit = unit;
	InitTargets(unit->xPos, unit->yPos);
	BmMapFill(gBmMapRange, 0);
	ForEachAdjacentUnit(unit->xPos, unit->yPos, TryAddUnitToGreenWindTargetList);
}

static u8 GreenWind_OnSelectTarget(ProcPtr proc, struct SelectTarget *target)
{
	gActionData.subjectIndex = gActiveUnit->index;
	gActionData.targetIndex = target->uid;
	gActionData.unk08 = ITEM_STAFF_GREEN_WIND;

	return TARGETSELECTION_ACTION_ENDFAST | TARGETSELECTION_ACTION_END |
		TARGETSELECTION_ACTION_SE_6A | TARGETSELECTION_ACTION_CLEARBGS;
}

static void GreenWind_Anim(ProcPtr proc)
{
	struct Unit *target = GetUnit(gActionData.targetIndex);

	if (!UNIT_IS_VALID(target))
		return;

	StartLightRuneAnim(proc, target->xPos, target->yPos);
}

static bool GreenWind_IsAnimRunning(ProcPtr proc)
{
	return Proc_Exists(ProcScr_LightRuneAnim);
}

static void GreenWind_Exec(ProcPtr proc)
{
	struct Unit *unit = GetUnit(gActionData.subjectIndex);
	struct Unit *target = GetUnit(gActionData.targetIndex);
	struct NewBwl *bwl;
	int oldHp;
	int restoredMp;

	if (!UNIT_IS_VALID(unit) || !UNIT_IS_VALID(target))
		return;

	if (!CheckHasBwl(UNIT_CHAR_ID(target)))
		return;

	bwl = GetNewBwl(UNIT_CHAR_ID(target));
	if (!bwl)
		return;

	oldHp = GetUnitCurrentHp(target);

	BattleInitItemEffect(unit, gActionData.itemSlotIndex);
	BattleInitItemEffectTarget(target);
	BattleApplyItemEffect(proc);
	SetUnitHp(target, oldHp);
	gBattleHitIterator->hpChange = 0;
	gBattleTarget.unit.curHP = oldHp;

	restoredMp = bwl->currentMP + GetUnitMaxMP(unit);
	if (restoredMp > GetUnitMaxMP(target))
		restoredMp = GetUnitMaxMP(target);

	bwl->currentMP = restoredMp;

	RefreshEntityBmMaps();
	RenderBmMap();
	RefreshUnitSprites();
}

static void GreenWind_ShowExpBar(ProcPtr proc)
{
	int expGain = gBattleActor.expGain;

	if (expGain <= 0)
		return;

	gManimSt.actorCount = 1;
	gManimSt.hp_changing = 0;
	gManimSt.subjectActorId = 0;
	gManimSt.targetActorId = 0;
	SetupMapBattleAnim(&gBattleActor, &gBattleTarget, gBattleHitArray);

	struct MAExpBarProc *barProc = Proc_StartBlocking(ProcScr_MapAnimExpBar, proc);

	barProc->expFrom = gBattleActor.expPrevious;
	barProc->expTo = gBattleActor.expPrevious + expGain;
	barProc->actorId = 0;
}

static bool GreenWind_ExpBarRunning(ProcPtr proc)
{
	return Proc_Exists(ProcScr_MapAnimExpBar);
}

static const struct ProcCmd ProcScr_GreenWind[] = {
	PROC_CALL(GreenWind_Anim),
	PROC_WHILE(GreenWind_IsAnimRunning),
	PROC_CALL(GreenWind_Exec),
	PROC_CALL(GreenWind_ShowExpBar),
	PROC_WHILE(GreenWind_ExpBarRunning),
	PROC_END,
};

bool IER_Usability_GreenWind(struct Unit *unit, int item)
{
	if (unit->state & US_CANTOING)
		return false;

	return HasSelectTarget(unit, MakeTargetListForGreenWind);
}

void IER_Effect_GreenWind(struct Unit *unit, int item)
{
	gActionData.unk08 = ITEM_STAFF_GREEN_WIND;
	gActionData.subjectIndex = unit->index;
	SetStaffUseAction(unit);

	MakeTargetListForGreenWind(unit);
	StartSubtitleHelp(
		NewTargetSelection_Specialized(&gSelectInfo_Dance, GreenWind_OnSelectTarget),
		GetStringFromIndex(MSG_ITEM_GREEN_WIND_STAFF_USEDESC));
}

void IER_Action_GreenWind(ProcPtr proc, struct Unit *unit, int item)
{
	Proc_StartBlocking(ProcScr_GreenWind, proc);
}