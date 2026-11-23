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
	u16 title;          /* the title's message (Districts.txt) */
	u16 text;           /* the box's summary: four lines at most (the fifth is "A: Try it") */
	const u8 *img;      /* the title in the Extras labels' letters (Source/TutorialTitles.c, generated) */
	u16 intro;          /* the explanation shown on its map, before the player tries it */
	u8 chapter;         /* its map: TUTORIAL_MAP, or a district */
	const struct UnitDefinition *units; /* loaded on TUTORIAL_MAP (blue and red), ending with charIndex 0 */
	void (*setup)(void); /* anything else the map needs (an option on, fog, a skill, a dead unit...), or NULL */
	void (*after)(ProcPtr event); /* run after the explanation, with the event engine as parent, or NULL */
};

extern const struct Tutorial gTutorials[];  /* ends with a title of 0 */

ProcPtr StartTutorialsScreen(ProcPtr parent);

/* Source/TutorialMap.c: a tutorial's map, a game of its own that writes nothing to the save memory */
struct SaveMenuProc;
void TutorialMap_Launch(struct SaveMenuProc *menu, int index);
int TutorialMap_Played(void);      /* the tutorial whose map was played last */
bool TutorialMap_Active(void);     /* on a tutorial's map */
