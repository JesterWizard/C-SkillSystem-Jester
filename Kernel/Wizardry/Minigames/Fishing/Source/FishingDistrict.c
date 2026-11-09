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

void Fishing_LoadMapMarker(int chr)
{
	CpuCopy16(gFishingMapBobberTile, OBJ_VRAM0 + chr * 0x20, sizeof(gFishingMapBobberTile));
}

/* A bobber on the water tile next to each spot, bobbing 1 pixel. The palette is the one the gate arrows load. */
void Fishing_DrawMapMarkers(int chr, int pal)
{
	static const s8 dirX[4] = { -1, 1, 0, 0 };
	static const s8 dirY[4] = { 0, 0, -1, 1 };
	struct Trap *trap;
	int x, y, d, bob = (GetGameClock() >> 4) & 1;

	for (trap = GetTrap(0); trap->type != TRAP_NONE; trap++) {
		if (trap->type != TRAP_FISHING_SPOT)
			continue;
		d = trap->extra & 3;
		x = (trap->xPos + dirX[d]) * 16 - gBmSt.camera.x;
		y = (trap->yPos + dirY[d]) * 16 - gBmSt.camera.y;
		if (x <= -16 || x >= 240 || y <= -16 || y >= 160)
			continue;
		PutSprite(4,
			  OAM1_X((x + 4) & 0x1FF),
			  OAM0_Y((y + 6 + bob) & 0xFF),
			  gObject_8x8,
			  OAM2_PAL(pal) + OAM2_LAYER(0b10) + OAM2_CHR(chr));
	}
}

#endif /* CONFIG_DISTRICT_FISHING */
