#pragma once

#include "common-chax.h"

/*
 * Chapter results (CONFIG_DISTRICT_GUILD, Kernel/Wizardry/ChapterResults). When a story chapter is cleared, before the
 * awards and the save menu, a screen counts up the chapter's numbers (turns, units fielded and lost, foes defeated,
 * damage dealt and taken, funds), names its MVP and stamps a rank, S to D. Documentation/Features/ChapterResults.md.
 *
 * The counts are the chapter awards' (gChapterAwards, chapter-awards.h). The snapshot taken at the chapter's first
 * player phase (the funds and the fallen so far) goes in the spare bytes of its unit rows, so it is in the same
 * suspend-save chunk, with no change to the save layout.
 */

/* gChapterAwards.units[i].spare */
enum chapter_results_spare {
	CHAPTER_RESULTS_SPARE_FUNDS = 0,  /* 0-2: the funds at the first player phase, 24 bits, low byte first */
	CHAPTER_RESULTS_SPARE_DEAD = 3,   /* the player units dead then */
	CHAPTER_RESULTS_SPARE_MARK = 4,   /* CHAPTER_RESULTS_MARK once the snapshot is taken */
	CHAPTER_RESULTS_SPARE_FIELDED = 5, /* the most player units fielded at a player phase's start (fallen included) */
};

#define CHAPTER_RESULTS_MARK 0xC5

/* Shows the screen when it is on and due; it then goes on to the awards or save menu if save. FALSE: not shown. */
bool ChapterResults_Start(ProcPtr parent, bool save);

/* Both toggles: the designer config's chapter_results_screen and the Options row (gPlaySt.config2) */
bool ChapterResults_Enabled(void);

/* gPrePhaseFuncs: the snapshot at the chapter's first player phase, and the units fielded at each player phase */
bool ChapterResults_PrePhase(ProcPtr proc);

/* The rank of a chapter, 0 (S) to 4 (D): the formula in Documentation/Features/ChapterResults.md */
int ChapterResults_Rank(int turns, int lost, int defeated);
