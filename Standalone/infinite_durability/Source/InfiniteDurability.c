#include "global.h"
#include "bmitem.h"
#include "bmitemuse.h"
#include "bmunit.h"
#include "fontgrp.h"
#include "functions.h"
#include "hardware.h"
#include "icon.h"

extern const u8 InfiniteDurabilityEnabled;

struct PrepScreenItemListEnt {
	/* 00 */ u8 pid;
	/* 01 */ u8 itemSlot;
	/* 02 */ u16 item;
};

extern struct PrepScreenItemListEnt gPrepScreenItemList[];
extern u16 gUnknown_02012F56;

static int HideWeaponDurability(int item)
{
	return InfiniteDurabilityEnabled && (GetItemAttributes(item) & IA_WEAPON);
}

static void PutItemUses(u16 *tm, int color, int item)
{
	if (!HideWeaponDurability(item))
		PutNumberOrBlank(tm, color, GetItemUses(item));
}

/**
 * Vanilla GetItemAfterUse. Weapons keep their current uses while the patch is on.
 * Staves, items, and unbreakable gear keep vanilla rules.
 */
u16 GetItemAfterUse_InfiniteDurability(int item)
{
	if (GetItemAttributes(item) & IA_UNBREAKABLE)
		return item;

	if (HideWeaponDurability(item))
		return item;

	item -= (1 << 8);

	if (item < (1 << 8))
		return 0;

	return item;
}

void DrawItemMenuLine_InfiniteDurability(struct Text *text, int item, s8 isUsable, u16 *mapOut)
{
	int color = isUsable ? TEXT_COLOR_SYSTEM_BLUE : TEXT_COLOR_SYSTEM_GRAY;

	Text_SetParams(text, 0, isUsable ? TEXT_COLOR_SYSTEM_WHITE : TEXT_COLOR_SYSTEM_GRAY);
	Text_DrawString(text, GetItemName(item));
	PutText(text, mapOut + 2);
	PutItemUses(mapOut + 11, color, item);
	DrawIcon(mapOut, GetItemIconId(item), 0x4000);
}

void DrawItemMenuLineLong_InfiniteDurability(struct Text *text, int item, s8 isUsable, u16 *mapOut)
{
	int color = isUsable ? TEXT_COLOR_SYSTEM_BLUE : TEXT_COLOR_SYSTEM_GRAY;

	Text_SetParams(text, 0, isUsable ? TEXT_COLOR_SYSTEM_WHITE : TEXT_COLOR_SYSTEM_GRAY);
	Text_DrawString(text, GetItemName(item));
	PutText(text, mapOut + 2);

	if (!HideWeaponDurability(item)) {
		PutNumberOrBlank(mapOut + 10, color, GetItemUses(item));
		PutNumberOrBlank(mapOut + 13, color, GetItemMaxUses(item));
		PutSpecialChar(mapOut + 11, isUsable ? TEXT_COLOR_SYSTEM_WHITE : TEXT_COLOR_SYSTEM_GRAY, TEXT_SPECIAL_SLASH);
	}

	DrawIcon(mapOut, GetItemIconId(item), 0x4000);
}

void DrawItemMenuLineNoColor_InfiniteDurability(struct Text *text, int item, u16 *mapOut)
{
	Text_SetCursor(text, 0);
	Text_DrawString(text, GetItemName(item));
	PutText(text, mapOut + 2);
	PutItemUses(mapOut + 11, Text_GetColor(text), item);
	DrawIcon(mapOut, GetItemIconId(item), 0x4000);
}

void DrawItemStatScreenLine_InfiniteDurability(struct Text *text, int item, int nameColor, u16 *mapOut)
{
	int color;

	ClearText(text);
	Text_SetColor(text, nameColor);
	Text_DrawString(text, GetItemName(item));

	if (!HideWeaponDurability(item)) {
		color = (nameColor == TEXT_COLOR_SYSTEM_GRAY) ? TEXT_COLOR_SYSTEM_GRAY : TEXT_COLOR_SYSTEM_WHITE;
		PutSpecialChar(mapOut + 12, color, TEXT_SPECIAL_SLASH);

		color = (nameColor != TEXT_COLOR_SYSTEM_GRAY) ? TEXT_COLOR_SYSTEM_BLUE : TEXT_COLOR_SYSTEM_GRAY;
		PutNumberOrBlank(mapOut + 11, color, GetItemUses(item));
		PutNumberOrBlank(mapOut + 14, color, GetItemMaxUses(item));
	}

	PutText(text, mapOut + 2);
	DrawIcon(mapOut, GetItemIconId(item), 0x4000);
}

void PrepItemScreen_DrawUnitItems_InfiniteDurability(struct Text *text, u16 *tilemap, struct Unit *unit, u16 flags)
{
	int itemCount;
	int i;

	TileMap_FillRect(tilemap, 12, 20, 0);

	if (flags & 2)
		ResetIconGraphics();

	if (unit == NULL)
		return;

	itemCount = GetUnitItemCount(unit);

	for (i = 0; i < itemCount; text++, i++) {
		u16 item = unit->items[i];
		int isUnusable = (flags & 4) ? !CanUnitUseItemPrepScreen(unit, item) : !IsItemDisplayUsable(unit, item);

		if (!(flags & 1)) {
			ClearText(text);
			Text_SetColor(text, isUnusable);
			Text_SetCursor(text, 0);
			Text_DrawString(text, GetItemName(item));
		}

		DrawIcon(TILEMAP_LOCATED(tilemap, 0, i * 2), GetItemIconId(item), 0x4000);
		PutText(text, TILEMAP_LOCATED(tilemap, 2, i * 2));
		PutItemUses(
			TILEMAP_LOCATED(tilemap, 11, i * 2),
			!isUnusable ? TEXT_COLOR_SYSTEM_BLUE : TEXT_COLOR_SYSTEM_GRAY,
			item);
	}
}

void DrawPrepScreenItems_InfiniteDurability(u16 *tm, struct Text *th, struct Unit *unit, u8 checkPrepUsability)
{
	int i;
	int itemCount = GetUnitItemCount(unit);

	TileMap_FillRect(tm, 11, 9, 0);

	for (i = 0; i < itemCount; i++) {
		int item = unit->items[i];
		s8 isUsable = checkPrepUsability ? CanUnitUseItemPrepScreen(unit, item) : IsItemDisplayUsable(unit, item);

		ClearText(th);
		PutDrawText(
			th,
			tm + i * 0x40 + 2,
			!isUsable ? TEXT_COLOR_SYSTEM_GRAY : TEXT_COLOR_SYSTEM_WHITE,
			0,
			0,
			GetItemName(item));
		PutItemUses(tm + i * 0x40 + 0xB, isUsable ? TEXT_COLOR_SYSTEM_BLUE : TEXT_COLOR_SYSTEM_GRAY, item);
		DrawIcon(tm + i * 0x40, GetItemIconId(item), 0x4000);
		th++;
	}
}

void PrepItemSupply_DrawItemList_InfiniteDurability(struct Text *textBase, u16 *tm, int yLines, struct Unit *unit)
{
	int i;

	TileMap_FillRect(tm, 12, 31, 0);

	if (gUnknown_02012F56 == 0) {
		ClearText(textBase);
		Text_InsertDrawString(textBase, 0, TEXT_COLOR_SYSTEM_GRAY, GetStringFromIndex(0x5A8));
		PutText(textBase, tm + 3);
		return;
	}

	for (i = yLines; (i < yLines + 7) && (i < gUnknown_02012F56); i++) {
		struct Text *th = textBase + (i & 7);
		int item = gPrepScreenItemList[i].item;
		int unusable = !IsItemDisplayUsable(unit, item);

		ClearText(th);
		Text_InsertDrawString(th, 0, unusable, GetItemName(item));
		DrawIcon(tm + TILEMAP_INDEX(1, i * 2 & 0x1F), GetItemIconId(item), 0x4000);
		PutText(th, tm + TILEMAP_INDEX(3, i * 2 & 0x1F));
		PutItemUses(tm + TILEMAP_INDEX(12, i * 2 & 0x1F), !unusable ? TEXT_COLOR_SYSTEM_BLUE : TEXT_COLOR_SYSTEM_GRAY, item);
	}
}

void PrepItemSupply_DrawItemListRow_InfiniteDurability(struct Text *textBase, u16 *tm, int yLines, struct Unit *unit)
{
	if (gUnknown_02012F56 > yLines) {
		int y = (yLines * 2) & 0x1F;
		struct Text *th = textBase + (yLines & 7);
		int item = gPrepScreenItemList[yLines].item;
		int unusable = !IsItemDisplayUsable(unit, item);
		int offset = TILEMAP_INDEX(0, y);

		TileMap_FillRect(tm + offset, 12, 1, 0);
		ClearText(th);
		Text_InsertDrawString(th, 0, unusable, GetItemName(item));
		DrawIcon(tm + offset + 1, GetItemIconId(item), 0x4000);
		PutText(th, tm + offset + 3);
		PutItemUses(tm + offset + 12, !unusable ? TEXT_COLOR_SYSTEM_BLUE : TEXT_COLOR_SYSTEM_GRAY, item);
	}
}
