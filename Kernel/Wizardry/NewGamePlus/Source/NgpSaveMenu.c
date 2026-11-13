#include "common-chax.h"
#include "kernel-lib.h"
#include "savemenu.h"
#include "bmsave.h"
#include "soundwrapper.h"
#include "new-game-plus.h"
#include "gallery.h"
#include "bestiary.h"
#include "skill-codex.h"
#include "constants/songs.h"
#include "constants/texts.h"

/*
 * New Game+ on the title's save menu (Documentation/Features/NewGamePlus.md). Extras > New Game+ picks a save that
 * has been to the Creature Campaign, then goes on as New Game does: the difficulty, then an empty file.
 *
 * The row is vanilla's unused Extras bit 3 ("Battle History"), whose label Img_NgpMenuObjs redraws. That sheet also
 * holds the Gallery's and the Bestiary's labels; gExtrasLabelSprites (below) is the Extras list's sprite for each row. The extras list
 * (sub_80A9B90), the save picker after it (sub_80A9E1C) and the header label (sub_80ABF74) are vanilla's, with a case
 * for it.
 */

extern const u8 Img_NgpMenuObjs[];

/* The first save, from the last one used, that has been to the postgame */
static int NgpSaveMenu_FirstSourceSlot(void)
{
	int i, slot = ReadLastGameSaveId();

	for (i = 0; i < 3; i++) {
		if (slot < 0 || slot > 2)
			slot = 0;

		if (NewGamePlus_SlotReachedPostgame(slot))
			return slot;

		slot++;
	}
	return 0;
}

/* Back on the main menu: no New Game+ is under way */
void NewGamePlus_SaveMenuMainLoop(struct SaveMenuProc *proc)
{
	NewGamePlus_Disarm();
	SameMenu_CtrlLoop(proc);
}

/* After the source save: fade out, then New Game's difficulty select, with the screen set as New Game leaves it */
static void NgpSaveMenu_StartNewGame(ProcPtr proc)
{
	struct SaveMenuProc *menu = ((struct Proc *)proc)->proc_parent;

	menu->main_sel_bitfile = MAIN_MENU_OPTION_NEW_GAME;
	menu->sus_slot = SaveMenuModifySaveSlot(menu->sus_slot, 0, 1);
	menu->unk_2f = 0;
	menu->unk_46 = 0;
	menu->scroll_cnt = 0;

	Proc_Goto(menu, PL_SAVEMENU_DIFFICULTY_SEL);
	StartBgmVolumeChange(0xC0, 0x100, 0x10, 0);
}

static const struct ProcCmd ProcScr_NgpStartNewGame[] = {
	PROC_NAME("NgpStartNewGame"),
	PROC_CALL_ARG(NewFadeOut, 8),
	PROC_WHILE(FadeOutExists),
	PROC_CALL(NgpSaveMenu_StartNewGame),
	PROC_END,
};

//! FE8U = 0x080A9B90: the Extras list
LYN_REPLACE_CHECK(sub_80A9B90);
void sub_80A9B90(struct SaveMenuProc *proc)
{
	int previous = proc->extra_select;

	proc->jump_label = PL_SAVEMENU_10;

	if (gKeyStatusPtr->repeatedKeys & DPAD_UP) {
		if (proc->extra_select != 0)
			proc->extra_select--;
		else if (gKeyStatusPtr->newKeys & DPAD_UP)
			proc->extra_select = proc->max_choice - 1;
	} else if (gKeyStatusPtr->repeatedKeys & DPAD_DOWN) {
		if (proc->extra_select < proc->max_choice - 1)
			proc->extra_select++;
		else if (gKeyStatusPtr->newKeys & DPAD_DOWN)
			proc->extra_select = 0;
	}

	if (previous != proc->extra_select)
		PlaySoundEffect(SONG_SE_SYS_CURSOR_UD1);

	if (gKeyStatusPtr->newKeys & A_BUTTON) {
		proc->extra_sel_bitfile = SaveMenuGetBitfile(proc->extra_options, proc->extra_select);
		PlaySoundEffect(SONG_SE_SYS_WINDOW_SELECT1);

		proc->scroll_cnt = 0;

		switch (proc->extra_sel_bitfile) {
		case EXTRA_MENU_OPTION_BONUS_CLAIM:
			proc->sus_slot = SaveMenuModifySaveSlot(ReadLastGameSaveId(), 1, 1);
			sub_80A9D20(proc, 0);
			PlaySoundEffect(SONG_SE_SYS_WINDOW_SELECT1);
			Proc_Goto(proc, PL_SAVEMENU_12);
			break;

		case EXTRA_MENU_OPTION_NEW_GAME_PLUS:
			proc->sus_slot = NgpSaveMenu_FirstSourceSlot();
			sub_80A9D20(proc, 0);
			PlaySoundEffect(SONG_SE_SYS_WINDOW_SELECT1);
			Proc_Goto(proc, PL_SAVEMENU_12);
			break;

		case EXTRA_MENU_OPTION_GALLERY:
		case EXTRA_MENU_OPTION_BESTIARY:
		case EXTRA_MENU_OPTION_SKILL_CODEX:
			/*
			 * The Gallery (Kernel/Wizardry/Gallery), in the Map row's place; the Bestiary, in the trial map's; the
			 * Skill Codex, in bit 7
			 */
			Proc_Goto(proc, PL_SAVEMENU_EXEC_EXTRA_MISC_OPTION);
			break;

		case EXTRA_MENU_OPTION_SOUND_ROOM:
			CallSomeSoundMaybe(SONG_NONE, 0xC0, 0, 0x18, NULL);
			Proc_Goto(proc, PL_SAVEMENU_EXEC_EXTRA_MISC_OPTION);
			break;

		case EXTRA_MENU_OPTION_SUPPORT:
			CallSomeSoundMaybe(SONG_DISTANT_ROADS, 0xC0, 0x100, 0x18, NULL);
			Proc_Goto(proc, PL_SAVEMENU_EXEC_EXTRA_MISC_OPTION);
			break;

		default:
			SaveMenu_NewGame(proc);
			Proc_Goto(proc, PL_SAVEMENU_NEW_GAME);
			break;
		}
	} else if (gKeyStatusPtr->newKeys & B_BUTTON) {
		proc->scroll_cnt = 0;
		Proc_Goto(proc, PL_SAVEMENU_9);
		PlaySoundEffect(SONG_SE_SYS_WINDOW_CANSEL1);
	}
}

/* A on a save, for New Game+ */
static void NgpSaveMenu_SelectSource(struct SaveMenuProc *proc)
{
	if (!NewGamePlus_SlotReachedPostgame(proc->sus_slot)) {
		sub_80A9DFC(0x28, 0x38, MSG_NewGamePlus_NeedPostgame, proc);
		return;
	}

	if (SaveMenuModifySaveSlot(proc->sus_slot, 0, 1) == (u8)-1) {
		sub_80A9DFC(0x28, 0x38, MSG_NewGamePlus_NoEmptyFile, proc);
		return;
	}

	NewGamePlus_Arm(proc->sus_slot);
	PlaySoundEffect(SONG_SE_SYS_WINDOW_SELECT1);

	Proc_Goto(proc, PL_SAVEMENU_BLOCKING);
	Proc_Start(ProcScr_NgpStartNewGame, proc);
}

//! FE8U = 0x080A9E1C: picking a save for an Extras row
LYN_REPLACE_CHECK(sub_80A9E1C);
void sub_80A9E1C(struct SaveMenuProc *proc)
{
	proc->jump_label = PL_SAVEMENU_SAVE_SLOT_SEL;

	if (proc->cursor_config == 0) {
		if (gKeyStatusPtr->newKeys & DPAD_UP)
			sub_80A9D20(proc, -1);
		else if (gKeyStatusPtr->newKeys & DPAD_DOWN)
			sub_80A9D20(proc, 1);
	} else if (gKeyStatusPtr->newKeys & DPAD_LEFT) {
		if (proc->cursor_config != 1) {
			proc->cursor_config = 1;
			PlaySoundEffect(SONG_SE_SYS_CURSOR_LR1);
		}
	} else if (gKeyStatusPtr->newKeys & DPAD_RIGHT) {
		if (proc->cursor_config != 2) {
			proc->cursor_config = 2;
			PlaySoundEffect(SONG_SE_SYS_CURSOR_LR1);
		}
	}

	if (gKeyStatusPtr->newKeys & A_BUTTON) {
		switch (proc->extra_sel_bitfile) {
		case EXTRA_MENU_OPTION_BONUS_CLAIM:
			if (proc->unk_3a[proc->sus_slot] & 2) {
				if (proc->sus_slot_cur != (u8)-1) {
					ExecSaveMenuMiscOption(proc);
					return;
				}

				ReadGameSave(proc->sus_slot);
				Proc_Goto(proc, PL_SAVEMENU_EXEC_EXTRA_MISC_OPTION);
				PlaySoundEffect(SONG_SE_SYS_WINDOW_SELECT1);
				return;
			}

			sub_80A9DFC(0x2E, 0x38, 0x891, proc); /* "Send data from Chapter 2+" */
			return;

		case EXTRA_MENU_OPTION_MAP:
			if (proc->unk_3a[proc->sus_slot] & 4) {
				if (proc->sus_slot_cur == (u8)-1) {
					ReadGameSave(proc->sus_slot);
					SaveMenu_NewGame(proc);
					PlaySoundEffect(SONG_SE_SYS_WINDOW_SELECT1);
					return;
				}

				ExecSaveMenuMiscOption(proc);
				return;
			}

			sub_80A9DFC(0x2E, 0x38, 0x895, proc); /* "Select cleared save data." */
			return;

		case EXTRA_MENU_OPTION_NEW_GAME_PLUS:
			NgpSaveMenu_SelectSource(proc);
			return;

		default:
			return;
		}
	} else if (gKeyStatusPtr->newKeys & B_BUTTON) {
		PlaySoundEffect(SONG_SE_SYS_WINDOW_CANSEL1);

		if (proc->cursor_config != 0) {
			SaveMenuDrawSubSelBox(proc, 0);
			SaveMenuHandleHelpBox(proc);
			return;
		}

		/* A help box may have used the labels' VRAM */
		Decompress(Img_NgpMenuObjs, (u8 *)0x06010000 + OBJCHR_SAVEMENU_MAINCHOICE_STR * 0x20);
		proc->scroll_cnt = 0;
		Proc_Goto(proc, PL_SAVEMENU_13);
	}
}

//! FE8U = 0x080ABF74: the chosen Extras row's label, for the header of the save picker (OBJ tile 0x10E)
LYN_REPLACE_CHECK(sub_80ABF74);
void sub_80ABF74(u8 bitfile)
{
	const u8 *src;
	u8 *dst = (u8 *)0x060121C0;
	int i;

	if (bitfile & EXTRA_MENU_OPTION_BONUS_CLAIM)
		src = (const u8 *)0x06014000;
	else if (bitfile & EXTRA_MENU_OPTION_MAP)
		src = (const u8 *)0x06014800;
	else if (bitfile & EXTRA_MENU_OPTION_NEW_GAME_PLUS)
		src = (const u8 *)0x06014000 + 0xE * 0x20;
	else
		return;

	/* Two rows of 14 tiles, 32 tiles apart */
	for (i = 0; i < 2; i++)
		CpuFastCopy(src + i * 0x400, dst + i * 0x400, 0xE * 0x20);
}

/* The Extras list's sprite for the row: four pieces, 112 pixels, as the other labels (SpriteArray_08A2099C[3]) */
const u16 Sprite_NgpExtrasLabel[] = {
	4,
	0x4000, 0x8008, 0x0A0E,
	0x4000, 0x8028, 0x0A12,
	0x4000, 0x8048, 0x0A16,
	0x0000, 0x4068, 0x0A1A,
};

/*
 * The Extras list's label sprites, one per row bit (vanilla's SpriteArray_08A2099C, which sub_80AAC40 reads; the
 * installer points it here). Vanilla's for the rows whose labels are where vanilla drew them. The Gallery's and Link
 * Arena's are cut to their letters, so that the free tiles round them can hold the Bestiary's label (the Gallery's
 * sheet row, tiles 0-3 and 10-11) and the Skill Codex's (Link Arena's row, tiles 14-17, 28-31 and 26-27).
 *
 * Every label is centred on x 64 from the sprite's origin, as vanilla's are: the left column of the sheet is drawn
 * from x 8 (its words sit 8 pixels left in their 112-pixel slot), the right column from x 0. The Bestiary's and the
 * Skill Codex's words are centred in their pieces, which start at 64 less half their width.
 */
static const u16 Sprite_GalleryExtrasLabel[] = {
	2,
	0x4000, 0x8000 + 40, 0x0A44,
	0x0000, 0x4000 + 72, 0x0A48,
};

static const u16 Sprite_BestiaryExtrasLabel[] = {
	2,
	0x4000, 0x8000 + 40, 0x0A40,
	0x0000, 0x4000 + 72, 0x0A4A,
};

static const u16 Sprite_LinkArenaExtrasLabel[] = {
	2,
	0x4000, 0x8000 + 32, 0x0A92,
	0x4000, 0x8000 + 64, 0x0A96,
};

static const u16 Sprite_SkillCodexExtrasLabel[] = {
	3,
	0x4000, 0x8000 + 24, 0x0A8E,
	0x4000, 0x8000 + 56, 0x0A9C,
	0x0000, 0x4000 + 88, 0x0A9A,
};

const u16 *const gExtrasLabelSprites[8] = {
	[EXTRA_MENU_LINK_ARENA]    = Sprite_LinkArenaExtrasLabel,
	[EXTRA_MENU_SOUND_ROOM]    = (const u16 *)0x08A20864,
	[EXTRA_MENU_SUPPORT]       = (const u16 *)0x08A2087E,
	[EXTRA_MENU_NEW_GAME_PLUS] = Sprite_NgpExtrasLabel,
	[EXTRA_MENU_GALLERY]       = Sprite_GalleryExtrasLabel,
	[EXTRA_MENU_BONUS_CLAIM]   = (const u16 *)0x08A208CC,
	[EXTRA_MENU_BESTIARY]      = Sprite_BestiaryExtrasLabel,
	[EXTRA_MENU_SKILL_CODEX]   = Sprite_SkillCodexExtrasLabel,
};
