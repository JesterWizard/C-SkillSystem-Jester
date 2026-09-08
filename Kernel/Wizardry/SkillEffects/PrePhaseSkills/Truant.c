#include "common-chax.h"
#include "unit-expa.h"
#include "bwl.h"

void PrePhase_Truant(ProcPtr proc)
{
#if defined(SID_Truant) && (COMMON_SKILL_VALID(SID_Truant))
    int uid;
    struct Unit *unit;
    if (gPlaySt.chapterTurnNumber % 2 == 0)
    {
        for (uid = gPlaySt.faction + 1; uid <= (gPlaySt.faction + GetFactionUnitAmount(gPlaySt.faction)); uid++)
        {
            unit = GetUnit(uid);
            if (UNIT_IS_VALID(unit) && SkillTester(unit, SID_Truant))
            {
                // If the unit has the Truant skill, set its state to unselectable and has moved
                // This prevents the unit from being selected or moved during this phase
                unit->state |= US_HAS_MOVED;
                unit->state |= US_UNSELECTABLE;
            }
        }
    }
#endif
}
