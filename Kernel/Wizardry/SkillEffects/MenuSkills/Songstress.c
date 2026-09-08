#include "common-chax.h"
#include "weapon-range.h"
#include "kernel-lib.h"
#include "skill-system.h"
#include "playst-expa.h"
#include "constants/skills.h"
#include "constants/texts.h"

#if defined(SID_Songstress) && (COMMON_SKILL_VALID(SID_Songstress))
u8 Songstress_Usability(const struct MenuItemDef *def, int number)
{
	if (gActiveUnit->state & US_CANTOING)
		return MENU_NOTSHOWN;

	if (PlayStExpa_CheckBit(PLAYSTEXPA_BIT_Songstress_Used))
		return MENU_NOTSHOWN;

	return MENU_ENABLED;
}

u8 Songstress_OnSelected(struct MenuProc *menu, struct MenuItemProc *item)
{
	gActionData.unk08 = SID_Songstress;
	gActionData.unitActionType = CONFIG_UNIT_ACTION_EXPA_ExecSkill;

	return MENU_ACT_SKIPCURSOR | MENU_ACT_END | MENU_ACT_SND6A | MENU_ACT_CLEAR;
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
	int i;

	for (i = FACTION_BLUE + 1; i < FACTION_GREEN; i++) {
		struct Unit *unit = GetUnit(i);

		if (!UnitOnMapAvaliable(unit) && unit != gActiveUnit)
			continue;

		unit->state &= ~(US_UNSELECTABLE | US_HAS_MOVED | US_HAS_MOVED_AI);
	}

	PlayStExpa_SetBit(PLAYSTEXPA_BIT_Songstress_Used);
	PlayStExpa_SetBit(PLAYSTEXPA_BIT_Songstress_InForce);
}

bool Action_Songstress(ProcPtr parent)
{
	NewMuSkillAnimOnActiveUnit(gActionData.unk08, callback_anim, callback_exec);
	return true;
}
#endif
