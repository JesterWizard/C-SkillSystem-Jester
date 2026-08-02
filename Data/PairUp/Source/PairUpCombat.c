#include "common-chax.h"
#include "pair-up.h"
#include "kernel-lib.h"
#include "bmmap.h"
#include "battle-system.h"
#include "combo-attack.h"
#include "debuff.h"
#include "skill-system.h"
#include "status-getter.h"
#include "constants/skills.h"
#include "weapon-range.h"

#pragma GCC optimize("Os", "no-jump-tables")

static bool PairUp_CombatIsInMap(int x, int y)
{
	return gBmMapSize.x > 0 && gBmMapSize.y > 0
		&& x >= 0 && y >= 0
		&& x < gBmMapSize.x && y < gBmMapSize.y;
}

static bool PairUp_CombatIsValidUnit(const struct Unit *unit)
{
	return UNIT_IS_VALID(unit) && (u8) unit->index != 0;
}

static bool PairUp_IsCombatSupportCandidate(
	const struct Unit *leader,
	const struct Unit *support)
{
	if (!PairUp_CombatIsValidUnit(leader)
		|| !PairUp_CombatIsValidUnit(support)
		|| leader == support)
		return false;

	if (!UnitOnMapAvaliable((struct Unit *) support)
		|| !IsSameAllegiance(leader->index, support->index))
		return false;

	if (PairUp_IsSupport(support)
		|| UNIT_STONED((struct Unit *) support)
		|| UNIT_IS_PHANTOM(support)
		|| support->statusIndex == UNIT_STATUS_BERSERK)
		return false;

	if (support->state
		& (US_DEAD | US_IN_BALLISTA | US_RESCUING | US_RESCUED))
		return false;

	return true;
}

static bool PairUp_IsStoredSupportUsable(
	const struct Unit *leader,
	const struct Unit *support)
{
	if (!PairUp_CombatIsValidUnit(leader)
		|| !PairUp_CombatIsValidUnit(support)
		|| !IsSameAllegiance(leader->index, support->index))
		return false;

	if (UNIT_STONED((struct Unit *) support)
		|| UNIT_IS_PHANTOM(support)
		|| support->statusIndex == UNIT_STATUS_BERSERK)
		return false;

	return !(support->state
		& (US_DEAD | US_NOT_DEPLOYED | US_IN_BALLISTA
			| US_RESCUING | US_RESCUED));
}

struct Unit *PairUp_GetCombatSupport(struct Unit *leader)
{
	static const s8 sDx[] = { 0, 1, 0, -1 };
	static const s8 sDy[] = { -1, 0, 1, 0 };
	struct Unit *support;
	struct Unit *best = NULL;
	int bestLevel = SUPPORT_LEVEL_NONE - 1;
	int i;

	if (!PairUp_IsEnabled() || !PairUp_CombatIsValidUnit(leader))
		return NULL;

	if (PairUp_IsSupport(leader))
		leader = PairUp_GetLeader(leader);

	if (!leader)
		return NULL;

	if (PairUp_IsLeader(leader)) {
		support = PairUp_GetSupport(leader);

		if (PairUp_IsStoredSupportUsable(leader, support))
			return support;
	}

	for (i = 0; i < (int) ARRAY_COUNT(sDx); ++i) {
		int x = leader->xPos + sDx[i];
		int y = leader->yPos + sDy[i];
		int level;

		if (!PairUp_CombatIsInMap(x, y) || !gBmMapUnit)
			continue;

		support = GetUnit(gBmMapUnit[y][x]);
		if (!PairUp_IsCombatSupportCandidate(leader, support))
			continue;

		level = PairUp_GetSupportLevel(leader, support);
		if (level > bestLevel) {
			best = support;
			bestLevel = level;
		}
	}

	return best;
}

int PairUp_GetDualStrikeChance(struct Unit *leader)
{
	struct Unit *support;
	int chance;
	int level;

	if (!PairUp_IsEnabled() || !PairUp_CombatIsValidUnit(leader))
		return 0;

	if (PairUp_IsSupport(leader))
		leader = PairUp_GetLeader(leader);

	support = PairUp_GetCombatSupport(leader);
	if (!support)
		return 0;

	level = PairUp_GetSupportLevel(leader, support);
	switch (level) {
	case SUPPORT_LEVEL_C:
		chance = 30;
		break;
	case SUPPORT_LEVEL_B:
		chance = 40;
		break;
	case SUPPORT_LEVEL_A:
		chance = 50;
		break;
	default:
		chance = 20;
		break;
	}

	chance += (SklGetter(leader) + SklGetter(support)) / 4;

#if defined(SID_DualStrikePlus) && (COMMON_SKILL_VALID(SID_DualStrikePlus))
	if (SkillTester(leader, SID_DualStrikePlus)
		|| SkillTester(support, SID_DualStrikePlus))
		chance += 10;
#endif

	if (chance > 100)
		chance = 100;

	return chance;
}

int PairUp_GetDualGuardChance(struct Unit *leader, int weapon)
{
	struct Unit *support;
	int level;
	int chance;
	int defense;

	if (!PairUp_IsEnabled() || !PairUp_CombatIsValidUnit(leader))
		return 0;

	if (PairUp_IsSupport(leader))
		leader = PairUp_GetLeader(leader);

	support = PairUp_GetCombatSupport(leader);
	if (!support)
		return 0;

	if (GetItemAttributes(weapon) & (IA_MAGIC | IA_MAGICDAMAGE))
		defense = ResGetter(leader) + ResGetter(support);
	else
		defense = DefGetter(leader) + DefGetter(support);

	level = PairUp_GetSupportLevel(leader, support);
	switch (level) {
	case SUPPORT_LEVEL_C:
		chance = 2;
		break;
	case SUPPORT_LEVEL_B:
		chance = 5;
		break;
	case SUPPORT_LEVEL_A:
		chance = 7;
		break;
	default:
		chance = 0;
		break;
	}

	chance += defense / 4;

#if defined(SID_DualGuardPlus) && (COMMON_SKILL_VALID(SID_DualGuardPlus))
	if (SkillTester(leader, SID_DualGuardPlus)
		|| SkillTester(support, SID_DualGuardPlus))
		chance += 10;
#endif

	if (chance > 100)
		chance = 100;

	return chance;
}

bool BattlePairUpGenerateSupportAttack(
	struct BattleUnit *attacker,
	struct BattleUnit *defender)
{
	struct Unit *leader;
	struct Unit *support;
	struct BattleUnit savedBattle;
	struct BattleGlobalFlags savedActorFlags;
	struct BattleGlobalFlags savedTargetFlags;
	struct BattleGlobalFlags supportFlags;
	int range;
	u16 weapon;
	u8 savedGuardResult;
	bool targetSide;
	bool stop;

	if (!PairUp_IsEnabled() || !attacker || !defender)
		return false;

	leader = GetUnit(attacker->unit.index);
	support = PairUp_GetCombatSupport(leader);
	if (!support || UNIT_STONED(support)
		|| (support->state
			& (US_DEAD | US_IN_BALLISTA | US_RESCUING | US_RESCUED)))
		return false;

	weapon = GetUnitEquippedWeapon(support);
	range = RECT_DISTANCE(
		support->xPos, support->yPos,
		defender->unit.xPos, defender->unit.yPos);

	if (!weapon || !IsItemCoveringRangeRework(weapon, range, support))
		return false;

	if (!BattleRoll1RN(PairUp_GetDualStrikeChance(leader), false))
		return false;

	targetSide = attacker == &gBattleTarget;
	savedActorFlags = gBattleActorGlobalFlag;
	savedTargetFlags = gBattleTargetGlobalFlag;

	if (targetSide) {
		savedBattle = gBattleTarget;
		gBattleTarget = gBattleActor;
		gBattleActorGlobalFlag = savedTargetFlags;
		gBattleTargetGlobalFlag = savedActorFlags;
	} else {
		savedBattle = gBattleActor;
	}

	InitBattleUnit(&gBattleActor, support);
	SetBattleUnitWeapon(&gBattleActor, BU_ISLOT_AUTO);
	SetBattleUnitTerrainBonusesAuto(&gBattleActor);
	BattleApplyWeaponTriangleEffect(&gBattleActor, &gBattleTarget);
	ComputeBattleUnitStats(&gBattleActor, &gBattleTarget);
	ComputeBattleUnitEffectiveStats(&gBattleActor, &gBattleTarget);

	gBattleHitIterator->attributes = targetSide
		? BATTLE_HIT_ATTR_RETALIATE
		: 0;
	gBattleHitIterator->info = targetSide
		? BATTLE_HIT_INFO_RETALIATION
		: 0;
	savedGuardResult = gBattleTemporaryFlag.pair_up_guard_result;
	gBattleTemporaryFlag.pair_up_guard_result = 0;

	stop = BattleGenerateHit(&gBattleActor, &gBattleTarget);
	gBattleTemporaryFlag.pair_up_guard_result = savedGuardResult;

	if (!(gBattleStats.config & BATTLE_CONFIG_SIMULATE))
		UpdateUnitFromBattle(support, &gBattleActor);

	if (targetSide) {
		supportFlags = gBattleActorGlobalFlag;
		gBattleActor = gBattleTarget;
		gBattleTarget = savedBattle;
		gBattleActorGlobalFlag = gBattleTargetGlobalFlag;
		gBattleTargetGlobalFlag = supportFlags;
	} else {
		gBattleActor = savedBattle;
	}

	return stop;
}
