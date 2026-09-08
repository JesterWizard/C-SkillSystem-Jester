#include <common-chax.h>
#include "kernel/realtime-battle.h"

/* Needed for VeslyDangerBones */
// LYN_REPLACE_CHECK(PlayerPhase_Suspend);
// void PlayerPhase_Suspend(void) {}

void Hook_BmMain_SuspendBeforePhase(void)
{
	RealtimeBattle_QuiesceForSuspend();
	gActionData.suspendPointType = SUSPEND_POINT_PHASECHANGE;
	WriteSuspendSave(SAVE_ID_SUSPEND);
}
