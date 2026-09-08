#include "common-chax.h"
#include "map-anims.h"
#include "skill-system.h"
#include "constants/skills.h"
#include "constants/texts.h"
#include "debuff.h"
#include "playst-expa.h"
#include "jester_headers/class-pairs.h"

#if defined(SID_Ballistary) && (COMMON_SKILL_VALID(SID_Ballistary))

u8 Ballistary_Usability(const struct MenuItemDef *def, int number)
{
    if (gActiveUnit->state & US_CANTOING)
        return MENU_NOTSHOWN;

    if (PlayStExpa_CheckBit(PLAYSTEXPA_BIT_Ballistary_Used))
		return MENU_DISABLED;

    if (gActiveUnit->state & US_IN_BALLISTA)
        return MENU_NOTSHOWN;

    return MENU_ENABLED;
}

u8 Ballistary_OnSelected(struct MenuProc *menu, struct MenuItemProc *item)
{
    if (item->availability == MENU_DISABLED)
    {
        MenuFrozenHelpBox(menu, MSG_Ballistary_NO_USE);
        return MENU_ACT_SND6B;
    }

    gActionData.unk08 = SID_Ballistary;

    gActionData.unitActionType = UNIT_ACTION_RIDE_BALLISTA;
    AddBallista(gActiveUnit->xPos, gActiveUnit->yPos, ITEM_BALLISTA_REGULAR);
    RideBallista(gActiveUnit);

    EndAllMus();
    StartMu(gActiveUnit);

    PlayStExpa_SetBit(PLAYSTEXPA_BIT_Ballistary_Used);

    return MENU_ACT_SKIPCURSOR | MENU_ACT_END | MENU_ACT_SND6A | MENU_ACT_CLEAR;
}

// static void callback_anim(ProcPtr proc)
// {

// }

// static void callback_exec(ProcPtr proc)
// {

// }

bool Action_Ballistary(ProcPtr parent)
{

    // NewMuSkillAnimOnActiveUnit(gActionData.unk08, callback_anim, callback_exec);
    return true;
}
#endif