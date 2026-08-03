#include "common-chax.h"
#include "kernel-lib.h"
#include "kernel/realtime-battle.h"
#include "combo-attack.h"

/**
 * Pre-hit
 */
extern HookProcFunc_t const gPreMapAnimBattleRoundHooks[];
extern HookProcFunc_t const *const gpPreMapAnimBattleRoundHooks;

STATIC_DECLAR void PreMapAnimBattleRound_OnStart(ProcPtr proc)
{
	MapAnim_PrepareNextBattleRound_CleanPreRoundCombo();

	/**
	 * This is part of function MapAnim_PrepareNextBattleRound()
	 * we need to put it external
	 */
	if (gManimSt.pCurrentRound->info & BATTLE_HIT_INFO_END) {
		Proc_Break(proc);
		Proc_GotoScript(proc, ProcScr_MapAnimEnd);
		return;
	}
	MapAnim_PrepareNextBattleRound(proc);

	gManimSt.pCurrentRound--;
	KernelStartBlockingHookProc(gpPreMapAnimBattleRoundHooks, proc);
}

STATIC_DECLAR void PreMapAnimBattleRound_OnEnd(ProcPtr proc)
{
	struct Unit *unit = gManimSt.actor[gManimSt.subjectActorId].unit;

	if (UNIT_IS_VALID(unit)) {
		if (IsRealtimeBattleActive())
			RealtimeBattle_SnapCameraOntoPosition(unit->xPos, unit->yPos);
		else
			EnsureCameraOntoPosition(proc, unit->xPos, unit->yPos);
	}

	gManimSt.pCurrentRound++;
}

/*
 * PlayerPhase_MainIdle keeps running in real-time mode, so a blocking CamMove
 * started from MapAnim can never finish (the idle loop pulls the camera toward
 * the free cursor). Snap instead whenever RT is active.
 */
LYN_REPLACE_CHECK(MapAnim_MoveCameraOntoSubject);
void MapAnim_MoveCameraOntoSubject(ProcPtr proc)
{
	struct Unit *unit = gManimSt.actor[0].unit;

	if (!UNIT_IS_VALID(unit))
		return;

	if (IsRealtimeBattleActive()) {
		RealtimeBattle_SnapCameraOntoPosition(unit->xPos, unit->yPos);
		return;
	}

	EnsureCameraOntoPosition(proc, unit->xPos, unit->yPos);
}

LYN_REPLACE_CHECK(MapAnim_MoveCameraOntoTarget);
void MapAnim_MoveCameraOntoTarget(ProcPtr proc)
{
	struct Unit *unit;

	if (gManimSt.actorCount == 1)
		return;

	unit = gManimSt.actor[1].unit;
	if (!UNIT_IS_VALID(unit))
		return;

	if (IsRealtimeBattleActive()) {
		RealtimeBattle_SnapCameraOntoPosition(unit->xPos, unit->yPos);
		return;
	}

	EnsureCameraOntoPosition(proc, unit->xPos, unit->yPos);
}

const struct ProcCmd ProcScr_MapAnimBattle_Rework[] = {
	PROC_CALL(LockGame),
	PROC_CALL(MapAnim_PrepareBattleTalk),
	PROC_SLEEP(0x1),
	PROC_CALL(MapAnim_MoveCameraOntoSubject),
	PROC_SLEEP(0x2),
	PROC_CALL(MapAnim_CallBattleQuoteEvents),
	PROC_WHILE(BattleEventEngineExists),
	PROC_SLEEP(0x5),
	PROC_CALL(SetBattleMuPalette),
	PROC_CALL(SetupBattleMOVEUNITs),
	PROC_SLEEP(0x1),
	PROC_CALL(MapAnim_InitInfoBox),
	PROC_SLEEP(0xF),
PROC_LABEL(0x0),

#if CHAX
	/* Pre-hit */
	PROC_CALL(PreMapAnimBattleRound_OnStart),
	PROC_YIELD,
	PROC_CALL(PreMapAnimBattleRound_OnEnd),
	PROC_YIELD,
#else
	PROC_REPEAT(MapAnim_PrepareNextBattleRound),
#endif

	PROC_CALL(MapAnim_DisplayRoundAnim),
	PROC_SLEEP(0x1),
	PROC_CALL(MapAnim_ShowPoisonEffectIfAny),
	PROC_SLEEP(0x1),
	PROC_SLEEP(0x5),
	PROC_GOTO(0x0),
};
