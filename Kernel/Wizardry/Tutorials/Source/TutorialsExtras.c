#include "common-chax.h"
#include "savemenu.h"
#include "skill-codex.h"
#include "tutorials.h"

/*
 * The order of the save menu's rows. Vanilla lists a menu's rows in bit order, 0 first. The Extras list's last row,
 * Tutorials, is bit 0 (Link Arena's, which this hack never sets), so for the Extras list bit 0 comes after bit 7.
 *
 * Both menus go through these two. The Extras list always has bit 7 (the Skill Codex); the main menu never uses bit
 * 7 (MAIN_MENU_7), so it keeps vanilla's order, Resume Chapter (bit 0) first.
 */
static int TutorialsExtras_BitAt(u8 bitfile, int pos)
{
	if (bitfile & EXTRA_MENU_OPTION_SKILL_CODEX)
		return pos < 7 ? pos + 1 : 0;

	return pos;
}

//! FE8U = 0x080A8844: the bit of the menu's row index
LYN_REPLACE_CHECK(SaveMenuGetBitfile);
u8 SaveMenuGetBitfile(u8 bitfile, u32 index)
{
	u32 row = 0;
	int pos, bit;

	for (pos = 0; pos < 8; pos++) {
		bit = TutorialsExtras_BitAt(bitfile, pos);
		if (!(bitfile & (1 << bit)))
			continue;
		if (row == index)
			return 1 << bit;
		row++;
	}
	return 0xFF;
}

//! FE8U = 0x080A887C: the row index of a bit (mask) of the menu
LYN_REPLACE_CHECK(SaveMenuGetBitfileByMask);
u8 SaveMenuGetBitfileByMask(u8 bitfile, u8 mask)
{
	int row = 0, pos, bit;

	for (pos = 0; pos < 8; pos++) {
		bit = TutorialsExtras_BitAt(bitfile, pos);
		if (!(bitfile & (1 << bit)))
			continue;
		if (mask & (1 << bit))
			return row;
		row++;
	}
	return 0xFF;
}

/* Vanilla's: one Extras row, its banner and its label */
void sub_80AAC40(struct SaveDrawProc *proc, int x, int y, u8 bit, int bannerPal, int labelPal);
void sub_80AAF6C(struct SaveDrawProc *proc);

/*
 * FE8U = 0x080AAF6C: the Extras list's rows and its hand, as vanilla's. Vanilla's layout holds 7 rows at most (from
 * y 2, 21 pixels apart); with Tutorials, New Game+ and the Bonus Claim all showing there are 8, so 8 go from y 2, 19
 * pixels apart, and the last two were off the screen before.
 */
LYN_REPLACE_CHECK(sub_80AAF6C);
void sub_80AAF6C(struct SaveDrawProc *proc)
{
	struct SaveMenuProc *menu = proc->proc_parent;
	int i, y, y0, step, bit;

	if ((u16)(menu->unk_46 - 1) > 0x1B6)
		return;

	if (menu->max_choice >= 8) {
		y0 = 2;
		step = 19;
	} else if (menu->max_choice == 7) {
		y0 = 2;
		step = 21;
	} else {
		y0 = 0x44 - menu->max_choice * 12;
		if (y0 <= 1)
			y0 = 2;
		step = 25;
	}

	for (i = 0, y = y0; i < menu->max_choice; i++, y += step) {
		bit = BitfileToIndex(SaveMenuGetBitfile(menu->extra_options, i));
		sub_80AAC40(proc, 0x10C - menu->unk_46, y, bit, i == menu->extra_select ? 1 : 6, 4);
	}

	if (menu->jump_label == 10)
		sub_80AB4F4(0, 0x1C, y0 + step * menu->extra_select, proc);
}
