#pragma once

#include "common-chax.h"

enum RealtimeActionSlotState {
	RT_SLOT_FREE = 0,
	RT_SLOT_PENDING,
	RT_SLOT_MOVING,
	RT_SLOT_COMMITTING,
	RT_SLOT_ANIMATING,
};

enum RealtimePauseReason {
	RT_PAUSE_NONE = 0,
	RT_PAUSE_EVENT = (1 << 0),
	RT_PAUSE_ARENA = (1 << 1),
	RT_PAUSE_SUSPEND = (1 << 2),
	RT_PAUSE_PLAYER_ACTION = (1 << 3),
	RT_PAUSE_GATE = (1 << 4),
};

#define REALTIME_ACTION_SLOTS 4
#define REALTIME_ENEMY_COOLDOWN_LEN 52
#define REALTIME_REFRESH_FRAMES (60 * 30) /* soft "turn" refresh */

struct RealtimeActionContext {
	u8 state;
	u8 unitId;
	u8 targetId;
	u8 actionId;
	u8 itemSlot;
	u8 xMove;
	u8 yMove;
	u8 xTarget;
	u8 yTarget;
	u8 reservedTileX;
	u8 reservedTileY;
	u8 pad;
};

struct RealtimeBattleState {
	u8 active;
	u8 paused;
	u8 gateOwner; /* unit index holding critical globals; 0 = free */
	u8 inFlightCount;
	u16 scheduleTimer;
	u16 refreshTimer;
	u8 nextEnemyIndex;
	u8 gameLocked; /* set while an enemy action holds LockGame() */
	struct RealtimeActionContext slots[REALTIME_ACTION_SLOTS];
	u8 enemyCooldown[REALTIME_ENEMY_COOLDOWN_LEN];
	struct Vec2 pinnedCursor; /* player cursor held in place during enemy actions */
	struct Vec2 pinnedCamera; /* view held in place so the cursor stays on screen */
	u8 sideWindowsHidden;     /* set while combat graphics own the window layers */
	u8 pad2;
};

_Static_assert(sizeof(struct RealtimeActionContext) == 12, "RealtimeActionContext size");
_Static_assert(sizeof(struct RealtimeBattleState) == 120, "RealtimeBattleState size");

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
bool RealtimeBattle_TileReserved(int x, int y, u8 exceptUnitId);
void RealtimeBattle_MarkTileReserved(u8 slot, int x, int y);
void RealtimeBattle_ClearTileReserved(u8 slot);
