#include "common-chax.h"
#include "status-getter.h"
#include "weapon-range.h"

/*
 * AiFillDangerMap (FE8U 0x0803E320): adds to gBmMapOther, on every tile an
 * opposing unit could attack next turn, half its Power plus its weapon's
 * Might. Runs once per AI decision (AiInitDangerMap).
 *
 * This replaces 7743's "FillAIDangerMap Efficiency Fix 20220712" assembly
 * and gives the same map. It also reads each stat getter once per unit: the
 * kernel's AiCouldReachByBirdsEyeDistance and GenerateUnitMovementMap both
 * call MovGetter, and FillMovementAndRangeMapForItem asks for the weapon's
 * reach once per tile.
 */

/*
 * The unit array as the assembly walks it, by address: blue, red, green, then
 * the first purple unit (gUnitArrayBlue alone is declared with 62 units)
 */
#define DANGER_UNIT_ARRAY ((struct Unit *)0x0202BE4C)
#define DANGER_UNIT_AMT 0x85

/* hidden, dead, not deployed, bit 12 */
#define DANGER_UNIT_SKIP 0x100D

/*
 * The word the assembly tests against DANGER_UNIT_SKIP. It meant to read
 * unit->state but reads offset 0xC of the character data instead: base HP,
 * Pow, Skl and Spd. So dead, hidden and undeployed units are not skipped,
 * and units whose base HP has bit 0, 2 or 3 set, or whose base Pow has bit 4
 * set, are left out of the map.
 */
static u32 GetDangerSkipWord(struct Unit *unit)
{
	return *(const u32 *)((const u8 *)unit->pCharacterData + 0xC);
}

void AiFillDangerMap(void);

LYN_REPLACE_CHECK(AiFillDangerMap);
void AiFillDangerMap(void)
{
	struct Unit *active = gActiveUnit;
	struct Unit *unit;
	int active_mov = -1;

	for (unit = DANGER_UNIT_ARRAY; unit < DANGER_UNIT_ARRAY + DANGER_UNIT_AMT; unit++) {
		const u16 *slot = unit->items;
		int i, ix, iy, mov, might = 0;
		u16 item = 0;
		u32 reach;
		u8 value;

		if (!unit->pCharacterData)
			continue;

		if (GetDangerSkipWord(unit) & DANGER_UNIT_SKIP)
			continue;

		if (AreUnitsAllied(active->index, unit->index))
			continue;

		/*
		 * Strongest usable weapon, the later one on a tie. As in the
		 * assembly, a full inventory is followed by one more read: the
		 * u16 after the items (the sword and lance ranks).
		 */
		for (i = 0; i <= UNIT_ITEM_COUNT && slot[i] != 0; i++) {
			int item_might;

			if (CanUnitUseWeapon(unit, slot[i]) != true)
				continue;

			item_might = GetItemMight(slot[i]);
			if (item_might < might)
				continue;

			might = item_might;
			item = slot[i];
		}

		if (item == 0)
			continue;

		/* AiCouldReachByBirdsEyeDistance(active, unit, item) */
		if (active_mov < 0)
			active_mov = MovGetter(active);

		mov = MovGetter(unit);

		if (RECT_DISTANCE(active->xPos, active->yPos, unit->xPos, unit->yPos) >
		    active_mov + mov + GetItemMaxRangeRework(item, active))
			continue;

		/* FillMovementAndRangeMapForItem(unit, item), with mov read once */
		SetWorkingMoveCosts(GetUnitMovementCost(unit));
		SetWorkingBmMap(gBmMapMovement);
		GenerateMovementMap(unit->xPos, unit->yPos, mov, unit->index);
		BmMapFill(gBmMapRange, 0);

		reach = GetItemReachBitsRework(item, unit);

		for (iy = 0; iy < gBmMapSize.y; iy++) {
			for (ix = 0; ix < gBmMapSize.x; ix++) {
				if (gBmMapMovement[iy][ix] > MAP_MOVEMENT_MAX)
					continue;

				AddMap(ix, iy, reach);
			}
		}

		value = (GetUnitPower(unit) + might) >> 1;

		for (iy = gBmMapSize.y - 1; iy >= 0; iy--)
			for (ix = gBmMapSize.x - 1; ix >= 0; ix--)
				if (gBmMapRange[iy][ix] != 0)
					gBmMapOther[iy][ix] += value;
	}
}
