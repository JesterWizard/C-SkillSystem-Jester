#pragma once

#include "common-chax.h"

/*
 * Planned third-tier classes (Data/ClassData/ThirdTierClasses.c), compiled only with CONFIG_THIRD_TIER_CLASSES
 * (include/configs/configs.h, off). Each row is the promoted class it follows and FE10's 3rd-tier caps.
 */
struct ThirdTierClassPlan {
	u8 from; /* the promoted class (CLASS_*) */
	const char *name;
	u8 hp, pow, mag, skl, spd, lck, def, res;
};

#ifdef CONFIG_THIRD_TIER_CLASSES
extern const struct ThirdTierClassPlan gThirdTierClassPlans[];
#endif
