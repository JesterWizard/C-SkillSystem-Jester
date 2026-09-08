#include "common-chax.h"
#include "map-anims.h"
#include "skill-system.h"
#include "constants/skills.h"
#include "constants/texts.h"
#include "unit-expa.h"
#include "action-expa.h"
#include "debuff.h"
#include "jester_headers/miscellaneous.h"

#ifndef CONFIG_UNIT_ACTION_EXPA_ExecSkill
    #define CONFIG_UNIT_ACTION_EXPA_ExecSkill 20
#endif

#if defined(SID_Bide) && (COMMON_SKILL_VALID(SID_Bide))

u8 Bide_Usability(const struct MenuItemDef * def, int number)
{
    int i;
    bool enemies = false;

    if (gActiveUnit->state & US_CANTOING || gActiveUnit->state & US_RESCUING)
        return MENU_NOTSHOWN;

    if (CheckBitUES(gActiveUnit, UES_BIT_BIDE_SKILL_USED))
        return MENU_NOTSHOWN;

    if(gActiveUnit->curHP == 1)
        return MENU_NOTSHOWN;

    /**
     * Since we can't use gBattleStats.range at this point, we check to
     * see if there are any adjacent enemies instead
     */
    for (i = 0; i < ARRAY_COUNT_RANGE1x1; i++)
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

    if (AttackCommandUsability(def, number) == MENU_ENABLED)
        return MENU_ENABLED;

    return MENU_NOTSHOWN;
}

u8 Bide_OnSelected(struct MenuProc * menu, struct MenuItemProc * item)
{
    ResetIconGraphics();

    LoadIconPalettes(4);

    gActionData.unk08 = SID_Bide;
    gActionData.unitActionType = CONFIG_UNIT_ACTION_EXPA_ExecSkill;

    if (gActiveUnit->state & US_IN_BALLISTA)
    {
        return StartUnitBallistaSelect(menu, item);
    }

    return StartUnitWeaponSelect(menu, item);
}

bool Action_Bide(ProcPtr parent)
{
    SetBitUES(gActiveUnit, UES_BIT_BIDE_SKILL_USED);
	return true;
}

#endif