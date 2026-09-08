#include "common-chax.h"
#include "map-anims.h"
#include "skill-system.h"
#include "constants/skills.h"
#include "constants/texts.h"
#include "unit-expa.h"
#include "action-expa.h"
#include "debuff.h"
#include "playst-expa.h"
#include "jester_headers/miscellaneous.h"

#ifndef CONFIG_UNIT_ACTION_EXPA_ExecSkill
    #define CONFIG_UNIT_ACTION_EXPA_ExecSkill 20
#endif

#if defined(SID_Thrust) && (COMMON_SKILL_VALID(SID_Thrust))

u8 Thrust_Usability(const struct MenuItemDef * def, int number)
{
    int i;
    bool enemies = false;

    if (gActiveUnit->state & US_CANTOING)
        return MENU_NOTSHOWN;

    if (PlayStExpa_CheckBit(PLAYSTEXPA_BIT_Thrust_Used))
		return MENU_NOTSHOWN;

    /**
     * Since we can't use gBattleStats.range at this point, we check to
     * see if there are any adjacent enemies instead
     */
    for (i = 0; i < ARRAY_COUNT_RANGE5x5; i++)
    {
        int _x = gActiveUnit->xPos + gVecs_1x1[i].x;
        int _y = gActiveUnit->yPos + gVecs_1x1[i].y;

        struct Unit * unit_enemy = GetUnitAtPosition(_x, _y);
        if (!UNIT_IS_VALID(unit_enemy))
            continue;

        if (unit_enemy->state & (US_HIDDEN | US_DEAD | US_RESCUED | US_BIT16))
            continue;

        if (!AreUnitsAllied(gActiveUnit->index, unit_enemy->index))
        {
            enemies = true;
            break;
        }
    }

    if (!enemies)
        return MENU_NOTSHOWN;

    return MENU_ENABLED;
}

u8 Thrust_OnSelected(struct MenuProc * menu, struct MenuItemProc * item)
{
    if (item->availability == MENU_DISABLED)
    {
        MenuFrozenHelpBox(menu, 0x858); // TODO: msgid "There's no more ammo for[NL]the ballista.[.]"
        return MENU_ACT_SND6B;
    }

    ResetIconGraphics();

    LoadIconPalettes(4);

    if (gActiveUnit->state & US_IN_BALLISTA)
    {
        return StartUnitBallistaSelect(menu, item);
    }

    gActionData.unk08 = SID_Thrust;
    gActionData.unitActionType = CONFIG_UNIT_ACTION_EXPA_ExecSkill;

    PlayStExpa_SetBit(PLAYSTEXPA_BIT_Thrust_InForce);

    return StartUnitWeaponSelect(menu, item);
}

bool Action_Thrust(ProcPtr parent)
{
	return true;
}

#endif