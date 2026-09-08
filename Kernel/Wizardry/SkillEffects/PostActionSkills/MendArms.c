#include "common-chax.h"
#include "debuff.h"
#include "kernel-lib.h"
#include "skill-system.h"
#include "battle-system.h"
#include "event-rework.h"
#include "constants/skills.h"
#include "constants/texts.h"
#include "strmag.h"

bool PostAction_MendArms(ProcPtr proc)
{
#if defined(SID_MendArms) && COMMON_SKILL_VALID(SID_MendArms)
    if (gActionData.unitActionType != UNIT_ACTION_COMBAT)
        return false;

    struct Unit *actorUnit  = GetUnit(gBattleActor.unit.index);
    struct Unit *targetUnit = GetUnit(gBattleTarget.unit.index);

    struct Unit *survivor = NULL;
    int weaponUsesExpended;

    // Actor kills target
    if (UNIT_ALIVE(actorUnit) && !UNIT_ALIVE(targetUnit))
    {
        if (!BattleFastSkillTester(&gBattleActor, SID_MendArms))
            return false;

        weaponUsesExpended = GetItemUses(gBattleActor.weaponBefore) - GetItemUses(gBattleActor.weapon);
        survivor = actorUnit;
    }
    // Target kills actor
    else if (UNIT_ALIVE(targetUnit) && !UNIT_ALIVE(actorUnit))
    {
        if (!BattleFastSkillTester(&gBattleTarget, SID_MendArms))
            return false;

        weaponUsesExpended = GetItemUses(gBattleTarget.weaponBefore) - GetItemUses(gBattleTarget.weapon);
        survivor = targetUnit;
    }
    else
    {
        return false;
    }

    // Must be exactly one use expended
    if (weaponUsesExpended != 1)
        return false;

    // Repair usable weapons by +1 use
    for (int i = 0; i < 5; i++)
    {
        int item = survivor->items[i];
        if (!item)
            continue;

        int itemIndex = GetItemIndex(item);

        if (!CanUnitUseWeapon(survivor, itemIndex))
            continue;

        int uses    = GetItemUses(item);
        int maxUses = GetItemMaxUses(itemIndex);

        if (uses < maxUses)
            survivor->items[i] = item + (1 << 8);
    }
#endif

    return true;
}