#include "common-chax.h"
#include "map-anims.h"
#include "skill-system.h"
#include "constants/skills.h"
#include "constants/texts.h"
#include "debuff.h"
#include "unit-expa.h"
#include "battle-system.h"

#ifndef CONFIG_UNIT_ACTION_EXPA_ExecSkill
    #define CONFIG_UNIT_ACTION_EXPA_ExecSkill 20
#endif

#if defined(SID_GrimReaper) && (COMMON_SKILL_VALID(SID_GrimReaper))

u8 GrimReaper_Usability(const struct MenuItemDef *def, int number)
{
    if (AttackCommandUsability(def, number) == MENU_ENABLED)
        return MENU_ENABLED;

    return MENU_NOTSHOWN;
}

u8 GrimReaper_OnSelected(struct MenuProc *menu, struct MenuItemProc *item)
{
    if (item->availability == MENU_DISABLED)
    {
        MenuFrozenHelpBox(menu, MSG_SKILL_CommonFail);
        return MENU_ACT_SND6B;
    }

    gActionData.unk08 = SID_GrimReaper;
    gActionData.unitActionType = CONFIG_UNIT_ACTION_EXPA_ExecSkill;

    SetBitUES(gActiveUnit, UES_BIT_GRIM_REAPER_SKILL_USED);

    // return MENU_ACT_SKIPCURSOR | MENU_ACT_END | MENU_ACT_SND6A | MENU_ACT_CLEAR;
    return StartUnitWeaponSelect(menu, item);
}

/* Unused as we're hijacking the attack command menu in GrimReaper_OnSelected */
bool Action_GrimReaper(ProcPtr parent)
{
    return true;
}
#endif