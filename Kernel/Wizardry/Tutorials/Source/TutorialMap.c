#include "common-chax.h"
#include "kernel-lib.h"
#include "save-data.h"
#include "savemenu.h"
#include "gamecontrol.h"
#include "soundwrapper.h"
#include "bmunit.h"
#include "bmmenu.h"
#include "bmsave.h"
#include "tutorials.h"
#include "jester_headers/maps.h"
#include "jester_headers/custom-arrays.h"

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

/* A on a tutorial (TutorialsScreen.c): its map, once the screen has faded out. menu: the save menu. */
void TutorialMap_Launch(struct SaveMenuProc *menu, int index)
{
	TutorialMap_NewGame();
	gPlaySt.chapterStateBits |= PLAY_FLAG_TUTORIAL;
	gPlaySt.chapterIndex = gTutorials[index].chapter;
	gPlaySt.save_menu_type = 2;

	TUTMAP_INDEX = index;
	TUTMAP_RETURN = 0;
	SetNextGameActionId(1);

	/* The save menu's last step, PostSaveMenuHandler: Extras' screens already ended these two */
	menu->approc = NULL;
	menu->savedraw = NULL;
	menu->main_sel_bitfile = 0x40;
	Proc_Goto(menu, PL_SAVEMENU_EXIT);
}

/* The game control's label 9: in tutorial mode, back to Tutorials (vanilla: the title, the ending or the next chapter) */
void TutorialMap_PostChapterSwitch(struct GameCtrlProc *proc)
{
	if (!TutorialMap_Active()) {
		GameControl_PostChapterSwitch(proc);
		return;
	}

	EndAllMus();
	gPlaySt.chapterStateBits &= ~PLAY_FLAG_TUTORIAL;
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
		TUTMAP_RETURN = 0;
		proc->extra_select = SaveMenuGetBitfileByMask(proc->extra_options, EXTRA_MENU_OPTION_TUTORIALS);
		Proc_Goto(proc, PL_SAVEMENU_EXEC_EXTRA_MISC_OPTION);
	}
}

/* ---- The map's events (CustomCampaign/Chapters/TutorialMap) ---- */

/* ASMC, the map's opening: the tutorial's units and set-up; its explanation's message in slot 2 */
void TutorialMap_Setup(void)
{
	const struct Tutorial *tut = TutorialMap_Current();
	const struct UnitDefinition *def;

	for (def = tut->units; def && def->charIndex; def++)
		LoadUnit(def);

	RefreshEntityBmMaps();
	if (tut->setup)
		tut->setup();

	RefreshEntityBmMaps();
	RefreshUnitSprites();
	gEventSlots[2] = tut->intro;
}

/* ASMC, after the explanation: anything shown once the map is up (the teleport's effect), with the event as parent */
void TutorialMap_AfterIntro(ProcPtr proc)
{
	const struct Tutorial *tut = TutorialMap_Current();

	if (tut->after)
		tut->after(proc);
}

/* ASMC, a district's opening (TutorialMap_DistrictIntro): slot C is 1 in tutorial mode, with the explanation in slot 2 */
void TutorialMap_CheckIntro(void)
{
	gEventSlots[0xC] = 0;
	if (!TutorialMap_Active())
		return;

	if (TutorialMap_Current()->setup)
		TutorialMap_Current()->setup();

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
