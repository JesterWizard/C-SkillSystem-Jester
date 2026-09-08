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

STATIC_DECLAR const EventScr EventScr_PostAction_Undying[] = {
    BREAKSTONE_CHOSEN_UNIT
    ASMC(MapAnim_CommonEnd)
    NOFADE
    ENDA
};

bool PostAction_Undying(ProcPtr proc)
{

    if (gActionData.unitActionType != UNIT_ACTION_COMBAT)
        return false;

    struct Unit * targetUnit = GetUnit(gBattleTarget.unit.index);
    struct Unit * actorUnit = GetUnit(gBattleActor.unit.index);

    if (targetUnit->curHP != 0 && actorUnit->curHP != 0)
        return false;

    if (gPlaySt.chapterTurnNumber != 1)
        return false;

#if (defined(SID_Undying) && (COMMON_SKILL_VALID(SID_Undying)))
    
    if (SkillTester(targetUnit, SID_Undying))
    {
        ClearFlag(EVFLAG_GAMEOVER);
        gEventSlots[EVT_SLOT_2] = targetUnit->pCharacterData->number;
        AddUnitHp(targetUnit, (targetUnit->maxHP - targetUnit->curHP));
        KernelCallEvent(EventScr_PostAction_Undying, EV_EXEC_CUTSCENE, proc);
        return true;
    }
    else if (SkillTester(actorUnit, SID_Undying))
    {
        ClearFlag(EVFLAG_GAMEOVER);
        gEventSlots[EVT_SLOT_2] = actorUnit->pCharacterData->number;
        AddUnitHp(actorUnit, (actorUnit->maxHP - actorUnit->curHP));   
        KernelCallEvent(EventScr_PostAction_Undying, EV_EXEC_CUTSCENE, proc);
        return true;
    }
#endif
    return false;
}
