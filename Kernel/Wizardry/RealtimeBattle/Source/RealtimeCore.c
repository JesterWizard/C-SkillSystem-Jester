#include "common-chax.h"
#include "kernel-lib.h"
#include "kernel/realtime-battle.h"

/* gRealtimeBattleState is reserved in config-memmap.s */

enum {
	REALTIME_INTERVAL_FALLBACK_FRAMES = 60,
	REALTIME_REFRESH_FALLBACK_FRAMES = 60 * 30,
};

bool IsRealtimeBattleEnabled(void)
{
	/* Build-time designer config is the master switch. */
	return gpKernelDesignerConfig->real_time_battle != 0;
}

bool IsRealtimeBattleActive(void)
{
	return IsRealtimeBattleEnabled() && gRealtimeBattleState.active;
}

u16 RealtimeBattle_GetIntervalFrames(void)
{
	u16 frames = gpKernelDesignerConfig->real_time_enemy_interval_frames;

	if (frames == 0)
		frames = REALTIME_INTERVAL_FALLBACK_FRAMES;

	return frames;
}

u16 RealtimeBattle_GetRefreshFrames(void)
{
	u16 frames = gpKernelDesignerConfig->real_time_refresh_frames;

	if (frames == 0)
		frames = REALTIME_REFRESH_FALLBACK_FRAMES;

	return frames;
}

void RealtimeBattle_InitChapter(ProcPtr proc)
{
	(void)proc;
	CpuFill16(0, &gRealtimeBattleState, sizeof(gRealtimeBattleState));

	if (!IsRealtimeBattleEnabled())
		return;

	gRealtimeBattleState.scheduleTimer = RealtimeBattle_GetIntervalFrames();
	gRealtimeBattleState.refreshTimer = RealtimeBattle_GetRefreshFrames();

	/*
	 * Do not start the scheduler here. ChapterInitHook runs before the map
	 * deployment table is populated; BmMain_StartPhase starts it after units
	 * are loaded.
	 */
}

void RealtimeBattle_Shutdown(void)
{
	CpuFill16(0, &gRealtimeBattleState, sizeof(gRealtimeBattleState));
}

void RealtimeBattle_Pause(u8 reason)
{
	gRealtimeBattleState.paused |= reason;
}

void RealtimeBattle_Resume(u8 reason)
{
	gRealtimeBattleState.paused &= ~reason;
}

bool RealtimeBattle_TryAcquireGate(u8 ownerId)
{
	if (ownerId == 0)
		return false;

	if (gRealtimeBattleState.gateOwner != 0 && gRealtimeBattleState.gateOwner != ownerId)
		return false;

	gRealtimeBattleState.gateOwner = ownerId;
	return true;
}

void RealtimeBattle_ReleaseGate(u8 ownerId)
{
	if (gRealtimeBattleState.gateOwner == ownerId)
		gRealtimeBattleState.gateOwner = 0;
}

bool RealtimeBattle_IsGateFree(void)
{
	return gRealtimeBattleState.gateOwner == 0;
}

bool RealtimeBattle_ShouldSkipEnemyPhase(void)
{
	return IsRealtimeBattleActive();
}

bool RealtimeBattle_ForceMapAnims(void)
{
	return IsRealtimeBattleActive();
}

void RealtimeBattle_SnapCameraOntoPosition(int x, int y)
{
	gBmSt.camera.x = GetCameraCenteredX(x * 16);
	gBmSt.camera.y = GetCameraCenteredY(y * 16);
	gBmSt.cameraPrevious = gBmSt.camera;
}

void RealtimeBattle_OnPlayerActionBegin(struct Unit *unit)
{
	u8 ownerId;
	struct Unit *owner;

	if (!IsRealtimeBattleActive() || !unit || UNIT_FACTION(unit) != FACTION_BLUE)
		return;

	/*
	 * UnitBeginAction is reached only after PlayerPhase_MainIdle has checked
	 * the gate. Pair-up can replace the selected unit before this hook runs,
	 * so move ownership to the actual active unit when the old blue owner is
	 * still visible.
	 */
	ownerId = gRealtimeBattleState.gateOwner;
	if (ownerId != 0 && ownerId != unit->index
		&& (ownerId & 0xC0) == FACTION_BLUE) {
		owner = GetUnit(ownerId);

		if (!owner || !(owner->state & US_HIDDEN))
			RealtimeBattle_ReleaseGate(ownerId);
	}

	if (RealtimeBattle_TryAcquireGate(unit->index))
		RealtimeBattle_Pause(RT_PAUSE_GATE | RT_PAUSE_PLAYER_ACTION);
}

void RealtimeBattle_OnPlayerActionEnd(struct Unit *unit)
{
	u8 ownerId;

	(void)unit;

	if (!IsRealtimeBattleActive())
		return;

	/*
	 * Pair-up / Turncoat can replace gActiveUnit before FinishAction. Release
	 * only from the stable gate owner id, and only if that id is a blue slot
	 * (never an enemy-owned gate).
	 */
	ownerId = gRealtimeBattleState.gateOwner;
	if (ownerId == 0)
		return;

	if ((ownerId & 0xC0) != FACTION_BLUE)
		return;

	RealtimeBattle_ReleaseGate(ownerId);
	RealtimeBattle_Resume(RT_PAUSE_GATE | RT_PAUSE_PLAYER_ACTION);
}

void RealtimeBattle_QuiesceForSuspend(void)
{
	if (!IsRealtimeBattleActive())
		return;

	RealtimeBattle_Pause(RT_PAUSE_SUSPEND);
	RealtimeBattle_EndInFlightAi();
	gRealtimeBattleState.gateOwner = 0;
	RealtimeBattle_Resume(RT_PAUSE_GATE | RT_PAUSE_PLAYER_ACTION);
}

void RealtimeBattle_QuiesceInFlight(void)
{
	u8 ownerId;

	if (!IsRealtimeBattleActive())
		return;

	/*
	 * Abort any enemy AI still running, but keep a player-owned gate (arena is
	 * entered mid player action and only adds RT_PAUSE_ARENA).
	 */
	RealtimeBattle_EndInFlightAi();

	ownerId = gRealtimeBattleState.gateOwner;
	if (ownerId != 0 && (ownerId & 0xC0) == FACTION_BLUE)
		return;

	gRealtimeBattleState.gateOwner = 0;
	RealtimeBattle_Resume(RT_PAUSE_GATE);
}
