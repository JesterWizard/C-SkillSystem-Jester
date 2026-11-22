#include "common-chax.h"
#include "kernel-lib.h"
#include "district.h"
#include "EAstdlib.h"
#include "tower.h"
#include "tower-art.h"

#ifdef CONFIG_DISTRICT_TOWER

/*
 * District Tower Stack spots. A spot is a TRAP_TOWER_SPOT trap (TOWER_SPOT(x, y) in a district's traps.c). Pressing A
 * on it (the free-movement A-press trap list, FreeMovement/cFreeMU/HookList.event) runs EventScr_DistrictTower, whose
 * ASMC fades the map out, runs the game and brings the map back, as the fishing spots do (FishingDistrict.c).
 */

static const struct ProcCmd ProcScr_TowerDistrict[] = {
	PROC_NAME("TowerDistrict"),
	PROC_CALL(DistrictNpc_EndWalk),
	PROC_CALL_ARG(NewFadeOut, 0x10),
	PROC_WHILE(FadeOutExists),
	PROC_CALL(BMapDispSuspend),
	PROC_YIELD,
	PROC_START_CHILD_BLOCKING(ProcScr_TowerScreen),
	PROC_YIELD,
	PROC_CALL(BMapDispResume),
	PROC_CALL(RefreshBMapGraphics),
	PROC_CALL_ARG(NewFadeIn, 0x10),
	PROC_WHILE(FadeInExists),
	PROC_END,
};

void StartDistrictTowerASMC(ProcPtr parent)
{
	Proc_StartBlocking(ProcScr_TowerDistrict, parent);
}

static const EventScr EventScr_DistrictTower[] = {
	ASMC(StartDistrictTowerASMC)
	NOFADE
	ENDA
};

void Tower_OnPressA(ProcPtr proc)
{
	CallEvent((const u16 *)EventScr_DistrictTower, 1);
}

/* ---- The towers on the district map ---- */

/* A tower of blocks over each spot (the district's spot icons, DistrictIcons.c) */
void Tower_DrawMapMarkers(void)
{
	struct Trap *trap;

	for (trap = GetTrap(0); trap->type != TRAP_NONE; trap++)
		if (trap->type == TRAP_TOWER_SPOT)
			District_PutSpotIcon(trap->xPos, trap->yPos, DISTRICT_ICON_TOWER);
}

#endif /* CONFIG_DISTRICT_TOWER */
