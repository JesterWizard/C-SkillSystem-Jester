#include "common-chax.h"
#include "kernel-lib.h"
#include "kernel/realtime-battle.h"
#include "ai-hack.h"
#include "enemy-fog-vision.h"

struct RealtimeSchedulerProc {
	PROC_HEADER;
};

struct RealtimeAiProc {
	PROC_HEADER;
	u8 unitId;
	u8 savedFaction;
	u8 savedActiveId;
	struct Unit *savedActive;
	u8 started;
};

_Static_assert(sizeof(struct RealtimeAiProc) <= sizeof(struct Proc), "RealtimeAiProc must fit Proc allocation");

static void RealtimeScheduler_OnLoop(struct RealtimeSchedulerProc *proc);
static void RealtimeScheduler_TickCooldowns(void);
static void RealtimeScheduler_TickRefresh(void);
static void RealtimeScheduler_TryRunEnemy(void);
static void RealtimeScheduler_ClearPlayerSideWindows(void);
static void RealtimeScheduler_RestoreEnemyAfterAction(struct Unit *unit);
static struct Unit *RealtimeScheduler_GetEnemyAtSlot(int slot);
static struct Unit *RealtimeScheduler_GetEnemyById(u8 unitId);
static int RealtimeScheduler_FindReadyEnemy(void);
static bool RealtimeScheduler_MapIsIdle(void);
static bool RealtimeScheduler_HasLoadedEnemy(void);
static void RealtimeScheduler_RefreshPlayerUnits(void);
static void RealtimeScheduler_RefreshEnemyCooldowns(void);
static void RealtimeScheduler_ForceAggressiveAi(void);
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
 *
 * End-cb runs the same cleanup as the normal tail call so Proc_EndEach during
 * quiesce still restores faction / gate / active-unit globals.
 */
static const struct ProcCmd ProcScr_RealtimeAiUnit[] = {
	PROC_NAME("RealtimeAiUnit"),
	PROC_SET_END_CB(RealtimeAi_End),
	PROC_YIELD,
	PROC_CALL(RealtimeAi_Start),
	PROC_YIELD,
	PROC_CALL(RealtimeAi_TryForcePursuit),
	PROC_YIELD,
	PROC_CALL(RealtimeAi_End),
	PROC_END,
};

void RealtimeBattle_StartScheduler(ProcPtr parent)
{
	if (!IsRealtimeBattleActive()) {
		return;
	}

	if (Proc_Find(ProcScr_RealtimeBattleScheduler)) {
		return;
	}

	(void)parent;
	Proc_Start(ProcScr_RealtimeBattleScheduler, PROC_TREE_3);
}

void RealtimeBattle_EndInFlightAi(void)
{
	Proc_EndEach(ProcScr_RealtimeAiUnit);
}

static void RealtimeScheduler_OnLoop(struct RealtimeSchedulerProc *proc)
{
	if (!IsRealtimeBattleActive()) {
		Proc_End(proc);
		return;
	}

	/*
	 * PlayerPhase_MainIdle remains alive so the cursor can be moved freely.
	 * Side windows are cleared once when the gate is acquired; do not tear
	 * them down every frame or MapAnim BG layers get wiped mid-combat.
	 */

	/*
	 * The scheduler lives at root so the blocking player phase cannot stall it,
	 * which means it also survives into menus and the world map. Everything
	 * below touches map/AI state, so it must only run while the map is idle.
	 */
	if (!RealtimeScheduler_MapIsIdle())
		return;

	RealtimeScheduler_TickCooldowns();
	RealtimeScheduler_TickRefresh();

	if (gRealtimeBattleState.scheduleTimer > 0) {
		gRealtimeBattleState.scheduleTimer--;
		return;
	}

	gRealtimeBattleState.scheduleTimer = RealtimeBattle_GetIntervalFrames();
	RealtimeScheduler_TryRunEnemy();
}

static void RealtimeScheduler_ClearPlayerSideWindows(void)
{
	Proc_EndEach(gProcScr_UnitDisplay_MinimugBox);
	Proc_EndEach(gProcScr_UnitDisplay_Burst);
	Proc_EndEach(gProcScr_TerrainDisplay);
	Proc_EndEach(gProcScr_GoalDisplay);
	Proc_EndEach(gProcScr_PrepMap_MenuButtonDisplay);
}

static void RealtimeScheduler_RestoreEnemyAfterAction(struct Unit *unit)
{
	if (!UNIT_IS_VALID(unit))
		return;

	/*
	 * UnitBeginAction hides the actor and removes it from gBmMapUnit. Vanilla
	 * cleanup normally restores both, but an AI action can leave through a
	 * special-action path before that restoration runs.
	 */
	if (unit->curHP == 0
		|| (unit->state & (US_UNAVAILABLE | US_DEAD | US_RESCUED | US_NOT_DEPLOYED | US_BIT16))
		|| unit->xPos < 0 || unit->yPos < 0
		|| unit->xPos >= gBmMapSize.x || unit->yPos >= gBmMapSize.y)
		return;

	unit->state &= ~US_HIDDEN;

	if (gBmMapUnit[unit->yPos][unit->xPos] == 0)
		gBmMapUnit[unit->yPos][unit->xPos] = unit->index;

	ShowUnitSprite(unit);
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
	if (unitId <= FACTION_RED)
		return NULL;

	return RealtimeScheduler_GetEnemyAtSlot(unitId - FACTION_RED);
}

/* Returns 0 when the map is idle, otherwise a code naming the blocker. */
static int RealtimeScheduler_MapIdleBlocker(void)
{
	if (!RealtimeBattle_IsGateFree())
		return 1;

	if (gRealtimeBattleState.paused
		& (RT_PAUSE_EVENT | RT_PAUSE_ARENA | RT_PAUSE_SUSPEND | RT_PAUSE_PLAYER_ACTION | RT_PAUSE_GATE))
		return 1;

	/* Not on a live map (title screen, world map, prep screen, ...). */
	if (!Proc_Find(gProcScr_PlayerPhase))
		return 2;

	if (gBmSt.gameStateBits & BM_FLAG_PREPSCREEN)
		return 3;

	if (!RealtimeScheduler_HasLoadedEnemy())
		return 8;

	/*
	 * This is an external game lock from a menu or map animation. The
	 * realtime scheduler itself never sets it; the action gate handles RT
	 * serialization.
	 */
	if (gBmSt.lock)
		return 4;

	if (gBmSt.gameGfxSemaphore)
		return 5;

	if (EventEngineExists())
		return 6;

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

static bool RealtimeScheduler_MapIsIdle(void)
{
	int blocker = RealtimeScheduler_MapIdleBlocker();

	if (blocker != 0)
		return false;

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

		if (!UNIT_IS_VALID(unit))
			continue;

		/*
		 * US_UNSELECTABLE / US_HAS_MOVED_AI are phase-scoped flags that vanilla
		 * clears when the enemy phase begins. Real-time mode never enters that
		 * phase, so eligibility is driven by the per-unit cooldown instead.
		 */
		if (unit->state & (US_HIDDEN | US_DEAD | US_RESCUED))
			continue;

		if (unit->statusIndex == UNIT_STATUS_SLEEP || unit->statusIndex == UNIT_STATUS_BERSERK)
			continue;

		if (gRealtimeBattleState.enemyCooldown[i] != 0)
			continue;

		gRealtimeBattleState.nextEnemyIndex = (i >= CONFIG_UNIT_AMT_ENEMY) ? 1 : (u8)(i + 1);
		return (u8)unit->index;
	}

	return 0;
}

static void RealtimeScheduler_TryRunEnemy(void)
{
	struct RealtimeAiProc *ai;
	int unitId = RealtimeScheduler_FindReadyEnemy();

	if (unitId == 0)
		return;

	if (!RealtimeBattle_TryAcquireGate(unitId))
		return;

	/* Serialise: shared AI/action/animation globals allow one actor at a time. */
	RealtimeBattle_Pause(RT_PAUSE_GATE);

	ai = Proc_Start(ProcScr_RealtimeAiUnit, PROC_TREE_3);
	if (!ai) {
		RealtimeBattle_ReleaseGate(unitId);
		RealtimeBattle_Resume(RT_PAUSE_GATE);
		return;
	}

	EndPlayerPhaseSideWindows();
	RealtimeScheduler_ClearPlayerSideWindows();
	ai->unitId = unitId;
	ai->savedFaction = gPlaySt.faction;
	ai->savedActive = gActiveUnit;
	ai->savedActiveId = gActiveUnitId;
	ai->started = 0;
}

static void RealtimeAi_Start(struct RealtimeAiProc *proc)
{
	struct Unit *unit = RealtimeScheduler_GetEnemyById(proc->unitId);

	proc->savedActive = gActiveUnit;
	proc->savedActiveId = gActiveUnitId;

	if (!UNIT_IS_VALID(unit))
		return;

	proc->started = 1;

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

		if (!EnemyFogVisionCanTargetUnit(candidate))
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

	if (!target)
		return;

	/* Ask the vanilla planner for the destination so CpPerform stays in charge. */
	gActiveUnitId = proc->unitId;
	gActiveUnit = unit;

	/* Drop whatever the AI script left behind so CpPerform only sees the move. */
	AiClearDecision();
	AiTryMoveTowards(target->xPos, target->yPos, AI_ACTION_NONE, 0, 1);

	if (!gAiDecision.actionPerformed ||
		(gAiDecision.xMove == unit->xPos && gAiDecision.yMove == unit->yPos))
		return;

	AiPhaseMarkUnitPerformed(proc->unitId);
	Proc_StartBlocking(gProcScr_CpPerform, proc);
}

static void RealtimeAi_End(struct RealtimeAiProc *proc)
{
	struct Unit *unit;

	/* Idempotent: normal completion and PROC_SET_END_CB both reach here. */
	if (proc->unitId == 0)
		return;

	/*
	 * The proc can be cancelled after allocation but before its first script
	 * command runs. Its saved globals are then not initialized by
	 * RealtimeAi_Start, so only release the gate in that case.
	 */
	if (!proc->started) {
		RealtimeBattle_ReleaseGate(proc->unitId);
		RealtimeBattle_Resume(RT_PAUSE_GATE);
		proc->unitId = 0;
		return;
	}

	unit = RealtimeScheduler_GetEnemyById(proc->unitId);

	/*
	 * Turncoat / UnitChangeFaction can clear the original red slot and move
	 * the living actor into a new faction array entry. Prefer gActiveUnit.
	 */
	if (!UNIT_IS_VALID(unit)
		&& gActiveUnit != proc->savedActive
		&& UNIT_IS_VALID(gActiveUnit))
		unit = gActiveUnit;

	gPlaySt.faction = proc->savedFaction;

	if (UNIT_IS_VALID(unit)) {
		RealtimeScheduler_RestoreEnemyAfterAction(unit);

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

	RealtimeBattle_ReleaseGate(proc->unitId);
	RealtimeBattle_Resume(RT_PAUSE_GATE);
	proc->unitId = 0;

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
