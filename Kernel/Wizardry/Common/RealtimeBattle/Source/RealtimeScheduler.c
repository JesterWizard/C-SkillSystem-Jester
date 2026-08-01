#include "common-chax.h"
#include "kernel-lib.h"
#include "kernel/realtime-battle.h"
#include "kernel/debug-kit.h"
#include "ai-hack.h"

/* Temporary instrumentation: set to 0 once real-time scheduling is confirmed. */
#define REALTIME_TRACE 1

/* Bump on every rebuild so a stale ROM is obvious in the log. */
#define REALTIME_BUILD_ID "rt-build-15"

#if REALTIME_TRACE
#define RT_TRACE(format, ...) LogInfof("[RT] "format, __VA_ARGS__)
#define RT_TRACE_IF(cond, format, ...)                                                                                 \
	do {                                                                                                               \
		if (cond)                                                                                                      \
			LogInfof("[RT] "format, __VA_ARGS__);                                                                      \
	} while (0)
#else
#define RT_TRACE(format, ...)
#define RT_TRACE_IF(cond, format, ...)
#endif

struct RealtimeSchedulerProc {
	PROC_HEADER;
	u16 traceTick;
};

struct RealtimeAiProc {
	PROC_HEADER;
	u8 unitId;
	u8 savedFaction;
	u8 savedActiveId;
	struct Unit *savedActive;
};

_Static_assert(sizeof(struct RealtimeAiProc) <= sizeof(struct Proc), "RealtimeAiProc must fit Proc allocation");

static void RealtimeScheduler_OnLoop(struct RealtimeSchedulerProc *proc);
static void RealtimeScheduler_TickCooldowns(void);
static void RealtimeScheduler_TickRefresh(void);
static void RealtimeScheduler_TryRunEnemy(struct RealtimeSchedulerProc *proc);
static struct Unit *RealtimeScheduler_GetEnemyAtSlot(int slot);
static struct Unit *RealtimeScheduler_GetEnemyById(u8 unitId);
static int RealtimeScheduler_FindReadyEnemy(void);
static bool RealtimeScheduler_MapIsIdle(bool trace);
static bool RealtimeScheduler_HasLoadedEnemy(void);
static void RealtimeScheduler_RefreshPlayerUnits(void);
static void RealtimeScheduler_RefreshEnemyCooldowns(void);
static void RealtimeScheduler_ForceAggressiveAi(void);
static void RealtimeGuard_OnLoop(ProcPtr proc);
static void RealtimeAi_Start(struct RealtimeAiProc *proc);
static void RealtimeAi_TryForcePursuit(struct RealtimeAiProc *proc);
static void RealtimeAi_End(struct RealtimeAiProc *proc);
static void RealtimeAi_PrepareVanillaState(void);

static const struct ProcCmd ProcScr_RealtimeBattleScheduler[] = {
	PROC_NAME("RealtimeBattle"),
	PROC_YIELD,
	PROC_REPEAT(RealtimeScheduler_OnLoop),
	PROC_END,
};

/*
 * Drives one enemy through the vanilla AI pipeline. CpDecide_Main performs the
 * map refresh, decision, and starts CpPerform, which owns movement, combat, and
 * post-action hooks. Doing any of that by hand desynchronises the action state.
 */
static const struct ProcCmd ProcScr_RealtimeAiUnit[] = {
	PROC_NAME("RealtimeAiUnit"),
	PROC_YIELD,
	PROC_CALL(RealtimeAi_Start),
	PROC_YIELD,
	PROC_CALL(RealtimeAi_TryForcePursuit),
	PROC_YIELD,
	PROC_CALL(RealtimeAi_End),
	PROC_END,
};

/*
 * Started after the AI proc so it runs later in the same frame: the AI subtree
 * has already retargeted the cursor by the time this undoes it, so the player
 * phase never gets to draw the moved position.
 */
static const struct ProcCmd ProcScr_RealtimeActionGuard[] = {
	PROC_NAME("RealtimeGuard"),
	PROC_REPEAT(RealtimeGuard_OnLoop),
	PROC_END,
};

void RealtimeBattle_StartScheduler(ProcPtr parent)
{
	if (!IsRealtimeBattleActive()) {
		RT_TRACE("start refused, enabled=%d active=%d", IsRealtimeBattleEnabled(), gRealtimeBattleState.active);
		return;
	}

	if (Proc_Find(ProcScr_RealtimeBattleScheduler)) {
		RT_TRACE("scheduler already running%s", "");
		return;
	}

	(void)parent;
	Proc_Start(ProcScr_RealtimeBattleScheduler, PROC_TREE_3);
	RT_TRACE("scheduler started, interval=%d build=%s", RealtimeBattle_GetIntervalFrames(), REALTIME_BUILD_ID);
}

static void RealtimeScheduler_OnLoop(struct RealtimeSchedulerProc *proc)
{
	bool trace;

	proc->traceTick++;
	trace = proc->traceTick >= 120;

	if (trace)
		proc->traceTick = 0;

	if (!IsRealtimeBattleActive()) {
		RT_TRACE("scheduler ending, active=%d", gRealtimeBattleState.active);
		Proc_End(proc);
		return;
	}

	/*
	 * The scheduler lives at root so the blocking player phase cannot stall it,
	 * which means it also survives into menus and the world map. Everything
	 * below touches map/AI state, so it must only run while the map is idle.
	 */
	if (!RealtimeScheduler_MapIsIdle(trace))
		return;

	RealtimeScheduler_TickCooldowns();
	RealtimeScheduler_TickRefresh();

	if (gRealtimeBattleState.scheduleTimer > 0) {
		gRealtimeBattleState.scheduleTimer--;
		return;
	}

	gRealtimeBattleState.scheduleTimer = RealtimeBattle_GetIntervalFrames();
	RealtimeScheduler_TryRunEnemy(proc);
}

/* Mirrors BuildAiUnitList_Proc: the engine addresses AI units by id, not by array. */
static struct Unit *RealtimeScheduler_GetEnemyAtSlot(int slot)
{
	if (slot < 1 || slot > CONFIG_UNIT_AMT_ENEMY)
		return NULL;

	return GetUnit(FACTION_RED + slot);
}

static struct Unit *RealtimeScheduler_GetEnemyById(u8 unitId)
{
	return RealtimeScheduler_GetEnemyAtSlot(unitId - FACTION_RED);
}

/* Returns 0 when the map is idle, otherwise a code naming the blocker. */
static int RealtimeScheduler_MapIdleBlocker(void)
{
	if (gRealtimeBattleState.paused & (RT_PAUSE_ARENA | RT_PAUSE_SUSPEND | RT_PAUSE_GATE))
		return 1;

	/* Not on a live map (title screen, world map, prep screen, ...). */
	if (!Proc_Find(gProcScr_PlayerPhase))
		return 2;

	if (gBmSt.gameStateBits & BM_FLAG_PREPSCREEN)
		return 3;

	if (!RealtimeScheduler_HasLoadedEnemy())
		return 8;

	/* A menu, full-screen UI, or cutscene owns the display or game logic. */
	if (gBmSt.lock)
		return 4;

	if (gBmSt.gameGfxSemaphore)
		return 5;

	if (EventEngineExists())
		return 6;

	/* Player unit is mid-action (selected, moving, or in combat). */
	if (gActiveUnit && UNIT_FACTION(gActiveUnit) == FACTION_BLUE && (gActiveUnit->state & US_HIDDEN))
		return 7;

	return 0;
}

static bool RealtimeScheduler_HasLoadedEnemy(void)
{
	int i;

	for (i = 1; i <= CONFIG_UNIT_AMT_ENEMY; i++) {
		if (UNIT_IS_VALID(RealtimeScheduler_GetEnemyAtSlot(i)))
			return true;
	}

	return false;
}

static bool RealtimeScheduler_MapIsIdle(bool trace)
{
	int blocker = RealtimeScheduler_MapIdleBlocker();

	if (blocker != 0) {
		RT_TRACE_IF(trace, "idle blocked, reason=%d paused=%02X lock=%d gfx=%d", blocker, gRealtimeBattleState.paused,
					gBmSt.lock, gBmSt.gameGfxSemaphore);
		return false;
	}

	(void)trace;
	return true;
}

static void RealtimeScheduler_TickCooldowns(void)
{
	int i;

	for (i = 0; i < REALTIME_ENEMY_COOLDOWN_LEN; i++) {
		if (gRealtimeBattleState.enemyCooldown[i] > 0)
			gRealtimeBattleState.enemyCooldown[i]--;
	}
}

static void RealtimeScheduler_TickRefresh(void)
{
	if (gRealtimeBattleState.refreshTimer > 0) {
		gRealtimeBattleState.refreshTimer--;
		return;
	}

	gRealtimeBattleState.refreshTimer = RealtimeBattle_GetRefreshFrames();
	RealtimeScheduler_RefreshPlayerUnits();
	RealtimeScheduler_RefreshEnemyCooldowns();
	RealtimeScheduler_ForceAggressiveAi();

	if (gPlaySt.chapterTurnNumber < 999)
		gPlaySt.chapterTurnNumber++;
}

static void RealtimeScheduler_RefreshPlayerUnits(void)
{
	int i;

	for (i = 1; i < CONFIG_UNIT_AMT_ALLY + 1; i++) {
		struct Unit *unit = GetUnit(i);

		if (!UNIT_IS_VALID(unit))
			continue;

		if (unit->state & (US_DEAD | US_NOT_DEPLOYED))
			continue;

		unit->state &= ~(US_UNSELECTABLE | US_HAS_MOVED | US_HAS_MOVED_AI);
	}

	RefreshEntityBmMaps();
	RefreshUnitSprites();
}

/*
 * Real-time mode has no phases for a defensive script to wait on, so every
 * enemy is rewritten to the vanilla "attack, pursue" pair. Reinforcements are
 * covered because this runs on the periodic refresh rather than once at start.
 */
static void RealtimeScheduler_ForceAggressiveAi(void)
{
	int i;

	for (i = 1; i <= CONFIG_UNIT_AMT_ENEMY; i++) {
		struct Unit *unit = RealtimeScheduler_GetEnemyAtSlot(i);

		if (!UNIT_IS_VALID(unit))
			continue;

		if (unit->state & (US_DEAD | US_NOT_DEPLOYED))
			continue;

		if (unit->ai1 == AI_A_00 && unit->ai2 == AI_B_00)
			continue;

		unit->ai1 = AI_A_00;
		unit->ai2 = AI_B_00;
		unit->ai_a_pc = 0;
		unit->ai_b_pc = 0;
	}
}

static void RealtimeScheduler_RefreshEnemyCooldowns(void)
{
	int i;

	for (i = 0; i < REALTIME_ENEMY_COOLDOWN_LEN; i++)
		gRealtimeBattleState.enemyCooldown[i] = 0;
}

static int RealtimeScheduler_FindReadyEnemy(void)
{
	int start = gRealtimeBattleState.nextEnemyIndex;
	int offset;
	int invalidCount = 0;
	int blockedStateCount = 0;
	int blockedStatusCount = 0;
	int cooldownCount = 0;
	struct Unit *probe;

	if (start < 1 || start > CONFIG_UNIT_AMT_ENEMY)
		start = 1;

	for (offset = 0; offset < CONFIG_UNIT_AMT_ENEMY; offset++) {
		/*
		 * Wrap by subtraction. The toolchain resolves __aeabi_idivmod to
		 * __modsi3, which returns its result in r0 while the generated code
		 * reads r1, so integer division must not be used here.
		 */
		int i = start + offset;

		if (i > CONFIG_UNIT_AMT_ENEMY)
			i -= CONFIG_UNIT_AMT_ENEMY;

		struct Unit *unit = RealtimeScheduler_GetEnemyAtSlot(i);

		if (!UNIT_IS_VALID(unit)) {
			invalidCount++;
			continue;
		}

		/*
		 * US_UNSELECTABLE / US_HAS_MOVED_AI are phase-scoped flags that vanilla
		 * clears when the enemy phase begins. Real-time mode never enters that
		 * phase, so eligibility is driven by the per-unit cooldown instead.
		 */
		if (unit->state & (US_HIDDEN | US_DEAD | US_RESCUED)) {
			blockedStateCount++;
			continue;
		}

		if (unit->statusIndex == UNIT_STATUS_SLEEP || unit->statusIndex == UNIT_STATUS_BERSERK) {
			blockedStatusCount++;
			continue;
		}

		if (gRealtimeBattleState.enemyCooldown[i] != 0) {
			cooldownCount++;
			continue;
		}

		gRealtimeBattleState.nextEnemyIndex = (i >= CONFIG_UNIT_AMT_ENEMY) ? 1 : (u8)(i + 1);
		return (u8)unit->index;
	}

	probe = RealtimeScheduler_GetEnemyAtSlot(1);
	RT_TRACE("no ready inv=%d st=%d cd=%d u81=%08X c81=%08X", invalidCount, blockedStateCount + blockedStatusCount,
			 cooldownCount, (unsigned)probe, probe ? (unsigned)probe->pCharacterData : 0);
	return 0;
}

static void RealtimeScheduler_TryRunEnemy(struct RealtimeSchedulerProc *proc)
{
	struct RealtimeAiProc *ai;
	int unitId = RealtimeScheduler_FindReadyEnemy();

	(void)proc;

	if (unitId == 0) {
		RT_TRACE("no ready enemy%s", "");
		return;
	}

	if (!RealtimeBattle_TryAcquireGate(unitId)) {
		RT_TRACE("gate busy, owner=%02X", gRealtimeBattleState.gateOwner);
		return;
	}

	RT_TRACE("running enemy %02X", unitId);

	/* Serialise: shared AI/action/animation globals allow one actor at a time. */
	RealtimeBattle_Pause(RT_PAUSE_GATE);

	/*
	 * Combat, map animations and gActionData are single-instance. Holding the
	 * map lock for the action stops the player from starting a second one
	 * through the still-running player phase, which corrupts those globals.
	 */
	if (!gRealtimeBattleState.gameLocked) {
		LockGame();
		gRealtimeBattleState.gameLocked = 1;
	}

	gRealtimeBattleState.pinnedCursor = gBmSt.playerCursor;
	gRealtimeBattleState.pinnedCamera = gBmSt.camera;

	ai = Proc_Start(ProcScr_RealtimeAiUnit, PROC_TREE_3);
	ai->unitId = unitId;
	ai->savedFaction = gPlaySt.faction;

	Proc_Start(ProcScr_RealtimeActionGuard, PROC_TREE_3);

	gRealtimeBattleState.inFlightCount = 1;
	gRealtimeBattleState.slots[0].state = RT_SLOT_COMMITTING;
	gRealtimeBattleState.slots[0].unitId = unitId;
}

static void RealtimeGuard_OnLoop(ProcPtr proc)
{
	if (gRealtimeBattleState.gateOwner == 0) {
		Proc_End(proc);
		return;
	}

	/* Belt and braces: AiRefreshMap no longer moves it, other steps might. */
	gBmSt.playerCursor = gRealtimeBattleState.pinnedCursor;
	gBmSt.cursorPrevious = gRealtimeBattleState.pinnedCursor;
	gBmSt.cursorTarget = gRealtimeBattleState.pinnedCursor;

	/*
	 * The AI pans the camera onto whatever it is doing. The cursor sprite is
	 * clamped to the visible area, so a camera that walks off leaves the cursor
	 * stuck against the screen edge even though its tile never changed. Hold the
	 * view still instead. The engine's camera proc runs on its own countdown and
	 * ends regardless of where the camera actually is, so overwriting its result
	 * cannot stall the action.
	 */
	gBmSt.camera = gRealtimeBattleState.pinnedCamera;
	gBmSt.cameraPrevious = gRealtimeBattleState.pinnedCamera;

	/*
	 * The side windows share BG0/BG1 with combat and talk graphics. The player
	 * phase is still idling underneath, and it rebuilds them as soon as they go
	 * away, so they have to be ended every frame the action runs rather than
	 * once. The first pass also clears what they already drew; later passes must
	 * not, or they would wipe the combat UI drawn into the same layers.
	 */
	if (!gRealtimeBattleState.sideWindowsHidden) {
		gRealtimeBattleState.sideWindowsHidden = 1;
		EndPlayerPhaseSideWindows();
		return;
	}

	Proc_EndEach(gProcScr_UnitDisplay_MinimugBox);
	Proc_EndEach(gProcScr_UnitDisplay_Burst);
	Proc_EndEach(gProcScr_TerrainDisplay);
	Proc_EndEach(gProcScr_GoalDisplay);
	Proc_EndEach(gProcScr_PrepMap_MenuButtonDisplay);
}

static void RealtimeAi_Start(struct RealtimeAiProc *proc)
{
	struct Unit *unit = RealtimeScheduler_GetEnemyById(proc->unitId);

	proc->savedActive = gActiveUnit;
	proc->savedActiveId = gActiveUnitId;

	if (!UNIT_IS_VALID(unit)) {
		RT_TRACE("invalid unit %02X", proc->unitId);
		return;
	}

	/* AI scripts and target selection read the acting unit's faction. */
	gPlaySt.faction = UNIT_FACTION(unit);
	unit->state &= ~(US_UNSELECTABLE | US_HAS_MOVED | US_HAS_MOVED_AI);

	/*
	 * CpPhaseInit normally performs this setup before CpOrder starts. The
	 * real-time scheduler never enters CpPhase, so reproduce that setup before
	 * invoking the vanilla order/decide pipeline.
	 */
	RealtimeAi_PrepareVanillaState();

	/*
	 * CpOrder would rebuild the complete enemy list. The scheduler intentionally
	 * supplies a one-entry list so only this timer slot can act.
	 */
	gAiState.units[0] = proc->unitId;
	gAiState.units[1] = 0;
	gAiState.unitIt = gAiState.units;

	AiDecideMainFunc = AiDecideMain;

	RT_TRACE("decide start unit=%02X pos=%d,%d state=%08X ai=%02X/%02X", proc->unitId, unit->xPos, unit->yPos,
			 (unsigned)unit->state, unit->ai1, unit->ai2);

	Proc_StartBlocking(gProcScr_CpDecide, proc);
}

static struct Unit *RealtimeAi_FindNearestEnemy(struct Unit *actor)
{
	struct Unit *best = NULL;
	int bestDistance = 0x7FFF;
	int i;

	for (i = 1; i < 0xE0; i++) {
		struct Unit *candidate = GetUnit(i);
		int dx;
		int dy;
		int distance;

		if (!UNIT_IS_VALID(candidate) || candidate == actor)
			continue;

		if (candidate->state & (US_HIDDEN | US_DEAD | US_NOT_DEPLOYED | US_RESCUED | US_BIT16))
			continue;

		if (AreUnitsAllied(actor->index, candidate->index))
			continue;

		dx = candidate->xPos - actor->xPos;
		dy = candidate->yPos - actor->yPos;

		if (dx < 0)
			dx = -dx;

		if (dy < 0)
			dy = -dy;

		distance = dx + dy;

		if (distance < bestDistance) {
			bestDistance = distance;
			best = candidate;
		}
	}

	return best;
}

static void RealtimeAi_TryForcePursuit(struct RealtimeAiProc *proc)
{
	struct Unit *unit = RealtimeScheduler_GetEnemyById(proc->unitId);
	struct Unit *target;

	/*
	 * Real-time mode expects every enemy to keep pressing. Any AI script that
	 * decided to sit still this tick - NeverMove, or a guard script whose
	 * trigger range is empty - gets overridden into a pursuit instead.
	 */
	if (!UNIT_IS_VALID(unit) || gAiDecision.actionPerformed)
		return;

	/*
	 * A unit that already ran CpPerform this tick must not run it a second time:
	 * gAiDecision still holds that action, and replaying it against a target
	 * that has since died reads freed state.
	 */
	if (unit->state & (US_DEAD | US_HIDDEN | US_HAS_MOVED | US_HAS_MOVED_AI))
		return;

	target = RealtimeAi_FindNearestEnemy(unit);

	if (!target) {
		RT_TRACE("force pursuit found no target for unit=%02X", proc->unitId);
		return;
	}

	/* Ask the vanilla planner for the destination so CpPerform stays in charge. */
	gActiveUnitId = proc->unitId;
	gActiveUnit = unit;

	/* Drop whatever the AI script left behind so CpPerform only sees the move. */
	AiClearDecision();
	AiTryMoveTowards(target->xPos, target->yPos, AI_ACTION_NONE, 0, 1);

	if (!gAiDecision.actionPerformed ||
		(gAiDecision.xMove == unit->xPos && gAiDecision.yMove == unit->yPos)) {
		RT_TRACE("force pursuit found no route unit=%02X target=%02X", proc->unitId, (u8)target->index);
		return;
	}

	RT_TRACE("force pursuit unit=%02X target=%02X move=%d,%d", proc->unitId, (u8)target->index,
			 gAiDecision.xMove, gAiDecision.yMove);
	AiPhaseMarkUnitPerformed(proc->unitId);
	Proc_StartBlocking(gProcScr_CpPerform, proc);
}

static void RealtimeAi_End(struct RealtimeAiProc *proc)
{
	struct Unit *unit = RealtimeScheduler_GetEnemyById(proc->unitId);

	gPlaySt.faction = proc->savedFaction;

	RT_TRACE("decide end unit=%02X performed=%d action=%d move=%d,%d", proc->unitId, gAiDecision.actionPerformed,
			 gAiDecision.actionId, gAiDecision.xMove, gAiDecision.yMove);

	if (UNIT_IS_VALID(unit)) {
		/* Stay ungreyed: the unit is only waiting on its real-time cooldown. */
		unit->state &= ~(US_UNSELECTABLE | US_HAS_MOVED_AI);

		if (proc->unitId > FACTION_RED) {
			int cdIdx = proc->unitId - FACTION_RED;

			if (cdIdx < REALTIME_ENEMY_COOLDOWN_LEN)
				gRealtimeBattleState.enemyCooldown[cdIdx] = RealtimeBattle_GetIntervalFrames();
		}
	}

	/* CpDecide changes these globals; restore the player-phase context. */
	gActiveUnit = proc->savedActive;
	gActiveUnitId = proc->savedActiveId;
	gAiState.flags = AI_FLAGS_NONE;

	gBmSt.playerCursor = gRealtimeBattleState.pinnedCursor;
	gBmSt.cursorPrevious = gRealtimeBattleState.pinnedCursor;
	gBmSt.cursorTarget = gRealtimeBattleState.pinnedCursor;
	gBmSt.camera = gRealtimeBattleState.pinnedCamera;
	gBmSt.cameraPrevious = gRealtimeBattleState.pinnedCamera;

	gRealtimeBattleState.slots[0].state = RT_SLOT_FREE;
	gRealtimeBattleState.inFlightCount = 0;

	if (gRealtimeBattleState.gameLocked) {
		gRealtimeBattleState.gameLocked = 0;
		UnlockGame();
	}

	/* The idling player phase rebuilds its own side windows once we stop
	 * tearing them down, so nothing has to restart them here. */
	gRealtimeBattleState.sideWindowsHidden = 0;

	RealtimeBattle_ReleaseGate(proc->unitId);
	RealtimeBattle_Resume(RT_PAUSE_GATE);

	RefreshEntityBmMaps();
	RefreshUnitSprites();
}

static void RealtimeAi_PrepareVanillaState(void)
{
	int i;

	gAiState.flags = AI_FLAG_0;
	gAiState.unk7E = 0xFF;
	gAiState.orderState = 0;

	for (i = 0; i < 8; i++)
		gAiState.cmd_result[i] = 0;

	gAiState.specialItemFlags = gAiItemConfigTable[gPlaySt.chapterIndex];
	gAiState.unk84 = 0;

	UpdateAllPhaseHealingAIStatus();
	SetupUnitInventoryAIFlags();
}

bool RealtimeBattle_HandleEndPlayerPhase(ProcPtr playerPhaseProc)
{
	if (!IsRealtimeBattleActive())
		return false;

	(void)playerPhaseProc;
	RealtimeScheduler_RefreshPlayerUnits();
	return true;
}
