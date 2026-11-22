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
 * A notice board over each board spot (the district's spot icons, DistrictIcons.c). While a quest on this board is met
 * and waiting to be claimed, it shows in turn with a red notice.
 */
void Guild_DrawMapMarkers(void)
{
	struct Trap *trap;
	int claim = ((GetGameClock() >> 5) & 1) && Guild_AnyClaimable(gPlaySt.chapterIndex);

	for (trap = GetTrap(0); trap->type != TRAP_NONE; trap++)
		if (trap->type == TRAP_GUILD_BOARD)
			District_PutSpotIcon(trap->xPos, trap->yPos, claim ? DISTRICT_ICON_BOARD_CLAIM : DISTRICT_ICON_BOARD);
}

#endif /* CONFIG_DISTRICT_GUILD */
