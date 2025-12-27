#include "common-chax.h"
#include "weapon-range.h"
#include "kernel-lib.h"
#include "skill-system.h"
#include "constants/skills.h"
#include "constants/texts.h"
#include "debuff.h"
#include "class-types.h"
#include "unit-expa.h"
#include "jester_headers/custom-functions.h"

#if defined(SID_GorillaTactics) && (COMMON_SKILL_VALID(SID_GorillaTactics))
u8 GorillaTactics_Usability(const struct MenuItemDef* def, int number)
{
	if (gActiveUnit->state & US_CANTOING)
		return MENU_NOTSHOWN;

	if (!HasSelectTarget(gActiveUnit, MakeTargetListForAdjacentSameFaction))
		return MENU_NOTSHOWN;

	return MENU_ENABLED;
}



u8 GorillaTactics_OnSelected(struct MenuProc* menu, struct MenuItemProc* item)
{
	if (item->availability == MENU_DISABLED) {
		MenuFrozenHelpBox(menu, MSG_No_Adjacent_Units);
		return MENU_ACT_SND6B;
	}

	gActionData.unk08 = SID_GorillaTactics;
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
	if (!CheckBitUES(gActiveUnit, UES_BIT_GORILLA_TACTICS_SKILL_USED))
		SetBitUES(gActiveUnit, UES_BIT_GORILLA_TACTICS_SKILL_USED);
	else
		ClearBitUES(gActiveUnit, UES_BIT_GORILLA_TACTICS_SKILL_USED);
}

bool Action_GorillaTactics(ProcPtr parent)
{
	NewMuSkillAnimOnActiveUnitWithDeamon(parent, gActionData.unk08, callback_anim, callback_exec);
	return true;
}
#endif
