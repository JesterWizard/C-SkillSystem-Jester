#include "global.h"
#include "bmbattle.h"
#include "bmitem.h"
#include "bmunit.h"
#include "constants/items.h"

/**
 * WaryFighterGlobalMode: 0 = unit list, 1 = everyone always-on, 2 = everyone enemy-init.
 * List entries are (pid, version). Version 1 = always-on, 2 = enemy-init.
 * Specific pid beats $FF. Always-on beats enemy-init if both combatants differ.
 */
#define WARY_MODE_LIST        0
#define WARY_VER_ALWAYS       1
#define WARY_VER_ENEMY_INIT   2

extern const u8 WaryFighterGlobalMode;
extern const u8 WaryFighterUnits[];

static int LookupUnitWaryVersion(const struct BattleUnit *bu)
{
	const u8 *it;
	u8 pid;
	int wildcard = 0;

	if (!bu->unit.pCharacterData)
		return 0;

	pid = UNIT_CHAR_ID(&bu->unit);

	for (it = WaryFighterUnits; it[0] != 0; it += 2) {
		if (it[0] == pid)
			return it[1];

		if (it[0] == 0xFF)
			wildcard = it[1];
	}

	return wildcard;
}

static int GetUnitWaryVersion(const struct BattleUnit *bu)
{
	if (WaryFighterGlobalMode != WARY_MODE_LIST)
		return WaryFighterGlobalMode;

	return LookupUnitWaryVersion(bu);
}

static int WaryFighterBlocksFollowUp(void)
{
	int actorVer = GetUnitWaryVersion(&gBattleActor);
	int targetVer = GetUnitWaryVersion(&gBattleTarget);

	if (actorVer == WARY_VER_ALWAYS || targetVer == WARY_VER_ALWAYS)
		return TRUE;

	if (actorVer != WARY_VER_ENEMY_INIT && targetVer != WARY_VER_ENEMY_INIT)
		return FALSE;

	return UNIT_FACTION(&gBattleActor.unit) == FACTION_RED;
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
