#include "common-chax.h"
#include "playst-expa.h"
#include "map-anims.h"
#include "skill-system.h"
#include "battle-system.h"
#include "constants/skills.h"
#include "constants/texts.h"

#if defined(SID_Reroll) && (COMMON_SKILL_VALID(SID_Reroll))

u8 Reroll_Usability(const struct MenuItemDef *def, int number)
{
    if (gActiveUnit->state & US_CANTOING)
        return MENU_NOTSHOWN;

    if (PlayStExpa_CheckBit(PLAYSTEXPA_BIT_Reroll_Used))
        return MENU_NOTSHOWN;

    return MENU_ENABLED;
}


u8 Reroll_OnSelected(struct MenuProc *menu, struct MenuItemProc *item)
{
	if (item->availability == MENU_DISABLED) {
		MenuFrozenHelpBox(menu, MSG_SKILL_CommonFail);
		return MENU_ACT_SND6B;
	}

	gActionData.unk08 = SID_Reroll;
	gActionData.unitActionType = CONFIG_UNIT_ACTION_EXPA_ExecSkill;
    gEventSlots[EVT_SLOT_8] = SID_Reroll; // The remove skill menu will overwrite gActionData.unk08 so we need to use an event slot instead 

    PlayStExpa_SetBit(PLAYSTEXPA_BIT_Reroll_Used);

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
    ResetIconGraphics();
	LoadIconPalettes(4);
    
    StartSubtitleHelp(
        StartOrphanMenu(&RemoveSkillMenuDef),
        GetStringFromIndex(MSG_RerollSelect)
    );
}

bool Action_Reroll(ProcPtr parent)
{
	NewMuSkillAnimOnActiveUnitWithDeamon(parent, gActionData.unk08, callback_anim, callback_exec);
	return true;
}
#endif
