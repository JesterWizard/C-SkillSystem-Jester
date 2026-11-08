#pragma once

#include "common-chax.h"

#ifdef CONFIG_HEX_CARDS

#include "kernel/hex-core.h"

#define HEX_CARD_TYPES 44
#define HEX_MONSTER_FIRST 32	/* types 32..43 are monsters: art from gHexMonsterArt, name from the class */

/* One row of gHexCardTypes (Data/Hex/HexCardTypes.c). The card type is the uses byte of the item word. */
struct HexCardType {
	u8 attack;
	u8 maxHp;
	u8 reach;	/* 6-bit mask, bit d = attacks in direction d (0 up-left, 1 up-right, 2 right, 3 down-right, 4 down-left, 5 left) */
	u8 tier;
	u8 starter;	/* copies handed out by HexCol_GiveStarters */
	u8 pid;		/* CHARACTER_* whose name the card shows; 0 for a monster card */
	u8 jid;		/* CLASS_* whose name a monster card shows (pid 0) */
};

extern const struct HexCardType *const gpHexCardTypes;

bool IsHexCardItem(int item);
int MakeHexCardItem(int type);
int HexCard_Type(int item);
int HexCardType_NameId(int type);
int HexCard_NameId(int item);

int HexCol_Count(int type);
int HexCol_Total(void);
bool HexCol_Give(int type);
bool HexCol_TakeOne(int type);
void HexCol_GiveStarters(void);
void HexCol_MigrateItems(void);

/*
 * Hex save data (Kernel/Wizardry/Minigames/Hex/Source/HexSave.c): the card binder and the district claim counters.
 * gHexSave is in RAM (include/link/config-memmap.s) and saved as a 0x54-byte chunk in each save list.
 */
#define HEX_BINDER_SLOTS 48	/* card types the binder can hold (HEX_CARD_TYPES <= this) */
#define HEX_CLAIM_SLOTS 32	/* district claim counters (Hex players' signature cards) */
#define HEX_BINDER_MAX 255

struct HexSaveData {
	u16 magic;
	u8 binder[HEX_BINDER_SLOTS];	/* copies owned of each card type */
	u8 claims[HEX_CLAIM_SLOTS];	/* times each district signature card was claimed */
	u8 pad[2];
};	/* 0x54 bytes */

extern struct HexSaveData gHexSave;

int HexBinder_Get(int type);
bool HexBinder_Add(int type);		/* false at HEX_BINDER_MAX or bad type */
bool HexBinder_Remove(int type);	/* false when none or bad type */
int HexBinder_Total(void);
int HexClaim_Get(int slot);
void HexClaim_Add(int slot);		/* saturates at 255; ignores bad slots */

void HexSave_NewGame(void);
void HexSave_Save(u8 *dst, const u32 size);
void HexSave_Load(u8 *src, const u32 size);

/*
 * Card art (Kernel/Wizardry/Minigames/Hex/Source/HexArt.c). The UI tiles and palettes are 4bpp, 1D OBJ order, row-major;
 * gHexPortraitKeep is the 32x32 portrait mask (Tools/hex/gen_hex_art.py).
 */
extern const u32 gHexFrameTiles[64][128];
extern const u32 gHexCardBackTiles[128];
extern const u32 gHexEmptyTiles[128];
extern const u32 gHexDigitTiles[11][8];
extern const u32 gHexDigitGreenTiles[11][8];
extern const u32 gHexScoreTiles[10][32];
extern const u32 gHexCursorTiles[2][32];
extern const u16 gHexFramePalRed[16];
extern const u16 gHexFramePalBlue[16];
extern const u32 gHexPortraitKeep[128];

/* The Hex screen's match state (struct HexScreenState in HexScreen.c), 0x280 bytes, include/link/config-memmap.s. */
extern u8 gHexScreenState[];

/* Prep screen "Hex" row (AtMenu.c). HexPrep.c starts the proc; HexScreen.c defines it. */
#define PREP_MAINMENU_HEX 12 /* prep_atmenu_item_index value of the Hex row; no vanilla row uses 12 */
struct ProcAtMenu;
void StartHexScreen_FromPrep(struct ProcAtMenu *pproc);
extern const struct ProcCmd ProcScr_HexScreen[];

/*
 * District NPCs (Data/Hex/HexNpcDecks.c, Kernel/Wizardry/Minigames/Hex/Source/HexDistrict.c): a map event starts a match against
 * the NPC of its district (StartDistrictHexASMC), with the map suspended around the screen.
 */
#define HEX_NO_CLAIM 0xFF
#define HEX_NO_DECK 0xFF
#define HEX_NPC_END 0xFF	/* chapter of the last gHexNpcDecks row */

/* A card of a district NPC's deck. Commons (claimSlot HEX_NO_CLAIM) can be won any number of times. A rare card can be won
 * `cap` times (counted in gHexSave.claims[claimSlot]); after that the NPC plays `fallback` (a tier 1 card) in its place. */
struct HexNpcCard {
	u8 type;
	u8 claimSlot;
	u8 cap;
	u8 fallback;
};

struct HexNpcDeck {
	u8 chapter;	/* district chapter id; HEX_NPC_END ends the table */
	u8 pid;		/* the NPC's character id */
	u8 aiLevel;	/* Hex_AiChoose level, 1 or 2 */
	u8 pad;
	u16 nameId;	/* text id of the NPC's name */
	u16 pad2;
	struct HexNpcCard cards[HEX_HAND];
};

extern const struct HexNpcDeck gHexNpcDecks[];

int HexNpc_FindDeck(int chapter, int pid);	/* index into gHexNpcDecks, or -1 */
int HexNpc_CardType(const struct HexNpcCard *card);	/* type, or fallback once the claim cap is reached */
bool HexNpc_IsRareLive(const struct HexNpcCard *card);	/* a rare card still below its cap */
void HexScreen_SetDistrictDeck(int deckIndex);	/* HEX_NO_DECK: the prep-menu screen */
void StartDistrictHexASMC(ProcPtr parent);

/*
 * District shops (Data/Hex/HexShops.c, Kernel/Wizardry/Minigames/Hex/Source/HexDistrict.c): a district's dealer sells an item for
 * N copies of ONE card type from the binder. The rows of one district are listed together. The Hex screen has no card exchange
 * any more; the shop is the vanilla FE8 shop screen in Hex mode (a later task).
 */
#define HEX_SHOP_END 0xFF	/* chapter of the last gHexShopItems row */

/*
 * One item of a district dealer. raw = 1: item is the item word, given as-is (tonics: (kind << 8) | ITEM_TONIC).
 * raw = 0: item is an item index, given with full uses via MakeNewItem.
 */
struct HexShopItem {
	u8 chapter;	/* district chapter id; HEX_SHOP_END ends the table */
	u8 raw;		/* 1: item is the item word; 0: item is an item index (MakeNewItem) */
	u16 item;	/* the item word (raw 1) or the item index (raw 0) */
	u8 costType;	/* the card type paid: an index of gHexCardTypes */
	u8 costCount;	/* copies of costType the item costs */
	u8 pad[2];
};	/* 8 bytes */

extern const struct HexShopItem gHexShopItems[];

int HexShop_Count(int chapter);				/* rows for this district */
const struct HexShopItem *HexShop_Get(int chapter, int n);	/* the n-th row for this district, NULL past the end */
int HexShop_ItemWord(const struct HexShopItem *row);	/* the item word to give */
const struct HexShopItem *HexShop_FindRow(int chapter, int itemWord);	/* the row whose HexShop_ItemWord is itemWord, NULL if none */
bool HexShop_CanAfford(const struct HexShopItem *row);	/* binder holds >= costCount of costType */
void HexShop_Pay(const struct HexShopItem *row);	/* takes costCount copies of costType from the binder */

/*
 * The Hex shop (Kernel/Wizardry/Minigames/Hex/Source/HexShopVanilla.c): a district dealer's talk event opens the vanilla shop screen
 * (StartShopScreen) with shopType SHOP_TYPE_HEX. That shop type is the Hex mode. 4 is past the vanilla shop types
 * (SHOP_TYPE_ARMORY..SHOP_TYPE_SECRET_SHOP, and SHOP_TYPE_MANAGE_ITEMS = 3 in bmshop.h), so no table or value overlaps it. items is
 * the list the shop was opened with (0-terminated; the row items, as StartShopScreen takes them). gHexShopRam is in RAM
 * (include/link/config-memmap.s), not saved.
 */
#define SHOP_TYPE_HEX 4	/* vanilla shop screen in Hex mode (HexShopVanilla.c) */

struct ProcShop;
struct Text;

struct HexShopRam {
	u16 items[13];	/* at most 12 rows and the 0 terminator */
	u8 pad[6];
};	/* 0x20 bytes */

extern struct HexShopRam gHexShopRam;

bool HexShop_IsActive(void);			/* the shop screen (gProcScr_Shop) is in Hex mode */
bool HexShop_IsHexProc(struct ProcShop *proc);	/* this shop proc is in Hex mode */
void HexShop_DrawPriceLine(struct Text *th, int item, u16 *dst);	/* a ware line in Hex mode: name, cost in cards, grey if unaffordable */
void HexShop_OnBuyPressed(struct ProcShop *proc);	/* A on a ware in Hex mode: cards short -> her line; else the confirm */
void HexShop_PayForItem(struct ProcShop *proc);	/* the bought ware's cards go to the dealer (HexShop_Pay) */
void StartDistrictHexShopASMC(ProcPtr parent);	/* a district dealer's talk event: opens the Hex shop */

#define FID_HEX_WITCH 0xAE	/* Hex shop witch portrait (Data/CustomPortraits/Portraits/hex_witch.png) */
#define FID_HEX_WITCH_SHOP 0xAF	/* Hex witch in the shopkeeper box (Data/CustomPortraits/Portraits/hex_witch_shop.png) */

/*
 * Monster minimugs (Data/Hex/HexMonsterArt.c, generated by Tools/hex/gen_monster_mugs.py from the FE8 monster class cards).
 * chibi is the 32x32 4bpp image, LZ77 type 0x10 (decompress it before use); pal is the card's 16 colours, index 0 transparent.
 */
#define HEX_MONSTER_COUNT 12

struct HexMonsterArt {
	const u8 *chibi;
	const u16 *pal;
};

extern const struct HexMonsterArt gHexMonsterArt[HEX_MONSTER_COUNT];

#endif /* CONFIG_HEX_CARDS */
