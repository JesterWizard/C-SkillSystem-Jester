#include "common-chax.h"
#include "battle-system.h"
#include "skill-system.h"
#include "constants/skills.h"
#include "kernel/realtime-battle.h"

LYN_REPLACE_CHECK(MoveActiveUnit);
void MoveActiveUnit(int x, int y)
{
	gActiveUnit->xPos = x;
	gActiveUnit->yPos = y;

	gActiveUnit->state |= US_UNSELECTABLE;

	PidStatsAddSquaresMoved(gActiveUnit->pCharacterData->number, gActionData.moveCount);

	if (GetUnitCurrentHp(gActiveUnit) != 0)
		gActiveUnit->state = gActiveUnit->state & ~US_HIDDEN;

	UnitFinalizeMovement(gActiveUnit);

}

LYN_REPLACE_CHECK(AddUnitHp);
void AddUnitHp(struct Unit *unit, int amount)
{
	int hp = unit->curHP;

	hp += amount;

	if (hp > GetUnitMaxHp(unit))
		hp = GetUnitMaxHp(unit);

	if (hp < 0)
		hp = 0;

	unit->curHP = hp;
	SyncReplicateLinkedHp(unit);
}

LYN_REPLACE_CHECK(SetUnitHp);
void SetUnitHp(struct Unit *unit, int value)
{
	if (value < 0)
		value = 0;

	unit->curHP = value;

	if (unit->curHP > GetUnitMaxHp(unit))
		unit->curHP = GetUnitMaxHp(unit);

	SyncReplicateLinkedHp(unit);
}

LYN_REPLACE_CHECK(SetupMapBattleAnim);
void SetupMapBattleAnim(struct BattleUnit *actor, struct BattleUnit *target, struct BattleHit *hit)
{
	int i;
	struct BattleHit *hit0 = prBattleHitArray;

	/*
	 * Real-time enemy actions can still hold a movement MU when map combat
	 * starts. Free that slot before allocating battle MUs or StartMu can fail
	 * and the actor MMS never appears. Vanilla already owns MU cleanup in its
	 * normal action path, so keep this extra cleanup RT-only.
	 */
	if (IsRealtimeBattleActive())
		EndAllMus();

	MakeBattleMOVEUNIT(0, actor, &actor->unit);

	if (gManimSt.actorCount > 1) {
		HideUnitSprite(&gBattleTarget.unit); // NOTE: uses gBattleTarget instead of target argument
		MakeBattleMOVEUNIT(1, target, &target->unit);
	}

	if (hit0->attributes & BATTLE_HIT_ATTR_TATTACK) {
		MakeBattleMOVEUNIT(2, actor, gBattleStats.taUnitA);
		MakeBattleMOVEUNIT(3, actor, gBattleStats.taUnitB);

		HideUnitSprite(gBattleStats.taUnitA);
		HideUnitSprite(gBattleStats.taUnitB);
	}

	SetupBattleMOVEUNITs();

	for (i = 0; i < gManimSt.actorCount; ++i) {
		gManimSt.actor[i].hp_cur = gManimSt.actor[i].bu->hpInitial;

#if CHAX
		gManimSt.actor[i].hp_max = gManimSt.actor[i].bu->unit.maxHP;
#else
		gManimSt.actor[i].hp_max = GetUnitMaxHp(gManimSt.actor[i].unit);
#endif
	}

	SetDefaultColorEffects();
}