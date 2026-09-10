#include "global.h"
#include "bmbattle.h"
#include "bmitem.h"
#include "bmunit.h"
#include "constants/items.h"

/**
 * WaryFighterGlobalMode: 0 = unit/class lists, 1 = everyone always-on, 2 = everyone enemy-init.
 * List entries are (id, version). Version 1 = always-on, 2 = enemy-init.
 * Specific pid beats specific jid. Both beat $FF. Unit $FF beats class $FF.
 * Always-on beats enemy-init if both combatants differ.
 */
#define WARY_MODE_LIST        0
#define WARY_VER_ALWAYS       1
#define WARY_VER_ENEMY_INIT   2

extern const u8 WaryFighterGlobalMode;
extern const u8 WaryFighterUnits[];
extern const u8 WaryFighterClasses[];

static int LookupTableEntry(const u8 *table, u8 id, int *wildcard)
{
	const u8 *it;

	for (it = table; it[0] != 0; it += 2) {
		if (it[0] == id)
			return it[1];

		if (it[0] == 0xFF)
			*wildcard = it[1];
	}

	return 0;
}

static int LookupListWaryVersion(const struct BattleUnit *bu)
{
	int unitWild = 0;
	int classWild = 0;
	int ver;
	u8 pid;
	u8 jid;

	if (!bu->unit.pCharacterData)
		return 0;

	pid = UNIT_CHAR_ID(&bu->unit);
	ver = LookupTableEntry(WaryFighterUnits, pid, &unitWild);
	if (ver)
		return ver;

	if (bu->unit.pClassData) {
		jid = UNIT_CLASS_ID(&bu->unit);
		ver = LookupTableEntry(WaryFighterClasses, jid, &classWild);
		if (ver)
			return ver;
	}

	if (unitWild)
		return unitWild;

	return classWild;
}

static int GetUnitWaryVersion(const struct BattleUnit *bu)
{
	if (WaryFighterGlobalMode != WARY_MODE_LIST)
		return WaryFighterGlobalMode;

	return LookupListWaryVersion(bu);
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
