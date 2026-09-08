#include "common-chax.h"
#include "kernel/realtime-battle.h"

void RealtimeBattle_BeginActionHook(struct Unit *unit)
{
	RealtimeBattle_OnPlayerActionBegin(unit);
}

bool RealtimeBattle_PostActionHook(ProcPtr proc)
{
	(void)proc;

	/*
	 * This hook runs before PlayerPhase_FinishAction has completed. Releasing
	 * the gate here lets a realtime enemy start during Canto, end events, or
	 * other post-action work. The terminal player-phase hook releases it.
	 */
	return false;
}
