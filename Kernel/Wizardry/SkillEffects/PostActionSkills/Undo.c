#include "common-chax.h"
#include "kernel-lib.h"
#include "skill-system.h"
#include "map-anims.h"
#include "unit-expa.h"
#include "bwl.h"
#include "constants/skills.h"

/**
 * Undo: once per map, a unit that survives a battle gets back the HP and MP
 * it lost in it. HP before the battle is the battle unit's hpInitial; MP
 * before the battle is recorded when the real battle starts, so a spell's
 * own MP cost is refunded too.
 */

struct UndoSnapshot {
	u8 valid;
	u8 actor_mp;
	u8 target_mp;
	u8 actor_hidden;
};

extern struct UndoSnapshot gUndoSnapshot;

#if defined(SID_Undo) && (COMMON_SKILL_VALID(SID_Undo))
static struct NewBwl *GetUndoBwl(struct Unit *unit)
{
	if (!gpKernelDesignerConfig->mp_system || !UNIT_IS_VALID(unit))
		return NULL;

	return GetNewBwl(UNIT_CHAR_ID(unit));
}

static int GetUndoMP(struct Unit *unit)
{
	struct NewBwl *bwl = GetUndoBwl(unit);

	return bwl ? bwl->currentMP : 0;
}

/* The acting unit's MU is gone during the heal anim, so draw its map sprite */
static void Undo_ShowActor(ProcPtr proc)
{
	gUndoSnapshot.actor_hidden = gActiveUnit->state & US_HIDDEN;
	gActiveUnit->state &= ~US_HIDDEN;
	RefreshEntityBmMaps();
	RefreshUnitSprites();
}

/* Restore the hidden bit so the vanilla action end sees the same state */
static void Undo_RestoreActor(ProcPtr proc)
{
	gActiveUnit->state |= gUndoSnapshot.actor_hidden;
}

static bool TryUndo(ProcPtr parent, struct BattleUnit *bu, int mp_before)
{
	struct Unit *unit;
	struct NewBwl *bwl;
	int hp_lost;
	bool mp_lost;

	if (bu->unit.index == 0)
		return false;

	unit = GetUnit(bu->unit.index);

	/* Non-lethal only: a unit killed in the battle stays dead */
	if (!UNIT_IS_VALID(unit) || unit->curHP <= 0 || (unit->state & US_DEAD))
		return false;

	if (!SkillTester(unit, SID_Undo) || CheckBitUES(unit, UES_BIT_UNDO_SKILL_USED))
		return false;

	bwl = GetUndoBwl(unit);
	hp_lost = bu->hpInitial - unit->curHP;
	mp_lost = bwl && bwl->currentMP < mp_before;

	if (hp_lost <= 0 && !mp_lost)
		return false;

	SetBitUES(unit, UES_BIT_UNDO_SKILL_USED);

	if (mp_lost)
		bwl->currentMP = mp_before;

	if (hp_lost <= 0)
		return false;

	if (unit == gActiveUnit)
		CallMapAnim_Heal(parent, unit, hp_lost);
	else
		CallMapAnim_HealExt(parent, unit, hp_lost, Undo_ShowActor, Undo_RestoreActor);
	return true;
}

static bool IsUndoBattleAction(void)
{
	if (!gUndoSnapshot.valid)
		return false;

	switch (gActionData.unitActionType) {
	case UNIT_ACTION_COMBAT:
	case CONFIG_UNIT_ACTION_EXPA_GaidenMagicCombat:
		return true;

	default:
		return false;
	}
}
#endif

void Undo_SnapshotBattle(struct Unit *actor, struct Unit *target)
{
#if defined(SID_Undo) && (COMMON_SKILL_VALID(SID_Undo))
	gUndoSnapshot.valid = true;
	gUndoSnapshot.actor_mp = GetUndoMP(actor);
	gUndoSnapshot.target_mp = GetUndoMP(target);
#endif
}

bool PostAction_UndoActor(ProcPtr parent)
{
#if defined(SID_Undo) && (COMMON_SKILL_VALID(SID_Undo))
	if (!IsUndoBattleAction())
		return false;

	return TryUndo(parent, &gBattleActor, gUndoSnapshot.actor_mp);
#else
	return false;
#endif
}

bool PostAction_UndoTarget(ProcPtr parent)
{
#if defined(SID_Undo) && (COMMON_SKILL_VALID(SID_Undo))
	bool valid = IsUndoBattleAction();

	/* The snapshot only covers the battle of this action */
	gUndoSnapshot.valid = false;

	if (!valid)
		return false;

	return TryUndo(parent, &gBattleTarget, gUndoSnapshot.target_mp);
#else
	return false;
#endif
}
