/* District NPC decks: the Hex opponent of each district's NPC (HexScreen_SetNpcOpponent). One row per (chapter, NPC).
 * The card types are the indices of gHexCardTypes (Data/Hex/HexCardTypes.c, the name of each card in the comment).
 * HEX_NPC_C is a common card (won any number of times); HEX_NPC_R is a rare card with a claim slot and cap (HexNpc_CardType).
 * Claim slots 18 and 19 are free (no deck uses them).
 */
#include "common-chax.h"
#include "constants/texts.h"
#include "kernel/hex.h"

#ifdef CONFIG_HEX_CARDS

#define HEX_NPC_C(type) { (type), HEX_NO_CLAIM, 0, 0 }
#define HEX_NPC_R(type, slot, cap, fallback) { (type), (slot), (cap), (fallback) }

const struct HexNpcDeck gHexNpcDecks[] = {
	/* Serafew, Orrin */
	{ 0x3B, 0xFA, 1, 0, MSG_HEXNPC_SERAFEW_A, 0, {
		HEX_NPC_R(12, 0, 2, 0),		/* Kyle */
		HEX_NPC_R(13, 1, 2, 3),		/* Forde */
		HEX_NPC_C(0),			/* Ross */
		HEX_NPC_C(1),			/* Amelia */
		HEX_NPC_C(2),			/* Ewan */
		HEX_NPC_C(3),			/* Franz */
		HEX_NPC_C(4),			/* Colm */
	} },
	/* Serafew, Della */
	{ 0x3B, 0xFB, 1, 0, MSG_HEXNPC_SERAFEW_B, 0, {
		HEX_NPC_R(16, 2, 3, 5),		/* Tethys */
		HEX_NPC_C(5),			/* Neimi */
		HEX_NPC_C(6),			/* Lute */
		HEX_NPC_C(7),			/* Garcia */
		HEX_NPC_C(8),			/* Natasha */
		HEX_NPC_C(9),			/* Moulder */
		HEX_NPC_C(11),			/* Vanessa */
	} },
	/* Port Kiris, Bram */
	{ 0x3C, 0xFA, 1, 0, MSG_HEXNPC_PORT_KIRIS_A, 0, {
		HEX_NPC_R(15, 3, 2, 6),		/* Joshua */
		HEX_NPC_R(18, 4, 2, 10),	/* Marisa */
		HEX_NPC_C(6),			/* Lute */
		HEX_NPC_C(10),			/* Gilliam */
		HEX_NPC_C(11),			/* Vanessa */
		HEX_NPC_C(7),			/* Garcia */
		HEX_NPC_C(0),			/* Ross */
	} },
	/* Port Kiris, Corra */
	{ 0x3C, 0xFB, 2, 0, MSG_HEXNPC_PORT_KIRIS_B, 0, {
		HEX_NPC_R(19, 5, 2, 1),		/* L'Arachel */
		HEX_NPC_R(14, 6, 2, 9),		/* Dozla */
		HEX_NPC_R(17, 7, 1, 7),		/* Gerik */
		HEX_NPC_C(8),			/* Natasha */
		HEX_NPC_C(9),			/* Moulder */
		HEX_NPC_C(4),			/* Colm */
		HEX_NPC_C(2),			/* Ewan */
	} },
	/* Caer Pelyn, Tamsin */
	{ 0x3F, 0xFA, 2, 0, MSG_HEXNPC_CAER_PELYN_A, 0, {
		HEX_NPC_R(22, 8, 2, 2),		/* Saleh */
		HEX_NPC_R(20, 9, 2, 6),		/* Innes */
		HEX_NPC_R(23, 10, 2, 8),	/* Tana */
		HEX_NPC_R(25, 11, 1, 1),	/* Myrrh */
		HEX_NPC_C(32),			/* Revenant */
		HEX_NPC_C(33),			/* Bonewalker */
		HEX_NPC_C(34),			/* Mogall */
	} },
	/* Caer Pelyn, Ewlyn */
	{ 0x3F, 0xFB, 2, 0, MSG_HEXNPC_CAER_PELYN_B, 0, {
		HEX_NPC_R(21, 12, 2, 9),	/* Cormag */
		HEX_NPC_R(26, 13, 1, 7),	/* Duessel */
		HEX_NPC_R(17, 14, 2, 10),	/* Gerik */
		HEX_NPC_R(37, 25, 2, 33),	/* Wight */
		HEX_NPC_R(41, 26, 2, 36),	/* Gargoyle */
		HEX_NPC_C(35),			/* Bael */
		HEX_NPC_C(36),			/* Gwyllgi */
	} },
	/* Jehanna, Rashid */
	{ 0x40, 0xFA, 2, 0, MSG_HEXNPC_JEHANNA_A, 0, {
		HEX_NPC_R(24, 15, 1, 3),	/* Seth */
		HEX_NPC_R(27, 16, 1, 0),	/* Ephraim */
		HEX_NPC_R(28, 17, 1, 4),	/* Glen */
		HEX_NPC_R(38, 27, 2, 35),	/* Tarvos */
		HEX_NPC_R(40, 28, 1, 32),	/* Cyclops */
		HEX_NPC_R(39, 29, 2, 34),	/* Arch Mogall */
		HEX_NPC_C(35),			/* Bael */
	} },
	/* Jehanna, Samira */
	{ 0x40, 0xFB, 2, 0, MSG_HEXNPC_JEHANNA_B, 0, {
		HEX_NPC_R(29, 20, 1, 10),	/* Valter */
		HEX_NPC_R(30, 21, 1, 9),	/* Lyon */
		HEX_NPC_R(31, 22, 1, 1),	/* Eirika */
		HEX_NPC_R(26, 23, 1, 7),	/* Duessel */
		HEX_NPC_R(25, 24, 1, 5),	/* Myrrh */
		HEX_NPC_R(42, 30, 1, 34),	/* Deathgoyle */
		HEX_NPC_R(43, 31, 1, 36),	/* Draco Zombie */
	} },
	/* End of the table (HexNpc_FindDeck stops at this chapter). */
	{ HEX_NPC_END, 0, 0, 0, 0, 0, { { 0, 0, 0, 0 } } },
};

#endif /* CONFIG_HEX_CARDS */
