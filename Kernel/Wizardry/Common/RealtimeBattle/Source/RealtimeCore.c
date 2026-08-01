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
	int i;

	(void)proc;
	CpuFill16(0, &gRealtimeBattleState, sizeof(gRealtimeBattleState));

	if (!IsRealtimeBattleEnabled())
		return;

	gRealtimeBattleState.scheduleTimer = RealtimeBattle_GetIntervalFrames();
	gRealtimeBattleState.refreshTimer = RealtimeBattle_GetRefreshFrames();

	for (i = 0; i < REALTIME_ACTION_SLOTS; i++)
		gRealtimeBattleState.slots[i].state = RT_SLOT_FREE;

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

bool RealtimeBattle_TileReserved(int x, int y, u8 exceptUnitId)
{
	int i;

	for (i = 0; i < REALTIME_ACTION_SLOTS; i++) {
		struct RealtimeActionContext *slot = &gRealtimeBattleState.slots[i];

		if (slot->state == RT_SLOT_FREE)
			continue;

		if (slot->unitId == exceptUnitId)
			continue;

		if (slot->reservedTileX == x && slot->reservedTileY == y)
			return true;
	}

	return false;
}

void RealtimeBattle_MarkTileReserved(u8 slotIdx, int x, int y)
{
	if (slotIdx >= REALTIME_ACTION_SLOTS)
		return;

	gRealtimeBattleState.slots[slotIdx].reservedTileX = x;
	gRealtimeBattleState.slots[slotIdx].reservedTileY = y;
}

void RealtimeBattle_ClearTileReserved(u8 slotIdx)
{
	if (slotIdx >= REALTIME_ACTION_SLOTS)
		return;

	gRealtimeBattleState.slots[slotIdx].reservedTileX = 0xFF;
	gRealtimeBattleState.slots[slotIdx].reservedTileY = 0xFF;
}

void RealtimeBattle_OnPlayerActionBegin(struct Unit *unit)
{
	(void)unit;
	/* Intentionally empty: selection pause is inferred from unit state / gBmSt.lock. */
}

void RealtimeBattle_OnPlayerActionEnd(struct Unit *unit)
{
	(void)unit;
	RealtimeBattle_Resume(RT_PAUSE_PLAYER_ACTION);
}

void RealtimeBattle_QuiesceForSuspend(void)
{
	RealtimeBattle_Pause(RT_PAUSE_SUSPEND);
	RealtimeBattle_QuiesceInFlight();
}

void RealtimeBattle_QuiesceInFlight(void)
{
	int i;

	if (!IsRealtimeBattleActive())
		return;

	for (i = 0; i < REALTIME_ACTION_SLOTS; i++) {
		struct RealtimeActionContext *slot = &gRealtimeBattleState.slots[i];

		if (slot->state == RT_SLOT_FREE)
			continue;

		RealtimeBattle_ClearTileReserved(i);
		slot->state = RT_SLOT_FREE;
	}

	gRealtimeBattleState.inFlightCount = 0;
	gRealtimeBattleState.gateOwner = 0;

	/* An action aborted mid-flight would otherwise leave the map locked. */
	if (gRealtimeBattleState.gameLocked) {
		gRealtimeBattleState.gameLocked = 0;
		UnlockGame();
	}

	gRealtimeBattleState.sideWindowsHidden = 0;
}
