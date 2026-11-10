#include "common-chax.h"
#include "constants/items.h"

/*
 * World-map vendors' stock (VendorStock.event points gWMNodeData[node].vendor here).
 *
 * Vanilla's vendors sell mostly tomes and staves. With Three Houses magic (gaiden_magic_three_houses) no shop sells
 * those (StartShopScreen skips them), so Serafew's vendor sold only a Vulnerary and the others 3 to 5 items (#704).
 * These lists hold what a vendor can sell either way, and grow with the story: the basics at Serafew, Elixirs and
 * Pure Water from Port Kiris, Mines from Caer Pelyn, Light Runes in the last nodes. Each ends with 0.
 */

/* Serafew (node 5) */
const u16 gWmVendorStock_Serafew[] = {
	ITEM_VULNERARY, ITEM_ANTITOXIN, ITEM_TORCH, ITEM_CHESTKEY, ITEM_DOORKEY, 0,
};

/* Port Kiris (node 9), and node 15, which shared vanilla's list */
const u16 gWmVendorStock_PortKiris[] = {
	ITEM_VULNERARY, ITEM_ELIXIR, ITEM_PUREWATER, ITEM_ANTITOXIN, ITEM_TORCH, ITEM_CHESTKEY, ITEM_DOORKEY, 0,
};

/* Caer Pelyn (node 11), and node 16, which shared vanilla's list */
const u16 gWmVendorStock_CaerPelyn[] = {
	ITEM_VULNERARY, ITEM_ELIXIR, ITEM_PUREWATER, ITEM_ANTITOXIN, ITEM_TORCH, ITEM_CHESTKEY, ITEM_DOORKEY, ITEM_MINE, 0,
};

/* Jehanna Hall's second node (19), Narube River (21) and Rausten Court (23) */
const u16 gWmVendorStock_Late[] = {
	ITEM_VULNERARY, ITEM_ELIXIR, ITEM_PUREWATER, ITEM_ANTITOXIN, ITEM_TORCH, ITEM_CHESTKEY, ITEM_DOORKEY, ITEM_MINE,
	ITEM_LIGHTRUNE, 0,
};
