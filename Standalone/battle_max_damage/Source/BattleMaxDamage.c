#include "global.h"
#include "bmunit.h"
#include "bmmind.h"
#include "bmbattle.h"

/* Defined in Installer.event (edit there, no recompile). */
extern const u8 BattleMaxDamageValue;

static void CapForecastAttack(struct BattleUnit *attacker, struct BattleUnit *defender)
{
	if (attacker->battleAttack - defender->battleDefense > BattleMaxDamageValue)
		attacker->battleAttack = defender->battleDefense + BattleMaxDamageValue;
}

/**
 * Vanilla BattleGenerate. In a simulated battle (the battle forecast and the
 * AI), each side's attack is lowered so that attack - defense never shows
 * more than the damage cap, as the C Skill System forecast does. Real
 * battles keep their attack: the cap there is applied per hit by the
 * patched immediates in BattleGenerateHitAttributes.
 */
void BattleGenerate_MaxDamage(struct Unit *actor, struct Unit *target)
{
	(void)actor;

	ComputeBattleUnitStats(&gBattleActor, &gBattleTarget);
	ComputeBattleUnitStats(&gBattleTarget, &gBattleActor);

	ComputeBattleUnitEffectiveStats(&gBattleActor, &gBattleTarget);
	ComputeBattleUnitEffectiveStats(&gBattleTarget, &gBattleActor);

	if (target == NULL)
		ComputeBattleObstacleStats();

	if (gBattleStats.config & BATTLE_CONFIG_SIMULATE) {
		CapForecastAttack(&gBattleActor, &gBattleTarget);
		CapForecastAttack(&gBattleTarget, &gBattleActor);
	}

	if ((gBattleStats.config & BATTLE_CONFIG_REAL) && (gActionData.scriptedBattleHits))
		BattleUnwindScripted();
	else
		BattleUnwind();
}
