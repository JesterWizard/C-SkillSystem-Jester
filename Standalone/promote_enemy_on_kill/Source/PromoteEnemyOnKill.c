#include "global.h"
#include "bmbattle.h"
#include "bmunit.h"

extern const u8 PromoteEnemyOnKillEnabled;
extern const u8 PromoteEnemyBoost;

static void TryPromoteEnemyOnKill(void)
{
	struct Unit* enemyUnit;

	if (!PromoteEnemyOnKillEnabled)
		return;

	enemyUnit = GetUnit(gBattleActor.unit.index);

	if (UNIT_FACTION(enemyUnit) != FACTION_RED)
		return;

	if (UNIT_CATTRIBUTES(enemyUnit) & CA_PROMOTED)
		return;

	ApplyUnitDefaultPromotion(enemyUnit);

	enemyUnit->maxHP += PromoteEnemyBoost;
	enemyUnit->curHP += PromoteEnemyBoost;
	enemyUnit->pow += PromoteEnemyBoost;
	enemyUnit->_u47 += PromoteEnemyBoost;
	enemyUnit->skl += PromoteEnemyBoost;
	enemyUnit->spd += PromoteEnemyBoost;
	enemyUnit->lck += PromoteEnemyBoost;
	enemyUnit->def += PromoteEnemyBoost;
	enemyUnit->res += PromoteEnemyBoost;
}

/**
 * Vanilla BattleGenerateHit with promote-enemy-on-kill:
 * when an unpromoted enemy kills a unit, auto-promote that enemy and boost stats.
 */
s8 BattleGenerateHit_PromoteEnemyOnKill(struct BattleUnit* attacker, struct BattleUnit* defender)
{
	if (attacker == &gBattleTarget)
		gBattleHitIterator->info |= BATTLE_HIT_INFO_RETALIATION;

	BattleUpdateBattleStats(attacker, defender);
	BattleGenerateHitTriangleAttack(attacker, defender);
	BattleGenerateHitAttributes(attacker, defender);
	BattleGenerateHitEffects(attacker, defender);

	if (attacker->unit.curHP == 0 || defender->unit.curHP == 0) {
		attacker->wexpMultiplier++;
		gBattleHitIterator->info |= BATTLE_HIT_INFO_FINISHES;

		if (gBattleTarget.unit.curHP != 0) {
			gBattleHitIterator++;
			return true;
		}

		TryPromoteEnemyOnKill();

		gBattleHitIterator->info |= BATTLE_HIT_INFO_KILLS_TARGET;
		gBattleHitIterator++;
		return true;
	}

	if (defender->statusOut == UNIT_STATUS_PETRIFY || defender->statusOut == UNIT_STATUS_13) {
		gBattleHitIterator->info |= BATTLE_HIT_INFO_FINISHES;
		gBattleHitIterator++;
		return true;
	}

	gBattleHitIterator++;
	return false;
}
