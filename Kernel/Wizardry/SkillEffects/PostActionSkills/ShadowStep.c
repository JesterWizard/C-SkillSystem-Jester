#include "common-chax.h"
#include "debuff.h"
#include "kernel-lib.h"
#include "skill-system.h"
#include "battle-system.h"
#include "event-rework.h"
#include "constants/skills.h"
#include "constants/texts.h"
#include "strmag.h"

FORCE_DECLARE static void callback_anim(ProcPtr proc)
{
    PlaySoundEffect(0x269);
    Proc_StartBlocking(ProcScr_DanceringAnim, proc);

    BG_SetPosition(
        BG_0,
        -SCREEN_TILE_IX(gActiveUnit->xPos - 1),
        -SCREEN_TILE_IX(gActiveUnit->yPos - 2));
}

FORCE_DECLARE static void callback_exec(ProcPtr proc)
{
    SetUnitStatus(gActiveUnit, NEW_UNIT_STATUS_HIDE);
}

bool PostAction_ShadowStep(ProcPtr proc)
{
#if defined(SID_ShadowStep) && (COMMON_SKILL_VALID(SID_ShadowStep))
    if (gActionData.unitActionType != UNIT_ACTION_COMBAT)
        return false;

    if (!UNIT_ALIVE(gActiveUnit) || UNIT_STONED(gActiveUnit))
        return false;

    if (gBattleActorGlobalFlag.enemy_defeated == false)
        return false;

#if defined(SID_ShadowStep) && (COMMON_SKILL_VALID(SID_ShadowStep))
    if (SkillListTester(gActiveUnit, SID_ShadowStep))
    {
        NewMuSkillAnimOnActiveUnit(gActionData.unk08, callback_anim, callback_exec);
        return true;
    }
#endif

#endif
    return false;
}