#include "common-chax.h"
#include "kernel-lib.h"
#include "kernel/realtime-battle.h"

/* gRealtimeBattleState is reserved in config-memmap.s */

enum {
	REALTIME_TURN_FALLBACK_FRAMES = 60 * 30,
};

/* Values behind the "RT Enemy Delay" option (1s, 2s, 3s, 5s) */
static const u16 sRealtimeIntervalOptionFrames[] = { 60, 120, 180, 300 };

bool IsRealtimeBattleEnabled(void)
{
	/* The designer config is the master switch, the options menu can opt out (0 = ON). */
	return gpKernelDesignerConfig->real_time_battle != 0 && gPlaySt.config.real_time_battle == 0;
}

bool IsRealtimeBattleActive(void)
{
	return IsRealtimeBattleEnabled();
}

u16 RealtimeBattle_GetIntervalFrames(void)
{
	return sRealtimeIntervalOptionFrames[gPlaySt.config.real_time_interval];
}

u16 RealtimeBattle_GetTurnFrames(void)
{
	u16 frames = gpKernelDesignerConfig->real_time_turn_frames;

	if (frames == 0)
		frames = REALTIME_TURN_FALLBACK_FRAMES;

	return frames;
}

int RealtimeBattle_GetTurnSecondsLeft(void)
{
	if (!gRealtimeBattleState.valid)
		return RealtimeBattle_GetTurnFrames() / 60;

	return (gRealtimeBattleState.turnTimer + 59) / 60;
}

int RealtimeBattle_IntervalToOption(int frames)
{
	int i;

	for (i = 0; i < (int)ARRAY_COUNT(sRealtimeIntervalOptionFrames) - 1; i++) {
		int mid = (sRealtimeIntervalOptionFrames[i] + sRealtimeIntervalOptionFrames[i + 1]) / 2;

		if (frames <= mid)
			return i;
	}

	return ARRAY_COUNT(sRealtimeIntervalOptionFrames) - 1;
}

void RealtimeBattle_InitChapter(ProcPtr proc)
{
	(void)proc;
	CpuFill16(0, &gRealtimeBattleState, sizeof(gRealtimeBattleState));
}

bool RealtimeBattle_IsAiActing(void)
{
	return gRealtimeBattleState.aiUnit != 0;
}

bool RealtimeBattle_ForceMapAnims(void)
{
	/* Only the AI actions that interrupt the player are forced onto the map */
	return IsRealtimeBattleActive() && RealtimeBattle_IsAiActing();
}
