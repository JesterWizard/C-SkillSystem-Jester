#include "common-chax.h"
#include "skill-system.h"
#include "battle-system.h"
#include "bwl.h"
#include "constants/skills.h"
#include "debuff.h"
#include "strmag.h"
#include "jester_headers/custom-functions.h"

static bool CanUnitGainBattleExp(struct Unit *unit)
{
    return UNIT_FACTION(unit) == FACTION_BLUE
        && unit->exp != UNIT_EXP_DISABLED
        && unit->curHP > 0;
}

int StaffEXP(int weapon)
{
    int exp = 0;

    switch (weapon)
    {
    case ITEM_STAFF_HEAL:
    case ITEM_STAFF_TORCH:
        exp = 15;
        break;
    case ITEM_STAFF_BARRIER:
    case ITEM_STAFF_FORCE:
    case ITEM_STAFF_TEMPEST:
    case ITEM_STAFF_ACUITY:
    case ITEM_STAFF_SPRINT:
    case ITEM_STAFF_FORTUNE:
    case ITEM_STAFF_IRON:
    case ITEM_STAFF_OMNI:
    case ITEM_STAFF_POISON:
    case ITEM_STAFF_DELAY:
    case ITEM_STAFF_INVEST:
        exp = 17;
        break;
    case ITEM_STAFF_MEND:
    case ITEM_STAFF_MINE:
    case ITEM_STAFF_ARBALEST:
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
    case ITEM_STAFF_RUNE:
    case ITEM_STAFF_SLOW:
    case ITEM_STAFF_FORGE:
    case ITEM_STAFF_REWARP:
    case ITEM_STAFF_QUICKEN:
    case ITEM_STAFF_HIDE:
    case ITEM_STAFF_PROVOKE:
    case ITEM_STAFF_PETRIFY:
    case ITEM_STAFF_SOOTH:
    case ITEM_STAFF_ENFEEBLE:
    case ITEM_STAFF_SKILL:
    case ITEM_STAFF_AGAIN:
    case ITEM_STAFF_ABORT:
    case ITEM_STAFF_WHITE_WIND:
    case ITEM_STAFF_GREEN_WIND:
        exp = 30;
        break;
    case ITEM_STAFF_RESCUE:
    case ITEM_STAFF_AUM:
    case ITEM_STAFF_ENTRAP:
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
    case ITEM_STAFF_PHOENIX:
        exp = 100;
        break;

    }
    return exp;
}

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

    /* Check last */
#if defined(SID_VoidCurse) && (COMMON_SKILL_VALID(SID_VoidCurse))
    if (BattleFastSkillTester(target, SID_VoidCurse))
        status = 0;
#endif

    switch (gActionData.itemSlotIndex)
    {
        case CHAX_BUISLOT_GAIDEN_WMAG1:
        case CHAX_BUISLOT_GAIDEN_WMAG2:
        case CHAX_BUISLOT_GAIDEN_WMAG3:
        case CHAX_BUISLOT_GAIDEN_WMAG4:
        case CHAX_BUISLOT_GAIDEN_WMAG5:
            status = 0;
            break;
        default:
            break;
    }

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

    if (gpKernelDesignerConfig->summons_gain_exp != true)
    {
        if (actor->pClassData->number == CLASS_PHANTOM)
            *exp = 0;
    }
}

int GetBattleUnitExpGainRework(struct BattleUnit* actor, struct BattleUnit* target)
{
    int result = GetBattleUnitExpGain(actor, target);

    result = KernelModifyBattleUnitExp(result, actor, target);
    ModifyUnitSpecialExp(&actor->unit, &target->unit, &result);

    return result;
}

static void ApplyReplicateLevelUpChanges(struct Unit *unit, struct BattleUnit *bu)
{
    unit->level = bu->unit.level;
    unit->exp = bu->unit.exp;
    unit->maxHP += bu->changeHP;
    unit->pow += bu->changePow;
    unit->skl += bu->changeSkl;
    unit->spd += bu->changeSpd;
    unit->def += bu->changeDef;
    unit->res += bu->changeRes;
    unit->lck += bu->changeLck;
    UNIT_MAG(unit) += BU_CHG_MAG(bu);

    UnitCheckStatCaps(unit);
    SyncReplicateLinkedHp(unit);
}

static void ApplyReplicateExpGain(struct BattleUnit *bu)
{
    struct Unit *linked = GetReplicateLinkedUnit(GetUnit(bu->unit.index));
    struct BattleUnit linkedBattle;

    if (!UNIT_IS_VALID(linked)) {
        bu->unit.exp += bu->expGain;
        CheckBattleUnitLevelUp(bu);
        return;
    }

    InitBattleUnit(&linkedBattle, linked);
    linkedBattle.expGain = bu->expGain;
    linkedBattle.unit.exp += linkedBattle.expGain;
    CheckBattleUnitLevelUp(&linkedBattle);
    ApplyReplicateLevelUpChanges(linked, &linkedBattle);

    bu->unit.exp = bu->expPrevious;
    bu->expGain = 0;
}

static void ApplyBattleUnitExpGain(struct BattleUnit *bu, struct BattleUnit *opponent)
{
    if (!CanUnitGainBattleExp(&bu->unit))
        return;

    bu->expGain = GetBattleUnitExpGainRework(bu, opponent);

    if (GetUnitStatusIndex(GetUnit(bu->unit.index)) == NEW_UNIT_STATUS_REPLICATE) {
        ApplyReplicateExpGain(bu);
        return;
    }

    bu->unit.exp += bu->expGain;
    CheckBattleUnitLevelUp(bu);
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

    if (gpKernelDesignerConfig->modular_staff_exp == true)
        result = StaffEXP(ITEM_INDEX(bu->weapon));
    else
        result = GetBattleUnitStaffExp(bu);

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

    if (!CanUnitGainBattleExp(&gBattleActor.unit))
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
    if (!(gPlaySt.chapterStateBits & PLAY_FLAG_EXTRA_MAP))
    {
        bool actorBlue  = CanUnitGainBattleExp(&gBattleActor.unit);
        bool targetBlue = CanUnitGainBattleExp(&gBattleTarget.unit);

        if (gpKernelDesignerConfig->summons_gain_exp == true)
        {
            switch (gBattleActor.unit.pCharacterData->number) 
            {
                case CHARACTER_SUMMON_EWAN:
                case CHARACTER_SUMMON_LYON:
                case CHARACTER_SUMMON_KNOLL:
                    if (actorBlue)
                    {
                        /* This is causing problems with the summoner becoming the one that fights */
                        /* Also they are only gaining 1 EXP */
                        InitBattleUnit(&gBattleActor, GetUnit(gBattleActor.unit.ranks[ITYPE_STAFF]));
                        gBattleActor.expGain = GetBattleUnitExpGainRework(&gBattleActor, &gBattleTarget);
                        gBattleActor.unit.exp += gBattleActor.expGain;
                        CheckBattleUnitLevelUp(&gBattleActor);
                    }
                    else
                    {
                        /* This is causing problems with the summoner becoming the one that fights */
                        /* Also they are only gaining 1 EXP */
                        InitBattleUnit(&gBattleTarget, GetUnit(gBattleTarget.unit.ranks[ITYPE_STAFF]));
                        gBattleTarget.expGain = GetBattleUnitExpGainRework(&gBattleTarget, &gBattleActor);
                        gBattleTarget.unit.exp += gBattleTarget.expGain;
                        CheckBattleUnitLevelUp(&gBattleTarget); 
                    }
                    break;
            }
        }


        if (actorBlue && gBattleActor.unit.exp != UNIT_EXP_DISABLED)
        {
            ApplyBattleUnitExpGain(&gBattleActor, &gBattleTarget);
        }

        if (targetBlue && gBattleTarget.unit.exp != UNIT_EXP_DISABLED)
        {
            ApplyBattleUnitExpGain(&gBattleTarget, &gBattleActor);
        }

    #if CHAX
        ResetPopupSkillStack();
    #endif
    }
}