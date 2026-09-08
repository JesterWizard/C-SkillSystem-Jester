#include "common-chax.h"
#include "debuff.h"
#include "kernel-lib.h"
#include "skill-system.h"
#include "battle-system.h"
#include "event-rework.h"
#include "constants/skills.h"
#include "constants/texts.h"
#include "strmag.h"

STATIC_DECLAR void PostAction_NecroCopySkipMenuIfNotAlly(struct EventEngineProc * proc)
{
    ShowUnitSprite(gActiveUnit);

    gEventSlots[EVT_SLOT_C] = false;
    if (UNIT_FACTION(gActiveUnit) == FACTION_BLUE)
        gEventSlots[EVT_SLOT_C] = true;
}

STATIC_DECLAR void PrepareNecroCopy(void)
{
    struct Unit* targetUnit = GetUnit(gBattleTarget.unit.index);

    // --- Copy stats ---
    gActiveUnit->pClassData = targetUnit->pClassData;
    gActiveUnit->maxHP = targetUnit->maxHP;
    gActiveUnit->curHP = targetUnit->maxHP;
    gActiveUnit->pow = targetUnit->pow;
    UNIT_MAG(gActiveUnit) = UNIT_MAG(targetUnit);
    gActiveUnit->skl = targetUnit->skl;
    gActiveUnit->spd = targetUnit->spd;
    gActiveUnit->lck = targetUnit->lck;
    gActiveUnit->def = targetUnit->def;
    gActiveUnit->res = targetUnit->res;

    // --- Copy weapon ranks ---
    for (int i = 0; i < UNIT_ITEM_COUNT; ++i)
        gActiveUnit->ranks[i] = targetUnit->ranks[i];

    // --- Copy items ---
    for (int i = 0; i < UNIT_ITEM_COUNT; ++i)
        gActiveUnit->items[i] = targetUnit->items[i];

    // --- Find NecroCopy skill slot in active unit ---
    int necroIndex = -1;
    for (int i = 0; i < UNIT_RAM_SKILLS_LEN; ++i)
    {
        if (GET_SKILL(gActiveUnit, i) == SID_NecroCopy)
        {
            necroIndex = i;
            break;
        }
    }

    // --- Copy target’s skills, skipping NecroCopy slot ---
    for (int i = 0; i < UNIT_RAM_SKILLS_LEN; ++i)
    {
        if (i == necroIndex)
            continue; // Skip overwriting the NecroCopy skill

        SET_SKILL(gActiveUnit, i, GET_SKILL(targetUnit, i));
    }
}

STATIC_DECLAR const EventScr EventScr_PostAction_NecroCopy[] = {
    EVBIT_MODIFY(0x4)
    ASMC(PostAction_NecroCopySkipMenuIfNotAlly)
    BEQ(0, EVT_SLOT_C, EVT_SLOT_0)

    TUTORIALTEXTBOXSTART
    SVAL(EVT_SLOT_B, 0xffffffff)
    TEXTSHOW(MSG_SkillEffect_NecroCopy_Confirm)
    TEXTEND
    REMA
    SVAL(EVT_SLOT_7, 0x1)
    BNE(99, EVT_SLOT_C, EVT_SLOT_7)

LABEL(0)
#if defined(SID_NecroCopy) && (COMMON_SKILL_VALID(SID_NecroCopy))
    SVAL(EVT_SLOT_B, SID_NecroCopy)
    CALL(EventScr_MuSkillAnim)
#endif
    ASMC(PrepareNecroCopy)

LABEL(99)
    ASMC(MapAnim_CommonEnd)
    NOFADE
    ENDA
};

bool PostAction_NecroCopy(ProcPtr proc)
{
    if (gActionData.unitActionType != UNIT_ACTION_COMBAT)
        return false;

    if (!UNIT_ALIVE(gActiveUnit) || UNIT_STONED(gActiveUnit))
        return false;

#if defined(SID_NecroCopy) && (COMMON_SKILL_VALID(SID_NecroCopy))
    if (!SkillListTester(gActiveUnit, SID_NecroCopy))
#else
    if (1)
#endif
        return false;

    if (gBattleActorGlobalFlag.enemy_defeated == false)
        return false;

    KernelCallEvent(EventScr_PostAction_NecroCopy, EV_EXEC_CUTSCENE, proc);

    return true;
}
