#include "common-chax.h"
#include "third-tier-classes.h"

#ifdef CONFIG_THIRD_TIER_CLASSES
/*
 * The planned third-tier classes, one per promoted class, with FE10's 3rd-tier caps
 * (Documentation/Features/FE10StatCaps.md). Off until each has its class id, map sprite, battle animations and texts.
 */
const struct ThirdTierClassPlan gThirdTierClassPlans[] = {
	{ CLASS_EPHRAIM_MASTER_LORD, "Sentinel", 60, 35, 20, 35, 33, 30, 34, 27 },
	{ CLASS_EIRIKA_MASTER_LORD, "Trueblade", 50, 31, 20, 40, 40, 35, 25, 26 },
	{ CLASS_PALADIN, "Silver Knight", 60, 35, 20, 34, 32, 30, 30, 25 },
	{ CLASS_PALADIN_F, "Silver Knight", 55, 33, 20, 34, 34, 35, 28, 27 },
	{ CLASS_GENERAL, "Marshall", 60, 36, 20, 34, 31, 30, 37, 30 },
	{ CLASS_GENERAL_F, "Marshall", 60, 36, 20, 33, 32, 30, 35, 32 },
	{ CLASS_HERO, "Vanguard", 65, 37, 15, 40, 37, 30, 32, 23 },
	{ CLASS_HERO_F, "Vanguard", 65, 37, 15, 40, 37, 30, 32, 23 },
	{ CLASS_SWORDMASTER, "Trueblade", 55, 32, 20, 40, 40, 30, 26, 24 },
	{ CLASS_SWORDMASTER_F, "Trueblade", 50, 31, 20, 40, 40, 35, 25, 26 },
	{ CLASS_ASSASSIN, "Assassin", 55, 32, 15, 40, 40, 25, 30, 25 },
	{ CLASS_ASSASSIN_F, "Assassin", 55, 32, 15, 40, 40, 25, 30, 25 },
	{ CLASS_ROGUE, "Whisper", 50, 28, 20, 38, 40, 35, 26, 25 },
	{ CLASS_SNIPER, "Marksman", 60, 36, 15, 40, 34, 30, 32, 25 },
	{ CLASS_SNIPER_F, "Marksman", 60, 36, 15, 40, 34, 30, 32, 25 },
	{ CLASS_RANGER, "Silver Knight", 50, 33, 20, 36, 35, 35, 27, 30 },
	{ CLASS_RANGER_F, "Silver Knight", 50, 33, 20, 36, 35, 35, 27, 30 },
	{ CLASS_WYVERN_LORD, "Dragonlord", 60, 38, 10, 38, 32, 30, 38, 20 },
	{ CLASS_WYVERN_LORD_F, "Dragonlord", 55, 35, 10, 38, 35, 35, 36, 22 },
	{ CLASS_WYVERN_KNIGHT, "Dragonlord", 55, 35, 10, 38, 35, 35, 36, 22 },
	{ CLASS_WYVERN_KNIGHT_F, "Dragonlord", 55, 35, 10, 38, 35, 35, 36, 22 },
	{ CLASS_SAGE, "Arch Sage", 50, 24, 38, 34, 32, 30, 25, 36 },
	{ CLASS_SAGE_F, "Arch Sage", 50, 24, 38, 34, 32, 30, 23, 38 },
	{ CLASS_MAGE_KNIGHT, "Valkyrie", 50, 25, 35, 35, 34, 35, 24, 36 },
	{ CLASS_MAGE_KNIGHT_F, "Valkyrie", 50, 25, 35, 35, 34, 35, 24, 36 },
	{ CLASS_BISHOP, "Saint", 50, 25, 36, 30, 30, 35, 22, 40 },
	{ CLASS_BISHOP_F, "Saint", 45, 23, 38, 30, 31, 40, 21, 40 },
	{ CLASS_DRUID, "Arch Sage", 50, 25, 39, 34, 31, 30, 24, 36 },
	{ CLASS_DRUID_F, "Arch Sage", 50, 25, 39, 34, 31, 30, 24, 36 },
	{ CLASS_SUMMONER, "Summoner", 50, 26, 40, 31, 30, 25, 23, 39 },
	{ CLASS_SUMMONER_F, "Summoner", 50, 26, 40, 31, 30, 25, 23, 39 },
	{ CLASS_GREAT_KNIGHT, "Gold Knight", 60, 36, 20, 35, 31, 30, 30, 24 },
	{ CLASS_GREAT_KNIGHT_F, "Gold Knight", 60, 34, 20, 35, 33, 30, 28, 26 },
	{ CLASS_RECRUIT_T2, "Silver Knight", 60, 35, 20, 34, 32, 30, 30, 25 },
	{ CLASS_JOURNEYMAN_T2, "Reaver", 68, 40, 10, 36, 35, 30, 30, 20 },
	{ CLASS_PUPIL_T2, "Arch Sage", 50, 24, 38, 34, 32, 30, 25, 36 },
	{ CLASS_WARRIOR, "Reaver", 68, 40, 10, 36, 35, 30, 30, 20 },
	{ CLASS_BERSERKER, "Reaver", 68, 40, 10, 36, 35, 30, 30, 20 },
	{ CLASS_FALCON_KNIGHT, "Seraph Knight", 50, 32, 20, 36, 36, 35, 28, 34 },
	{ CLASS_VALKYRIE, "Valkyrie", 50, 25, 35, 35, 34, 35, 24, 36 },
	{ 0 },
};
#endif
