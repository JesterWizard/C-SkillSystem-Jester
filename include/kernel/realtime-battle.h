#pragma once

#include "common-chax.h"

/*
 * Real-time battle mode.
 *
 * The phase loop keeps running, but only the player phase is interactive:
 * the enemy (and green) phases pass instantly at the end of each turn so their
 * start-of-phase bookkeeping, turn events and traps still happen. During the
 * player phase a clock runs while the player is on the free map cursor or
 * choosing where to move. Every enemy delay, one AI unit that has not acted
 * this turn interrupts the cursor and performs a normal AI action. When the
 * turn clock runs out (or the player has nothing left to do and every AI unit
 * has acted) the turn ends.
 */

struct RealtimeBattleState {
	u8 valid;         /* timers below belong to the current turn */
	u8 endRequested;  /* "End" was chosen: finish the AI units, then end the turn */
	u8 aiUnit;        /* unit id of the AI action in flight; 0 = none */
	u8 nextSlot;      /* round-robin cursor over the AI units */
	u16 enemyTimer;   /* frames until the next AI unit may act */
	u16 turnTimer;    /* frames left in this turn */
	u8 loopCount;     /* player phase loop iterations, counted by the clock proc */
	u8 idleLoop;      /* loopCount when PlayerPhase_MainIdle last ran */
	s8 cursorX;       /* player cursor saved across an AI action */
	s8 cursorY;
	u16 hudSeconds;   /* last value drawn into the goal window */
	u16 turn;         /* chapter turn the timers were started on */
};

_Static_assert(sizeof(struct RealtimeBattleState) == 16, "RealtimeBattleState size");

extern struct RealtimeBattleState gRealtimeBattleState;

bool IsRealtimeBattleEnabled(void);
bool IsRealtimeBattleActive(void);
u16 RealtimeBattle_GetIntervalFrames(void);
u16 RealtimeBattle_GetTurnFrames(void);
int RealtimeBattle_GetTurnSecondsLeft(void);

/* Maps a delay in frames onto the closest "RT Enemy Delay" option index. */
int RealtimeBattle_IntervalToOption(int frames);

void RealtimeBattle_InitChapter(ProcPtr proc);

/* Called at the top of PlayerPhase_MainIdle; true when it took over the proc. */
bool RealtimeBattle_OnPlayerPhaseIdle(ProcPtr playerPhaseProc);

/* The player chose "End" from the map menu. */
void RealtimeBattle_RequestEndTurn(void);

/* True while an AI unit is acting on the real-time clock. */
bool RealtimeBattle_IsAiActing(void);

bool RealtimeBattle_ForceMapAnims(void);

/* Goal window line showing the turn and the time left in it */
void RealtimeBattle_DrawGoalClock(struct Text *text);
bool RealtimeBattle_GoalClockChanged(void);
