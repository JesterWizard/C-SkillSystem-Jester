#include "common-chax.h"
#include "strmag.h"
#include "unit-expa.h"
#include "battle-system.h"
#include "skill-system.h"
#include "constants/skills.h"
#include "bwl.h"

void BattleDamageCalc_DefenderSkills(struct BattleUnit* attacker, struct BattleUnit* defender)
{
	_maybe_unused int tmp0, tmp1;
	int _skill_list_cnt;
	struct SkillList* list;

	/**
	 * Skip arena judgement
	 */
	if (gBattleStats.config & BATTLE_CONFIG_ARENA)
		return;

	list = GetUnitSkillList(&defender->unit);

#if defined(SID_Symbiosis) && (COMMON_SKILL_VALID(SID_Symbiosis))
	if (BattleFastSkillTester(defender, SID_Symbiosis))
	{
		struct SkillList * rescueeList = GetUnitSkillList(GetUnit(defender->unit.rescue));

		for (int i = 0; i < rescueeList->amt; i++) 
			list->sid[list->amt++] = rescueeList->sid[i];
	}
#endif

	for (_skill_list_cnt = 0; _skill_list_cnt < list->amt; _skill_list_cnt++) {
		switch (list->sid[_skill_list_cnt]) {
#if (defined(SID_GreatShield) && (COMMON_SKILL_VALID(SID_GreatShield)))
		case SID_GreatShield:
			if (CheckBattleSkillActivate(defender, attacker, SID_GreatShield, defender->unit.skl)) {
				RegisterTargetEfxSkill(GetBattleHitRound(gBattleHitIterator), SID_GreatShield);
				gBattleHitIterator->attributes |= BATTLE_HIT_ATTR_GREATSHLD;
				gBattleStats.defense = BATTLE_MAX_DAMAGE;
			}
			break;
#endif

#if (defined(SID_TowerShieldPlus) && (COMMON_SKILL_VALID(SID_TowerShieldPlus)))
		case SID_TowerShieldPlus:
			if (gBattleStats.range > 1) {
				RegisterTargetEfxSkill(GetBattleHitRound(gBattleHitIterator), SID_TowerShieldPlus);
				gBattleHitIterator->attributes |= BATTLE_HIT_ATTR_GREATSHLD;
				gBattleStats.defense = BATTLE_MAX_DAMAGE;
			}
			break;
#endif

#if (defined(SID_Dazzling) && (COMMON_SKILL_VALID(SID_Dazzling)))
		case SID_Dazzling:
			if (gBattleStats.range >= 3) {
				RegisterTargetEfxSkill(GetBattleHitRound(gBattleHitIterator), SID_Dazzling);
				gBattleHitIterator->attributes |= BATTLE_HIT_ATTR_GREATSHLD;
				gBattleStats.defense = BATTLE_MAX_DAMAGE;
			}
			break;
#endif

#if (defined(SID_Aegis) && (COMMON_SKILL_VALID(SID_Aegis)))
		case SID_Aegis:
			if (IsMagicAttack(attacker) && CheckBattleSkillActivate(defender, attacker, SID_Aegis, defender->unit.skl)) {
				RegisterTargetEfxSkill(GetBattleHitRound(gBattleHitIterator), SID_Aegis);
				gBattleHitIterator->attributes |= BATTLE_HIT_ATTR_GREATSHLD;
				gBattleStats.defense = BATTLE_MAX_DAMAGE;
			}
			break;
#endif

#if (defined(SID_Pavise) && (COMMON_SKILL_VALID(SID_Pavise)))
		case SID_Pavise:
			if (!IsMagicAttack(attacker) && CheckBattleSkillActivate(defender, attacker, SID_Pavise, defender->unit.skl)) {
				RegisterTargetEfxSkill(GetBattleHitRound(gBattleHitIterator), SID_Pavise);
				gBattleHitIterator->attributes |= BATTLE_HIT_ATTR_GREATSHLD;
				gBattleStats.defense = BATTLE_MAX_DAMAGE;
			}
			break;
#endif

#if (defined(SID_Gambit) && (COMMON_SKILL_VALID(SID_Gambit)))
		case SID_Gambit:
			if (gBattleStats.range == 1)
				gDmg.crit_correction -= SKILL_EFF0(SID_Gambit);

			break;
#endif

#if (defined(SID_MagicGambit) && (COMMON_SKILL_VALID(SID_MagicGambit)))
		case SID_MagicGambit:
			if (gBattleStats.range > 1)
				gDmg.crit_correction -= SKILL_EFF0(SID_MagicGambit);

			break;
#endif

#if (defined(SID_Multiscale) && (COMMON_SKILL_VALID(SID_Multiscale)))
		case SID_Multiscale:
			if (defender->unit.curHP == defender->unit.maxHP) {
				RegisterTargetEfxSkill(GetBattleHitRound(gBattleHitIterator), SID_Multiscale);
				gDmg.decrease += DAMAGE_DECREASE(SKILL_EFF0(SID_Multiscale));
			}
			break;
#endif

#if defined(SID_Expertise) && (COMMON_SKILL_VALID(SID_Expertise))
		case SID_Expertise:
			if (gDmg.crit_atk)
				gDmg.decrease += DAMAGE_DECREASE(SKILL_EFF0(SID_Expertise));

			break;
#endif

#if defined(SID_GuardBearing) && (COMMON_SKILL_VALID(SID_GuardBearing))
		case SID_GuardBearing:
			if (!AreUnitsAllied(defender->unit.index, gPlaySt.faction) && GetBattleGlobalFlags(attacker)->round_cnt_hit == 1)
			{

				RegisterTargetEfxSkill(GetBattleHitRound(gBattleHitIterator), SID_GuardBearing);
				gDmg.decrease += DAMAGE_DECREASE(SKILL_EFF0(SID_GuardBearing));
			}
			break;
#endif

#if (defined(SID_CounterRoar) && (COMMON_SKILL_VALID(SID_CounterRoar)))
		case SID_CounterRoar:
			if (GetBattleGlobalFlags(attacker)->round_cnt_hit == 1)
				gDmg.decrease += DAMAGE_DECREASE(SKILL_EFF0(SID_CounterRoar));
			else
				gDmg.decrease += DAMAGE_DECREASE(SKILL_EFF1(SID_CounterRoar));

			break;
#endif

#if (defined(SID_DragonsWrath) && (COMMON_SKILL_VALID(SID_DragonsWrath)))
		case SID_DragonsWrath:
			if (GetBattleGlobalFlags(attacker)->round_cnt_hit == 1)
				gDmg.decrease += DAMAGE_DECREASE(SKILL_EFF0(SID_DragonsWrath));

			break;
#endif

#if (defined(SID_CrusaderWard) && (COMMON_SKILL_VALID(SID_CrusaderWard)))
		case SID_CrusaderWard:
			if (GetBattleGlobalFlags(attacker)->round_cnt_hit > 1 && gBattleStats.range > 1)
				gDmg.decrease += DAMAGE_DECREASE(SKILL_EFF0(SID_CrusaderWard));

			break;
#endif

#if (defined(SID_ShieldPulse) && (COMMON_SKILL_VALID(SID_ShieldPulse)))
		case SID_ShieldPulse:
			if (gDmg.crit_atk) {
				RegisterTargetEfxSkill(GetBattleHitRound(gBattleHitIterator), SID_ShieldPulse);
				gDmg.decrease += DAMAGE_DECREASE(SKILL_EFF0(SID_ShieldPulse));
			}
			break;
#endif

#if (defined(SID_BarricadePlus) && (COMMON_SKILL_VALID(SID_BarricadePlus)))
		case SID_BarricadePlus:
			if (GetBattleGlobalFlags(attacker)->round_cnt_hit >= 2) {
				int _i, _reduction = SKILL_EFF0(SID_BarricadePlus);
				int _base = _reduction;

				for (_i = 0; _i < GetBattleGlobalFlags(attacker)->round_cnt_hit - 1; _i++) {
					_base = _base / 2;
					_reduction += _base;
				}
				gDmg.decrease += DAMAGE_DECREASE(_reduction);
			}
			break;
#endif

#if (defined(SID_Barricade) && (COMMON_SKILL_VALID(SID_Barricade)))
		case SID_Barricade:
			if (defender->unit.curHP < defender->hpInitial)
				gDmg.decrease += DAMAGE_DECREASE(SKILL_EFF0(SID_Barricade));

			break;
#endif

#if defined(SID_DamageToMP) && (COMMON_SKILL_VALID(SID_DamageToMP))
		case SID_DamageToMP:
			if (gpKernelDesignerConfig->mp_system == true)
			{
				struct NewBwl* bwl = GetNewBwl(UNIT_CHAR_ID(GetUnit(defender->unit.index)));

				if (bwl != NULL && bwl->currentMP > 0)
				{
					gActionData.unk08 = SID_DamageToMP;
					gBattleHitIterator->attributes |= BATTLE_HIT_ATTR_GREATSHLD;
				}
			}
			break;
#endif
		case MAX_SKILL_NUM:
			Fatal("ENOSUPP");
			break;

		default:
			break;
		}
	}
}
