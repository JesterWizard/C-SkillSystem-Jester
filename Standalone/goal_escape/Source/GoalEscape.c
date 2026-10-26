#include "global.h"
#include "EAstdlib.h"
#include "bm.h"
#include "bmmap.h"
#include "bmsave.h"
#include "bmtrap.h"
#include "bmudisp.h"
#include "bmunit.h"
#include "chapterdata.h"
#include "constants/event-flags.h"
#include "event.h"
#include "eventinfo.h"
#include "fontgrp.h"
#include "functions.h"
#include "mu.h"
#include "player_interface.h"
#include "playerphase.h"
#include "proc.h"

u32 GetBattleMapKind(void);

/*
 * Escape goal (goal window type 6).
 *
 * A player unit that ends its move with Wait on one of the chapter's escape
 * tiles says its escape line and leaves the map. When no deployed player unit
 * is left, the win flag (EVFLAG_WIN) is set and vanilla PlayerPhase_FinishAction
 * logic runs the chapter's own ending scene, the same way Seize does.
 */

struct EscapeTileEntry {
	u8 chapterId;
	u8 x;
	u8 y;
	u8 pad;
};

/* Defined in Installer.event / EscapeTiles.event */
extern const struct EscapeTileEntry gGoalEscapeTiles[];
extern const u16 gGoalEscapeQuoteTextId;

#define ESCAPE_TILES_END 0xFF

/*
 * Marks units this patch took off the map. Vanilla never uses state bit 28;
 * suspend saves keep the whole state word, and the chapter-change cleanup
 * (ChapterChangeUnitCleanup) clears it.
 */
#define US_GOAL_ESCAPED (1 << 28)

static bool IsEscapeChapter(void)
{
	if (GetBattleMapKind() == BATTLEMAP_KIND_SKIRMISH)
		return false;

	return GetROMChapterStruct(gPlaySt.chapterIndex)->goalWindowDataType == GOAL_TYPE_ESCAPE;
}

static bool IsEscapeTile(int chapterIndex, int x, int y)
{
	const struct EscapeTileEntry *it;

	for (it = gGoalEscapeTiles; it->chapterId != ESCAPE_TILES_END; it++) {
		if (it->chapterId == chapterIndex && it->x == x && it->y == y)
			return true;
	}

	return false;
}

/* Player phase only: the active unit just used Wait on an escape tile */
static bool IsEscapeAction(void)
{
	struct Unit *unit = gActiveUnit;

	if (!IsEscapeChapter())
		return false;

	if (!UNIT_IS_VALID(unit) || UNIT_FACTION(unit) != FACTION_BLUE)
		return false;

	if (gActionData.unitActionType != UNIT_ACTION_WAIT && gActionData.unitActionType != UNIT_ACTION_FORCE_WAIT)
		return false;

	if (GetUnitCurrentHp(unit) <= 0)
		return false;

	return IsEscapeTile(gPlaySt.chapterIndex, unit->xPos, unit->yPos);
}

static int CountRemainingPlayerUnits(void)
{
	int i, count = 0;

	for (i = FACTION_BLUE + 1; i < FACTION_BLUE + 0x40; i++) {
		struct Unit *unit = GetUnit(i);

		if (!UNIT_IS_VALID(unit))
			continue;

		if (unit->state & (US_HIDDEN | US_NOT_DEPLOYED | US_DEAD))
			continue;

		count++;
	}

	return count;
}

static void RemoveEscapedUnit(struct Unit *unit)
{
	if (unit->state & US_RESCUING) {
		/* A carried unit leaves with its rescuer */
		struct Unit *rescuee = GetUnit(unit->rescue);

		if (UNIT_IS_VALID(rescuee)) {
			rescuee->state &= ~US_RESCUED;
			rescuee->state |= US_HIDDEN | US_NOT_DEPLOYED | US_UNSELECTABLE | US_GOAL_ESCAPED;
			rescuee->rescue = 0;
			rescuee->xPos = -1;
			rescuee->yPos = -1;
		}

		unit->state &= ~US_RESCUING;
		unit->rescue = 0;
	}

	HideUnitSprite(unit);

	unit->state |= US_HIDDEN | US_NOT_DEPLOYED | US_UNSELECTABLE | US_GOAL_ESCAPED;
	unit->xPos = -1;
	unit->yPos = -1;
}

/*
 * Before the chapter ends: escaped units count as deployed again, so the
 * chapter-change code does not mark them "not deployed last chapter" (which
 * would keep them hidden in the next chapter). They stay hidden until then.
 */
static void RestoreEscapedUnits(void)
{
	int i;

	for (i = FACTION_BLUE + 1; i < FACTION_BLUE + 0x40; i++) {
		struct Unit *unit = GetUnit(i);

		if (!UNIT_IS_VALID(unit) || !(unit->state & US_GOAL_ESCAPED))
			continue;

		unit->state &= ~(US_NOT_DEPLOYED | US_GOAL_ESCAPED);
	}
}

/* Escape line: TEXTSHOW(-1) reads the text ID from event slot 2 */
static void EscapeQuote_SetTextSlot(void)
{
	gEventSlots[EVT_SLOT_2] = gGoalEscapeQuoteTextId;
}

static const EventListScr EventScr_GoalEscapeQuote[] = {
	ASMC(EscapeQuote_SetTextSlot)
	TEXTSTART
	TEXTSHOW(-1)
	TEXTEND
	REMA
	NOFADE
	ENDA
};

static void EscapeQuote_Start(void)
{
	CallEvent((const u16 *)EventScr_GoalEscapeQuote, EV_EXEC_CUTSCENE);
}

static const struct ProcCmd ProcScr_GoalEscapeQuote[] = {
	PROC_NAME("GoalEscapeQuote"),
	PROC_YIELD,
	PROC_CALL(EscapeQuote_Start),
	PROC_YIELD,
	PROC_WHILE(EventEngineExists),
	PROC_END,
};

/*
 * Replaces the PROC_CALL_2(HandlePostActionTraps) of gProcScr_PlayerPhase.
 * Escaping units skip traps and say their line first; everyone else gets
 * vanilla HandlePostActionTraps.
 */
bool PostActionTraps_GoalEscape(ProcPtr proc)
{
	if (!IsEscapeAction())
		return HandlePostActionTraps(proc);

	if (gGoalEscapeQuoteTextId == 0)
		return true;

	Proc_StartBlocking(ProcScr_GoalEscapeQuote, proc);
	return false;
}

/*
 * Replaces the PROC_CALL(PlayerPhase_FinishAction) of gProcScr_PlayerPhase.
 * Same as vanilla, except the escaping unit is taken off the map instead of
 * being placed on its destination, and the chapter is won when nobody is left.
 */
void PlayerPhase_FinishAction_GoalEscape(ProcPtr proc)
{
	struct Unit *unit = gActiveUnit;

	if (!IsEscapeAction()) {
		/* The chapter can still be won another way (e.g. an event sets the win flag) */
		if (IsEscapeChapter() && ShouldCallEndEvent())
			RestoreEscapedUnits();

		PlayerPhase_FinishAction(proc);
		return;
	}

	PidStatsAddSquaresMoved(unit->pCharacterData->number, gActionData.moveCount);

	if (gPlaySt.chapterVisionRange != 0) {
		RenderBmMapOnBg2();
		RemoveEscapedUnit(unit);
		RefreshEntityBmMaps();
		RenderBmMap();
		NewBMXFADE(0);
	} else {
		RemoveEscapedUnit(unit);
		RefreshEntityBmMaps();
		RenderBmMap();
	}

	RefreshUnitSprites();

	SetCursorMapPosition(gActionData.xMove, gActionData.yMove);

	gPlaySt.xCursor = gBmSt.playerCursor.x;
	gPlaySt.yCursor = gBmSt.playerCursor.y;

	if (CountRemainingPlayerUnits() == 0)
		SetFlag(EVFLAG_WIN);

	if (ShouldCallEndEvent()) {
		RestoreEscapedUnits();

		EndAllMus();

		RefreshEntityBmMaps();
		RenderBmMap();
		RefreshUnitSprites();

		MaybeCallEndEvent_();

		Proc_Goto(proc, 8);
		return;
	}

	EndAllMus();
}

/*
 * Replaces the PROC_CALL(GoalDisplay_Init) of gProcScr_GoalDisplay.
 * Runs the original init (vanilla, or another patch's hook of it) and then
 * draws the escape objective for escape chapters.
 */
void GoalDisplay_Init_GoalEscape(struct PlayerInterfaceProc *proc)
{
	static const char sLine1[] = "Escape";
	static const char sLine2[] = "With all";

	GoalDisplay_Init(proc);

	if (!IsEscapeChapter())
		return;

	ClearText(&proc->texts[0]);
	ClearText(&proc->texts[1]);

	Text_InsertDrawString(&proc->texts[0], GetStringTextCenteredPos(64, sLine1), TEXT_COLOR_SYSTEM_WHITE, sLine1);
	Text_InsertDrawString(&proc->texts[1], GetStringTextCenteredPos(64, sLine2), TEXT_COLOR_SYSTEM_WHITE, sLine2);

	proc->unitClock = 1;
}
