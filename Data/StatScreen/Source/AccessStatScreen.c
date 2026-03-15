#include "common-chax.h"
#include "kernel-lib.h"
#include "jester_headers/custom-arrays.h"

//! FE8U = 0x0801C928
LYN_REPLACE_CHECK(CanShowUnitStatScreen);
bool CanShowUnitStatScreen(struct Unit * unit)
{
    if (gpKernelDesignerConfig->deny_stat_screen_access == true)
    {
        for (int i = 0; i < (int)ARRAY_COUNT(statScreenDenyClasses); i++)
            if (UNIT_CLASS_ID(unit) == statScreenDenyClasses[i])
                return false;
    }

/* Deny stat screen access for stages 1-3 (any fog obscures the unit in some way) */
    if (gpKernelDesignerConfig->multiple_fog_stages == true
            && gPlaySt.chapterVisionRange && gBmMapFog[unit->yPos][unit->xPos] < 3)
        return false;

    if (UNIT_IS_GORGON_EGG(unit))
        return false;

    return true;
}

LYN_REPLACE_CHECK(FindNextUnit);
struct Unit* FindNextUnit(struct Unit* u, int direction)
{
    int faction = UNIT_FACTION(u);
    int i       = u->index;

    struct Unit* unit;

    while (TRUE)
    {
        i = (i + direction) & 0x3F;
        unit = GetUnit(faction + i);

        if (!UNIT_IS_VALID(unit))
            continue;

        if ((sStatScreenInfo.config & STATSCREEN_CONFIG_NONDEAD) && (unit->state & US_DEAD))
            continue;

        if ((sStatScreenInfo.config & STATSCREEN_CONFIG_NONBENCHED) && (unit->state & US_NOT_DEPLOYED))
            continue;

        if ((sStatScreenInfo.config & STATSCREEN_CONFIG_NONUNK9) && (unit->state & US_BIT9))
            continue;

        if ((sStatScreenInfo.config & STATSCREEN_CONFIG_NONROOFED) && (unit->state & US_UNDER_A_ROOF))
            continue;

        if ((sStatScreenInfo.config & STATSCREEN_CONFIG_NONUNK16) && (unit->state & US_BIT16))
            continue;

        if ((sStatScreenInfo.config & STATSCREEN_CONFIG_NONSUPPLY) && (UNIT_CATTRIBUTES(unit) & CA_SUPPLY))
            continue;

    if (gpKernelDesignerConfig->deny_stat_screen_access == true)
    {
        for (int i = 0; i < (int)ARRAY_COUNT(statScreenDenyClasses); i++)
        {
            if (UNIT_CLASS_ID(unit) == statScreenDenyClasses[i])
                continue;
        }
    }

/* Deny browsing to units in any fog stage (stages 1-3 all obscure the unit) */
        if (gpKernelDesignerConfig->multiple_fog_stages == true
                && gPlaySt.chapterVisionRange && gBmMapFog[unit->yPos][unit->xPos] < 3)
            continue;

        if (UNIT_IS_GORGON_EGG(unit))
            continue;

        return unit;
    }
}