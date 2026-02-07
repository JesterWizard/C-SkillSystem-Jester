#include "common-chax.h"
#include "strmag.h"
#include "lvup.h"
#include "skill-system.h"
#include "constants/skills.h"
#include "kernel-lib.h"
#include "debuff.h"
#include "jester_headers/class-pairs.h"
#include "jester_headers/custom-functions.h"
#include "status-getter.h"

STATIC_DECLAR void CheckBattleUnitStatCapsVanilla(struct Unit *unit, struct BattleUnit *bu)
{
    int limitBreaker = 0;

    int UNIT_POW_MAX = UNIT_POW_MAX(unit);
    int UNIT_MAG_MAX = GetUnitMaxMagic(unit);
    int UNIT_SKL_MAX = UNIT_SKL_MAX(unit);
    int UNIT_SPD_MAX = UNIT_SPD_MAX(unit);
    int UNIT_DEF_MAX = UNIT_DEF_MAX(unit);
    int UNIT_RES_MAX = UNIT_RES_MAX(unit);
    int UNIT_LCK_MAX = UNIT_LCK_MAX(unit);

#if defined(SID_LimitBreaker) && (COMMON_SKILL_VALID(SID_LimitBreaker))
    if (SkillTester(unit, SID_LimitBreaker))
        limitBreaker = SKILL_EFF0(SID_LimitBreaker);
#endif

#if defined(SID_LimitBreakerPlus) && (COMMON_SKILL_VALID(SID_LimitBreakerPlus))
    if (SkillTesterPlus(unit, SID_LimitBreakerPlus))
        limitBreaker = SKILL_EFF0(SID_LimitBreakerPlus);
#endif

#if defined(SID_OgreBody) && (COMMON_SKILL_VALID(SID_OgreBody))
    if (SkillTester(unit, SID_OgreBody))
    {
        if (unit->maxHP > SKILL_EFF0(SID_OgreBody))
            unit->maxHP = SKILL_EFF0(SID_OgreBody);
    }
    else
    {
        if (unit->maxHP > UNIT_MHP_MAX(unit) + limitBreaker)
            unit->maxHP = UNIT_MHP_MAX(unit) + limitBreaker;
    }
#endif

#if defined(SID_Dismount) && (COMMON_SKILL_VALID(SID_Dismount))
    if (SkillTester(unit, SID_Dismount) && GetUnitStatusIndex(unit) == NEW_UNIT_STATUS_DISMOUNT)
    {
        for (int i = 0; i < (int)ARRAY_COUNT(dismountPairs); i++)
        {
            if (gActiveUnit->pClassData->number == dismountPairs[i][1])
            {
                const struct ClassData * mountedClass = GetClassData(dismountPairs[i][0]);
                UNIT_POW_MAX = mountedClass->maxPow;
                UNIT_SKL_MAX = mountedClass->maxSkl;
                UNIT_SPD_MAX = mountedClass->maxSpd;
                UNIT_DEF_MAX = mountedClass->maxDef;
                UNIT_RES_MAX = mountedClass->maxRes;
                UNIT_LCK_MAX = MAX_LUCK;
                UNIT_MAG_MAX = gpMagicJInfos[mountedClass->number].cap;
                break;
            }
        }
    }
#endif

    if (unit->maxHP + bu->changeHP > UNIT_MHP_MAX(unit) + limitBreaker)
        bu->changeHP = (UNIT_MHP_MAX(unit) + limitBreaker) - unit->maxHP;

    if ((unit->pow + bu->changePow) > UNIT_POW_MAX + limitBreaker)
        bu->changePow = (UNIT_POW_MAX  + limitBreaker) - unit->pow;

    if ((UNIT_MAG(unit) + BU_CHG_MAG(bu)) > UNIT_MAG_MAX + limitBreaker)
        BU_CHG_MAG(bu) = (UNIT_MAG_MAX + limitBreaker) - UNIT_MAG(unit);

    if ((unit->skl + bu->changeSkl) > UNIT_SKL_MAX + limitBreaker)
        bu->changeSkl = (UNIT_SKL_MAX + limitBreaker) - unit->skl;

    if ((unit->spd + bu->changeSpd) > UNIT_SPD_MAX + limitBreaker)
        bu->changeSpd = (UNIT_SPD_MAX + limitBreaker) - unit->spd;

    if ((unit->def + bu->changeDef) > UNIT_DEF_MAX + limitBreaker)
        bu->changeDef = (UNIT_DEF_MAX + limitBreaker) - unit->def;

    if ((unit->res + bu->changeRes) > UNIT_RES_MAX + limitBreaker)
        bu->changeRes = (UNIT_RES_MAX + limitBreaker) - unit->res;

    if ((unit->lck + bu->changeLck) > UNIT_LCK_MAX + limitBreaker)
        bu->changeLck = (UNIT_LCK_MAX + limitBreaker) - unit->lck;
}

STATIC_DECLAR void UnitCheckStatCapsVanilla(struct Unit *unit)
{
    int limitBreaker = 0;

    int UNIT_POW_MAX = UNIT_POW_MAX(unit);
    int UNIT_MAG_MAX = GetUnitMaxMagic(unit);
    int UNIT_SKL_MAX = UNIT_SKL_MAX(unit);
    int UNIT_SPD_MAX = UNIT_SPD_MAX(unit);
    int UNIT_DEF_MAX = UNIT_DEF_MAX(unit);
    int UNIT_RES_MAX = UNIT_RES_MAX(unit);
    int UNIT_LCK_MAX = UNIT_LCK_MAX(unit);
    int UNIT_CON_MAX = UNIT_CON_MAX(unit);
    int UNIT_MOV_MAX = UNIT_MOV_MAX(unit);

    int UNIT_CON_BASE = UNIT_CON_BASE(unit);
    int UNIT_MOV_BASE = UNIT_MOV_BASE(unit);

#if defined(SID_LimitBreaker) && (COMMON_SKILL_VALID(SID_LimitBreaker))
    if (SkillTester(unit, SID_LimitBreaker) && limitBreaker == 0)
        limitBreaker = SKILL_EFF0(SID_LimitBreaker);
#endif

#if defined(SID_LimitBreakerPlus) && (COMMON_SKILL_VALID(SID_LimitBreakerPlus))
    if (SkillTesterPlus(unit, SID_LimitBreakerPlus))
        limitBreaker = SKILL_EFF0(SID_LimitBreakerPlus);
#endif

/* Having an issue with HP maxing out with this turned on right now */
// #if defined(SID_OgreBody) && (COMMON_SKILL_VALID(SID_OgreBody))
//     if (SkillTester(unit, SID_OgreBody))
//     {
//         if (unit->maxHP > SKILL_EFF0(SID_OgreBody))
//             unit->maxHP = SKILL_EFF0(SID_OgreBody);
//     }
//     else
//     {
//         if (unit->maxHP > UNIT_MHP_MAX(unit) + limitBreaker)
//             unit->maxHP = UNIT_MHP_MAX(unit) + limitBreaker;
//     }
// #endif

//     if (unit->maxHP > UNIT_MHP_MAX(unit) + limitBreaker)
//         unit->maxHP = UNIT_MHP_MAX(unit) + limitBreaker;

#if defined(SID_Dismount) && (COMMON_SKILL_VALID(SID_Dismount))
    if (SkillTester(unit, SID_Dismount) && GetUnitStatusIndex(unit) == NEW_UNIT_STATUS_DISMOUNT)
    {
        for (int i = 0; i < (int)ARRAY_COUNT(dismountPairs); i++)
        {
            if (gActiveUnit->pClassData->number == dismountPairs[i][1])
            {
                const struct ClassData * mountedClass = GetClassData(dismountPairs[i][0]);
                UNIT_POW_MAX = mountedClass->maxPow;
                UNIT_SKL_MAX = mountedClass->maxSkl;
                UNIT_SPD_MAX = mountedClass->maxSpd;
                UNIT_DEF_MAX = mountedClass->maxDef;
                UNIT_RES_MAX = mountedClass->maxRes;
                UNIT_LCK_MAX = MAX_LUCK;
                UNIT_MAG_MAX = gpMagicJInfos[mountedClass->number].cap;
                break;
            }
        }
    }
#endif

    if (unit->pow > UNIT_POW_MAX + limitBreaker)
        unit->pow = UNIT_POW_MAX + limitBreaker;

    if (UNIT_MAG(unit) > UNIT_MAG_MAX + limitBreaker)
        UNIT_MAG(unit) = UNIT_MAG_MAX + limitBreaker;

    if (unit->skl > UNIT_SKL_MAX + limitBreaker)
        unit->skl = UNIT_SKL_MAX + limitBreaker;

    if (unit->spd > UNIT_SPD_MAX + limitBreaker)
        unit->spd = UNIT_SPD_MAX + limitBreaker;

    if (unit->def > UNIT_DEF_MAX + limitBreaker)
        unit->def = UNIT_DEF_MAX + limitBreaker;

    if (unit->res > UNIT_RES_MAX + limitBreaker)
        unit->res = UNIT_RES_MAX + limitBreaker;

    if (unit->lck > UNIT_LCK_MAX + limitBreaker)
        unit->lck = UNIT_LCK_MAX + limitBreaker;

    if (unit->conBonus > (UNIT_CON_MAX - UNIT_CON_BASE))
        unit->conBonus = (UNIT_CON_MAX - UNIT_CON_BASE);

    if (unit->movBonus > (UNIT_MOV_MAX - UNIT_MOV_BASE))
        unit->movBonus = (UNIT_MOV_MAX - UNIT_MOV_BASE);
}

LYN_REPLACE_CHECK(CheckBattleUnitStatCaps);
void CheckBattleUnitStatCaps(struct Unit *unit, struct BattleUnit *bu)
{
	CheckBattleUnitStatCapsVanilla(unit, bu);

	/* Hooks */
	if ((UNIT_MAG(unit) + BU_CHG_MAG(bu)) > GetUnitMaxStatusMag(unit))
		BU_CHG_MAG(bu) = GetUnitMaxStatusMag(unit) - UNIT_MAG(unit);
}

#if 0
LYN_UNUSED_REPLACE_CHECK(UnitCheckStatCaps);
void UnitCheckStatCaps(struct Unit *unit)
#else
/* External hook to save spaces */
void _UnitCheckStatCaps(struct Unit *unit)
#endif
{
	UnitCheckStatCapsVanilla(unit);

    int limitBreaker = 0;

#if defined(SID_LimitBreaker) && (COMMON_SKILL_VALID(SID_LimitBreaker))
    if (SkillTester(unit, SID_LimitBreaker) && limitBreaker == 0)
        limitBreaker = SKILL_EFF0(SID_LimitBreaker);
#endif

#if defined(SID_LimitBreakerPlus) && (COMMON_SKILL_VALID(SID_LimitBreakerPlus))
    if (SkillTesterPlus(unit, SID_LimitBreakerPlus))
        limitBreaker = SKILL_EFF0(SID_LimitBreakerPlus);
#endif

	/* Hooks */
	if (UNIT_MAG(unit) > GetUnitMaxStatusMag(unit) + limitBreaker)
		UNIT_MAG(unit) = GetUnitMaxStatusMag(unit) + limitBreaker;
}
