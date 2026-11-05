#pragma once

#include "common-chax.h"

#ifdef CONFIG_HEX_CARDS

#define HEX_CARD_TYPES 32

/* One row of gHexCardTypes (Data/Hex/HexCardTypes.c). The card type is the uses byte of the item word. */
struct HexCardType {
	u8 attack;
	u8 maxHp;
	u8 reach;	/* 6-bit mask, bit d = attacks in direction d (0 up-left, 1 up-right, 2 right, 3 down-right, 4 down-left, 5 left) */
	u8 tier;
	u8 starter;	/* copies handed out by HexCol_GiveStarters */
	u8 pid;		/* CHARACTER_* whose name the card shows */
};

extern const struct HexCardType *const gpHexCardTypes;

bool IsHexCardItem(int item);
int MakeHexCardItem(int type);
int HexCard_Type(int item);
int HexCard_NameId(int item);

int HexCol_Count(int type);
int HexCol_Total(void);
bool HexCol_GiveToConvoy(int type);
bool HexCol_TakeOne(int type);
void HexCol_GiveStarters(void);

#endif /* CONFIG_HEX_CARDS */
