#include "common-chax.h"
#include "skill-system.h"
#include "debuff.h"
#include "constants/skills.h"
#include "constants/texts.h"

#if defined(SID_Prayer) && (COMMON_SKILL_VALID(SID_Prayer))

/*
 * Prayer: the user becomes Lucky (NEW_UNIT_STATUS_LUCKY) for 3 turns. While
 * Lucky, its chance-based skills are 1.5x as likely (CheckSkillActivateExt).
 */
u8 Prayer_Usability(const struct MenuItemDef *def, int number)
{
	if (gActiveUnit->state & US_CANTOING)
		return MENU_NOTSHOWN;

	if (GetUnitStatusIndex(gActiveUnit) == NEW_UNIT_STATUS_LUCKY)
		return MENU_DISABLED;

	return MENU_ENABLED;
}

u8 Prayer_OnSelected(struct MenuProc *menu, struct MenuItemProc *item)
{
	if (item->availability == MENU_DISABLED) {
		MenuFrozenHelpBox(menu, MSG_SKILL_Prayer_FRtext);
		return MENU_ACT_SND6B;
	}

	gActionData.unk08 = SID_Prayer;
	gActionData.unitActionType = CONFIG_UNIT_ACTION_EXPA_ExecSkill;

	return MENU_ACT_SKIPCURSOR | MENU_ACT_END | MENU_ACT_SND6A | MENU_ACT_CLEAR;
}

static void callback_anim(ProcPtr proc)
{
	PlaySoundEffect(0x269);
	Proc_StartBlocking(ProcScr_DanceringAnim, proc);

	BG_SetPosition(BG_0, -SCREEN_TILE_IX(gActiveUnit->xPos - 1), -SCREEN_TILE_IX(gActiveUnit->yPos - 2));
}

static void callback_exec(ProcPtr proc)
{
	SetUnitStatus(gActiveUnit, NEW_UNIT_STATUS_LUCKY);
}

bool Action_Prayer(ProcPtr parent)
{
	NewMuSkillAnimOnActiveUnit(gActionData.unk08, callback_anim, callback_exec);
	return true;
}

#endif
