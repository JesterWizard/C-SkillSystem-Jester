#include "common-chax.h"
#include "map-anims.h"
#include "kernel-lib.h"
#include "skill-system.h"
#include "constants/skills.h"
#include "constants/texts.h"
#include "debuff.h"

#if defined(SID_Fogger) && (COMMON_SKILL_VALID(SID_Fogger))
u8 Fogger_Usability(const struct MenuItemDef *def, int number)
{
	if (gActiveUnit->state & US_CANTOING)
		return MENU_NOTSHOWN;

	return MENU_ENABLED;
}

u8 Fogger_OnSelected(struct MenuProc *menu, struct MenuItemProc *item)
{
	if (item->availability == MENU_DISABLED) {
		MenuFrozenHelpBox(menu, MSG_SKILL_CommonFail);
		return MENU_ACT_SND6B;
	}

	ClearBg0Bg1();

	gActionData.unk08 = SID_Fogger;
	gActionData.unitActionType = CONFIG_UNIT_ACTION_EXPA_ExecSkill;

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
	gPlaySt.chapterVisionRange = 1;
}

bool Action_Fogger(ProcPtr parent)
{
	NewMuSkillAnimOnActiveUnitWithDeamon(parent, gActionData.unk08, callback_anim, callback_exec);
	return true;
}
#endif