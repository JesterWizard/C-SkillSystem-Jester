#include "common-chax.h"
#include "skill-system.h"
#include "constants/skills.h"

const u16 gConstSkillTable_Job[0x100][2] = {
	[CLASS_EPHRAIM_LORD] = {
	},

	[CLASS_EIRIKA_LORD] = {
	},

	[CLASS_EPHRAIM_MASTER_LORD] = {
		[0] = SID_Canto,
	},

	[CLASS_PALADIN] = {
		[0] = SID_Canto,
	},

	[CLASS_PALADIN_F] = {
		[0] = SID_Canto,
	},

	[CLASS_GREAT_KNIGHT] = {
		[0] = SID_Canto,
	},

	[CLASS_GREAT_KNIGHT_F] = {
		[0] = SID_Canto,
	},

	[CLASS_RANGER] = {
		[0] = SID_Canto,
	},

	[CLASS_RANGER_F] = {
		[0] = SID_Canto,
	},

	[CLASS_MAGE_KNIGHT] = {
		[0] = SID_Canto,
	},

	[CLASS_MAGE_KNIGHT_F] = {
		[0] = SID_Canto,
	},

	[CLASS_VALKYRIE] = {
		[0] = SID_Canto,
	},

	[CLASS_TROUBADOUR] = {
		[0] = SID_Canto,
	},

	[CLASS_THIEF] = {
	},

	[CLASS_GENERAL] = {
	},

	[CLASS_GENERAL_F] = {
	},

	[CLASS_MYRMIDON_F] = {
	},

	[CLASS_SWORDMASTER] = {
	},

	[CLASS_SWORDMASTER_F] = {
	},

	[CLASS_ASSASSIN] = {
	},

	[CLASS_ASSASSIN_F] = {
	},

	[CLASS_SNIPER] = {
	},

	[CLASS_SNIPER_F] = {
	},

	[CLASS_WYVERN_RIDER] = {
		[0] = SID_Canto,
	},

	[CLASS_WYVERN_RIDER_F] = {
		[0] = SID_Canto,
	},

	[CLASS_WYVERN_LORD] = {
		[0] = SID_Canto,
	},

	[CLASS_WYVERN_LORD_F] = {
		[0] = SID_Canto,
	},

	[CLASS_WYVERN_KNIGHT] = {
		[0] = SID_Canto,
	},

	[CLASS_WYVERN_KNIGHT_F] = {
		[0] = SID_Canto,
	},

	[CLASS_SUMMONER] = {
	},

	[CLASS_SUMMONER_F] = {
	},

	[CLASS_ROGUE] = {
	},

	[CLASS_FIGHTER] = {
	},

	[CLASS_BERSERKER] = {
	},

	[CLASS_BARD] = {
	},

	[CLASS_PEGASUS_KNIGHT] = {
		[0] = SID_Canto,
	},

	[CLASS_FALCON_KNIGHT] = {
		[0] = SID_Canto,
	},

	[CLASS_DANCER] = {
	},

	[CLASS_SOLDIER] = {
	},

	/* Third-tier classes (Documentation/Features/ThirdTierClasses.md) */
#if defined(SID_Lethality) && (COMMON_SKILL_VALID(SID_Lethality))
	[CLASS_TRICKSTER][0] = SID_Lethality,
#endif
#if defined(SID_LuckySeven) && (COMMON_SKILL_VALID(SID_LuckySeven))
	[CLASS_TRICKSTER][1] = SID_LuckySeven,
#endif
#if defined(SID_Pavise) && (COMMON_SKILL_VALID(SID_Pavise))
	[CLASS_MARSHALL][0] = SID_Pavise,
#endif
#if defined(SID_FaireLance) && (COMMON_SKILL_VALID(SID_FaireLance))
	[CLASS_MARSHALL][1] = SID_FaireLance,
#endif
#if defined(SID_Astra) && (COMMON_SKILL_VALID(SID_Astra))
	[CLASS_TRUEBLADE][0] = SID_Astra,
#endif
#if defined(SID_FaireSword) && (COMMON_SKILL_VALID(SID_FaireSword))
	[CLASS_TRUEBLADE][1] = SID_FaireSword,
#endif
#if defined(SID_Flare) && (COMMON_SKILL_VALID(SID_Flare))
	[CLASS_ARCHSAGE][0] = SID_Flare,
#endif
#if defined(SID_FaireAnima) && (COMMON_SKILL_VALID(SID_FaireAnima))
	[CLASS_ARCHSAGE][1] = SID_FaireAnima,
#endif
};
