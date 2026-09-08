#include "common-chax.h"
#include "debuff.h"
#include "kernel-lib.h"
#include "skill-system.h"
#include "constants/skills.h"
#include "jester_headers/class-pairs.h"


bool PrePhase_TickDopplegangerSkillStatus(ProcPtr proc)
{
#if (defined(SID_Doppleganger) && COMMON_SKILL_VALID(SID_Doppleganger))
    int i;

    for (i = gPlaySt.faction + 1; i <= (gPlaySt.faction + GetFactionUnitAmount(gPlaySt.faction)); ++i)
    {
        struct Unit *unit = GetUnit(i);

        if (!UNIT_IS_VALID(unit))
            continue;

        if (unit->state & (US_HIDDEN | US_DEAD | US_RESCUED | US_BIT16))
            continue;

        if (SkillTester(unit, SID_Doppleganger))
        {
            if (unit->ballistaIndex > 0)
            {
                unit->pClassData = GetClassData(unit->ballistaIndex);
            }
        }
    }
#endif
    return false;
}
