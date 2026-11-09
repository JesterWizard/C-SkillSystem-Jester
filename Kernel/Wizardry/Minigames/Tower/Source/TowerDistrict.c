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

void Tower_LoadMapMarker(int chr)
{
	CpuCopy16(gTowerMapTile, OBJ_VRAM0 + chr * 0x20, sizeof(gTowerMapTile));
}

/* A small tower on each spot. The palette is the one the gate arrows load. */
void Tower_DrawMapMarkers(int chr, int pal)
{
	struct Trap *trap;
	int x, y;

	for (trap = GetTrap(0); trap->type != TRAP_NONE; trap++) {
		if (trap->type != TRAP_TOWER_SPOT)
			continue;
		x = trap->xPos * 16 - gBmSt.camera.x;
		y = trap->yPos * 16 - gBmSt.camera.y;
		if (x <= -16 || x >= 240 || y <= -16 || y >= 160)
			continue;
		PutSprite(4,
			  OAM1_X((x + 4) & 0x1FF),
			  OAM0_Y((y + 2) & 0xFF),
			  gObject_8x8,
			  OAM2_PAL(pal) + OAM2_LAYER(0b10) + OAM2_CHR(chr));
	}
}

#endif /* CONFIG_DISTRICT_TOWER */
