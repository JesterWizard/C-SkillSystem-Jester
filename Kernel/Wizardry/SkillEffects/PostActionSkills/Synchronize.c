#include "common-chax.h"
#include "debuff.h"
#include "kernel-lib.h"
#include "battle-system.h"
#include "skill-system.h"
#include "map-anims.h"
#include "constants/skills.h"

STATIC_DECLAR void PostAction_Synchronize_Init(ProcPtr proc)
{
	struct Unit *unit;
	struct MuProc *mu;

	unit = GetUnit(gActionData.subjectIndex);

	HideUnitSprite(unit);
	mu = StartMu(unit);

	FreezeSpriteAnim(mu->sprite_anim);
	SetMuDefaultFacing(mu);
	SetDefaultColorEffects();
	EnsureCameraOntoPosition(proc, unit->xPos, unit->yPos);
}

STATIC_DECLAR void PostAction_Synchronize_StartActor(ProcPtr proc)
{
	struct Unit *unit = GetUnit(gActionData.subjectIndex);

	StartMuActionAnim(GetUnitMu(unit));
}

STATIC_DECLAR void PostAction_Synchronize_StartTargetAnim(ProcPtr proc)
{
	struct Unit *unit_tar = GetUnit(gActionData.targetIndex);

	if (IsDebuff(GetUnitStatusIndex(GetUnit(gActionData.subjectIndex))))
		CallMapAnim_HeavyGravity(proc, unit_tar->xPos, unit_tar->yPos);
	else {
		Proc_StartBlocking(ProcScr_DanceringAnim, proc);

		BG_SetPosition(
			BG_0,
			-SCREEN_TILE_IX(unit_tar->xPos),
			-SCREEN_TILE_IX(unit_tar->yPos));
	}
}

STATIC_DECLAR void PostAction_Synchronize_ResetActor(ProcPtr proc)
{
	struct Unit *unit = GetUnit(gActionData.subjectIndex);
	struct MuProc *mu = GetUnitMu(unit);

	FreezeSpriteAnim(mu->sprite_anim);
	EnsureCameraOntoPosition(proc, unit->xPos, unit->yPos);
}

STATIC_DECLAR void PostAction_Synchronize_End(ProcPtr proc)
{
	SetUnitStatus(
		GetUnit(gActionData.targetIndex),
		GetUnitStatusIndex(GetUnit(gActionData.subjectIndex)));

	MapAnim_CommonEnd();
}

STATIC_DECLAR const struct ProcCmd ProcScr_PostAction_Synchronize[] = {
	PROC_CALL(LockGame),
	PROC_CALL(PostAction_Synchronize_Init),
	PROC_SLEEP(2),
	PROC_CALL(PostAction_Synchronize_StartActor),
	PROC_SLEEP(30),
	PROC_CALL(PostAction_Synchronize_StartTargetAnim),
	PROC_SLEEP(0xA),
	PROC_CALL(PostAction_Synchronize_ResetActor),
	PROC_SLEEP(2),
	PROC_CALL(UnlockGame),
	PROC_CALL(PostAction_Synchronize_End),
	PROC_END
};

bool PostAction_Synchronize(ProcPtr parent)
{
	struct Unit *unit_act, *unit_tar;
	int debuff_act;

	if (!UNIT_IS_VALID(gActiveUnit))
		return false;

#if (defined(SID_Synchronize) && (COMMON_SKILL_VALID(SID_Synchronize)))
	if (!SkillListTester(gActiveUnit, SID_Synchronize))
#else
	if (1)
#endif
		return false;

	switch (gActionData.unitActionType) {
	case UNIT_ACTION_COMBAT:
	case CONFIG_UNIT_ACTION_EXPA_GaidenMagicCombat:
		/**
		 * If battle is not hitted, return
		 */
		if (gBattleActorGlobalFlag.hitted == false)
			return false;

		break;

	case UNIT_ACTION_STAFF:
	case CONFIG_UNIT_ACTION_EXPA_GaidenMagicStaff:
		if (GetSpellAssocCharCount(gBattleActor.weaponBefore) != 2)
			return false;

		break;

	case UNIT_ACTION_DANCE:
		break;

	default:
		return false;
	}

	unit_act = GetUnit(gActionData.subjectIndex);
	if (!UnitAvaliable(unit_act) || UNIT_STONED(unit_act))
		return false;

	unit_tar = GetUnit(gActionData.targetIndex);
	if (!UnitAvaliable(unit_tar) || UNIT_STONED(unit_tar))
		return false;

	/* actor should already hold a special status */
	debuff_act = GetUnitStatusIndex(unit_act);
	if (debuff_act == UNIT_STATUS_NONE)
		return false;

	/* Give ally positive status, give enemy debuff */
	if (AreUnitsAllied(unit_act->index, unit_tar->index) == IsDebuff(debuff_act))
		return false;

	EndAllMus();
	RenderBmMap();
	ShowUnitSprite(GetUnit(gActionData.targetIndex));
	Proc_Start(ProcScr_PostAction_Synchronize, PROC_TREE_3);
	return true;
}
