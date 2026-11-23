#include "common-chax.h"
#include "kernel-lib.h"
#include "bmmenu.h"
#include "uimenu.h"
#include "event.h"
#include "eventinfo.h"
#include "eventcall.h"
#include "EAstdlib.h"
#include "statscreen.h"
#include "stat-screen.h"
#include "tutorials.h"
#include "player_interface.h"
#include "constants/texts.h"

/*
 * The tutorials' guided steps (Documentation/Features/Tutorials.md, "Guided steps"): on a tutorial's map, the player is
 * walked through the lesson's action as vanilla's Easy-mode tutorial does (prologue-tutorials.h in the decomp), and is
 * free once it is done. Vanilla's own engine runs them: a step is an event the player phase starts at a set point
 * (the player phase's start, a unit selected, a destination chosen, the unit moved, the forecast, the action done),
 * queued with EvtEnqueueConditionalTutCall, which finds the script in the chapter's tutorial list (TUTORIAL_MAP's
 * tutorialEvents: gTutorialGuideScripts, below). TutEventExecType0 and 1 (EAstdlib.h) check the unit and the tile
 * and ask again when they are wrong; IGNORE_KEYS keeps the player on the step (SELECT, to leave, is never ignored).
 *
 * Two parts are the kernel's: the unit menu cut down to the lesson's command (vanilla's DISABLEOPTIONS knows only its
 * own 15 commands, not this hack's B.Mag, W.Mag, Prestige...), and the stat screen steps (R on the unit opens its
 * stat screen on the lesson's page).
 *
 * Every map tutorial has an end: its lesson's action done (EventScr_TutG_Done after the action), its stat screen
 * closed, or a turn played (the enemy phase over: EventScr_TutG_Turn). The end goes straight back to Tutorials.
 */

/* gPlaySt.config2.unused_22 (TutorialMap.c has [0] and [1]): 0 in every save, written only on a tutorial's map */
#define TUTG_MENU       (gPlaySt.config2.unused_22[2])  /* the unit menu's one command (TUTG_CMD_*), 0: all */
#define TUTG_PAGE       (gPlaySt.config2.unused_22[5])  /* the stat screen step's page (shown pages: 0 is the first) */
#define TUTG_WALK_X     (gPlaySt.config2.unused_22[6])  /* Free Movement: where the walk started */
#define TUTG_WALK_Y     (gPlaySt.config2.unused_22[7])
#define TUTG_WALK_TILES 4                               /* the walk that ends Free Movement's lesson */

enum {
	TUTG_CMD_NONE,
	TUTG_CMD_ATTACK,
	TUTG_CMD_BMAG,
	TUTG_CMD_WMAG,
	TUTG_CMD_ITEM,
	TUTG_CMD_WAIT,
	TUTG_CMD_PRESTIGE,
};

u8 pr_GaidenBMagActionCommandEffect(struct MenuProc *menu, struct MenuItemProc *menuItem);
u8 pr_GaidenWMagActionCommandEffect(struct MenuProc *menu, struct MenuItemProc *menuItem);
u8 PrestigeCommandEffect(struct MenuProc *menu, struct MenuItemProc *menuItem);
extern const struct MenuItemDef gUnitActionMenuItemsRework[];
extern struct ProcCmd gProcScr_PlayerPhase[];
extern struct MenuItemOverride sMenuOverrides[];

/* Each command by its menu row's effect (Data/UnitMenu/Source/UnitMenu.c) */
static const MenuSelectFunc sTutGuideCommands[] = {
	[TUTG_CMD_ATTACK]   = UnitActionMenu_Attack,
	[TUTG_CMD_BMAG]     = pr_GaidenBMagActionCommandEffect,
	[TUTG_CMD_WMAG]     = pr_GaidenWMagActionCommandEffect,
	[TUTG_CMD_ITEM]     = ItemCommandEffect,
	[TUTG_CMD_WAIT]     = EffectWait,
	[TUTG_CMD_PRESTIGE] = PrestigeCommandEffect,
};

/* The step's command shows in the unit menu (a menu with no row at all would crash the game) */
static bool TutGuide_CommandShown(void)
{
	const struct MenuItemDef *def;

	for (def = gUnitActionMenuItemsRework; def->isAvailable; def++) {
		if (def->onSelected == sTutGuideCommands[TUTG_MENU] &&
		    def->isAvailable(def, def - gUnitActionMenuItemsRework) != MENU_NOTSHOWN)
			return true;
	}
	return false;
}

/*
 * FE8U = 0x0804F7AC: a menu row's forced availability (vanilla's DISABLEOPTIONS and DISABLEWEAPONS), or 0 for its own.
 * Vanilla's, and on a tutorial's map with a step's command set, every other row of the unit menu is hidden, as long as
 * that command shows (else the menu is whole). The unit menu is told by its rows: StartMenuCore asks for row `number`
 * of its list.
 */
LYN_REPLACE_CHECK(OverriddenMenuAvailability);
u8 OverriddenMenuAvailability(const struct MenuItemDef *def, int number)
{
	struct MenuItemOverride *it;
	int cmd = TUTG_MENU;

	for (it = sMenuOverrides; it->kind != MENU_OVERRIDE_NONE; it++) {
		if (it->kind == MENU_OVERRIDE_ISAVAILABLE && it->cmdid == def->overrideId)
			return ((MenuAvailabilityFunc)it->func)(def, number);
	}

	if (cmd != TUTG_CMD_NONE && TutorialMap_Active() && def - number == gUnitActionMenuItemsRework &&
	    def->onSelected != sTutGuideCommands[cmd] && TutGuide_CommandShown())
		return MENU_NOTSHOWN;

	return 0;
}


const u8 *GetPINumbers(void);   /* VeslyUI/C_Code.c: the player interface's digits, by UI style */

/*
 * ASMC, after every tutorial box: the sprite tiles it borrowed back. The box's window (vanilla's box dialogue) takes
 * OBJ tiles 0x2E0-0x2FF, where the unit window's HP digits are (0x2E0, PlayerPhaseInterfaceNumbersHook loads them as
 * the phase interface starts) and the move arrow (0x2F0, PathArrowDisp_Init loads it when a unit is selected). Left
 * as they were, the HP digits in the enemy phase and the enemy promotion's popup showed garbage, and after "Not
 * there" the arrow was a solid blue band.
 */
void TutGuide_RestoreBoxTiles(ProcPtr proc)
{
	Decompress(GetPINumbers(), (void *)(OBJ_VRAM0 + 0x5C00));
	Decompress(gUnknown_08A03054, (void *)(OBJ_VRAM0 + 0x5E00));   /* vanilla's gImg_PathArrow */
	ApplyPalette(gUnknown_08A0328C, 0x13);                         /* gPal_PathArrow */
}

/* ASMC: the unit menu shows only command slot 1's (TUTG_CMD_*) until the lesson is done */
void TutGuide_LockMenu(ProcPtr proc)
{
	TUTG_MENU = gEventSlots[EVT_SLOT_1];
}

/* The step's state cleared: every key and command back (the lesson done, the map left or started) */
void TutGuide_Reset(void)
{
	TUTG_MENU = TUTG_CMD_NONE;
	TUTG_PAGE = 0;
	TUTG_WALK_X = 0;
	TUTG_WALK_Y = 0;
	gPlaySt.config2.unused_22[3] = 0;   /* TutorialMap.c's keys kept while SELECT's prompt asks */
	gPlaySt.config2.unused_22[4] = 0;
	gPlaySt.tutorial_counter = 0;
	gPlaySt.tutorial_exec_type = 0;
	SetKeyStatus_IgnoreMask(0);
}

/* ASMC: every key and command back (the lesson done, or the map the player's) */
void TutGuide_Unlock(ProcPtr proc)
{
	TutGuide_Reset();
}

extern const EventScr EventScr_TutG_Leave[];

/*
 * After the lesson's action: leave once the player phase waits again (a unit left to act). With none left the turn ends
 * by itself, and the phase change leaves instead (EventScr_TutG_Done queues it). Not at once: the action's post-action
 * hooks (ProcScr_HookCommon) are still running, and ended from under them they drew the map over the Tutorials screen.
 */
static void TutGuide_LeaveWhenIdleLoop(ProcPtr proc)
{
	ProcPtr idle;

	if (!TutorialMap_Active() || TutorialMap_Leaving()) {
		Proc_Break(proc);
		return;
	}

	idle = TutorialMap_IdlePhase();
	if (!idle)
		return;

	gPlaySt.tutorial_counter = 0;
	gPlaySt.tutorial_exec_type = 0;
	KernelCallEvent(EventScr_TutG_Leave, EV_EXEC_GAMEPLAY, idle);
	Proc_Break(proc);
}

static const struct ProcCmd ProcScr_TutGuideLeaveWhenIdle[] = {
	PROC_NAME("TutGuideLeave"),
	PROC_YIELD,
	PROC_REPEAT(TutGuide_LeaveWhenIdleLoop),
	PROC_END,
};

/* ASMC */
void TutGuide_LeaveWhenIdle(ProcPtr proc)
{
	Proc_Start(ProcScr_TutGuideLeaveWhenIdle, PROC_TREE_3);
}

/* StartPlayerPhaseSideWindows loads the side windows' maker from this literal (VeslyAchievements repoints it) */
#define pr_SideWindowMakerScript (*(const struct ProcCmd *const *)0x0808D14C)

/*
 * The player phase's idle loop, not blocked, with no event running, ends its turn: the AI Rescue lesson starts with
 * the enemy phase (vanilla's End command, CommandEffectEndPlayerPhase)
 */
static void TutGuide_EndTurnLoop(ProcPtr proc)
{
	if (!TutorialMap_Active()) {
		Proc_Break(proc);
		return;
	}

	if (!TutorialMap_IdlePhase())
		return;

	/*
	 * The side windows' maker (VeslyAchievements' repoint, as RealtimeScheduler.c and TrapData.c end it) first: it
	 * spawned the unit and goal windows again, and they stayed up through the enemy phase, whose map battle box and
	 * enemy promotion popup reuse their sprite tiles (garbled windows, no popup)
	 */
	Proc_EndEach(pr_SideWindowMakerScript);
	EndPlayerPhaseSideWindows();
	Proc_EndEach(gProcScr_PlayerPhase);
	Proc_Break(proc);
}

static const struct ProcCmd ProcScr_TutGuideEndTurn[] = {
	PROC_NAME("TutGuideEndTurn"),
	PROC_YIELD,
	PROC_REPEAT(TutGuide_EndTurnLoop),
	PROC_END,
};

/* Free Movement: once the walking unit (the first player unit) is TUTG_WALK_TILES from its start, the lesson is over */
static void TutGuide_WalkLoop(ProcPtr proc)
{
	struct Unit *unit = GetUnit(FACTION_BLUE + 1);
	ProcPtr idle;

	if (!TutorialMap_Active() || TutorialMap_Leaving()) {
		Proc_Break(proc);
		return;
	}

	if (!UNIT_IS_VALID(unit) || RECT_DISTANCE(unit->xPos, unit->yPos, TUTG_WALK_X, TUTG_WALK_Y) < TUTG_WALK_TILES)
		return;

	idle = TutorialMap_IdlePhase();
	if (!idle)
		return;

	KernelCallEvent(EventScr_TutG_Leave, EV_EXEC_GAMEPLAY, idle);
	Proc_Break(proc);
}

static const struct ProcCmd ProcScr_TutGuideWalk[] = {
	PROC_NAME("TutGuideWalk"),
	PROC_REPEAT(TutGuide_WalkLoop),
	PROC_END,
};

/*
 * The walk starts here (free movement's A acts on what is next to the unit: no action ends this lesson). Started by
 * Free Movement's set-up: free movement takes the player phase's place, so its start (the guides' first point) never
 * comes.
 */
void TutGuide_WalkStep(ProcPtr proc)
{
	struct Unit *unit = GetUnit(FACTION_BLUE + 1);

	if (Proc_Find(ProcScr_TutGuideWalk))
		return;

	if (UNIT_IS_VALID(unit)) {
		TUTG_WALK_X = unit->xPos;
		TUTG_WALK_Y = unit->yPos;
	}
	Proc_Start(ProcScr_TutGuideWalk, PROC_TREE_3);
}

/* ASMC: the turn ends as soon as the player phase waits (after this event) */
void TutGuide_EndTurn(ProcPtr proc)
{
	Proc_Start(ProcScr_TutGuideEndTurn, PROC_TREE_3);
}

/* ---- Stat screen steps ---- */

/*
 * R opens the stat screen on its first page (StartStatScreen: vanilla keeps the last one in gPlaySt's two low state
 * bits, too few for this hack's pages). It draws it two frames later (gProcScr_StatScreen's PROC_SLEEP(2)): the page
 * set as soon as the screen starts is the one drawn. Then its keys are free (turning pages, B to close it).
 */
static void TutGuide_StatWatch(ProcPtr proc)
{
	if (!TutorialMap_Active()) {
		Proc_End(proc);
		return;
	}

	if (Proc_Find(gProcScr_StatScreen)) {
		gStatScreen.page = TUTG_PAGE;
		TutGuide_Reset();
		Proc_Break(proc);
	}
}

/* The stat screen closed and the player phase waiting again: the lesson is over */
static void TutGuide_StatClosed(ProcPtr proc)
{
	ProcPtr idle;

	if (!TutorialMap_Active()) {
		Proc_Break(proc);
		return;
	}

	if (Proc_Find(gProcScr_StatScreen))
		return;

	idle = TutorialMap_IdlePhase();
	if (!idle)
		return;

	KernelCallEvent(EventScr_TutG_Leave, EV_EXEC_GAMEPLAY, idle);
	Proc_Break(proc);
}

static const struct ProcCmd ProcScr_TutGuideStatWatch[] = {
	PROC_NAME("TutGuideStat"),
	PROC_REPEAT(TutGuide_StatWatch),
	PROC_REPEAT(TutGuide_StatClosed),
	PROC_END,
};

/*
 * ASMC: R opens the stat screen on the page whose draw-table index is in slot 1 (gStatScreen.page counts only the
 * pages shown: TranslateStatPageId)
 */
void TutGuide_StatStep(ProcPtr proc)
{
	int page;

	TUTG_PAGE = 0;
	for (page = 0; page < 10; page++) {
		if (TranslateStatPageId(page) == (int)gEventSlots[EVT_SLOT_1]) {
			TUTG_PAGE = page;
			break;
		}
	}

	if (!Proc_Find(ProcScr_TutGuideStatWatch))
		Proc_Start(ProcScr_TutGuideStatWatch, PROC_TREE_3);
}

/* ---- The steps ---- */

#define KEYS_ALL        (A_BUTTON | B_BUTTON | SELECT_BUTTON | START_BUTTON | DPAD_RIGHT | DPAD_LEFT | DPAD_UP | \
			 DPAD_DOWN | R_BUTTON | L_BUTTON)
/* Only A (and SELECT): the cursor stays on the unit to select */
#define KEYS_ONLY_A     (KEYS_ALL & ~(A_BUTTON | SELECT_BUTTON))
/* Only R (and SELECT): the stat screen */
#define KEYS_ONLY_R     (KEYS_ALL & ~(R_BUTTON | SELECT_BUTTON))
/* No going back: B would undo the move or close the menu */
#define KEYS_NO_BACK    (R_BUTTON | START_BUTTON | B_BUTTON)

#define TUTG_POS        (-1)            /* the box's place: the middle of the screen */

/*
 * A tutorial box (vanilla's in-map tutorials' text box). Not over the battle forecast: the box's font takes the tiles
 * of the forecast's numbers until it is drawn again.
 */
#define TUTG_TEXT(msg) \
	TUTORIALTEXTBOXSTART \
	SVAL(EVT_SLOT_B, TUTG_POS) \
	TEXTSHOW(msg) \
	TEXTEND \
	REMA \
	ASMC(TutGuide_RestoreBoxTiles)

/* The cursor flashing on a tile, with a message */
#define TUTG_POINT(x, y, msg) \
	SET_CURSOR(x, y) \
	CURSOR_FLASHING(x, y) \
	STAL(30) \
	TUTG_TEXT(msg) \
	CURE

extern const EventScr EventScr_TutG_Free[];
extern const EventScr EventScr_TutG_TurnEnd[];

/*
 * A unit's action: select it at (ux, uy) (only A works there), move it to (dx, dy) (its own tile: it stays), its menu
 * with only `cmd` (cmd_msg says what to do), then `next` at the point `type` (TUTORIAL_EVT_TYPE_*): the end of the
 * lesson after the action (EventScr_TutG_Done). The scripts: name (the player phase's start), _Select, _Dest, _Moved.
 */
#define TUTG_ACTION(name, pid, ux, uy, dx, dy, move_msg, cmd, cmd_msg, next, type) \
	extern const EventScr EventScr_TutG_##name##_Select[]; \
	extern const EventScr EventScr_TutG_##name##_Dest[]; \
	extern const EventScr EventScr_TutG_##name##_Moved[]; \
	const EventScr EventScr_TutG_##name[] = { \
		TUTG_POINT(ux, uy, MSG_TUTG_SELECT) \
		IGNORE_KEYS(KEYS_ONLY_A) \
		EvtEnqueueConditionalTutCall(EventScr_TutG_##name##_Select, TUTORIAL_EVT_TYPE_ONSELECT) \
		NOFADE \
		ENDA \
	}; \
	const EventScr EventScr_TutG_##name##_Select[] = { \
		NOFADE \
		IGNORE_KEYS(0) \
		TutEventExecType0(pid, dx, dy, move_msg, TUTG_POS, MSG_TUTG_WRONG_UNIT, TUTG_POS, \
				  EventScr_TutG_##name##_Dest, EventScr_TutG_##name##_Select) \
		ASMC(TutGuide_RestoreBoxTiles) \
		ENDA \
	}; \
	const EventScr EventScr_TutG_##name##_Dest[] = { \
		NOFADE \
		IGNORE_KEYS(0) \
		TutEventExecType1(dx, dy, MSG_TUTG_WRONG_TILE, TUTG_POS, \
				  EventScr_TutG_##name##_Moved, EventScr_TutG_##name##_Dest) \
		ASMC(TutGuide_RestoreBoxTiles) \
		SVAL(EVT_SLOT_1, cmd) \
		ASMC(TutGuide_LockMenu) \
		IGNORE_KEYS(KEYS_NO_BACK) \
		ENDA \
	}; \
	const EventScr EventScr_TutG_##name##_Moved[] = { \
		NOFADE \
		IGNORE_KEYS(0) \
		TUTG_TEXT(cmd_msg) \
		IGNORE_KEYS(KEYS_NO_BACK) \
		EvtEnqueueConditionalTutCall(next, type) \
		ENDA \
	};

#define TUTG_ACTION_LIST(name) \
	EventScr_TutG_##name, EventScr_TutG_##name##_Select, EventScr_TutG_##name##_Dest, EventScr_TutG_##name##_Moved,

/*
 * Select the unit at (x, y) (only A works there); then the map is free (the lesson is the selected unit's range) until
 * the unit's action ends it
 */
#define TUTG_SELECT(name, x, y) \
	const EventScr EventScr_TutG_##name[] = { \
		TUTG_POINT(x, y, MSG_TUTG_SELECT) \
		IGNORE_KEYS(KEYS_ONLY_A) \
		EvtEnqueueConditionalTutCall(EventScr_TutG_Free, TUTORIAL_EVT_TYPE_ONSELECT) \
		NOFADE \
		ENDA \
	};

/* R on the unit at (x, y) (only R works there): its stat screen on the page drawn by `page` (STATSCREEN_PAGE_*) */
#define TUTG_STAT(name, x, y, page) \
	const EventScr EventScr_TutG_##name[] = { \
		TUTG_POINT(x, y, MSG_TUTG_STAT) \
		SVAL(EVT_SLOT_1, page) \
		ASMC(TutGuide_StatStep) \
		IGNORE_KEYS(KEYS_ONLY_R) \
		NOFADE \
		ENDA \
	};

void TutorialMap_Leave(void);

/*
 * Back to Tutorials, as SELECT's Yes does (TutorialMap_Leave ends the map from a proc of its own; the event waits for
 * it). From a quiet point only: a player phase's start or idle loop, or a phase change.
 */
const EventScr EventScr_TutG_Leave[] = {
	ASMC(TutGuide_Unlock)
	FADI(16)
	ASMC(TutorialMap_Leave)
	LABEL(1)
	STAL(1)
	GOTO(1)
	ENDA
};

/* The lesson's action done: back to Tutorials as soon as the action is over (TutGuide_LeaveWhenIdle) */
const EventScr EventScr_TutG_Done[] = {
	NOFADE
	ASMC(TutGuide_Unlock)
	EvtEnqueueConditionalTutCall(EventScr_TutG_Leave, TUTORIAL_EVT_TYPE_PHASECHANGE)
	ASMC(TutGuide_LeaveWhenIdle)
	ENDA
};

/* Every key back while the player looks (the lesson goes on as the cursor moves); the unit's action ends it */
const EventScr EventScr_TutG_Free[] = {
	NOFADE
	ASMC(TutGuide_Unlock)
	EvtEnqueueConditionalTutCall(EventScr_TutG_Done, TUTORIAL_EVT_TYPE_POSTACTION)
	ENDA
};

/* A lesson to watch: it ends once a turn is played, at the next player phase's start */
const EventScr EventScr_TutG_Turn[] = {
	NOFADE
	EvtEnqueueConditionalTutCall(EventScr_TutG_TurnEnd, TUTORIAL_EVT_TYPE_PHASECHANGE)
	ENDA
};

/* The enemy phase starts (the phase change): the lesson ends at the next player phase's start */
const EventScr EventScr_TutG_TurnEnd[] = {
	NOFADE
	EvtEnqueueConditionalTutCall(EventScr_TutG_Leave, TUTORIAL_EVT_TYPE_PLAYERPHASE)
	ENDA
};

/* Free Movement's guide (it ends once the unit has walked a few tiles): its set-up starts the walk, TutGuide_WalkStep */
const EventScr EventScr_TutG_Walk[] = {
	NOFADE
	ASMC(TutGuide_WalkStep)
	ENDA
};

/* AI Rescue: the enemy phase plays first (the paladin rescues the soldier), then the lesson ends */
const EventScr EventScr_TutG_EnemyFirst[] = {
	NOFADE
	EvtEnqueueConditionalTutCall(EventScr_TutG_TurnEnd, TUTORIAL_EVT_TYPE_PHASECHANGE)
	ASMC(TutGuide_EndTurn)
	ENDA
};

/* The units' places: TutorialsData.c */
#define POSTACTION TUTORIAL_EVT_TYPE_POSTACTION

TUTG_ACTION(Arts, CHARACTER_EIRIKA, 4, 5, 8, 5, MSG_TUTG_MOVE, TUTG_CMD_ATTACK, MSG_TUTG_ARTS_ATTACK,
	    EventScr_TutG_Done, POSTACTION)
TUTG_ACTION(Cutin, CHARACTER_JOSHUA, 5, 5, 8, 5, MSG_TUTG_MOVE, TUTG_CMD_ATTACK, MSG_TUTG_CMD_ATTACK,
	    EventScr_TutG_Done, POSTACTION)
TUTG_STAT(Slots, 5, 5, STATSCREEN_PAGE_2)
TUTG_ACTION(HP, CHARACTER_GILLIAM, 5, 5, 8, 5, MSG_TUTG_MOVE, TUTG_CMD_ATTACK, MSG_TUTG_CMD_ATTACK,
	    EventScr_TutG_Done, POSTACTION)
TUTG_ACTION(Gaiden, CHARACTER_LUTE, 5, 5, 7, 5, MSG_TUTG_MOVE, TUTG_CMD_BMAG, MSG_TUTG_CMD_BMAG,
	    EventScr_TutG_Done, POSTACTION)
TUTG_STAT(MP, 5, 5, PAGE_GAIDEN_MAGIC)
TUTG_ACTION(Reward, CHARACTER_ARTUR, 5, 5, 7, 5, MSG_TUTG_MOVE, TUTG_CMD_BMAG, MSG_TUTG_CMD_BMAG,
	    EventScr_TutG_Done, POSTACTION)
TUTG_ACTION(Seal, CHARACTER_LUTE, 3, 6, 7, 6, MSG_TUTG_MOVE, TUTG_CMD_WAIT, MSG_TUTG_SEAL_WAIT,
	    EventScr_TutG_Done, POSTACTION)
TUTG_ACTION(Tonic, CHARACTER_EIRIKA, 4, 5, 4, 5, MSG_TUTG_STAY, TUTG_CMD_ITEM, MSG_TUTG_CMD_ITEM,
	    EventScr_TutG_Done, POSTACTION)
TUTG_ACTION(StaffExp, CHARACTER_NATASHA, 5, 5, 5, 5, MSG_TUTG_STAY, TUTG_CMD_WMAG, MSG_TUTG_CMD_WMAG,
	    EventScr_TutG_Done, POSTACTION)
/*
 * Enemy Promotion: Ross waits, and the turn ends (Eirika, out of the way, does not act); the fighter defeats him and
 * promotes in the enemy phase, and the lesson ends at the next player phase
 */
TUTG_ACTION(Promote, CHARACTER_ROSS, 7, 5, 7, 5, MSG_TUTG_STAY, TUTG_CMD_WAIT, MSG_TUTG_CMD_WAIT,
	    EventScr_TutG_EnemyFirst, POSTACTION)
TUTG_SELECT(Ghost, 4, 5)
TUTG_ACTION(Traps, CHARACTER_EIRIKA, 4, 5, 3, 6, MSG_TUTG_MOVE, TUTG_CMD_WAIT, MSG_TUTG_CMD_WAIT,
	    EventScr_TutG_Done, POSTACTION)
TUTG_SELECT(Threat, 4, 5)
TUTG_ACTION(Prestige, CHARACTER_FRANZ, 5, 5, 5, 5, MSG_TUTG_STAY, TUTG_CMD_PRESTIGE, MSG_TUTG_CMD_PRESTIGE,
	    EventScr_TutG_Done, POSTACTION)
TUTG_STAT(Capacity, 4, 5, STATSCREEN_PAGE_2)
TUTG_ACTION(Trainee, CHARACTER_ROSS, 7, 5, 7, 5, MSG_TUTG_STAY, TUTG_CMD_ATTACK, MSG_TUTG_CMD_ATTACK,
	    EventScr_TutG_Done, POSTACTION)

/*
 * TUTORIAL_MAP's tutorialEvents (CustomCampaign/Chapters/TutorialMap/events/events.c): every script a step queues,
 * which vanilla's EnqueueTutEvent looks up by address. Ends with NULL.
 */
const EventScr *const gTutorialGuideScripts[] = {
	EventScr_TutG_Leave,
	EventScr_TutG_Done,
	EventScr_TutG_Free,
	EventScr_TutG_Turn,
	EventScr_TutG_TurnEnd,
	EventScr_TutG_Walk,
	EventScr_TutG_EnemyFirst,
	TUTG_ACTION_LIST(Arts)
	TUTG_ACTION_LIST(Cutin)
	EventScr_TutG_Slots,
	TUTG_ACTION_LIST(HP)
	TUTG_ACTION_LIST(Gaiden)
	EventScr_TutG_MP,
	TUTG_ACTION_LIST(Reward)
	TUTG_ACTION_LIST(Seal)
	TUTG_ACTION_LIST(Tonic)
	TUTG_ACTION_LIST(StaffExp)
	TUTG_ACTION_LIST(Promote)
	EventScr_TutG_Ghost,
	TUTG_ACTION_LIST(Traps)
	EventScr_TutG_Threat,
	TUTG_ACTION_LIST(Prestige)
	EventScr_TutG_Capacity,
	TUTG_ACTION_LIST(Trainee)
	NULL,
};
