#include "common-chax.h"
#include "weapon-slots.h"
#include "bmitem.h"
#include "constants/classes.h"

/**
 * Sparse overrides only. Classes absent from this table keep the vanilla
 * identity mapping (slot N <=> weapon type N).
 *
 * Thief line keeps swords in slot 0 and adds knives in free slot 1.
 * baseRanks[] optionally supplies starting WEXP by physical slot. A zero
 * entry falls back to the corresponding ClassData.baseRanks[] value.
 */
const struct ClassWeaponSlotConf gClassWeaponSlotConf[] = {
	{
		.jid = CLASS_THIEF,
		.wtypes = {
			[0] = ITYPE_SWORD,
			[1] = ITYPE_KNIFE,
			[2] = WEAPON_SLOT_NONE,
			[3] = WEAPON_SLOT_NONE,
			[4] = WEAPON_SLOT_NONE,
			[5] = WEAPON_SLOT_NONE,
			[6] = WEAPON_SLOT_NONE,
			[7] = WEAPON_SLOT_NONE,
		},
		.baseRanks = {
			[1] = WPN_EXP_E, /* slot 1: knife */
		},
	},
	{
		.jid = CLASS_ASSASSIN,
		.wtypes = {
			[0] = ITYPE_SWORD,
			[1] = ITYPE_KNIFE,
			[2] = WEAPON_SLOT_NONE,
			[3] = WEAPON_SLOT_NONE,
			[4] = WEAPON_SLOT_NONE,
			[5] = WEAPON_SLOT_NONE,
			[6] = WEAPON_SLOT_NONE,
			[7] = WEAPON_SLOT_NONE,
		},
		.baseRanks = {
			[1] = WPN_EXP_E, /* slot 1: knife */
		},
	},
	{
		.jid = CLASS_ASSASSIN_F,
		.wtypes = {
			[0] = ITYPE_SWORD,
			[1] = ITYPE_KNIFE,
			[2] = WEAPON_SLOT_NONE,
			[3] = WEAPON_SLOT_NONE,
			[4] = WEAPON_SLOT_NONE,
			[5] = WEAPON_SLOT_NONE,
			[6] = WEAPON_SLOT_NONE,
			[7] = WEAPON_SLOT_NONE,
		},
		.baseRanks = {
			[1] = WPN_EXP_E, /* slot 1: knife */
		},
	},
	{
		.jid = CLASS_ROGUE,
		.wtypes = {
			[0] = ITYPE_SWORD,
			[1] = ITYPE_KNIFE,
			[2] = WEAPON_SLOT_NONE,
			[3] = WEAPON_SLOT_NONE,
			[4] = WEAPON_SLOT_NONE,
			[5] = WEAPON_SLOT_NONE,
			[6] = WEAPON_SLOT_NONE,
			[7] = WEAPON_SLOT_NONE,
		},
		.baseRanks = {
			[1] = WPN_EXP_E, /* slot 1: knife */
		},
	},
	/* Sentinel */
	{ .jid = 0 },
};
