#include "common-chax.h"
#include "weapon-range.h"
#include "kernel-lib.h"
#include "skill-system.h"
#include "playst-expa.h"
#include "constants/skills.h"
#include "constants/texts.h"
#include "jester_headers/custom-arrays.h"

/* This skill needs gpKernelDesignerConfig->collect_dead_units to be set to true to work */

#if defined(SID_Necromancy) && (COMMON_SKILL_VALID(SID_Necromancy))
u8 Necromancy_Usability(const struct MenuItemDef *def, int number)
{
	if (gActiveUnit->state & US_CANTOING)
		return MENU_NOTSHOWN;

	if (PlayStExpa_CheckBit(PLAYSTEXPA_BIT_Necromancy_Used))
		return MENU_NOTSHOWN;

	if (GetLastDeadUnit() == 0)
		return MENU_NOTSHOWN;

	return MENU_ENABLED;
}

u8 Necromancy_OnSelected(struct MenuProc *menu, struct MenuItemProc *item)
{
	gActionData.unk08 = SID_Necromancy;
	gActionData.unitActionType = CONFIG_UNIT_ACTION_EXPA_ExecSkill;

	int unitId = GetLastDeadUnit();
	struct Unit * unit = GetUnit(unitId);

	unit->state &= ~(US_HIDDEN | US_UNSELECTABLE | US_DEAD);
	UnitChangeFaction(unit, FACTION_GREEN);
    unit->curHP = unit->maxHP;

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
	NewPopup_VerySimple(MSG_UnitRevived, 0x5A, proc);
	PlayStExpa_SetBit(PLAYSTEXPA_BIT_Necromancy_Used);
}

bool Action_Necromancy(ProcPtr parent)
{
	NewMuSkillAnimOnActiveUnit(gActionData.unk08, callback_anim, callback_exec);
	return true;
}
#endif
