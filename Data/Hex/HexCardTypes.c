/* Hex card types: 44 cards, index = card type (the uses byte of the card item word).
 * attack, maxHp, reach, tier and starter are copied from the Minish Cap mod's gHexTiles
 * (src_custom/hexArt.c, by row index). pid is the FE8 character the card is named after.
 * Types 32..43 are monsters (HEX_MONSTER_FIRST): pid 0, jid is the class the card is named after,
 * and the art is gHexMonsterArt (Data/Hex/HexMonsterArt.c) in the same order. */
#include "common-chax.h"
#include "constants/characters.h"
#include "constants/classes.h"
#include "kernel/hex.h"

#ifdef CONFIG_HEX_CARDS

const struct HexCardType gHexCardTypes[HEX_CARD_TYPES] = {
	[0] = { .attack = 1, .maxHp = 2, .reach = 0x16, .tier = 1, .starter = 2, .pid = CHARACTER_ROSS }, /* Ross */
	[1] = { .attack = 1, .maxHp = 3, .reach = 0x03, .tier = 1, .starter = 2, .pid = CHARACTER_AMELIA }, /* Amelia */
	[2] = { .attack = 3, .maxHp = 1, .reach = 0x24, .tier = 1, .starter = 1, .pid = CHARACTER_EWAN }, /* Ewan */
	[3] = { .attack = 2, .maxHp = 2, .reach = 0x18, .tier = 1, .starter = 1, .pid = CHARACTER_FRANZ }, /* Franz */
	[4] = { .attack = 2, .maxHp = 1, .reach = 0x0B, .tier = 1, .starter = 0, .pid = CHARACTER_COLM }, /* Colm */
	[5] = { .attack = 1, .maxHp = 1, .reach = 0x1B, .tier = 1, .starter = 1, .pid = CHARACTER_NEIMI }, /* Neimi */
	[6] = { .attack = 4, .maxHp = 1, .reach = 0x04, .tier = 1, .starter = 0, .pid = CHARACTER_LUTE }, /* Lute */
	[7] = { .attack = 2, .maxHp = 3, .reach = 0x20, .tier = 1, .starter = 0, .pid = CHARACTER_GARCIA }, /* Garcia */
	[8] = { .attack = 1, .maxHp = 2, .reach = 0x0E, .tier = 1, .starter = 0, .pid = CHARACTER_NATASHA }, /* Natasha */
	[9] = { .attack = 2, .maxHp = 3, .reach = 0x02, .tier = 1, .starter = 0, .pid = CHARACTER_MOULDER }, /* Moulder */
	[10] = { .attack = 3, .maxHp = 2, .reach = 0x08, .tier = 1, .starter = 0, .pid = CHARACTER_GILLIAM }, /* Gilliam */
	[11] = { .attack = 2, .maxHp = 1, .reach = 0x25, .tier = 1, .starter = 0, .pid = CHARACTER_VANESSA }, /* Vanessa */
	[12] = { .attack = 3, .maxHp = 2, .reach = 0x07, .tier = 2, .starter = 0, .pid = CHARACTER_KYLE }, /* Kyle */
	[13] = { .attack = 3, .maxHp = 3, .reach = 0x24, .tier = 2, .starter = 0, .pid = CHARACTER_FORDE }, /* Forde */
	[14] = { .attack = 1, .maxHp = 5, .reach = 0x18, .tier = 2, .starter = 0, .pid = CHARACTER_DOZLA }, /* Dozla */
	[15] = { .attack = 4, .maxHp = 2, .reach = 0x12, .tier = 2, .starter = 0, .pid = CHARACTER_JOSHUA }, /* Joshua */
	[16] = { .attack = 2, .maxHp = 2, .reach = 0x1B, .tier = 2, .starter = 0, .pid = CHARACTER_TETHYS }, /* Tethys */
	[17] = { .attack = 2, .maxHp = 4, .reach = 0x03, .tier = 2, .starter = 0, .pid = CHARACTER_GERIK }, /* Gerik */
	[18] = { .attack = 4, .maxHp = 3, .reach = 0x04, .tier = 2, .starter = 0, .pid = CHARACTER_MARISA }, /* Marisa */
	[19] = { .attack = 2, .maxHp = 2, .reach = 0x2D, .tier = 2, .starter = 0, .pid = CHARACTER_LARACHEL }, /* L'Arachel */
	[20] = { .attack = 3, .maxHp = 2, .reach = 0x2A, .tier = 2, .starter = 0, .pid = CHARACTER_INNES }, /* Innes */
	[21] = { .attack = 3, .maxHp = 4, .reach = 0x02, .tier = 2, .starter = 0, .pid = CHARACTER_CORMAG }, /* Cormag */
	[22] = { .attack = 4, .maxHp = 1, .reach = 0x0E, .tier = 2, .starter = 0, .pid = CHARACTER_SALEH }, /* Saleh */
	[23] = { .attack = 2, .maxHp = 4, .reach = 0x11, .tier = 2, .starter = 0, .pid = CHARACTER_TANA }, /* Tana */
	[24] = { .attack = 4, .maxHp = 4, .reach = 0x03, .tier = 3, .starter = 0, .pid = CHARACTER_SETH }, /* Seth */
	[25] = { .attack = 2, .maxHp = 4, .reach = 0x27, .tier = 3, .starter = 0, .pid = CHARACTER_MYRRH }, /* Myrrh */
	[26] = { .attack = 2, .maxHp = 5, .reach = 0x07, .tier = 3, .starter = 0, .pid = CHARACTER_DUESSEL }, /* Duessel */
	[27] = { .attack = 4, .maxHp = 4, .reach = 0x24, .tier = 3, .starter = 0, .pid = CHARACTER_EPHRAIM }, /* Ephraim */
	[28] = { .attack = 2, .maxHp = 5, .reach = 0x15, .tier = 3, .starter = 0, .pid = CHARACTER_GLEN }, /* Glen */
	[29] = { .attack = 2, .maxHp = 5, .reach = 0x29, .tier = 3, .starter = 0, .pid = CHARACTER_VALTER }, /* Valter */
	[30] = { .attack = 4, .maxHp = 2, .reach = 0x1B, .tier = 3, .starter = 0, .pid = CHARACTER_LYON }, /* Lyon */
	[31] = { .attack = 3, .maxHp = 5, .reach = 0x03, .tier = 3, .starter = 0, .pid = CHARACTER_EIRIKA }, /* Eirika */
	[32] = { .attack = 1, .maxHp = 3, .reach = 0x0C, .tier = 1, .starter = 0, .pid = 0, .jid = CLASS_REVENANT }, /* Revenant */
	[33] = { .attack = 2, .maxHp = 2, .reach = 0x21, .tier = 1, .starter = 0, .pid = 0, .jid = CLASS_BONEWALKER }, /* Bonewalker */
	[34] = { .attack = 3, .maxHp = 1, .reach = 0x12, .tier = 1, .starter = 0, .pid = 0, .jid = CLASS_MOGALL }, /* Mogall */
	[35] = { .attack = 2, .maxHp = 2, .reach = 0x0D, .tier = 1, .starter = 0, .pid = 0, .jid = CLASS_BAEL }, /* Bael */
	[36] = { .attack = 3, .maxHp = 1, .reach = 0x19, .tier = 1, .starter = 0, .pid = 0, .jid = CLASS_GWYLLGI }, /* Gwyllgi */
	[37] = { .attack = 3, .maxHp = 3, .reach = 0x21, .tier = 2, .starter = 0, .pid = 0, .jid = CLASS_WIGHT }, /* Wight */
	[38] = { .attack = 3, .maxHp = 3, .reach = 0x0C, .tier = 2, .starter = 0, .pid = 0, .jid = CLASS_TARVOS }, /* Tarvos */
	[39] = { .attack = 4, .maxHp = 2, .reach = 0x15, .tier = 2, .starter = 0, .pid = 0, .jid = CLASS_ARCH_MOGALL }, /* Arch Mogall */
	[40] = { .attack = 4, .maxHp = 3, .reach = 0x01, .tier = 2, .starter = 0, .pid = 0, .jid = CLASS_CYCLOPS }, /* Cyclops */
	[41] = { .attack = 2, .maxHp = 3, .reach = 0x36, .tier = 2, .starter = 0, .pid = 0, .jid = CLASS_GARGOYLE }, /* Gargoyle */
	[42] = { .attack = 3, .maxHp = 4, .reach = 0x2D, .tier = 3, .starter = 0, .pid = 0, .jid = CLASS_DEATHGOYLE }, /* Deathgoyle */
	[43] = { .attack = 4, .maxHp = 5, .reach = 0x21, .tier = 3, .starter = 0, .pid = 0, .jid = CLASS_DRACO_ZOMBIE }, /* Draco Zombie */
};

#endif /* CONFIG_HEX_CARDS */
