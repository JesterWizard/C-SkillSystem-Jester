#include "common-chax.h"
#include "map-anims.h"
#include "skill-system.h"
#include "constants/skills.h"
#include "constants/texts.h"
#include "debuff.h"
#include "action-expa.h"
#include "icon-rework.h"
#include "item-sys.h"
#include "jester_headers/miscellaneous.h"

#ifndef CONFIG_UNIT_ACTION_EXPA_ExecSkill
    #define CONFIG_UNIT_ACTION_EXPA_ExecSkill 20
#endif

#if defined(SID_ScrollScribePlus) && (COMMON_SKILL_VALID(SID_ScrollScribePlus))

u8 ScrollScribePlus_Usability(const struct MenuItemDef * def, int number)
{
    if (gActiveUnit->state & US_CANTOING)
        return MENU_NOTSHOWN;

    return MENU_ENABLED;
}


u8 ScrollScribePlus_OnSelected(struct MenuProc * menu, struct MenuItemProc * item)
{
    if (item->availability == MENU_DISABLED)
    {
        MenuFrozenHelpBox(menu, MSG_SKILL_CommonFail);
        return MENU_ACT_SND6B;
    }

    HideMoveRangeGraphics();

    ResetIconGraphics();
    LoadIconPalettes(4);

    gActionData.unk08 = SID_ScrollScribePlus;
    gActionData.unitActionType = CONFIG_UNIT_ACTION_EXPA_ExecSkill;
    gEventSlots[EVT_SLOT_8] = SID_ScrollScribePlus; // The remove skill menu will overwrite gActionData.unk08 so we need to use an event slot instead 
    
    return MENU_ACT_SKIPCURSOR | MENU_ACT_END | MENU_ACT_SND6A | MENU_ACT_CLEAR;
}

static void callback_anim(ProcPtr proc)
{

}

static void callback_exec(ProcPtr proc)
{	
    StartSubtitleHelp(
        StartOrphanMenu(&RemoveSkillMenuDef),
        GetStringFromIndex(MSG_RemoveSkillSubtitle)
    );
}

bool Action_ScrollScribePlus(ProcPtr parent)
{
	NewMuSkillAnimOnActiveUnit(gActionData.unk08, callback_anim, callback_exec);
	return true;
}
#endif