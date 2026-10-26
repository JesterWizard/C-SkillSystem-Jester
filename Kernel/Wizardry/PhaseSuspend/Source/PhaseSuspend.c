#include <common-chax.h>
#include "kernel/realtime-battle.h"

/* Needed for VeslyDangerBones */
// LYN_REPLACE_CHECK(PlayerPhase_Suspend);
// void PlayerPhase_Suspend(void) {}

void Hook_BmMain_SuspendBeforePhase(void)
{
	/* Real-time battle passes through the AI phases instantly, keep the player phase save */
	if (IsRealtimeBattleActive() && gPlaySt.faction != FACTION_BLUE)
		return;

	gActionData.suspendPointType = SUSPEND_POINT_PHASECHANGE;
	WriteSuspendSave(SAVE_ID_SUSPEND);
}
