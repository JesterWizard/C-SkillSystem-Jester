#include "common-chax.h"
#include "skill-system.h"
#include "status-getter.h"
#include "constants/skills.h"

/**
 * <!> WARNNING
 * We did not inject CON-Getters to a lot of vanilla function.
 * thus this function should not modified.
 */
int _GetUnitCon(struct Unit *unit)
{
    const StatusGetterFunc_t * it;
    int status = UNIT_CON(unit);

    for (it = gpConGetters; *it; it++)
        status = (*it)(status, unit);

    return status;
}

int ConGetterSkills(int status, struct Unit * unit)
{
#if defined(SID_HeavyMetal) && (COMMON_SKILL_VALID(SID_HeavyMetal))
    if (SkillTester(unit, SID_HeavyMetal))
        status += SKILL_EFF0(SID_HeavyMetal);
#endif
    return status;
}