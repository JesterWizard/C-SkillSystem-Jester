#include "common-chax.h"
#include "kernel-lib.h"
#include "item-sys.h"
#include "bmshop.h"
#include "constants/texts.h"
#include "hex.h"

#ifdef CONFIG_HEX_CARDS

/*
 * The district Hex shop: a dealer's talk event opens the vanilla FE8 shop screen (StartShopScreen, gProcScr_Shop) with shopType
 * SHOP_TYPE_HEX. That shop type is the Hex mode:
 *   - the portrait is the Hex witch in the shopkeeper box (FID_HEX_WITCH_SHOP) and the lines are hers (MSG_HEXSHOP_*);
 *   - the shop goes straight to the buy list (no Buy/Sell question);
 *   - a ware costs cards from the binder: the price line shows the cards, a ware you can't afford is grey, and buying takes the
 *     cards (no gold, no gold box);
 *   - B leaves with her goodbye (LimitedShopStock.c, Shop_Loop_BuyKeyHandler).
 * Outside Hex mode the four vanilla functions below run as before (LynJump.event redirects them here).
 */

#define HEX_SHOP_RAM_SIZE 0x20
enum { HEX_SHOP_RAM_FITS = 1 / (sizeof(struct HexShopRam) <= HEX_SHOP_RAM_SIZE) };

#define HEX_SHOP_MAX_ROWS 12	/* gHexShopRam.items keeps 12 rows and a 0 terminator */
#define HEX_SHOP_COST_LEN 32	/* "<card name> x<count>" and its 0 */
#define HEX_SHOP_COST_X 72	/* x of the cost text in the price line (Text_InsertDrawString) */

static bool HexShop_IsLiving(struct Unit *unit)
{
	return UNIT_IS_VALID(unit) && !(unit->state & US_DEAD);
}

/* The shop screen (gProcScr_Shop) is in Hex mode. No shop proc, or a vanilla shop, is not. */
bool HexShop_IsActive(void)
{
	struct ProcShop *proc = (struct ProcShop *)Proc_Find(gProcScr_Shop);

	return proc && proc->shopType == SHOP_TYPE_HEX;
}

bool HexShop_IsHexProc(struct ProcShop *proc)
{
	return proc->shopType == SHOP_TYPE_HEX;
}

/*
 * ASMC from a dealer's talk event (the parent is the event proc, so the event waits for the shop). The shop lists the rows of this
 * district. The items are the row items as StartShopScreen takes them (an item index, or a dura item word for tonics); StartShopScreen
 * makes the item words. The unit is the active unit, or the first living player unit when there is none.
 */
void StartDistrictHexShopASMC(ProcPtr parent)
{
	struct Unit *unit;
	const struct HexShopItem *row;
	int i, n;

	for (i = 0; i < (int)ARRAY_COUNT(gHexShopRam.items); i++)
		gHexShopRam.items[i] = 0;

	for (n = 0; n < HEX_SHOP_MAX_ROWS; n++) {
		row = HexShop_Get(gPlaySt.chapterIndex, n);
		if (!row)
			break;
		gHexShopRam.items[n] = row->item;
	}
	if (n == 0)
		return;
	gHexShopRam.items[n] = 0;

	unit = gActiveUnit;
	if (!HexShop_IsLiving(unit)) {
		unit = NULL;
		for (i = 0; i < (int)ARRAY_COUNT(gUnitArrayBlue); i++) {
			if (HexShop_IsLiving(&gUnitArrayBlue[i])) {
				unit = &gUnitArrayBlue[i];
				break;
			}
		}
	}
	if (!unit)
		return;

	StartShopScreen(unit, gHexShopRam.items, SHOP_TYPE_HEX, parent);
}

/*
 * "<card name> x<count>" into buf. The card name is copied first: GetStringFromIndex returns a shared buffer. The count is written
 * with loops (no / or % on the count).
 */
static void HexShop_CostString(char *buf, const struct HexShopItem *row)
{
	const char *name = GetStringFromIndex(HexCardType_NameId(row->costType));
	int i = 0, n = row->costCount, hundreds = 0, tens = 0;

	while (name[i] != 0 && i < HEX_SHOP_COST_LEN - 8) {
		buf[i] = name[i];
		i++;
	}
	buf[i++] = ' ';
	buf[i++] = 'x';

	while (n >= 100) {
		n -= 100;
		hundreds++;
	}
	while (n >= 10) {
		n -= 10;
		tens++;
	}
	if (hundreds != 0)
		buf[i++] = '0' + hundreds;
	if (hundreds != 0 || tens != 0)
		buf[i++] = '0' + tens;
	buf[i++] = '0' + n;
	buf[i] = 0;
}

/*
 * A ware line of the buy list in Hex mode (DrawShopItemPriceLine, LimitedShopStock.c). The name and icon are where
 * DrawStockedItemLine puts them. No uses, no stock, no gold: the cost is the card name and count, in blue if the binder has them
 * and grey if not (the name is grey too).
 */
void HexShop_DrawPriceLine(struct Text *th, int item, u16 *dst)
{
	const struct HexShopItem *row = HexShop_FindRow(gPlaySt.chapterIndex, item);
	bool ok = row && HexShop_CanAfford(row);
	char cost[HEX_SHOP_COST_LEN];

	Text_SetParams(th, 0, ok ? TEXT_COLOR_SYSTEM_WHITE : TEXT_COLOR_SYSTEM_GRAY);
	Text_DrawString(th, GetItemName(item));

	if (row) {
		HexShop_CostString(cost, row);
		Text_InsertDrawString(th, HEX_SHOP_COST_X, ok ? TEXT_COLOR_SYSTEM_BLUE : TEXT_COLOR_SYSTEM_GRAY, cost);
	}

	if (gpKernelDesignerConfig->limited_shop_items == true) {
		PutText(th, dst - 1);
		DrawIcon(dst - 3, GetItemIconId(item), 0x4000);
	} else {
		PutText(th, dst + 2);
		DrawIcon(dst, GetItemIconId(item), 0x4000);
	}
}

/* A on a ware in Hex mode. Without the cards she says so (MSG_HEXSHOP_POOR); with them the confirm asks for the cost. */
void HexShop_OnBuyPressed(struct ProcShop *proc)
{
	const struct HexShopItem *row = HexShop_FindRow(gPlaySt.chapterIndex, proc->shopItems[proc->head_loc]);

	if (!row || !HexShop_CanAfford(row)) {
		StartShopDialogue(0x8B2, proc);
		Proc_Goto(proc, PL_SHOP_BUY);
		return;
	}

	SetTalkNumber(row->costCount);
	StartShopDialogue(0x8B5, proc);
	Proc_Break(proc);
}

/* The bought ware's cards go to the dealer. The ware on the head line is the one A was pressed on. */
void HexShop_PayForItem(struct ProcShop *proc)
{
	const struct HexShopItem *row = HexShop_FindRow(gPlaySt.chapterIndex, proc->shopItems[proc->head_loc]);

	if (row)
		HexShop_Pay(row);
}

/* The vanilla base message id -> the Hex witch's line, or 0 for the vanilla line. Hex lines are the same for every shop type. */
static int HexShop_MsgFor(int baseMsgId)
{
	switch (baseMsgId) {
	case 0x89A:
	case 0x8A3:
		return MSG_HEXSHOP_BUY;
	case 0x89D:
	case 0x8A6:
		return MSG_HEXSHOP_MORE;
	case 0x8B2:
		return MSG_HEXSHOP_POOR;
	case 0x8B5:
		return MSG_HEXSHOP_CONFIRM;
	case 0x8B8:
		return MSG_HEXSHOP_BYE;
	}
	return 0;
}

LYN_REPLACE_CHECK(Shop_GetPortraitIndex);
int Shop_GetPortraitIndex(struct ProcShop * proc)
{
	if (HexShop_IsActive())
		return FID_HEX_WITCH_SHOP;

	return gShopPortraitLut[proc->shopType];
}

LYN_REPLACE_CHECK(StartShopDialogue);
void StartShopDialogue(int baseMsgId, struct ProcShop * proc)
{
	int msgId = 0;
	int offset;

	/* The Hex shop has no dialogue LUT row: its unremapped lines are the vendor's */
	if (HexShop_IsActive()) {
		msgId = HexShop_MsgFor(baseMsgId);
		offset = 1;
	}
	else
		offset = gShopDialogueOffsetLut[proc->shopType];

	if (msgId == 0)
		msgId = baseMsgId + offset;

	SetInitTalkTextFont();
	ClearTalkText();

	StartTalkExt(8, 2, GetStringFromIndex(msgId), proc);

	SetTalkPrintColor(0);

	SetTalkFlag(TALK_FLAG_INSTANTSHIFT);
	SetTalkFlag(TALK_FLAG_NOBUBBLE);
	SetTalkFlag(TALK_FLAG_NOSKIP);

	SetActiveTalkFace(1);
}

LYN_REPLACE_CHECK(Shop_EntryDialogue);
void Shop_EntryDialogue(struct ProcShop * proc)
{
	if (HexShop_IsActive()) {
		Proc_Goto(proc, PL_SHOP_BUY);
		return;
	}

	if (!proc->unit)
	{
		Proc_Goto(proc, PL_SHOP_PREP_ENTRY);
	}
	else
	{
		StartShopDialogue(0x89A, proc);
	}
}

LYN_REPLACE_CHECK(Shop_AnythingElseRestartDialogue);
void Shop_AnythingElseRestartDialogue(struct ProcShop * proc)
{
	proc->head_loc = 0;

	if (HexShop_IsActive()) {
		StartShopDialogue(0x8A6, proc);
		Proc_Goto(proc, PL_SHOP_BUY_MAIN);
		return;
	}

	StartShopDialogue(0x89D, proc);
}

#endif /* CONFIG_HEX_CARDS */
