/* TEST ONLY - do not install. Extension row for the Unlimited Prep Menu Options test.
 *
 * The row follows the contract in Source/PrepMenuExt.h ("Writing an extension row"),
 * the same way the integrated kernel's Hex row does:
 *   onSelect      plays the sound, sets proc->state = 0x40 + slot, Proc_Goto(proc, 0xA)
 *   startSubmenu  starts a blocking child proc with the AtMenu proc as parent
 *   child end     sets proc->cur_cmd = 0x40 + slot (cursor restore) and proc->state = 1
 *                 (state 1 makes the AtMenu script go to label 9: redraw, fade in, menu)
 */
#include "global.h"
#include "proc.h"
#include "fontgrp.h"
#include "prepscreen.h"
#include "sysutil.h"
#include "hardware.h"
#include "soundwrapper.h"
#include "constants/songs.h"
#include "../Source/PrepMenuExt.h"

#define TESTROW_SLOT   0
#define TESTROW_FRAMES 60 /* how long the test child screen stays up */

struct ProcTestRow {
	PROC_HEADER;
	/* 29 */ u8 _pad29_;
	/* 2A */ u16 savedBackdrop; /* backdrop colour before the child (kept here: a static made lyn fail) */
	/* the AtMenu proc is proc_parent */
};

/* The child shows a red backdrop so its screen is visible in the shots (the prep
 * menu's sprites and text are gone while a sub-screen runs). The old colour is put
 * back when the child ends.
 */
#define TESTROW_BACKDROP_COLOR 0x001F /* BGR555 red */

static void TestRow_SetBackdrop(ProcPtr proc)
{
	((struct ProcTestRow *)proc)->savedBackdrop = gPaletteBuffer[0];
	gPaletteBuffer[0] = TESTROW_BACKDROP_COLOR;
	/* The fade-out before the submenu leaves the BG layers off. No layers on:
	 * only the backdrop shows.
	 */
	SetDispEnable(0, 0, 0, 0, 0);
	EnablePaletteSync();
}

static void TestRow_OnChildEnd(ProcPtr proc)
{
	struct ProcAtMenu *pproc = (struct ProcAtMenu *)((struct ProcTestRow *)proc)->proc_parent;

	gPaletteBuffer[0] = ((struct ProcTestRow *)proc)->savedBackdrop;
	EnablePaletteSync();

	/* Same hand-off as the vanilla sub-screens and the kernel's HexScreen_OnEnd */
	pproc->cur_cmd = PREP_MENU_EXT_KEY_BASE + TESTROW_SLOT;
	pproc->state = 1;
}

/* Test child screen: red backdrop, wait TESTROW_FRAMES frames, then end. The end
 * callback hands control back. (No fade: the child is shown at once.)
 */
const struct ProcCmd ProcScr_PMXTestRow[] = {
	PROC_NAME("PMXTestRow"),
	PROC_SET_END_CB(TestRow_OnChildEnd),
	PROC_CALL(TestRow_SetBackdrop),
	PROC_SLEEP(TESTROW_FRAMES),
	PROC_END,
};

static void TestRow_OnSelect(struct ProcAtMenu *proc)
{
	PlaySoundEffect(SONG_SE_SYS_WINDOW_SELECT1);
	proc->state = PREP_MENU_EXT_STATE_BASE + TESTROW_SLOT;
	Proc_Goto(proc, 0xA);
}

/* Called from AtMenu_StartSubmenu (Source/PrepMenuUnlimited.c). Starts the child
 * screen; the child's end callback sets state = 1 and the menu returns.
 */
static void TestRow_StartSubmenu(struct ProcAtMenu *proc)
{
	Proc_StartBlocking(ProcScr_PMXTestRow, proc);
}

const struct PrepMenuExtRow TestRowDef = {
	"Test",
	"Test row",
	0,
	TestRow_OnSelect,
	TestRow_StartSubmenu,
	TEXT_COLOR_SYSTEM_WHITE,
	{ 0, 0, 0 },
};
