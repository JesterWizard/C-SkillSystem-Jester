#include "common-chax.h"
#include "skill-system.h"
#include "playst-expa.h"
#include "constants/skills.h"

#ifndef CONFIG_UNIT_ACTION_EXPA_ExecSkill
    #define CONFIG_UNIT_ACTION_EXPA_ExecSkill 20
#endif

#if defined(SID_AssumingCommand) && (COMMON_SKILL_VALID(SID_AssumingCommand))

u8 AssumingCommand_Usability(const struct MenuItemDef* def, int number)
{
    if (gActiveUnit->state & US_CANTOING)
        return MENU_NOTSHOWN;

    if (PlayStExpa_CheckBit(PLAYSTEXPA_BIT_AssumingCommand_Used))
        return MENU_NOTSHOWN;

    return MENU_ENABLED;
}

u8 AssumingCommand_OnSelected(struct MenuProc* menu, struct MenuItemProc* item)
{
    gActionData.unk08 = SID_AssumingCommand;
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
    PlayStExpa_SetBit(PLAYSTEXPA_BIT_AssumingCommand_Used);
}

bool Action_AssumingCommand(ProcPtr parent)
{
    NewMuSkillAnimOnActiveUnitWithDeamon(parent, gActionData.unk08, callback_anim, callback_exec);
    return true;
}

#endif
