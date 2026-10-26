#include "global.h"
#include "bmunit.h"
#include "playerphase.h"

/**
 * Wrapper around vanilla PlayerPhase_PrepareAction, called from
 * gProcScr_PlayerPhase in its place. Clearing gBmSt.taken_action first means
 * that cancelling the action menu after Trade / Supply / Take / Give /
 * Ballista sends the unit back to movement selection instead of forcing Wait.
 */
s8 PlayerPhase_PrepareAction_NoWait(ProcPtr proc)
{
	gBmSt.taken_action = 0;

	return PlayerPhase_PrepareAction(proc);
}
