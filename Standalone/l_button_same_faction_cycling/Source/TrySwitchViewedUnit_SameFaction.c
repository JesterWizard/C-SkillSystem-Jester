#include "global.h"
#include "bmunit.h"
#include "bmmap.h"
#include "playerphase.h"

/**
 * Vanilla TrySwitchViewedUnit (L button on the map), except that the cycle
 * stays in the faction of the unit under the cursor:
 *   - cursor on an enemy (0x80..0xBF) -> next enemy
 *   - cursor on an NPC   (0x40..0x7F) -> next NPC
 *   - anything else      (0x01..0x3F) -> next player unit (vanilla)
 */
void TrySwitchViewedUnit_SameFaction(int x, int y)
{
	int i;
	int unitId = gBmMapUnit[y][x];
	int factionMin, factionMax;

	if (unitId & 0x80) {
		factionMin = 0x80;
		factionMax = 0xBF;
	} else if (unitId & 0x40) {
		factionMin = 0x40;
		factionMax = 0x7F;
	} else {
		factionMin = 0x01;
		factionMax = 0x3F;
	}

	if (unitId < factionMin || unitId > factionMax)
		unitId = factionMin - 1;

	unitId++;

	if (unitId > factionMax)
		unitId = factionMin;

	for (i = unitId; i <= factionMax; ++i) {
		if (TrySetCursorOn(i))
			return;
	}

	for (i = factionMin; i < unitId; ++i) {
		if (TrySetCursorOn(i))
			return;
	}
}
