#include "common-chax.h"
#include "playst-expa.h"
#include "kernel-lib.h"
#include "skill-system.h"
#include "map-anims.h"
#include "constants/skills.h"
#include "constants/texts.h"

#ifndef CONFIG_UNIT_ACTION_EXPA_ExecSkill
    #define CONFIG_UNIT_ACTION_EXPA_ExecSkill 20
#endif

#if defined(SID_BloodyAlchemy) && (COMMON_SKILL_VALID(SID_BloodyAlchemy))

u8 BloodyAlchemy_Usability(const struct MenuItemDef *def, int number)
{
	if (gActiveUnit->state & US_CANTOING)
		return MENU_NOTSHOWN;

	if (PlayStExpa_CheckBit(PLAYSTEXPA_BIT_BloodyAlchemy_Used))
		return MENU_DISABLED;

	return MENU_ENABLED;
}

u8 BloodyAlchemy_OnSelected(struct MenuProc *menu, struct MenuItemProc *item)
{
	if (item->availability == MENU_DISABLED) {
		MenuFrozenHelpBox(menu, MSG_SKILL_CommonFail);
		return MENU_ACT_SND6B;
	}

	gActionData.unk08 = SID_BloodyAlchemy;
	gActionData.unitActionType = CONFIG_UNIT_ACTION_EXPA_ExecSkill;

	return MENU_ACT_SKIPCURSOR | MENU_ACT_END | MENU_ACT_SND6A | MENU_ACT_CLEAR;
}

static void callback_anim(ProcPtr proc)
{
	PlaySoundEffect(0x269);
	StartNinianPrayfx(proc, gActiveUnit->xPos, gActiveUnit->yPos);
}

static void callback_exec(ProcPtr proc)
{
	PlayStExpa_SetBit(PLAYSTEXPA_BIT_BloodyAlchemy_Used);
}

bool Action_BloodyAlchemy(ProcPtr parent)
{
	NewMuSkillAnimOnActiveUnitWithDeamon(parent, gActionData.unk08, callback_anim, callback_exec);
	return true;
}

#endif