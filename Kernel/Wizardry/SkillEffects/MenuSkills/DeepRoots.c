#include "common-chax.h"
#include "kernel-lib.h"
#include "map-anims.h"
#include "skill-system.h"
#include "status-getter.h"
#include "event-rework.h"
#include "action-expa.h"
#include "big-unit.h"
#include "constants/skills.h"
#include "constants/texts.h"
#include "constants/terrains.h"

/*
 * Deep Roots: a unit standing in a forest teleports to another empty forest tile up to Mov tiles away (counted
 * straight, ignoring terrain and units). The hop is free: the unit is left unmoved, so it can still walk its full Mov,
 * act, or hop again, any number of times this turn. Hidden during canto, so it never gives a second action.
 */
#if defined(SID_DeepRoots) && (COMMON_SKILL_VALID(SID_DeepRoots))

static bool DeepRoots_IsForest(int x, int y)
{
	return gBmMapTerrain[y][x] == TERRAIN_FOREST;
}

static bool DeepRoots_CanLandAt(int x, int y)
{
	if (!DeepRoots_IsForest(x, y))
		return false;

	if (gBmMapUnit[y][x] != 0)
		return false;

	if (gBmMapHidden[y][x] & HIDDEN_BIT_UNIT)
		return false;

	if (GetTrapAt(x, y) != NULL)
		return false;

	return true;
}

void MakeTargetListForDeepRoots(struct Unit *unit)
{
	int x, y, dx, dy;
	int mov = MovGetter(unit);

	gSubjectUnit = unit;
	InitTargets(unit->xPos, unit->yPos);

	for (y = 0; y < gBmMapSize.y; y++) {
		dy = y - unit->yPos;
		if (dy < 0)
			dy = -dy;

		if (dy > mov)
			continue;

		for (x = 0; x < gBmMapSize.x; x++) {
			dx = x - unit->xPos;
			if (dx < 0)
				dx = -dx;

			if (dx + dy == 0 || dx + dy > mov)
				continue;

			if (DeepRoots_CanLandAt(x, y))
				AddTarget(x, y, 0, 0);
		}
	}
}

u8 DeepRoots_Usability(const struct MenuItemDef *def, int number)
{
	/* After an action (canto) a hop would refresh the unit and give it a second action */
	if (gActiveUnit->state & US_CANTOING)
		return MENU_NOTSHOWN;

	if (BigUnit_IsBig(gActiveUnit))
		return MENU_NOTSHOWN;

	if (!DeepRoots_IsForest(gActiveUnit->xPos, gActiveUnit->yPos))
		return MENU_NOTSHOWN;

	if (!HasSelectTarget(gActiveUnit, MakeTargetListForDeepRoots))
		return MENU_DISABLED;

	return MENU_ENABLED;
}

static u8 DeepRoots_OnSelectTarget(ProcPtr proc, struct SelectTarget *target)
{
	gActionData.xOther = target->x;
	gActionData.yOther = target->y;

	HideMoveRangeGraphics();

	BG_Fill(gBG2TilemapBuffer, 0);
	BG_EnableSyncByMask(BG2_SYNC_BIT);

	gActionData.unk08 = SID_DeepRoots;
	gActionData.unitActionType = CONFIG_UNIT_ACTION_EXPA_ExecSkill;

	return TARGETSELECTION_ACTION_ENDFAST | TARGETSELECTION_ACTION_END | TARGETSELECTION_ACTION_SE_6A |
	       TARGETSELECTION_ACTION_CLEARBGS;
}

u8 DeepRoots_OnSelected(struct MenuProc *menu, struct MenuItemProc *item)
{
	if (item->availability == MENU_DISABLED) {
		MenuFrozenHelpBox(menu, MSG_SKILL_DeepRoots_FRtext);
		return MENU_ACT_SND6B;
	}

	ClearBg0Bg1();

	MakeTargetListForDeepRoots(gActiveUnit);
	BmMapFill(gBmMapMovement, -1);

	StartSubtitleHelp(
		NewTargetSelection_Specialized(&gSelectInfo_PutTrap, DeepRoots_OnSelectTarget),
		GetStringFromIndex(MSG_SKILL_DeepRoots_Target));

	PlaySoundEffect(0x6A);
	return MENU_ACT_SKIPCURSOR | MENU_ACT_END | MENU_ACT_SND6A;
}

static void DeepRoots_Prepare(void)
{
	EndAllMus();
	RefreshUnitSprites();
	gEventSlots[EVT_SLOT_2] = gActiveUnit->index;
}

/* The unit lands on the chosen forest; PlayerPhase_FinishAction moves it there for good (gActionData.xMove) */
static void DeepRoots_Land(void)
{
	gActiveUnit->xPos = gActionData.xOther;
	gActiveUnit->yPos = gActionData.yOther;
	gActionData.xMove = gActionData.xOther;
	gActionData.yMove = gActionData.yOther;
	gEventSlots[EVT_SLOT_2] = gActiveUnit->index;
}

STATIC_DECLAR const EventScr EventScr_DeepRoots[] = {
LABEL(0)
	SVAL(EVT_SLOT_B, SID_DeepRoots)
	CALL(EventScr_MuSkillAnim)
	ASMC(DeepRoots_Prepare)
	CALL(EventScr_UidWarpOUT)
	STAL(20)
	ASMC(DeepRoots_Land)
	CALL(EventScr_UidWarpIN)
	STAL(20)

LABEL(99)
	NOFADE
	ENDA
};

bool Action_DeepRoots(ProcPtr parent)
{
	/* The hop is free: PlayerPhase_FinishAction leaves the unit selectable and unmoved */
	gActionDataExpa.refrain_action = true;

	KernelCallEvent(EventScr_DeepRoots, EV_EXEC_CUTSCENE, parent);
	return true;
}

#endif
