/*
 * Unlimited Prep Menu Options, standalone (FE8U clean ROM, no kernel).
 *
 * Port of Kernel/Wizardry/SkillSys/PrepSkill/Source/AtMenu.c. Only the vanilla
 * Preparations rows are kept, in vanilla order and with vanilla conditions:
 *
 *   key 0  Pick Units   (always)
 *   key 1  Items        (always)
 *   key 4  Support      (vanilla AtMenu_AddPrepScreenSupportMenuItem decides)
 *   key 7  Check Map    (grey when CanPrepScreenCheckMap() is false)
 *   key 2  Save         (grey when CanPrepScreenSave() is false)
 *   key 3  arena row    (link arena only, vanilla msg 0x75C)
 *
 * Extension rows (PrepMenuExt.h) follow, in slot order.
 *
 * Visible list: gPrepMenuVisibleTableSlots[i] holds the row key of visible row
 * i, with PMX_FLAG_GREY (0x80) set for a grey row. Keys are:
 *   0..7        vanilla rows (table above)
 *   0x20..0x23  Check Map submenu rows (View Map, Formation, Options, Save)
 *   0x40..0x47  extension rows, 0x40 + slot
 *
 * Row numbering follows vanilla: AtMenu_StartSubmenu sees proc->state = key + 1
 * for vanilla rows (Pick Units 1, Items 2, Save 3, Support 4) and 0x40 + slot for
 * extension rows.
 *
 * Test-only note: nothing in this file is a test helper.
 */

#include "global.h"
#include "proc.h"
#include "fontgrp.h"
#include "hardware.h"
#include "soundwrapper.h"
#include "sysutil.h"
#include "statscreen.h"
#include "helpbox.h"
#include "uiutils.h"
#include "bm.h"
#include "bmlib.h"
#include "bmudisp.h"
#include "sio.h"
#include "prepscreen.h"
#include "functions.h"
#include "constants/songs.h"
#include "PrepMenuExt.h"

#define PMX_LIST_VISIBLE 5 /* rows drawn at once; vanilla PREP_MENU_VISIBLE_COUNT */
#define PMX_ROW_CAP      16 /* gPrepMenuVisibleTableSlots size (RamAlloc.s) */
#define PMX_MAP_ROWS     4  /* Check Map submenu rows */

#define PMX_KEY_UNIT     0
#define PMX_KEY_ITEM     1
#define PMX_KEY_SAVE     2
#define PMX_KEY_ARENA    3
#define PMX_KEY_SUPPORT  4
#define PMX_KEY_CHECKMAP 7
#define PMX_KEY_MAP_BASE 0x20

#define PMX_KEY_MASK  0x7F
#define PMX_FLAG_GREY 0x80

/* RAM from Source/RamAlloc.s */
extern u8 gPrepMenuVisibleTableSlots[PMX_ROW_CAP];
extern u8 gPrepMenuTopVisibleIndex;
extern u8 gPrepMenuBuildingList;

/* Header placed by Installer.event at $1000004 (see PrepMenuExt.h) */
extern const struct PrepMenuExtRow *PrepMenuExtSlots[PREP_MENU_EXT_SLOT_COUNT];

/* Vanilla functions that have no prototype in FE-CLib */
int sub_8095024(void);

/* Hooks that are used before they are defined */
void PMX_InitPrepScreenMainMenu(struct ProcAtMenu *proc);
void PMX_SetPrepScreenMenuPosition(int x, int y);
void PMX_SetPrepScreenMenuSelectedItem(int index);
int PMX_GetActivePrepMenuItemIndex(void);

/* ------------------------------------------------------------------------ */
/* Row tables                                                                */
/* ------------------------------------------------------------------------ */

struct PMXMainRow {
	int key;
	int msg;                                 /* vanilla title text id */
	void (*effect)(struct ProcAtMenu *proc); /* A pressed */
};

struct PMXMapRow {
	int msg;     /* vanilla title text id */
	int msg_rtext; /* R-button help text id */
};

static void PMX_OnPickUnits(struct ProcAtMenu *proc);
static void PMX_OnItems(struct ProcAtMenu *proc);
static void PMX_OnSave(struct ProcAtMenu *proc);
static void PMX_OnSupport(struct ProcAtMenu *proc);
static void PMX_OnArenaRow(struct ProcAtMenu *proc);
void PMX_PrepScreenMenu_OnCheckMap(struct ProcAtMenu *proc);

static const struct PMXMainRow gPMXMainRows[] = {
	{ PMX_KEY_UNIT,     0x574, PMX_OnPickUnits },
	{ PMX_KEY_ITEM,     0x576, PMX_OnItems },
	{ PMX_KEY_SUPPORT,  0x577, PMX_OnSupport },
	{ PMX_KEY_CHECKMAP, 0x578, PMX_PrepScreenMenu_OnCheckMap },
	{ PMX_KEY_SAVE,     0x579, PMX_OnSave },
	{ PMX_KEY_ARENA,    0x75C, PMX_OnArenaRow },
};

/* Order matches the vanilla Check Map submenu (PrepScreenProc_StartMapMenu). */
static const struct PMXMapRow gPMXMapRows[PMX_MAP_ROWS] = {
	{ 0x590, 0x5BB }, /* View Map  */
	{ 0x591, 0x5BC }, /* Formation */
	{ 0x592, 0x5BD }, /* Options   */
	{ 0x579, 0x5BE }, /* Save      */
};

#define PMX_ARRAY_SIZE(a) ((int)(sizeof(a) / sizeof((a)[0])))
#define PMX_MAIN_ROW_COUNT PMX_ARRAY_SIZE(gPMXMainRows)

/* ------------------------------------------------------------------------ */
/* Key helpers                                                               */
/* ------------------------------------------------------------------------ */

static int PMX_IsExt(int key)
{
	return key >= PREP_MENU_EXT_KEY_BASE && key < PREP_MENU_EXT_KEY_BASE + PREP_MENU_EXT_SLOT_COUNT;
}

static int PMX_IsMap(int key)
{
	return key >= PMX_KEY_MAP_BASE && key < PMX_KEY_MAP_BASE + PMX_MAP_ROWS;
}

static const struct PrepMenuExtRow *PMX_ExtRow(int key)
{
	if (!PMX_IsExt(key))
		return NULL;

	return PrepMenuExtSlots[key - PREP_MENU_EXT_KEY_BASE];
}

static struct ProcPrepMenu *PMX_Menu(void)
{
	return Proc_Find(ProcScr_PrepMenu);
}

/* Visible list value (key | grey flag) at index, or -1 when there is none. */
static int PMX_ValueAt(const struct ProcPrepMenu *menu, int index)
{
	if (!menu || index < 0 || index >= menu->max_index || index >= PMX_ROW_CAP)
		return -1;

	return gPrepMenuVisibleTableSlots[index];
}

static const char *PMX_TitleOf(int key)
{
	const struct PrepMenuExtRow *row;
	int i;

	if (PMX_IsExt(key)) {
		row = PMX_ExtRow(key);
		return row ? row->title : NULL;
	}

	if (PMX_IsMap(key))
		return GetStringFromIndex(gPMXMapRows[key - PMX_KEY_MAP_BASE].msg);

	for (i = 0; i < PMX_MAIN_ROW_COUNT; i++) {
		if (gPMXMainRows[i].key == key)
			return GetStringFromIndex(gPMXMainRows[i].msg);
	}

	return NULL;
}

static int PMX_ColorOf(int value)
{
	int key = value & PMX_KEY_MASK;

	if (PMX_IsExt(key)) {
		const struct PrepMenuExtRow *row = PMX_ExtRow(key);

		if (row)
			return row->color;
	}

	return (value & PMX_FLAG_GREY) ? TEXT_COLOR_SYSTEM_GRAY : TEXT_COLOR_SYSTEM_WHITE;
}

static int PMX_RtextOf(int key)
{
	if (PMX_IsMap(key))
		return gPMXMapRows[key - PMX_KEY_MAP_BASE].msg_rtext;

	return 0; /* vanilla rows and extension rows have no R-button help */
}

/* Text of one visible row. Vanilla rows use message ids, extension rows use
 * the plain string from the add-on.
 */
static void PMX_PutRow(int textIndex, int x, int y, int value)
{
	const char *title = PMX_TitleOf(value & PMX_KEY_MASK);

	ClearText(&gPrepMainMenuTexts[textIndex]);

	if (title) {
		PutDrawText(
			&gPrepMainMenuTexts[textIndex],
			TILEMAP_LOCATED(gBG0TilemapBuffer, x + 2, y + 2 * textIndex + 1),
			PMX_ColorOf(value),
			0, 0,
			title
		);
	}
}

/* Writes an extension row's help into the description texts, the same way
 * ParsePrepMenuDescTexts does for message ids (same text objects, 0x01 or
 * '\n' starts the next line).
 */
static void PMX_ParseHelp(const char *str)
{
	struct Text *text = &gPrepMainMenuTexts[5];
	const char *next;

	while (*str) {
		u8 c = (u8)*str;

		if (c == 0x01 || c == '\n') {
			text++;
			str++;
		} else {
			if (text >= &gPrepMainMenuTexts[10])
				break;

			next = Text_DrawCharacter(text, str);
			str = (next > str) ? next : str + 1;
		}

		if (text >= &gPrepMainMenuTexts[10])
			break;
	}
}

static void PMX_AddRow(int key, int grey)
{
	struct ProcPrepMenu *proc = PMX_Menu();

	if (!proc || proc->max_index >= PMX_ROW_CAP)
		return;

	gPrepMenuVisibleTableSlots[proc->max_index] = (u8)(key | (grey ? PMX_FLAG_GREY : 0));
	proc->max_index++;
}

static void PMX_AddExtRows(void)
{
	int slot;

	for (slot = 0; slot < PREP_MENU_EXT_SLOT_COUNT; slot++) {
		const struct PrepMenuExtRow *row = PrepMenuExtSlots[slot];

		if (!row || !row->title)
			continue;

		if (row->isVisible && !row->isVisible())
			continue;

		PMX_AddRow(PREP_MENU_EXT_KEY_BASE + slot, row->color == TEXT_COLOR_SYSTEM_GRAY);
	}
}

/* ------------------------------------------------------------------------ */
/* Row callbacks. The vanilla callbacks do not play a sound, so the row plays
 * the window select sound here (one sound per A press, as before).
 */
/* ------------------------------------------------------------------------ */

static void PMX_OnPickUnits(struct ProcAtMenu *proc)
{
	PlaySoundEffect(SONG_SE_SYS_WINDOW_SELECT1);
	PrepScreenMenu_OnPickUnits(proc);
}

static void PMX_OnItems(struct ProcAtMenu *proc)
{
	PlaySoundEffect(SONG_SE_SYS_WINDOW_SELECT1);
	PrepScreenMenu_OnItems(proc);
}

static void PMX_OnSupport(struct ProcAtMenu *proc)
{
	PlaySoundEffect(SONG_SE_SYS_WINDOW_SELECT1);
	PrepScreenMenu_OnSupport(proc);
}

static void PMX_OnSave(struct ProcAtMenu *proc)
{
	PlaySoundEffect(SONG_SE_SYS_WINDOW_SELECT1);
	PrepScreenMenu_OnSave(proc);
}

static void PMX_OnArenaRow(struct ProcAtMenu *proc)
{
	PlaySoundEffect(SONG_SE_SYS_WINDOW_SELECT1);
	sub_8095284(proc);
}

/* ------------------------------------------------------------------------ */
/* Hooks (ROM entry points replaced by Installer.event)                      */
/* ------------------------------------------------------------------------ */

/* Hook $952C0: PrepScreenMenu_OnCheckMap. Opens the Check Map submenu
 * (label 5 of the AtMenu script).
 */
void PMX_PrepScreenMenu_OnCheckMap(struct ProcAtMenu *proc)
{
	PlaySoundEffect(SONG_SE_SYS_WINDOW_SELECT1);
	Proc_Goto(proc, 0x5);
}

/* Hook $96CCC: PrepMenu_OnInit */
void PMX_PrepMenu_OnInit(struct ProcPrepMenu *proc)
{
	proc->cur_index = 0;
	proc->max_index = 0;

	ResetSysHandCursor(proc);
	DisplaySysHandCursorTextShadow(0x740, 1);

	proc->on_PressB     = 0;
	proc->on_PressStart = 0;
	proc->on_End        = 0;
	proc->do_help       = 0;

	gPrepMenuTopVisibleIndex = 0;

	StartMenuScrollBar(proc);
	PutMenuScrollBarAt(3, 65);
	InitMenuScrollBarImg(0xE00, 2);
}

/* Hook $96D10: PrepMenu_CtrlLoop */
void PMX_PrepMenu_CtrlLoop(struct ProcPrepMenu *proc)
{
	int index    = proc->cur_index;
	int xPos     = (proc->xPos + 1) * 8 + 4;
	int visibleY = (proc->yPos + 1) * 8 + (proc->cur_index - gPrepMenuTopVisibleIndex) * 16;

	ShowSysHandCursor(xPos, visibleY, 0x6, 0x400);

	if (proc->do_help) {
		if ((R_BUTTON | B_BUTTON) & gKeyStatusPtr->newKeys) {
			CloseHelpBox();
			proc->do_help = 0;
			return;
		}
	} else {
		if (R_BUTTON & gKeyStatusPtr->newKeys) {
			int value = PMX_ValueAt(proc, proc->cur_index);
			int rtext = (value >= 0) ? PMX_RtextOf(value & PMX_KEY_MASK) : 0;

			if (rtext) {
				StartHelpBox((proc->xPos + 1) * 8 + 4, visibleY, rtext);
				proc->do_help = 1;
			}
			return;
		}

		if (A_BUTTON & gKeyStatusPtr->newKeys) {
			int value = PMX_ValueAt(proc, proc->cur_index);
			int key;
			int i;

			if (value < 0)
				return;

			key = value & PMX_KEY_MASK;

			/* Grey rows (Save without a save, Check Map in a chapter that
			 * cannot show it, extension rows with color GRAY) make the error
			 * sound and do nothing.
			 */
			if (value & PMX_FLAG_GREY) {
				PlaySoundEffect(SONG_6C);
				return;
			}

			if (PMX_IsMap(key)) {
				struct ProcPrepSallyCursor *sally = Proc_Find(gProcScr_SALLYCURSOR);

				switch (key - PMX_KEY_MAP_BASE) {
				case 0:
					PrepMapMenu_OnViewMap(sally);
					break;
				case 1:
					PrepMapMenu_OnFormation(sally);
					break;
				case 2:
					PrepMapMenu_OnOptions(sally);
					break;
				case 3:
					PrepMapMenu_OnSave(sally);
					break;
				default:
					break;
				}
				return;
			}

			if (PMX_IsExt(key)) {
				const struct PrepMenuExtRow *row = PMX_ExtRow(key);

				/* The row's onSelect plays the sound and sets proc->state. */
				if (row && row->onSelect)
					row->onSelect((struct ProcAtMenu *)proc->proc_parent);
				return;
			}

			for (i = 0; i < PMX_MAIN_ROW_COUNT; i++) {
				if (gPMXMainRows[i].key == key) {
					if (gPMXMainRows[i].effect)
						gPMXMainRows[i].effect((struct ProcAtMenu *)proc->proc_parent);
					return;
				}
			}

			return;
		}

		if (B_BUTTON & gKeyStatusPtr->newKeys) {
			if (proc->on_PressB) {
				if (proc->on_PressB(proc->proc_parent)) {
					PlaySoundEffect(SONG_SE_SYS_WINDOW_CANSEL1);
				} else {
					PlaySoundEffect(SONG_6C);
				}
			}
			return;
		}

		if (START_BUTTON & gKeyStatusPtr->newKeys) {
			if (proc->on_PressStart) {
				if (proc->on_PressStart(proc->proc_parent)) {
					PlaySoundEffect(SONG_SE_SYS_WINDOW_SELECT1);
					Proc_Goto(proc, 0x0);
				} else {
					PlaySoundEffect(SONG_6C);
				}
			}
			return;
		}
	}

	if (DPAD_UP & gKeyStatusPtr->repeatedKeys) {
		if (proc->cur_index)
			proc->cur_index--;
		else if (DPAD_UP & gKeyStatusPtr->newKeys)
			proc->cur_index = proc->max_index - 1;
	}

	if (DPAD_DOWN & gKeyStatusPtr->repeatedKeys) {
		if (proc->cur_index < (proc->max_index - 1))
			proc->cur_index++;
		else if (DPAD_DOWN & gKeyStatusPtr->newKeys)
			proc->cur_index = 0;
	}

	if (index != proc->cur_index) {
		PlaySoundEffect(SONG_SE_SYS_CURSOR_UD1);

		if (proc->do_help) {
			int value = PMX_ValueAt(proc, proc->cur_index);
			int rtext = (value >= 0) ? PMX_RtextOf(value & PMX_KEY_MASK) : 0;
			int newVisibleY = (proc->yPos + 1) * 8 + (proc->cur_index - gPrepMenuTopVisibleIndex) * 16;

			if (rtext)
				StartHelpBox((proc->xPos + 1) * 8 + 4, newVisibleY, rtext);
		}
	}

	if (proc->cur_index < gPrepMenuTopVisibleIndex)
		gPrepMenuTopVisibleIndex = proc->cur_index;

	if (proc->cur_index >= gPrepMenuTopVisibleIndex + PMX_LIST_VISIBLE - 1)
		gPrepMenuTopVisibleIndex = proc->cur_index - PMX_LIST_VISIBLE + 1;

	if (proc->max_index > 4)
		PMX_SetPrepScreenMenuPosition(proc->xPos, proc->yPos);

	UpdateMenuScrollBarConfig(8, (u16)gPrepMenuTopVisibleIndex * 16, (u16)proc->max_index, (u8)PMX_LIST_VISIBLE);
}

/* Hook $95C84: AtMenu_Reinitialize (ported from the integrated kernel body;
 * only the menu-table parts differ).
 */
void PMX_AtMenu_Reinitialize(struct ProcAtMenu *proc)
{
	int i;

	SetupBackgrounds(gBgConfig_ItemUseScreen);
	ResetText();
	LoadUiFrameGraphics();
	LoadHelpBoxGfx(NULL, 0xE);
	SetDispEnable(0, 0, 0, 0, 0);
	LoadObjUIGfx();
	ResetUnitSprites();

	MakePrepUnitList();
	PrepAutoCapDeployUnits(proc);
	ReorderPlayerUnitsBasedOnDeployment();

	BG_Fill(gBG0TilemapBuffer, 0);
	BG_Fill(gBG1TilemapBuffer, 0);
	BG_Fill(gBG2TilemapBuffer, 0);

	for (i = 0; i < 5; i++)
		InitText(&gPrepMainMenuTexts[i + 5], 0xE);

	for (i = 0; i < 4; i++)
		InitText(&gPrepMainMenuTexts[i + 1], 0x8);

	InitText(&gPrepMainMenuTexts[0], 0xA);

	/* "Preparations" */
	Decompress(gUnknown_08A1A4C8, (void *)0x6014800);
	/* "Menu", "Start" button */
	Decompress(gUnknown_08A1D510, (void *)0x6016000);
	ApplyPalettes(Pal_SysBrownBox, 0x19, 2);

	/* Chapter text */
	sub_8095C50(0x7000, 0x6);
	ApplyPalette(gUnknown_08A1D4C8, 0x14);
	EnablePaletteSync();

	gLCDControlBuffer.bg0cnt.priority = 0;
	gLCDControlBuffer.bg1cnt.priority = 2;
	gLCDControlBuffer.bg2cnt.priority = 1;
	gLCDControlBuffer.bg3cnt.priority = 3;

	gLCDControlBuffer.dispcnt.win0_on   = 0;
	gLCDControlBuffer.dispcnt.win1_on   = 0;
	gLCDControlBuffer.dispcnt.objWin_on = 0;

	BG_SetPosition(0, 0, 0);
	BG_SetPosition(1, 0, 0);
	BG_SetPosition(2, 0, 0);

	PMX_InitPrepScreenMainMenu(proc);
	BG_EnableSyncByMask(0xF);
	SetDefaultColorEffects();

	StartPrepSpecialCharEffect(proc);
	RestartMuralBackground();
	ApplyPalettes(gUiFramePaletteB, 0x2, 3);

	if (CheckInLinkArena()) {
		Decompress(gUnknown_08A1B698, gGenericBuffer);
		CallARM_FillTileRect(TILEMAP_LOCATED(gBG1TilemapBuffer, 1, 5), gGenericBuffer, 0x1000);
	} else {
		/* Chapter objective tile background */
		Decompress(gUnknown_08A1B658, gGenericBuffer);
		CallARM_FillTileRect(TILEMAP_LOCATED(gBG1TilemapBuffer, 0x10, 2), gGenericBuffer, 0x1000);
		/* Options titles and descriptions tile background */
		Decompress(gUnknown_08A1B698, gGenericBuffer);
		CallARM_FillTileRect(TILEMAP_LOCATED(gBG1TilemapBuffer, 1, 6), gGenericBuffer, 0x1000);
	}

	Prep_DrawChapterGoal(0x5800, 0xB);
	NewSysBlackBoxHandler(proc);
	SysBlackBoxSetGfx(0x6800);
	proc->unk_35 = PMX_GetActivePrepMenuItemIndex();

	{
		struct ProcPrepMenu *menuProc = PMX_Menu();
		int value = PMX_ValueAt(menuProc, menuProc ? menuProc->cur_index : 0);
		int key = (value >= 0) ? (value & PMX_KEY_MASK) : -1;
		const struct PrepMenuExtRow *row = PMX_ExtRow(key);

		if (row) {
			if (row->help)
				PMX_ParseHelp(row->help);
		} else if (key >= 0 && !PMX_IsMap(key)) {
			ParsePrepMenuDescTexts(sub_8095024());
		}
	}

	DrawPrepMenuDescTexts();
}

/* Hook $95C00: sub_8095C00 (description of the row under the cursor). The
 * vanilla caller passes the vanilla description message (sub_8095024) in msg.
 */
void PMX_UpdatePrepMenuDescription(int msg, ProcPtr parent)
{
	struct ProcPrepMenuDesc *proc;
	struct ProcPrepMenu *menuProc;
	int value;
	int key;
	const struct PrepMenuExtRow *row;

	proc = Proc_Find(ProcScr_PrepMenuDescHandler);
	if (proc)
		Proc_End(proc);

	menuProc = PMX_Menu();
	value = PMX_ValueAt(menuProc, menuProc ? menuProc->cur_index : 0);
	key = (value >= 0) ? (value & PMX_KEY_MASK) : -1;
	row = PMX_ExtRow(key);

	if (row) {
		/* Extension row: draw its help string directly (no desc message). */
		ResetPrepMenuDescTexts();
		if (row->help)
			PMX_ParseHelp(row->help);
		DrawPrepMenuDescTexts();
		return;
	}

	if (key < 0 || PMX_IsMap(key))
		return;

	proc = Proc_Start(ProcScr_PrepMenuDescHandler, parent);
	proc->msg = msg;
}

/* Hook $97200: SetPrepScreenMenuPosition */
void PMX_SetPrepScreenMenuPosition(int x, int y)
{
	struct ProcPrepMenu *proc = PMX_Menu();
	int i;

	if (proc) {
		proc->xPos = x;
		proc->yPos = y;

		if (proc->max_index > 1) {
			for (i = 0; i < PMX_LIST_VISIBLE; i++) {
				int slot = gPrepMenuTopVisibleIndex + i;

				if (slot >= proc->max_index)
					break;

				PMX_PutRow(i, x, y, PMX_ValueAt(proc, slot));
			}
		}

		BG_EnableSyncByMask(0x1);
	}
}

/* Hook $97024: SetPrepScreenMenuItem.
 * While InitPrepScreenMainMenu runs (gPrepMenuBuildingList) the call comes from
 * AtMenu_AddPrepScreenSupportMenuItem with index 4 (Support). Otherwise it comes
 * from the Check Map submenu builder: one call per row, in order.
 */
void PMX_SetPrepScreenMenuItem(int index, const void *func, int color, int msg, int msg_rtext)
{
	struct ProcPrepMenu *proc = PMX_Menu();
	int key;

	(void)func;
	(void)msg;
	(void)msg_rtext;

	if (!proc || proc->max_index >= PMX_ROW_CAP)
		return;

	if (gPrepMenuBuildingList) {
		key = index;
	} else {
		if (proc->max_index >= PMX_MAP_ROWS)
			return;

		key = PMX_KEY_MAP_BASE + proc->max_index;
	}

	gPrepMenuVisibleTableSlots[proc->max_index] =
		(u8)(key | ((color == TEXT_COLOR_SYSTEM_GRAY) ? PMX_FLAG_GREY : 0));
	proc->max_index++;
}

/* Hook $95524: InitPrepScreenMainMenu (the Preparations rows) */
void PMX_InitPrepScreenMainMenu(struct ProcAtMenu *proc)
{
	struct ProcPrepMenu *menu;
	int i;

	StartPrepScreenMenu(proc);

	menu = PMX_Menu();
	if (menu)
		menu->max_index = 0;

	for (i = 0; i < PMX_ROW_CAP; i++)
		gPrepMenuVisibleTableSlots[i] = 0;

	gPrepMenuBuildingList = 1;

	if (!CheckInLinkArena()) {
		PMX_AddRow(PMX_KEY_UNIT, 0);
		PMX_AddRow(PMX_KEY_ITEM, 0);
		AtMenu_AddPrepScreenSupportMenuItem(proc);
		PMX_AddRow(PMX_KEY_CHECKMAP, !CanPrepScreenCheckMap());
		PMX_AddRow(PMX_KEY_SAVE, !CanPrepScreenSave());
	} else {
		PMX_AddRow(PMX_KEY_UNIT, 0);
		PMX_AddRow(PMX_KEY_ITEM, 0);
		PMX_AddRow(PMX_KEY_ARENA, 0);
	}

	PMX_AddExtRows();

	gPrepMenuBuildingList = 0;

	SetPrepScreenMenuOnBPress((const void *)PrepScreenMenu_OnBPress);
	SetPrepScreenMenuOnStartPress((const void *)PrepScreenMenu_OnStartPress);

	TileMap_FillRect(gBG0TilemapBuffer, 0xC, 0x13, 0);
	TileMap_FillRect(gBG1TilemapBuffer, 0xC, 0x13, 0);

	if (CheckInLinkArena())
		PMX_SetPrepScreenMenuPosition(1, 5);
	else
		PMX_SetPrepScreenMenuPosition(1, 6);

	/* proc->cur_cmd is the key of the row that opened the last submenu
	 * (sub_8095C2C stores it). Extension submenus leave 0x40 + slot there.
	 */
	PMX_SetPrepScreenMenuSelectedItem(proc->cur_cmd);
}

/* Hook $9631C: AtMenu_StartSubmenu. Vanilla states 1..4 are kept; states
 * 0x40..0x47 start the extension row's startSubmenu.
 */
void PMX_AtMenu_StartSubmenu(struct ProcAtMenu *proc)
{
	sub_8095C2C(proc);

	switch (proc->state) {
	case 1:
		Proc_StartBlocking(ProcScr_PrepUnitScreen, proc);
		break;
	case 2:
		StartPrepItemScreen(proc);
		break;
	case 3:
		StartPrepSaveScreen(proc);
		break;
	case 4:
		StartFortuneSubMenu(2, proc);
		break;
	default:
		if (proc->state >= PREP_MENU_EXT_STATE_BASE &&
		    proc->state < PREP_MENU_EXT_STATE_BASE + PREP_MENU_EXT_SLOT_COUNT) {
			const struct PrepMenuExtRow *row = PMX_ExtRow(proc->state);

			if (row && row->startSubmenu)
				row->startSubmenu(proc);
		}
		break;
	}

	Proc_Break(proc);
}

/* Hook $97154: DrawPrepScreenMenuFrameAt (the Check Map submenu frame) */
void PMX_DrawPrepScreenMenuFrameAt(int x, int y)
{
	struct ProcPrepMenu *proc = PMX_Menu();
	int i;

	if (proc) {
		proc->xPos = x;
		proc->yPos = y;

		DrawUiFrame2(x, y, 0xA, proc->max_index * 2 + 2, 0);

		if (proc->max_index > 1) {
			for (i = 0; i < proc->max_index && i < PMX_MAP_ROWS; i++)
				PMX_PutRow(i, x, y, PMX_ValueAt(proc, i));
		}

		BG_EnableSyncByMask(0x3);
	}
}

/* Hook $9710C: GetActivePrepMenuItemIndex. Returns the key of the cursor row. */
int PMX_GetActivePrepMenuItemIndex(void)
{
	struct ProcPrepMenu *proc = PMX_Menu();
	int value = PMX_ValueAt(proc, proc ? proc->cur_index : 0);

	if (value < 0)
		return 0;

	return value & PMX_KEY_MASK;
}

/* Hook $970CC: SetPrepScreenMenuSelectedItem. Puts the cursor on the visible
 * row whose key is index (no change if the key is not visible).
 */
void PMX_SetPrepScreenMenuSelectedItem(int index)
{
	struct ProcPrepMenu *proc = PMX_Menu();
	int i;

	if (!proc)
		return;

	for (i = 0; i < proc->max_index && i < PMX_ROW_CAP; i++) {
		if ((gPrepMenuVisibleTableSlots[i] & PMX_KEY_MASK) == index) {
			proc->cur_index = (u8)i;
			break;
		}
	}
}
