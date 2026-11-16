#pragma once

#include "common-chax.h"

/*
 * The Gallery (Documentation/Features/Gallery.md): Extras > Gallery shows the CGs (backgrounds) listed in
 * gGalleryEntries (Data/Gallery/Gallery.c), 3x3 to a page. A CG unlocks the first time an event shows it, in any save
 * file, or is unlocked from the start (GALLERY_UNLOCKED); the unlocks are kept outside the save files.
 */

/* The Extras row: vanilla's Map row (bit 4), which the Creature Campaign no longer needs */
#define EXTRA_MENU_GALLERY        4
#define EXTRA_MENU_OPTION_GALLERY (1 << EXTRA_MENU_GALLERY)

/* At most 96 CGs: their unlock bits are 12 bytes */
#define GALLERY_MAX 96
#define GALLERY_END 0xFFFF

/* A row's unlocked field: shown unlocked from the start (for CGs no scene shows yet) */
#define GALLERY_UNLOCKED 1

struct GalleryEntry {
	u16 bg;		/* the background table's index (TEXT_BG, BACG) */
	u16 name;	/* its name's message */
	u16 unlocked;	/* GALLERY_UNLOCKED: unlocked from the start, without an event showing it */
};

extern const struct GalleryEntry gGalleryEntries[];

int Gallery_Count(void);
void Gallery_ReadUnlocks(u8 *bits);
bool Gallery_IsUnlocked(const u8 *bits, int index);
void Gallery_UnlockBackground(int bg);

ProcPtr StartGalleryScreen(ProcPtr parent);
