#include "common-chax.h"
#include "item-sys.h"
#include "battle-system.h"
#include "debuff.h"
#include "weapon-range.h"
#include "constants/items.h"
#include "constants/texts.h"
#include "jester_headers/custom-functions.h"
#include "mapanim.h"
#include "kernel/map-anims.h"
#include "proc.h"
#include "vanilla.h"
#include "bm.h"

static bool Kirin_CanAffectUnit(struct Unit *unit)
{
	if (!UNIT_IS_VALID(unit))
		return false;

	if (!AreUnitsAllied(gSubjectUnit->index, unit->index))
		return false;

	if (unit->state & (US_RESCUED | US_HIDDEN | US_DEAD | US_BIT16))
		return false;

	return true;
}

static int Kirin_GetFirstTargetInArea(int x, int y)
{
	for (int iy = y - 1; iy <= y + 1; iy++) {
		for (int ix = x - 1; ix <= x + 1; ix++) {
			struct Unit *unit;

			if (ix < 0 || iy < 0 || ix >= gBmMapSize.x || iy >= gBmMapSize.y)
				continue;

			unit = GetUnitAtPosition(ix, iy);
			if (Kirin_CanAffectUnit(unit))
				return unit->index;
		}
	}

	return 0;
}

static void TryAddKirinTarget(int x, int y)
{
	int target = Kirin_GetFirstTargetInArea(x, y);

	if (target != 0)
		AddTarget(x, y, target, 0);
}

static void MakeTargetListForKirin(struct Unit *unit, int item)
{
	gSubjectUnit = unit;

	InitTargets(unit->xPos, unit->yPos);
	BmMapFill(gBmMapRange, 0);
	AddMapForItem(unit, item);
	ForEachPosInRange(TryAddKirinTarget);
}

static u8 Kirin_OnSelectTarget(ProcPtr proc, struct SelectTarget *target)
{
	gActionData.subjectIndex = gActiveUnit->index;
	gActionData.targetIndex = target->uid;
	gActionData.xOther = target->x;
	gActionData.yOther = target->y;
	gActionData.unk08 = ITEM_STAFF_KIRIN;

	HideMoveRangeGraphics();
	BG_Fill(gBG2TilemapBuffer, 0);
	BG_EnableSyncByMask(BG2_SYNC_BIT);

	return TARGETSELECTION_ACTION_ENDFAST | TARGETSELECTION_ACTION_END |
		TARGETSELECTION_ACTION_SE_6A | TARGETSELECTION_ACTION_CLEARBGS;
}

static void Kirin_Anim(ProcPtr proc)
{
	StartLightRuneAnim(proc, gActionData.xOther, gActionData.yOther);
}

static bool Kirin_IsAnimRunning(ProcPtr proc)
{
	return Proc_Exists(ProcScr_LightRuneAnim);
}

static void Kirin_Exec(ProcPtr proc)
{
	struct Unit *unit = GetUnit(gActionData.subjectIndex);
	struct Unit *target = GetUnit(gActionData.targetIndex);

	if (!UNIT_IS_VALID(unit))
		return;

	if (!Kirin_CanAffectUnit(target))
		target = GetUnit(Kirin_GetFirstTargetInArea(gActionData.xOther, gActionData.yOther));

	if (!UNIT_IS_VALID(target))
		return;

	BattleInitItemEffect(unit, gActionData.itemSlotIndex);
	BattleInitItemEffectTarget(target);
	BattleApplyItemEffect(proc);

	for (int iy = gActionData.yOther - 1; iy <= gActionData.yOther + 1; iy++) {
		for (int ix = gActionData.xOther - 1; ix <= gActionData.xOther + 1; ix++) {
			struct Unit *areaUnit;

			if (ix < 0 || iy < 0 || ix >= gBmMapSize.x || iy >= gBmMapSize.y)
				continue;

			areaUnit = GetUnitAtPosition(ix, iy);
			if (!Kirin_CanAffectUnit(areaUnit))
				continue;

			SetUnitStatus(areaUnit, NEW_UNIT_STATUS_RENEWAL);
		}
	}

	RefreshEntityBmMaps();
	RenderBmMap();
	RefreshUnitSprites();
}

static void Kirin_ShowExpBar(ProcPtr proc)
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

static bool Kirin_ExpBarRunning(ProcPtr proc)
{
	return Proc_Exists(ProcScr_MapAnimExpBar);
}

static const struct ProcCmd ProcScr_Kirin[] = {
	PROC_CALL(Kirin_Anim),
	PROC_WHILE(Kirin_IsAnimRunning),
	PROC_CALL(Kirin_Exec),
	PROC_CALL(Kirin_ShowExpBar),
	PROC_WHILE(Kirin_ExpBarRunning),
	PROC_END,
};

bool IER_Usability_Kirin(struct Unit *unit, int item)
{
	if (unit->state & US_CANTOING)
		return false;

	MakeTargetListForKirin(unit, item);
	return GetSelectTargetCount() > 0;
}

void IER_Effect_Kirin(struct Unit *unit, int item)
{
	gActionData.unk08 = ITEM_STAFF_KIRIN;
	gActionData.subjectIndex = unit->index;
	SetStaffUseAction(unit);

	MakeTargetListForKirin(unit, item);
	StartSubtitleHelp(
		NewTargetSelection_Specialized(&gSelectInfo_PutTrap, Kirin_OnSelectTarget),
		GetStringFromIndex(MSG_ITEM_KIRIN_STAFF_USEDESC));
}

void IER_Action_Kirin(ProcPtr proc, struct Unit *unit, int item)
{
	Proc_StartBlocking(ProcScr_Kirin, proc);
}