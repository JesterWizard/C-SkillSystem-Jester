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

void Crossing_LoadMapMarker(int chr)
{
	CpuCopy16(gCrossingMapFlagTile, OBJ_VRAM0 + chr * 0x20, sizeof(gCrossingMapFlagTile));
}

/* A flag on each spot, waving 1 pixel. The palette is the one the gate arrows load. */
void Crossing_DrawMapMarkers(int chr, int pal)
{
	struct Trap *trap;
	int x, y, wave = (GetGameClock() >> 4) & 1;

	for (trap = GetTrap(0); trap->type != TRAP_NONE; trap++) {
		if (trap->type != TRAP_CROSSING_SPOT)
			continue;
		x = trap->xPos * 16 - gBmSt.camera.x;
		y = trap->yPos * 16 - gBmSt.camera.y;
		if (x <= -16 || x >= 240 || y <= -16 || y >= 160)
			continue;
		PutSprite(4,
			  OAM1_X((x + 9 + wave) & 0x1FF),
			  OAM0_Y((y + 1) & 0xFF),
			  gObject_8x8,
			  OAM2_PAL(pal) + OAM2_LAYER(0b10) + OAM2_CHR(chr));
	}
}

#endif /* CONFIG_DISTRICT_CROSSING */
