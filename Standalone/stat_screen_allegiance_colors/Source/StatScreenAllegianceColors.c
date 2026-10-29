#include "global.h"
#include "bmunit.h"
#include "statscreen.h"
#include "variables.h"

extern const u16 Pal_SSBlue[];
extern const u16 Pal_SSRed[];
extern const u16 Pal_SSGreen[];
extern const u16 Pal_SSGold[];

/**
 * Called from StatScreen_LoadBgPal_Allegiance (asm, hooked into
 * StatScreen_InitDisplay) instead of loading vanilla Pal_StatscreenBG.
 * Blue = player, red = enemy, green = NPC, gold = anything else (purple/link arena).
 */
const u16 *GetStatScreenBgPal_Allegiance(void)
{
	struct Unit *unit = gStatScreen.unit;

	if (unit == NULL)
		return Pal_StatscreenBG;

	switch (UNIT_FACTION(unit)) {
	case FACTION_BLUE:
		return Pal_SSBlue;
	case FACTION_RED:
		return Pal_SSRed;
	case FACTION_GREEN:
		return Pal_SSGreen;
	default:
		return Pal_SSGold;
	}
}
