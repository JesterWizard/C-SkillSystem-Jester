#include "common-chax.h"
#include "weapon-slots.h"
#include "bmitem.h"

/**
 * Sparse overrides only. Classes absent from this table keep the vanilla
 * identity mapping (slot N <=> weapon type N).
 *
 * Example — give Thief knives in the sword slot instead of swords:
 *
 *  {
 *      .jid = CLASS_THIEF,
 *      .wtypes = {
 *          [0] = ITYPE_KNIFE, // occupies physical ranks[0]
 *          [1] = WEAPON_SLOT_NONE,
 *          [2] = WEAPON_SLOT_NONE,
 *          [3] = WEAPON_SLOT_NONE,
 *          [4] = WEAPON_SLOT_NONE,
 *          [5] = WEAPON_SLOT_NONE,
 *          [6] = WEAPON_SLOT_NONE,
 *          [7] = WEAPON_SLOT_NONE,
 *      },
 *  },
 *
 * Pair that with ClassData.baseRanks[0] = WPN_EXP_E (slot 0 starting EXP).
 * Character baseRanks remain type-indexed for types 0-7 only.
 */
const struct ClassWeaponSlotConf gClassWeaponSlotConf[] = {
	/* Sentinel */
	{ 0, { 0 } },
};
