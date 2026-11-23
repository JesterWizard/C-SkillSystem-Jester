#include "common-chax.h"
#include "kernel-lib.h"
#include "vanilla.h"
#include "save-data.h"
#include "savemenu.h"
#include "gamecontrol.h"
#include "soundwrapper.h"
#include "bmunit.h"
#include "bmmenu.h"
#include "mu.h"
#include "muctrl.h"
#include "bmio.h"
#include "event.h"
#include "eventinfo.h"
#include "bmsave.h"
#include "prepscreen.h"
#include "worldmap.h"
#include "tutorials.h"
#include "bmphase.h"
#include "phasechangefx.h"
#include "jester_headers/maps.h"
#include "jester_headers/custom-arrays.h"
#include "jester_headers/custom-functions.h"

/*
 * Extras > Tutorials' maps (Documentation/Features/Tutorials.md, "The maps"): A on a tutorial starts its map, a game
 * of its own that never touches the save files, and the end of the map (won, lost or quit) comes back to the list.
 *
 * Starting (TutorialMap_Launch, from the Tutorials screen): a new game's state in RAM only (as WriteNewGameSave sets
 * it up, without the save), vanilla's tutorial flag (PLAY_FLAG_TUTORIAL: it stops suspend saves and greys out
 * Suspend), the tutorial's chapter, and save_menu_type 2, so the game control goes straight to the map
 * (GameCtrl_CheckNewGameAndBranch). The save menu then ends through its last step (PostSaveMenuHandler), which sets no
 * action for main_sel_bitfile 0x40, so the action set here (1) stands.
 *
 * Coming back: every end of a map reaches the game control's label 9 (GameControl_PostChapterSwitch, whose pointer is
 * replaced by TutorialMap_PostChapterSwitch). In tutorial mode it goes to the save menu (label 5) instead, which
 * TutorialMap_LoadExtraMenuGraphics sends straight to the Tutorials screen.
 */

#define GAMECTRL_LABEL_SAVEMENU 5
#define SONG_TITLE              0x43    /* the title screen's (StartTitleScreen_WithMusic) */

/*
 * Two of gPlaySt's unused bytes (config2.unused_22: 0 in every save, and written only here). gPlaySt stays as the map
 * left it until a save is loaded or a new game starts.
 */
#define TUTMAP_RETURN   (gPlaySt.config2.unused_22[0])  /* 1: the save menu opens on Tutorials */
#define TUTMAP_INDEX    (gPlaySt.config2.unused_22[1])  /* the tutorial played */
/* [2] and [5] are TutorialGuides.c's. The keys a guided step ignored, while SELECT's prompt asks (it needs A and the D-pad) */
#define TUTMAP_KEYS_LO  (gPlaySt.config2.unused_22[3])
#define TUTMAP_KEYS_HI  (gPlaySt.config2.unused_22[4])

extern void GameControl_PostChapterSwitch(struct GameCtrlProc *proc);
extern void SaveMenu_LoadExtraMenuGraphics(struct SaveMenuProc *proc);
extern void NewGameSaveGenericHook(void);

int TutorialMap_Played(void)
{
	return TUTMAP_INDEX;
}

bool TutorialMap_Active(void)
{
	return (gPlaySt.chapterStateBits & PLAY_FLAG_TUTORIAL) != 0;
}

static const struct Tutorial *TutorialMap_Current(void)
{
	return &gTutorials[TUTMAP_INDEX];
}

/* A new game in RAM: WriteNewGameSave's set-up, less the writes to the save memory */
static void TutorialMap_NewGame(void)
{
	const new_save_hook *it;
	u8 *it2;

	InitPlayConfig(0, 0);
	gPlaySt.unk_2C_1 = 0;
	gPlaySt.unk_2C_04 = 0;
	CpuFill16(0, &gPlaySt.unk_30, sizeof(gPlaySt.unk_30));
	gPlaySt.unk_2C_0D = 0;
	gPlaySt.chapterModeIndex = CHAPTER_MODE_EIRIKA;
	gPlaySt.unk_2B_00 = 1;
	gPlaySt.playerName[0] = '\0';
	for (it2 = (u8 *)&gPlaySt.config2; it2 < (u8 *)&gPlaySt.config2 + sizeof(gPlaySt.config2); it2++)
		*it2 = 0;

	/* NewGameSaveGenericHook's RAM parts (it also clears the suspend save and the world map in the save memory) */
	SetGameTime(0);
	InitUnits();
	ClearSupplyItems();
	ResetPermanentFlags();
	ResetChapterFlags();
	CpuFill16(0, gDeadUnits, sizeof(gDeadUnits));

	for (it = gpNewSaveHooks; *it != NULL; it++)
		if (*it != NewGameSaveGenericHook)
			(*it)();
}

/*
 * The message decoder (GetStringFromIndexInBuffer, AntiHuffman.c), on a tutorial's map: the chapter title card shows the
 * title of the chapter row its chapTitleId names (the chapter-name routine, Chname), which becomes the tutorial's own
 * title. TUTORIAL_MAP names itself (MSG_TUTMAP_TITLE); Port Kiris names Chapter 9's row.
 */
int TutorialMap_TitleMessage(int msg)
{
	const struct ROMChapterData *chapter = GetROMChapterStruct(gPlaySt.chapterIndex);

	if (msg == GetROMChapterStruct(chapter->chapTitleId)->chapTitleTextId)
		return gTutorials[TUTMAP_INDEX].title;

	return msg;
}

/* A on a tutorial (TutorialsScreen.c): its map, once the screen has faded out. menu: the save menu. */
void TutorialMap_Launch(struct SaveMenuProc *menu, int index)
{
	TutorialMap_NewGame();
	DisableFreeMovementASMC();  /* on only where a tutorial turns it on (Free Movement, the districts) */
	gPlaySt.chapterStateBits |= PLAY_FLAG_TUTORIAL;
	gPlaySt.chapterIndex = gTutorials[index].chapter;
	gPlaySt.save_menu_type = 2;
	TutGuide_Reset();

	TUTMAP_INDEX = index;
	TUTMAP_RETURN = 0;
	sActiveMsg = -1;    /* GetStringFromIndex's last message: its title may be the card's, decoded before */
	SetNextGameActionId(1);

	/* The save menu's last step, PostSaveMenuHandler: Extras' screens already ended these two */
	menu->approc = NULL;
	menu->savedraw = NULL;
	menu->main_sel_bitfile = 0x40;
	Proc_Goto(menu, PL_SAVEMENU_EXIT);
}

/*
 * Every colour black and no layer shown: from the end of a tutorial's map to the Tutorials screen, nothing shows (the
 * map for a frame as the map ended, the save menu's main menu on the way)
 */
static void TutorialMap_Blackout(void)
{
	CpuFill16(0, gPaletteBuffer, 0x400);
	EnablePaletteSync();
	SetDispEnable(0, 0, 0, 0, 0);
}

/*
 * Back from a tutorial's map, until the Tutorials screen starts: no layer shown. The save menu on the way shows its
 * layers (its set-up, then its fade out's EnableAllGfx) and its slot sprites put their palettes back every frame
 * (SaveDraw_UpdateSlotPalette), so the main menu's frame showed on the black screen for a few frames. In the vblank
 * tree, which OnVBlank runs right before it copies the display settings (FlushLCDControl): after everything the frame
 * wrote. In a main-loop tree, the fade out started in the same tree ran its EnableAllGfx after it, and the frame showed.
 */
static void TutorialMap_HideUntilTutorials(ProcPtr proc)
{
	if (TutorialsScreen_Exists()) {
		Proc_Break(proc);
		return;
	}
	SetDispEnable(0, 0, 0, 0, 0);
}

static const struct ProcCmd ProcScr_TutorialMapHide[] = {
	PROC_NAME("TutorialMapHide"),
	PROC_REPEAT(TutorialMap_HideUntilTutorials),
	PROC_END,
};

/* The game control's label 9: in tutorial mode, back to Tutorials (vanilla: the title, the ending or the next chapter) */
void TutorialMap_PostChapterSwitch(struct GameCtrlProc *proc)
{
	if (!TutorialMap_Active()) {
		GameControl_PostChapterSwitch(proc);
		return;
	}

	EndAllMus();
	DisableFreeMovementASMC();
	TutGuide_Reset();
	/* Black until the Tutorials screen fades in: the save menu starts on the way (TutorialMap_LoadExtraMenuGraphics) */
	TutorialMap_Blackout();
	Proc_Start(ProcScr_TutorialMapHide, PROC_TREE_VSYNC);
	/*
	 * The leaving event (SELECT's Yes, a lesson's end) locked the game as a gameplay event does, and was deleted with
	 * the map before it ended (DeleteEventEngines): its lock stayed, and the next tutorial's map never ran (black)
	 */
	gBmSt.lock = 0;
	gPlaySt.chapterStateBits &= ~PLAY_FLAG_TUTORIAL;
	sActiveMsg = -1;    /* it may hold a title message decoded as the tutorial's */
	TUTMAP_RETURN = 1;
	StartBgmExt(SONG_TITLE, 0, NULL);
	Proc_Goto(proc, GAMECTRL_LABEL_SAVEMENU);
}

/* The save menu's set-up: back from a tutorial's map, it opens on the Tutorials screen (as Extras > Tutorials does) */
void TutorialMap_LoadExtraMenuGraphics(struct SaveMenuProc *proc)
{
	bool back = TUTMAP_RETURN;

	if (back) {
		proc->main_sel_bitfile = MAIN_MENU_OPTION_EXTRAS;
		proc->extra_sel_bitfile = EXTRA_MENU_OPTION_TUTORIALS;
	}

	SaveMenu_LoadExtraMenuGraphics(proc);

	if (back) {
		/*
		 * Straight to Tutorials: the menu just drawn stays black through the save menu's fade out (it shows every
		 * layer, EnableAllGfx, and fades the palettes it finds); the Tutorials screen loads its own and fades in
		 */
		TutorialMap_Blackout();
		if (!Proc_Find(ProcScr_TutorialMapHide))
			Proc_Start(ProcScr_TutorialMapHide, PROC_TREE_VSYNC);
		TUTMAP_RETURN = 0;
		proc->extra_select = SaveMenuGetBitfileByMask(proc->extra_options, EXTRA_MENU_OPTION_TUTORIALS);
		Proc_Goto(proc, PL_SAVEMENU_EXEC_EXTRA_MISC_OPTION);
	}
}

/* ---- SELECT: leave the tutorial ---- */

extern const EventScr EventScr_TutorialMap_Quit[];
extern struct ProcCmd gProcScr_PlayerPhase[];
extern const struct ProcCmd FreeMovementControlProc[];
extern void PlayerPhase_MainIdle(ProcPtr proc);
void EndPlayerPhaseSideWindows(void);

/* The player phase, if it waits in its idle loop, not blocked by a child (a menu, a battle) */
static ProcPtr TutorialMap_IdleProc(const struct ProcCmd *script, void *idle)
{
	struct Proc *proc = Proc_Find(script);

	if (proc && proc->proc_idleCb == (ProcFunc)idle && proc->proc_lockCnt == 0)
		return proc;
	return NULL;
}

/* The player phase (or free movement) waiting for input, with no event running; NULL if not */
ProcPtr TutorialMap_IdlePhase(void)
{
	ProcPtr idle;

	if (EventEngineExists())
		return NULL;

	/* Free movement steps through its labels while idle: there, idle is not blocked and not walking */
	idle = Proc_Find(FreeMovementControlProc);
	if (idle && (((struct Proc *)idle)->proc_lockCnt != 0 || MuCtrExists()))
		return NULL;
	if (!idle)
		idle = TutorialMap_IdleProc(gProcScr_PlayerPhase, PlayerPhase_MainIdle);
	return idle;
}

static void TutorialMap_QuitWatch(ProcPtr proc)
{
	ProcPtr idle;

	if (!TutorialMap_Active()) {
		Proc_Break(proc);
		return;
	}

	if (!(gKeyStatusPtr->newKeys & SELECT_BUTTON))
		return;

	idle = TutorialMap_IdlePhase();
	if (!idle)
		return;

	/* "Leave the tutorial?" Yes/No, with the phase waiting for it; a guided step's ignored keys come back on No */
	TUTMAP_KEYS_LO = GetKeyStatus_IgnoreMask();
	TUTMAP_KEYS_HI = GetKeyStatus_IgnoreMask() >> 8;
	SetKeyStatus_IgnoreMask(0);
	KernelCallEvent(EventScr_TutorialMap_Quit, EV_EXEC_GAMEPLAY, idle);
}

/* ASMC, SELECT's prompt, No: back to the guided step, with the keys it ignored */
void TutorialMap_QuitCancelled(void)
{
	SetKeyStatus_IgnoreMask(TUTMAP_KEYS_LO | (TUTMAP_KEYS_HI << 8));
}

static const struct ProcCmd ProcScr_TutorialMapQuitWatch[] = {
	PROC_NAME("TutorialMapQuit"),
	PROC_REPEAT(TutorialMap_QuitWatch),
	PROC_END,
};

static void TutorialMap_StartQuitWatch(void)
{
	if (!Proc_Find(ProcScr_TutorialMapQuitWatch))
		Proc_Start(ProcScr_TutorialMapQuitWatch, PROC_TREE_3);
}

/* ---- The map's events (CustomCampaign/Chapters/TutorialMap) ---- */

/*
 * ASMC, the map's opening: the tutorial's units and set-up; its explanation's message in slot 2; its guided steps
 * (TutorialGuides.c) from the player phase's start
 */
void TutorialMap_Setup(void)
{
	const struct Tutorial *tut = TutorialMap_Current();
	const struct UnitDefinition *def;

	for (def = tut->units; def && def->charIndex; def++)
		LoadUnit(def);

	RefreshEntityBmMaps();
	SetFlag(TUTMAP_FLAG_VILLAGE);   /* Free Movement's village: its set-up clears it (TutorialsData.c) */
	if (tut->setup)
		tut->setup();

	RefreshEntityBmMaps();
	RefreshUnitSprites();
	gEventSlots[2] = tut->intro;
	TutorialMap_StartQuitWatch();

	TutGuide_Reset();
	if (tut->guide) {
		EnqueueTutEvent((uintptr_t)tut->guide, TUTORIAL_EVT_TYPE_PLAYERPHASE);
		SetFlag(TUTMAP_FLAG_NO_WIN);    /* the lesson ends the map, not the last enemy (events.c) */
	}
}

/* ASMC, after the explanation: anything shown once the map is up (the teleport's effect), with the event as parent */
void TutorialMap_AfterIntro(ProcPtr proc)
{
	const struct Tutorial *tut = TutorialMap_Current();

	if (tut->after)
		tut->after(proc);
}

/*
 * ASMC, a district's opening, once its people are on the map and before they start walking: in tutorial mode, the
 * tutorial's set-up (only its spot or people stay, TutorialsData.c), and SELECT to leave
 */
void TutorialMap_TownSetup(void)
{
	if (!TutorialMap_Active())
		return;

	if (TutorialMap_Current()->setup)
		TutorialMap_Current()->setup();

	TutorialMap_StartQuitWatch();
}

/*
 * ASMC, the map's win (no enemy left): slot C is 1 if the tutorial had enemies to defeat. A tutorial with none (the
 * stat screen's, Prestige...) has nothing to win: the check runs as soon as its opening ends, and it goes on. One with
 * guided steps (all of the map's) ends with its lesson (TutorialGuides.c), not with its last enemy.
 */
void TutorialMap_CheckWin(void)
{
	const struct Tutorial *tut = TutorialMap_Current();
	const struct UnitDefinition *def;

	gEventSlots[0xC] = 0;
	if (tut->guide)
		return;

	gEventSlots[0xC] = tut->after != NULL;
	for (def = tut->units; def && def->charIndex; def++)
		if (def->allegiance == FACTION_ID_RED)
			gEventSlots[0xC] = 1;
}

/*
 * The kill hooks (gKillUnitHooks): on the tutorials' map, the last player unit falling is a loss, as the lord's is
 * (EVFLAG_GAMEOVER, which the map's lose event waits for). Not every tutorial has Eirika.
 */
extern const EventScr EventScr_TutG_Promote[];

void TutorialMap_OnUnitKill(struct Unit *unit)
{
	int i;

	if (!TutorialMap_Active() || gPlaySt.chapterIndex != TUTORIAL_MAP || UNIT_FACTION(unit) != FACTION_BLUE)
		return;

	/* Enemy Promotion: Ross falling is the lesson (the fighter promotes); it ends at the next player phase */
	if (TutorialMap_Current()->guide == EventScr_TutG_Promote)
		return;

	for (i = FACTION_BLUE + 1; i < FACTION_GREEN; i++) {
		struct Unit *other = GetUnit(i);

		if (UNIT_IS_VALID(other) && other != unit && !(other->state & (US_DEAD | US_NOT_DEPLOYED)))
			return;
	}
	SetFlag(EVFLAG_GAMEOVER);
}

/* ASMC, a district's opening (TutorialMap_DistrictIntro): slot C is 1 in tutorial mode, with the explanation in slot 2 */
void TutorialMap_CheckIntro(void)
{
	gEventSlots[0xC] = 0;
	if (!TutorialMap_Active())
		return;

	gEventSlots[0xC] = 1;
	gEventSlots[2] = TutorialMap_Current()->intro;
}

/* The map menu's Retreat ("Retreat?", then back to Tutorials): on a tutorial's map as in a skirmish */
u8 TutorialMap_IsRetreatAvailable(const struct MenuItemDef *def, int number)
{
	if (TutorialMap_Active())
		return MENU_ENABLED;

	return MapMenu_IsRetreatCommandAvailable(def, number);
}

/*
 * FE8U = 0x080A3F08: a song heard for the first time unlocks it in the Sound Room, in the save memory (StartBgm calls
 * it). Vanilla's, except on a tutorial's map, which writes nothing to the save memory.
 */
LYN_REPLACE_CHECK(UnlockSoundRoomSong);
void UnlockSoundRoomSong(struct SoundRoomSaveData *buf, int song)
{
	struct SoundRoomSaveData tmp;
	u32 *bits;

	if (TutorialMap_Active())
		return;

	if (buf == NULL) {
		buf = &tmp;
		if (!LoadAndVerifySoundRoomData(buf))
			return;
	}

	bits = (u32 *)buf;
	if (bits[song >> 5] & (1 << (song & 31)))
		return;

	bits[song >> 5] |= 1 << (song & 31);

	/* Songs that unlock in pairs (vanilla's): 0x43 and 2, 0x54 and 0x30 */
	if (song == 0x43)
		bits[0] |= 1 << 2;
	else if (song == 0x02)
		bits[2] |= 1 << 3;

	if (song == 0x54)
		bits[1] |= 1 << 16;
	else if (song == 0x30)
		bits[2] |= 1 << 20;

	WriteSoundRoomSaveData(buf);
}

/*
 * FE8U = 0x08033258: the player's leader, whom the camera and the cursor go to at the player phase's start. Vanilla's,
 * except that a tutorial's map falls back on the first player unit when the leader is not on it, as vanilla's skirmishes
 * do: most tutorials have no Eirika, and the phase start read her position through a missing unit (the cursor went off
 * the map, and A there decoded a bad message until it overran gPlaySt).
 */
LYN_REPLACE_CHECK(GetPlayerLeaderUnitId);
int GetPlayerLeaderUnitId(void)
{
	int pid = CHARACTER_EIRIKA;
	struct Unit *unit;
	int i;

	if (gPlaySt.chapterModeIndex == CHAPTER_MODE_EPHRAIM)
		pid = CHARACTER_EPHRAIM;

	if (gPlaySt.chapterIndex == 5)
		return CHARACTER_EPHRAIM;

	if (GetBattleMapKind() == BATTLEMAP_KIND_STORY && !TutorialMap_Active())
		return pid;

	unit = GetUnitFromCharId(pid);
	if (unit && !(unit->state & US_DEAD))
		return pid;

	for (i = FACTION_BLUE + 1; i < FACTION_GREEN; i++) {
		unit = GetUnit(i);
		if (unit && unit->pCharacterData && !(unit->state & (US_DEAD | US_NOT_DEPLOYED | US_BIT16)))
			return unit->pCharacterData->number;
	}
	return pid;
}

/*
 * FE8U = 0x0801F2AC: the phase banner ("PLAYER PHASE"), which dims the map and lights it again, ends at once when the
 * phase has no unit to act. On a tutorial's map it never shows: the explanation goes straight on to the guided step.
 * The banner's end starts the map's music (StartMapSongBgm): done here instead.
 */
LYN_REPLACE_CHECK(PhaseIntro_EndIfNoUnits);
void PhaseIntro_EndIfNoUnits(ProcPtr proc)
{
	if (TutorialMap_Active()) {
		StartMapSongBgm();
		Proc_End(proc);
		return;
	}

	if (GetPhaseAbleUnitCount(gPlaySt.faction) == FACTION_BLUE)
		Proc_End(proc);
}

/*
 * Leaving from SELECT's prompt: its event blocks the player phase (or free movement), and the retreat's way out
 * (StartRetreatProcessing) waits on the phase, so the map is ended from a proc of its own instead, a frame later, as
 * vanilla's game over does once its screen is over (PostGameOverHandler). The game control's label 9 then goes back to
 * Tutorials (TutorialMap_PostChapterSwitch).
 */
static void TutorialMap_LeaveNow(ProcPtr proc)
{
	TutorialMap_Blackout();
	EndPlayerPhaseSideWindows();    /* the unit and terrain windows: the phase that put them up does not end them */
	SetNextGameActionId(0);
	EndBMapMain();
	DeleteEventEngines();
}

static const struct ProcCmd ProcScr_TutorialMapLeave[] = {
	PROC_NAME("TutorialMapLeave"),
	PROC_YIELD,
	PROC_CALL(TutorialMap_LeaveNow),
	PROC_END,
};

/* ASMC, SELECT's prompt, Yes (or a lesson's end, TutorialGuides.c) */
void TutorialMap_Leave(void)
{
	if (!TutorialMap_Leaving())
		Proc_Start(ProcScr_TutorialMapLeave, PROC_TREE_3);
}

/* The map is ending (TutorialMap_Leave's proc runs) */
bool TutorialMap_Leaving(void)
{
	return Proc_Find(ProcScr_TutorialMapLeave) != NULL;
}
