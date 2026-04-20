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

static void Kirin_Anim(ProcPtr proc)
{
	PlaySoundEffect(0x269);
	Proc_StartBlocking(ProcScr_DanceringAnim, proc);

	BG_SetPosition(
		BG_0,
		-SCREEN_TILE_IX(gActiveUnit->xPos - 1),
		-SCREEN_TILE_IX(gActiveUnit->yPos - 2));
}

static bool Kirin_IsAnimRunning(ProcPtr proc)
{
	return Proc_Exists(ProcScr_DanceringAnim);
}

static void Kirin_Exec(ProcPtr proc)
{
	struct Unit *unit = GetUnit(gActionData.subjectIndex);
	int oldHp;

	if (!UNIT_IS_VALID(unit))
		return;

	oldHp = GetUnitCurrentHp(unit);

	BattleInitItemEffect(unit, gActionData.itemSlotIndex);
	BattleInitItemEffectTarget(unit);
	BattleApplyItemEffect(proc);
	SetUnitHp(unit, oldHp);
	gBattleHitIterator->hpChange = 0;
	gBattleTarget.unit.curHP = oldHp;

	for (int iy = unit->yPos - 1; iy <= unit->yPos + 1; iy++) {
		for (int ix = unit->xPos - 1; ix <= unit->xPos + 1; ix++) {
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

	gSubjectUnit = unit;

	for (int iy = unit->yPos - 1; iy <= unit->yPos + 1; iy++) {
		for (int ix = unit->xPos - 1; ix <= unit->xPos + 1; ix++) {
			if (ix < 0 || iy < 0 || ix >= gBmMapSize.x || iy >= gBmMapSize.y)
				continue;

			if (Kirin_CanAffectUnit(GetUnitAtPosition(ix, iy)))
				return true;
		}
	}

	return false;
}

void IER_Effect_Kirin(struct Unit *unit, int item)
{
	gActionData.unk08 = ITEM_STAFF_KIRIN;
	gActionData.subjectIndex = unit->index;
	gActionData.targetIndex = unit->index;
	SetStaffUseAction(unit);
}

void IER_Action_Kirin(ProcPtr proc, struct Unit *unit, int item)
{
	Proc_StartBlocking(ProcScr_Kirin, proc);
}