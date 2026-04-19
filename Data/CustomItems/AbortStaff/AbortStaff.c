#include "common-chax.h"
#include "item-sys.h"
#include "battle-system.h"
#include "constants/items.h"
#include "constants/texts.h"
#include "bmarch.h"
#include "bmitemuse.h"
#include "bmtarget.h"
#include "bmudisp.h"
#include "jester_headers/custom-functions.h"
#include "mapanim.h"
#include "menu_def.h"
#include "proc.h"
#include "vanilla.h"

#define ABORT_STAFF_EXP 30

static void AbortStaff_Exec(ProcPtr proc)
{
	struct Unit *unit = GetUnit(gActionData.subjectIndex);
	struct Unit *target = GetUnit(gActionData.targetIndex);

	if (!UNIT_IS_VALID(unit) || !UNIT_IS_VALID(target))
		return;

	BattleInitItemEffect(unit, gActionData.itemSlotIndex);
	BattleInitItemEffectTarget(target);
	BattleApplyItemEffect(proc);

	TryRemoveUnitFromBallista(target);
	HideUnitSprite(target);
	target->state |= US_HIDDEN;

	RefreshEntityBmMaps();
	RenderBmMap();
	RefreshUnitSprites();
}

static void AbortStaff_Anim(ProcPtr proc)
{
	PlaySoundEffect(0x269);
	Proc_StartBlocking(ProcScr_DanceringAnim, proc);

	BG_SetPosition(
		BG_0,
		-SCREEN_TILE_IX(gActiveUnit->xPos - 1),
		-SCREEN_TILE_IX(gActiveUnit->yPos - 2));
}

static bool AbortStaff_IsAnimRunning(ProcPtr proc)
{
	return Proc_Exists(ProcScr_DanceringAnim);
}

static void AbortStaff_ShowExpBar(ProcPtr proc)
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

static bool AbortStaff_ExpBarRunning(ProcPtr proc)
{
	return Proc_Exists(ProcScr_MapAnimExpBar);
}

static const struct ProcCmd ProcScr_AbortStaff[] = {
	PROC_CALL(AbortStaff_Anim),
	PROC_WHILE(AbortStaff_IsAnimRunning),
	PROC_CALL(AbortStaff_Exec),
	PROC_CALL(AbortStaff_ShowExpBar),
	PROC_WHILE(AbortStaff_ExpBarRunning),
	PROC_END,
};

bool IER_Usability_Abort(struct Unit *unit, int item)
{
	if (unit->state & US_CANTOING)
		return false;

	return HasSelectTarget(unit, MakeTargetListForAdjacentSameFaction);
}

void IER_Effect_Abort(struct Unit *unit, int item)
{
	gActionData.unk08 = ITEM_STAFF_ABORT;
	gActionData.subjectIndex = unit->index;
	SetStaffUseAction(unit);

	MakeTargetListForAdjacentSameFaction(unit);
	StartSubtitleHelp(
		NewTargetSelection_Specialized(&gSelectInfo_Dance, StaffSelectOnSelect),
		GetStringFromIndex(MSG_ITEM_ABORT_STAFF_USEDESC));
}

void IER_Action_Abort(ProcPtr proc, struct Unit *unit, int item)
{
	Proc_StartBlocking(ProcScr_AbortStaff, proc);
}