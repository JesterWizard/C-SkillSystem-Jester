#include "global.h"
#include "bmio.h"
#include "bmitem.h"
#include "bmunit.h"
#include "variables.h"
#include "constants/classes.h"

extern const u8 AutoRepairWeaponsEnabled;

/**
 * Vanilla ChapterChangeUnitCleanup with optional auto weapon repair:
 * restores unbroken weapons to full durability at chapter transition.
 */
void ChapterChangeUnitCleanup_AutoRepairWeapons(void)
{
	int i, j;

	/* Clear phantoms */
	for (i = 1; i < 0x40; ++i) {
		struct Unit* unit = GetUnit(i);

		if (unit && unit->pCharacterData)
			if (UNIT_IS_PHANTOM(unit))
				ClearUnit(unit);
	}

	/* Clear all non player units (green & red units) */
	for (i = 0x41; i < 0xC0; ++i) {
		struct Unit* unit = GetUnit(i);

		if (unit && unit->pCharacterData)
			ClearUnit(unit);
	}

	/* Reset player unit temporary states (HP, status, some state flags, etc) */
	for (j = 1; j < 0x40; ++j) {
		struct Unit* unit = GetUnit(j);

		if (unit && unit->pCharacterData) {
			SetUnitHp(unit, GetUnitMaxHp(unit));
			SetUnitStatus(unit, UNIT_STATUS_NONE);

			unit->torchDuration = 0;
			unit->barrierDuration = 0;

			if (unit->state & US_NOT_DEPLOYED)
				unit->state = unit->state | US_BIT21;
			else
				unit->state = unit->state & ~US_BIT21;

			unit->state &= (
				US_DEAD | US_GROWTH_BOOST | US_SOLOANIM_1 | US_SOLOANIM_2 |
				US_BIT16 | US_BIT20 | US_BIT21 | US_BIT25 | US_BIT26
			);

			if (UNIT_CATTRIBUTES(unit) & CA_SUPPLY)
				unit->state = unit->state & ~US_DEAD;

			unit->state |= US_HIDDEN | US_NOT_DEPLOYED;

			unit->rescue = 0;
			unit->supportBits = 0;

			if (AutoRepairWeaponsEnabled) {
				for (i = 0; i < UNIT_ITEM_COUNT; i++)
					unit->items[i] = MakeNewItem(unit->items[i]);
			}
		}
	}

	gPlaySt.chapterStateBits = gPlaySt.chapterStateBits & ~PLAY_FLAG_PREPSCREEN;
}
