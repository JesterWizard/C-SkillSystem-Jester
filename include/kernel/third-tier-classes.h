#pragma once

#include "common-chax.h"

/*
 * Third-tier classes (Data/ClassData/ThirdTierClasses.c, Documentation/Features/ThirdTierClasses.md), compiled only with
 * CONFIG_THIRD_TIER_CLASSES (include/configs/configs.h). Each row is the promoted class it follows, the third-tier
 * class once it exists (0 while it is only planned), a name and FE10's 3rd-tier caps.
 */
struct ThirdTierClassPlan {
	u8 from; /* the promoted class (CLASS_*) */
	u8 jid; /* the third-tier class (CLASS_*), or 0: planned, no class yet */
	const char *name;
	u8 hp, pow, mag, skl, spd, lck, def, res;
};

#ifdef CONFIG_THIRD_TIER_CLASSES
extern const struct ThirdTierClassPlan gThirdTierClassPlans[];
#endif

/*
 * True if the unit's promoted class promotes to a third-tier class (its gPromoJidLut choice is that class in
 * gThirdTierClassPlans): the Master Seal (CanUnitUsePromotionItem) and the automatic promotion at level 20
 * (AutoPromotionDue) promote it. Always false without CONFIG_THIRD_TIER_CLASSES. In Kernel/Wizardry/Lvupfx/Lvupfx/MapLvup.c.
 */
bool CanUnitPromoteToThirdTier(const struct Unit *unit);
