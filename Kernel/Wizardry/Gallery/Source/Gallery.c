#include "common-chax.h"
#include "kernel-lib.h"
#include "bmsave.h"
#include "event.h"
#include "eventscript.h"
#include "savemenu.h"
#include "soundroom.h"
#include "bonusclaim.h"
#include "uisupport.h"
#include "gallery.h"
#include "bestiary.h"
#include "skill-codex.h"

/*
 * The Gallery's unlocks and its Extras row (Documentation/Features/Gallery.md).
 *
 * The unlocks are global, as the Sound Room's are: one bit per gGalleryEntries row, in the SRAM entry vanilla keeps
 * for the e-Reader trial map's save block (gSram->saveBlockInfo[SAVE_ID_XMAP]), which this hack never uses. Only the
 * trial map reads it, and its magic word is not the save blocks' (SAVEMAGIC32), so that stays "no trial map". It is
 * outside the game and suspend saves: their layout and signature do not change, and it outlives erasing a file. Only
 * wiping all of SRAM (vanilla's first-boot init) clears it.
 */

#define GALLERY_SRAM_MAGIC 0x59524C47 /* "GLRY" */

struct GallerySram {
	u32 magic;
	u8 bits[GALLERY_MAX / 8];
};

_Static_assert(sizeof(struct GallerySram) <= sizeof(struct SaveBlockInfo), "the unlocks fill one SaveBlockInfo");

int Gallery_Count(void)
{
	int n = 0;

	while (n < GALLERY_MAX && gGalleryEntries[n].bg != GALLERY_END)
		n++;

	return n;
}

static void Gallery_ReadSram(struct GallerySram *buf)
{
	if (IsSramWorking())
		ReadSramFast(&gSram->saveBlockInfo[SAVE_ID_XMAP], buf, sizeof(*buf));

	if (!IsSramWorking() || buf->magic != GALLERY_SRAM_MAGIC) {
		CpuFill16(0, buf, sizeof(*buf));
		buf->magic = GALLERY_SRAM_MAGIC;
	}
}

void Gallery_ReadUnlocks(u8 *bits)
{
	struct GallerySram buf;

	Gallery_ReadSram(&buf);
	memcpy(bits, buf.bits, sizeof(buf.bits));
}

bool Gallery_IsUnlocked(const u8 *bits, int index)
{
	return (unsigned int)index < GALLERY_MAX && (bits[index >> 3] & (1 << (index & 7)));
}

/* An event showed a background: every row that lists it unlocks */
void Gallery_UnlockBackground(int bg)
{
	struct GallerySram buf;
	int i, n = Gallery_Count();
	bool changed = false;

	if (!IsSramWorking())
		return;

	Gallery_ReadSram(&buf);

	for (i = 0; i < n; i++) {
		if (gGalleryEntries[i].bg != bg || Gallery_IsUnlocked(buf.bits, i))
			continue;

		buf.bits[i >> 3] |= 1 << (i & 7);
		changed = true;
	}

	if (changed)
		WriteAndVerifySramFast(&buf, &gSram->saveBlockInfo[SAVE_ID_XMAP], sizeof(buf));
}

/*
 * Event command 0x21 (gEventLoCmdTableRe, the installer): TEXT_BG / BACG, the background transition and the fade in
 * unlock their background, then vanilla's Event21_TextBg runs. A skipped scene counts. The fade's "fade out" form
 * passes background 0, so it unlocks nothing.
 */
u8 Gallery_Event21(struct EventEngineProc *proc)
{
	int subcode = EVT_SUB_CMD(proc->pEventCurrent);
	int bg = EVT_CMD_ARGV(proc->pEventCurrent)[0];

	if (bg < 0)
		bg = gEventSlots[2];

	if (subcode == EVSUBCMD_BACG || subcode == EVSUBCMD_0x2141 || (subcode == EVSUBCMD_2142 && bg != 0))
		Gallery_UnlockBackground(bg);

	return Event21_TextBg(proc);
}

//! FE8U = 0x080AA158: the Extras rows with a screen of their own; the Gallery's, the Bestiary's and the Skill Codex's are new
LYN_REPLACE_CHECK(SaveMenuStartExtraMiscScreen);
void SaveMenuStartExtraMiscScreen(struct SaveMenuProc *proc)
{
	proc->main_sel_bitfile = MAIN_MENU_OPTION_EXTRAS;

	Proc_End(proc->savedraw);

	SetPrimaryHBlankHandler(NULL);

	if (proc->approc != NULL)
		APProc_Delete(proc->approc);

	switch (proc->extra_sel_bitfile) {
	case EXTRA_MENU_OPTION_BESTIARY:
		StartBestiaryScreen(proc);
		return;

	case EXTRA_MENU_OPTION_SKILL_CODEX:
		StartSkillCodexScreen(proc);
		return;

	case EXTRA_MENU_OPTION_BONUS_CLAIM:
		StartBonusClaimScreen(proc);
		return;

	case EXTRA_MENU_OPTION_SOUND_ROOM:
		StartSoundRoomScreen(proc);
		return;

	case EXTRA_MENU_OPTION_SUPPORT:
		StartSupportScreen(proc);
		return;

	case EXTRA_MENU_OPTION_GALLERY:
		StartGalleryScreen(proc);
		return;

	default:
		return;
	}
}

//! FE8U = 0x080AA1BC: back from that screen. The Gallery returns to the Extras list, as the Sound Room does.
LYN_REPLACE_CHECK(SaveMenuPostExtraMiscScreen);
void SaveMenuPostExtraMiscScreen(struct SaveMenuProc *proc)
{
	switch (proc->extra_sel_bitfile) {
	case EXTRA_MENU_OPTION_BONUS_CLAIM:
		Proc_Goto(proc, PL_SAVEMENU_POST_BONUS_CLAIM);
		return;

	case EXTRA_MENU_OPTION_SUPPORT:
	case EXTRA_MENU_OPTION_SOUND_ROOM:
	case EXTRA_MENU_OPTION_GALLERY:
	case EXTRA_MENU_OPTION_BESTIARY:
	case EXTRA_MENU_OPTION_SKILL_CODEX:
		Proc_Goto(proc, PL_SAVEMENU_10);
		return;
	}
}
