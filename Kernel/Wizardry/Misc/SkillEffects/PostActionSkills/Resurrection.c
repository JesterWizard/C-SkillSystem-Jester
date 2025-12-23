#include "common-chax.h"
#include "debuff.h"
#include "kernel-lib.h"
#include "skill-system.h"
#include "battle-system.h"
#include "event-rework.h"
#include "constants/skills.h"
#include "constants/texts.h"
#include "strmag.h"
#include "jester_headers/macros.h"
#include "unit-expa.h"

#if (defined(SID_Resurrection) && (COMMON_SKILL_VALID(SID_Resurrection)))
STATIC_DECLAR void PrepareResurrection(void)
{
    struct Unit * targetUnit = GetUnit(gBattleTarget.unit.index);

    if (!CheckBitUES(targetUnit, UES_BIT_RESURRECTION_SKILL_USED))
    {
        AddUnitHp(targetUnit, (targetUnit->maxHP - targetUnit->curHP));
        SetBitUES(targetUnit, UES_BIT_RESURRECTION_SKILL_USED);
    }
}
#endif

STATIC_DECLAR const EventScr EventScr_PostAction_Resurrection[] = {
    ASMC(PrepareResurrection)
    BREAKSTONE_TARGET_UNIT
    ASMC(MapAnim_CommonEnd)
    NOFADE
    ENDA
};

bool PostAction_Resurrection(ProcPtr proc)
{

    if (gActionData.unitActionType != UNIT_ACTION_COMBAT)
        return false;

    if (!UNIT_ALIVE(gActiveUnit) || UNIT_STONED(gActiveUnit))
        return false;

#if (defined(SID_Resurrection) && (COMMON_SKILL_VALID(SID_Resurrection)))
    if (SkillTester(GetUnit(gBattleTarget.unit.index), SID_Resurrection) && gBattleTargetGlobalFlag.skill_activated_resurrection)
    {
        KernelCallEvent(EventScr_PostAction_Resurrection, EV_EXEC_CUTSCENE, proc);
        return true;
    }
#endif
    return false;
}
