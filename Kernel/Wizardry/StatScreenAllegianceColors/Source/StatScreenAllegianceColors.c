#include "common-chax.h"
#include "kernel-lib.h"
#include "statscreen.h"
#include "bmunit.h"

extern const u16 Pal_SSBlue[];
extern const u16 Pal_SSRed[];
extern const u16 Pal_SSGreen[];
extern const u16 Pal_SSGold[];

const u16 *GetStatScreenBgPal(void)
{
	struct Unit *unit;

	if (gpKernelDesignerConfig->stat_screen_allegiance_colors == false)
		return Pal_StatscreenBG;

	unit = gStatScreen.unit;
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
