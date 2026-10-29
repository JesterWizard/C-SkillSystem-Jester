#include "global.h"
#include "bmitem.h"
#include "bmunit.h"
#include "bmbattle.h"

/**
 * Vanilla ComputeBattleUnitSpeed, plus the C Skill System
 * s_rank_weapon_no_weight rule: a weapon the unit has S rank in weighs
 * nothing, so it never lowers attack speed.
 */
void ComputeBattleUnitSpeed_SRankNoWeight(struct BattleUnit *bu)
{
	int effWt = GetItemWeight(bu->weaponBefore);
	int wType = GetItemType(bu->weaponBefore);

	if (bu->weaponBefore && wType < 8 && bu->unit.ranks[wType] >= WPN_EXP_S)
		effWt = 0;
	else
		effWt -= bu->unit.conBonus;

	if (effWt < 0)
		effWt = 0;

	bu->battleSpeed = bu->unit.spd - effWt;

	if (bu->battleSpeed < 0)
		bu->battleSpeed = 0;
}
