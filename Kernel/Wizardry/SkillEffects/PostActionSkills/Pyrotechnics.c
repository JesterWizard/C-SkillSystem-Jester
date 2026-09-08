#include "common-chax.h"
#include "debuff.h"
#include "skill-system.h"
#include "kernel-lib.h"
#include "constants/skills.h"

bool PostAction_Pyrotechnics(ProcPtr parent)
{
    if (gActionData.unitActionType != UNIT_ACTION_COMBAT)
        return false;

    if (!UnitAvaliable(gActiveUnit) || UNIT_STONED(gActiveUnit))
        return false;

#if defined(SID_Pyrotechnics) && COMMON_SKILL_VALID(SID_Pyrotechnics)
    if (!SkillListTester(gActiveUnit, SID_Pyrotechnics))
        return false;

    int x = gActiveUnit->xPos;
    int y = gActiveUnit->yPos;

    for (int i = 0; i < ARRAY_COUNT_RANGE2x2; i++)
    {
        struct Unit * enemy = GetUnitAtPosition(x + gVecs_2x2[i].x, y + gVecs_2x2[i].y);

        if (!UNIT_IS_VALID(enemy))
            continue;

        if (enemy->state & (US_HIDDEN | US_DEAD | US_RESCUED | US_BIT16))
            continue;

        int hp = enemy->curHP - (enemy->maxHP / 5);
        enemy->curHP = (hp > 1) ? hp : 1;
    }
#endif

    return false;
}
