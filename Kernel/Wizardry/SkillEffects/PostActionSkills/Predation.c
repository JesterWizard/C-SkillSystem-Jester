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
    #define GET_SKILL_SCROLL_INDEX(sid) (((sid) > 0x2FF) ? CONFIG_ITEM_INDEX_SKILL_SCROLL_4 : \
                                        (((sid) > 0x1FF) ? CONFIG_ITEM_INDEX_SKILL_SCROLL_3 : \
                                        (((sid) > 0x0FF) ? CONFIG_ITEM_INDEX_SKILL_SCROLL_2 : \
                                                           CONFIG_ITEM_INDEX_SKILL_SCROLL_1)))
    #define GET_SKILL_SCROLL_VALUE(sid) (((sid) > 0x2FF) ? ((sid) - 0x2FF) : \
                                        (((sid) > 0x1FF) ? ((sid) - 0x1FF) : \
                                        (((sid) > 0x0FF) ? ((sid) - 0x0FF) : \
                                                           (sid))))
#else
    #ifdef  CONFIG_ITEM_INDEX_SKILL_SCROLL_1
        #define GET_SKILL_SCROLL_INDEX(sid) CONFIG_ITEM_INDEX_SKILL_SCROLL_1
        #define GET_SKILL_SCROLL_VALUE(sid) (sid)
    #else
        #define GET_SKILL_SCROLL_INDEX(sid) 200 // Ignore this line, it's just filler
        #define GET_SKILL_SCROLL_VALUE(sid) (sid)
    #endif
#endif

static u16 MakeSkillScrollPopupItem(u16 sid)
{
    return ((GET_SKILL_SCROLL_VALUE(sid) & 0xFF) << 8) | GET_SKILL_SCROLL_INDEX(sid);
}

#if defined(SID_Predation) && (COMMON_SKILL_VALID(SID_Predation))
static void callback_anim(ProcPtr proc)
{

}
#endif 

#if defined(SID_PredationPlus) && (COMMON_SKILL_VALID(SID_PredationPlus))
static void callback_exec_predationPlus(ProcPtr proc)
{
    struct Unit * targetUnit = GetUnit(gBattleTarget.unit.index);

    if (!UNIT_IS_VALID(targetUnit) || !EQUIP_SKILL_VALID(GET_SKILL(targetUnit, 0)))
        return;

    Proc_StartBlocking(ProcScr_PredationPlusSoftLock, proc);
}
#endif 

#if defined(SID_Predation) && (COMMON_SKILL_VALID(SID_Predation))
static void callback_exec_predation(ProcPtr proc)
{
    struct Unit * targetUnit = GetUnit(gBattleTarget.unit.index);
    u16 sid;

    if (!UNIT_IS_VALID(targetUnit))
        return;

    sid = GET_SKILL(targetUnit, 0);
    if (!EQUIP_SKILL_VALID(sid))
        return;

    SetPopupUnit(gActiveUnit);

    /* The active unit has space for an additional skill */
    if (!EQUIP_SKILL_VALID(GET_SKILL(gActiveUnit, UNIT_RAM_SKILLS_LEN - 1)))
    {
        AddSkill(gActiveUnit, sid);
        SetPopupItem(sid);
        NewPopup_Simple(PopupScr_LearnSkill, 0x5A, 0, proc);
    }
    else
    {
        SetPopupItem(MakeSkillScrollPopupItem(sid));
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
            NewMuSkillAnimOnActiveUnit(SID_PredationPlus, callback_anim, callback_exec_predationPlus);
            return true;
        }
#endif

#if defined(SID_Predation) && (COMMON_SKILL_VALID(SID_Predation))
        if (SkillListTester(gActiveUnit, SID_Predation))
        {
            NewMuSkillAnimOnActiveUnit(SID_Predation, callback_anim, callback_exec_predation);
            return true;
        }
#endif

    return false;
}
