#include "common-chax.h"
#include "strmag.h"
#include "lvup.h"
#include "bwl.h"
#include "skill-system.h"
#include "weapon-slots.h"
#include "constants/skills.h"
#include "jester_headers/custom-functions.h"

STATIC_DECLAR NOINLINE void ApplyUnitPromotionVanilla(struct Unit *unit, u8 classId, int bonusCaps)
{
	const struct ClassData *promotedClass = GetClassData(classId);
	const struct ClassData *oldClass = unit->pClassData;

	RemapUnitWeaponRanksOnClassChange(unit, oldClass, promotedClass, false);

	// Apply stat ups

	unit->maxHP += promotedClass->promotionHp;
	unit->pow += promotedClass->promotionPow;
	unit->skl += promotedClass->promotionSkl;
	unit->spd += promotedClass->promotionSpd;
	unit->def += promotedClass->promotionDef;
	unit->res += promotedClass->promotionRes;

	unit->level = 1;
	unit->exp   = 0;

	unit->curHP += promotedClass->promotionHp;

    if (bonusCaps == 0)
    {
		if (unit->curHP > GetUnitMaxHp(unit))
			unit->curHP = GetUnitMaxHp(unit);

		if (unit->maxHP > GetUnitMaxStatusHp(unit))
			unit->maxHP = GetUnitMaxStatusHp(unit);

		if (unit->pow > GetUnitMaxStatusPow(unit))
			unit->pow = GetUnitMaxStatusPow(unit);

        if (UNIT_MAG(unit) > GetUnitMaxMagic(unit))
            UNIT_MAG(unit) = GetUnitMaxMagic(unit);

		if (unit->skl > GetUnitMaxStatusSkl(unit))
			unit->skl = GetUnitMaxStatusSkl(unit);

		if (unit->spd > GetUnitMaxStatusSpd(unit))
			unit->spd = GetUnitMaxStatusSpd(unit);

		if (unit->def > GetUnitMaxStatusDef(unit))
			unit->def = GetUnitMaxStatusDef(unit);

		if (unit->res > GetUnitMaxStatusRes(unit))
			unit->res = GetUnitMaxStatusRes(unit);

		if (unit->lck > GetUnitMaxStatusLck(unit))
			unit->lck = GetUnitMaxStatusLck(unit);
	}
}

void GenerateBattleUnitStatGainsComparativelyVanilla(struct BattleUnit *bu, struct Unit *unit)
{
    int bonusCaps = 0;

#if defined(SID_LimitBreaker) && (COMMON_SKILL_VALID(SID_LimitBreaker))
        if (SkillTester(unit, SID_LimitBreaker))
            bonusCaps = SKILL_EFF0(SID_LimitBreaker);
#endif

#if defined(SID_LimitBreakerPlus) && (COMMON_SKILL_VALID(SID_LimitBreakerPlus))
        if (SkillTesterPlus(unit, SID_LimitBreakerPlus))
            bonusCaps = SKILL_EFF0(SID_LimitBreakerPlus);
#endif

    bu->changeHP  = (bu->unit.maxHP + bonusCaps) - unit->maxHP;
    bu->changePow = (bu->unit.pow + bonusCaps)   - unit->pow;
    bu->changeSkl = (bu->unit.skl + bonusCaps)   - unit->skl;
    bu->changeSpd = (bu->unit.spd + bonusCaps)   - unit->spd;
    bu->changeDef = (bu->unit.def + bonusCaps)   - unit->def;
    bu->changeRes = (bu->unit.res + bonusCaps)   - unit->res;
    bu->changeLck = (bu->unit.lck + bonusCaps)   - unit->lck;
    BU_CHG_MAG(bu) = (UNIT_MAG(&bu->unit) + bonusCaps) - UNIT_MAG(unit);

	if (bu->unit.conBonus != unit->conBonus)
		bu->changeCon = bu->unit.conBonus - unit->conBonus;
	else {
		bu->changeCon = 0;
		bu->unit.conBonus = unit->conBonus;
	}
}

LYN_REPLACE_CHECK(ApplyUnitPromotion);
void ApplyUnitPromotion(struct Unit *unit, u8 jid)
{
    int bonusCaps = 0;

#if defined(SID_LimitBreaker) && (COMMON_SKILL_VALID(SID_LimitBreaker))
    if (SkillTester(unit, SID_LimitBreaker))
        bonusCaps = SKILL_EFF0(SID_LimitBreaker);
#endif

#if defined(SID_LimitBreakerPlus) && (COMMON_SKILL_VALID(SID_LimitBreakerPlus))
    if (SkillTesterPlus(unit, SID_LimitBreakerPlus))
        bonusCaps = SKILL_EFF0(SID_LimitBreakerPlus);
#endif

    NewBwlRecordHiddenLevel(unit);
    ApplyUnitPromotionVanilla(unit, jid, bonusCaps);
    TryAddSkillPromotion(unit, jid);

    /* Hooks */
    UNIT_MAG(unit) += GetClassChgMagicBonus(jid);

    if (bonusCaps == 0)
    {
        if (UNIT_MAG(unit) > GetUnitMaxMagic(unit))
            UNIT_MAG(unit) = GetUnitMaxMagic(unit);
    }
}

LYN_REPLACE_CHECK(ApplyUnitDefaultPromotion);
void ApplyUnitDefaultPromotion(struct Unit *unit)
{
	ApplyUnitPromotion(unit, GetClassData(unit->pClassData->promotion)->number);
}

LYN_REPLACE_CHECK(GenerateBattleUnitStatGainsComparatively);
void GenerateBattleUnitStatGainsComparatively(struct BattleUnit *bu, struct Unit *unit)
{
	GenerateBattleUnitStatGainsComparativelyVanilla(bu, unit);

	/* Hooks */
	BU_CHG_MAG(bu) = UNIT_MAG(&bu->unit) - UNIT_MAG(unit);
}
