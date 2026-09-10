#include "global.h"
#include "bmbattle.h"
#include "bmitem.h"
#include "bmunit.h"
#include "constants/items.h"

/**
 * Vanilla BattleGetFollowUpOrder with Wary Fighter.
 * Character IDs come from WaryFighterUnits.event ($FF = all units).
 * If a listed combatant is in the fight, nobody follows up.
 */
extern const u8 WaryFighterUnits[];

static int UnitHasWaryFighter(const struct BattleUnit *bu)
{
	const u8 *it;
	u8 pid;

	if (!bu->unit.pCharacterData)
		return FALSE;

	pid = UNIT_CHAR_ID(&bu->unit);

	for (it = WaryFighterUnits; *it != 0; it++) {
		if (*it == 0xFF || *it == pid)
			return TRUE;
	}

	return FALSE;
}

static int WaryFighterBlocksFollowUp(void)
{
	return UnitHasWaryFighter(&gBattleActor) || UnitHasWaryFighter(&gBattleTarget);
}

s8 BattleGetFollowUpOrder_WaryFighter(struct BattleUnit **outAttacker, struct BattleUnit **outDefender)
{
	int speedDiff;

	if (WaryFighterBlocksFollowUp())
		return FALSE;

	if (gBattleTarget.battleSpeed > 250)
		return FALSE;

	speedDiff = gBattleActor.battleSpeed - gBattleTarget.battleSpeed;

	if (speedDiff < 0)
		speedDiff = -speedDiff;

	if (speedDiff < BATTLE_FOLLOWUP_SPEED_THRESHOLD)
		return FALSE;

	if (gBattleActor.battleSpeed > gBattleTarget.battleSpeed) {
		*outAttacker = &gBattleActor;
		*outDefender = &gBattleTarget;
	} else {
		*outAttacker = &gBattleTarget;
		*outDefender = &gBattleActor;
	}

	if (GetItemWeaponEffect((*outAttacker)->weaponBefore) == WPN_EFFECT_HPHALVE)
		return FALSE;

	if (GetItemIndex((*outAttacker)->weapon) == ITEM_MONSTER_STONE)
		return FALSE;

	return TRUE;
}
