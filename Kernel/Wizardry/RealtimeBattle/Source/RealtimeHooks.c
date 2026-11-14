#include "common-chax.h"
#include "kernel/realtime-battle.h"
#include "district.h"

void CpPerform_UpdateMapMusic(void);

/*
 * In real-time mode the enemy and green phases only exist for their
 * start-of-phase bookkeeping, so skip their banner and cursor reset.
 * These wrap the procs that gProc_BMapMain starts (see CommonProtection.event).
 */
static void RealtimePhaseFx_SkipIfAiPhase(ProcPtr proc)
{
	if (IsRealtimeBattleActive() && gPlaySt.faction != FACTION_BLUE)
		Proc_End(proc);
}

/* A district is free movement in one long player phase: no banner, and no cursor reset (the camera is on Eirika) */
static void PhaseFx_SkipIfAiPhaseOrDistrict(ProcPtr proc)
{
	if (District_IsDistrictChapter(gPlaySt.chapterIndex))
		Proc_End(proc);
	else
		RealtimePhaseFx_SkipIfAiPhase(proc);
}

const struct ProcCmd ProcScr_RealtimePhaseIntro[] = {
	PROC_CALL(PhaseFx_SkipIfAiPhaseOrDistrict),
	PROC_START_CHILD_BLOCKING(ProcScr_PhaseIntro),
	PROC_END,
};

const struct ProcCmd ProcScr_RealtimeResetCursorPosition[] = {
	PROC_CALL(PhaseFx_SkipIfAiPhaseOrDistrict),
	PROC_START_CHILD_BLOCKING(gProcScr_ResetCursorPosition),
	PROC_END,
};

/* CpPerform would switch to the enemy phase song for every real-time AI action */
void CpPerform_UpdateMapMusic_Realtime(void)
{
	if (RealtimeBattle_IsAiActing())
		return;

	CpPerform_UpdateMapMusic();
}
