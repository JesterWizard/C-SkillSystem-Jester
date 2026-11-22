#pragma once

#include "common-chax.h"

/*
 * Tutorials (Documentation/Features/Tutorials.md): Extras > Tutorials, the last Extras row. A list of tutorials on
 * Select Mode's banners, and the chosen one's text in Select Mode's box, a page at a time.
 */

/*
 * The Extras row: vanilla's Link Arena bit (0), which this hack never sets (RemoveLinkArena.event). The Extras list
 * shows its rows in bit order; SaveMenuGetBitfile and SaveMenuGetBitfileByMask (TutorialsExtras.c) list bit 0 last
 * when bit 7 (the Skill Codex, always there) is set, so the row is under the Skill Codex. The main menu never sets
 * bit 7, so its order (Resume Chapter first) is unchanged.
 */
#define EXTRA_MENU_TUTORIALS        0
#define EXTRA_MENU_OPTION_TUTORIALS (1 << EXTRA_MENU_TUTORIALS)

struct Tutorial {
	u16 title;  /* the banner's text */
	u16 text;   /* the box's text: wrapped to the box and shown four lines a page */
};

extern const struct Tutorial gTutorials[];  /* ends with a title of 0 */

ProcPtr StartTutorialsScreen(ProcPtr parent);
