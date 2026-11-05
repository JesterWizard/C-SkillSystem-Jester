/*
 * Hex Cards, standalone add-on (see ../README.md). Cards are vanilla items: one item slot
 * (HEX_CARD_ITEM) with the card type in the uses byte. Header for the card helpers in
 * HexCards.c, the card table in HexCardTypes.c and the item-name hooks in HexItem.c.
 *
 * HEX_CARD_ITEM must match HEX_CARD_ITEM in ../Installer.event, which writes the item row.
 */
#pragma once

#include "global.h"

/* Vanilla item slot used for cards (a "Dummy" item, see README). */
#define HEX_CARD_ITEM 0xC1

#define HEX_CARD_TYPES 32

/* One row of gHexCardTypes (HexCardTypes.c). The card type is the uses byte of the card item word. */
struct HexCardType {
	u8 attack;
	u8 maxHp;
	u8 reach;	/* 6-bit mask, bit d = attacks in direction d (0 up-left, 1 up-right, 2 right, 3 down-right, 4 down-left, 5 left) */
	u8 tier;
	u8 starter;	/* copies handed out by HexCol_GiveStarters */
	u8 pid;		/* CHARACTER_* whose name and description the card shows */
};

extern const struct HexCardType gHexCardTypes[HEX_CARD_TYPES];

/* HexCards.c */
bool IsHexCardItem(int item);
int MakeHexCardItem(int type);
int HexCard_Type(int item);
int HexCard_NameId(int item);
int HexCard_DescId(int item);

int HexCol_Count(int type);
int HexCol_Total(void);
bool HexCol_GiveToConvoy(int type);
bool HexCol_TakeOne(int type);
void HexCol_GiveStarters(void);

/* HexItem.c: the GetItemName / GetItemDescId hooks (vanilla behaviour for every other item). */
char *HexItem_GetItemName(int item);
int HexItem_GetItemDescId(int item);
