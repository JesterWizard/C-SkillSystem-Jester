#include "global.h"
#include "bmbattle.h"
#include "variables.h"

extern const u8 RestoreHpOnLevelUpEnabled;

static int BattleUnitLeveledUp(const struct BattleUnit *bu)
{
	return (int)bu->unit.level > (int)bu->levelPrevious;
}

static void FillUnitHp(struct Unit *unit)
{
	if (unit == NULL || unit->curHP == 0 || unit->curHP >= unit->maxHP)
		return;

	unit->curHP = unit->maxHP;
}

/**
 * Vanilla BattleApplyUnitUpdates.
 * When anyone in the battle levels up, the player participant is refilled:
 * the actor on player phase, the target on enemy or NPC phase.
 * Dead units stay dead.
 */
void BattleApplyUnitUpdates_RestoreHp(void)
{
	struct Unit *actor = GetUnit(gBattleActor.unit.index);
	struct Unit *target = GetUnit(gBattleTarget.unit.index);

	if (gBattleActor.canCounter)
		gBattleActor.unit.items[gBattleActor.weaponSlotIndex] = gBattleActor.weapon;

	if (gBattleTarget.canCounter)
		gBattleTarget.unit.items[gBattleTarget.weaponSlotIndex] = gBattleTarget.weapon;

	UpdateUnitFromBattle(actor, &gBattleActor);

	if (target)
		UpdateUnitFromBattle(target, &gBattleTarget);
	else
		UpdateObstacleFromBattle(&gBattleTarget);

	if (!RestoreHpOnLevelUpEnabled)
		return;

	if (!BattleUnitLeveledUp(&gBattleActor) && !BattleUnitLeveledUp(&gBattleTarget))
		return;

	if (gPlaySt.faction == FACTION_BLUE)
		FillUnitHp(actor);
	else
		FillUnitHp(target);
}
