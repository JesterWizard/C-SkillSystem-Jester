#pragma once

#include "common-chax.h"

/*
 * Chapter intro styles and the background behind their title (Kernel/Wizardry/ChapterIntroStyles), picked on the
 * Options screen's "Chapter Intro" and "Intro Backdrop" rows: gPlaySt.config.chapter_intro holds style + 5 * background.
 */
int ChapterIntro_StyleCount(void);
int ChapterIntro_BgCount(void);
const char *ChapterIntro_StyleName(int style);
const char *ChapterIntro_StyleDesc(int style);
const char *ChapterIntro_BgName(int bg);
const char *ChapterIntro_BgDesc(int bg);
int ChapterIntro_GetStyle(void);
int ChapterIntro_GetBg(void);
bool ChapterIntro_GetBand(void); /* the see-through black bar behind the title on the Black and Last Scene backdrops */
void ChapterIntro_SetStyle(int style);
void ChapterIntro_SetBg(int bg);
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
