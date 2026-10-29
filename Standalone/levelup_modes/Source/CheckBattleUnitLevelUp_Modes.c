#include "global.h"
#include "bmbattle.h"
#include "bmunit.h"
#include "rng.h"
#include "eventinfo.h"
#include "gba/syscall.h"

/*
 * Setting bytes from Installer.event:
 *   [0] mode on Easy, [1] mode on Normal, [2] mode on Hard,
 *   [3] guaranteed level-up retries (0 = off).
 */
extern const u8 LvupModesConfig[4];

enum {
	LVUP_CFG_EASY = 0,
	LVUP_CFG_NORMAL = 1,
	LVUP_CFG_HARD = 2,
	LVUP_CFG_GUARANTEED = 3,
};

enum {
	LVUP_MODE_VANILLA = 0,
	LVUP_MODE_RANDC = 1,
	LVUP_MODE_FIXED = 2,
	LVUP_MODE_100 = 3,
	LVUP_MODE_0 = 4,
};

#define LVUP_GROWTH_BOOST_BONUS 5
#define LVUP_GUARANTEED_RETRIES 10
#define LVUP_GUARANTEED_BONUS   10

/* Mode 1 RNG state (Source/RamAlloc.s): seeds[0..2] and a check value. */
extern u16 sLvupModesRandSeeds[4];

#define LVUP_RANDC_CHECK(s) ((u16)((s)[0] ^ (s)[1] ^ (s)[2] ^ 0xA5C3))

static void RandC_EnsureSeeded(void)
{
	u16 *seeds = sLvupModesRandSeeds;
	int i;

	if (seeds[3] == LVUP_RANDC_CHECK(seeds))
		return;

	for (i = 0; i < 3; i++)
		seeds[i] = NextRN();

	if ((seeds[0] | seeds[1] | seeds[2]) == 0)
		seeds[0] = 1;

	seeds[3] = LVUP_RANDC_CHECK(seeds);
}

/* Same generator as the vanilla RNG, on its own seeds. */
static int RandC_Next100(void)
{
	u16 *seeds = sLvupModesRandSeeds;
	u16 rn = (seeds[1] << 11) + (seeds[0] >> 5);

	seeds[2] *= 2;

	if (seeds[1] & 0x8000)
		seeds[2]++;

	rn ^= seeds[2];

	seeds[2] = seeds[1];
	seeds[1] = seeds[0];
	seeds[0] = rn;

	seeds[3] = LVUP_RANDC_CHECK(seeds);

	return (rn * 100) >> 16;
}

static int GetStatIncrease_RandC(int growth)
{
	int result = 0;

	while (growth > 100) {
		result++;
		growth -= 100;
	}

	if (growth > RandC_Next100())
		result++;

	return result;
}

static int GetStatIncrease_Fixed(int growth, int ref)
{
	/* DivRem: vanilla __aeabi_idivmod does not return the remainder where GCC expects it. */
	return (growth + DivRem(growth * ref, 100)) / 100;
}

static int GetTotalStatGains(const struct BattleUnit *bu)
{
	return bu->changeHP + bu->changePow + bu->changeSkl + bu->changeSpd +
		bu->changeDef + bu->changeRes + bu->changeLck;
}

/* Vanilla rolls, in vanilla order. GetStatIncrease stays hookable. */
static void LvupRoll_Vanilla(struct BattleUnit *bu, int bonus)
{
	const struct CharacterData *ch = bu->unit.pCharacterData;

	bu->changeHP = GetStatIncrease(ch->growthHP + bonus);
	bu->changePow = GetStatIncrease(ch->growthPow + bonus);
	bu->changeSkl = GetStatIncrease(ch->growthSkl + bonus);
	bu->changeSpd = GetStatIncrease(ch->growthSpd + bonus);
	bu->changeDef = GetStatIncrease(ch->growthDef + bonus);
	bu->changeRes = GetStatIncrease(ch->growthRes + bonus);
	bu->changeLck = GetStatIncrease(ch->growthLck + bonus);
}

/* Vanilla fallback when nothing was gained: two passes, one stat at a time. */
static void LvupRetry_Vanilla(struct BattleUnit *bu)
{
	const struct CharacterData *ch = bu->unit.pCharacterData;
	int i;

	for (i = 0; i < 2; i++) {
		if ((bu->changeHP = GetStatIncrease(ch->growthHP)) != 0)
			break;
		if ((bu->changePow = GetStatIncrease(ch->growthPow)) != 0)
			break;
		if ((bu->changeSkl = GetStatIncrease(ch->growthSkl)) != 0)
			break;
		if ((bu->changeSpd = GetStatIncrease(ch->growthSpd)) != 0)
			break;
		if ((bu->changeDef = GetStatIncrease(ch->growthDef)) != 0)
			break;
		if ((bu->changeRes = GetStatIncrease(ch->growthRes)) != 0)
			break;
		if ((bu->changeLck = GetStatIncrease(ch->growthLck)) != 0)
			break;
	}
}

/* Mode 1: growth rolls on a separate RNG stream (integrated order). */
static void LvupRoll_RandC(struct BattleUnit *bu, int bonus)
{
	const struct CharacterData *ch = bu->unit.pCharacterData;

	RandC_EnsureSeeded();

	bu->changeHP = GetStatIncrease_RandC(ch->growthHP + bonus);
	bu->changePow = GetStatIncrease_RandC(ch->growthPow + bonus);
	bu->changeSkl = GetStatIncrease_RandC(ch->growthSkl + bonus);
	bu->changeSpd = GetStatIncrease_RandC(ch->growthSpd + bonus);
	bu->changeLck = GetStatIncrease_RandC(ch->growthLck + bonus);
	bu->changeDef = GetStatIncrease_RandC(ch->growthDef + bonus);
	bu->changeRes = GetStatIncrease_RandC(ch->growthRes + bonus);
}

/* Mode 2: fixed growths, driven by the level the unit had before this level-up. */
static void LvupRoll_Fixed(struct BattleUnit *bu, int bonus)
{
	struct Unit *unit = GetUnit(bu->unit.index);
	const struct CharacterData *ch = unit->pCharacterData;
	int ref = unit->level - 1;

	if (UNIT_CATTRIBUTES(unit) & CA_PROMOTED)
		ref += 19;

	bu->changeHP = GetStatIncrease_Fixed(ch->growthHP + bonus, ref += 5);
	bu->changePow = GetStatIncrease_Fixed(ch->growthPow + bonus, ref += 5);
	bu->changeSkl = GetStatIncrease_Fixed(ch->growthSkl + bonus, ref += 5);
	bu->changeSpd = GetStatIncrease_Fixed(ch->growthSpd + bonus, ref += 5);
	bu->changeLck = GetStatIncrease_Fixed(ch->growthLck + bonus, ref += 5);
	bu->changeDef = GetStatIncrease_Fixed(ch->growthDef + bonus, ref += 5);
	bu->changeRes = GetStatIncrease_Fixed(ch->growthRes + bonus, ref += 5);
}

/* Mode 3: +1 to every stat (caps still apply). */
static void LvupRoll_100(struct BattleUnit *bu, int bonus)
{
	(void)bonus;

	bu->changeHP = 1;
	bu->changePow = 1;
	bu->changeSkl = 1;
	bu->changeSpd = 1;
	bu->changeLck = 1;
	bu->changeDef = 1;
	bu->changeRes = 1;
}

/* Mode 4: no stat gains. */
static void LvupRoll_0(struct BattleUnit *bu, int bonus)
{
	(void)bonus;

	bu->changeHP = 0;
	bu->changePow = 0;
	bu->changeSkl = 0;
	bu->changeSpd = 0;
	bu->changeLck = 0;
	bu->changeDef = 0;
	bu->changeRes = 0;
}

static void LvupRoll(struct BattleUnit *bu, int mode, int bonus)
{
	switch (mode) {
	case LVUP_MODE_RANDC:
		LvupRoll_RandC(bu, bonus);
		break;

	case LVUP_MODE_FIXED:
		LvupRoll_Fixed(bu, bonus);
		break;

	case LVUP_MODE_100:
		LvupRoll_100(bu, bonus);
		break;

	case LVUP_MODE_0:
		LvupRoll_0(bu, bonus);
		break;

	case LVUP_MODE_VANILLA:
	default:
		LvupRoll_Vanilla(bu, bonus);
		break;
	}
}

static int GetLvupMode(void)
{
	int mode;

	if (gPlaySt.chapterStateBits & PLAY_FLAG_HARD)
		mode = LvupModesConfig[LVUP_CFG_HARD];
	else if (TUTORIAL_MODE())
		mode = LvupModesConfig[LVUP_CFG_EASY];
	else
		mode = LvupModesConfig[LVUP_CFG_NORMAL];

	if (mode > LVUP_MODE_0)
		mode = LVUP_MODE_VANILLA;

	return mode;
}

/**
 * Vanilla CheckBattleUnitLevelUp with the stat roll picked per difficulty.
 * Level / EXP handling and the final cap check are unchanged.
 */
void CheckBattleUnitLevelUp_Modes(struct BattleUnit *bu)
{
	int mode, bonus, i;

	if (!CanBattleUnitGainLevels(bu) || bu->unit.exp < 100)
		return;

	bu->unit.exp -= 100;
	bu->unit.level++;

	if (UNIT_CATTRIBUTES(&bu->unit) & CA_MAXLEVEL10) {
		if (bu->unit.level == 10) {
			bu->expGain -= bu->unit.exp;
			bu->unit.exp = UNIT_EXP_DISABLED;
		}
	} else if (bu->unit.level == UNIT_LEVEL_MAX) {
		bu->expGain -= bu->unit.exp;
		bu->unit.exp = UNIT_EXP_DISABLED;
	}

	bonus = (bu->unit.state & US_GROWTH_BOOST) ? LVUP_GROWTH_BOOST_BONUS : 0;
	mode = GetLvupMode();

	LvupRoll(bu, mode, bonus);

	if (LvupModesConfig[LVUP_CFG_GUARANTEED]) {
		for (i = 0; i < LVUP_GUARANTEED_RETRIES && GetTotalStatGains(bu) == 0; i++)
			LvupRoll(bu, mode, bonus + LVUP_GUARANTEED_BONUS);
	} else if (mode == LVUP_MODE_VANILLA && GetTotalStatGains(bu) == 0) {
		LvupRetry_Vanilla(bu);
	}

	CheckBattleUnitStatCaps(GetUnit(bu->unit.index), bu);
}
