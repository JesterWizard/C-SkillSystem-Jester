#include "common-chax.h"
#include "playst-expa.h"
#include "kernel-lib.h"
#include "skill-system.h"
#include "map-anims.h"
#include "constants/skills.h"
#include "constants/texts.h"

#if defined(SID_GraceOfWater) && (COMMON_SKILL_VALID(SID_GraceOfWater))

u8 GraceOfWater_Usability(const struct MenuItemDef *def, int number)
{
	if (gActiveUnit->state & US_CANTOING)
		return MENU_NOTSHOWN;

	if (PlayStExpa_CheckBit(PLAYSTEXPA_BIT_GraceOfWater_Used))
		return MENU_DISABLED;

	return MENU_ENABLED;
}

u8 GraceOfWater_OnSelected(struct MenuProc *menu, struct MenuItemProc *item)
{
	if (item->availability == MENU_DISABLED) {
		MenuFrozenHelpBox(menu, MSG_SKILL_CannotUseAgain_FRtext);
		return MENU_ACT_SND6B;
	}

	gActionData.unk08 = SID_GraceOfWater;
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
	struct Unit * unit;

	PlayStExpa_SetBit(PLAYSTEXPA_BIT_GraceOfWater_Used);

	 for (int uid = gPlaySt.faction + 1; uid <= (gPlaySt.faction + GetFactionUnitAmount(gPlaySt.faction)); uid++)
	 {
		unit = GetUnit(uid);

		if (!UNIT_IS_VALID(unit))
            break;

		unit->curHP += SKILL_EFF0(SID_GraceOfWater);
	 }
}

bool Action_GraceOfWater(ProcPtr parent)
{
	NewMuSkillAnimOnActiveUnitWithDeamon(parent, gActionData.unk08, callback_anim, callback_exec);
	return true;
}

#endif /* SID_GraceOfWater */