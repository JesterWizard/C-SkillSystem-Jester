#include "global.h"
#include "bmbattle.h"

/* Defined in Installer.event (edit there, no recompile). */
extern const u8 HitDecreaseOnRangePenalty; /* hit lost per 2 tiles of distance */

/**
 * Vanilla ComputeBattleUnitEffectiveHitRate, plus the C Skill System
 * hit_decrease_on_range rule: when the initiating unit attacks from 3 or
 * more tiles away (not with a ballista), its hit drops by
 * (distance / 2) * penalty before the 0..100 clamp.
 */
void ComputeBattleUnitEffectiveHitRate_RangePenalty(struct BattleUnit *attacker, struct BattleUnit *defender)
{
	attacker->battleEffectiveHitRate = attacker->battleHitRate - defender->battleAvoidRate;

	if (gBattleStats.range > 2 && attacker == &gBattleActor && !(gBattleStats.config & BATTLE_CONFIG_BALLISTA))
		attacker->battleEffectiveHitRate -= (gBattleStats.range / 2) * HitDecreaseOnRangePenalty;

	if (attacker->battleEffectiveHitRate > 100)
		attacker->battleEffectiveHitRate = 100;

	if (attacker->battleEffectiveHitRate < 0)
		attacker->battleEffectiveHitRate = 0;
}
