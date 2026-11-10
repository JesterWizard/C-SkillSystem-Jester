#include <common-chax.h>
#include "constants/items.h"
#include "constants/texts.h"
#include "kernel/guild.h"

#ifdef CONFIG_DISTRICT_GUILD

/*
 * The guild board's quests and prizes (Kernel/Wizardry/Guild, include/kernel/guild.h).
 *
 * Quests: a board shows the first three of its district's quests (and of the ones with district 0) not yet claimed, in
 * this order, so put a district's easy ones first and the every-board ones last. At most GUILD_QUESTS_MAX (32): each has
 * a claimed bit in the save, by its place here, so add new quests at the end.
 *
 * Prizes: the order is the list's. One-time prizes have a bit each, by their rank among the one-time prizes: add new ones
 * at the end too.
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

	{ .kind = GUILD_QUEST_END },
};

const struct GuildPrize gGuildPrizes[] = {
	{ .item = ITEM_VULNERARY, .cost = 5 },
	{ .item = ITEM_ANTITOXIN, .cost = 5 },
	{ .item = ITEM_TORCH, .cost = 5 },
	{ .item = ITEM_DOORKEY, .cost = 10 },
	{ .item = ITEM_PUREWATER, .cost = 15 },
	{ .item = ITEM_ELIXIR, .cost = 25 },
	{ .item = ITEM_MASTERSEAL, .cost = 60 },
	{ .item = ITEM_BOOSTER_HP, .cost = 80, .once = 1 },
	{ .item = ITEM_BOOSTER_POW, .cost = 80, .once = 1 },
	{ .item = ITEM_BOOSTER_SKL, .cost = 80, .once = 1 },
	{ .item = ITEM_BOOSTER_SPD, .cost = 80, .once = 1 },
	{ .item = ITEM_BOOSTER_LCK, .cost = 80, .once = 1 },
	{ .item = ITEM_BOOSTER_DEF, .cost = 80, .once = 1 },
	{ .item = ITEM_BOOSTER_RES, .cost = 80, .once = 1 },
	{ .item = 0 },
};

#endif /* CONFIG_DISTRICT_GUILD */
