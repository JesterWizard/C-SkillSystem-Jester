#include "common-chax.h"
#include "kernel-lib.h"
#include "district.h"
#include "EAstdlib.h"
#include "crossing.h"
#include "crossing-art.h"

#ifdef CONFIG_DISTRICT_CROSSING

/*
 * District crossing spots. A spot is a TRAP_CROSSING_SPOT trap (CROSSING_SPOT(x, y) in a district's traps.c). Pressing A
 * on it (the free-movement A-press trap list, FreeMovement/cFreeMU/HookList.event) runs EventScr_DistrictCrossing,
 * whose ASMC fades the map out, runs the game and brings the map back, as the fishing spots do (FishingDistrict.c).
 */

static const struct ProcCmd ProcScr_CrossingDistrict[] = {
	PROC_NAME("CrossingDistrict"),
	PROC_CALL(DistrictNpc_EndWalk),
	PROC_CALL_ARG(NewFadeOut, 0x10),
	PROC_WHILE(FadeOutExists),
	PROC_CALL(BMapDispSuspend),
	PROC_YIELD,
	PROC_START_CHILD_BLOCKING(ProcScr_CrossingScreen),
	PROC_YIELD,
	PROC_CALL(BMapDispResume),
	PROC_CALL(RefreshBMapGraphics),
	PROC_CALL_ARG(NewFadeIn, 0x10),
	PROC_WHILE(FadeInExists),
	PROC_END,
};

void StartDistrictCrossingASMC(ProcPtr parent)
{
	Proc_StartBlocking(ProcScr_CrossingDistrict, parent);
}

static const EventScr EventScr_DistrictCrossing[] = {
	ASMC(StartDistrictCrossingASMC)
	NOFADE
	ENDA
};

void Crossing_OnPressA(ProcPtr proc)
{
	CallEvent((const u16 *)EventScr_DistrictCrossing, 1);
}

/* ---- The flags on the district map ---- */

/* A frog over each spot (the district's spot icons, DistrictIcons.c) */
void Crossing_DrawMapMarkers(void)
{
	struct Trap *trap;

	for (trap = GetTrap(0); trap->type != TRAP_NONE; trap++)
		if (trap->type == TRAP_CROSSING_SPOT)
			District_PutSpotIcon(trap->xPos, trap->yPos, DISTRICT_ICON_CROSSING);
}

#endif /* CONFIG_DISTRICT_CROSSING */
