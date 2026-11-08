#include "common-chax.h"
#include "kernel-lib.h"
#include "hex.h"

#ifdef CONFIG_HEX_CARDS

/*
 * District Hex: a map event starts a match against the NPC of its district. The NPC's deck is in gHexNpcDecks (Data/Hex/HexNpcDecks.c),
 * the event sets gEventSlots[2] to the NPC's character id. The map is suspended while the Hex screen runs (its own fades), then
 * restored and faded in again. In district mode the screen goes straight to the hand picker and leaves after the match.
 */

int HexNpc_FindDeck(int chapter, int pid)
{
	int i;

	for (i = 0; gHexNpcDecks[i].chapter != HEX_NPC_END; i++) {
		if (gHexNpcDecks[i].chapter == chapter && gHexNpcDecks[i].pid == pid)
			return i;
	}
	return -1;
}

/* A rare card the NPC has run out of (its claims reached the cap) is replaced by its fallback. */
int HexNpc_CardType(const struct HexNpcCard *card)
{
	if (card->claimSlot != HEX_NO_CLAIM && HexClaim_Get(card->claimSlot) >= card->cap)
		return card->fallback;
	return card->type;
}

bool HexNpc_IsRareLive(const struct HexNpcCard *card)
{
	return card->claimSlot != HEX_NO_CLAIM && HexClaim_Get(card->claimSlot) < card->cap;
}

static const struct ProcCmd ProcScr_HexDistrict[] = {
	PROC_NAME("HexDistrict"),
	PROC_CALL_ARG(NewFadeOut, 0x10),
	PROC_WHILE(FadeOutExists),
	PROC_CALL(BMapDispSuspend),
	PROC_YIELD,
	PROC_START_CHILD_BLOCKING(ProcScr_HexScreen),
	PROC_YIELD,
	PROC_CALL(BMapDispResume),
	PROC_CALL(RefreshBMapGraphics),
	PROC_CALL_ARG(NewFadeIn, 0x10),
	PROC_WHILE(FadeInExists),
	PROC_END,
};

/* ASMC from a talk event: gEventSlots[2] is the NPC. An unknown NPC in this chapter starts nothing. */
void StartDistrictHexASMC(ProcPtr parent)
{
	int deck = HexNpc_FindDeck(gPlaySt.chapterIndex, gEventSlots[2]);

	if (deck < 0)
		return;
	HexScreen_SetDistrictDeck(deck);
	Proc_StartBlocking(ProcScr_HexDistrict, parent);
}

/* Rows of gHexShopItems (Data/Hex/HexShops.c) for this district. */
int HexShop_Count(int chapter)
{
	int i, n = 0;

	for (i = 0; gHexShopItems[i].chapter != HEX_SHOP_END; i++) {
		if (gHexShopItems[i].chapter == chapter)
			n++;
	}
	return n;
}

/* The n-th row for this district, or NULL past the last one. */
const struct HexShopItem *HexShop_Get(int chapter, int n)
{
	int i;

	for (i = 0; gHexShopItems[i].chapter != HEX_SHOP_END; i++) {
		if (gHexShopItems[i].chapter != chapter)
			continue;
		if (n == 0)
			return &gHexShopItems[i];
		n--;
	}
	return NULL;
}

int HexShop_ItemWord(const struct HexShopItem *row)
{
	if (row->raw)
		return row->item;
	return MakeNewItem(row->item);
}

/* The row whose item word is itemWord, in this district; NULL if there is none. */
const struct HexShopItem *HexShop_FindRow(int chapter, int itemWord)
{
	int i;

	for (i = 0; gHexShopItems[i].chapter != HEX_SHOP_END; i++) {
		if (gHexShopItems[i].chapter == chapter && HexShop_ItemWord(&gHexShopItems[i]) == itemWord)
			return &gHexShopItems[i];
	}
	return NULL;
}

/* The binder holds at least costCount copies of costType. */
bool HexShop_CanAfford(const struct HexShopItem *row)
{
	return HexCol_Count(row->costType) >= row->costCount;
}

/* Takes costCount copies of costType from the binder. The caller checks HexShop_CanAfford first. */
void HexShop_Pay(const struct HexShopItem *row)
{
	int n;

	for (n = 0; n < row->costCount; n++)
		HexCol_TakeOne(row->costType);
}

#endif /* CONFIG_HEX_CARDS */
