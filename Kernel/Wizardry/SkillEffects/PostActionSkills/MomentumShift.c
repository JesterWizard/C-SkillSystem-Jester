#include "common-chax.h"
#include "skill-system.h"
#include "debuff.h"
#include "kernel-lib.h"
#include "battle-system.h"
#include "constants/skills.h"

#if defined(SID_MomentumShift) && (COMMON_SKILL_VALID(SID_MomentumShift))
/* Whether the side (the actor, or the target with retaliation set) dodged one of the other side's strikes */
static bool DodgedAStrike(bool actor)
{
	int i;

	for (i = 0; i < NEW_BATTLE_HIT_MAX; i++) {
		struct BattleHit *hit = (struct BattleHit *)((u8 *)prBattleHitArray + BattleHitArrayWidth * i);
		bool by_target = (hit->info & BATTLE_HIT_INFO_RETALIATION) != 0;

		if (hit->info & BATTLE_HIT_INFO_END)
			break;

		/* the actor dodges the target's strikes, and the other way round */
		if (by_target == actor && (hit->attributes & BATTLE_HIT_ATTR_MISS))
			return true;
	}
	return false;
}

static void TryMomentumShift(struct BattleUnit *bu, bool actor)
{
	struct Unit *unit;

	if ((u8)bu->unit.index == 0)
		return;

	unit = GetUnit((u8)bu->unit.index);
	if (!UNIT_IS_VALID(unit) || unit->curHP == 0 || (unit->state & US_DEAD))
		return;

	if (SkillListTester(unit, SID_MomentumShift) && DodgedAStrike(actor))
		SetUnitStatDebuff(unit, UNIT_STAT_BUFF_MOMENTUM_SHIFT);
}
#endif

/*
 * Momentum Shift: a unit that dodged a strike in the fight gets Mov +2 (UNIT_STAT_BUFF_MOMENTUM_SHIFT). The buff is
 * cleared when the other side's phase starts, so it lasts through the unit's next phase, either side of the fight.
 */
bool PostAction_MomentumShift(ProcPtr parent)
{
#if defined(SID_MomentumShift) && (COMMON_SKILL_VALID(SID_MomentumShift))
	switch (gActionData.unitActionType) {
	case UNIT_ACTION_COMBAT:
	case CONFIG_UNIT_ACTION_EXPA_GaidenMagicCombat:
		break;

	default:
		return false;
	}

	TryMomentumShift(&gBattleActor, true);
	TryMomentumShift(&gBattleTarget, false);
#endif
	return false;
}
