#include "common-chax.h"
#include "map-anims.h"
#include "skill-system.h"
#include "constants/skills.h"
#include "constants/texts.h"
#include "debuff.h"
#include "unit-expa.h"
#include "battle-system.h"
#include "playst-expa.h"

#ifndef CONFIG_UNIT_ACTION_EXPA_ExecSkill
    #define CONFIG_UNIT_ACTION_EXPA_ExecSkill 20
#endif

#if defined(SID_LethalityPlus) && (COMMON_SKILL_VALID(SID_LethalityPlus))

u8 LethalityPlus_Usability(const struct MenuItemDef *def, int number)
{
    if (AttackCommandUsability(def, number) == MENU_ENABLED)
        return MENU_ENABLED;

    if (PlayStExpa_CheckBit(PLAYSTEXPA_BIT_LethalityPlus_Used))
        return MENU_DISABLED;

    return MENU_NOTSHOWN;
}

u8 LethalityPlus_OnSelected(struct MenuProc *menu, struct MenuItemProc *item)
{
    if (item->availability == MENU_DISABLED)
    {
        MenuFrozenHelpBox(menu, MSG_SKILL_CommonFail);
        return MENU_ACT_SND6B;
    }

    gActionData.unk08 = SID_LethalityPlus;
    gActionData.unitActionType = CONFIG_UNIT_ACTION_EXPA_ExecSkill;

	ResetIconGraphics();
	LoadIconPalettes(4);

    return StartUnitWeaponSelect(menu, item);
}

/* Unused as we're hijacking the attack command menu in LethalityPlus_OnSelected */
bool Action_LethalityPlus(ProcPtr parent)
{
    return true;
}
#endif