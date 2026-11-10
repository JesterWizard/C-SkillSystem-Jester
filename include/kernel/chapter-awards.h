#pragma once

#include "common-chax.h"

/*
 * Chapter awards (CONFIG_DISTRICT_GUILD, Kernel/Wizardry/Guild/Source/ChapterAwards.c). Through a story chapter, every
 * battle a player unit fights against an enemy is counted: foes defeated, damage dealt and damage taken. When the
 * chapter is cleared, a screen before the save menu names three units and pays each award's guild tokens:
 *
 *   MVP           the most damage dealt, plus 10 per foe defeated
 *   Most defeated the most foes defeated
 *   Unscathed     the least damage taken, among the units that fought (more battles wins a tie)
 *
 * A unit wins nothing it has no claim to (no damage, no foe, no battle), and dead units win nothing. The awards and
 * their tokens are gChapterAwardTypes (Data/Guild/Guild.c). Districts give no awards.
 */

#define CHAPTER_AWARDS_MAGIC 0x4341 /* "AC" */
#define CHAPTER_AWARDS_UNITS 12
#define CHAPTER_AWARDS_KILL_SCORE 10 /* MVP points per foe defeated */

enum chapter_award_kind {
	CHAPTER_AWARD_MVP,
	CHAPTER_AWARD_KILLS,
	CHAPTER_AWARD_UNSCATHED,
	CHAPTER_AWARD_COUNT,
};

/* An award: its name, and the tokens it pays (gChapterAwardTypes, CHAPTER_AWARD_COUNT rows) */
struct ChapterAwardType {
	u16 msg;
	u16 tokens;
};

struct ChapterAwardsUnit {
	/* 00 */ u8 pid;
	/* 01 */ u8 kills;
	/* 02 */ u8 battles;
	/* 03 */ u8 pad;
	/* 04 */ u16 dealt;
	/* 06 */ u16 taken;
};

/* This chapter's counts (a suspend-save chunk, 0x64 bytes; reset when a chapter starts) */
struct ChapterAwardsData {
	/* 00 */ u16 magic;
	/* 02 */ u8 chapter;
	/* 03 */ u8 count;
	/* 04 */ struct ChapterAwardsUnit units[CHAPTER_AWARDS_UNITS];
};

extern struct ChapterAwardsData gChapterAwards;
extern const struct ChapterAwardType gChapterAwardTypes[CHAPTER_AWARD_COUNT];

/* The winner of an award: an index in gChapterAwards.units, or -1 */
int ChapterAwards_Winner(int kind);

/* Hooks: ChapterInitHooks, gExternalBattleToUnitHook, the suspend-save chunk */
void ChapterInit_ResetChapterAwards(ProcPtr proc);
int ChapterAwards_OnBattleToUnit(struct BattleUnit *bu, struct Unit *unit);
void ChapterAwards_Save(u8 *dst, const u32 size);
void ChapterAwards_Load(u8 *src, const u32 size);

extern const struct ProcCmd ProcScr_ChapterAwards[];
