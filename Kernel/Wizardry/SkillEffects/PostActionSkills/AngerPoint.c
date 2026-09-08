#include "common-chax.h"
#include "debuff.h"
#include "status-getter.h"
#include "action-expa.h"
#include "unit-expa.h"
#include "skill-system.h"
#include "battle-system.h"
#include "combat-art.h"
#include "constants/skills.h"
#include "constants/combat-arts.h"

void PostAction_AngerPoint(ProcPtr parent)
{
    if (gActionData.unitActionType != UNIT_ACTION_COMBAT)
        return;

#if defined(SID_AngerPoint) && (COMMON_SKILL_VALID(SID_AngerPoint))
    if (SkillTester(gActiveUnit, SID_AngerPoint) && gBattleActorGlobalFlag.skill_activated_angerpoint)
        SetUnitStatDebuff(gActiveUnit, UNIT_STAT_BUFF_ANGERPOINT);
    else if (SkillTester(GetUnit(gActionData.targetIndex), SID_AngerPoint) && gBattleActorGlobalFlag.skill_activated_angerpoint)
        SetUnitStatDebuff(GetUnit(gActionData.targetIndex), UNIT_STAT_BUFF_ANGERPOINT);
#endif
}
