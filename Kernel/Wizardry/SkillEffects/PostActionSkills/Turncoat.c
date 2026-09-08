#include "common-chax.h"
#include "debuff.h"
#include "skill-system.h"
#include "kernel-lib.h"
#include "unit-expa.h"
#include "constants/skills.h"

bool PostAction_Turncoat(ProcPtr parent)
{
#if defined(SID_Turncoat) && (COMMON_SKILL_VALID(SID_Turncoat))
    if (gActionData.unitActionType != UNIT_ACTION_COMBAT)
        return false;

    struct Unit *unit = gActiveUnit;
    struct Unit *unit_tar = GetUnit(gActionData.targetIndex);

    if (!UNIT_ALIVE(unit) || UNIT_STONED(unit))
        return false;

    if (!UNIT_ALIVE(unit_tar) || UNIT_STONED(unit_tar))
        return false;

    if (SkillTester(unit, SID_Turncoat))
    {
        SetBitUES(unit_tar, UES_BIT_CHANGED_FACTIONS);

        if (UNIT_FACTION(unit_tar) == FACTION_RED)
            UnitChangeFaction(unit_tar, FACTION_BLUE);
        else
            UnitChangeFaction(unit_tar, FACTION_RED);
    }
    else if (SkillTester(unit_tar, SID_Turncoat))
    {
        SetBitUES(unit, UES_BIT_CHANGED_FACTIONS);

        if (UNIT_FACTION(unit) == FACTION_RED)
            UnitChangeFaction(unit, FACTION_BLUE);
        else
            UnitChangeFaction(unit, FACTION_RED);
    }

    return false;
#endif
}
