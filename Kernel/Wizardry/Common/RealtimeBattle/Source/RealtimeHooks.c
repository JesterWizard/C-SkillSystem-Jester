#include "common-chax.h"
#include "kernel/realtime-battle.h"

void RealtimeBattle_BeginActionHook(struct Unit *unit)
{
	RealtimeBattle_OnPlayerActionBegin(unit);
}

bool RealtimeBattle_PostActionHook(ProcPtr proc)
{
	(void)proc;
	RealtimeBattle_OnPlayerActionEnd(gActiveUnit);
	return false;
}
