#include "common-chax.h"
#include "skill-system.h"
#include "battle-system.h"
#include "bwl.h"
#include "constants/skills.h"
#include "debuff.h"
#include "strmag.h"

#ifdef CONFIG_MODULAR_STAFF_EXP
inline static int StaffEXP(int weapon)
{
    int exp = 0;

    switch (weapon)
    {
    case ITEM_STAFF_HEAL:
    case ITEM_STAFF_TORCH:
        exp = 15;
        break;
#ifdef CONFIG_ITEM_INDEX_FORCE_STAFF
    case CONFIG_ITEM_INDEX_FORCE_STAFF:
#endif
#ifdef CONFIG_ITEM_INDEX_TEMPEST_STAFF
    case CONFIG_ITEM_INDEX_TEMPEST_STAFF:
#endif
#ifdef CONFIG_ITEM_INDEX_ACUITY_STAFF
    case CONFIG_ITEM_INDEX_ACUITY_STAFF:
#endif
#ifdef CONFIG_ITEM_INDEX_SPRINT_STAFF
    case CONFIG_ITEM_INDEX_SPRINT_STAFF:
#endif
#ifdef CONFIG_ITEM_INDEX_FORTUNE_STAFF
    case CONFIG_ITEM_INDEX_FORTUNE_STAFF:
#endif
#ifdef CONFIG_ITEM_INDEX_IRON_STAFF
    case CONFIG_ITEM_INDEX_IRON_STAFF:
#endif
#ifdef CONFIG_ITEM_INDEX_OMNI_STAFF
    case CONFIG_ITEM_INDEX_OMNI_STAFF:
#endif
#ifdef CONFIG_ITEM_INDEX_POISON_STAFF
    case CONFIG_ITEM_INDEX_POISON_STAFF:
#endif
#ifdef CONFIG_ITEM_INDEX_DELAY_STAFF
    case CONFIG_ITEM_INDEX_DELAY_STAFF:
#endif
#ifdef CONFIG_ITEM_INDEX_INVEST_STAFF
    case CONFIG_ITEM_INDEX_INVEST_STAFF:
#endif
    case ITEM_STAFF_BARRIER:
        exp = 17;
        break;
#ifdef CONFIG_ITEM_INDEX_MINE_STAFF
    case CONFIG_ITEM_INDEX_MINE_STAFF:
#endif
    case ITEM_STAFF_MEND:
        exp = 20;
        break;
    case ITEM_STAFF_UNLOCK:
    case ITEM_STAFF_RESTORE:
    case ITEM_STAFF_RECOVER:
        exp = 25;
        break;
    case ITEM_STAFF_BERSERK:
    case ITEM_STAFF_SLEEP:
    case ITEM_STAFF_SILENCE:
    case ITEM_STAFF_PHYSIC:
#ifdef CONFIG_ITEM_INDEX_RUNE_STAFF
    case CONFIG_ITEM_INDEX_RUNE_STAFF:
#endif
#ifdef CONFIG_ITEM_INDEX_SLOW_STAFF
    case CONFIG_ITEM_INDEX_SLOW_STAFF:
#endif
#ifdef CONFIG_ITEM_INDEX_FORGE_STAFF
    case CONFIG_ITEM_INDEX_FORGE_STAFF:
#endif
#ifdef CONFIG_ITEM_INDEX_REWARP_STAFF
    case CONFIG_ITEM_INDEX_REWARP_STAFF:
#endif
#ifdef CONFIG_ITEM_INDEX_QUICKEN_STAFF
    case CONFIG_ITEM_INDEX_QUICKEN_STAFF:
#endif
#ifdef CONFIG_ITEM_INDEX_HIDE_STAFF
    case CONFIG_ITEM_INDEX_HIDE_STAFF:
#endif
#ifdef CONFIG_ITEM_INDEX_PROVOKE_STAFF
    case CONFIG_ITEM_INDEX_PROVOKE_STAFF:
#endif
#ifdef CONFIG_ITEM_INDEX_PETRIFY_STAFF
    case CONFIG_ITEM_INDEX_PETRIFY_STAFF:
#endif
#ifdef CONFIG_ITEM_INDEX_SOOTH_STAFF
    case CONFIG_ITEM_INDEX_SOOTH_STAFF:
#endif
#ifdef CONFIG_ITEM_INDEX_ENFEEBLE_STAFF
    case CONFIG_ITEM_INDEX_ENFEEBLE_STAFF:
#endif
        exp = 30;
        break;
    case ITEM_STAFF_RESCUE:
#ifdef CONFIG_ITEM_INDEX_ENTRAP_STAFF
    case CONFIG_ITEM_INDEX_ENTRAP_STAFF:
#endif
        exp = 50;
        break;
    case ITEM_STAFF_WARP:
        exp = 60;
        break;
    case ITEM_STAFF_REPAIR:
    case ITEM_STAFF_FORTIFY:
        exp = 70;
        break;
    case ITEM_STAFF_LATONA:
        exp = 100;
        break;

    }
    return exp;
}
#endif

LYN_REPLACE_CHECK(GetUnitExpLevel);
int GetUnitExpLevel(struct Unit* unit)
{
    int base, bonus;

    base = unit->level;

    if (CheckHasBwl(UNIT_CHAR_ID(unit)))
        bonus = GetUnitHiddenLevel(unit);
    else
        bonus = gpClassPreLoadHiddenLevel[UNIT_CLASS_ID(unit)];

    return base + bonus;
}

STATIC_DECLAR int KernelModifyBattleUnitExp(int base, struct BattleUnit* actor, struct BattleUnit* target)
{
    int status = base;

#if defined(SID_Blossom) && (COMMON_SKILL_VALID(SID_Blossom))
    if (BattleFastSkillTester(actor, SID_Blossom))
        status = status / 2;
#endif

#if defined(SID_Paragon) && (COMMON_SKILL_VALID(SID_Paragon))
    if (BattleFastSkillTester(actor, SID_Paragon))
        status = status * 2;
#endif

#if defined(SID_Mentorship) && (COMMON_SKILL_VALID(SID_Mentorship))
    if (BattleFastSkillTester(actor, SID_Mentorship))
        status = status + Div(status * SKILL_EFF0(SID_Mentorship), 100);
    else
    {
        for (int i = 0; i < ARRAY_COUNT_RANGE2x2; i++)
        {
            int _x = actor->unit.xPos + gVecs_2x2[i].x;
            int _y = actor->unit.yPos + gVecs_2x2[i].y;

            struct Unit* unit_ally = GetUnitAtPosition(_x, _y);

            if (!UNIT_IS_VALID(unit_ally))
                continue;

            if (unit_ally->state & (US_HIDDEN | US_DEAD | US_RESCUED | US_BIT16))
                continue;

            if (!AreUnitsAllied(actor->unit.index, unit_ally->index))
                continue;

            if (SkillTester(unit_ally, SID_Mentorship))
            {
                status = status * Div(status * SKILL_EFF0(SID_Mentorship), 100);;
                break;
            }
        }
    }
#endif

#if defined(SID_StaffParagon) && (COMMON_SKILL_VALID(SID_StaffParagon))
    if (BattleFastSkillTester(actor, SID_StaffParagon))
    {
        int bonusEXP = 0;
        int unitMagicStat = GetUnitMagic(GetUnit(actor->unit.index));
        int itemHealAmount = GetUnitItemHealAmount(GetUnit(actor->unit.index), GetUnit(actor->unit.index)->items[gActionData.itemSlotIndex]);
        int hpChange = gBattleTarget.unit.curHP - gBattleTarget.hpInitial;
        bonusEXP = (unitMagicStat + itemHealAmount) - hpChange;

        /* Halve the exp gain if promoted */
        if (UNIT_CATTRIBUTES(&actor->unit) & CA_PROMOTED)
            bonusEXP /= 2;

        /* The final score shoud be 1/5 of excess healing */
        bonusEXP /= 5;

        status += bonusEXP;
    }
#endif

#if defined(SID_Prodigy) && (COMMON_SKILL_VALID(SID_Prodigy))
    if (BattleFastSkillTester(actor, SID_Prodigy))
    {
        LIMIT_AREA(status, 1, 254);
    }
    else
    {
        LIMIT_AREA(status, 1, 100);
    }
#else
    LIMIT_AREA(status, 1, 100);
#endif

#if defined(SID_Pulse) && (COMMON_SKILL_VALID(SID_Pulse))
    if (BattleFastSkillTester(actor, SID_Pulse))
        if (status < 10)
            status = 10;
#endif

#if defined(SID_Perseverance) && (COMMON_SKILL_VALID(SID_Perseverance))
    if (BattleFastSkillTester(actor, SID_Perseverance))
        if (actor->wTriangleDmgBonus < 0 || actor->wTriangleHitBonus < 0)
            status += 5;
#endif

    /* Check last */
#if defined(SID_VoidCurse) && (COMMON_SKILL_VALID(SID_VoidCurse))
    if (BattleFastSkillTester(target, SID_VoidCurse))
        status = 0;
#endif

#if defined(SID_Carnage) && (COMMON_SKILL_VALID(SID_Carnage))
    if (BattleFastSkillTester(actor, SID_Carnage))
        status += ((target->hpInitial - target->unit.curHP) / 2);
#endif

#if defined(SID_Crisis) && (COMMON_SKILL_VALID(SID_Crisis))
    if (BattleFastSkillTester(actor, SID_Crisis))
        status += ((actor->hpInitial - actor->unit.curHP) / 2);
#endif

    // LIMIT_AREA(status, 0, 100); // JESTER - Turned off for skill Prodigy
    if (base > 0 && status <= 0)
        status = 1;

    return status;
}

LYN_REPLACE_CHECK(ModifyUnitSpecialExp);
void ModifyUnitSpecialExp(struct Unit* actor, struct Unit* target, int* exp) {
    if (UNIT_IS_GORGON_EGG(target)) {
        if (target->curHP == 0)
            *exp = 50;
        else
            *exp = 0;
    }

    if (target->pClassData->number == CLASS_DEMON_KING)
        if (target->curHP == 0)
            *exp = 0;

#ifndef CONFIG_SUMMONERS_GAIN_EXP_FROM_SUMMON_FIGHTS
    if (actor->pClassData->number == CLASS_PHANTOM)
        *exp = 0;
#endif
}

int GetBattleUnitExpGainRework(struct BattleUnit* actor, struct BattleUnit* target)
{
    int result = GetBattleUnitExpGain(actor, target);

    result = KernelModifyBattleUnitExp(result, actor, target);
    ModifyUnitSpecialExp(&actor->unit, &target->unit, &result);

    return result;
}

LYN_REPLACE_CHECK(BattleApplyMiscActionExpGains);
void BattleApplyMiscActionExpGains(void)
{
    int exp;

    if (UNIT_FACTION(&gBattleActor.unit) != FACTION_BLUE)
        return;

    if (!CanBattleUnitGainLevels(&gBattleActor))
        return;

    if (gPlaySt.chapterStateBits & PLAY_FLAG_EXTRA_MAP)
        return;

    exp = 10;
    exp = KernelModifyBattleUnitExp(exp, &gBattleActor, &gBattleTarget);

    gBattleActor.expGain = exp;
    gBattleActor.unit.exp += exp;

#if CHAX
    ResetPopupSkillStack();
#endif

    CheckBattleUnitLevelUp(&gBattleActor);
}

int GetBattleUnitStaffExpRework(struct BattleUnit* bu)
{
    int result = 0;

#ifdef CONFIG_MODULAR_STAFF_EXP
    result = StaffEXP(ITEM_INDEX(bu->weapon));
#else
    result = GetBattleUnitStaffExp(bu);
#endif

    result = KernelModifyBattleUnitExp(
        result,
        bu,
        bu == &gBattleActor
        ? &gBattleTarget
        : &gBattleActor);

#if defined(SID_Prodigy) && (COMMON_SKILL_VALID(SID_Prodigy))
    if (BattleFastSkillTester(bu, SID_Prodigy))
    {
        /* 255 is marked in vanilla as no exp gain */
        if (result > 254)
            result = 254;
    }
    else
    {
        if (result > 100)
            result = 100;
    }
#else
    if (result > 100)
        result = 100;
#endif

    return result;
}

LYN_REPLACE_CHECK(BattleApplyItemExpGains);
void BattleApplyItemExpGains(void)
{
    if (gPlaySt.chapterStateBits & PLAY_FLAG_EXTRA_MAP)
        return;

#if CHAX
    ResetPopupSkillStack();
#endif

    if (gBattleActor.weaponAttributes & IA_STAFF) {
        if (UNIT_FACTION(&gBattleActor.unit) == FACTION_BLUE)
            gBattleActor.wexpMultiplier++;

        gBattleActor.expGain = GetBattleUnitStaffExpRework(&gBattleActor);
        gBattleActor.unit.exp += gBattleActor.expGain;

        CheckBattleUnitLevelUp(&gBattleActor);
    }
    else if ((gBattleActor.weaponType == ITYPE_12) && (gBattleActor.unit.exp != UNIT_EXP_DISABLED)) {
        gBattleActor.expGain = 20;
        gBattleActor.unit.exp += 20;

        CheckBattleUnitLevelUp(&gBattleActor);
    }
}

/* JESTER - Rewrote this to only work for player units to fix a bug where enemies could gain EXP */
LYN_REPLACE_CHECK(BattleApplyExpGains);
void BattleApplyExpGains(void)
{

#ifdef CONFIG_SUMMONERS_GAIN_EXP_FROM_SUMMON_FIGHTS
    InitBattleUnit(&gBattleActor, GetUnit(CHARACTER_EIRIKA + 1));
#endif

    if (!(gPlaySt.chapterStateBits & PLAY_FLAG_EXTRA_MAP))
    {
        bool actorBlue  = (UNIT_FACTION(&gBattleActor.unit)  == FACTION_BLUE);
        bool targetBlue = (UNIT_FACTION(&gBattleTarget.unit) == FACTION_BLUE);

        if (actorBlue && gBattleActor.unit.exp != UNIT_EXP_DISABLED)
        {
            gBattleActor.expGain = GetBattleUnitExpGainRework(&gBattleActor, &gBattleTarget);
            gBattleActor.unit.exp += gBattleActor.expGain;
            CheckBattleUnitLevelUp(&gBattleActor);
        }

        if (targetBlue && gBattleTarget.unit.exp != UNIT_EXP_DISABLED)
        {
            gBattleTarget.expGain = GetBattleUnitExpGainRework(&gBattleTarget, &gBattleActor);
            gBattleTarget.unit.exp += gBattleTarget.expGain;
            CheckBattleUnitLevelUp(&gBattleTarget);
        }

    #if CHAX
        ResetPopupSkillStack();
    #endif

        // Handle replicate status (copy EXP to original unit)
        if (GetUnitStatusIndex(GetUnit(gBattleActor.unit.index)) == NEW_UNIT_STATUS_REPLICATE)
        {
            for (int i = FACTION_BLUE; i < FACTION_GREEN; i++)
            {
                struct Unit* unit = GetUnit(i);

                if (!UNIT_IS_VALID(unit))
                    continue;

                if (gBattleActor.unit.pCharacterData->number == unit->pCharacterData->number &&
                    GetUnitStatusIndex(unit) != NEW_UNIT_STATUS_REPLICATE)
                {
                    unit->exp += gBattleActor.expGain;
                    gBattleActor.unit.exp -= gBattleActor.expGain;
                    InitBattleUnit(&gBattleActor, unit);
                    break;
                }
            }
        }

        CheckBattleUnitLevelUp(&gBattleActor);
    }
}