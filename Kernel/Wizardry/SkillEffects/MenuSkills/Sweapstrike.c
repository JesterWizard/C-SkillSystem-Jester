#include "common-chax.h"
#include "kernel-lib.h"
#include "skill-system.h"
#include "playst-expa.h"
#include "weapon-range.h"
#include "constants/skills.h"
#include "constants/texts.h"
#include "jester_headers/custom-functions.h"
#include "jester_headers/macros.h"

#if defined(SID_Sweapstrike) && (COMMON_SKILL_VALID(SID_Sweapstrike))

enum {
	SWEAPSTRIKE_DAMAGE = 10,
};

/*
 * One target per map column, all on the user's row. Target selection starts
 * on entry 0 and moves to the next entry on LEFT and the previous one on
 * RIGHT, so the list starts at the user's column and runs leftwards, wrapping
 * from column 0 to the last column.
 */
static void Sweapstrike_MakeTargetList(struct Unit *unit)
{
	int i, x = unit->xPos;

	InitTargets(unit->xPos, unit->yPos);

	for (i = 0; i < gBmMapSize.x; i++) {
		AddTarget(x, unit->yPos, 0, 0);
		x = (x == 0) ? gBmMapSize.x - 1 : x - 1;
	}
}

/* Shows the column the bolt will strike in red. */
static u8 Sweapstrike_OnSwitchIn(ProcPtr proc, struct SelectTarget *target)
{
	int y;

	BmMapFill(gBmMapRange, 0);

	for (y = 0; y < gBmMapSize.y; y++)
		gBmMapRange[y][target->x] = 1;

	DisplayMoveRangeGraphics(MOVLIMITV_RMAP_RED);
	return 0;
}

static const struct SelectInfo sSelectInfo_Sweapstrike = {
	.onEnd = SubtitleMapSelect_End,
	.onSwitchIn = Sweapstrike_OnSwitchIn,
	.onCancel = GenericSelection_BackToUM,
};

u8 Sweapstrike_Usability(const struct MenuItemDef *def, int number)
{
	if (gActiveUnit->state & US_CANTOING)
		return MENU_NOTSHOWN;

	if (PlayStExpa_CheckBit(PLAYSTEXPA_BIT_Sweapstrike_Used))
		return MENU_DISABLED;

	return MENU_ENABLED;
}

static u8 Sweapstrike_OnSelectTarget(ProcPtr proc, struct SelectTarget *target)
{
	gActionData.xOther = target->x;
	gActionData.yOther = target->y;

	HideMoveRangeGraphics();

	BG_Fill(gBG2TilemapBuffer, 0);
	BG_EnableSyncByMask(BG2_SYNC_BIT);

	gActionData.unk08 = SID_Sweapstrike;
	gActionData.unitActionType = CONFIG_UNIT_ACTION_EXPA_ExecSkill;

	return TARGETSELECTION_ACTION_ENDFAST | TARGETSELECTION_ACTION_END
	     | TARGETSELECTION_ACTION_SE_6A | TARGETSELECTION_ACTION_CLEARBGS;
}

u8 Sweapstrike_OnSelected(struct MenuProc *menu, struct MenuItemProc *item)
{
	if (item->availability == MENU_DISABLED) {
		MenuFrozenHelpBox(menu, MSG_SKILL_CannotUseAgain_FRtext);
		return MENU_ACT_SND6B;
	}

	ClearBg0Bg1();

	Sweapstrike_MakeTargetList(gActiveUnit);
	BmMapFill(gBmMapMovement, -1);

	StartSubtitleHelp(
		NewTargetSelection_Specialized(&sSelectInfo_Sweapstrike, Sweapstrike_OnSelectTarget),
		GetStringFromIndex(MSG_SKILL_Sweapstrike_Target));

	PlaySoundEffect(0x6A);
	return MENU_ACT_SKIPCURSOR | MENU_ACT_END | MENU_ACT_SND6A;
}

/*
 * The vanilla arrow trap, shot down column xOther: every unit in the column
 * takes SWEAPSTRIKE_DAMAGE and units brought to 0 HP die (ShootArrow_ASMC,
 * Kernel/Wizardry/VeslyShootArrow).
 */
static const EventScr EventScr_Sweapstrike[] = {
	SHOOT_ARROW_DYNAMIC(SWEAPSTRIKE_DAMAGE)
	NOFADE
	ENDA
};

bool Action_Sweapstrike(ProcPtr parent)
{
	PlayStExpa_SetBit(PLAYSTEXPA_BIT_Sweapstrike_Used);

	gEventSlots[EVT_SLOT_B] = COORDS(gActionData.xOther, gActionData.yOther);
	ClearBg0Bg1();
	KernelCallEvent(EventScr_Sweapstrike, EV_EXEC_CUTSCENE, parent);
	return true;
}

#endif
