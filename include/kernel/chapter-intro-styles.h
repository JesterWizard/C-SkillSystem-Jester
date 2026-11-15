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
void ChapterIntro_SetStyle(int style);
void ChapterIntro_SetBg(int bg);
int ChapterIntro_DefaultOption(void); /* the designer config's style and background, for a new game */
