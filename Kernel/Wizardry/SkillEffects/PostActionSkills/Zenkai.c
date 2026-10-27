#include "common-chax.h"
#include "kernel-lib.h"
#include "skill-system.h"
#include "constants/skills.h"
#include "jester_headers/custom-functions.h"

/**
 * Zenkai: ExecStandardHeal queues the EXP in gZenkaiPending while the staff
 * action owns gBattleActor. Grant it here, after the action, with the normal
 * EXP bar and level-up window for the healed unit.
 */

#if defined(SID_Zenkai) && (COMMON_SKILL_VALID(SID_Zenkai))
extern void BattleApplyMiscActionExpGains_Modular(int exp);

/* TransferStatsandExperience, for whichever unit gBattleActor holds */
static void Zenkai_TransferExp(void)
{
	struct Unit *unit = GetUnit(gBattleActor.unit.index);

	UpdateUnitFromBattle(unit, &gBattleActor);
	unit->state &= ~US_HIDDEN;
}

/* ProcScr_AddExp (MiscFunctions.c), ending with the transfer above */
static const struct ProcCmd ProcScr_ZenkaiExp[] = {
	PROC_CALL(LockGame),
	PROC_WHILE(BattleEventEngineExists),
	PROC_CALL(EndMapAnimInfoWindow),
	PROC_SLEEP(1),
	PROC_CALL(MapAnim_DisplayExpBar),
	PROC_YIELD,
	PROC_CALL(UnlockGame),
	PROC_CALL(MapAnim_Cleanup),
	PROC_CALL(Zenkai_TransferExp),
	PROC_END
};
#endif

bool PostAction_Zenkai(ProcPtr parent)
{
#if defined(SID_Zenkai) && (COMMON_SKILL_VALID(SID_Zenkai))
	struct Unit *unit;
	int exp;

	if (gZenkaiPending[0] == 0)
		return false;

	unit = GetUnit(gZenkaiPending[0]);
	exp = gZenkaiPending[1];
	gZenkaiPending[0] = 0;

	if (!UNIT_IS_VALID(unit) || (unit->state & US_DEAD))
		return false;

	gManimSt.actorCount = 1;
	gManimSt.hp_changing = 0;
	gManimSt.subjectActorId = 0;
	gManimSt.targetActorId = 0;
	InitBattleUnit(&gBattleActor, unit);
	BattleApplyMiscActionExpGains_Modular(exp);
	SetupMapBattleAnim(&gBattleActor, &gBattleTarget, gBattleHitArray);
	Proc_StartBlocking(ProcScr_ZenkaiExp, PROC_TREE_3);
#endif
	return false;
}

void ChapterInit_ResetZenkaiPending(void)
{
	gZenkaiPending[0] = 0;
}
