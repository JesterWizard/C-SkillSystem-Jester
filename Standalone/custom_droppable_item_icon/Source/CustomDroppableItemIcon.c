#include "global.h"
#include "bmudisp.h"
#include "bmunit.h"
#include "chapterdata.h"
#include "constants/event-flags.h"
#include "ctc.h"
#include "eventinfo.h"
#include "functions.h"
#include "types.h"
#include "variables.h"

/* OBJ tile of the potion icon in the patched LoadObjUIGfx sheet (row 3, column 4) */
#define DROP_ITEM_ICON_CHR (0xC80 / 0x20)

/**
 * Same timing as the vanilla rescue / boss / protection icons:
 * shown while (clock & 0x3F) < 0x28, hidden for the rest of the cycle.
 */
static bool IsBlinkingIconVisible(void)
{
	return (GetGameClock() & 0x3F) < 0x28;
}

/**
 * Vanilla PutUnitSpriteIconsOam draws one "misc" icon in the bottom-right
 * corner of a unit: rescuing > boss > protected character. The integrated
 * drop icon is pushed after that icon, so the misc icon covers it; skip the
 * drop icon for those units to keep that look.
 */
static bool HasVanillaCornerIcon(struct Unit *unit, int protect_pid)
{
	if (unit->state & US_RESCUING)
		return true;

	if (UNIT_FACTION(unit) != FACTION_BLUE && (UNIT_CATTRIBUTES(unit) & CA_BOSS))
		return true;

	return UNIT_CHAR_ID(unit) == protect_pid;
}

/**
 * Wraps the MapTask PROC_CALL(PutUnitSpritesOam). A potion icon is pushed on
 * the bottom-right corner of every unit carrying a droppable item
 * (US_DROP_ITEM), at the same spot and blink phase as the boss icon.
 *
 * The icons are pushed BEFORE calling vanilla PutUnitSpritesOam so that they
 * land in OAM ahead of the map sprites and are drawn on top of them, like the
 * icons vanilla PutUnitSpriteIconsOam pushes at the start of that function.
 */
void PutUnitSpritesOam_CustomDroppableItemIcon(void)
{
	int i;
	int protect_pid;

	if (CheckFlag(EVFLAG_HIDE_BLINKING_ICON) || !IsBlinkingIconVisible())
		goto vanilla;

	protect_pid = GetROMChapterStruct(gPlaySt.chapterIndex)->protectCharacterIndex;

	for (i = 1; i <= 0xC0; i++) {
		struct Unit *unit = GetUnit(i);
		int ix;
		int iy;

		if (!UNIT_IS_VALID(unit) || (unit->state & US_HIDDEN))
			continue;

		if (!(unit->state & US_DROP_ITEM))
			continue;

		if (HasVanillaCornerIcon(unit, protect_pid))
			continue;

		if (!unit->pMapSpriteHandle || (unit->pMapSpriteHandle->config & 0x80))
			continue;

		ix = unit->xPos * 16 - gBmSt.camera.x;
		iy = unit->yPos * 16 - gBmSt.camera.y;

		if (ix < -16 || ix > DISPLAY_WIDTH)
			continue;

		if (iy < -16 || iy > DISPLAY_HEIGHT)
			continue;

		CallARM_PushToSecondaryOAM(
			OAM1_X(0x200 + ix + 9),
			OAM0_Y(0x100 + iy + 7),
			gObject_8x8,
			OAM2_PAL(0) + OAM2_LAYER(2) + OAM2_CHR(DROP_ITEM_ICON_CHR));
	}

vanilla:
	PutUnitSpritesOam();
}
