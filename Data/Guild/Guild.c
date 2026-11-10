#include <common-chax.h>
#include "constants/items.h"
#include "constants/texts.h"
#include "constants/characters.h"
#include "jester_headers/maps.h"
#include "kernel/guild.h"
#include "kernel/chapter-awards.h"
#include "skill-system.h"
#include "constants/skills.h"

#ifdef CONFIG_DISTRICT_GUILD

/*
 * The guild board's quests and prizes (Kernel/Wizardry/Guild, include/kernel/guild.h).
 *
 * Quests: a board shows the first three of its district's quests (and of the ones with district 0) not yet claimed, in
 * this order, so put a district's easy ones first and the every-board ones last. At most GUILD_QUESTS_MAX (32): each has
 * a claimed bit in the save, by its place here, so add new quests at the end.
 *
 * Prizes: the order is the list's. One-time prizes have a bit each, by their rank among the one-time prizes: add new ones
 * after the last one-time prize.
 */

#define SERAFEW 0x3B
#define PORT_KIRIS 0x3C
#define CAER_PELYN 0x41
#define JEHANNA 0x42

#define QUEST(d, k, p, t, tokens, gold, item, m) \
	{ .district = (d), .kind = (k), .param = (p), .target = (t), .rewardTokens = (tokens), .rewardGold = (gold), \
	  .rewardItem = (item), .msg = (m) }

const struct GuildQuest gGuildQuests[] = {
	/* Serafew */
	QUEST(SERAFEW, GUILD_QUEST_CROSSING_SCORE, 0, 800, 15, 0, 0, MSG_GUILD_Q_SERAFEW_CROSSING),
	QUEST(SERAFEW, GUILD_QUEST_DELIVER, ITEM_VULNERARY, 2, 10, 300, 0, MSG_GUILD_Q_SERAFEW_VULNERARY),
	QUEST(SERAFEW, GUILD_QUEST_FISH_SPECIES, 0, 5, 15, 0, 0, MSG_GUILD_Q_SERAFEW_FISH),
	QUEST(SERAFEW, GUILD_QUEST_WINS, 0, 20, 20, 0, ITEM_ELIXIR, MSG_GUILD_Q_SERAFEW_WINS),
	QUEST(SERAFEW, GUILD_QUEST_TOWER_SCORE, 0, 100, 15, 0, 0, MSG_GUILD_Q_SERAFEW_TOWER),

	/* Port Kiris */
	QUEST(PORT_KIRIS, GUILD_QUEST_CROSSING_MEDAL, 1, 1, 15, 0, 0, MSG_GUILD_Q_KIRIS_MEDAL),
	QUEST(PORT_KIRIS, GUILD_QUEST_FISH_RANK, 0, 2, 20, 0, 0, MSG_GUILD_Q_KIRIS_ANGLER),
	QUEST(PORT_KIRIS, GUILD_QUEST_HEX_CARDS, 0, 10, 15, 0, 0, MSG_GUILD_Q_KIRIS_HEX),
	QUEST(PORT_KIRIS, GUILD_QUEST_DELIVER, ITEM_PUREWATER, 1, 15, 500, 0, MSG_GUILD_Q_KIRIS_WATER),
	QUEST(PORT_KIRIS, GUILD_QUEST_SNEK_SCORE, 0, 40, 15, 0, 0, MSG_GUILD_Q_KIRIS_SNEK),

	/* Caer Pelyn */
	QUEST(CAER_PELYN, GUILD_QUEST_CROSSING_SCORE, 2, 1500, 25, 0, 0, MSG_GUILD_Q_CAER_CROSSING),
	QUEST(CAER_PELYN, GUILD_QUEST_FISH_LEGENDS, 0, 1, 30, 0, ITEM_BOOSTER_SPD, MSG_GUILD_Q_CAER_LEGEND),
	QUEST(CAER_PELYN, GUILD_QUEST_WINS, 0, 60, 30, 0, 0, MSG_GUILD_Q_CAER_WINS),
	QUEST(CAER_PELYN, GUILD_QUEST_TOKENS_EARNED, 0, 200, 20, 1000, 0, MSG_GUILD_Q_CAER_TOKENS),

	/* Jehanna Hall */
	QUEST(JEHANNA, GUILD_QUEST_CROSSING_MEDAL, 3, 3, 50, 0, ITEM_MASTERSEAL, MSG_GUILD_Q_JEHANNA_GOLD),
	QUEST(JEHANNA, GUILD_QUEST_FISH_SPECIES, 0, 20, 50, 0, 0, MSG_GUILD_Q_JEHANNA_JOURNAL),
	QUEST(JEHANNA, GUILD_QUEST_HEX_CARDS, 0, 30, 30, 0, 0, MSG_GUILD_Q_JEHANNA_HEX),
	QUEST(JEHANNA, GUILD_QUEST_WINS, 0, 150, 50, 0, ITEM_BOOSTER_POW, MSG_GUILD_Q_JEHANNA_WINS),

	/* Every board */
	QUEST(0, GUILD_QUEST_TOWER_SCORE, 0, 300, 30, 0, 0, MSG_GUILD_Q_ANY_TOWER),
	QUEST(0, GUILD_QUEST_SNEK_SCORE, 0, 100, 30, 0, 0, MSG_GUILD_Q_ANY_SNEK),

	/* Story quests: each unlocks a biography's last page or support points (gGuildUnlocks below) */
	QUEST(SERAFEW, GUILD_QUEST_WINS, CHARACTER_EIRIKA, 25, 10, 0, 0, MSG_GUILD_Q_STORY_EIRIKA),
	QUEST(SERAFEW, GUILD_QUEST_WINS, CHARACTER_SETH, 25, 10, 0, 0, MSG_GUILD_Q_STORY_SETH),
	QUEST(PORT_KIRIS, GUILD_QUEST_WINS, CHARACTER_FRANZ, 20, 10, 0, 0, MSG_GUILD_Q_STORY_FRANZ),
	QUEST(PORT_KIRIS, GUILD_QUEST_WINS, CHARACTER_GILLIAM, 20, 10, 0, 0, MSG_GUILD_Q_STORY_GILLIAM),
	QUEST(CAER_PELYN, GUILD_QUEST_WINS, CHARACTER_TANA, 15, 10, 0, 0, MSG_GUILD_Q_STORY_TANA),
	QUEST(SERAFEW, GUILD_QUEST_TOKENS_EARNED, 0, 60, 5, 0, 0, MSG_GUILD_Q_SUPPORT_EIRIKA_SETH),
	QUEST(PORT_KIRIS, GUILD_QUEST_TOKENS_EARNED, 0, 120, 5, 0, 0, MSG_GUILD_Q_SUPPORT_FRANZ_GILLIAM),

	{ .kind = GUILD_QUEST_END },
};

#define UNLOCK_BIOGRAPHY(quest, pid, page) { .questMsg = (quest), .kind = GUILD_UNLOCK_BIOGRAPHY, .a = (pid), .b = (page) }
#define UNLOCK_SUPPORT(quest, pid, partner, points) \
	{ .questMsg = (quest), .kind = GUILD_UNLOCK_SUPPORT, .a = (pid), .b = (partner), .amount = (points) }

/*
 * Biography pages (Data/MapMenus/CharacterBiographies.c) behind a quest stay locked, with the pages after them, until
 * it is claimed. Support points go both ways, up to the next rank: its conversation then shows as usual.
 */
const struct GuildUnlock gGuildUnlocks[] = {
	UNLOCK_BIOGRAPHY(MSG_GUILD_Q_STORY_EIRIKA, CHARACTER_EIRIKA, 3),
	UNLOCK_BIOGRAPHY(MSG_GUILD_Q_STORY_SETH, CHARACTER_SETH, 3),
	UNLOCK_BIOGRAPHY(MSG_GUILD_Q_STORY_FRANZ, CHARACTER_FRANZ, 3),
	UNLOCK_BIOGRAPHY(MSG_GUILD_Q_STORY_GILLIAM, CHARACTER_GILLIAM, 3),
	UNLOCK_BIOGRAPHY(MSG_GUILD_Q_STORY_TANA, CHARACTER_TANA, 3),
	UNLOCK_SUPPORT(MSG_GUILD_Q_SUPPORT_EIRIKA_SETH, CHARACTER_EIRIKA, CHARACTER_SETH, 40),
	UNLOCK_SUPPORT(MSG_GUILD_Q_SUPPORT_FRANZ_GILLIAM, CHARACTER_FRANZ, CHARACTER_GILLIAM, 40),
	{ .kind = GUILD_UNLOCK_END },
};

#define FEATURED(story, board, quest, bonus) \
	{ .storyChapter = (story), .district = (board), .questMsg = (quest), .bonusTokens = (bonus) }

/*
 * Each story chapter, a board can feature one quest: it tops that board, shows in gold with "Featured!", and pays the
 * bonus on top of its reward when claimed there before the story moves on. The chapter is gWmStoryChapter, the one the
 * world map leads to next; the boards open with Chapter 6 (Serafew), 8 (Caer Pelyn), 9 (Jehanna Hall) and 10 (Port
 * Kiris).
 */
const struct GuildFeatured gGuildFeatured[] = {
	FEATURED(CHAPTER_06, SERAFEW, MSG_GUILD_Q_SERAFEW_CROSSING, 15),
	FEATURED(CHAPTER_07, SERAFEW, MSG_GUILD_Q_SERAFEW_FISH, 15),
	FEATURED(CHAPTER_08, SERAFEW, MSG_GUILD_Q_SERAFEW_TOWER, 15),
	FEATURED(CHAPTER_08, CAER_PELYN, MSG_GUILD_Q_CAER_CROSSING, 20),
	FEATURED(CHAPTER_09, CAER_PELYN, MSG_GUILD_Q_CAER_LEGEND, 25),
	FEATURED(CHAPTER_09, JEHANNA, MSG_GUILD_Q_JEHANNA_HEX, 20),
	FEATURED(CHAPTER_10, PORT_KIRIS, MSG_GUILD_Q_KIRIS_SNEK, 15),
	FEATURED(CHAPTER_10, JEHANNA, MSG_GUILD_Q_JEHANNA_GOLD, 30),
	FEATURED(CHAPTER_10, SERAFEW, MSG_GUILD_Q_ANY_TOWER, 20),
	{ .storyChapter = GUILD_FEATURED_END },
};

#define PRIZE(i, c)        { .item = (i), .cost = (c), .kind = GUILD_PRIZE_ITEM }
#define PRIZE_ONCE(i, c)   { .item = (i), .cost = (c), .kind = GUILD_PRIZE_ITEM, .once = 1 }
#define TONIC(k, c)        { .item = ((k) << 8) | ITEM_TONIC, .cost = (c), .kind = GUILD_PRIZE_WORD }
#define SCROLL(sid, c)     { .item = (sid), .cost = (c), .kind = GUILD_PRIZE_SKILL, .once = 1 }

/*
 * One-time prizes keep their bought bit by their order among the one-time prizes (struct GuildSave): add new ones after
 * the last one-time prize, never between. Rows that are not one-time can go anywhere.
 */
const struct GuildPrize gGuildPrizes[] = {
	PRIZE(ITEM_VULNERARY, 5),
	PRIZE(ITEM_ANTITOXIN, 5),
	PRIZE(ITEM_TORCH, 5),
	PRIZE(ITEM_DOORKEY, 10),
	PRIZE(ITEM_PUREWATER, 15),
	PRIZE(ITEM_ELIXIR, 25),
	PRIZE(ITEM_MASTERSEAL, 60),
	PRIZE(ITEM_RARE_CANDY, 100), /* +100 EXP: nothing else in the campaign gives it */

	/* Tonics: +2 to a stat (Omni: to every stat) until the chapter ends (Documentation/Features/Tonics.md) */
	TONIC(0, 8),  /* HP */
	TONIC(1, 8),  /* Str */
	TONIC(2, 8),  /* Mag */
	TONIC(3, 8),  /* Skl */
	TONIC(4, 8),  /* Spd */
	TONIC(5, 8),  /* Lck */
	TONIC(6, 8),  /* Def */
	TONIC(7, 8),  /* Res */
	TONIC(8, 30), /* Omni */

	PRIZE_ONCE(ITEM_BOOSTER_HP, 80),
	PRIZE_ONCE(ITEM_BOOSTER_POW, 80),
	PRIZE_ONCE(ITEM_BOOSTER_SKL, 80),
	PRIZE_ONCE(ITEM_BOOSTER_SPD, 80),
	PRIZE_ONCE(ITEM_BOOSTER_LCK, 80),
	PRIZE_ONCE(ITEM_BOOSTER_DEF, 80),
	PRIZE_ONCE(ITEM_BOOSTER_RES, 80),

	/* Skill scrolls, once each */
#if defined(SID_Vantage) && (COMMON_SKILL_VALID(SID_Vantage))
	SCROLL(SID_Vantage, 120),
#endif
#if defined(SID_Renewal) && (COMMON_SKILL_VALID(SID_Renewal))
	SCROLL(SID_Renewal, 120),
#endif
#if defined(SID_Adept) && (COMMON_SKILL_VALID(SID_Adept))
	SCROLL(SID_Adept, 150),
#endif
#if defined(SID_Pavise) && (COMMON_SKILL_VALID(SID_Pavise))
	SCROLL(SID_Pavise, 150),
#endif
#if defined(SID_Luna) && (COMMON_SKILL_VALID(SID_Luna))
	SCROLL(SID_Luna, 150),
#endif
#if defined(SID_Galeforce) && (COMMON_SKILL_VALID(SID_Galeforce))
	SCROLL(SID_Galeforce, 250),
#endif
	{ .item = 0 },
};

/* Chapter awards (include/kernel/chapter-awards.h): each award's name and its tokens, in CHAPTER_AWARD_* order. */
const struct ChapterAwardType gChapterAwardTypes[CHAPTER_AWARD_COUNT] = {
	[CHAPTER_AWARD_MVP] = { MSG_AWARDS_MVP, 10 },
	[CHAPTER_AWARD_KILLS] = { MSG_AWARDS_KILLS, 5 },
	[CHAPTER_AWARD_UNSCATHED] = { MSG_AWARDS_UNSCATHED, 5 },
};

#endif /* CONFIG_DISTRICT_GUILD */
