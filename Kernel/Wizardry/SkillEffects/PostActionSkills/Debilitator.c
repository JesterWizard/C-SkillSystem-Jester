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

void PostAction_Debilitator(ProcPtr parent)
{
    if (gActionData.unitActionType != UNIT_ACTION_COMBAT)
        return;

#if defined(SID_Debilitator) && (COMMON_SKILL_VALID(SID_Debilitator))
    if (SkillListTester(gActiveUnit, SID_Debilitator) && gBattleActorGlobalFlag.skill_activated_debilitator)
        SetUnitStatDebuff(GetUnit(gActionData.targetIndex), UNIT_STAT_DEBUFF_DEBILITATOR);
    else if (SkillTester(GetUnit(gActionData.targetIndex), SID_Debilitator) && gBattleActorGlobalFlag.skill_activated_debilitator)
        SetUnitStatDebuff(gActiveUnit, UNIT_STAT_DEBUFF_DEBILITATOR);
#endif
}
