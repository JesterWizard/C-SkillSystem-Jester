#include <common-chax.h>
#include <gaiden-magic.h>
#include <skill-system.h>
#include <constants/skills.h>

const u8 gGaidenWeaponCostList[0x100] = {
	[ITEM_SWORD_KILLER] = 2,
	[ITEM_ANIMA_FIRE] = 2,
	[ITEM_ANIMA_THUNDER] = 2,
	[ITEM_DARK_FLUX] = 2,
	[ITEM_DARK_LUNA] = 3,
	[ITEM_DARK_NOSFERATU] = 2,
	[ITEM_STAFF_HEAL] = 2,
	[ITEM_STAFF_WHITE_WIND] = 8,
	[ITEM_STAFF_GREEN_WIND] = 8,
	[ITEM_STAFF_KIRIN] = 8,
	[ITEM_STAFF_PHYSIC] = 5,
	[ITEM_STAFF_SLEEP] = 5,
	[ITEM_STAFF_LATONA] = 10,
	[ITEM_STAFF_WARP] = 7,
	[ITEM_STAFF_BERSERK] = 8,
	[ITEM_SWORD_AUDHULMA] = 10,
	[ITEM_AXE_KILLER] = 3,
	[ITEM_DARK_GLEIPNIR] = 9,
};

/**
 * Three Houses-style magic (gaiden_magic_three_houses): uses per chapter.
 * 0 = fall back to the item's max uses. Unbreakable items are unlimited.
 */
const u8 gGaidenMagicUsesList[0x100] = {
	/* Anima */
	[ITEM_ANIMA_FIRE] = 10,
	[ITEM_ANIMA_THUNDER] = 7,
	[ITEM_ANIMA_ELFIRE] = 5,
	[ITEM_ANIMA_FIMBULVETR] = 5,
	[ITEM_ANIMA_AIRCALIBUR] = 5,
	[ITEM_ANIMA_EXCALIBUR] = 2,
	[ITEM_ANIMA_FORBLAZE] = 2,
	[ITEM_ANIMA_BOLTING] = 1,

	/* Light */
	[ITEM_LIGHT_LIGHTNING] = 10,
	[ITEM_LIGHT_SHINE] = 7,
	[ITEM_LIGHT_DIVINE] = 5,
	[ITEM_LIGHT_AURA] = 3,
	[ITEM_LIGHT_LUCE] = 2,
	[ITEM_LIGHT_IVALDI] = 2,
	[ITEM_LIGHT_PURGE] = 1,

	/* Dark */
	[ITEM_DARK_FLUX] = 6,
	[ITEM_DARK_NOSFERATU] = 5,
	[ITEM_DARK_LUNA] = 4,
	[ITEM_DARK_FENRIR] = 3,
	[ITEM_DARK_GLEIPNIR] = 2,
	[ITEM_DARK_NAGLFAR] = 2,
	[ITEM_DARK_ECLIPSE] = 1,

	/* Staves */
	[ITEM_STAFF_HEAL] = 10,
	[ITEM_STAFF_MEND] = 5,
	[ITEM_STAFF_PHYSIC] = 5,
	[ITEM_STAFF_RESTORE] = 5,
	[ITEM_STAFF_BARRIER] = 5,
	[ITEM_STAFF_TORCH] = 5,
	[ITEM_STAFF_UNLOCK] = 5,
	[ITEM_STAFF_RECOVER] = 3,
	[ITEM_STAFF_SILENCE] = 3,
	[ITEM_STAFF_SLEEP] = 3,
	[ITEM_STAFF_RESCUE] = 3,
	[ITEM_STAFF_BERSERK] = 2,
	[ITEM_STAFF_WHITE_WIND] = 2,
	[ITEM_STAFF_GREEN_WIND] = 2,
	[ITEM_STAFF_KIRIN] = 2,
	[ITEM_STAFF_FORTIFY] = 1,
	[ITEM_STAFF_WARP] = 1,
	[ITEM_STAFF_REPAIR] = 1,
	[ITEM_STAFF_LATONA] = 1,

	/* Non-magic B.Mag (gaiden_magic_must_be_magic = false) */
	[ITEM_SWORD_KILLER] = 5,
	[ITEM_AXE_KILLER] = 5,
	[ITEM_SWORD_AUDHULMA] = 2,
};

/* Like the Black/White Magic Uses x2 class abilities in Three Houses */
const u8 gGaidenMagicClassUsesBonus[0x100] = {
	[CLASS_SAGE] = GAIDEN_MAGIC_USES_BMAG_X2,
	[CLASS_SAGE_F] = GAIDEN_MAGIC_USES_BMAG_X2,
	[CLASS_DRUID] = GAIDEN_MAGIC_USES_BMAG_X2,
	[CLASS_DRUID_F] = GAIDEN_MAGIC_USES_BMAG_X2,
	[CLASS_BISHOP] = GAIDEN_MAGIC_USES_WMAG_X2,
	[CLASS_BISHOP_F] = GAIDEN_MAGIC_USES_WMAG_X2,
	[CLASS_VALKYRIE] = GAIDEN_MAGIC_USES_WMAG_X2,
	[CLASS_NECROMANCER] = GAIDEN_MAGIC_USES_BMAG_X2 | GAIDEN_MAGIC_USES_WMAG_X2,
};

/**
 * Three Houses-style magic: spells learned from weapon ranks.
 *
 * Each entry is { rank, spell }. The rank is checked against the unit's rank
 * in the spell's own weapon type (Fire -> Anima, Heal -> Staff), so a unit
 * needs at least E rank in a type to learn its spells.
 * A unit can hold up to 7 B.Mag and 7 W.Mag (the stat page shows 5 of each).
 */
const struct GaidenMagicRankEnt gGaidenMagicRankDefaultList[] = {
	/* Anima */
	{ WPN_LEVEL_E, ITEM_ANIMA_FIRE },
	{ WPN_LEVEL_D, ITEM_ANIMA_THUNDER },
	{ WPN_LEVEL_C, ITEM_ANIMA_ELFIRE },
	{ WPN_LEVEL_B, ITEM_ANIMA_BOLTING },
	{ WPN_LEVEL_A, ITEM_ANIMA_FIMBULVETR },

	/* Light */
	{ WPN_LEVEL_E, ITEM_LIGHT_LIGHTNING },
	{ WPN_LEVEL_D, ITEM_LIGHT_SHINE },
	{ WPN_LEVEL_C, ITEM_LIGHT_DIVINE },
	{ WPN_LEVEL_B, ITEM_LIGHT_PURGE },
	{ WPN_LEVEL_A, ITEM_LIGHT_AURA },

	/* Dark */
	{ WPN_LEVEL_E, ITEM_DARK_FLUX },
	{ WPN_LEVEL_C, ITEM_DARK_LUNA },
	{ WPN_LEVEL_C, ITEM_DARK_NOSFERATU },
	{ WPN_LEVEL_B, ITEM_DARK_ECLIPSE },
	{ WPN_LEVEL_A, ITEM_DARK_FENRIR },

	/* Staff */
	{ WPN_LEVEL_E, ITEM_STAFF_HEAL },
	{ WPN_LEVEL_D, ITEM_STAFF_MEND },
	{ WPN_LEVEL_C, ITEM_STAFF_RESTORE },
	{ WPN_LEVEL_C, ITEM_STAFF_RECOVER },
	{ WPN_LEVEL_B, ITEM_STAFF_PHYSIC },
	{ WPN_LEVEL_B, ITEM_STAFF_RESCUE },
	{ WPN_LEVEL_A, ITEM_STAFF_FORTIFY },
	{ 0 }
};

/* Personal lists replace the default list, like each student's list in Three Houses */
const struct GaidenMagicRankConfig gGaidenMagicRankPList[0x100] = {
	[CHARACTER_LUTE] = { .ent = (const struct GaidenMagicRankEnt []) {
		{ WPN_LEVEL_E, ITEM_ANIMA_FIRE },
		{ WPN_LEVEL_D, ITEM_ANIMA_THUNDER },
		{ WPN_LEVEL_C, ITEM_ANIMA_ELFIRE },
		{ WPN_LEVEL_B, ITEM_ANIMA_BOLTING },
		{ WPN_LEVEL_A, ITEM_ANIMA_FIMBULVETR },
		{ WPN_LEVEL_S, ITEM_ANIMA_EXCALIBUR },
		{ 0 }
	}},
	[CHARACTER_KNOLL] = { .ent = (const struct GaidenMagicRankEnt []) {
		{ WPN_LEVEL_E, ITEM_DARK_FLUX },
		{ WPN_LEVEL_C, ITEM_DARK_NOSFERATU },
		{ WPN_LEVEL_C, ITEM_DARK_LUNA },
		{ WPN_LEVEL_B, ITEM_DARK_ECLIPSE },
		{ WPN_LEVEL_A, ITEM_DARK_FENRIR },
		{ WPN_LEVEL_S, ITEM_DARK_GLEIPNIR },
		{ 0 }
	}},
	[CHARACTER_LARACHEL] = { .ent = (const struct GaidenMagicRankEnt []) {
		{ WPN_LEVEL_E, ITEM_STAFF_HEAL },
		{ WPN_LEVEL_D, ITEM_STAFF_MEND },
		{ WPN_LEVEL_C, ITEM_STAFF_RESTORE },
		{ WPN_LEVEL_B, ITEM_STAFF_PHYSIC },
		{ WPN_LEVEL_A, ITEM_STAFF_FORTIFY },
		{ WPN_LEVEL_E, ITEM_LIGHT_LIGHTNING },
		{ WPN_LEVEL_D, ITEM_LIGHT_SHINE },
		{ WPN_LEVEL_C, ITEM_LIGHT_DIVINE },
		{ WPN_LEVEL_S, ITEM_LIGHT_IVALDI },
		{ 0 }
	}},
};

struct GaidenPinfoConfig const gGaidenPinfoConfigList[0x100] = {
	[CHARACTER_EIRIKA] = { .ent = (const struct GaidenPinfoConfigEnt []) {
		// { 1, ITEM_SWORD_KILLER },
		// { 1, ITEM_SWORD_AUDHULMA},
		// { 1, ITEM_ANIMA_THUNDER },
		// { 1, ITEM_AXE_KILLER },
		// { 1, ITEM_DARK_GLEIPNIR },
		// { 1, ITEM_STAFF_SLEEP },
		// { 1, ITEM_STAFF_LATONA},
		// { 1, ITEM_STAFF_BERSERK },
		// { 1, ITEM_STAFF_PHYSIC },
		// { 1, ITEM_STAFF_GREEN_WIND },
		{ 0 }
	}},
	[CHARACTER_NATASHA] = { .ent = (const struct GaidenPinfoConfigEnt []) {
		// { 1, ITEM_STAFF_HEAL },
		// { 1, ITEM_DARK_LUNA },
		// { 1, ITEM_STAFF_SLEEP },
		{ 0 }
	}},
	[CHARACTER_SALEH] = { .ent = (const struct GaidenPinfoConfigEnt []) {
		// { 1, ITEM_DARK_NOSFERATU },
		// { 1, ITEM_STAFF_PHYSIC },
		// { 1, ITEM_STAFF_HEAL },
		{ 0 }
	}},
	[CHARACTER_SELENA] = { .ent = (const struct GaidenPinfoConfigEnt []) {
		// { 1, ITEM_STAFF_PHYSIC },
		{ 0 }
	}},
	[CHARACTER_NOVALA] = { .ent = (const struct GaidenPinfoConfigEnt []) {
		// { 1, ITEM_DARK_FLUX },
		{ 0 }
	}},
	[CHARACTER_RIEV] = { .ent = (const struct GaidenPinfoConfigEnt []) {
		// { 1, ITEM_STAFF_SLEEP },
		{ 0 }
	}},
};

// This config is valid only when CONFIG_GAIDEN_EXT_CONF_EN is set!
const struct GaidenChaxConfigEnt gGaidenChaxConfigs[] = {
#if (defined(SID_Latona) && COMMON_SKILL_VALID(SID_Latona))
	{
		.iid = ITEM_STAFF_LATONA,
		.skill = SID_Latona,
	},
#endif
	{ 0 },
};
