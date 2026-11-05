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

/*
 * Card art (Kernel/Wizardry/Minigames/Hex/Source/HexArt.c). The UI tiles and palettes are 4bpp, 1D OBJ order, row-major;
 * gHexPortraitKeep is the 32x32 portrait mask (Tools/hex/gen_hex_art.py).
 */
extern const u32 gHexFrameTiles[64][128];
extern const u32 gHexEmptyTiles[128];
extern const u32 gHexDigitTiles[11][8];
extern const u32 gHexDigitGreenTiles[11][8];
extern const u32 gHexScoreTiles[10][32];
extern const u32 gHexCursorTiles[2][32];
extern const u16 gHexFramePalRed[16];
extern const u16 gHexFramePalBlue[16];
extern const u32 gHexWoodBgTiles[4][8];
extern const u16 gHexWoodBgPal[16];
extern const u32 gHexPortraitKeep[128];

/* Prep screen "Hex" row (AtMenu.c). HexPrep.c starts the proc; HexScreen.c defines it. */
#define PREP_MAINMENU_HEX 12 /* prep_atmenu_item_index value of the Hex row; no vanilla row uses 12 */
struct ProcAtMenu;
void StartHexScreen_FromPrep(struct ProcAtMenu *pproc);
extern const struct ProcCmd ProcScr_HexScreen[];

#endif /* CONFIG_HEX_CARDS */
