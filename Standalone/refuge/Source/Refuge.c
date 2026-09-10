#include "gbafe.h"

#define REFUGE_REFRAIN_FLAG 0x52

static const char sRefugeTargetHelp[] = "Select a unit.";

static s8 CanUnitRefuge(struct Unit *actor, struct Unit *target)
{
	int actorCon = UNIT_CON(actor);
	int targetAid = GetUnitAid(target);

	return (actorCon <= targetAid) ? TRUE : FALSE;
}

static void TryAddUnitToRefugeTargetList(struct Unit *unit)
{
	if (!IsSameAllegiance(gSubjectUnit->index, unit->index))
		return;

	if (gSubjectUnit->pClassData->number == CLASS_PHANTOM || unit->pClassData->number == CLASS_PHANTOM)
		return;

	if (unit->statusIndex == UNIT_STATUS_BERSERK)
		return;

	if (unit->state & (US_RESCUING | US_RESCUED))
		return;

	if (!CanUnitRefuge(gSubjectUnit, unit))
		return;

	AddTarget(unit->xPos, unit->yPos, unit->index, 0);
}

static void MakeRefugeTargetList(struct Unit *unit)
{
	static const s8 adj[4][2] = { { 1, 0 }, { -1, 0 }, { 0, 1 }, { 0, -1 } };
	int i;

	gSubjectUnit = unit;
	InitTargets(unit->xPos, unit->yPos);

	for (i = 0; i < 4; i++) {
		int x = unit->xPos + adj[i][0];
		int y = unit->yPos + adj[i][1];
		u8 uid;
		struct Unit *other;

		if (x < 0 || y < 0 || x >= gBmMapSize.x || y >= gBmMapSize.y)
			continue;

		uid = gBmMapUnit[y][x];
		if (uid == 0)
			continue;

		other = GetUnit(uid);
		if (!UNIT_IS_VALID(other))
			continue;

		TryAddUnitToRefugeTargetList(other);
	}
}

u8 Refuge_Usability(const struct MenuItemDef *def, int number)
{
	(void)def;
	(void)number;

	if (gActiveUnit->state & US_HAS_MOVED)
		return MENU_NOTSHOWN;

	if (gActiveUnit->state & (US_IN_BALLISTA | US_RESCUING))
		return MENU_NOTSHOWN;

	MakeRefugeTargetList(gActiveUnit);

	if (GetSelectTargetCount() == 0)
		return MENU_NOTSHOWN;

	return MENU_ENABLED;
}

static bool Action_Refuge(ProcPtr parent)
{
	struct Unit *actor = gActiveUnit;
	struct Unit *target = GetUnit(gActionData.targetIndex);

	(void)parent;

	target->state |= US_RESCUING;
	actor->state |= US_RESCUED | US_HIDDEN;

	target->rescue = actor->index;
	actor->rescue = target->index;

	HideUnitSprite(actor);

	gActionData.xMove = gActionData.xOther;
	gActionData.yMove = gActionData.yOther;

	gActionData.subjectIndex = target->index;
	gActionData.targetIndex = actor->index;
	gActionData.unitActionType = UNIT_ACTION_WAIT;

	gActiveUnit = target;

	if (!(target->state & US_HAS_MOVED))
		gActionData.unk08 = REFUGE_REFRAIN_FLAG;

	return TRUE;
}

static u8 Refuge_OnSelectTarget(ProcPtr proc, struct SelectTarget *target)
{
	gActionData.targetIndex = target->uid;
	gActionData.xOther = target->x;
	gActionData.yOther = target->y;

	HideMoveRangeGraphics();

	BG_Fill(gBG2TilemapBuffer, 0);
	BG_EnableSyncByMask(BG2_SYNC_BIT);

	Action_Refuge(proc);

	return TARGETSELECTION_ACTION_ENDFAST | TARGETSELECTION_ACTION_END | TARGETSELECTION_ACTION_SE_6A |
		TARGETSELECTION_ACTION_CLEARBGS;
}

u8 Refuge_OnSelected(struct MenuProc *menu, struct MenuItemProc *item)
{
	(void)menu;

	if (item->availability == MENU_DISABLED)
		return MENU_ACT_SND6B;

	ClearBg0Bg1();

	MakeRefugeTargetList(gActiveUnit);
	BmMapFill(gBmMapMovement, -1);

	StartSubtitleHelp(
		NewTargetSelection_Specialized(&gSelectInfo_Rescue, Refuge_OnSelectTarget),
		sRefugeTargetHelp);

	PlaySoundEffect(0x6A);
	return MENU_ACT_SKIPCURSOR | MENU_ACT_END | MENU_ACT_SND6A;
}

void PlayerPhase_FinishAction_Refuge(ProcPtr proc)
{
	if (gPlaySt.chapterVisionRange != 0) {
		RenderBmMapOnBg2();
		MoveActiveUnit(gActionData.xMove, gActionData.yMove);
		RefreshEntityBmMaps();
		RenderBmMap();
		NewBMXFADE(0);
		RefreshUnitSprites();
	} else {
		MoveActiveUnit(gActionData.xMove, gActionData.yMove);
		RefreshEntityBmMaps();
		RenderBmMap();
	}

	SetCursorMapPosition(gActiveUnit->xPos, gActiveUnit->yPos);

	gPlaySt.xCursor = gBmSt.playerCursor.x;
	gPlaySt.yCursor = gBmSt.playerCursor.y;

	if (gActionData.unk08 == REFUGE_REFRAIN_FLAG) {
		gActionData.unk08 = 0;
		gActiveUnit->state &= ~(US_UNSELECTABLE | US_CANTOING);
	} else if (TryMakeCantoUnit(proc)) {
		HideUnitSprite(gActiveUnit);
		return;
	}

	if (ShouldCallEndEvent()) {
		EndAllMus();
		RefreshEntityBmMaps();
		RenderBmMap();
		RefreshUnitSprites();
		MaybeCallEndEvent_();
		Proc_Goto(proc, 8);
		return;
	}

	EndAllMus();
}
