#pragma once

#include "common-chax.h"

enum RealtimePauseReason {
	RT_PAUSE_NONE = 0,
	RT_PAUSE_EVENT = (1 << 0),
	RT_PAUSE_ARENA = (1 << 1),
	RT_PAUSE_SUSPEND = (1 << 2),
	RT_PAUSE_PLAYER_ACTION = (1 << 3),
	RT_PAUSE_GATE = (1 << 4),
};

#define REALTIME_ENEMY_COOLDOWN_LEN 52
#define REALTIME_REFRESH_FRAMES (60 * 30) /* soft "turn" refresh */

struct RealtimeBattleState {
	u8 active;
	u8 paused;
	u8 gateOwner; /* unit index holding the shared action state; 0 = free */
	u8 pad;
	u16 scheduleTimer;
	u16 refreshTimer;
	u8 nextEnemyIndex;
	u8 pad2;
	u8 enemyCooldown[REALTIME_ENEMY_COOLDOWN_LEN];
};

_Static_assert(sizeof(struct RealtimeBattleState) == 62, "RealtimeBattleState size");

extern struct RealtimeBattleState gRealtimeBattleState;

bool IsRealtimeBattleEnabled(void);
bool IsRealtimeBattleActive(void);
u16 RealtimeBattle_GetIntervalFrames(void);

u16 RealtimeBattle_GetRefreshFrames(void);

void RealtimeBattle_InitChapter(ProcPtr proc);
void RealtimeBattle_Shutdown(void);
void RealtimeBattle_StartScheduler(ProcPtr parent);
void RealtimeBattle_Pause(u8 reason);
void RealtimeBattle_Resume(u8 reason);
void RealtimeBattle_QuiesceForSuspend(void);
void RealtimeBattle_QuiesceInFlight(void);

bool RealtimeBattle_TryAcquireGate(u8 ownerId);
void RealtimeBattle_ReleaseGate(u8 ownerId);
bool RealtimeBattle_IsGateFree(void);

bool RealtimeBattle_ShouldSkipEnemyPhase(void);
bool RealtimeBattle_HandleEndPlayerPhase(ProcPtr playerPhaseProc);
void RealtimeBattle_OnPlayerActionBegin(struct Unit *unit);
void RealtimeBattle_OnPlayerActionEnd(struct Unit *unit);

bool RealtimeBattle_ForceMapAnims(void);

/* Instant camera move that never starts a blocking CamMove proc. */
void RealtimeBattle_SnapCameraOntoPosition(int x, int y);

/* End any in-flight realtime AI unit and run its end callback. */
void RealtimeBattle_EndInFlightAi(void);
