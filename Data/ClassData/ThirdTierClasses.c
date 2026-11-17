#include "common-chax.h"
#include "third-tier-classes.h"

#ifdef CONFIG_THIRD_TIER_CLASSES
/*
 * The third-tier classes, one per promoted class, with FE10's 3rd-tier caps (Documentation/Features/FE10StatCaps.md).
 * A row with a class id (the second field) is a real class (Documentation/Features/ThirdTierClasses.md): its promoted
 * class promotes to it (CanUnitPromoteToThirdTier). The others are planned: they need a class id, map sprite, battle
 * animations and texts first.
 */
const struct ThirdTierClassPlan gThirdTierClassPlans[] = {
	{ CLASS_EPHRAIM_MASTER_LORD, 0, "Sentinel", 60, 35, 20, 35, 33, 30, 34, 27 },
	{ CLASS_EIRIKA_MASTER_LORD, 0, "Trueblade", 50, 31, 20, 40, 40, 35, 25, 26 },
	{ CLASS_PALADIN, 0, "Silver Knight", 60, 35, 20, 34, 32, 30, 30, 25 },
	{ CLASS_PALADIN_F, 0, "Silver Knight", 55, 33, 20, 34, 34, 35, 28, 27 },
	{ CLASS_GENERAL, CLASS_MARSHALL, "Marshall", 60, 36, 20, 34, 31, 30, 37, 30 },
	{ CLASS_GENERAL_F, CLASS_MARSHALL, "Marshall", 60, 36, 20, 33, 32, 30, 35, 32 },
	{ CLASS_HERO, 0, "Vanguard", 65, 37, 15, 40, 37, 30, 32, 23 },
	{ CLASS_HERO_F, 0, "Vanguard", 65, 37, 15, 40, 37, 30, 32, 23 },
	{ CLASS_SWORDMASTER, CLASS_TRUEBLADE, "Trueblade", 55, 32, 20, 40, 40, 30, 26, 24 },
	{ CLASS_SWORDMASTER_F, 0, "Trueblade", 50, 31, 20, 40, 40, 35, 25, 26 },
	{ CLASS_ASSASSIN, 0, "Assassin", 55, 32, 15, 40, 40, 25, 30, 25 },
	{ CLASS_ASSASSIN_F, 0, "Assassin", 55, 32, 15, 40, 40, 25, 30, 25 },
	{ CLASS_ROGUE, CLASS_TRICKSTER, "Trickster", 50, 28, 20, 38, 40, 35, 26, 25 }, /* FE10's Whisper */
	{ CLASS_SNIPER, 0, "Marksman", 60, 36, 15, 40, 34, 30, 32, 25 },
	{ CLASS_SNIPER_F, 0, "Marksman", 60, 36, 15, 40, 34, 30, 32, 25 },
	{ CLASS_RANGER, 0, "Silver Knight", 50, 33, 20, 36, 35, 35, 27, 30 },
	{ CLASS_RANGER_F, 0, "Silver Knight", 50, 33, 20, 36, 35, 35, 27, 30 },
	{ CLASS_WYVERN_LORD, 0, "Dragonlord", 60, 38, 10, 38, 32, 30, 38, 20 },
	{ CLASS_WYVERN_LORD_F, 0, "Dragonlord", 55, 35, 10, 38, 35, 35, 36, 22 },
	{ CLASS_WYVERN_KNIGHT, 0, "Dragonlord", 55, 35, 10, 38, 35, 35, 36, 22 },
	{ CLASS_WYVERN_KNIGHT_F, 0, "Dragonlord", 55, 35, 10, 38, 35, 35, 36, 22 },
	{ CLASS_SAGE, CLASS_ARCHSAGE, "Archsage", 50, 24, 38, 34, 32, 30, 25, 36 },
	{ CLASS_SAGE_F, 0, "Arch Sage", 50, 24, 38, 34, 32, 30, 23, 38 },
	{ CLASS_MAGE_KNIGHT, 0, "Valkyrie", 50, 25, 35, 35, 34, 35, 24, 36 },
	{ CLASS_MAGE_KNIGHT_F, 0, "Valkyrie", 50, 25, 35, 35, 34, 35, 24, 36 },
	{ CLASS_BISHOP, 0, "Saint", 50, 25, 36, 30, 30, 35, 22, 40 },
	{ CLASS_BISHOP_F, 0, "Saint", 45, 23, 38, 30, 31, 40, 21, 40 },
	{ CLASS_DRUID, 0, "Arch Sage", 50, 25, 39, 34, 31, 30, 24, 36 },
	{ CLASS_DRUID_F, 0, "Arch Sage", 50, 25, 39, 34, 31, 30, 24, 36 },
	{ CLASS_SUMMONER, 0, "Summoner", 50, 26, 40, 31, 30, 25, 23, 39 },
	{ CLASS_SUMMONER_F, 0, "Summoner", 50, 26, 40, 31, 30, 25, 23, 39 },
	{ CLASS_GREAT_KNIGHT, 0, "Gold Knight", 60, 36, 20, 35, 31, 30, 30, 24 },
	{ CLASS_GREAT_KNIGHT_F, 0, "Gold Knight", 60, 34, 20, 35, 33, 30, 28, 26 },
	{ CLASS_RECRUIT_T2, 0, "Silver Knight", 60, 35, 20, 34, 32, 30, 30, 25 },
	{ CLASS_JOURNEYMAN_T2, 0, "Reaver", 68, 40, 10, 36, 35, 30, 30, 20 },
	{ CLASS_PUPIL_T2, 0, "Arch Sage", 50, 24, 38, 34, 32, 30, 25, 36 },
	{ CLASS_WARRIOR, 0, "Reaver", 68, 40, 10, 36, 35, 30, 30, 20 },
	{ CLASS_BERSERKER, 0, "Reaver", 68, 40, 10, 36, 35, 30, 30, 20 },
	{ CLASS_FALCON_KNIGHT, 0, "Seraph Knight", 50, 32, 20, 36, 36, 35, 28, 34 },
	{ CLASS_VALKYRIE, 0, "Valkyrie", 50, 25, 35, 35, 34, 35, 24, 36 },
	{ 0 },
};
#endif
