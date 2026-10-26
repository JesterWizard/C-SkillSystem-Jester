#include "common-chax.h"
#include "skill-system.h"
#include "constants/skills.h"

/**
 * Skill granted to the wielder while the weapon is equipped, once it has been
 * forged to the maxCount set for it in gForgeLimits (requires forge_mechanic)
 */
const u16 gConstSkillTable_Forge[0x100] = {
	[ITEM_SWORD_IRON] = SID_SwordBreaker,
};
