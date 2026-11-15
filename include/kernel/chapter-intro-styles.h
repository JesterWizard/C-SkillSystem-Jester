#pragma once

#include "common-chax.h"

/*
 * Chapter intro styles (Kernel/Wizardry/ChapterIntroStyles), picked on the Options screen's "Chapter Intro" row. The
 * background behind the title and the title band come from the designer config. gPlaySt.config.chapter_intro holds
 * style + 5 * background.
 */
int ChapterIntro_StyleCount(void);
const char *ChapterIntro_StyleName(int style);
const char *ChapterIntro_StyleDesc(int style);
int ChapterIntro_GetStyle(void);
int ChapterIntro_GetBg(void);
bool ChapterIntro_GetBand(void); /* the designer config's chapter_intro_band: the see-through black bar behind the title on the Black and Last Scene backdrops */
void ChapterIntro_SetStyle(int style);
int ChapterIntro_DefaultOption(void); /* the designer config's style and background, for a new game */

/* In RAM and in the tail of every game save and of the suspend save (SaveTail.c): 4 bytes */
struct ChapterIntroExtra {
	/* 00 */ u16 magic;
	/* 02 */ u8 band;  /* the see-through black bar behind the title on the Black and Last Scene backdrops */
	/* 03 */ u8 pad;
};

#define CHAPTER_INTRO_EXTRA_MAGIC 0x4943 /* "CI" */

extern struct ChapterIntroExtra gChapterIntroExtra;

void ChapterIntro_SetBand(bool on);
void ChapterIntro_ResetExtra(void); /* a new game: the designer config's defaults */
void ChapterIntro_SaveTail(u8 *dst, const u32 size);
void ChapterIntro_LoadTail(u8 *src, const u32 size);
