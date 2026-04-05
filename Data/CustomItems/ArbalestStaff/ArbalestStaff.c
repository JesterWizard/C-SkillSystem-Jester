#include "common-chax.h"
#include "item-sys.h"
#include "battle-system.h"
#include "skill-system.h"
#include "constants/items.h"
#include "constants/texts.h"
#include "jester_headers/custom-functions.h"

static void ArbalestStaff_Anim(ProcPtr proc)
{
	StartLightRuneAnim(proc, gActionData.xOther, gActionData.yOther);
}

static bool ArbalestStaff_IsAnimRunning(ProcPtr proc)
{
	return Proc_Exists(ProcScr_LightRuneAnim);
}

static void ArbalestStaff_Exec(ProcPtr proc)
{
	struct Unit *unit = GetUnit(gActionData.subjectIndex);
	BattleInitItemEffect(unit, gActionData.itemSlotIndex);
	AddBallista(gActionData.xOther, gActionData.yOther, ITEM_BALLISTA_REGULAR);

	RefreshEntityBmMaps();
	RenderBmMap();
	RefreshUnitSprites();
	BattleApplyItemEffect(proc);
}

static bool ArbalestStaff_ExecRunning(ProcPtr proc)
{
	return Proc_Exists(sProcScr_BattleAnimSimpleLock);
}

static void ArbalestStaff_ShowExpBar(ProcPtr proc)
{
    struct Unit *unit = GetUnit(gActionData.subjectIndex);
	int expGain = gBattleActor.expGain;

    if (!UNIT_IS_VALID(unit))
		return;

	if (expGain <= 0)
		return;

	gManimSt.actorCount = 1;
	gManimSt.hp_changing = 0;
	gManimSt.subjectActorId = 0;
	gManimSt.targetActorId = 0;
	SetupMapBattleAnim(&gBattleActor, &gBattleTarget, gBattleHitArray);

    struct MAExpBarProc *barProc = Proc_StartBlocking(ProcScr_MapAnimExpBar, proc);

    barProc->expFrom = gBattleActor.expPrevious;
    barProc->expTo   = gBattleActor.expPrevious + expGain;
    barProc->actorId = 0;
}

static bool ArbalestStaff_ExpBarRunning(ProcPtr proc)
{
	return Proc_Exists(ProcScr_MapAnimExpBar);
}

STATIC_DECLAR const struct ProcCmd ProcScr_ArbalestStaff[] = {
	PROC_CALL(ArbalestStaff_Anim),
	PROC_WHILE(ArbalestStaff_IsAnimRunning),
	PROC_CALL(ArbalestStaff_Exec),
	PROC_WHILE(ArbalestStaff_ExecRunning),
	PROC_CALL(ArbalestStaff_ShowExpBar),
	PROC_WHILE(ArbalestStaff_ExpBarRunning),
	PROC_END,
};

static u8 Arbalest_OnSelectTarget(ProcPtr proc, struct SelectTarget * target)
{
    gActionData.targetIndex = target->uid;

    gActionData.xOther = target->x;
    gActionData.yOther = target->y;

    HideMoveRangeGraphics();

    BG_Fill(gBG2TilemapBuffer, 0);
    BG_EnableSyncByMask(BG2_SYNC_BIT);

    return TARGETSELECTION_ACTION_ENDFAST | TARGETSELECTION_ACTION_END | TARGETSELECTION_ACTION_SE_6A | TARGETSELECTION_ACTION_CLEARBGS;
}

bool IER_Usability_Arbalest(struct Unit *unit, int item)
{
	if (unit->state & US_CANTOING)
		return false;

	return HasSelectTarget(unit, MakeTargetListForMine);
}

void IER_Effect_Arbalest(struct Unit *unit, int item)
{
	gActionData.unk08 = ITEM_STAFF_ARBALEST;
	gActionData.subjectIndex = unit->index;
	SetStaffUseAction(unit);

	MakeTargetListForMine(unit);
	StartSubtitleHelp(NewTargetSelection_Specialized(&gSelectInfo_PutTrap, Arbalest_OnSelectTarget), GetStringFromIndex(MSG_ITEM_ARBALEST_STAFF_USEDESC));
}

void IER_Action_Arbalest(ProcPtr proc, struct Unit *unit, int item)
{
	Proc_StartBlocking(ProcScr_ArbalestStaff, proc);
}