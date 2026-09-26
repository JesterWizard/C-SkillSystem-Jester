#include "global.h"
#include "bmbattle.h"
#include "bmitem.h"
#include "constants/items.h"

extern const u8 LastWeaponHitCritEnabled;

/* Vanilla durability cost is 1. Combat-art costs are not part of clean FE8U. */
static int IsLastWeaponHit(struct BattleUnit *attacker)
{
	int uses;

	if (!LastWeaponHitCritEnabled)
		return 0;

	if (attacker == NULL || attacker->weapon == 0)
		return 0;

	if (GetItemAttributes(attacker->weapon) & IA_UNBREAKABLE)
		return 0;

	uses = GetItemUses(attacker->weapon);
	if (uses <= 0 || uses >= 0xFF)
		return 0;

	return uses <= 1;
}

/**
 * Vanilla ComputeBattleUnitEffectiveCritRate.
 * A weapon on its last use shows 100 crit in the forecast.
 * Negate-crit items and monster stones are overridden for that strike.
 */
void ComputeBattleUnitEffectiveCritRate_LastWeapon(struct BattleUnit *attacker, struct BattleUnit *defender)
{
	int item, i;

	attacker->battleEffectiveCritRate = attacker->battleCritRate - defender->battleDodgeRate;

	if (GetItemIndex(attacker->weapon) == ITEM_MONSTER_STONE)
		attacker->battleEffectiveCritRate = 0;

	if (attacker->battleEffectiveCritRate < 0)
		attacker->battleEffectiveCritRate = 0;

	for (i = 0; i < UNIT_ITEM_COUNT; ++i) {
		item = defender->unit.items[i];
		if (item == 0)
			break;

		if (GetItemAttributes(item) & IA_NEGATE_CRIT) {
			attacker->battleEffectiveCritRate = 0;
			break;
		}
	}

	if (IsLastWeaponHit(attacker))
		attacker->battleEffectiveCritRate = 100;
}

/**
 * Vanilla BattleUpdateBattleStats.
 * Re-checked every strike so a follow-up crits once remaining uses
 * drop to 1, even if the forecast was computed at 2 or more uses.
 */
void BattleUpdateBattleStats_LastWeapon(struct BattleUnit *attacker, struct BattleUnit *defender)
{
	gBattleStats.attack = attacker->battleAttack;
	gBattleStats.defense = defender->battleDefense;
	gBattleStats.hitRate = attacker->battleEffectiveHitRate;
	gBattleStats.critRate = attacker->battleEffectiveCritRate;
	gBattleStats.silencerRate = attacker->battleSilencerRate;

	if (IsLastWeaponHit(attacker))
		gBattleStats.critRate = 100;
}
