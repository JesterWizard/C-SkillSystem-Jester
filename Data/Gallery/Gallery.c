#include "common-chax.h"
#include "gallery.h"
#include "constants/texts.h"

/*
 * The Gallery's CGs (Documentation/Features/Gallery.md), in the order the Gallery shows them, 9 to a page. Each is a
 * background table index (Kernel/Wizardry/Backgrounds_224_256_Colors/gfx/BG/BG.event) and its name's message. A CG
 * unlocks the first time an event shows that background (TEXT_BG, BACG and the background fades).
 * The unlock bits follow the rows: add new rows at the end. Removing or reordering rows moves the unlocks already
 * made, so change GALLERY_SRAM_MAGIC (Kernel/Wizardry/Gallery/Source/Gallery.c) with it. At most 96 (GALLERY_MAX).
 */
const struct GalleryEntry gGalleryEntries[] = {
	/* The story's CGs, in story order */
	{ 0xDB, MSG_GALLERY_CG_FLIGHT_FROM_RENAIS },
	{ 0xDC, MSG_GALLERY_CG_REUNITED },
	{ 0xDD, MSG_GALLERY_CG_SETHS_PLEA },
	{ 0xDE, MSG_GALLERY_CG_IDE_BURNS },
	{ 0xDF, MSG_GALLERY_CG_NEIMIS_MIRROR },
	{ 0xE0, MSG_GALLERY_CG_THE_MOGALL },
	{ 0xE1, MSG_GALLERY_CG_JOSHUA_INTERVENES },
	{ 0xE2, MSG_GALLERY_CG_RENVALL_AMBUSH },
	{ 0xE3, MSG_GALLERY_CG_BAEL_ATTACK },
	{ 0xE4, MSG_GALLERY_CG_WAR_COUNCIL },
	{ 0xE5, MSG_GALLERY_CG_MYRRHS_WINGS },	/* not shown in a scene yet: locked until one shows it */

	{ GALLERY_END, 0 },
};
