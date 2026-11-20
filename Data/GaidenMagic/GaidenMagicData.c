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

/* MP spent per cast, whatever the uses system (mp_system only) */
const u8 gGaidenMagicMpCostList[0x100] = {
	[ITEM_ANIMA_FREEZEBLINK] = 30,
	[ITEM_ANIMA_PROMINENCE] = 30,
	[ITEM_ANIMA_TEMPEST] = 30,
	[ITEM_DARK_GRAVITY] = 30,
	[ITEM_STAFF_UNICORN] = 30,
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

/**
 * Personal spell lists by level: { level, spell }. A unit knows every spell up to its level.
 *
 * The level is the unit's level plus its earlier levels: its hidden level (the levels before a promotion, as the
 * BWL records them), or 20 for a promoted unit if that is more. So 21 is a promoted unit's level 1 (as before:
 * promoted classes count 20 levels), and a trainee keeps counting after its first promotion.
 * With gaiden_magic_learn_by_level on (the default), these lists are how the player's units learn spells, and
 * crossing a level shows the New Spell window once the battle is over (GaidenMagic/Source/LearnWindow.c).
 * Bit 7 of the level (0x80) means "only once promoted".
 * A unit holds up to 7 B.Mag and 7 W.Mag.
 */
struct GaidenPinfoConfig const gGaidenPinfoConfigList[0x100] = {
	/* Mage, then Sage or Mage Knight: anima */
	[CHARACTER_LUTE] = { .ent = (const struct GaidenPinfoConfigEnt []) {
		{ 1,  ITEM_ANIMA_FIRE },
		{ 4,  ITEM_ANIMA_THUNDER },
		{ 8,  ITEM_ANIMA_ELFIRE },
		{ 13, ITEM_ANIMA_FIMBULVETR },
		{ 18, ITEM_ANIMA_BOLTING },
		{ 21, ITEM_STAFF_HEAL },
		{ 24, ITEM_ANIMA_AIRCALIBUR },
		{ 30, ITEM_ANIMA_EXCALIBUR },
		{ 0 }
	}},
	/* Monk, then Bishop or Sage: light, and healing */
	[CHARACTER_ARTUR] = { .ent = (const struct GaidenPinfoConfigEnt []) {
		{ 1,  ITEM_LIGHT_LIGHTNING },
		{ 5,  ITEM_LIGHT_SHINE },
		{ 9,  ITEM_STAFF_HEAL },
		{ 12, ITEM_LIGHT_DIVINE },
		{ 17, ITEM_LIGHT_PURGE },
		{ 21, ITEM_STAFF_MEND },
		{ 25, ITEM_LIGHT_AURA },
		{ 32, ITEM_LIGHT_LUCE },
		{ 0 }
	}},
	/* Cleric, then Bishop or Valkyrie: staves, and light once promoted */
	[CHARACTER_NATASHA] = { .ent = (const struct GaidenPinfoConfigEnt []) {
		{ 1,  ITEM_STAFF_HEAL },
		{ 4,  ITEM_STAFF_MEND },
		{ 8,  ITEM_STAFF_RESTORE },
		{ 12, ITEM_STAFF_PHYSIC },
		{ 16, ITEM_STAFF_RESCUE },
		{ 21, ITEM_LIGHT_LIGHTNING },
		{ 23, ITEM_STAFF_RECOVER },
		{ 26, ITEM_LIGHT_SHINE },
		{ 30, ITEM_STAFF_FORTIFY },
		{ 0 }
	}},
	/* Shaman, then Druid or Summoner: dark */
	[CHARACTER_KNOLL] = { .ent = (const struct GaidenPinfoConfigEnt []) {
		{ 1,  ITEM_DARK_FLUX },
		{ 5,  ITEM_DARK_LUNA },
		{ 9,  ITEM_DARK_NOSFERATU },
		{ 14, ITEM_DARK_FENRIR },
		{ 20, ITEM_DARK_ECLIPSE },
		{ 21, ITEM_STAFF_HEAL },
		{ 26, ITEM_DARK_GLEIPNIR },
		{ 32, ITEM_DARK_NAGLFAR },
		{ 0 }
	}},
	/* Priest, then Bishop or Sage: staves, and light once promoted */
	[CHARACTER_MOULDER] = { .ent = (const struct GaidenPinfoConfigEnt []) {
		{ 1,  ITEM_STAFF_HEAL },
		{ 3,  ITEM_STAFF_MEND },
		{ 7,  ITEM_STAFF_PHYSIC },
		{ 10, ITEM_STAFF_RESTORE },
		{ 15, ITEM_STAFF_RECOVER },
		{ 21, ITEM_LIGHT_LIGHTNING },
		{ 24, ITEM_LIGHT_SHINE },
		{ 27, ITEM_STAFF_FORTIFY },
		{ 32, ITEM_STAFF_WARP },
		{ 0 }
	}},
	/* Troubadour, then Valkyrie or Mage Knight: staves, then light. Tempest is her 30 MP spell */
	[CHARACTER_LARACHEL] = { .ent = (const struct GaidenPinfoConfigEnt []) {
		{ 1,  ITEM_ANIMA_TEMPEST },
		{ 1,  ITEM_STAFF_HEAL },
		{ 4,  ITEM_STAFF_MEND },
		{ 9,  ITEM_STAFF_RESTORE },
		{ 14, ITEM_STAFF_PHYSIC },
		{ 21, ITEM_LIGHT_LIGHTNING },
		{ 24, ITEM_LIGHT_SHINE },
		{ 28, ITEM_LIGHT_DIVINE },
		{ 30, ITEM_STAFF_FORTIFY },
		{ 34, ITEM_LIGHT_IVALDI },
		{ 0 }
	}},
	/* Sage from his first level (21): anima and staves */
	[CHARACTER_SALEH] = { .ent = (const struct GaidenPinfoConfigEnt []) {
		{ 21, ITEM_ANIMA_THUNDER },
		{ 21, ITEM_ANIMA_ELFIRE },
		{ 21, ITEM_STAFF_HEAL },
		{ 23, ITEM_ANIMA_FIMBULVETR },
		{ 24, ITEM_STAFF_PHYSIC },
		{ 26, ITEM_ANIMA_BOLTING },
		{ 27, ITEM_STAFF_RESTORE },
		{ 28, ITEM_ANIMA_AIRCALIBUR },
		{ 31, ITEM_ANIMA_EXCALIBUR },
		{ 0 }
	}},
	/* Pupil (a trainee: his levels go on counting as a Mage or Shaman). Freezeblink is his 30 MP spell */
	[CHARACTER_EWAN] = { .ent = (const struct GaidenPinfoConfigEnt []) {
		{ 1,  ITEM_ANIMA_FREEZEBLINK },
		{ 1,  ITEM_ANIMA_FIRE },
		{ 5,  ITEM_ANIMA_THUNDER },
		{ 8,  ITEM_DARK_FLUX },
		{ 12, ITEM_ANIMA_ELFIRE },
		{ 16, ITEM_DARK_LUNA },
		{ 0 }
	}},

	/* The other 30 MP spells, given to randomly picked units that can hold 30 MP */
	[CHARACTER_TETHYS] = { .ent = (const struct GaidenPinfoConfigEnt []) {
		{ 1, ITEM_ANIMA_PROMINENCE },
		{ 0 }
	}},
	[CHARACTER_GERIK] = { .ent = (const struct GaidenPinfoConfigEnt []) {
		{ 1, ITEM_DARK_GRAVITY },
		{ 0 }
	}},
	[CHARACTER_VANESSA] = { .ent = (const struct GaidenPinfoConfigEnt []) {
		{ 1, ITEM_STAFF_UNICORN },
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
