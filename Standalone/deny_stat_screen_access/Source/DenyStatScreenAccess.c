#include "global.h"
#include "bmunit.h"
#include "statscreen.h"
#include "constants/classes.h"

/**
 * Class IDs whose units cannot be inspected on the stat screen.
 * Zero-terminated u8 list defined in Installer.event (StatScreenDenyClasses).
 */
extern const u8 StatScreenDenyClasses[];

static bool IsStatScreenDeniedClass(struct Unit *unit)
{
	const u8 *it;

	for (it = StatScreenDenyClasses; *it != 0; it++)
		if (UNIT_CLASS_ID(unit) == *it)
			return true;

	return false;
}

/* Replaces vanilla CanShowUnitStatScreen (FE8U 0x0801C928) */
bool CanShowUnitStatScreen_Deny(struct Unit *unit)
{
	if (IsStatScreenDeniedClass(unit))
		return false;

	if (UNIT_IS_GORGON_EGG(unit))
		return false;

	return true;
}

/* Replaces vanilla FindNextUnit (FE8U 0x08087920): Up/Down unit switching on the stat screen */
struct Unit *FindNextUnit_Deny(struct Unit *u, int direction)
{
	int faction = UNIT_FACTION(u);
	int i = u->index;
	struct Unit *unit;

	while (TRUE) {
		i = (i + direction) & 0x3F;
		unit = GetUnit(faction + i);

		/* Wrapped back to the unit we started from: nothing else to show */
		if (unit == u)
			return u;

		if (!UNIT_IS_VALID(unit))
			continue;

		if ((sStatScreenInfo.config & STATSCREEN_CONFIG_NONDEAD) && (unit->state & US_DEAD))
			continue;

		if ((sStatScreenInfo.config & STATSCREEN_CONFIG_NONBENCHED) && (unit->state & US_NOT_DEPLOYED))
			continue;

		if ((sStatScreenInfo.config & STATSCREEN_CONFIG_NONUNK9) && (unit->state & US_BIT9))
			continue;

		if ((sStatScreenInfo.config & STATSCREEN_CONFIG_NONROOFED) && (unit->state & US_UNDER_A_ROOF))
			continue;

		if ((sStatScreenInfo.config & STATSCREEN_CONFIG_NONUNK16) && (unit->state & US_BIT16))
			continue;

		if ((sStatScreenInfo.config & STATSCREEN_CONFIG_NONSUPPLY) && (UNIT_CATTRIBUTES(unit) & CA_SUPPLY))
			continue;

		if (IsStatScreenDeniedClass(unit))
			continue;

		if (UNIT_IS_GORGON_EGG(unit))
			continue;

		return unit;
	}
}
