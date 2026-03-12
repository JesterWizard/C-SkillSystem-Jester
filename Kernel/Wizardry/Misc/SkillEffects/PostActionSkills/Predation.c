#include "common-chax.h"
#include "debuff.h"
#include "kernel-lib.h"
#include "skill-system.h"
#include "battle-system.h"
#include "event-rework.h"
#include "constants/skills.h"
#include "constants/texts.h"
#include "strmag.h"
#include "jester_headers/custom-functions.h"

#ifdef CONFIG_TURN_ON_ALL_SKILLS
    // Choose the proper scroll index based on the high byte of the skill id.
    #define GET_SKILL_SCROLL_INDEX(sid) (((sid) > 0x2FF) ? CONFIG_ITEM_INDEX_SKILL_SCROLL_4 : \
                                        (((sid) > 0x1FF) ? CONFIG_ITEM_INDEX_SKILL_SCROLL_3 : \
                                        (((sid) > 0x0FF) ? CONFIG_ITEM_INDEX_SKILL_SCROLL_2 : \
                                                           CONFIG_ITEM_INDEX_SKILL_SCROLL_1)))
#else
    #ifdef  CONFIG_ITEM_INDEX_SKILL_SCROLL_1
        #define GET_SKILL_SCROLL_INDEX(sid) CONFIG_ITEM_INDEX_SKILL_SCROLL_1
    #else
        #define GET_SKILL_SCROLL_INDEX(sid) 200 // Ignore this line, it's just filler
    #endif
#endif

#if defined(SID_Predation) && (COMMON_SKILL_VALID(SID_Predation))
static void callback_anim(ProcPtr proc)
{

}
#endif 

#if defined(SID_PredationPlus) && (COMMON_SKILL_VALID(SID_PredationPlus))
static void callback_exec_predationPlus(ProcPtr proc)
{
    struct Unit * targetUnit = GetUnit(gBattleTarget.unit.index);

    /* If the target unit's first learned skill slot is empty, we can assume they have no skills learned */
    if (UNIT_RAM_SKILLS(targetUnit)[0] == 0)
        return;

    Proc_StartBlocking(ProcScr_PredationPlusSoftLock, proc);
}
#endif 

#if defined(SID_Predation) && (COMMON_SKILL_VALID(SID_Predation))
static void callback_exec_predation(ProcPtr proc)
{
    struct Unit * targetUnit = GetUnit(gBattleTarget.unit.index);

    /* If the target unit's first learned skill slot is empty, we can assume they have no skills learned */
    if (UNIT_RAM_SKILLS(targetUnit)[0] == 0)
        return;

     /* The active unit has space for an additional skill */
    if (UNIT_RAM_SKILLS(gActiveUnit)[UNIT_RAM_SKILLS_LEN-1] == 0)
    {
        /* The first learned skill in the target unit's struct */
        AddSkill(gActiveUnit, GET_SKILL(targetUnit, 0));
        SetPopupItem(((targetUnit->supports[0] & 0xFF) << 8) | GET_SKILL_SCROLL_INDEX(targetUnit->supports[0]));
        NewPopup_Simple(PopupScr_LearnSkill, 0x5A, 0, proc);
    }
    else
    {
        SetPopupItem(((targetUnit->supports[0] & 0xFF) << 8) | GET_SKILL_SCROLL_INDEX(targetUnit->supports[0]));
        NewPopup_Simple(PopupScr_ObtainedSkill, 0x5A, 0, proc);
        Proc_StartBlocking(ProcScr_PredationSoftLock, proc);
    }
}
#endif

bool PostAction_Predation(ProcPtr proc)
{
    if (gActionData.unitActionType != UNIT_ACTION_COMBAT)
        return false;

    if (!UNIT_ALIVE(gActiveUnit) || UNIT_STONED(gActiveUnit))
        return false;

    if (gBattleActorGlobalFlag.enemy_defeated == false)
        return false;

#if defined(SID_PredationPlus) && (COMMON_SKILL_VALID(SID_PredationPlus))
        if (SkillTesterPlus(gActiveUnit, SID_PredationPlus))
        {
            NewMuSkillAnimOnActiveUnit(gActionData.unk08, callback_anim, callback_exec_predationPlus);
            return true;
        }
#endif

#if defined(SID_Predation) && (COMMON_SKILL_VALID(SID_Predation))
        if (SkillListTester(gActiveUnit, SID_Predation))
        {
            NewMuSkillAnimOnActiveUnit(gActionData.unk08, callback_anim, callback_exec_predation);
            return true;
        }
#endif

    return false;
}
