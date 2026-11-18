#include "common-chax.h"
#include "battle-system.h"

/* Swordmasters have no innate crit: the Crit skill is their class skill (SkillTable-job.c) */
const s8 gCriticalBonus[0x100] = {
	[CLASS_BERSERKER] = 10,
};

const s8 gSilencerBonus[0x100] = {
	[CLASS_SWORDMASTER] = 1,
	[CLASS_SWORDMASTER_F] = 1,
	[CLASS_ASSASSIN] = 20,
	[CLASS_ASSASSIN_F] = 20,
};
