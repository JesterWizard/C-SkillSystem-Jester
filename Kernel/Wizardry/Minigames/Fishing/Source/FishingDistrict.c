#include "common-chax.h"
#include "kernel-lib.h"
#include "district.h"
#include "EAstdlib.h"
#include "fishing.h"
#include "fishing-art.h"

#ifdef CONFIG_DISTRICT_FISHING

/*
 * District fishing spots. A spot is a TRAP_FISHING_SPOT trap (FISHING_SPOT(x, y, dir) in a district's traps.c) on a
 * walkable tile next to water. Pressing A on it (the free-movement A-press trap list, FreeMovement/cFreeMU/HookList.event)
 * runs EventScr_DistrictFishing, whose ASMC fades the map out, runs the fishing screen and brings the map back, as the
 * district Hex matches do (HexDistrict.c).
 */

static const struct ProcCmd ProcScr_FishingDistrict[] = {
	PROC_NAME("FishingDistrict"),
	PROC_CALL(DistrictNpc_EndWalk),
	PROC_CALL_ARG(NewFadeOut, 0x10),
	PROC_WHILE(FadeOutExists),
	PROC_CALL(BMapDispSuspend),
	PROC_YIELD,
	PROC_START_CHILD_BLOCKING(ProcScr_FishingScreen),
	PROC_YIELD,
	PROC_CALL(BMapDispResume),
	PROC_CALL(RefreshBMapGraphics),
	PROC_CALL_ARG(NewFadeIn, 0x10),
	PROC_WHILE(FadeInExists),
	PROC_END,
};

void StartDistrictFishingASMC(ProcPtr parent)
{
	Proc_StartBlocking(ProcScr_FishingDistrict, parent);
}

static const EventScr EventScr_DistrictFishing[] = {
	ASMC(StartDistrictFishingASMC)
	NOFADE
	ENDA
};

void Fishing_OnPressA(ProcPtr proc)
{
	CallEvent((const u16 *)EventScr_DistrictFishing, 1);
}

/* ---- The bobbers on the district map ---- */

/* A fish leaping from the water tile next to each spot (the district's spot icons, DistrictIcons.c) */
void Fishing_DrawMapMarkers(void)
{
	static const s8 dirX[4] = { -1, 1, 0, 0 };
	static const s8 dirY[4] = { 0, 0, -1, 1 };
	struct Trap *trap;
	int d;

	for (trap = GetTrap(0); trap->type != TRAP_NONE; trap++) {
		if (trap->type != TRAP_FISHING_SPOT)
			continue;
		d = trap->extra & 3;
		District_PutSpotIcon(trap->xPos + dirX[d], trap->yPos + dirY[d], DISTRICT_ICON_FISHING);
	}
}

#endif /* CONFIG_DISTRICT_FISHING */
