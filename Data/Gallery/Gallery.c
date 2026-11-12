#include "common-chax.h"
#include "gallery.h"
#include "constants/texts.h"

/*
 * The Gallery's CGs (Documentation/Features/Gallery.md), in the order the Gallery shows them, 9 to a page. Each is a
 * background table index (Kernel/Wizardry/Backgrounds_224_256_Colors/gfx/BG/BG.event) and its name's message. A CG
 * unlocks the first time an event shows that background (TEXT_BG, BACG and the background fades).
 * Add rows anywhere before the end row; at most 96 (GALLERY_MAX).
 */
const struct GalleryEntry gGalleryEntries[] = {
	{ 0x38, MSG_GALLERY_CG_MYRRH_MORVA },
	{ 0x39, MSG_GALLERY_CG_ISMAIRE },
	{ 0x3A, MSG_GALLERY_CG_SETH_ATTACKED },

	/* Backgrounds the story shows, in story order */
	{ 0x1D, MSG_GALLERY_CG_MOUNTAIN_FOREST },
	{ 0x18, MSG_GALLERY_CG_FOREST_MEADOW },
	{ 0x28, MSG_GALLERY_CG_RUINED_HALL },
	{ 0x29, MSG_GALLERY_CG_RUINED_FORT },
	{ 0x0C, MSG_GALLERY_CG_CASTLE_ARCADE },
	{ 0x05, MSG_GALLERY_CG_VILLAGE },
	{ 0x24, MSG_GALLERY_CG_CASTLE_GATE },
	{ 0x21, MSG_GALLERY_CG_WOODLAND_POND },
	{ 0x10, MSG_GALLERY_CG_CASTLE_HALL },
	{ 0x07, MSG_GALLERY_CG_SEASIDE_TOWN },

	{ GALLERY_END, 0 },
};
