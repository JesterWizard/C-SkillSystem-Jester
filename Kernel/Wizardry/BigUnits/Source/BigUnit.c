#include "common-chax.h"
#include "big-unit.h"
#include "map-movement.h"

/* struct Unit's index is signed: enemies' (0x81 and up) read negative. Compare it as a u8. */
#define UID(unit) ((u8)(unit)->index)

/*
 * 2x2 units: the groups, the corners' spawning, keeping them on their block, and the save chunks.
 * See include/kernel/big-unit.h and Documentation/Features/BigUnits.md.
 */

const s8 gBigUnitCornerOffsets[BIG_CORNERS][2] = {
	[BIG_CORNER_TL] = { 0, 0 },
	[BIG_CORNER_TR] = { 1, 0 },
	[BIG_CORNER_BL] = { 0, 1 },
	[BIG_CORNER_BR] = { 1, 1 },
};

static bool BigUnit_InList(const u8 *list, int id)
{
	for (; *list; list++)
		if (*list == id)
			return true;
	return false;
}

static bool BigUnit_Listed(struct Unit *unit)
{
	return BigUnit_InList(gBigUnitCharacters, unit->pCharacterData->number) ||
	       BigUnit_InList(gBigUnitClasses, unit->pClassData->number);
}

/* A corner that is still on the map: not broken, dead or carried off */
struct Unit *BigUnit_GetCorner(struct BigUnitGroup *group, int corner)
{
	struct Unit *unit;

	if (!group->uid[corner])
		return NULL;

	unit = GetUnit(group->uid[corner]);
	if (!UNIT_IS_VALID(unit) || (unit->state & (US_DEAD | US_BIT16 | US_NOT_DEPLOYED)))
		return NULL;

	return unit;
}

struct BigUnitGroup *BigUnit_GetGroup(struct Unit *unit, int *corner)
{
	int i, c;

	if (!unit || !UID(unit))
		return NULL;

	for (i = 0; i < BIG_UNIT_GROUPS; i++) {
		for (c = 0; c < BIG_CORNERS; c++) {
			if (gBigUnitGroups[i].uid[c] != UID(unit))
				continue;
			if (corner)
				*corner = c;
			return &gBigUnitGroups[i];
		}
	}
	return NULL;
}

bool BigUnit_IsBig(struct Unit *unit)
{
	return BigUnit_GetGroup(unit, NULL) != NULL;
}

static bool BigUnit_GroupEmpty(const struct BigUnitGroup *group)
{
	return !(group->uid[0] | group->uid[1] | group->uid[2] | group->uid[3]);
}

void BigUnit_OnLoadUnit(struct Unit *unit)
{
	struct BigUnitGroup *group = NULL;
	struct Unit *copy;
	int i;

	/* Not allies: their corners would share one character in the roster, events and saves */
	if (!UNIT_IS_VALID(unit) || UNIT_FACTION(unit) == FACTION_BLUE || !BigUnit_Listed(unit) || BigUnit_IsBig(unit))
		return;

	for (i = 0; i < BIG_UNIT_GROUPS; i++) {
		if (BigUnit_GroupEmpty(&gBigUnitGroups[i])) {
			group = &gBigUnitGroups[i];
			break;
		}
	}
	if (!group)
		return;

	group->uid[BIG_CORNER_TL] = UID(unit);
	group->x = unit->xPos;
	group->y = unit->yPos;

	for (i = BIG_CORNER_TR; i < BIG_CORNERS; i++) {
		copy = GetFreeUnit(UNIT_FACTION(unit));
		if (!copy)
			break;

		CopyUnit(unit, copy);
		copy->xPos = unit->xPos + gBigUnitCornerOffsets[i][0];
		copy->yPos = unit->yPos + gBigUnitCornerOffsets[i][1];
		copy->pMapSpriteHandle = NULL;
		group->uid[i] = UID(copy);
	}
}

/* Moves a corner to its tile on the block, on the unit map and its sprite handle too */
static void BigUnit_PlaceCorner(struct Unit *unit, int x, int y)
{
	if (x < 0 || y < 0 || x >= gBmMapSize.x || y >= gBmMapSize.y)
		return;

	if (unit->xPos >= 0 && unit->yPos >= 0 && unit->xPos < gBmMapSize.x && unit->yPos < gBmMapSize.y &&
	    gBmMapUnit[unit->yPos][unit->xPos] == UID(unit))
		gBmMapUnit[unit->yPos][unit->xPos] = 0;

	unit->xPos = x;
	unit->yPos = y;

	if (!(unit->state & US_HIDDEN))
		gBmMapUnit[y][x] = UID(unit);

	if (unit->pMapSpriteHandle) {
		unit->pMapSpriteHandle->xDisplay = x * 16;
		unit->pMapSpriteHandle->yDisplay = y * 16;
	}
}

static bool BigUnit_OnMap(int x, int y)
{
	return x >= 0 && y >= 0 && x < gBmMapSize.x && y < gBmMapSize.y;
}

/*
 * The block is where its corners are: when one corner moved (the one that acted, or an event moved it), the block's
 * top-left follows it and the other corners are put back around it.
 */
static void BigUnit_SyncPosition(struct BigUnitGroup *group)
{
	struct Unit *unit;
	int c, x, y;

	for (c = 0; c < BIG_CORNERS; c++) {
		unit = BigUnit_GetCorner(group, c);
		if (!unit || (unit->state & US_RESCUED))
			continue;
		x = unit->xPos - gBigUnitCornerOffsets[c][0];
		y = unit->yPos - gBigUnitCornerOffsets[c][1];
		if (x != group->x || y != group->y)
			break;
	}
	if (c == BIG_CORNERS)
		return;

	group->x = x;
	group->y = y;
	for (c = 0; c < BIG_CORNERS; c++) {
		unit = BigUnit_GetCorner(group, c);
		if (!unit || (unit->state & US_RESCUED))
			continue;
		BigUnit_PlaceCorner(unit, x + gBigUnitCornerOffsets[c][0], y + gBigUnitCornerOffsets[c][1]);
	}
}

/* Once the active corner has acted (grayed), so has the whole block: one action per phase */
static void BigUnit_SyncActed(struct BigUnitGroup *group)
{
	struct Unit *unit;
	int c;

	if (!gActiveUnit || !(gActiveUnit->state & US_UNSELECTABLE) || BigUnit_GetGroup(gActiveUnit, NULL) != group)
		return;

	for (c = 0; c < BIG_CORNERS; c++) {
		unit = BigUnit_GetCorner(group, c);
		if (!unit || (unit->state & US_UNSELECTABLE))
			continue;

		unit->state |= US_UNSELECTABLE | US_HAS_MOVED_AI;
		if (unit->pMapSpriteHandle)
			unit->pMapSpriteHandle->oam2Base = (unit->pMapSpriteHandle->oam2Base & 0x0FFF) |
				((GetUnitDisplayedSpritePalette(unit) & 0xF) << 12);
	}
}

void BigUnit_SyncAll(void)
{
	int i;

	for (i = 0; i < BIG_UNIT_GROUPS; i++) {
		if (BigUnit_GroupEmpty(&gBigUnitGroups[i]))
			continue;
		BigUnit_SyncPosition(&gBigUnitGroups[i]);
		BigUnit_SyncActed(&gBigUnitGroups[i]);
	}
}

/* The other corners off the unit map (lift) or back on it, so the block can move onto their tiles */
static void BigUnit_LiftOthers(struct Unit *unit, bool lift)
{
	struct BigUnitGroup *group = BigUnit_GetGroup(unit, NULL);
	struct Unit *other;
	int c;

	if (!group)
		return;

	for (c = 0; c < BIG_CORNERS; c++) {
		other = BigUnit_GetCorner(group, c);
		if (!other || other == unit || (other->state & (US_RESCUED | US_HIDDEN)))
			continue;
		if (!BigUnit_OnMap(other->xPos, other->yPos))
			continue;
		if (lift && gBmMapUnit[other->yPos][other->xPos] == UID(other))
			gBmMapUnit[other->yPos][other->xPos] = 0;
		else if (!lift && gBmMapUnit[other->yPos][other->xPos] == 0)
			gBmMapUnit[other->yPos][other->xPos] = UID(other);
	}
}

static bool BigUnit_AiDecideAs(struct Unit *unit)
{
	gActiveUnitId = UID(unit);
	gActiveUnit = unit;
	gAiState.decideState = 0;
	AiClearDecision();

	BigUnit_LiftOthers(unit, true);
	AiDecideMainFunc();
	BigUnit_LiftOthers(unit, false);

	return gAiDecision.actionPerformed && gAiDecision.actionId != AI_ACTION_NONE;
}

/*
 * CpDecide_Main's decision for gActiveUnit. A 2x2 unit acts once per phase, from its best placed corner: each corner
 * that has not decided yet is asked in turn, and the first that can act (fight, a staff, an item) does, as
 * gActiveUnit. If none can, the first corner's decision stands: a move, or nothing.
 */
void BigUnit_AiDecide(void)
{
	struct BigUnitGroup *group = BigUnit_GetGroup(gActiveUnit, NULL);
	struct Unit *first = gActiveUnit, *unit;
	struct AiDecision decision;
	int c;

	if (!group) {
		AiDecideMainFunc();
		return;
	}

	if (BigUnit_AiDecideAs(first))
		return;

	decision = gAiDecision;
	for (c = 0; c < BIG_CORNERS; c++) {
		unit = BigUnit_GetCorner(group, c);
		if (!unit || unit == first || (unit->state & (US_HAS_MOVED_AI | US_UNSELECTABLE)))
			continue;
		if (BigUnit_AiDecideAs(unit))
			return;
	}

	gActiveUnitId = UID(first);
	gActiveUnit = first;
	gAiDecision = decision;
}

/*
 * The end of PreGenerateMovementMap, for a corner's movement map: a tile is open to it only where each other corner's
 * tile (the same offset away) is on the map, passable for its class and free of other units. The flood adds this
 * barrier to the terrain cost, so the block neither stops nor passes where it does not fit.
 */
void BigUnit_MarkFootprintBarriers(struct Unit *unit)
{
	struct BigUnitGroup *group;
	int corner, c, dx, dy, ix, iy, tx, ty, uid;
	bool blocked = false;

	group = BigUnit_GetGroup(unit, &corner);
	if (!group)
		return;

	for (c = 0; c < BIG_CORNERS; c++) {
		if (c == corner || !BigUnit_GetCorner(group, c))
			continue;

		dx = gBigUnitCornerOffsets[c][0] - gBigUnitCornerOffsets[corner][0];
		dy = gBigUnitCornerOffsets[c][1] - gBigUnitCornerOffsets[corner][1];

		for (iy = 0; iy < gBmMapSize.y; iy++) {
			for (ix = 0; ix < gBmMapSize.x; ix++) {
				tx = ix + dx;
				ty = iy + dy;
				if (BigUnit_OnMap(tx, ty) && gWorkingTerrainMoveCosts[gBmMapTerrain[ty][tx]] != (u8)-1) {
					uid = gBmMapUnit[ty][tx];
					if (!uid || BigUnit_GetGroup(GetUnit(uid), NULL) == group)
						continue;
				}
				KernelExtMoveBarrierMap[iy][ix] = -1;
				blocked = true;
			}
		}
	}

	if (blocked)
		KernelMoveMapFlags |= FMOVSTRE_BARRIER;
}

/* A corner leaves its group when its unit is cleared (an enemy's death, the end of a chapter) */
void BigUnit_OnClearUnit(struct Unit *unit)
{
	struct BigUnitGroup *group;
	int corner;

	/* The battle units are copies: clearing one must not break the real corner */
	if (!UID(unit) || GetUnit(UID(unit)) != unit)
		return;

	group = BigUnit_GetGroup(unit, &corner);
	if (group)
		group->uid[corner] = 0;
}

/* A corner at 0 HP breaks: its tile empties and the others fight on */
void BigUnit_OnUnitKill(struct Unit *unit)
{
	BigUnit_OnClearUnit(unit);
}

/* The chapter's end (ChapterChangeUnitCleanup, before the save, which clears the enemies and NPCs right after) */
void BigUnit_ChapterEnd(void)
{
	CpuFill16(0, gBigUnitGroups, sizeof(gBigUnitGroups));
}

void BigUnit_Save(u8 *dst, const u32 size)
{
	Assert(size >= sizeof(gBigUnitGroups));
	WriteAndVerifySramFast(gBigUnitGroups, dst, sizeof(gBigUnitGroups));
}

void BigUnit_Load(u8 *src, const u32 size)
{
	int i, c;

	Assert(size >= sizeof(gBigUnitGroups));
	ReadSramFast(src, gBigUnitGroups, sizeof(gBigUnitGroups));

	for (i = 0; i < BIG_UNIT_GROUPS; i++)
		for (c = 0; c < BIG_CORNERS; c++)
			if (gBigUnitGroups[i].uid[c] >= 0xD0)
				gBigUnitGroups[i].uid[c] = 0;
}

/*
 * A 2x2 enemy's range, shown when the player selects one of its corners (DisplayUnitEffectRange): every tile the block
 * can cover, at the cost of the move that covers it, so its attack range is the four corners' too. For the display
 * only: the AI moves on the corner's own map. New tiles are marked 0x80 + cost until the pass is over.
 */
void BigUnit_ExpandMovementMap(struct Unit *unit)
{
	struct BigUnitGroup *group;
	int corner, c, dx, dy, ix, iy, tx, ty, v;

	group = BigUnit_GetGroup(unit, &corner);
	if (!group)
		return;

	for (iy = 0; iy < gBmMapSize.y; iy++) {
		for (ix = 0; ix < gBmMapSize.x; ix++) {
			v = gBmMapMovement[iy][ix];
			if (v > MAP_MOVEMENT_MAX)
				continue;

			for (c = 0; c < BIG_CORNERS; c++) {
				if (c == corner || !BigUnit_GetCorner(group, c))
					continue;
				dx = gBigUnitCornerOffsets[c][0] - gBigUnitCornerOffsets[corner][0];
				dy = gBigUnitCornerOffsets[c][1] - gBigUnitCornerOffsets[corner][1];
				tx = ix + dx;
				ty = iy + dy;
				if (!BigUnit_OnMap(tx, ty) || gBmMapMovement[ty][tx] <= MAP_MOVEMENT_MAX)
					continue;
				if (gBmMapMovement[ty][tx] > 0x80 + v)
					gBmMapMovement[ty][tx] = 0x80 + v;
			}
		}
	}

	for (iy = 0; iy < gBmMapSize.y; iy++)
		for (ix = 0; ix < gBmMapSize.x; ix++)
			if (gBmMapMovement[iy][ix] >= 0x80 && gBmMapMovement[iy][ix] != 0xFF)
				gBmMapMovement[iy][ix] -= 0x80;
}

/*
 * Whether a 2x2 unit of this character still has a corner standing (HP above 0): its defeat quote, and the defeated
 * flag that ends a "defeat the boss" chapter, wait for the last corner (CheckBattleDefeatTalk, DisplayDefeatTalkForPid).
 */
bool BigUnit_PidStillStanding(int pid)
{
	struct Unit *unit;
	int i, c;

	for (i = 0; i < BIG_UNIT_GROUPS; i++) {
		for (c = 0; c < BIG_CORNERS; c++) {
			unit = BigUnit_GetCorner(&gBigUnitGroups[i], c);
			if (unit && unit->curHP > 0 && unit->pCharacterData->number == pid)
				return true;
		}
	}
	return false;
}
