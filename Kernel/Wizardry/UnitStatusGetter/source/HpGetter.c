#include "common-chax.h"
#include "skill-system.h"
#include "status-getter.h"
#include "constants/skills.h"
#include "bwl.h"
#include "unit-expa.h"
#include "debuff.h"
#include "jester_headers/custom-functions.h"

int _GetUnitMaxHp(struct Unit *unit)
{
	const StatusGetterFunc_t *it;
	int status = unit->maxHP;

#if defined(SID_Unaware) && (COMMON_SKILL_VALID(SID_Unaware))
    if (unit == GetUnit(gBattleActor.unit.index) && GetUnit(gBattleTarget.unit.index) && SkillTester(GetUnit(gBattleTarget.unit.index), SID_Unaware))
        return status;
    else if (unit == GetUnit(gBattleTarget.unit.index) && GetUnit(gBattleActor.unit.index) && SkillTester(GetUnit(gBattleActor.unit.index), SID_Unaware))
        return status;
#endif

	for (it = gpHpGetters; *it; it++)
		status = (*it)(status, unit);

    if (gpExternalHpGetters)
        status = gpExternalHpGetters(status, unit);

    status += HPTonic(unit);
    status += OmniTonic(unit);

    if (GetUnitStatusIndex(unit) == NEW_UNIT_STATUS_HEX) {
        status = status / 2;
    }

#if defined(SID_ManaRush) && (COMMON_SKILL_VALID(SID_ManaRush))
    if (SkillTester(unit, SID_ManaRush))
        status = status / 2;
#endif

	return status;
}

/* Hooks */
int HpGetterWeaponBonus(int status, struct Unit *unit)
{
	u16 weapon = GetUnitEquippedWeapon(unit);

	status += GetItemHpBonus(weapon);
	return status;
}

int HpGetterSkills(int status, struct Unit *unit)
{
    int originalStatus = status;

#if defined(SID_HpBonus) && (COMMON_SKILL_VALID(SID_HpBonus))
	if (SkillTester(unit, SID_HpBonus))
		status += SKILL_EFF0(SID_HpBonus);
#endif

#if defined(SID_Overdraft) && (COMMON_SKILL_VALID(SID_Overdraft))
	if (SkillTester(unit, SID_Overdraft))
		status += unit->maxHP;
#endif

#if defined(SID_Lifefont) && (COMMON_SKILL_VALID(SID_Lifefont))
    if (SkillTester(unit, SID_Lifefont))
    {
        switch (gBmMapTerrain[unit->yPos][unit->xPos]) {
            case TERRAIN_FORT:
            case TERRAIN_THRONE:
                int boost = (GetTerrainHealAmount(gBmMapTerrain[unit->yPos][unit->xPos]) * unit->maxHP / 100);
                status += boost;
                break;
    
            default:
                break;
        }
    }
#endif

#if defined(SID_GoldenGlory) && (COMMON_SKILL_VALID(SID_GoldenGlory))
    if (SkillTester(unit, SID_GoldenGlory))
    {
        const int statBoost = gPlaySt.partyGoldAmount / 20000;
        status += statBoost > 5 ? 5 : statBoost;
    }
#endif

#if defined(SID_Sellsword) && (COMMON_SKILL_VALID(SID_Sellsword))
    if (SkillTester(unit, SID_Sellsword) && CheckBitUES(unit, UES_BIT_SELLSWORD_SKILL_USED))
        status += SKILL_EFF0(SID_Sellsword);
#endif

#if defined(SID_BonusDoubler) && (COMMON_SKILL_VALID(SID_BonusDoubler))
    if (SkillTester(unit, SID_BonusDoubler))
        status += (status - originalStatus);
#endif

	return status;
}
