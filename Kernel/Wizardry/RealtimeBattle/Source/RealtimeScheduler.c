#include "common-chax.h"
#include "kernel-lib.h"
#include "kernel/realtime-battle.h"

/* Replaced by AiOptimization; decides (and performs) the unit at gAiState.unitIt */
void CpDecide_Main(ProcPtr proc);

/* Minimum delay before an AI unit may act once the player is back on the cursor */
#define REALTIME_RESUME_GRACE_FRAMES 20

struct RealtimeClockProc {
	PROC_HEADER;
};

struct RealtimeAiProc {
	PROC_HEADER;

	/* 29 */ u8 unitId;
	/* 2A */ u8 savedFaction;
	/* 2B */ u8 savedActiveId;
	/* 2C */ struct Unit *savedActive;
	/* 30 */ u8 started;
};

static void RealtimeClock_OnLoop(struct RealtimeClockProc *proc);
static void RealtimeAi_Decide(struct RealtimeAiProc *proc);
static void RealtimeAi_RestoreGlobals(struct RealtimeAiProc *proc);
static void RealtimeAi_RefreshMap(struct RealtimeAiProc *proc);
static void RealtimeAi_ReturnCamera(struct RealtimeAiProc *proc);

/*
 * Child of the player phase proc, so it dies with the phase. Children keep
 * running while their parent is blocked, which is why the clock checks what
 * the player phase is doing before it ticks.
 */
static const struct ProcCmd ProcScr_RealtimeClock[] = {
	PROC_NAME("RealtimeClock"),
	PROC_REPEAT(RealtimeClock_OnLoop),
	PROC_END,
};

/*
 * One AI unit acting while the player phase is blocked on it. This is the
 * CpPhase -> CpOrder -> CpDecide chain cut down to a single unit, run from
 * RealtimeBattle_OnPlayerPhaseIdle: CpDecide_Main either starts CpPerform
 * blocking on this proc, or jumps to label 0 when the unit does nothing.
 */
static const struct ProcCmd ProcScr_RealtimeAiUnit[] = {
	PROC_NAME("RealtimeAiUnit"),
	PROC_SET_END_CB(RealtimeAi_RestoreGlobals),
	PROC_YIELD,

PROC_LABEL(0),
	PROC_CALL(RealtimeAi_RestoreGlobals),
	PROC_CALL(RealtimeAi_RefreshMap),
	PROC_CALL(RealtimeAi_ReturnCamera),
	PROC_YIELD,
	PROC_END,
};

static bool IsRealtimeAiUnitReady(struct Unit *unit)
{
	if (!UNIT_IS_VALID(unit))
		return false;

	/* Same filter as BuildAiUnitList, plus units that are not on the map */
	if (unit->statusIndex == UNIT_STATUS_SLEEP || unit->statusIndex == UNIT_STATUS_BERSERK)
		return false;

	if (unit->state & (US_HIDDEN | US_UNSELECTABLE | US_DEAD | US_RESCUED | US_HAS_MOVED_AI | US_NOT_DEPLOYED | US_BIT16))
		return false;

	return true;
}

#ifdef CONFIG_FOURTH_ALLEGIANCE
#define REALTIME_AI_SLOT_COUNT (CONFIG_UNIT_AMT_ENEMY + CONFIG_UNIT_AMT_NPC + CONFIG_UNIT_AMT_FOURTH)
#else
#define REALTIME_AI_SLOT_COUNT (CONFIG_UNIT_AMT_ENEMY + CONFIG_UNIT_AMT_NPC)
#endif

/* Round-robin slots: enemies, then green units, then the fourth allegiance */
static struct Unit *GetRealtimeAiSlotUnit(int slot)
{
	if (slot < CONFIG_UNIT_AMT_ENEMY)
		return GetUnit(FACTION_RED + 1 + slot);

	slot -= CONFIG_UNIT_AMT_ENEMY;

	if (slot < CONFIG_UNIT_AMT_NPC)
		return GetUnit(FACTION_GREEN + 1 + slot);

#ifdef CONFIG_FOURTH_ALLEGIANCE
	slot -= CONFIG_UNIT_AMT_NPC;

	if (gpKernelDesignerConfig->fourth_allegiance && slot < CONFIG_UNIT_AMT_FOURTH)
		return GetUnit(FACTION_PURPLE + 1 + slot);
#endif

	return NULL;
}

static int FindReadyRealtimeAiUnit(bool advance)
{
	int slotCount = REALTIME_AI_SLOT_COUNT;
	int slot = gRealtimeBattleState.nextSlot;
	int i;

	for (i = 0; i < slotCount; i++, slot++) {
		struct Unit *unit;

		if (slot >= slotCount)
			slot = 0;

		unit = GetRealtimeAiSlotUnit(slot);

		if (!IsRealtimeAiUnitReady(unit))
			continue;

		if (advance)
			gRealtimeBattleState.nextSlot = slot + 1;

		return unit->index;
	}

	return 0;
}

static bool IsPlayerDoneForTurn(void)
{
	if (gRealtimeBattleState.endRequested)
		return true;

	if (gPlaySt.config.disableAutoEndTurns)
		return false;

	/* Counting the able units is slow for every idle frame; a few frames late is fine */
	if (gRealtimeBattleState.loopCount & 7)
		return false;

	return GetPhaseAbleUnitCount(FACTION_BLUE) == 0;
}

static void StartRealtimeTurnClock(void)
{
	gRealtimeBattleState.valid = true;
	gRealtimeBattleState.endRequested = false;
	gRealtimeBattleState.aiUnit = 0;
	gRealtimeBattleState.nextSlot = 0;
	gRealtimeBattleState.turn = gPlaySt.chapterTurnNumber;
	gRealtimeBattleState.turnTimer = RealtimeBattle_GetTurnFrames();
	gRealtimeBattleState.enemyTimer = RealtimeBattle_GetIntervalFrames();
}

/*
 * StartPlayerPhaseSideWindows loads its proc script from this literal, which
 * VeslyAchievements repoints to its own side window maker.
 */
#define pr_SideWindowMakerScript (*(const struct ProcCmd *const *)0x0808D14C)

static void EndRealtimeSideWindows(void)
{
	/* The idle loop may have just been entered: stop the maker before it spawns windows */
	Proc_EndEach(pr_SideWindowMakerScript);
	EndPlayerPhaseSideWindows();
}

static void EndRealtimeTurn(ProcPtr playerPhaseProc)
{
	gRealtimeBattleState.valid = false;
	gRealtimeBattleState.endRequested = false;

	EndRealtimeSideWindows();
	Proc_Goto(playerPhaseProc, 3);
}

/* Returns true when the unit acts and the player phase now waits on it */
static bool StartRealtimeAiAction(ProcPtr playerPhaseProc, int unitId)
{
	struct RealtimeAiProc *proc;

	gRealtimeBattleState.aiUnit = unitId;
	gRealtimeBattleState.cursorX = gBmSt.playerCursor.x;
	gRealtimeBattleState.cursorY = gBmSt.playerCursor.y;

	proc = Proc_StartBlocking(ProcScr_RealtimeAiUnit, playerPhaseProc);
	proc->unitId = unitId;
	proc->started = false;

	/* The decision runs right away so units that stay put never touch the screen */
	RealtimeAi_Decide(proc);

	if (proc->proc_script != NULL && proc->proc_lockCnt != 0) {
		/* CpPerform is running: back to the top of the idle loop once it is done */
		gRealtimeBattleState.enemyTimer = RealtimeBattle_GetIntervalFrames();
		EndRealtimeSideWindows();
		Proc_Goto(playerPhaseProc, 9);
		return true;
	}

	/* Nothing to show, try the next unit on the next frame */
	if (proc->proc_script != NULL)
		Proc_End(proc);

	/* CpDecide refreshed the maps from the AI faction's point of view */
	RefreshEntityBmMaps();
	RenderBmMap();
	RefreshUnitSprites();

	return false;
}

bool RealtimeBattle_OnPlayerPhaseIdle(ProcPtr playerPhaseProc)
{
	int unitId;
	bool clockStalled;

	if (!IsRealtimeBattleActive())
		return false;

	/* The clock bumps loopCount every frame, so an unchanged count means it is not running */
	clockStalled = gRealtimeBattleState.loopCount == gRealtimeBattleState.idleLoop;

	/* Coming back to the cursor (after an action, a menu, ...): give the player a beat */
	if ((u8)(gRealtimeBattleState.loopCount - gRealtimeBattleState.idleLoop) > 1
		&& gRealtimeBattleState.enemyTimer < REALTIME_RESUME_GRACE_FRAMES)
		gRealtimeBattleState.enemyTimer = REALTIME_RESUME_GRACE_FRAMES;

	gRealtimeBattleState.idleLoop = gRealtimeBattleState.loopCount;

	if (!gRealtimeBattleState.valid || gRealtimeBattleState.turn != gPlaySt.chapterTurnNumber)
		StartRealtimeTurnClock();

	if (clockStalled && !Proc_Find(ProcScr_RealtimeClock))
		Proc_Start(ProcScr_RealtimeClock, playerPhaseProc);

	/* Most idle frames have nothing due: skip the proc scans below */
	if (gRealtimeBattleState.enemyTimer != 0 && gRealtimeBattleState.turnTimer != 0 && !IsPlayerDoneForTurn())
		return false;

	if (DoesBMXFADEExist() || Proc_Find(ProcScr_CamMove))
		return false;

	if (gRealtimeBattleState.enemyTimer == 0) {
		unitId = FindReadyRealtimeAiUnit(true);

		if (unitId != 0 && StartRealtimeAiAction(playerPhaseProc, unitId))
			return true;
	}

	if (gRealtimeBattleState.turnTimer == 0
		|| (IsPlayerDoneForTurn() && FindReadyRealtimeAiUnit(false) == 0)) {
		EndRealtimeTurn(playerPhaseProc);
		return true;
	}

	return false;
}

void RealtimeBattle_RequestEndTurn(void)
{
	gRealtimeBattleState.endRequested = true;
}

static void RealtimeClock_OnLoop(struct RealtimeClockProc *proc)
{
	struct Proc *parent = proc->proc_parent;
	bool onCursor;

	if (!IsRealtimeBattleActive()) {
		Proc_End(proc);
		return;
	}

	/* Children run after their parent, so MainIdle has already had its turn this loop */
	onCursor = gRealtimeBattleState.idleLoop == gRealtimeBattleState.loopCount;
	gRealtimeBattleState.loopCount++;

	if (!gRealtimeBattleState.valid || parent->proc_lockCnt != 0)
		return;

	/*
	 * Time only passes while the player is on the free cursor or picking a
	 * destination. Menus, battles, events and AI actions all stop the clock.
	 */
	if (!onCursor && ((u32)parent->proc_idleCb | 1) != ((u32)PlayerPhase_RangeDisplayIdle | 1))
		return;

	if (EventEngineExists())
		return;

	if (gRealtimeBattleState.turnTimer != 0)
		gRealtimeBattleState.turnTimer--;

	if (gRealtimeBattleState.enemyTimer != 0)
		gRealtimeBattleState.enemyTimer--;
}

static void RealtimeAi_Decide(struct RealtimeAiProc *proc)
{
	int i;
	struct Unit *unit = GetUnit(proc->unitId);

	proc->savedFaction = gPlaySt.faction;
	proc->savedActive = gActiveUnit;
	proc->savedActiveId = gActiveUnitId;
	proc->started = true;

	/* AI scripts, danger maps and fog checks all read the phase faction */
	gPlaySt.faction = UNIT_FACTION(unit);

	/* AiPhaseInit */
	gAiState.flags = AI_FLAG_0;
	gAiState.unk7E = -1;
	gAiState.orderState = 0;

	for (i = 0; i < 8; i++)
		gAiState.cmd_result[i] = 0;

	gAiState.specialItemFlags = gAiItemConfigTable[gPlaySt.chapterIndex];
	gAiState.unk84 = 0;

	UpdateAllPhaseHealingAIStatus();
	SetupUnitInventoryAIFlags();

	/* CpOrderFunc_BeginDecide, with a single entry list */
	gAiState.units[0] = proc->unitId;
	gAiState.units[1] = 0;
	gAiState.unitIt = gAiState.units;

	AiDecideMainFunc = AiDecideMain;

	CpDecide_Main(proc);
}

static void RealtimeAi_RestoreGlobals(struct RealtimeAiProc *proc)
{
	gRealtimeBattleState.aiUnit = 0;

	if (!proc->started)
		return;

	proc->started = false;

	gPlaySt.faction = proc->savedFaction;
	gActiveUnit = proc->savedActive;
	gActiveUnitId = proc->savedActiveId;

	/* AiPhaseCleanup */
	gAiState.flags = AI_FLAGS_NONE;
}

static void RealtimeAi_RefreshMap(struct RealtimeAiProc *proc)
{
	RefreshEntityBmMaps();
	RenderBmMap();
	RefreshUnitSprites();

	SetCursorMapPosition(gRealtimeBattleState.cursorX, gRealtimeBattleState.cursorY);
}

static void RealtimeAi_ReturnCamera(struct RealtimeAiProc *proc)
{
	EnsureCameraOntoPosition(proc, gRealtimeBattleState.cursorX, gRealtimeBattleState.cursorY);
}
