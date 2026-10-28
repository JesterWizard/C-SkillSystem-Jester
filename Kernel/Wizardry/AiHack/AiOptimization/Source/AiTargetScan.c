#include "common-chax.h"
#include "skill-system.h"
#include "kernel-lib.h"
#include "debuff.h"
#include "constants/skills.h"
#include "jester_headers/custom-functions.h"
#include "enemy-fog-vision.h"
#include "AiTargetScan.h"

/*
 * Target search for AiAttemptOffensiveAction (AiOptimization.c).
 */

/*
 * Adds a target to list, kept sorted by (distance, unit id) so that the
 * nearest come first. A full list drops its farthest entry.
 */
static int AiInsertTarget(u16 *list, int count, int dist, int uid)
{
	u16 key = (dist << 8) | uid;
	int i;

	if (count == AI_TARGET_LIST_MAX) {
		if (key >= list[count - 1])
			return count;

		count--;
	}

	for (i = count; i > 0 && list[i - 1] > key; i--)
		list[i] = list[i - 1];

	list[i] = key;
	return count + 1;
}

/*
 * Fills list with the units the active unit may attack with item, as
 * AI_TARGET_UID entries, and returns how many there are. With
 * calculate_map_range_faster they are the ones within move_distance, nearest
 * first; otherwise every target AiReachesByBirdsEyeDistance accepts, in unit
 * order.
 */
int AiCollectOffensiveTargets(s8 (*isEnemy)(struct Unit *unit), u16 item, int move_distance, u16 *list)
{
	int uid, dist, count = 0;
	u8 unit_amount = 0xC0;

#ifdef CONFIG_FOURTH_ALLEGIANCE
	if (gpKernelDesignerConfig->fourth_allegiance)
		unit_amount = 0xD0;
#endif

	for (uid = 1; uid < unit_amount; uid++) {
		struct Unit *unit = GetUnit(uid);

		if (!UNIT_IS_VALID(unit))
			continue;

		if (unit->state & (US_HIDDEN | US_DEAD | US_RESCUED | US_BIT16))
			continue;

		if (gpKernelDesignerConfig->calculate_map_range_faster == true) {
			dist = RECT_DISTANCE(unit->xPos, unit->yPos, gActiveUnit->xPos, gActiveUnit->yPos);
			if (move_distance < dist)
				continue;
		} else {
			if (!AiReachesByBirdsEyeDistance(gActiveUnit, unit, item))
				continue;

			dist = 0;
		}

#if defined(SID_ShadePlus) && (COMMON_SKILL_VALID(SID_ShadePlus))
		if (SkillTesterPlus(unit, SID_ShadePlus))
			continue;
#endif

#if defined(SID_Shade) && (COMMON_SKILL_VALID(SID_Shade))
		/* Shade skill may make unit avoid to be target */
		if (SkillTester(unit, SID_Shade))
			if (Roll1RN(unit->lck))
				continue;
#endif

		if (GetUnitStatusIndex(unit) == NEW_UNIT_STATUS_HIDE)
			continue;

#if defined(SID_Rampage) && (COMMON_SKILL_VALID(SID_Rampage))
		if (SkillTester(gActiveUnit, SID_Rampage)) {
			if (AreUnitsAllied(gActiveUnit->index, unit->index))
				continue;
			if (gActiveUnit->index == unit->index)
				continue;
			if (!EnemyFogVisionCanSeeUnit(unit))
				continue;
		} else {
			if (!isEnemy(unit))
				continue;
		}
#else
		if (!isEnemy(unit))
			continue;
#endif

		count = AiInsertTarget(list, count, dist, uid);
	}

	return count;
}

/*
 * The tile search of AiSimulateBestBattleAgainstTarget (FE8U 0x0803DCC4),
 * but only over the tiles within range of the target at (tx, ty):
 * AiFillReversedAttackRangeMap marks gMapRangeSigned nowhere else. Same scan
 * order and tie-break, so the chosen tile is the same. Returns whether a tile
 * was found; the caller then runs AiSimulateBattleAgainstTargetAtPosition.
 */
bool AiFindBestTileNearTarget(struct AiCombatSimulationSt *st, int tx, int ty, int range)
{
	int ix, iy;
	int x0 = tx - range, x1 = tx + range;
	int y0 = ty - range, y1 = ty + range;
	u32 score = 0, tmp;

	if (x0 < 0)
		x0 = 0;
	if (y0 < 0)
		y0 = 0;
	if (x1 > gBmMapSize.x - 1)
		x1 = gBmMapSize.x - 1;
	if (y1 > gBmMapSize.y - 1)
		y1 = gBmMapSize.y - 1;

	for (iy = y1; iy >= y0; iy--) {
		for (ix = x1; ix >= x0; ix--) {
			if (gBmMapMovement[iy][ix] > MAP_MOVEMENT_MAX)
				continue;

			if (gMapRangeSigned[iy][ix] == 0)
				continue;

			if (gBmMapUnit[iy][ix] != 0 && gBmMapUnit[iy][ix] != gActiveUnitId)
				continue;

			tmp = AiGetCombatPositionScore(ix, iy, st);
			if (tmp <= score)
				continue;

			st->xMove = ix;
			st->yMove = iy;
			score = tmp;
		}
	}

	return score != 0;
}
