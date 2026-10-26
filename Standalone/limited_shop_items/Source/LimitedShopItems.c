#include "global.h"
#include "proc.h"
#include "bmunit.h"
#include "bmitem.h"
#include "bmshop.h"
#include "bmtrick.h"
#include "bmlib.h"
#include "bm.h"
#include "bmio.h"
#include "bmmind.h"
#include "fontgrp.h"
#include "icon.h"
#include "hardware.h"
#include "scene.h"
#include "statscreen.h"
#include "uiutils.h"
#include "soundwrapper.h"
#include "constants/songs.h"

/* ------------------------------------------------------------------------
 * Data from Installer.event (edit there, no recompile)
 * --------------------------------------------------------------------- */

struct ShopStockEntry {
	u8 item;
	u8 stock;
};

/* One pointer per chapter ID (0x80 entries); NULL = no limited stock. */
extern const struct ShopStockEntry *const LimitedShopStockTable[];

extern const u8 LimitedShopTrapType;        /* trap type used to keep the stock */
extern const u16 LimitedShopOutOfStockText; /* 0 = built-in strings, else text ID base */
extern const u8 LimitedShopHideWeaponUses;  /* 1 = don't draw weapon uses (infinite_durability) */

extern struct ProcCmd CONST_DATA ProcScr_ShopSellInit[];

/* ------------------------------------------------------------------------
 * Stock storage
 *
 * The remaining stock lives in the vanilla trap pool, which the suspend save
 * already stores, and which vanilla clears whenever a chapter map starts.
 * The pool also keeps positionless state in vanilla (map change traps).
 *
 * Each record is a trap with type = LimitedShopTrapType, xPos = 0xFF (off
 * every map) and yPos = record number:
 *   record 0: extra = chapter ID, data[0..3] = stock of entries 0..3
 *   record N: extra, data[0..3] = stock of entries 4+5*(N-1) .. 8+5*(N-1)
 * --------------------------------------------------------------------- */

#define STOCK_TRAP_X 0xFF
#define STOCK_MAX_ENTRIES 20
#define STOCK_TABLE_SIZE 0x80

static const struct ShopStockEntry *GetChapterStockList(void)
{
	int chapter = gPlaySt.chapterIndex;

	if (chapter < 0 || chapter >= STOCK_TABLE_SIZE)
		return NULL;

	return LimitedShopStockTable[chapter];
}

static int CountStockEntries(const struct ShopStockEntry *list)
{
	int n = 0;

	while (n < STOCK_MAX_ENTRIES && list[n].item != 0)
		n++;

	return n;
}

static int GetRecordCount(int entries)
{
	if (entries <= 4)
		return 1;

	return 1 + (entries - 4 + 4) / 5;
}

static struct Trap *FindStockRecord(int record)
{
	struct Trap *trap;

	for (trap = GetTrap(0); trap->type != TRAP_NONE; ++trap) {
		if (trap->type == LimitedShopTrapType && trap->xPos == STOCK_TRAP_X && trap->yPos == record)
			return trap;
	}

	return NULL;
}

static int CountTraps(void)
{
	struct Trap *trap;
	int n = 0;

	for (trap = GetTrap(0); trap->type != TRAP_NONE; ++trap)
		n++;

	return n;
}

/* Address of the stock byte for entry i, or NULL. */
static u8 *GetStockByte(int i)
{
	struct Trap *trap;
	int record, offset;

	if (i < 4) {
		record = 0;
		offset = 4 + i;
	} else {
		int j = i - 4;

		record = 1 + j / 5;
		offset = 3 + (j - (record - 1) * 5);
	}

	trap = FindStockRecord(record);

	if (trap == NULL)
		return NULL;

	return ((u8 *)trap) + offset;
}

/* Sets up this chapter's stock records if they don't exist yet. */
static bool EnsureStock(const struct ShopStockEntry *list)
{
	struct Trap *head = FindStockRecord(0);
	int entries, records, missing, i;

	if (head != NULL && head->extra == (u8)gPlaySt.chapterIndex)
		return true;

	entries = CountStockEntries(list);
	records = GetRecordCount(entries);

	missing = 0;
	for (i = 0; i < records; i++) {
		if (FindStockRecord(i) == NULL)
			missing++;
	}

	/* Leave at least one free trap slot for vanilla. */
	if (CountTraps() + missing > TRAP_MAX_COUNT - 1)
		return false;

	for (i = 0; i < records; i++) {
		struct Trap *trap = FindStockRecord(i);

		if (trap == NULL)
			trap = AddTrap(STOCK_TRAP_X, i, LimitedShopTrapType, 0);

		trap->extra = 0;
		trap->data[0] = 0;
		trap->data[1] = 0;
		trap->data[2] = 0;
		trap->data[3] = 0;
	}

	for (i = 0; i < entries; i++)
		*GetStockByte(i) = list[i].stock;

	FindStockRecord(0)->extra = (u8)gPlaySt.chapterIndex;
	return true;
}

/* Returns the stock byte of a stocked item, or NULL if it isn't limited. */
static u8 *GetItemStockByte(int item)
{
	const struct ShopStockEntry *list = GetChapterStockList();
	int itemId = GetItemIndex(item);
	int i;

	if (list == NULL || itemId == 0)
		return NULL;

	for (i = 0; i < STOCK_MAX_ENTRIES && list[i].item != 0; i++) {
		if (list[i].item == itemId) {
			if (!EnsureStock(list))
				return NULL;

			return GetStockByte(i);
		}
	}

	return NULL;
}

/* -1: not a limited item. */
static int GetItemStock(int item)
{
	u8 *stock = GetItemStockByte(item);

	if (stock == NULL)
		return -1;

	return *stock;
}

static void ReduceItemStock(int item)
{
	u8 *stock = GetItemStockByte(item);

	if (stock != NULL && *stock > 0)
		(*stock)--;
}

/* ------------------------------------------------------------------------
 * Shop lines
 *
 * The item window is three tiles wider than vanilla (frame x 3..25):
 * icon at dst-3, name at dst-1, uses ending at dst+8, price ending at
 * dst+14, stock ending at dst+17. dst is vanilla's line start (column 7).
 * --------------------------------------------------------------------- */

static bool ShouldDrawUses(int item)
{
	if (LimitedShopHideWeaponUses && (GetItemAttributes(item) & IA_WEAPON))
		return false;

	return true;
}

static void ClearShopItemIconArea(u16 *tm)
{
	tm[0x00] = 0;
	tm[0x01] = 0;
	tm[0x20] = 0;
	tm[0x21] = 0;
}

static void DrawLimitedShopBuyLine(struct Text *th, int item, struct Unit *unit, u16 *dst)
{
	int usable = (unit == NULL) ? true : IsItemDisplayUsable(unit, item);
	int price = GetItemPurchasePrice(unit, item);
	int stock = GetItemStock(item);
	int nameColor, statColor, priceColor;

	priceColor = (int)GetPartyGoldAmount() >= price ? TEXT_COLOR_SYSTEM_BLUE : TEXT_COLOR_SYSTEM_GRAY;

	if (stock == 0) {
		nameColor = TEXT_COLOR_SYSTEM_GRAY;
		statColor = TEXT_COLOR_SYSTEM_GRAY;
		priceColor = TEXT_COLOR_SYSTEM_GRAY;
	} else if (stock > 0) {
		nameColor = TEXT_COLOR_SYSTEM_GOLD;
		statColor = TEXT_COLOR_SYSTEM_BLUE;
	} else {
		nameColor = usable ? TEXT_COLOR_SYSTEM_WHITE : TEXT_COLOR_SYSTEM_GRAY;
		statColor = usable ? TEXT_COLOR_SYSTEM_BLUE : TEXT_COLOR_SYSTEM_GRAY;
	}

	Text_SetParams(th, 0, nameColor);
	Text_DrawString(th, GetItemName(item));
	PutText(th, dst - 1);

	if (ShouldDrawUses(item))
		PutNumberOrBlank(dst + 8, statColor, GetItemUses(item));

	if (stock >= 0)
		PutNumber(dst + 17, statColor, stock);

	DrawIcon(dst - 3, GetItemIconId(item), 0x4000);

	PutNumber(dst + 14, priceColor, price);
}

static void DrawLimitedShopSellLine(struct Text *th, int item, struct Unit *unit, u16 *dst)
{
	int usable = IsItemDisplayUsable(unit, item);

	Text_SetParams(th, 0, usable ? TEXT_COLOR_SYSTEM_WHITE : TEXT_COLOR_SYSTEM_GRAY);
	Text_DrawString(th, GetItemName(item));
	PutText(th, dst - 1);

	if (ShouldDrawUses(item))
		PutNumberOrBlank(dst + 8, usable ? TEXT_COLOR_SYSTEM_BLUE : TEXT_COLOR_SYSTEM_GRAY, GetItemUses(item));

	DrawIcon(dst - 3, GetItemIconId(item), 0x4000);

	if (IsItemSellable(item))
		PutNumber(dst + 14, TEXT_COLOR_SYSTEM_BLUE, GetItemSellPrice(item));
	else
		Text_InsertDrawString(th, 0x5C, TEXT_COLOR_SYSTEM_BLUE, GetStringFromIndex(0x537));
}

/* ------------------------------------------------------------------------
 * Out of stock dialogue
 * --------------------------------------------------------------------- */

/* 0x03 = [A], end of string = [X] */
static const char sOutOfStock_Armory[] = "Sorry, we don't have that.\x03";
static const char sOutOfStock_Vendor[] = "You took the last one!\x03";
static const char sOutOfStock_Secret[] = "Hmm... We're out.\x03";

static void StartOutOfStockDialogue(struct ProcShop *proc)
{
	const char *str;

	if (LimitedShopOutOfStockText != 0) {
		/* Text ID base + 0/1/2 for armory/vendor/secret shop, like vanilla */
		StartShopDialogue(LimitedShopOutOfStockText, proc);
		return;
	}

	switch (proc->shopType) {
	case SHOP_TYPE_VENDOR:
		str = sOutOfStock_Vendor;
		break;

	case SHOP_TYPE_SECRET_SHOP:
		str = sOutOfStock_Secret;
		break;

	default:
		str = sOutOfStock_Armory;
		break;
	}

	/* Same as vanilla StartShopDialogue, with a raw string */
	SetInitTalkTextFont();
	ClearTalkText();

	StartTalkExt(8, 2, str, proc);

	SetTalkPrintColor(0);

	SetTalkFlag(TALK_FLAG_INSTANTSHIFT);
	SetTalkFlag(TALK_FLAG_NOBUBBLE);
	SetTalkFlag(TALK_FLAG_NOSKIP);

	SetActiveTalkFace(1);
}

/* ------------------------------------------------------------------------
 * Hooked vanilla shop functions
 * --------------------------------------------------------------------- */

/* Replaces ShopDrawBuyItemLine (buy list line redraw while scrolling) */
void ShopDrawBuyItemLine_Limited(struct ProcShop *proc, int itemIndex)
{
	u16 item;
	int index = DivRem(itemIndex, SHOP_TEXT_LINES + 1);

	SetTextFont(0);
	InitSystemTextFont();

	BG_EnableSyncByMask(BG2_SYNC_BIT);

	ClearText(&gShopItemTexts[index]);

	item = proc->shopItems[itemIndex];

	if (item != 0)
		DrawLimitedShopBuyLine(
			&gShopItemTexts[index],
			item,
			proc->unit,
			gBG2TilemapBuffer + TILEMAP_INDEX(7, (itemIndex * 2 & 0x1F)));
}

/* Replaces ShopDrawSellItemLine (sell list line redraw while scrolling) */
void ShopDrawSellItemLine_Limited(struct ProcShop *proc, int itemIndex)
{
	u16 item;
	int index = DivRem(itemIndex, SHOP_TEXT_LINES + 1);

	SetTextFont(0);
	InitSystemTextFont();

	BG_EnableSyncByMask(BG2_SYNC_BIT);

	ClearText(&gShopItemTexts[index]);

	item = proc->shopItems[itemIndex];

	if (item != 0)
		DrawLimitedShopSellLine(
			&gShopItemTexts[index],
			item,
			proc->unit,
			gBG2TilemapBuffer + TILEMAP_INDEX(7, (itemIndex * 2 & 0x1F)));
}

/* Replaces Shop_Loop_BuyKeyHandler: hand moved left, out of stock check */
void Shop_Loop_BuyKeyHandler_Limited(struct ProcShop *proc)
{
	u8 head_loc;
	u32 cursor_at_head;
	int price;
	int a;
	int b;

	Shop_TryMoveHandPage();

	BG_SetPosition(2, 0, ShopSt_GetBg2Offset());

	head_loc = proc->head_loc;
	cursor_at_head = ShopSt_GetHeadLoc() != head_loc;

	proc->head_loc = ShopSt_GetHeadLoc();
	proc->hand_loc = ShopSt_GetHandLoc();

	proc->head_idx = proc->head_loc;
	proc->hand_idx = proc->hand_loc;

	a = proc->head_loc * 16;
	b = (proc->hand_loc * 16) - 72;

	DisplayUiHand(56 - 32, a - b);

	if ((proc->helpTextActive != 0) && (cursor_at_head != 0))
		StartItemHelpBox(56, a - b, proc->shopItems[proc->head_loc]);

	DisplayShopUiArrows();

	if (IsShopPageScrolling() != 0)
		return;

	if (proc->helpTextActive != 0) {
		if (gKeyStatusPtr->newKeys & (B_BUTTON | R_BUTTON)) {
			proc->helpTextActive = 0;
			CloseHelpBox();
		}
		return;
	}

	if (gKeyStatusPtr->newKeys & R_BUTTON) {
		proc->helpTextActive = 1;
		StartItemHelpBox(56, a - b, proc->shopItems[proc->head_loc]);
		return;
	}

	price = GetItemPurchasePrice(proc->unit, proc->shopItems[proc->head_loc]);

	if (gKeyStatusPtr->newKeys & A_BUTTON) {
		if (GetItemStock(proc->shopItems[proc->head_loc]) == 0) {
			StartOutOfStockDialogue(proc);
			Proc_Goto(proc, PL_SHOP_BUY);
		} else if (price > (int)GetPartyGoldAmount()) {
			StartShopDialogue(0x8B2, proc);
			Proc_Goto(proc, PL_SHOP_BUY);
		} else {
			SetTalkNumber(price);
			StartShopDialogue(0x8B5, proc);
			Proc_Break(proc);
		}
		return;
	}

	if (gKeyStatusPtr->newKeys & B_BUTTON) {
		PlaySoundEffect(SONG_SE_SYS_WINDOW_CANSEL1);
		Proc_Goto(proc, PL_SHOP_SELL_NOITEM);
		return;
	}
}

/* Replaces DrawShopSoldItems (whole buy list) */
void DrawShopSoldItems_Limited(struct ProcShop *proc)
{
	int item;
	int index;
	int i;

	SetTextFont(0);
	InitSystemTextFont();

	for (i = proc->hand_idx; i < proc->hand_idx + SHOP_TEXT_LINES; i++) {
		index = DivRem(i, SHOP_TEXT_LINES + 1);
		ClearText(&gShopItemTexts[index]);
		ClearShopItemIconArea(gBG2TilemapBuffer + TILEMAP_INDEX(7, ((i * 2) & 0x1F)) - 3);
	}

	for (i = proc->hand_idx; i < proc->hand_idx + SHOP_TEXT_LINES; i++) {
		index = DivRem(i, SHOP_TEXT_LINES + 1);
		item = proc->shopItems[i];

		if (item == 0)
			break;

		DrawLimitedShopBuyLine(
			&gShopItemTexts[index],
			item,
			proc->unit,
			gBG2TilemapBuffer + TILEMAP_INDEX(7, ((i * 2) & 0x1F)));
	}

	BG_SetPosition(BG_2, 0, (proc->hand_idx * 0x10) - 0x48);
	BG_EnableSyncByMask(BG2_SYNC_BIT);
}

/* Replaces ShopInitTexts_OnSell: also clears the moved icon column */
void ShopInitTexts_OnSell_Limited(struct ProcShop *parent)
{
	struct ProcShopInit *proc;
	int i;

	parent->buy_or_sel = SHOP_ST_SELL;

	proc = Proc_Start(ProcScr_ShopSellInit, PROC_TREE_3);
	proc->shopproc = parent;

	SetTextFont(0);
	InitSystemTextFont();

	for (i = 0; i < SHOP_TEXT_LINES; i++) {
		u16 *dst = gBG2TilemapBuffer + TILEMAP_INDEX(7, ((i * 2) & 0x1F));

		PutBlankText(&gShopItemTexts[DivRem(i, SHOP_TEXT_LINES + 1)], dst);
		ClearShopItemIconArea(dst - 3);
	}

	BG_SetPosition(2, 0, -0x48);
	BG_EnableSyncByMask(BG2_SYNC_BIT);
}

/* Replaces ShopDrawDefaultSellItemLine (whole sell list) */
void ShopDrawDefaultSellItemLine_Limited(struct ProcShop *proc)
{
	int i;
	int index;

	SetTextFont(0);
	InitSystemTextFont();

	for (i = 0; i < SHOP_TEXT_LINES; i++) {
		index = DivRem(i, SHOP_TEXT_LINES + 1);
		ClearText(&gShopItemTexts[index]);
	}

	for (i = 0; i < SHOP_TEXT_LINES; i++) {
		int item;

		index = DivRem(i, SHOP_TEXT_LINES + 1);
		item = proc->unit->items[i];

		if (item == 0)
			break;

		DrawLimitedShopSellLine(
			&gShopItemTexts[index],
			item,
			proc->unit,
			gBG2TilemapBuffer + TILEMAP_INDEX(7, ((i * 2) & 0x1F)));
	}

	BG_EnableSyncByMask(BG2_SYNC_BIT);
}

/* Replaces HandleShopBuyAction: also spends one stock */
void HandleShopBuyAction_Limited(struct ProcShop *proc)
{
	PlaySeDelayed(0xB9, 8);

	gActionData.unitActionType = UNIT_ACTION_SHOPPED;

	SetPartyGoldAmount(GetPartyGoldAmount() - GetItemPurchasePrice(proc->unit, proc->shopItems[proc->head_loc]));

	ReduceItemStock(proc->shopItems[proc->head_loc]);

	UpdateShopItemCounts(proc);
	DrawShopSoldItems_Limited(proc);

	DisplayGoldBoxText(gBG0TilemapBuffer + TILEMAP_INDEX(27, 6));
}
