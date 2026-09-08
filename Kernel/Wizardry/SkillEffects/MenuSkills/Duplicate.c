#include "common-chax.h"
#include "playst-expa.h"
#include "map-anims.h"
#include "kernel-lib.h"
#include "skill-system.h"
#include "status-getter.h"
#include "constants/skills.h"
#include "constants/texts.h"
#include "debuff.h"

extern u16 gUnknown_085A0D4C[];

#if defined(SID_Duplicate) && (COMMON_SKILL_VALID(SID_Duplicate))

static void AddTargetForDuplicate(struct Unit *unit)
{
	if (!UnitOnMapAvaliable(unit))
		return;

	if (AreUnitsAllied(gSubjectUnit->index, unit->index))
		return;

	for (int i = 0; i < UNIT_ITEM_COUNT; i++)
	{
		if (GetItemAttributes(unit->items[0]) & IA_WEAPON)
			AddTarget(unit->xPos, unit->yPos, unit->index, 1);
	}
}

static void MakeTargetListForDuplicate(struct Unit *unit)
{
	int x = unit->xPos;
	int y = unit->yPos;

	gSubjectUnit = unit;
	BmMapFill(gBmMapRange, 0);
	MapAddInBoundedRange(x, y, 1, 1);

	InitTargets(x, y);
	ForEachUnitInRange(AddTargetForDuplicate);
}

u8 Duplicate_Usability(const struct MenuItemDef *def, int number)
{
	if (gActiveUnit->state & US_CANTOING)
		return MENU_NOTSHOWN;

	if (!HasSelectTarget(gActiveUnit, MakeTargetListForDuplicate))
		return MENU_NOTSHOWN;

	if (PlayStExpa_CheckBit(PLAYSTEXPA_BIT_Duplicate_Used))
		return MENU_NOTSHOWN;

	return MENU_ENABLED;
}

u8 Duplicate_OnSelected(struct MenuProc *menu, struct MenuItemProc *item)
{
	if (item->availability == MENU_DISABLED) {
		MenuFrozenHelpBox(menu, MSG_SKILL_CommonFail);
		return MENU_ACT_SND6B;
	}

	ClearBg0Bg1();

	MakeTargetListForDuplicate(gActiveUnit);
	NewTargetSelection(&gSelectInfo_Steal);

	gActionData.unk08 = SID_Duplicate;
	gActionData.unitActionType = CONFIG_UNIT_ACTION_EXPA_ExecSkill;

	PlayStExpa_SetBit(PLAYSTEXPA_BIT_Duplicate_Used);

	PlaySoundEffect(0x6A);
	return MENU_ACT_SKIPCURSOR | MENU_ACT_END | MENU_ACT_SND6A;
}

static void callback_anim(ProcPtr proc)
{
	PlaySoundEffect(0x269);
	Proc_StartBlocking(ProcScr_DanceringAnim, proc);

	BG_SetPosition(
		BG_0,
		-SCREEN_TILE_IX(gActiveUnit->xPos - 1),
		-SCREEN_TILE_IX(gActiveUnit->yPos - 2));
}

static void callback_exec(ProcPtr proc)
{

}

bool Action_Duplicate(ProcPtr parent)
{
	NewMuSkillAnimOnActiveUnitWithDeamon(parent, gActionData.unk08, callback_anim, callback_exec);
	return true;
}
#endif