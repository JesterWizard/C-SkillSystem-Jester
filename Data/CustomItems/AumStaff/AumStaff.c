#include "common-chax.h"
#include "item-sys.h"
#include "battle-system.h"
#include "constants/items.h"
#include "constants/texts.h"
#include "jester_headers/custom-arrays.h"
#include "jester_headers/custom-functions.h"
#include "bmfx.h"
#include "mapanim.h"
#include "popup.h"
#include "bmitemuse.h"
#include "bmtarget.h"
#include "bm.h"
#include "proc.h"

static const int sAdjTileOffsets[4][2] = {
	{-1, 0},
	{ 1, 0},
	{ 0,-1},
	{ 0, 1},
};

extern u8 sAumDeadUnit;

static void AumTryAddTarget(int x, int y)
{
	struct Unit *deadUnit = GetUnit(sAumDeadUnit);

	if (!UNIT_IS_VALID(deadUnit))
		return;

	if (!Generic_CanUnitBeOnPos(deadUnit, x, y, -1, -1))
		return;

	AddTarget(x, y, sAumDeadUnit, 0);
}

static void MakeTargetListForAum(struct Unit *unit)
{
	gSubjectUnit = unit;
	InitTargets(unit->xPos, unit->yPos);
	BmMapFill(gBmMapRange, 0);

	sAumDeadUnit = GetLastDeadUnit();
	if (sAumDeadUnit == 0)
		return;

	if (!UNIT_IS_VALID(GetUnit(sAumDeadUnit)))
		return;

	for (int i = 0; i < 4; i++)
	{
		int x = unit->xPos + sAdjTileOffsets[i][0];
		int y = unit->yPos + sAdjTileOffsets[i][1];

		if (x < 0 || y < 0 || x >= gBmMapSize.x || y >= gBmMapSize.y)
			continue;

		AumTryAddTarget(x, y);
	}
}

static u8 Aum_OnSelectTarget(ProcPtr proc, struct SelectTarget *target)
{
	gActionData.subjectIndex = gActiveUnit->index;
	gActionData.targetIndex = target->uid;
	gActionData.xOther = target->x;
	gActionData.yOther = target->y;
	gActionData.unk08 = ITEM_STAFF_AUM;

	return TARGETSELECTION_ACTION_ENDFAST | TARGETSELECTION_ACTION_END | TARGETSELECTION_ACTION_SE_6A |
		TARGETSELECTION_ACTION_CLEARBGS;
}

static void Aum_Anim(ProcPtr proc)
{
	StartLightRuneAnim(proc, gActionData.xOther, gActionData.yOther);
}

static bool Aum_IsAnimRunning(ProcPtr proc)
{
	return Proc_Exists(ProcScr_LightRuneAnim);
}

extern struct PopupInstruction const AumRevivedPopup[];

static void Aum_Exec(ProcPtr proc)
{
	struct Unit *unit = GetUnit(gActionData.subjectIndex);
	struct Unit *target = GetUnit(gActionData.targetIndex);

	if (!UNIT_IS_VALID(unit) || !UNIT_IS_VALID(target))
		return;

	BattleInitItemEffect(unit, gActionData.itemSlotIndex);
	BattleInitItemEffectTarget(target);

	target->state &= ~(US_HIDDEN | US_UNSELECTABLE | US_DEAD);
	target->xPos = gActionData.xOther;
	target->yPos = gActionData.yOther;
	target->curHP = 1;
	target->rescue = 0;

	RemoveDeadUnit(target->index);

	RefreshEntityBmMaps();
	RenderBmMap();
	RefreshUnitSprites();

	BattleApplyItemEffect(proc);
}

static void Aum_ShowPopup(ProcPtr proc)
{
	struct Unit *target = GetUnit(gActionData.targetIndex);

	if (UNIT_IS_VALID(target)) {
		SetPopupUnit(target);
		NewPopup_Simple(AumRevivedPopup, 0x5A, 0, proc);
	}
}

static bool Aum_PopupRunning(ProcPtr proc)
{
	return Proc_Exists(ProcScr_Popup);
}

struct PopupInstruction const AumRevivedPopup[] = {
	POPUP_SOUND(0x5A),
	POPUP_COLOR(TEXT_COLOR_SYSTEM_BLUE),
	POPUP_UNIT_NAME,
	POPUP_SPACE(2),
	POPUP_COLOR(TEXT_COLOR_SYSTEM_WHITE),
	POPUP_MSG(MSG_AumUnitRevived),
	POPUP_END,
};

STATIC_DECLAR const struct ProcCmd ProcScr_AumRevive[] = {
	PROC_CALL(Aum_Anim),
	PROC_WHILE(Aum_IsAnimRunning),
	PROC_CALL(Aum_Exec),
	PROC_CALL(Aum_ShowPopup),
	PROC_WHILE(Aum_PopupRunning),
	PROC_END,
};

bool IER_Usability_Aum(struct Unit *unit, int item)
{
	if (unit->state & US_CANTOING)
		return false;

	return HasSelectTarget(unit, MakeTargetListForAum);
}

void IER_Effect_Aum(struct Unit *unit, int item)
{
	gActionData.unk08 = ITEM_STAFF_AUM;
	gActionData.subjectIndex = unit->index;
	SetStaffUseAction(unit);

	MakeTargetListForAum(unit);
	StartSubtitleHelp(
		NewTargetSelection_Specialized(&gSelectInfo_PutTrap, Aum_OnSelectTarget),
		GetStringFromIndex(MSG_ITEM_AUM_STAFF_SUBTITLE));
}

void IER_Action_Aum(ProcPtr proc, struct Unit *unit, int item)
{
	Proc_StartBlocking(ProcScr_AumRevive, proc);
}