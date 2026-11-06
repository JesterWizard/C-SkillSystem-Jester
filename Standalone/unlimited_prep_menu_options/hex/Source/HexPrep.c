/*
 * The Hex row of the Preparations menu (see ../../Source/PrepMenuExt.h for the contract, and ../../test/TestRow.c for the
 * same pattern with a test screen). The parent's menu draws the row and calls onSelect on A and startSubmenu from
 * AtMenu_StartSubmenu. The screen (HexScreen.c) hands control back with cur_cmd = 0x40 + HEX_PREP_SLOT and state = 1.
 */
#include "global.h"
#include "proc.h"
#include "fontgrp.h"
#include "prepscreen.h"
#include "soundwrapper.h"
#include "constants/songs.h"
#include "../../Source/PrepMenuExt.h"
#include "Hex.h"

static void HexPrep_OnSelect(struct ProcAtMenu *proc)
{
	PlaySoundEffect(SONG_SE_SYS_WINDOW_SELECT1);
	proc->state = PREP_MENU_EXT_STATE_BASE + HEX_PREP_SLOT;
	Proc_Goto(proc, 0xA); /* the menu fades out, then calls startSubmenu */
}

static void HexPrep_StartSubmenu(struct ProcAtMenu *proc)
{
	Proc_StartBlocking(ProcScr_HexScreen, proc);
}

const struct PrepMenuExtRow HexPrepRow = {
	"Hex",
	"Play Hex with your army\nand view your cards.",
	0,
	HexPrep_OnSelect,
	HexPrep_StartSubmenu,
	TEXT_COLOR_SYSTEM_WHITE,
	{ 0, 0, 0 },
};
