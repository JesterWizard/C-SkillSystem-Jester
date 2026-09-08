#include "common-chax.h"
#include "debuff.h"
#include "skill-system.h"
#include "kernel-lib.h"
#include "constants/skills.h"

void PostAction_EXPShare(ProcPtr parent)
{
    if (gActionData.unitActionType != UNIT_ACTION_COMBAT)
        return;

    struct Unit *actor = gActiveUnit;

    if (!UnitAvaliable(actor) || UNIT_STONED(actor))
        return;

#if defined(SID_EXPShare) && COMMON_SKILL_VALID(SID_EXPShare)
    int x = actor->xPos;
    int y = actor->yPos;
    int actorIndex = actor->index;

    for (int i = 0; i < ARRAY_COUNT_RANGE2x2; i++)
    {
        struct Unit * ally = GetUnitAtPosition(x + gVecs_2x2[i].x, y + gVecs_2x2[i].y);

        if (!UNIT_IS_VALID(ally))
            continue;

        if (!AreUnitsAllied(actorIndex, ally->index))
            continue;

        if (ally->state & (US_HIDDEN | US_DEAD | US_RESCUED | US_BIT16))
            continue;

        // Skill check LAST
        if (SkillTester(ally, SID_EXPShare))
            ally->exp += 10;
    }
#endif
}
