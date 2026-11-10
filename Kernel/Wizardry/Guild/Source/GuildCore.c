#include "common-chax.h"
#include "kernel-lib.h"
#include "bwl.h"
#include "bmcontainer.h"
#include "guild.h"
#include "crossing.h"
#include "tower.h"
#include "fishing.h"
#include "hex.h"
#include "skill-system.h"
#include "vanilla.h"
#include "wm-story-chapter.h"

#ifdef CONFIG_DISTRICT_GUILD

/*
 * The guild board's rules: tokens, quests and prizes, and the save chunk (gGuildSave, 0x10 bytes in both save lists,
 * Kernel/Wizardry/SaveData/data.event). A quest's progress is read from what the game already saves, so nothing here
 * has to watch the minigames or the battles: only the tokens and the claimed and bought bits are kept.
 */

typedef char GuildSave_SizeCheck[(sizeof(struct GuildSave) == 0x10) ? 1 : -1];
typedef char GuildQuest_SizeCheck[(sizeof(struct GuildQuest) == 0x0C) ? 1 : -1];

extern u16 gSnekHighScore;

static void Guild_Clear(void)
{
	CpuFill16(0, &gGuildSave, sizeof(gGuildSave));
	gGuildSave.magic = GUILD_SAVE_MAGIC;
}

static void Guild_Check(void)
{
	if (gGuildSave.magic != GUILD_SAVE_MAGIC)
		Guild_Clear();
}

/* ---- Tokens ---- */

int Guild_Tokens(void)
{
	Guild_Check();
	return gGuildSave.tokens;
}

void Guild_AddTokens(int n)
{
	int v;

	if (n <= 0)
		return;
	Guild_Check();
	v = gGuildSave.tokens + n;
	gGuildSave.tokens = v > GUILD_TOKENS_MAX ? GUILD_TOKENS_MAX : v;
	v = gGuildSave.earned + n;
	gGuildSave.earned = v > 0xFFFF ? 0xFFFF : v;
}

bool Guild_SpendTokens(int n)
{
	Guild_Check();
	if (n > gGuildSave.tokens)
		return FALSE;
	gGuildSave.tokens -= n;
	return TRUE;
}

/* ---- Items ---- */

static struct Unit *Guild_Receiver(void)
{
	int i;

	if (UNIT_IS_VALID(gActiveUnit) && !(gActiveUnit->state & US_DEAD))
		return gActiveUnit;
	for (i = 1; i < 0x40; i++) {
		struct Unit *unit = GetUnit(i);

		if (UNIT_IS_VALID(unit) && !(unit->state & US_DEAD))
			return unit;
	}
	return NULL;
}

void Guild_GiveItemWord(int item)
{
	struct Unit *unit = Guild_Receiver();

	if (unit && UnitAddItem(unit, item))
		return;
	AddItemToConvoy(item);
}

void Guild_GiveItem(int item)
{
	Guild_GiveItemWord(MakeNewItem(item));
}

int Guild_CountItem(int item)
{
	u16 *convoy = GetConvoyItemArray();
	int i, slot, n = 0, count = GetConvoyItemCount();

	for (i = 0; i < count; i++)
		if (ITEM_INDEX(convoy[i]) == item)
			n++;
	for (i = 1; i < 0x40; i++) {
		struct Unit *unit = GetUnit(i);

		if (!UNIT_IS_VALID(unit) || (unit->state & US_DEAD))
			continue;
		for (slot = 0; slot < UNIT_ITEM_COUNT; slot++)
			if (ITEM_INDEX(unit->items[slot]) == item)
				n++;
	}
	return n;
}

/* Takes n copies of an item: from the convoy first, then from the units */
static void Guild_TakeItems(int item, int n)
{
	u16 *convoy = GetConvoyItemArray();
	int i, slot;

	for (i = GetConvoyItemCount() - 1; i >= 0 && n > 0; i--) {
		if (ITEM_INDEX(convoy[i]) == item) {
			RemoveItemFromConvoy(i);
			n--;
		}
	}
	for (i = 1; i < 0x40 && n > 0; i++) {
		struct Unit *unit = GetUnit(i);

		if (!UNIT_IS_VALID(unit) || (unit->state & US_DEAD))
			continue;
		for (slot = UNIT_ITEM_COUNT - 1; slot >= 0 && n > 0; slot--) {
			if (ITEM_INDEX(unit->items[slot]) == item) {
				UnitRemoveItem(unit, slot);
				n--;
			}
		}
	}
}

/* ---- Quests ---- */

static int Guild_Wins(int pid)
{
	struct NewBwl *bwl;
	int i, n = 0;

	if (pid != 0) {
		bwl = GetNewBwl(pid);
		return bwl ? bwl->winAmt : 0;
	}
	for (i = 1; i < NEW_BWL_ARRAY_NUM; i++) {
		bwl = GetNewBwl(i);
		if (bwl)
			n += bwl->winAmt;
	}
	return n;
}

static int Guild_BitCount(u32 v)
{
	int n = 0;

	while (v) {
		n += v & 1;
		v >>= 1;
	}
	return n;
}

int Guild_QuestProgress(const struct GuildQuest *q)
{
	int i, n;

	switch (q->kind) {
#ifdef CONFIG_DISTRICT_CROSSING
	case GUILD_QUEST_CROSSING_SCORE:
		return CrossingRecords_Best(q->param);
	case GUILD_QUEST_CROSSING_MEDAL:
		return CrossingRecords_Medal(q->param);
#endif
#ifdef CONFIG_DISTRICT_TOWER
	case GUILD_QUEST_TOWER_SCORE:
		return gTowerRecords.best;
#endif
	case GUILD_QUEST_SNEK_SCORE:
		return gSnekHighScore;
#ifdef CONFIG_DISTRICT_FISHING
	case GUILD_QUEST_FISH_SPECIES:
		for (i = 0, n = 0; i < FISHING_JOURNAL_SPECIES; i++)
			if (FishingRecords_Caught(i))
				n++;
		return n;
	case GUILD_QUEST_FISH_LEGENDS:
		return Guild_BitCount(gFishingProgress.legends);
	case GUILD_QUEST_FISH_RANK:
		return FishingProgress_Rank();
#endif
#ifdef CONFIG_HEX_CARDS
	case GUILD_QUEST_HEX_CARDS:
		return HexBinder_Total();
#endif
	case GUILD_QUEST_WINS:
		return Guild_Wins(q->param);
	case GUILD_QUEST_DELIVER:
		return Guild_CountItem(q->param);
	case GUILD_QUEST_TOKENS_EARNED:
		Guild_Check();
		return gGuildSave.earned;
	default:
		return 0;
	}
}

int Guild_QuestsCount(void)
{
	int n = 0;

	while (n < GUILD_QUESTS_MAX && gGuildQuests[n].kind != GUILD_QUEST_END)
		n++;
	return n;
}

bool Guild_QuestClaimed(int index)
{
	Guild_Check();
	return (gGuildSave.claimed >> index) & 1;
}

bool Guild_QuestMet(int index)
{
	const struct GuildQuest *q = &gGuildQuests[index];

	return !Guild_QuestClaimed(index) && Guild_QuestProgress(q) >= q->target;
}

/* ---- Unlocks ---- */

static int Guild_QuestIndexByMsg(int msg)
{
	int i, n = Guild_QuestsCount();

	for (i = 0; i < n; i++)
		if (gGuildQuests[i].msg == msg)
			return i;
	return -1;
}

int Guild_QuestUnlockKind(int index)
{
	const struct GuildUnlock *u;

	for (u = gGuildUnlocks; u->kind != GUILD_UNLOCK_END; u++)
		if (u->questMsg == gGuildQuests[index].msg)
			return u->kind;
	return GUILD_UNLOCK_END;
}

bool Guild_BiographyPageUnlocked(int pid, int page)
{
	const struct GuildUnlock *u;
	int index;

	for (u = gGuildUnlocks; u->kind != GUILD_UNLOCK_END; u++) {
		if (u->kind != GUILD_UNLOCK_BIOGRAPHY || u->a != pid || u->b > page)
			continue;
		index = Guild_QuestIndexByMsg(u->questMsg);
		if (index < 0 || !Guild_QuestClaimed(index))
			return FALSE;
	}
	return TRUE;
}

/*
 * Support points from pid to partner, up to the next rank (its conversation then shows as usual). The partner's slot
 * comes from the character data, so it works whether or not either unit is on the map (in a district only Eirika is).
 */
static void Guild_AddSupportOneWay(int pid, int partner, int amount)
{
	const struct CharacterData *data = GetCharacterData(pid);
	const struct SupportData *support = data ? data->pSupportData : NULL;
	struct NewBwl *bwl = GetNewBwl(pid);
	int num, exp, level, max;

	if (!support || !bwl)
		return;
	for (num = 0; num < support->supportCount && num < UNIT_SUPPORT_MAX_COUNT; num++)
		if (support->characters[num] == partner)
			break;
	if (num >= support->supportCount || num >= UNIT_SUPPORT_MAX_COUNT)
		return;
	exp = bwl->supports[num];
	level = exp > 240 ? SUPPORT_LEVEL_A : exp > 160 ? SUPPORT_LEVEL_B : exp > 80 ? SUPPORT_LEVEL_C : SUPPORT_LEVEL_NONE;
	max = sSupportMaxExpLookup[level];
	exp += amount;
	bwl->supports[num] = exp > max ? max : exp;
}

static void Guild_ApplyUnlocks(int index)
{
	const struct GuildUnlock *u;

	for (u = gGuildUnlocks; u->kind != GUILD_UNLOCK_END; u++) {
		if (u->questMsg != gGuildQuests[index].msg || u->kind != GUILD_UNLOCK_SUPPORT)
			continue;
		Guild_AddSupportOneWay(u->a, u->b, u->amount);
		Guild_AddSupportOneWay(u->b, u->a, u->amount);
	}
}

/* ---- Featured quests ---- */

static const struct GuildFeatured *Guild_FeaturedRow(int district)
{
	const struct GuildFeatured *f;

	for (f = gGuildFeatured; f->storyChapter != GUILD_FEATURED_END; f++)
		if (f->storyChapter == gWmStoryChapter && f->district == district)
			return f;
	return NULL;
}

int Guild_FeaturedQuest(int district)
{
	const struct GuildFeatured *f = Guild_FeaturedRow(district);
	int index;

	if (!f)
		return -1;
	index = Guild_QuestIndexByMsg(f->questMsg);
	if (index < 0 || Guild_QuestClaimed(index))
		return -1;
	return index;
}

int Guild_FeaturedBonus(int index)
{
	const struct GuildFeatured *f = Guild_FeaturedRow(gPlaySt.chapterIndex);

	if (!f || f->questMsg != gGuildQuests[index].msg)
		return 0;
	return f->bonusTokens;
}

/* ---- The rival ---- */

bool Guild_RivalActive(int index)
{
	return WmStoryReached(gGuildRival[index].storyChapter);
}

int Guild_RivalCurrent(int kind, int param)
{
	int i, current = -1;

	for (i = 0; gGuildRival[i].kind != GUILD_QUEST_END && i < GUILD_RIVAL_MAX; i++)
		if (gGuildRival[i].kind == kind && gGuildRival[i].param == param && Guild_RivalActive(i))
			current = i;
	return current;
}

bool Guild_RivalBeaten(int index)
{
	Guild_Check();
	return (gGuildSave.rival >> index) & 1;
}

int Guild_RivalPay(void)
{
	const struct GuildRivalRecord *r;
	struct GuildQuest q;
	int i, paid = 0;

	Guild_Check();
	for (i = 0; gGuildRival[i].kind != GUILD_QUEST_END && i < GUILD_RIVAL_MAX; i++) {
		r = &gGuildRival[i];
		if (Guild_RivalBeaten(i) || !Guild_RivalActive(i))
			continue;
		CpuFill16(0, &q, sizeof(q));
		q.kind = r->kind;
		q.param = r->param;
		if (Guild_QuestProgress(&q) < r->score)
			continue;
		gGuildSave.rival |= 1 << i;
		Guild_AddTokens(r->rewardTokens);
		paid += r->rewardTokens;
	}
	return paid;
}

bool Guild_ClaimQuest(int index)
{
	const struct GuildQuest *q = &gGuildQuests[index];
	u32 gold;

	if (index < 0 || index >= Guild_QuestsCount() || !Guild_QuestMet(index))
		return FALSE;
	if (q->kind == GUILD_QUEST_DELIVER)
		Guild_TakeItems(q->param, q->target);
	Guild_AddTokens(q->rewardTokens + Guild_FeaturedBonus(index));
	gGuildSave.claimed |= 1 << index;
	Guild_ApplyUnlocks(index);
	if (q->rewardGold) {
		gold = GetPartyGoldAmount() + q->rewardGold;
		SetPartyGoldAmount(gold > 999999 ? 999999 : gold);
	}
	if (q->rewardItem)
		Guild_GiveItem(q->rewardItem);
	return TRUE;
}

int Guild_QuestsClaimedCount(void)
{
	Guild_Check();
	return Guild_BitCount(gGuildSave.claimed);
}

/* A quest on this board is met and waiting: the marker on the map shows it */
bool Guild_AnyClaimable(int district)
{
	int i, n = Guild_QuestsCount();

	for (i = 0; i < n; i++)
		if ((gGuildQuests[i].district == 0 || gGuildQuests[i].district == district) && Guild_QuestMet(i))
			return TRUE;
	return FALSE;
}

/* ---- Prizes ---- */

/* The one-time bit of a prize: its rank among the one-time prizes */
static int Guild_PrizeBit(int index)
{
	int i, bit = 0;

	for (i = 0; i < index; i++)
		if (gGuildPrizes[i].once)
			bit++;
	return bit;
}

bool Guild_PrizeSoldOut(int index)
{
	int bit;

	if (!gGuildPrizes[index].once)
		return FALSE;
	bit = Guild_PrizeBit(index);
	Guild_Check();
	return bit < GUILD_PRIZES_ONCE_MAX && ((gGuildSave.prizes >> bit) & 1);
}

int Guild_PrizeItem(const struct GuildPrize *p)
{
	switch (p->kind) {
	case GUILD_PRIZE_WORD:
		return p->item;
	case GUILD_PRIZE_SKILL:
		return MakeSkillScrollItem(p->item);
	default:
		return MakeNewItem(p->item);
	}
}

bool Guild_BuyPrize(int index)
{
	const struct GuildPrize *p = &gGuildPrizes[index];
	int bit;

	if (Guild_PrizeSoldOut(index) || !Guild_SpendTokens(p->cost))
		return FALSE;
	if (p->once) {
		bit = Guild_PrizeBit(index);
		if (bit < GUILD_PRIZES_ONCE_MAX)
			gGuildSave.prizes |= 1 << bit;
	}
	Guild_GiveItemWord(Guild_PrizeItem(p));
	return TRUE;
}

/* ---- Save chunk ---- */

/* New-game hook (gNewSaveHooks) */
void Guild_NewGame(void)
{
	Guild_Clear();
}

void Guild_Save(u8 *dst, const u32 size)
{
	Guild_Check();
	WriteAndVerifySramFast(&gGuildSave, dst, sizeof(gGuildSave));
}

/* Saves made before this chunk existed have no magic: they load an empty board */
void Guild_Load(u8 *src, const u32 size)
{
	ReadSramFast(src, &gGuildSave, sizeof(gGuildSave));
	Guild_Check();
}

#endif /* CONFIG_DISTRICT_GUILD */
