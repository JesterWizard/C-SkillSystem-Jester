#include "common-chax.h"
#include "status-getter.h"
#include "skill-system.h"
#include "efx-skill.h"
#include "battle-system.h"
#include "gaiden-magic.h"
#include "combat-art.h"
#include "strmag.h"
#include "bwl.h"
#include "unit-expa.h"
#include "combo-attack.h"
#include "constants/skills.h"
#include "combat-art.h"
#include "debuff.h"

#define LOCAL_TRACE 0

STATIC_DECLAR bool CheckCanContinueAttack(struct BattleUnit *bu)
{
	if (!bu->weapon)
		return false;

	if (CheckGaidenMagicAttack(bu))
		if (bu->unit.curHP <= GetGaidenWeaponHpCost(&bu->unit, bu->weapon))
			return false;

    if (gpKernelDesignerConfig->ignore_stop_on_petrify_sleep == true)
    {
        switch (bu->statusOut) {
        case UNIT_STATUS_SLEEP:
        case UNIT_STATUS_PETRIFY:
        case UNIT_STATUS_13:
            return false;

        case UNIT_STATUS_SILENCED:
            if (IsMagicAttack(bu))
                return false;

            break;

        case NEW_UNIT_STATUS_BOUND:
            if (!IsMagicAttack(bu))
                return false;

            break;

        default:
            break;
        }
    }

	return true;
}

/* This function should also be called by BKSEL, so non static */
bool CheckCanTwiceAttackOrder(struct BattleUnit *actor, struct BattleUnit *target)
{
	FORCE_DECLARE bool followup_nullified_en = true;
	FORCE_DECLARE bool ref_actor_hp_above_half  = ((actor->hpInitial  * 2) > actor->unit.maxHP);
	FORCE_DECLARE bool ref_target_hp_above_half = ((target->hpInitial * 2) > target->unit.maxHP);
	FORCE_DECLARE u8 cid;

	if (&gBattleActor == actor) {
		gBattleTemporaryFlag.act_force_twice_order = false;
		gBattleTemporaryFlag.act_normal_judge_twice_order = false;
	} else {
		gBattleTemporaryFlag.tar_force_twice_order = false;
		gBattleTemporaryFlag.tar_normal_judge_twice_order = false;
	}

	if (target->battleSpeed > 250)
		return false;

	if (!actor->weapon)
		return false;

	if (GetItemWeaponEffect(actor->weaponBefore) == WPN_EFFECT_HPHALVE)
		return false;

	if (GetItemIndex(actor->weapon) == ITEM_MONSTER_STONE)
		return false;

#if defined(SID_LoadstarRush) && (COMMON_SKILL_VALID(SID_LoadstarRush))
	if (gActionData.unk08 == SID_LoadstarRush && !CheckBitUES(gActiveUnit, UES_BIT_LOADSTAR_RUSH_SKILL_USED))
		return false;
#endif

#if defined(SID_Bide) && (COMMON_SKILL_VALID(SID_Bide))
	if (gActionData.unk08 == SID_Bide && !CheckBitUES(gActiveUnit, UES_BIT_BIDE_SKILL_USED))
		return false;
#endif

	/* Check combat-art */
	if (&gBattleActor == actor) {
		int cid = GetCombatArtInForce(&actor->unit);

		if (COMBART_VALID(cid)) {
			switch (GetCombatArtInfo(cid)->double_attack) {
			case COMBART_DOUBLE_DISABLED:
				return false;

			case COMBART_DOUBLE_FORCE_ENABLED:
				return true;

			case COMBART_DOUBLE_ENABLED:
			default:
				break;
			}
		}
	}

	/* Check can prevent the follow-up attack */
	followup_nullified_en = true;
	if (&gBattleActor == actor) {
#if defined(SID_YngviAscendant) && (COMMON_SKILL_VALID(SID_YngviAscendant))
		if (BattleFastSkillTester(actor, SID_YngviAscendant))
			followup_nullified_en = false;
#endif
	} else {
#if defined(SID_DragonWrath) && (COMMON_SKILL_VALID(SID_DragonWrath))
		if (BattleFastSkillTester(actor, SID_DragonWrath))
			followup_nullified_en = false;
#endif
	}

	if (followup_nullified_en) {
		if (&gBattleActor == actor) {
#if defined(SID_WaryFighter) && (COMMON_SKILL_VALID(SID_WaryFighter))
			if (BattleFastSkillTester(target, SID_WaryFighter))
				if (ref_target_hp_above_half)
					return false;
#endif

#if defined(SID_Moonlight) && (COMMON_SKILL_VALID(SID_Moonlight))
			if (BattleFastSkillTester(actor, SID_Moonlight))
				return false;
#endif

			if (GetUnitStatusIndex(GetUnit(gBattleActor.unit.index)) == NEW_UNIT_STATUS_DELAY) {
				return false;
			}
		}
		else if (&gBattleTarget == actor) {
			if (GetUnitStatusIndex(GetUnit(gBattleTarget.unit.index)) == NEW_UNIT_STATUS_DELAY) {
				return false;
			}
		}
	}

	/* Check attacker */
	if (&gBattleActor == actor) {
#if defined(SID_TrickRoom) && (COMMON_SKILL_VALID(SID_TrickRoom))
		if (BattleFastSkillTester(actor, SID_TrickRoom))
		{
			if (followup_nullified_en == false)
				return true;
			else
				return false;
		}
		else if (BattleFastSkillTester(target, SID_TrickRoom))
		{
			if (followup_nullified_en == false)
				return true;
			else
				return false;
    }
#endif

#if defined(SID_Switcher) && (COMMON_SKILL_VALID(SID_Switcher))
        if (BattleFastSkillTester(actor, SID_Switcher))
        {
            if (followup_nullified_en == true && gActionData.unk08 == SID_Switcher)
            {
                if (!CheckBitUES(GetUnit(actor->unit.index), UES_BIT_SWITCHER_SKILL_USED))
                    return true;
                else
                    return false;
            }
        }
#endif
#if defined(SID_BoldFighter) && (COMMON_SKILL_VALID(SID_BoldFighter))
		if (BattleFastSkillTester(actor, SID_BoldFighter) && ref_actor_hp_above_half) {
			gBattleTemporaryFlag.act_force_twice_order = true;
			RegisterBattleOrderSkill(SID_BoldFighter, BORDER_ACT_TWICE);
			return true;
		}
#endif

#if defined(SID_RecklessFighter) && (COMMON_SKILL_VALID(SID_RecklessFighter))
		if (BattleFastSkillTester(actor, SID_RecklessFighter) && ref_actor_hp_above_half) {
			RegisterBattleOrderSkill(SID_RecklessFighter, BORDER_ACT_TWICE);
			gBattleTemporaryFlag.act_force_twice_order = true;
			return true;
		} else if (BattleFastSkillTester(target, SID_RecklessFighter) && (target->hpInitial * 2) >= target->unit.maxHP) {
			gBattleTemporaryFlag.act_force_twice_order = true;
			return true;
		}
#endif

#if defined(SID_BidingBlow) && (COMMON_SKILL_VALID(SID_BidingBlow))
		if (BattleFastSkillTester(actor, SID_BidingBlow)) {
			if (target->canCounter == false) {
				gBattleTemporaryFlag.act_force_twice_order = true;
				RegisterBattleOrderSkill(SID_BidingBlow, BORDER_ACT_TWICE);
				return true;
			}
		}
#endif

#if defined(SID_AdvantageChaser) && (COMMON_SKILL_VALID(SID_AdvantageChaser))
		if (BattleFastSkillTester(actor, SID_AdvantageChaser)) {
			if (actor->wTriangleDmgBonus > 0 || actor->wTriangleHitBonus > 0) {
				gBattleTemporaryFlag.act_force_twice_order = true;
				RegisterBattleOrderSkill(SID_AdvantageChaser, BORDER_ACT_TWICE);
				return true;
			}
		}
#endif

#if defined(SID_PridefulWarrior) && (COMMON_SKILL_VALID(SID_PridefulWarrior))
		if (BattleFastSkillTester(actor, SID_PridefulWarrior)) {
			gBattleTemporaryFlag.act_force_twice_order = true;
			RegisterBattleOrderSkill(SID_PridefulWarrior, BORDER_ACT_TWICE);
			return true;
		}
#endif

#if defined(SID_PassionsFlow) && (COMMON_SKILL_VALID(SID_PassionsFlow))
		if (BattleFastSkillTester(actor, SID_PassionsFlow)) {
			struct SupportBonuses bonuses;

			if (GetUnitSupportBonuses(GetUnit(actor->unit.index), &bonuses) != 0) {
				gBattleTemporaryFlag.act_force_twice_order = true;
				RegisterBattleOrderSkill(SID_PassionsFlow, BORDER_ACT_TWICE);
				return true;
			}
		}
#endif

#if defined(SID_BrashAssault) && (COMMON_SKILL_VALID(SID_BrashAssault))
		if (BattleFastSkillTester(actor, SID_BrashAssault)) {
			if (target->canCounter) {
				gBattleTemporaryFlag.act_force_twice_order = true;
				RegisterBattleOrderSkill(SID_BrashAssault, BORDER_ACT_TWICE);
				return true;
			}
		}
#endif

#if defined(SID_QuickLearner) && (COMMON_SKILL_VALID(SID_QuickLearner))
		if (BattleFastSkillTester(actor, SID_QuickLearner)) {
			int lv1 = actor->levelPrevious  + GetUnitHiddenLevel(&actor->unit);
			int lv2 = target->levelPrevious + GetUnitHiddenLevel(&target->unit);

			if (lv1 < lv2) {
				gBattleTemporaryFlag.act_force_twice_order = true;
				RegisterBattleOrderSkill(SID_QuickLearner, BORDER_ACT_TWICE);
				return true;
			}
		}
#endif

#if defined(SID_RapidFighter) && (COMMON_SKILL_VALID(SID_RapidFighter))
        if (BattleFastSkillTester(actor, SID_RapidFighter))
        {
			if (gPlaySt.faction != UNIT_FACTION(GetUnit(actor->unit.index)))
			{
				gBattleTemporaryFlag.act_force_twice_order = true;
				RegisterBattleOrderSkill(SID_RapidFighter, BORDER_ACT_TWICE);
				return false;
			}
        }
#endif

	} else if (&gBattleTarget == actor) {
#if defined(SID_TrickRoom) && (COMMON_SKILL_VALID(SID_TrickRoom))
		if (BattleFastSkillTester(actor, SID_TrickRoom))
		{
			if (followup_nullified_en == false)
				return true;
			else
				return false;
		}
		else if (BattleFastSkillTester(target, SID_TrickRoom))
		{
			if (followup_nullified_en == false)
				return true;
			else
				return false;
		}
#endif

#if defined(SID_Switcher) && (COMMON_SKILL_VALID(SID_Switcher))
        if (BattleFastSkillTester(actor, SID_Switcher))
        {
            if (followup_nullified_en == true && gActionData.unk08 == SID_Switcher)
            {
                if (!CheckBitUES(GetUnit(actor->unit.index), UES_BIT_SWITCHER_SKILL_USED))
                    return true;         
                else
                    return false;
            }
        }
#endif

#if defined(SID_VengefulFighter) && (COMMON_SKILL_VALID(SID_VengefulFighter))
		if (BattleFastSkillTester(actor, SID_VengefulFighter) && ref_actor_hp_above_half) {
			gBattleTemporaryFlag.tar_force_twice_order = true;
			RegisterBattleOrderSkill(SID_VengefulFighter, BORDER_TAR_TWICE);
			return true;
		}
#endif

#if defined(SID_DragonWrath) && (COMMON_SKILL_VALID(SID_DragonWrath))
		if (BattleFastSkillTester(actor, SID_DragonWrath) && ref_actor_hp_above_half) {
			gBattleTemporaryFlag.tar_force_twice_order = true;
			RegisterBattleOrderSkill(SID_DragonWrath, BORDER_TAR_TWICE);
			return true;
		}
#endif

#if defined(SID_RecklessFighter) && (COMMON_SKILL_VALID(SID_RecklessFighter))
		if (BattleFastSkillTester(actor, SID_RecklessFighter)) {
			RegisterBattleOrderSkill(SID_RecklessFighter, BORDER_TAR_TWICE);
			gBattleTemporaryFlag.tar_force_twice_order = true;
			return true;
		} else if (BattleFastSkillTester(target, SID_RecklessFighter)) {
			gBattleTemporaryFlag.tar_force_twice_order = true;
			return true;
		}
#endif

#if defined(SID_QuickRiposte) && (COMMON_SKILL_VALID(SID_QuickRiposte))
		if (BattleFastSkillTester(actor, SID_QuickRiposte)) {
			if (ref_actor_hp_above_half) {
				gBattleTemporaryFlag.tar_force_twice_order = true;
				RegisterBattleOrderSkill(SID_QuickRiposte, BORDER_TAR_TWICE);
				return true;
			}
		}
#endif

#if defined(SID_AdvantageChaser) && (COMMON_SKILL_VALID(SID_AdvantageChaser))
		if (BattleFastSkillTester(actor, SID_AdvantageChaser)) {
			if (actor->wTriangleDmgBonus > 0 || actor->wTriangleHitBonus > 0) {
				gBattleTemporaryFlag.tar_force_twice_order = true;
				RegisterBattleOrderSkill(SID_AdvantageChaser, BORDER_TAR_TWICE);
				return true;
			}
		}
#endif

#if defined(SID_PassionsFlow) && (COMMON_SKILL_VALID(SID_PassionsFlow))
		if (BattleFastSkillTester(actor, SID_PassionsFlow)) {
			struct SupportBonuses bonuses;

			if (GetUnitSupportBonuses(GetUnit(actor->unit.index), &bonuses) != 0) {
				gBattleTemporaryFlag.tar_force_twice_order = true;
				RegisterBattleOrderSkill(SID_PassionsFlow, BORDER_TAR_TWICE);
				return true;
			}
		}
#endif

#if defined(SID_RapidFighter) && (COMMON_SKILL_VALID(SID_RapidFighter))
        if (BattleFastSkillTester(actor, SID_RapidFighter))
		{
			if (gPlaySt.faction != UNIT_FACTION(GetUnit(actor->unit.index)))
			{
				gBattleTemporaryFlag.tar_force_twice_order = true;
				RegisterBattleOrderSkill(SID_RapidFighter, BORDER_TAR_TWICE);
				return false;
			}
        }
#endif

	}

	/* Basic judgement */
	if (&gBattleActor == actor)
		gBattleTemporaryFlag.act_normal_judge_twice_order = true;
	else
		gBattleTemporaryFlag.tar_normal_judge_twice_order = true;

	bool canActorDouble = ((actor->battleSpeed - target->battleSpeed) >= get_battle_followup_speed_threshold());


#if defined(SID_Hasty) && (COMMON_SKILL_VALID(SID_Hasty))
    if (BattleFastSkillTester(actor, SID_Hasty))
		canActorDouble = (actor->battleSpeed - target->battleSpeed) >= 1;
#endif

	return canActorDouble;
}

STATIC_DECLAR bool CheckDesperationOrder(void)
{
	gBattleTemporaryFlag.desperation_order = false;

	if (!gBattleActor.weapon)
		return false;

#if defined(SID_Desperation) && (COMMON_SKILL_VALID(SID_Desperation))
	if (BattleFastSkillTester(&gBattleActor, SID_Desperation)) {
		if ((gBattleActor.hpInitial * 2) < gBattleActor.unit.maxHP) {
			gBattleTemporaryFlag.desperation_order = true;
			RegisterBattleOrderSkill(SID_Desperation, BORDER_DESPERATION);
			return true;
		}
	}
#endif

#if defined(SID_CloseCombat) && (COMMON_SKILL_VALID(SID_CloseCombat))
	if (BattleFastSkillTester(&gBattleActor, SID_CloseCombat) && gBattleStats.range == 1) {
		gBattleTemporaryFlag.desperation_order = true;
		RegisterBattleOrderSkill(SID_CloseCombat, BORDER_DESPERATION);
		return true;
	}
#endif

#if defined(SID_Pursuer) && (COMMON_SKILL_VALID(SID_Pursuer))
	if (BattleFastSkillTester(&gBattleActor, SID_Pursuer)) {
		gBattleTemporaryFlag.desperation_order = true;
		RegisterBattleOrderSkill(SID_Pursuer, BORDER_DESPERATION);
		return true;
	}
#endif

#if defined(SID_AlacrityPlus) && (COMMON_SKILL_VALID(SID_AlacrityPlus))
    if (BattleFastSkillTester(&gBattleActor, SID_AlacrityPlus))
    {
        if ((gBattleActor.battleSpeed - 5) >= gBattleTarget.battleSpeed)
        {
            gBattleTemporaryFlag.desperation_order = true;
            RegisterBattleOrderSkill(SID_AlacrityPlus, BORDER_DESPERATION);
            return true;
        }
    }
#endif

#if defined(SID_Alacrity) && (COMMON_SKILL_VALID(SID_Alacrity))
    if (BattleFastSkillTester(&gBattleActor, SID_Alacrity))
    {
        if ((gBattleActor.battleSpeed - 9) >= gBattleTarget.battleSpeed)
        {
            gBattleTemporaryFlag.desperation_order = true;
            RegisterBattleOrderSkill(SID_Alacrity, BORDER_DESPERATION);
            return true;
        }
    }
#endif

#if defined(SID_YngviAscendant) && (COMMON_SKILL_VALID(SID_YngviAscendant))
	if (BattleFastSkillTester(&gBattleActor, SID_YngviAscendant)) {
		gBattleTemporaryFlag.desperation_order = true;
		RegisterBattleOrderSkill(SID_YngviAscendant, BORDER_DESPERATION);
		return true;
	}
#endif

	return false;
}

STATIC_DECLAR bool CheckVantageOrder(void)
{
	gBattleTemporaryFlag.vantage_order = false;

	/* Combat art will not allow vantage */
	if (COMBART_VALID(GetCombatArtInForce(&gBattleActor.unit)))
		return false;

	if (!gBattleTarget.weapon)
		return false;

#if defined(SID_EyeForAnEye) && (COMMON_SKILL_VALID(SID_EyeForAnEye))
    if (BattleFastSkillTester(&gBattleActor, SID_EyeForAnEye)) {
        RegisterBattleOrderSkill(SID_EyeForAnEye, BORDER_VANTAGE);
        gBattleTemporaryFlag.vantage_order = true;
        return true;
    }
#endif

#if defined(SID_Vantage) && (COMMON_SKILL_VALID(SID_Vantage))
	if (BattleFastSkillTester(&gBattleTarget, SID_Vantage)) {
		if ((gBattleTarget.hpInitial * 2) < gBattleTarget.unit.maxHP) {
			RegisterBattleOrderSkill(SID_Vantage, BORDER_VANTAGE);
			gBattleTemporaryFlag.vantage_order = true;
			return true;
		}
	}
#endif

#if defined(SID_GaleWings) && (COMMON_SKILL_VALID(SID_GaleWings))
	if (BattleFastSkillTester(&gBattleTarget, SID_GaleWings)) {
		if (gBattleTarget.hpInitial == gBattleTarget.unit.maxHP) {
			RegisterBattleOrderSkill(SID_GaleWings, BORDER_VANTAGE);
			gBattleTemporaryFlag.vantage_order = true;
			return true;
		}
	}
#endif

#if defined(SID_PridefulWarrior) && (COMMON_SKILL_VALID(SID_PridefulWarrior))
	if (BattleFastSkillTester(&gBattleActor, SID_PridefulWarrior)) {
		/* actor can enable the foe to attack first */
		gBattleTemporaryFlag.vantage_order = true;
		return true;
	}
#endif

	return false;
}

STATIC_DECLAR u8 PostCalc_ModifyBattleOrderRoundMask(u8 round_mask)
{
	u8 ret = round_mask;

#if defined(SID_LastWord) && (COMMON_SKILL_VALID(SID_LastWord))
	if (!(round_mask & UNWIND_DOUBLE_ACT)) {
		if (BattleFastSkillTester(&gBattleActor, SID_LastWord)) {
			RegisterBattleOrderSkill(SID_LastWord, BORDER_ACT_TWICE);
			ret |= UNWIND_DOUBLE_ACT;
		}
	}

	if (!(round_mask & UNWIND_DOUBLE_TAR)) {
		if (BattleFastSkillTester(&gBattleTarget, SID_LastWord)) {
			RegisterBattleOrderSkill(SID_LastWord, BORDER_TAR_TWICE);
			ret |= UNWIND_DOUBLE_TAR;
		}
	}
#endif

	return ret;
}

void PreBattleGenerate_GenerateOrderFlags(void)
{
	gBattleFlagExt.round_mask = 0;

	/* Well, we need to setup battle speed for twice-order (double-attack) judgement */
	ComputeBattleUnitSpeed(&gBattleActor);
	ComputeBattleUnitSpeed(&gBattleTarget);

	if (CheckDesperationOrder())
		gBattleFlagExt.round_mask |= UNWIND_DESPERA;

	if (CheckVantageOrder())
		gBattleFlagExt.round_mask |= UNWIND_VANTAGE;

	if (CheckCanTwiceAttackOrder(&gBattleActor, &gBattleTarget))
		gBattleFlagExt.round_mask |= UNWIND_DOUBLE_ACT;

	if (CheckCanTwiceAttackOrder(&gBattleTarget, &gBattleActor))
		gBattleFlagExt.round_mask |= UNWIND_DOUBLE_TAR;

	gBattleFlagExt.round_mask = PostCalc_ModifyBattleOrderRoundMask(gBattleFlagExt.round_mask);
}

STATIC_DECLAR void RegenerateBattleOrderFlagsAfterCalc(void)
{
	int act_as = gBattleActor.battleSpeed;
	int tar_as = gBattleTarget.battleSpeed;

	int follow_up_threshold_act = get_battle_followup_speed_threshold();
	int follow_up_threshold_tar = get_battle_followup_speed_threshold();

#if defined(SID_Hasty) && (COMMON_SKILL_VALID(SID_Hasty))
        if (BattleFastSkillTester(&gBattleActor, SID_Hasty))
			follow_up_threshold_act = 1;
        if (BattleFastSkillTester(&gBattleTarget, SID_Hasty))
			follow_up_threshold_tar = 1;
#endif

	/**
	 * As battle speed may be modifed during pre-battle calc,
	 * so here we need to retry for speed judgement for twice-order
	 */
	if (gBattleTemporaryFlag.act_normal_judge_twice_order == true) {
		if ((act_as - tar_as) >= follow_up_threshold_act)
			gBattleFlagExt.round_mask |=  UNWIND_DOUBLE_ACT;
		else
			gBattleFlagExt.round_mask &= ~UNWIND_DOUBLE_ACT;
	}

	if (gBattleTemporaryFlag.tar_normal_judge_twice_order == true) {
		if ((tar_as - act_as) >= follow_up_threshold_tar)
			gBattleFlagExt.round_mask |=  UNWIND_DOUBLE_TAR;
		else
			gBattleFlagExt.round_mask &= ~UNWIND_DOUBLE_TAR;
	}
}

static inline void ClearBattleStateForUnwind(void)
{
    ClearBattleHits();
    gBattleHitIterator->info |= BATTLE_HIT_INFO_BEGIN;
}

static inline bool IsRealBattle(void)
{
    return (gBattleStats.config & BATTLE_CONFIG_REAL);
}

static inline void EndBattleEarly(void)
{
    gBattleHitIterator->info |= BATTLE_HIT_INFO_END;
}

typedef enum {
    ATTACK_NONE,
    ATTACK_ACTOR,
    ATTACK_TARGET
} AttackType;

static inline AttackType GetAttackTypeForStep(u8 cfg)
{
    if (cfg == ACT_ATTACK) return ATTACK_ACTOR;
    if (cfg == TAR_ATTACK) return ATTACK_TARGET;
    return ATTACK_NONE;
}

static inline void ApplyReturnAttackFlagIfNeeded(AttackType t)
{
    if (t == ATTACK_TARGET)
        gBattleHitIterator->attributes |= BATTLE_HIT_ATTR_RETALIATE;
}

static inline void IncrementAttackCount(AttackType t, int *actorCount, int *targetCount)
{
    if (t == ATTACK_ACTOR) (*actorCount)++;
    if (t == ATTACK_TARGET) (*targetCount)++;
}

static inline void TryRegisterCombatArt(struct BattleHit *start)
{
    int cid = GetCombatArtInForce(&gBattleActor.unit);
    if (COMBART_VALID(cid))
        RegisterEfxSkillCombatArt(GetBattleHitRound(start), cid);
}

static inline void ApplyFollowUpFlagIfSameAttacker(int i, const u8 *cfg)
{
    if (i != 0 && cfg[i-1] == cfg[i])
        gBattleHitIterator->attributes = BATTLE_HIT_ATTR_FOLLOWUP;
}

static inline void TryRegisterVantage(int i, struct BattleHit *start)
{
    if (i != 0) return;
    if (!(gBattleFlagExt.round_mask & UNWIND_VANTAGE)) return;
    if (!gBattleTemporaryFlag.vantage_order) return;

    RegisterActorEfxSkill(GetBattleHitRound(start), BattleOrderSkills[BORDER_VANTAGE]);
}

static inline void TryRegisterDesperation(int i, const u8 *cfg, struct BattleHit *start)
{
    if (i != 1) return;
    if (!(gBattleFlagExt.round_mask & UNWIND_DESPERA)) return;
    if (!gBattleTemporaryFlag.desperation_order) return;

    bool pattern =
        cfg[0] == ACT_ATTACK &&
        cfg[1] == ACT_ATTACK &&
        cfg[2] == TAR_ATTACK;

    if (pattern)
        RegisterActorEfxSkill(GetBattleHitRound(start), BattleOrderSkills[BORDER_DESPERATION]);
}

static inline void TryRegisterForcedDouble(AttackType t, int actor, int target, struct BattleHit *start)
{
    if (t == ATTACK_TARGET && target > 1 && gBattleTemporaryFlag.tar_force_twice_order)
        RegisterActorEfxSkill(GetBattleHitRound(start), BattleOrderSkills[BORDER_TAR_TWICE]);

    if (t == ATTACK_ACTOR && actor > 1 && gBattleTemporaryFlag.act_force_twice_order)
        RegisterActorEfxSkill(GetBattleHitRound(start), BattleOrderSkills[BORDER_ACT_TWICE]);
}

static inline void MarkBattleEnd(void)
{
    if (GetBattleHitRound(gBattleHitIterator) != 0) {
        struct BattleHit *pre = gBattleHitIterator - 1;
        pre->info |= BATTLE_HIT_INFO_FINISHES;
    }

    gBattleHitIterator->info |= BATTLE_HIT_INFO_END;
}

static inline bool HasDuel(struct BattleUnit *bu)
{
#if defined(SID_Duel) && COMMON_SKILL_VALID(SID_Duel)
    return BattleFastSkillTester(bu, SID_Duel);
#else
    return false;
#endif
}

static inline void DuelResetBattleHits(void)
{
    // Fully reset the battle-hit array just like the beginning of any battle
    ClearBattleHits();
    gBattleHitIterator->info |= BATTLE_HIT_INFO_BEGIN;
}

static bool DuelHitBufferOverflowed(void)
{
    return CheckBattleHitOverflow();
}


static int RunDuelCombatLoop(void)
{
    const u8 *roundOrder = BattleUnwindConfig[gBattleFlagExt.round_mask];

    int actor_count  = 0;
    int target_count = 0;

DuelRoundStart:

    while (true)
    {
        //---------------------------------------------------------
        // Execute ONE FULL ROUND (vanilla attack order)
        //---------------------------------------------------------
        for (int i = 0; i < 4; i++)
        {
            u8 step = roundOrder[i];
            if (step == NOP_ATTACK)
                break;

            struct BattleUnit *atk;
            struct BattleUnit *def;
            AttackType atype;

            if (step == ACT_ATTACK) {
                atk   = &gBattleActor;
                def   = &gBattleTarget;
                atype = ATTACK_ACTOR;
            } else {
                atk   = &gBattleTarget;
                def   = &gBattleActor;
                atype = ATTACK_TARGET;
            }

            if (!CheckCanContinueAttack(atk))
                continue;

            if (atype == ATTACK_TARGET)
                gBattleHitIterator->attributes |= BATTLE_HIT_ATTR_RETALIATE;

            struct BattleHit *roundStart = gBattleHitIterator;

            int stop = BattleGenerateRoundHits(atk, def);

            //-----------------------------------------------------
            // ★ NEW: Overflow protection for Duel
            //-----------------------------------------------------
            if (DuelHitBufferOverflowed())
            {
                // Reset buffer and restart the entire Duel round
                DuelResetBattleHits();
                actor_count = 0;
                target_count = 0;
                goto DuelRoundStart;
            }

            //-----------------------------------------------------
            // Original bookkeeping
            //-----------------------------------------------------
            if (atype == ATTACK_ACTOR) actor_count++;
            else                       target_count++;

            if (i == 0 && atype == ATTACK_ACTOR)
                TryRegisterCombatArt(roundStart);

            if (i != 0 && roundOrder[i-1] == roundOrder[i])
                gBattleHitIterator->attributes = BATTLE_HIT_ATTR_FOLLOWUP;

            TryRegisterVantage(i, roundStart);
            TryRegisterDesperation(i, roundOrder, roundStart);
            TryRegisterForcedDouble(atype, actor_count, target_count, roundStart);

            if (stop)
                goto DuelFinish;
            if (!CheckCanContinueAttack(def))
                goto DuelFinish;
        }

        //---------------------------------------------------------
        // End of this Duel round → start next
        //---------------------------------------------------------
    }

DuelFinish:
    return 1;
}

// -----------------------------------------------------------------------------
// TryExecuteComboAttackOnce
//
// - Runs the chain/combo attack logic once (the original "combo_atk_done" behavior).
// - Parameters:
//     atk        : attacker unit to test for ChainAttack
//     def        : defender unit (chain may be triggered by either unit)
//     comboDone  : pointer to a bool that tracks whether we've already executed the combo
//     pOldHit    : pointer to a struct BattleHit* in the caller that should be
//                  updated to gBattleHitIterator after the combo finishes
// - Returns:
//     true  => BattleComboGenerateHits() returned non-zero (signal to stop unwind loop)
//     false => either combo not triggered or it finished normally
// ----------------------------------------------------------------------------- 
static bool TryExecuteComboAttackOnce(
    struct BattleUnit *atk,
    struct BattleUnit *def,
    bool *comboDone,
    struct BattleHit **pOldHit)
{
    if (!comboDone || !pOldHit)
        return false;

    // Only run combo once
    if (!(*comboDone))
    {
        // Only consider ChainAttack if the skill is defined and valid
    #if (defined(SID_ChainAttack) && COMMON_SKILL_VALID(SID_ChainAttack))
        if (BattleFastSkillTester(atk, SID_ChainAttack) ||
            BattleFastSkillTester(def, SID_ChainAttack))
        {
            *comboDone = true;

            // BattleComboGenerateHits mirrors original behavior: non-zero -> stop unwind
            int ret = BattleComboGenerateHits();
            if (ret)
                return true;

            // Reload the caller's 'oldHit' to reflect new iterator position
            *pOldHit = gBattleHitIterator;
            LTRACEF("Combo end at round %d", GetBattleHitRound(*pOldHit));
        }
    #endif
    }

    return false;
}

bool TryTriggerAccostRoundRepeat(struct BattleUnit * attacker, struct BattleUnit * defender)
{
#if defined(SID_Accost) && COMMON_SKILL_VALID(SID_Accost)
    // Unit must actually have Accost
    if (!BattleFastSkillTester(attacker, SID_Accost))
        return false;

    // RN roll based on unit’s skill
    if (!Roll1RN((attacker->unit.spd - defender->unit.spd) + attacker->unit.curHP/2))
        return false;

    // Both sides must be able to continue fighting
    if (!CheckCanContinueAttack(&gBattleActor))
        return false;

    if (!CheckCanContinueAttack(&gBattleTarget))
        return false;

    return true;
#else
    return false;
#endif
}

LYN_REPLACE_CHECK(BattleUnwind);
void BattleUnwind(void)
{
    ClearBattleStateForUnwind();

    // Fake battle? End immediately.
    if (!IsRealBattle()) {
        EndBattleEarly();
        return;
    }

    RegenerateBattleOrderFlagsAfterCalc();

    //---------------------------------------------------------
    // DUEL OVERRIDE
    //---------------------------------------------------------
    if (HasDuel(&gBattleActor) || HasDuel(&gBattleTarget))
    {
        RunDuelCombatLoop();
        MarkBattleEnd();
        return;
    }

repeat_full_round:;   // <--- label for Accost repeat

    const u8 *config = BattleUnwindConfig[gBattleFlagExt.round_mask];
    int actor_count  = 0;
    int target_count = 0;

    bool combo_done = false;
    bool round_stopped = false;

    for (int i = 0; i < 4; i++)
    {
        struct BattleHit *roundStart = gBattleHitIterator;

        AttackType atkType = GetAttackTypeForStep(config[i]);
        if (atkType == ATTACK_NONE)
            break;

        struct BattleUnit *atk = (atkType == ATTACK_ACTOR) ? &gBattleActor : &gBattleTarget;
        struct BattleUnit *def = (atkType == ATTACK_ACTOR) ? &gBattleTarget : &gBattleActor;

        if (!CheckCanContinueAttack(atk))
            continue;

        ApplyReturnAttackFlagIfNeeded(atkType);

        if (TryExecuteComboAttackOnce(atk, def, &combo_done, &roundStart)) {
            round_stopped = true;
            break;
        }

        int stop = BattleGenerateRoundHits(atk, def);

        IncrementAttackCount(atkType, &actor_count, &target_count);

        if (i == 0)
            TryRegisterCombatArt(roundStart);

        ApplyFollowUpFlagIfSameAttacker(i, config);

        TryRegisterVantage(i, roundStart);
        TryRegisterDesperation(i, config, roundStart);
        TryRegisterForcedDouble(atkType, actor_count, target_count, roundStart);

        if (stop) {
            round_stopped = true;
            break;
        }
    }

    //---------------------------------------------------------
    // ACCOST — must occur AFTER a full round is completed
    //---------------------------------------------------------
#if defined(SID_Accost) && COMMON_SKILL_VALID(SID_Accost)
    if (!round_stopped)
    {
        bool actor_accost  = TryTriggerAccostRoundRepeat(&gBattleActor, &gBattleTarget);
        bool target_accost = TryTriggerAccostRoundRepeat(&gBattleTarget, &gBattleActor);

        if (actor_accost || target_accost)
        {
            // Repeat the entire round
            goto repeat_full_round;
        }
    }
#endif
    //---------------------------------------------------------

    MarkBattleEnd();
}

// ================================================================
// ENUM FOR RESULT STATE
// ================================================================
typedef enum BattleRoundResult {
    BATTLE_ROUND_ABORT = 0,
    BATTLE_ROUND_END   = 1
} BattleRoundResult;


// ================================================================
// HELPER: Determines if attacker can start combat and sets unarmedCombat out-flag
// Matches original logic exactly, including the #ifdef behavior.
// ================================================================
static bool CanInitiateCombat(struct BattleUnit* attacker, bool* outUnarmedCombat)
{
    *outUnarmedCombat = false;

#if defined(SID_UnarmedCombat) && (COMMON_SKILL_VALID(SID_UnarmedCombat))
    if (!BattleFastSkillTester(attacker, SID_UnarmedCombat))
    {
        if (!attacker->weapon)
            return false;
    }
    else
    {
        *outUnarmedCombat = true;
    }
#else
    if (!attacker->weapon)
        return false;
#endif

    return CheckCanContinueAttack(attacker);
}


// ================================================================
// HELPER: Applies round-based effect skills AFTER the first round
// ================================================================
static void ApplyRoundSkillEffects(int roundIndex, int roundNumber)
{
    if (roundIndex == 0)
        return;

    int skillId = DequeueRoundEfxSkill();

    if (COMMON_SKILL_VALID(skillId)) {
        LTRACEF("Round skill %02x registered at round %d", skillId, roundNumber);
        RegisterActorEfxSkill(roundNumber, skillId);
    }
}


// ================================================================
// HELPER: Handles battle hit overflow and returns whether it ended the sequence
// Preserves original hack behavior.
// ================================================================
static bool HandleBattleHitOverflow(void)
{
    if (!CheckBattleHitOverflow())
        return false;

    Error("Battle hit overflowed!");

    // Rewind one entry and mark it as finished
    gBattleHitIterator = gBattleHitIterator - 1;
    gBattleHitIterator->info |= (BATTLE_HIT_INFO_FINISHES | BATTLE_HIT_INFO_END);

    return true;
}


// ================================================================
// MAIN FUNCTION REWRITE WITH HELPERS
// ================================================================
LYN_REPLACE_CHECK(BattleGenerateRoundHits);
bool BattleGenerateRoundHits(struct BattleUnit *attacker, struct BattleUnit *defender)
{
    bool unarmedCombat    = false;
    int  roundCount       = 0;
    int  roundIndex       = 0;
    u32  baseAttrs        = 0;

    // ------------------------------------------------------------
    // 1. Validate that attacker can begin combat
    // ------------------------------------------------------------
    if (!CanInitiateCombat(attacker, &unarmedCombat))
        return BATTLE_ROUND_ABORT;

    // ------------------------------------------------------------
    // 2. Prepare per-round state
    // ------------------------------------------------------------
    ResetRoundEfxSkills();

    baseAttrs  = gBattleHitIterator->attributes;
    roundCount = GetBattleUnitHitCount(attacker);

    LTRACEF("Battle rounds: %d", roundCount);


    // ------------------------------------------------------------
    // 3. Process each hit round
    // ------------------------------------------------------------
    for (roundIndex = 0; roundIndex < roundCount; ++roundIndex)
    {
        int roundNumber = GetBattleHitRound(gBattleHitIterator);

        // Restore baseline hit attributes for the round
        gBattleHitIterator->attributes |= baseAttrs;

        // Apply EFX skills for rounds > 0
        ApplyRoundSkillEffects(roundIndex, roundNumber);

        // --------------------------------------------------------
        // Generate the hit
        // If this returns true, combat ends (old behavior)
        // --------------------------------------------------------
        if (BattleGenerateHit(attacker, defender))
            return BATTLE_ROUND_END;

        // --------------------------------------------------------
        // Overflow protection
        // --------------------------------------------------------
        if (HandleBattleHitOverflow())
            return BATTLE_ROUND_END;

        // --------------------------------------------------------
        // Mid-round continuation check
        // --------------------------------------------------------
        if (!CheckCanContinueAttack(attacker) && !unarmedCombat)
            return BATTLE_ROUND_ABORT;
    }

    // ------------------------------------------------------------
    // End of all rounds without early finish
    // Original function returns FALSE here → abort-class result
    // ------------------------------------------------------------
    return BATTLE_ROUND_ABORT;
}

LYN_REPLACE_CHECK(BattleGetFollowUpOrder);
bool BattleGetFollowUpOrder(struct BattleUnit **outAttacker, struct BattleUnit **outDefender)
{
	if (CheckCanTwiceAttackOrder(&gBattleActor, &gBattleTarget)) {
		*outAttacker = &gBattleActor;
		*outDefender = &gBattleTarget;
		return true;
	} else if (CheckCanTwiceAttackOrder(&gBattleTarget, &gBattleActor)) {
		*outAttacker = &gBattleTarget;
		*outDefender = &gBattleActor;
		return true;
	}
	return false;
}

LYN_REPLACE_CHECK(GetBattleUnitHitCount);
int GetBattleUnitHitCount(struct BattleUnit *actor)
{
	FORCE_DECLARE struct BattleUnit *target = (actor == &gBattleActor)
										    ? &gBattleTarget
										    : &gBattleActor;
	int result = 1;
	u8 cid;

	if (BattleCheckBraveEffect(actor))
		result = result + 1;

#if defined(SID_RuinedBladePlus) && (COMMON_SKILL_VALID(SID_RuinedBladePlus))
	if (BattleFastSkillTester(actor, SID_RuinedBladePlus)) {
		EnqueueRoundEfxSkill(SID_RuinedBladePlus);
		result = result + SKILL_EFF2(SID_RuinedBladePlus);
	}
#endif

#if defined(SID_LoadstarRush) && (COMMON_SKILL_VALID(SID_LoadstarRush))
    if (gActionData.unk08 == SID_LoadstarRush && !CheckBitUES(gActiveUnit, UES_BIT_LOADSTAR_RUSH_SKILL_USED) && BattleFastSkillTester(actor, SID_LoadstarRush))
    {
        EnqueueRoundEfxSkill(SID_LoadstarRush);
        result = result + SKILL_EFF0(SID_LoadstarRush);
    }
#endif


#if defined(SID_Astra) && (COMMON_SKILL_VALID(SID_Astra))
	if (actor == &gBattleActor && CheckBattleSkillActivate(actor, target, SID_Astra, actor->unit.spd)) {
		EnqueueRoundEfxSkill(SID_Astra);
		gBattleActorGlobalFlag.skill_activated_astra = true;
		result = result + SKILL_EFF0(SID_Astra);
	}
#endif

    /* Check combat-art */
    cid = GetCombatArtInForce(&actor->unit);
    if (&gBattleActor == actor && COMBART_VALID(cid))
    {
        if (GetCombatArtInfo(cid)->quintuple_attack == COMBART_QUINTUPLE_ENABLED)
            result = result + 4;
    }

#if defined(SID_Adept) && (COMMON_SKILL_VALID(SID_Adept))
	if (BattleFastSkillTester(actor, SID_Adept) && actor->hpInitial == actor->unit.maxHP) {
		EnqueueRoundEfxSkill(SID_Adept);
		result = result + 1;
	}
#endif

#if defined(SID_ChargePlus) && (COMMON_SKILL_VALID(SID_ChargePlus))
		if (BattleFastSkillTester(actor, SID_ChargePlus)) {
			if (MovGetter(gActiveUnit) == gActionData.moveCount)
				result = result + 1;
		}
#endif

#if defined(SID_DoubleLion) && (COMMON_SKILL_VALID(SID_DoubleLion))
	if (actor == &gBattleActor && BattleFastSkillTester(actor, SID_DoubleLion) && actor->hpInitial == actor->unit.maxHP) {
		gBattleActorGlobalFlag.skill_activated_double_lion = true;
		EnqueueRoundEfxSkill(SID_DoubleLion);
		result = result + 1;
	}
#endif

#if defined(SID_LastStand) && (COMMON_SKILL_VALID(SID_LastStand))
    if (actor == &gBattleActor && BattleFastSkillTester(actor, SID_LastStand) && actor->hpInitial == 1)
    {
        EnqueueRoundEfxSkill(SID_LastStand);
        result = result + 1;
    }
#endif

#if defined(SID_Echo) && (COMMON_SKILL_VALID(SID_Echo))
    if (actor == &gBattleActor && BattleFastSkillTester(actor, SID_Echo))
    {
        EnqueueRoundEfxSkill(SID_Echo);
        gBattleActorGlobalFlag.skill_activated_echo = true;
        result = result + SKILL_EFF0(SID_Echo);
    }
#endif

#if defined(SID_Flurry) && (COMMON_SKILL_VALID(SID_Flurry))
    if (BattleFastSkillTester(actor, SID_Flurry))
    {
        EnqueueRoundEfxSkill(SID_Echo);
        int extraAttacks = (actor->battleSpeed - target->battleSpeed) / 4;
        result = result + extraAttacks;
    }
#endif

#if defined(SID_UnarmedCombat) && (COMMON_SKILL_VALID(SID_UnarmedCombat))
    if (BattleFastSkillTester(actor, SID_UnarmedCombat))
	{
        result = result + 1;
	}
#endif

	if (GetUnitStatusIndex(GetUnit(actor->unit.index)) == NEW_UNIT_STATUS_QUICKEN) {
		result = result + 1;
	}

	return result;
}

LYN_REPLACE_CHECK(BattleCheckBraveEffect);
int BattleCheckBraveEffect(struct BattleUnit* attacker) {
    bool braveEffect = false;
    
    if (attacker->weaponAttributes & IA_BRAVE)
    {
        braveEffect = true;
    }
    else
    {
#if defined(SID_DualWieldPlus) && (COMMON_SKILL_VALID(SID_DualWieldPlus))
        if (BattleFastSkillTester(attacker, SID_DualWieldPlus))
        {
            for (int i = 1; i < UNIT_MAX_INVENTORY; i++)
            {
                if (GetItemMight(attacker->unit.items[i]) > 0 && CanUnitUseWeapon(GetUnit(attacker->unit.index), attacker->unit.items[i]))
                {
                    if (GetItemAttributes(attacker->unit.items[i]) & IA_BRAVE)
                    {
                        braveEffect = true;
                        break;
                    }
                }
            }
        }
#endif 
    }

    if (braveEffect)
        gBattleHitIterator->attributes |= BATTLE_HIT_ATTR_BRAVE;

    return braveEffect;
}