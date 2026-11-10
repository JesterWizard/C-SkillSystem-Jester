#include "common-chax.h"
#include "kernel-lib.h"
#include "district.h"
#include "EAstdlib.h"
#include "guild.h"

#ifdef CONFIG_DISTRICT_GUILD

/*
 * The guild board on a district map. It is a TRAP_GUILD_BOARD trap (GUILD_BOARD(x, y) in a district's traps.c). Pressing
 * A on it (the free-movement A-press trap list, FreeMovement/cFreeMU/HookList.event) runs EventScr_DistrictGuild, whose
 * ASMC fades the map out, runs the board's screen and brings the map back, as the minigame spots do.
 */

static const struct ProcCmd ProcScr_GuildDistrict[] = {
	PROC_NAME("GuildDistrict"),
	PROC_CALL(DistrictNpc_EndWalk),
	PROC_CALL_ARG(NewFadeOut, 0x10),
	PROC_WHILE(FadeOutExists),
	PROC_CALL(BMapDispSuspend),
	PROC_YIELD,
	PROC_START_CHILD_BLOCKING(ProcScr_GuildScreen),
	PROC_YIELD,
	PROC_CALL(BMapDispResume),
	PROC_CALL(RefreshBMapGraphics),
	PROC_CALL_ARG(NewFadeIn, 0x10),
	PROC_WHILE(FadeInExists),
	PROC_END,
};

void StartDistrictGuildASMC(ProcPtr parent)
{
	Proc_StartBlocking(ProcScr_GuildDistrict, parent);
}

static const EventScr EventScr_DistrictGuild[] = {
	ASMC(StartDistrictGuildASMC)
	NOFADE
	ENDA
};

void Guild_OnPressA(ProcPtr proc)
{
	CallEvent((const u16 *)EventScr_DistrictGuild, 1);
}

/* ---- The board on the district map ---- */

/*
 * Two 8x8 tiles in item icon palette 0 (as the other district markers): a wooden notice board with white papers, and the
 * same with a red notice, shown in turn while a quest on this board is met and waiting to be claimed.
 */
static const u8 sGuildMarkerTiles[2][32] = {
	{
		0xE0, 0xEE, 0xEE, 0x0E, 0xDE, 0x11, 0xDD, 0xED, 0xDE, 0x11, 0x1D, 0xE1, 0xDE, 0xDD, 0x1D, 0xE1,
		0xDE, 0x11, 0xDD, 0xED, 0xE0, 0xEE, 0xEE, 0x0E, 0x00, 0x0C, 0xC0, 0x00, 0x00, 0x0C, 0xC0, 0x00,
	},
	{
		0xE0, 0xEE, 0xEE, 0x0E, 0xDE, 0x11, 0x6D, 0xE6, 0xDE, 0x11, 0x6D, 0xE6, 0xDE, 0xDD, 0x6D, 0xE6,
		0xDE, 0x11, 0xDD, 0xED, 0xE0, 0xEE, 0xEE, 0x0E, 0x00, 0x0C, 0xC0, 0x00, 0x00, 0x0C, 0xC0, 0x00,
	},
};

/* Both tiles, at chr and chr + 1 */
void Guild_LoadMapMarker(int chr)
{
	CpuCopy16(sGuildMarkerTiles, OBJ_VRAM0 + chr * 0x20, sizeof(sGuildMarkerTiles));
}

void Guild_DrawMapMarkers(int chr, int pal)
{
	struct Trap *trap;
	int x, y, alt;

	/* The red notice blinks while a quest here can be claimed */
	alt = ((GetGameClock() >> 5) & 1) && Guild_AnyClaimable(gPlaySt.chapterIndex);

	for (trap = GetTrap(0); trap->type != TRAP_NONE; trap++) {
		if (trap->type != TRAP_GUILD_BOARD)
			continue;
		x = trap->xPos * 16 - gBmSt.camera.x;
		y = trap->yPos * 16 - gBmSt.camera.y;
		if (x <= -16 || x >= 240 || y <= -16 || y >= 160)
			continue;
		PutSprite(4,
			  OAM1_X((x + 4) & 0x1FF),
			  OAM0_Y((y + 4) & 0xFF),
			  gObject_8x8,
			  OAM2_PAL(pal) + OAM2_LAYER(0b10) + OAM2_CHR(chr + alt));
	}
}

#endif /* CONFIG_DISTRICT_GUILD */
