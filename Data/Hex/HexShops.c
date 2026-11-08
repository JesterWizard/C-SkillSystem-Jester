/* District shops: the dealer of each district sells an item for N copies of ONE card type from the binder (HexShop_CanAfford,
 * HexShop_Pay). One row per item, the rows of a district together. The card types are the indices of gHexCardTypes
 * (Data/Hex/HexCardTypes.c, the name of each card in the comment). raw = 1 gives the item word as-is (tonics:
 * (kind << 8) | ITEM_TONIC); raw = 0 gives MakeNewItem(item).
 */
#include "common-chax.h"
#include "constants/items.h"
#include "kernel/hex.h"

#ifdef CONFIG_HEX_CARDS

const struct HexShopItem gHexShopItems[] = {
	/* Serafew: Vulnerary (Ross x2) */
	{ 0x3B, 0, ITEM_VULNERARY, 0, 2, { 0, 0 } },
	/* Serafew: Elixir (Amelia x3) */
	{ 0x3B, 0, ITEM_ELIXIR, 1, 3, { 0, 0 } },
	/* Serafew: HP Tonic (Franz x3) */
	{ 0x3B, 1, (0 << 8) | ITEM_TONIC, 3, 3, { 0, 0 } },
	/* Serafew: STR Tonic (Colm x3) */
	{ 0x3B, 1, (1 << 8) | ITEM_TONIC, 4, 3, { 0, 0 } },
	/* Serafew: Booster HP (Kyle x1) */
	{ 0x3B, 0, ITEM_BOOSTER_HP, 12, 1, { 0, 0 } },
	/* Serafew: Booster LCK (Tethys x1) */
	{ 0x3B, 0, ITEM_BOOSTER_LCK, 16, 1, { 0, 0 } },
	/* Port Kiris: Purewater (Lute x2) */
	{ 0x3C, 0, ITEM_PUREWATER, 6, 2, { 0, 0 } },
	/* Port Kiris: DEF Tonic (Garcia x3) */
	{ 0x3C, 1, (6 << 8) | ITEM_TONIC, 7, 3, { 0, 0 } },
	/* Port Kiris: SPD Tonic (Vanessa x3) */
	{ 0x3C, 1, (4 << 8) | ITEM_TONIC, 11, 3, { 0, 0 } },
	/* Port Kiris: Booster SPD (Joshua x1) */
	{ 0x3C, 0, ITEM_BOOSTER_SPD, 15, 1, { 0, 0 } },
	/* Port Kiris: Booster DEF (Dozla x1) */
	{ 0x3C, 0, ITEM_BOOSTER_DEF, 14, 1, { 0, 0 } },
	/* Port Kiris: Booster SKL (Marisa x1) */
	{ 0x3C, 0, ITEM_BOOSTER_SKL, 18, 1, { 0, 0 } },
	/* Caer Pelyn: Elixir (Revenant x2) */
	{ 0x41, 0, ITEM_ELIXIR, 32, 2, { 0, 0 } },
	/* Caer Pelyn: RES Tonic (Bael x3) */
	{ 0x41, 1, (7 << 8) | ITEM_TONIC, 35, 3, { 0, 0 } },
	/* Caer Pelyn: MAG Tonic (Mogall x3) */
	{ 0x41, 1, (2 << 8) | ITEM_TONIC, 34, 3, { 0, 0 } },
	/* Caer Pelyn: Booster RES (Tana x1) */
	{ 0x41, 0, ITEM_BOOSTER_RES, 23, 1, { 0, 0 } },
	/* Caer Pelyn: Booster CON (Gerik x1) */
	{ 0x41, 0, ITEM_BOOSTER_CON, 17, 1, { 0, 0 } },
	/* Caer Pelyn: Booster POW (Cormag x1) */
	{ 0x41, 0, ITEM_BOOSTER_POW, 21, 1, { 0, 0 } },
	/* Jehanna: OMNI Tonic (Bael x4) */
	{ 0x42, 1, (8 << 8) | ITEM_TONIC, 35, 4, { 0, 0 } },
	/* Jehanna: Booster POW (Seth x1) */
	{ 0x42, 0, ITEM_BOOSTER_POW, 24, 1, { 0, 0 } },
	/* Jehanna: Booster MOV (Ephraim x1) */
	{ 0x42, 0, ITEM_BOOSTER_MOV, 27, 1, { 0, 0 } },
	/* Jehanna: Booster LCK (Eirika x1) */
	{ 0x42, 0, ITEM_BOOSTER_LCK, 31, 1, { 0, 0 } },
	/* Jehanna: Booster SKL (Lyon x1) */
	{ 0x42, 0, ITEM_BOOSTER_SKL, 30, 1, { 0, 0 } },
	/* Jehanna: Booster HP (Draco Zombie x1) */
	{ 0x42, 0, ITEM_BOOSTER_HP, 43, 1, { 0, 0 } },
	/* End of the table (HexShop_Get stops at this chapter). */
	{ HEX_SHOP_END, 0, 0, 0, 0, { 0, 0 } },
};

#endif /* CONFIG_HEX_CARDS */
