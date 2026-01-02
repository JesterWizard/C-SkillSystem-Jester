#include <common-chax.h>
#include <battle-system.h>
#include <combat-art.h>
#include <skill-system.h>
#include <constants/combat-arts.h>
#include <constants/skills.h>

#include <gaiden-magic.h>
#include "bwl.h"

bool TryBattleHpCost(struct BattleUnit *bu, int hp_cost)
{
#ifdef CONFIG_MP_SYSTEM
	if (CheckGaidenMagicAttack(bu)) {
		struct NewBwl * bwl = GetNewBwl(UNIT_CHAR_ID(GetUnit(bu->unit.index)));

		if (hp_cost > bwl->currentMP)
			return false;
	}
	else
	{
		if (hp_cost >= bu->unit.curHP)
			return false;
	}
#else
	if (hp_cost >= bu->unit.curHP)
		return false;
#endif

	return true;
}

bool AddBattleHpCost(struct BattleUnit *bu, int round, int hp_cost)
{
	if (!TryBattleHpCost(bu, hp_cost))
		return false;

#ifdef CONFIG_MP_SYSTEM

	if (CheckGaidenMagicAttack(bu)) {

#if (defined(SID_TurboMP) && COMMON_SKILL_VALID(SID_TurboMP))
		if (SkillTester(GetUnit(bu->unit.index), SID_TurboMP))
			hp_cost *= 2;
#endif

#if (defined(SID_HalfMP) && COMMON_SKILL_VALID(SID_HalfMP))
		if (SkillTester(GetUnit(bu->unit.index), SID_HalfMP))
			hp_cost /= 2;
#endif

#if (defined(SID_BloodMagic) && COMMON_SKILL_VALID(SID_BloodMagic))
		if (SkillTester(GetUnit(bu->unit.index), SID_BloodMagic))
			if (hp_cost >= bu->unit.curHP)
				bu->unit.curHP = 1;
			else
				bu->unit.curHP -= hp_cost;
		else
		{
			struct NewBwl * bwl = GetNewBwl(UNIT_CHAR_ID(GetUnit(bu->unit.index)));

			if (hp_cost >= bwl->currentMP)
				bwl->currentMP = 0;
			else
				bwl->currentMP -= hp_cost;			
		}
#else
		struct NewBwl * bwl = GetNewBwl(UNIT_CHAR_ID(GetUnit(bu->unit.index)));
		bwl->currentMP -= hp_cost;
#endif
	}
	else
		if (hp_cost >= bu->unit.curHP)
			bu->unit.curHP = 1;
		else
			bu->unit.curHP -= hp_cost;
#else
	bu->unit.curHP -= hp_cost;
#endif

/* JESTER - Turn off the battle hit from gaiden magic if we're using MP instead of HP */
#ifndef CONFIG_MP_SYSTEM
	GetExtBattleHit(round)->hp_cost += hp_cost;
#endif

	return true;
}

void ForceAddBattleHpCost(struct BattleUnit *bu, int round, int hp_cost)
{
	
#ifdef CONFIG_MP_SYSTEM
	if (CheckGaidenMagicAttack(bu)) {
		struct NewBwl * bwl = GetNewBwl(UNIT_CHAR_ID(GetUnit(bu->unit.index)));
		
		if (hp_cost > bwl->currentMP)
			hp_cost = bwl->currentMP - 1;
	}
	else
	{
		if (hp_cost >= bu->unit.curHP)
			hp_cost = bu->unit.curHP - 1;		
	}
#else
	if (hp_cost >= bu->unit.curHP)
		hp_cost = bu->unit.curHP - 1;
#endif

	AddBattleHpCost(bu, round, hp_cost);
}

void BattleGenerateHitHpCost(struct BattleUnit *attacker, struct BattleUnit *defender)
{
	BattleGenerateHitHpCostForGaidenMagic(attacker, defender);
	BattleHit_CalcWeaponHpCost(attacker, defender);
}
