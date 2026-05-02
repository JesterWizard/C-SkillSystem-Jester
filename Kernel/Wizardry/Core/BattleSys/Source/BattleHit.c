#include "common-chax.h"
#include "skill-system.h"
#include "efx-skill.h"
#include "battle-system.h"
#include "strmag.h"
#include "debuff.h"
#include "combat-art.h"
#include "gaiden-magic.h"
#include "shield.h"
#include "kernel-tutorial.h"
#include "constants/skills.h"
#include "jester_headers/custom-structs.h"
#include "jester_headers/custom-functions.h"
#include "jester_headers/custom-arrays.h"
#include "jester_headers/Forging.h"
#include "playst-expa.h"
#include "bwl.h"
#include "unit-expa.h"

#define LOCAL_TRACE 0

LYN_REPLACE_CHECK(BattleUpdateBattleStats);
void BattleUpdateBattleStats(struct BattleUnit* attacker, struct BattleUnit* defender)
{
	int attack = attacker->battleAttack;
	int defense = defender->battleDefense;
	int hitRate = attacker->battleEffectiveHitRate;
	int critRate = attacker->battleEffectiveCritRate;
	int silencerRate = attacker->battleSilencerRate;

	/* Fasten simulation */
	if (gBattleStats.config & BATTLE_CONFIG_SIMULATE) {
		LIMIT_AREA(attack, 0, 255);
		LIMIT_AREA(defense, 0, 255);
		LIMIT_AREA(hitRate, 0, 100);
		LIMIT_AREA(critRate, 0, 100);
		LIMIT_AREA(silencerRate, 0, 100);

		gBattleStats.attack = attack;
		gBattleStats.defense = defense;
		gBattleStats.hitRate = hitRate;
		gBattleStats.critRate = critRate;
		gBattleStats.silencerRate = silencerRate;
		return;
	}

#if defined(SID_AxeFaith) && (COMMON_SKILL_VALID(SID_AxeFaith))
	if (attacker->weaponType == ITYPE_AXE && CheckBattleSkillActivate(attacker, defender, SID_AxeFaith, attacker->battleAttack)) {
		RegisterActorEfxSkill(GetBattleHitRound(gBattleHitIterator), SID_AxeFaith);
		hitRate += attacker->battleAttack;
	}
#endif

	gBattleTemporaryFlag.skill_activated_sure_shoot = false;

#if (defined(SID_SureShot) && (COMMON_SKILL_VALID(SID_SureShot)))
	if (CheckBattleSkillActivate(attacker, defender, SID_SureShot, attacker->unit.skl)) {
		gBattleTemporaryFlag.skill_activated_sure_shoot = true;
		RegisterActorEfxSkill(GetBattleHitRound(gBattleHitIterator), SID_SureShot);
		hitRate = 100;
	}
#endif

	gBattleTemporaryFlag.skill_activated_dead_eye = false;

#if defined(SID_Deadeye) && (COMMON_SKILL_VALID(SID_Deadeye))
	if (CheckBattleSkillActivate(attacker, defender, SID_Deadeye, attacker->unit.skl)) {
		gBattleTemporaryFlag.skill_activated_dead_eye = true;
		RegisterActorEfxSkill(GetBattleHitRound(gBattleHitIterator), SID_Deadeye);
		hitRate *= 2;
	}
#endif

#if defined(SID_Momentum) && (COMMON_SKILL_VALID(SID_Momentum))
	if (BattleFastSkillTester(attacker, SID_Momentum))
		critRate += SKILL_EFF0(SID_Momentum) * GetBattleGlobalFlags(attacker)->round_cnt_hit;
#endif

#if defined(SID_LimitBreak) && (COMMON_SKILL_VALID(SID_LimitBreak))
	if (BattleFastSkillTester(attacker, SID_LimitBreak)) {
		if (GetBattleGlobalFlags(attacker)->round_cnt_hit == SKILL_EFF0(SID_LimitBreak)) {
			RegisterActorEfxSkill(GetBattleHitRound(gBattleHitIterator), SID_LimitBreak);
			hitRate = 100;
		}
	}
#endif

	LIMIT_AREA(attack, 0, 255);
	LIMIT_AREA(defense, 0, 255);
	LIMIT_AREA(hitRate, 0, 100);
	LIMIT_AREA(critRate, 0, 100);
	LIMIT_AREA(silencerRate, 0, 100);

	gBattleStats.attack = attack;
	gBattleStats.defense = defense;
	gBattleStats.hitRate = hitRate;
	gBattleStats.critRate = critRate;
	gBattleStats.silencerRate = silencerRate;

	LTRACEF("attack=%d, base=%d", attack, BattleUnitOriginalStatus(attacker)->atk);
}

LYN_REPLACE_CHECK(BattleCheckTriangleAttack);
s8 BattleCheckTriangleAttack(struct BattleUnit * attacker, struct BattleUnit * defender)
{
    s8 adjacentLookup[] = { -1, 0, 0, -1, +1, 0, 0, +1 };

    int i, count = 0;

    int triangleAttackAttr = CA_TRIANGLEATTACK_ANY & UNIT_CATTRIBUTES(&attacker->unit);

    int x = defender->unit.xPos;
    int y = defender->unit.yPos;

    int faction = UNIT_FACTION(&attacker->unit);

    gBattleStats.taUnitA = NULL;
    gBattleStats.taUnitB = NULL;

    for (i = 0; i < 4; ++i)
    {
        int uId = gBmMapUnit[adjacentLookup[i * 2 + 1] + y][adjacentLookup[i * 2 + 0] + x];
        struct Unit * unit;

        if (!uId)
            continue;

        unit = GetUnit(uId);

        if ((uId & 0xC0) != faction)
            continue;

#if (defined(SID_TriangleAttack) && (COMMON_SKILL_VALID(SID_TriangleAttack)))
        if (SkillTester(unit, SID_TriangleAttack))
        {
            ++count;

            if (!gBattleStats.taUnitA)
                gBattleStats.taUnitA = unit;
            else if (!gBattleStats.taUnitB)
                gBattleStats.taUnitB = unit;

            continue;
        }
#endif

        if (unit->statusIndex == UNIT_STATUS_SLEEP)
            continue;

        if (unit->statusIndex == UNIT_STATUS_PETRIFY)
            continue;

        if (unit->statusIndex == UNIT_STATUS_13)
            continue;

        if (unit->pClassData->number == CLASS_WYVERN_KNIGHT_F)
            continue;

        if (UNIT_CATTRIBUTES(unit) & triangleAttackAttr)
        {
            ++count;

            if (!gBattleStats.taUnitA)
                gBattleStats.taUnitA = unit;
            else if (!gBattleStats.taUnitB)
                gBattleStats.taUnitB = unit;
        }
    }
    return count >= 2 ? TRUE : FALSE;
}

LYN_REPLACE_CHECK(BattleGenerateHitTriangleAttack);
void BattleGenerateHitTriangleAttack(struct BattleUnit * attacker, struct BattleUnit * defender)
{

    /**
     * Since we're no longer limiting ourselves to just checking a
     * hardcoded attribute we can turn this off.
     */
    // if (!(UNIT_CATTRIBUTES(&attacker->unit) & CA_TRIANGLEATTACK_ANY))
    //    return;

    if (gBattleStats.range != 1)
        return;

    if (!(gBattleHitIterator->info & BATTLE_HIT_INFO_BEGIN))
        return;

    if (attacker->unit.statusIndex == UNIT_STATUS_BERSERK)
        return;

    if (gBattleStats.config & BATTLE_CONFIG_ARENA)
        return;

    if (!BattleCheckTriangleAttack(attacker, defender))
        return;

    gBattleHitIterator->attributes |= BATTLE_HIT_ATTR_TATTACK;

    gBattleStats.critRate = 100;
    gBattleStats.hitRate = 100;
}


LYN_REPLACE_CHECK(BattleGenerateHitAttributes);
void BattleGenerateHitAttributes(struct BattleUnit* attacker, struct BattleUnit* defender)
{
	FORCE_DECLARE bool grazingBlow = false;
	gBattleStats.damage = 0;

	/* Fasten simulation */
	if (!BattleRoll2RN(gBattleStats.hitRate, FALSE)) {
#if (defined(SID_DivinePulse) && (COMMON_SKILL_VALID(SID_DivinePulse)))
		if (CheckBattleSkillActivate(attacker, defender, SID_DivinePulse, SKILL_EFF0(SID_DivinePulse) + attacker->unit.lck))
			RegisterActorEfxSkill(GetBattleHitRound(gBattleHitIterator), SID_DivinePulse);
		else {
/* Divine Pulse fails, so now we check for Grazing Blow */
#if (defined(SID_GrazingBlow) && (COMMON_SKILL_VALID(SID_GrazingBlow)))
            if (BattleFastSkillTester(attacker, SID_GrazingBlow))
                grazingBlow = true;
            else
            {
                RegisterHitCnt(attacker, true);
                gBattleHitIterator->attributes |= BATTLE_HIT_ATTR_MISS;
                return;
            }
#endif
		}
#else
    /* Divine Pulse fails, so now we check for Grazing Blow */
    #if (defined(SID_GrazingBlow) && (COMMON_SKILL_VALID(SID_GrazingBlow)))
                if (BattleFastSkillTester(attacker, SID_GrazingBlow))
                        grazingBlow = true;
                else
                {
                    RegisterHitCnt(attacker, true);
                    gBattleHitIterator->attributes |= BATTLE_HIT_ATTR_MISS;
                    return;
                }
    #else 
                RegisterHitCnt(attacker, true);
                gBattleHitIterator->attributes |= BATTLE_HIT_ATTR_MISS;
                return;
    #endif
#endif
	}

	RegisterHitCnt(attacker, false);

	gBattleStats.damage = BattleHit_CalcDamage(attacker, defender);

#if defined(SID_BloodyAlchemy) && (COMMON_SKILL_VALID(SID_BloodyAlchemy))
    if (PlayStExpa_CheckBit(PLAYSTEXPA_BIT_BloodyAlchemy_Used))
        SetPartyGoldAmount(GetPartyGoldAmount() + gBattleStats.damage);
#endif

	if (gBattleStats.config & BATTLE_CONFIG_REAL) {
		if (gDmg.real_damage > 0)
			TriggerKtutorial(KTUTORIAL_REAL_DAMAGE);
	}

#if defined(SID_Debilitator) && (COMMON_SKILL_VALID(SID_Debilitator))
    if (BattleFastSkillTester(attacker, SID_Debilitator) && gBattleStats.damage == 0 && !gBattleActorGlobalFlag.skill_activated_debilitator)
        gBattleActorGlobalFlag.skill_activated_debilitator = true;
#endif

#if (defined(SID_InevitableEnd) && (COMMON_SKILL_VALID(SID_InevitableEnd)))
    if (BattleFastSkillTester(attacker, SID_InevitableEnd))
    {
        /* Ensure it only applies for one hit, essentially making it only stack 'per battle' */
        if (GetBattleGlobalFlags(attacker)->round_cnt_hit < 2)
        {
            struct Unit * target = GetUnit(defender->unit.index);

            /* If we're at the max for the hit counter, don't increment it further */
            target->counters += (target->counters == 7 ? 0 : 1);
        }
    }
#endif

	BattleCheckPetrify(attacker, defender);

	if (gBattleStats.damage != 0)
		attacker->nonZeroDamage = TRUE;
}

LYN_REPLACE_CHECK(BattleGenerateHitEffects);
void BattleGenerateHitEffects(struct BattleUnit* attacker, struct BattleUnit* defender)
{
	FORCE_DECLARE bool divertDamageToMP = false;
	FORCE_DECLARE bool gainWEXP = true;

#if (defined(SID_ShadowGiftPlus) && (COMMON_SKILL_VALID(SID_ShadowGiftPlus)))
    if (BattleFastSkillTester(attacker, SID_ShadowGiftPlus))
        if (GetItemType(GetUnitEquippedWeapon(GetUnit(attacker->unit.index))) == ITYPE_DARK)
            if (GetUnit(attacker->unit.index)->ranks[ITYPE_DARK] == 0)
                gainWEXP = false;
#endif

#if (defined(SID_ShadowGift) && (COMMON_SKILL_VALID(SID_ShadowGift)))
    if (BattleFastSkillTester(attacker, SID_ShadowGift))
        if (GetItemType(GetUnitEquippedWeapon(GetUnit(attacker->unit.index))) == ITYPE_DARK)
            if (GetUnit(attacker->unit.index)->ranks[ITYPE_DARK] == 0)
                gainWEXP = false;
#endif

#if (defined(SID_LightGiftPlus) && (COMMON_SKILL_VALID(SID_LightGiftPlus)))
    if (BattleFastSkillTester(attacker, SID_LightGiftPlus))
        if (GetItemType(GetUnitEquippedWeapon(GetUnit(attacker->unit.index))) == ITYPE_LIGHT)
            if (GetUnit(attacker->unit.index)->ranks[ITYPE_LIGHT] == 0)
                gainWEXP = false;
#endif

#if (defined(SID_LightGift) && (COMMON_SKILL_VALID(SID_LightGift)))
    if (BattleFastSkillTester(attacker, SID_LightGift))
        if (GetItemType(GetUnitEquippedWeapon(GetUnit(attacker->unit.index))) == ITYPE_LIGHT)
            if (GetUnit(attacker->unit.index)->ranks[ITYPE_LIGHT] == 0)
                gainWEXP = false;
#endif

#if (defined(SID_StormgiftPlus) && (COMMON_SKILL_VALID(SID_StormgiftPlus)))
    if (BattleFastSkillTester(attacker, SID_StormgiftPlus))
        if (GetItemType(GetUnitEquippedWeapon(GetUnit(attacker->unit.index))) == ITYPE_ANIMA)
            if (GetUnit(attacker->unit.index)->ranks[ITYPE_ANIMA] == 0)
                gainWEXP = false;
#endif

#if (defined(SID_Stormgift) && (COMMON_SKILL_VALID(SID_Stormgift)))
    if (BattleFastSkillTester(attacker, SID_Stormgift))
        if (GetItemType(GetUnitEquippedWeapon(GetUnit(attacker->unit.index))) == ITYPE_ANIMA)
            if (GetUnit(attacker->unit.index)->ranks[ITYPE_ANIMA] == 0)
                gainWEXP = false;
#endif

#if (defined(SID_BladegiftPlus) && (COMMON_SKILL_VALID(SID_BladegiftPlus)))
    if (BattleFastSkillTester(attacker, SID_BladegiftPlus))
        if (GetItemType(GetUnitEquippedWeapon(GetUnit(attacker->unit.index))) == ITYPE_SWORD)
            if (GetUnit(attacker->unit.index)->ranks[ITYPE_SWORD] == 0)
                gainWEXP = false;
#endif

#if (defined(SID_Bladegift) && (COMMON_SKILL_VALID(SID_Bladegisft)))
    if (BattleFastSkillTester(attacker, SID_Bladegift))
        if (GetItemType(GetUnitEquippedWeapon(GetUnit(attacker->unit.index))) == ITYPE_SWORD)
            if (GetUnit(attacker->unit.index)->ranks[ITYPE_SWORD] == 0)
                gainWEXP = false;
#endif

#if (defined(SID_PiercegiftPlus) && (COMMON_SKILL_VALID(SID_PiercegiftPlus)))
    if (BattleFastSkillTester(attacker, SID_PiercegiftPlus))
        if (GetItemType(GetUnitEquippedWeapon(GetUnit(attacker->unit.index))) == ITYPE_LANCE)
            if (GetUnit(attacker->unit.index)->ranks[ITYPE_LANCE] == 0)
                gainWEXP = false;
#endif

#if (defined(SID_Piercegift) && (COMMON_SKILL_VALID(SID_Piercegisft)))
    if (BattleFastSkillTester(attacker, SID_Piercegift))
        if (GetItemType(GetUnitEquippedWeapon(GetUnit(attacker->unit.index))) == ITYPE_LANCE)
            if (GetUnit(attacker->unit.index)->ranks[ITYPE_LANCE] == 0)
                gainWEXP = false;
#endif

#if (defined(SID_HackgiftPlus) && (COMMON_SKILL_VALID(SID_HackgiftPlus)))
    if (BattleFastSkillTester(attacker, SID_HackgiftPlus))
        if (GetItemType(GetUnitEquippedWeapon(GetUnit(attacker->unit.index))) == ITYPE_AXE)
            if (GetUnit(attacker->unit.index)->ranks[ITYPE_AXE] == 0)
                gainWEXP = false;
#endif

#if (defined(SID_Hackgift) && (COMMON_SKILL_VALID(SID_Hackgisft)))
    if (BattleFastSkillTester(attacker, SID_Hackgift))
        if (GetItemType(GetUnitEquippedWeapon(GetUnit(attacker->unit.index))) == ITYPE_AXE)
            if (GetUnit(attacker->unit.index)->ranks[ITYPE_AXE] == 0)
                gainWEXP = false;
#endif

#if (defined(SID_ArcgiftPlus) && (COMMON_SKILL_VALID(SID_ArcgiftPlus)))
    if (BattleFastSkillTester(attacker, SID_ArcgiftPlus))
        if (GetItemType(GetUnitEquippedWeapon(GetUnit(attacker->unit.index))) == ITYPE_BOW)
            if (GetUnit(attacker->unit.index)->ranks[ITYPE_BOW] == 0)
                gainWEXP = false;
#endif

#if (defined(SID_Arcgift) && (COMMON_SKILL_VALID(SID_Arcgift)))
    if (BattleFastSkillTester(attacker, SID_Arcgift))
        if (GetItemType(GetUnitEquippedWeapon(GetUnit(attacker->unit.index))) == ITYPE_BOW)
            if (GetUnit(attacker->unit.index)->ranks[ITYPE_BOW] == 0)
                gainWEXP = false;
#endif

    if (gainWEXP)
    {
#if (defined(SID_Discipline) && (COMMON_SKILL_VALID(SID_Discipline)))
        if (BattleFastSkillTester(attacker, SID_Discipline))
            attacker->wexpMultiplier += 2;
        else
            attacker->wexpMultiplier++;
#else
        attacker->wexpMultiplier++;
#endif
    }

	if (!(gBattleHitIterator->attributes & BATTLE_HIT_ATTR_MISS)) {
		if (CheckBattleHpHalve(attacker, defender)) {
			gBattleHitIterator->attributes |= BATTLE_HIT_ATTR_HPHALVE;
			gBattleStats.damage = defender->unit.curHP / 2;
		}

		if (CheckDevilAttack(attacker, defender)) {
			gBattleHitIterator->attributes |= BATTLE_HIT_ATTR_DEVIL;
			if (gBattleStats.damage > attacker->unit.curHP)
				gBattleStats.damage = attacker->unit.curHP;

			attacker->unit.curHP -= gBattleStats.damage;

			// if (attacker->unit.curHP < 0)
			// 	attacker->unit.curHP = 0;
		}
		else {
			if (gBattleStats.damage > defender->unit.curHP)
				gBattleStats.damage = defender->unit.curHP;

#if defined(SID_Bane) && (COMMON_SKILL_VALID(SID_Bane))
			if (gBattleStats.damage < (defender->unit.curHP - 1)) {
				if (CheckBattleSkillActivate(attacker, defender, SID_Bane, attacker->unit.skl)) {
					RegisterActorEfxSkill(GetBattleHitRound(gBattleHitIterator), SID_Bane);
					gBattleStats.damage = defender->unit.curHP - 1;
					gBattleHitIterator->attributes |= BATTLE_HIT_ATTR_CRIT;
				}
			}
#endif

#if defined(SID_DownWithArch) && (COMMON_SKILL_VALID(SID_DownWithArch))
            if (BattleFastSkillTester(attacker, SID_DownWithArch))
            {
                char name[] = "Arch";
                if (strcmp(GetStringFromIndex(GetUnit(defender->unit.index)->pCharacterData->nameTextId), name) == 0)
                {
                    RegisterActorEfxSkill(GetBattleHitRound(gBattleHitIterator), SID_DownWithArch);
                    gBattleStats.damage = defender->unit.curHP;
                    gBattleHitIterator->attributes |= BATTLE_HIT_ATTR_CRIT;
                }
            }
#endif

			bool absorb = false;

/* Check if the enemy should die */
#if (defined(SID_AbsorbMelee) && COMMON_SKILL_VALID(SID_AbsorbMelee))
            if (BattleFastSkillTester(defender, SID_AbsorbMelee) && !IsMagicAttack(attacker) && !absorb)
            {
                absorb = true;
                defender->unit.curHP += gBattleStats.damage;
            }
#endif

#if (defined(SID_AbsorbMagic) && COMMON_SKILL_VALID(SID_AbsorbMagic))
            if (BattleFastSkillTester(defender, SID_AbsorbMagic) && IsMagicAttack(attacker) && !absorb)
            {
                absorb = true;
                defender->unit.curHP += gBattleStats.damage;
            }
#endif

#if (defined(SID_AbsorbAlternation) && COMMON_SKILL_VALID(SID_AbsorbAlternation))
            if (BattleFastSkillTester(defender, SID_AbsorbAlternation))
            {
                if (IsMagicAttack(attacker) && !absorb && PlayStExpa_CheckBit(PLAYSTEXPA_BIT_AbsorbAlternation_InForce))
                {
                    absorb = true;
                    defender->unit.curHP += gBattleStats.damage;
                }
                else if (!IsMagicAttack(attacker) && !absorb && !PlayStExpa_CheckBit(PLAYSTEXPA_BIT_AbsorbAlternation_InForce))
                {
                    absorb = true;
                    defender->unit.curHP += gBattleStats.damage;
                }
            }
#endif

        if (gpKernelDesignerConfig->mp_system == true)
        {
#if defined(SID_DamageToMP) && (COMMON_SKILL_VALID(SID_DamageToMP))
			if (BattleFastSkillTester(defender, SID_DamageToMP)) {
				struct NewBwl* bwl = GetNewBwl(UNIT_CHAR_ID(GetUnit(defender->unit.index)));

				if (bwl != NULL && bwl->currentMP > 0)
				{
					bwl->currentMP = gBattleStats.damage > bwl->currentMP ? 0 : bwl->currentMP - gBattleStats.damage;
					divertDamageToMP = true;
					attacker->nonZeroDamage = false;
				}
			}
#endif
        }

			if (!divertDamageToMP && !absorb)
				defender->unit.curHP -= gBattleStats.damage;
		}

#if CHAX
		BattleHit_CalcHpDrain(attacker, defender);
#else
		if (GetItemWeaponEffect(attacker->weapon) == WPN_EFFECT_HPDRAIN) {
			if (attacker->unit.maxHP < (attacker->unit.curHP + gBattleStats.damage))
				attacker->unit.curHP = attacker->unit.maxHP;
			else
				attacker->unit.curHP += gBattleStats.damage;

			gBattleHitIterator->attributes |= BATTLE_HIT_ATTR_HPSTEAL;
		}
#endif

		BattleHit_InjectNegativeStatus(attacker, defender);
	}

    if (gpKernelDesignerConfig->mp_system == true)
    {
    #if defined(SID_DamageToMP) && (COMMON_SKILL_VALID(SID_DamageToMP))
        if (!BattleFastSkillTester(defender, SID_DamageToMP)) {
            gBattleHitIterator->hpChange = gBattleStats.damage;
        }
    #else
        gBattleHitIterator->hpChange = gBattleStats.damage;
    #endif
    }

    bool absorb = false;

/* Check if to reduce or increase the enemy's HP after attacking */
#if (defined(SID_AbsorbMelee) && COMMON_SKILL_VALID(SID_AbsorbMelee))
    if (BattleFastSkillTester(defender, SID_AbsorbMelee) && !IsMagicAttack(attacker) && !absorb)
    {
        absorb = true;
        gBattleHitIterator->hpChange = -gBattleStats.damage;
    }
#endif

#if (defined(SID_AbsorbMagic) && COMMON_SKILL_VALID(SID_AbsorbMagic))
    if (BattleFastSkillTester(defender, SID_AbsorbMagic) && IsMagicAttack(attacker) && !absorb)
    {
        absorb = true;
        gBattleHitIterator->hpChange = -gBattleStats.damage;
    }
#endif


#if (defined(SID_AbsorbAlternation) && COMMON_SKILL_VALID(SID_AbsorbAlternation))
    if (BattleFastSkillTester(defender, SID_AbsorbAlternation))
    {
        if (IsMagicAttack(attacker) && !absorb && PlayStExpa_CheckBit(PLAYSTEXPA_BIT_AbsorbAlternation_InForce))
        {
            absorb = true;
            gBattleHitIterator->hpChange = -gBattleStats.damage;
        }
        else if (!IsMagicAttack(attacker) && !absorb && !PlayStExpa_CheckBit(PLAYSTEXPA_BIT_AbsorbAlternation_InForce))
        {
            absorb = true;
            gBattleHitIterator->hpChange = -gBattleStats.damage;
        }
    }
#endif

    if (!absorb)
	{
        gBattleHitIterator->hpChange = gBattleStats.damage;
	}

	BattleHit_ConsumeWeapon(attacker, defender);
	BattleHit_ConsumeShield(attacker, defender);
}

LYN_REPLACE_CHECK(BattleGenerateHit);
bool BattleGenerateHit(struct BattleUnit* attacker, struct BattleUnit* defender)
{
	int hp_pre = defender->unit.curHP;

	if (attacker == &gBattleTarget)
		gBattleHitIterator->info |= BATTLE_HIT_INFO_RETALIATION;

	BattleUpdateBattleStats(attacker, defender);

#if CHAX
	/**
	 * Gaiden magic needs hp-cost.
	 * Only gate on cost availability for the initial attack; follow-up
	 * attacks (speed doubles, brave second hits, etc.) are free.
	 */
	if (CheckGaidenMagicAttack(attacker) && IsFirstGaidenMagicAttackInBattle(attacker)) {
		int hp_cost = GetGaidenWeaponHpCost(&attacker->unit, attacker->weapon);

		if (!TryBattleHpCost(attacker, hp_cost)) {
			gBattleHitIterator->info |= BATTLE_HIT_INFO_FINISHES;
			gBattleHitIterator++;
			return true;
		}
	}
#endif

	/**
	 * Hp cost must be calculated first
	 */
	BattleGenerateHitHpCost(attacker, defender);

	BattleGenerateHitTriangleAttack(attacker, defender);
	BattleGenerateHitAttributes(attacker, defender);
	BattleGenerateHitEffects(attacker, defender);

    GetBattleGlobalFlags(defender)->last_damage_taken = MAX(hp_pre - defender->unit.curHP, 0);

	if (attacker->unit.curHP == 0 || defender->unit.curHP == 0) {
#if (defined(SID_Discipline) && (COMMON_SKILL_VALID(SID_Discipline)))
		if (BattleFastSkillTester(attacker, SID_Discipline))
			attacker->wexpMultiplier += 2;
		else
			attacker->wexpMultiplier++;
#else
		attacker->wexpMultiplier++;
#endif

/* 
** Set the ballista byte in the unit's unit struct to the chapter number they died on  to check when using arise 
** Since we're only concerned with player units using his skill, we can use an AI byte to check if they've already been revived
** EDIT: May need to revisit this as I think ai bytes get refreshed even in player structs
*/
#if (defined(SID_Arise) && (COMMON_SKILL_VALID(SID_Arise)))
    if (BattleFastSkillTester(attacker, SID_Arise))
    {
        if (GetUnit(attacker->unit.index)->ai1 != 0xFF)
            if (attacker->unit.curHP == 0)
            {
                GetUnit(attacker->unit.index)->ai1 = 0xFF;
                GetUnit(attacker->unit.index)->ballistaIndex = gPlaySt.chapterIndex;
            }

    }
    if (BattleFastSkillTester(defender, SID_Arise))
    {
        if (GetUnit(defender->unit.index)->ai1 != 0xFF)
            if (defender->unit.curHP == 0)
            {
                GetUnit(attacker->unit.index)->ai1 = 0xFF;
                GetUnit(defender->unit.index)->ballistaIndex = gPlaySt.chapterIndex;
            }
    }
#endif

		gBattleHitIterator->info |= BATTLE_HIT_INFO_FINISHES;

#if CHAX

#if (defined(SID_Forcefield) && (COMMON_SKILL_VALID(SID_Forcefield)))
        if (BattleFastSkillTester(defender, SID_Forcefield))
        {
            if(defender->unit.curHP == defender->unit.maxHP && gBattleStats.damage >= defender->unit.maxHP/2)
            {
                gBattleStats.damage = 0;
                gBattleHitIterator->hpChange = 0;
                defender->unit.curHP = defender->unit.maxHP;

                gBattleHitIterator->info |= BATTLE_HIT_INFO_FINISHES;
                gBattleHitIterator++;
                return true;
            }
        }
#endif
		if (defender->unit.curHP == 0) {
			if (CheckBattleInori(attacker, defender)) {
				gBattleStats.damage = hp_pre - 1;
				gBattleHitIterator->hpChange = gBattleStats.damage;
				defender->unit.curHP = 1;

				gBattleHitIterator->info |= BATTLE_HIT_INFO_FINISHES;
				gBattleHitIterator++;
				return true;
			}

#if (defined(SID_OverKill) && (COMMON_SKILL_VALID(SID_OverKill)))
			if (BattleFastSkillTester(attacker, SID_OverKill)) {
				RegisterActorEfxSkill(GetBattleHitRound(gBattleHitIterator), SID_OverKill);
				AppendHpDrain(attacker, defender, gDmg.result - gBattleStats.damage);
			}
#endif
		}
#endif

		if (gBattleTarget.unit.curHP == 0) {
			gBattleActorGlobalFlag.enemy_defeated = true;
		    struct NewBwl* bwl = GetNewBwl(UNIT_CHAR_ID(GetUnit(gBattleActor.unit.index)));

        if (gpKernelDesignerConfig->promote_enemy_on_kill == true) 
	    {
			struct Unit* enemyUnit = GetUnit(gBattleActor.unit.index);
            if (UNIT_FACTION(enemyUnit) == FACTION_RED && !(UNIT_CATTRIBUTES(enemyUnit) & CA_PROMOTED))
			{
				ApplyUnitDefaultPromotion(enemyUnit);
				enemyUnit->maxHP += gpKernelDesignerConfig->promote_enemy_boost;
				enemyUnit->curHP += gpKernelDesignerConfig->promote_enemy_boost;
				enemyUnit->pow += gpKernelDesignerConfig->promote_enemy_boost;
				enemyUnit->_u47 += gpKernelDesignerConfig->promote_enemy_boost; // magic
				enemyUnit->skl += gpKernelDesignerConfig->promote_enemy_boost;
				enemyUnit->spd += gpKernelDesignerConfig->promote_enemy_boost;
				enemyUnit->lck += gpKernelDesignerConfig->promote_enemy_boost;
				enemyUnit->def += gpKernelDesignerConfig->promote_enemy_boost;
				enemyUnit->res += gpKernelDesignerConfig->promote_enemy_boost;
			}
        }

#if (defined(SID_Emulate) && (COMMON_SKILL_VALID(SID_Emulate)))
            if (SkillTester(GetUnit(gBattleActor.unit.index), SID_Emulate))
            {
                if (gBattleActor.unit.curHP > GetUnit(gBattleActor.unit.index)->maxHP)
                    gBattleActor.unit.curHP = GetUnit(gBattleActor.unit.index)->maxHP;
            }
#endif

#if (defined(SID_Galeforce) && (COMMON_SKILL_VALID(SID_Galeforce)))
			if (BattleFastSkillTester(&gBattleActor, SID_Galeforce))
				gBattleActorGlobalFlag.skill_activated_galeforce = true;
#endif

#if (defined(SID_GaleforcePlus) && (COMMON_SKILL_VALID(SID_GaleforcePlus)))
			if (SkillTesterPlus(GetUnit(gBattleActor.unit.index), SID_GaleforcePlus))
				gBattleActorGlobalFlag.skill_activated_galeforce = true;
#endif

#if (defined(SID_LeadByExample) && (COMMON_SKILL_VALID(SID_LeadByExample)))
            if (CheckBattleSkillActivate(&gBattleActor, &gBattleTarget, SID_LeadByExample, 100))
                gBattleActorGlobalFlag.skill_activated_lead_by_example = true;
#endif

#if (defined(SID_Pickup) && (COMMON_SKILL_VALID(SID_Pickup)))
			if (CheckBattleSkillActivate(&gBattleActor, &gBattleTarget, SID_Pickup, gBattleActor.unit.lck)) {
				struct Unit* unit_tar = &gBattleTarget.unit;

				unit_tar->state |= US_DROP_ITEM;
			}
#endif

#if (defined(SID_Resurrection) && (COMMON_SKILL_VALID(SID_Resurrection)))
            if (BattleFastSkillTester(&gBattleTarget, SID_Resurrection) && !CheckBitUES(GetUnit(gBattleTarget.unit.index), UES_BIT_RESURRECTION_SKILL_USED))
            {
                gBattleTargetGlobalFlag.skill_activated_resurrection = true;
                gBattleTarget.unit.curHP = 1;
                gBattleHitIterator->info |= BATTLE_HIT_INFO_FINISHES;
                gBattleHitIterator++;
                return true;
            }
#endif

#if (defined(SID_Ripple) && (COMMON_SKILL_VALID(SID_Ripple)))
			if (BattleFastSkillTester(&gBattleActor, SID_Ripple))
            {
                if (gBattleTarget.hpInitial <= (gBattleTarget.unit.maxHP / 4))
				    PlayStExpa_SetBit(PLAYSTEXPA_BIT_Ripple_Used);
            }
#endif

#if defined(SID_Bloodthirst) && (COMMON_SKILL_VALID(SID_Bloodthirst))
    if (BattleFastSkillTester(&gBattleActor, SID_Bloodthirst))
    {
        int weapon = gBattleActor.weapon;

        if (weapon != ITEM_NONE)
        {
            if (!(GetItemAttributes(weapon) & IA_UNBREAKABLE))
            {
                gBattleActor.weapon = (weapon += (1 << 8));
                RegisterActorEfxSkill(GetBattleHitRound(gBattleHitIterator), SID_Bloodthirst);
            }
        }
    }
#endif

        if (gpKernelDesignerConfig->forge_mechanic == true)
        {
            u16 item = GetUnitEquippedWeapon(GetUnit(gBattleActor.unit.index));
            if (CanItemBeForged(item))
            {
                u8 id = ITEM_USES(item);
                gForgedItemRam[id].crit += 1;
            }
        }

        if (gpKernelDesignerConfig->mp_system == true)
        {
			if (bwl != NULL)
			{
                int killMpGeneration =  gMpSystemPInfoConfigList[UNIT_CHAR_ID(GetUnit(gBattleActor.unit.index))].killGeneration;

#if defined(SID_Enlightenment) && (COMMON_SKILL_VALID(SID_Enlightenment))
                if (BattleFastSkillTester(attacker, SID_Enlightenment))
                    killMpGeneration = 0;
#endif

#if defined(SID_ManaTransfusion) && (COMMON_SKILL_VALID(SID_ManaTransfusion))
                if (BattleFastSkillTester(attacker, SID_ManaTransfusion))
                    gBattleActor.unit.curHP += killMpGeneration;
#endif

				bwl->currentMP += killMpGeneration;
				if (bwl->currentMP > GetUnitMaxMP(GetUnit(attacker->unit.index)))
					bwl->currentMP = GetUnitMaxMP(GetUnit(attacker->unit.index));
			}
        }

            if (gpKernelDesignerConfig->skill_shop)
            {
                if (bwl != NULL)
                {
                    int spBoost = 0;

                    if (UNIT_CATTRIBUTES(&defender->unit) & CA_BOSS)
                        spBoost += gSkillPointsSystemPInfoConfigList[0].boostedRate;    
                    else
                        spBoost += gSkillPointsSystemPInfoConfigList[0].standardRate;

    #if (defined(SID_SPConversion) && (COMMON_SKILL_VALID(SID_SPConversion)))
                if (BattleFastSkillTester(&gBattleActor, SID_SPConversion))
                    spBoost += SKILL_EFF0(SID_SPConversion);
    #endif

                    bwl->skillPoints = bwl->skillPoints + spBoost > 255 ? 255 : bwl->skillPoints + spBoost;
                }
            }

			gBattleHitIterator->info |= BATTLE_HIT_INFO_KILLS_TARGET;
		}
        else if (
            defender->statusOut == UNIT_STATUS_PETRIFY || defender->statusOut == UNIT_STATUS_13 ||
            defender->statusOut == UNIT_STATUS_SLEEP || (GetUnitStatusIndex(GetUnit(defender->unit.index)) == NEW_UNIT_STATUS_BREAK && defender->canCounter == true))
        {
            gBattleHitIterator->info |= BATTLE_HIT_INFO_FINISHES;
            gBattleHitIterator++;
            return true;
        }

		gBattleHitIterator++;
		return true;
	}

    if (gpKernelDesignerConfig->ignore_stop_on_petrify_sleep == false)
    {
        if (defender->statusOut == UNIT_STATUS_PETRIFY || defender->statusOut == UNIT_STATUS_13 || defender->statusOut == UNIT_STATUS_SLEEP) {
            gBattleHitIterator->info |= BATTLE_HIT_INFO_FINISHES;
            gBattleHitIterator++;
            return true;
        }
    }

	gBattleHitIterator++;
	return false;
}

//! FE8U = 0x080328D0
LYN_REPLACE_CHECK(BATTLE_HandleItemDrop);
bool BATTLE_HandleItemDrop(struct CombatActionProc* proc) {
    struct Unit* unitA = NULL;
    struct Unit* unitB = NULL;

    proc->unitIdA = gBattleActor.unit.index;
    proc->unitIdB = gBattleTarget.unit.index;

    if (gBattleActor.unit.curHP == 0) {
        unitA = GetUnit(gBattleActor.unit.index);
        unitB = GetUnit(gBattleTarget.unit.index);
    }

    if (gBattleTarget.unit.curHP == 0) {
        unitA = GetUnit(gBattleTarget.unit.index);
        unitB = GetUnit(gBattleActor.unit.index);
    }

    if (unitA == NULL) {
        return true;
    }

    if (gpKernelDesignerConfig->kill_rewards == true)
    {
        int itemReward = GetItemReward(unitB, unitA);

        if (itemReward != 0)
            NewPopup_ItemGot(proc, unitB, itemReward);
    }

    if (!(unitA->state & US_DROP_ITEM)) {
        return true;
    }

    if (unitA->items[0] == 0) {
        return true;
    }

    if (UNIT_FACTION(unitB) != FACTION_BLUE) {
        return true;
    }

    NewPopup_GeneralItemGot(
        unitB,
        GetUnitLastItem(unitA),
        proc
    );

    return false;
}
