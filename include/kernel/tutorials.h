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

/* TUTORIAL_MAP's chapter flag (a free temporary one) that keeps its "defeat every enemy" event from starting */
#define TUTMAP_FLAG_NO_WIN 7
/* Free Movement's village (visited, or not Free Movement: set) and its talk with Garcia (once) */
#define TUTMAP_FLAG_VILLAGE 8
#define TUTMAP_FLAG_GARCIA  9
/* Skill Capacity's talk with Saleh, who teaches Eirika Luna (once) */
#define TUTMAP_FLAG_SALEH   10

struct Tutorial {
	u16 title;          /* the title's message (Districts.txt) */
	u16 text;           /* the box's summary: four lines at most (the fifth is "A: Try it") */
	const u8 *img;      /* the title in the Extras labels' letters (Source/TutorialTitles.c, generated) */
	u16 intro;          /* the explanation shown on its map, before the player tries it */
	u8 chapter;         /* its map: TUTORIAL_MAP, or a district */
	const struct UnitDefinition *units; /* loaded on TUTORIAL_MAP (blue and red), ending with charIndex 0 */
	void (*setup)(void); /* anything else the map needs (an option on, fog, a skill, a dead unit...), or NULL */
	void (*after)(ProcPtr event); /* run after the explanation, with the event engine as parent, or NULL */
	const void *guide;  /* its guided steps' first event (Source/TutorialGuides.c), from the player phase's start, or NULL */
};

extern const struct Tutorial gTutorials[];  /* ends with a title of 0 */

ProcPtr StartTutorialsScreen(ProcPtr parent);
bool TutorialsScreen_Exists(void);

/* Source/TutorialMap.c: a tutorial's map, a game of its own that writes nothing to the save memory */
struct SaveMenuProc;
void TutorialMap_Launch(struct SaveMenuProc *menu, int index);
int TutorialMap_Played(void);      /* the tutorial whose map was played last */
bool TutorialMap_Active(void);     /* on a tutorial's map */

/* Source/TutorialGuides.c: the guided steps' state cleared (every key and unit menu command back) */
void TutGuide_Reset(void);
bool TutorialMap_Leaving(void);     /* the map is ending (TutorialMap_Leave) */
ProcPtr TutorialMap_IdlePhase(void); /* the player phase (or free movement) waiting for input, no event running */

/* Source/TutorialsData.c: the tutorials shown in this game (MP System's or Gaiden Magic's, by the magic in use) */
int Tutorials_Count(void);          /* how many are shown */
int Tutorials_Index(int shown);     /* gTutorials' index of the shown one */
int Tutorials_Shown(int index);     /* the shown place of gTutorials[index], or -1 */
