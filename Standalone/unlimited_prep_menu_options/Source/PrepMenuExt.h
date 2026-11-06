#pragma once

/*
 * Add-on interface for the Unlimited Prep Menu Options standalone patch.
 *
 * An add-on appends rows to the Preparations menu without recompiling
 * Standalone/unlimited_prep_menu_options. It writes one row per free slot
 * in the header that Installer.event places at ROM offset $1000000:
 *
 *   offset  size  content
 *   $1000000  4   magic "PMX1" (0x50 0x4D 0x58 0x31)
 *   $1000004 32   PrepMenuExtSlots[8], one POIN per slot, 0 = empty
 *
 * Slot k lives at $1000004 + 4 * k. A non-zero slot points to a
 * struct PrepMenuExtRow in ROM. Rows appear after the vanilla rows, in slot
 * order. A row with isVisible == NULL is always shown; otherwise it is shown
 * only while isVisible() returns true (it is checked when the menu is built).
 *
 * Keys and states (the full table is in the patch README):
 *   key   = 0x40 + slot   kept in the menu's visible list and in proc->cur_cmd;
 *                         the cursor is put back on the row by this value
 *   state = 0x40 + slot   proc->state after the row is selected (onSelect)
 *
 * Writing an extension row (see README "Writing an extension row"):
 *
 *   onSelect(proc)      A pressed. Play the sound, set proc->state = 0x40 + slot,
 *                       Proc_Goto(proc, 0xA). Nothing else (the menu calls the rest).
 *   startSubmenu(proc)  Called from AtMenu_StartSubmenu. Start the row's screen
 *                       with Proc_StartBlocking(ScreenScript, proc), so proc is the
 *                       parent. Do not Proc_Goto the AtMenu proc from here.
 *   screen end          The screen's end callback hands control back:
 *                           pproc = proc->proc_parent;
 *                           pproc->cur_cmd = 0x40 + slot;
 *                           pproc->state = 1;
 *                       The AtMenu script then goes to label 9 (AtMenu_Reinitialize,
 *                       fade in, menu loop), and the menu is rebuilt with the cursor
 *                       on the row. This is the same hand-off as the vanilla
 *                       sub-screens and the kernel's Hex row (HexScreen_OnEnd).
 *
 * Rules for the return:
 *   - Use state = 1 (any of 1, 2, 4, 5 does the same, label 9). State 3 goes to
 *     label 7 (the Save path); do not use it for an extension row.
 *   - Do not Proc_Goto the AtMenu proc to a label to return. Label 0xD runs the old
 *     AtMenu_CtrlLoop, which takes A itself and opens the Support fortune popup
 *     (it sets state 4). Labels 1 and 0xC leave the screen blank.
 *   - A row with no screen must still hand back: in startSubmenu set
 *     pproc->cur_cmd = 0x40 + slot and pproc->state = 1 yourself. Not tested in
 *     the emulator. startSubmenu must not be NULL for a row that does anything
 *     (with NULL the state stays 0x40 + slot and, by reading the AtMenu script,
 *     nothing returns: the screen stays faded out).
 *
 * Text: title and help are FE8 encoded strings. In help, '\n' (0x0A) or
 * 0x01 starts the next line. help may be NULL. Rows whose color is
 * TEXT_COLOR_SYSTEM_GRAY are drawn grey and do nothing when A is pressed
 * (SONG_6C error sound).
 */

#include "global.h"

struct ProcAtMenu;

#define PREP_MENU_EXT_MAGIC_P 0x50 /* 'P' */
#define PREP_MENU_EXT_MAGIC_M 0x4D /* 'M' */
#define PREP_MENU_EXT_MAGIC_X 0x58 /* 'X' */
#define PREP_MENU_EXT_MAGIC_1 0x31 /* '1' */

#define PREP_MENU_EXT_SLOT_COUNT 8
#define PREP_MENU_EXT_KEY_BASE   0x40
#define PREP_MENU_EXT_STATE_BASE 0x40

struct PrepMenuExtRow {
	const char *title;                             /* FE8 text, drawn in the row */
	const char *help;                              /* FE8 text, may be NULL */
	bool (*isVisible)(void);                       /* NULL = always shown */
	void (*onSelect)(struct ProcAtMenu *proc);     /* A pressed: sound + state + Proc_Goto(proc, 0xA) */
	void (*startSubmenu)(struct ProcAtMenu *proc); /* from AtMenu_StartSubmenu; starts the screen, see above */
	u8 color;                                      /* TEXT_COLOR_SYSTEM_*; GRAY = not selectable */
	u8 pad[3];
};
