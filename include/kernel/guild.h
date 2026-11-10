#pragma once

#include "common-chax.h"

/*
 * The district guild board (CONFIG_DISTRICT_GUILD, Kernel/Wizardry/Guild). Each district has a board on its map, a
 * TRAP_GUILD_BOARD trap (GUILD_BOARD(x, y) in its traps.c). Pressing A on it opens a screen with three pages:
 *
 *   Quests   the district's requests (gGuildQuests, Data/Guild/Guild.c). A quest's progress is read from what the game
 *            already saves: the minigames' records, the battle records, the items held. A met quest is claimed here
 *            for its reward: tokens, gold and an item.
 *   Prizes   items for tokens (gGuildPrizes).
 *   Records  every minigame's bests on one page.
 *
 * Tokens are the minigames' shared currency: each minigame pays some at its end (Guild_AddTokens), and so do quests.
 */

#define GUILD_SAVE_MAGIC 0x4447 /* "GD" */
#define GUILD_QUESTS_MAX 32     /* a claimed bit each in struct GuildSave */
#define GUILD_PRIZES_ONCE_MAX 16
#define GUILD_TOKENS_MAX 9999

enum guild_quest_kind {
	GUILD_QUEST_END,
	GUILD_QUEST_CROSSING_SCORE, /* param: the course (0 Serafew .. 3 Jehanna Hall); target: a best score */
	GUILD_QUEST_CROSSING_MEDAL, /* param: the course; target: 1 bronze, 2 silver, 3 gold */
	GUILD_QUEST_TOWER_SCORE,    /* target: Tower Stack's best score */
	GUILD_QUEST_SNEK_SCORE,     /* target: Snek's high score */
	GUILD_QUEST_FISH_SPECIES,   /* target: species in the fish journal */
	GUILD_QUEST_FISH_LEGENDS,   /* target: legends caught */
	GUILD_QUEST_FISH_RANK,      /* target: the angler rank (FISHING_RANK_*) */
	GUILD_QUEST_HEX_CARDS,      /* target: cards in the Hex binder */
	GUILD_QUEST_WINS,           /* param: a character, or 0 for the whole army; target: battles won */
	GUILD_QUEST_DELIVER,        /* param: an item; target: how many. They are handed over when the quest is claimed */
	GUILD_QUEST_TOKENS_EARNED,  /* target: tokens earned in all */
};

/* A quest (gGuildQuests, ends with kind GUILD_QUEST_END; GUILD_QUESTS_MAX at most). */
struct GuildQuest {
	u8 district;     /* the district chapter whose board posts it; 0: every board */
	u8 kind;         /* GUILD_QUEST_* */
	u8 param;
	u8 rewardItem;   /* 0: none */
	u16 target;
	u16 rewardTokens;
	u16 rewardGold;
	u16 msg;         /* its line on the board */
};

enum guild_prize_kind {
	GUILD_PRIZE_ITEM,  /* item: an item index, given with full uses */
	GUILD_PRIZE_WORD,  /* item: an item word, given as-is (tonics: (kind << 8) | ITEM_TONIC) */
	GUILD_PRIZE_SKILL, /* item: a skill id, given as its skill scroll (MakeSkillScrollItem) */
};

/* A prize (gGuildPrizes, ends with item 0). */
struct GuildPrize {
	u16 item;        /* GUILD_PRIZE_*: an item index, an item word or a skill id */
	u16 cost;        /* tokens */
	u8 kind;         /* GUILD_PRIZE_* */
	u8 once;         /* 1: it can be bought once (a bit in struct GuildSave, in the order of the one-time prizes) */
	u8 pad[2];
};

/* Saved (a save chunk in both save lists, 0x10 bytes). */
struct GuildSave {
	/* 00 */ u16 magic;
	/* 02 */ u16 tokens;
	/* 04 */ u16 earned;      /* tokens earned in all, up to 65535 */
	/* 06 */ u16 prizes;      /* a bit per one-time prize bought */
	/* 08 */ u32 claimed;     /* a bit per quest claimed (gGuildQuests order) */
	/* 0C */ u16 pad[2];
};

extern struct GuildSave gGuildSave;
extern const struct GuildQuest gGuildQuests[];
extern const struct GuildPrize gGuildPrizes[];

/* Tokens */
int Guild_Tokens(void);
void Guild_AddTokens(int n);
bool Guild_SpendTokens(int n);

/* Quests: progress toward the target, met (and not claimed), claimed */
int Guild_QuestProgress(const struct GuildQuest *q);
bool Guild_QuestMet(int index);
bool Guild_QuestClaimed(int index);
bool Guild_ClaimQuest(int index);    /* the reward, and the quest is done; FALSE when not met */
int Guild_QuestsClaimedCount(void);
int Guild_QuestsCount(void);
bool Guild_AnyClaimable(int district);

/* Prizes */
bool Guild_PrizeSoldOut(int index);
bool Guild_BuyPrize(int index);      /* FALSE when short of tokens or sold out */
int Guild_PrizeItem(const struct GuildPrize *p); /* the item word it gives */

/* Items to the unit, or else the convoy */
void Guild_GiveItem(int item);
void Guild_GiveItemWord(int item);
int Guild_CountItem(int item);       /* held by the army's units and in the convoy */

/* Save chunk and new-game hook (Kernel/Wizardry/SaveData/data.event) */
void Guild_NewGame(void);
void Guild_Save(u8 *dst, const u32 size);
void Guild_Load(u8 *src, const u32 size);

/* The board on the district map: the A-press handler (FreeMovement/cFreeMU/HookList.event), and its marker */
void Guild_OnPressA(ProcPtr proc);
void StartDistrictGuildASMC(ProcPtr parent);
void Guild_LoadMapMarker(int chr);
void Guild_DrawMapMarkers(int chr, int pal);

extern const struct ProcCmd ProcScr_GuildScreen[];
