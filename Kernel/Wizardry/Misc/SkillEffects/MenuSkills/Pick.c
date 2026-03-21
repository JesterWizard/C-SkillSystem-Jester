#include "common-chax.h"
#include "kernel/traps.h"
#include "skill-system.h"

/**
 * Check for adjacent (4-directional) traps of a given type.
 */
static struct Trap *GetAdjacentTrap(int xPos, int yPos, int trapType, int *outDx, int *outDy)
{
	struct Trap *trap;

	// Check 4 adjacent tiles (up, down, left, right)
	trap = GetTrapAt(xPos, yPos - 1);
	if (trap && trap->type == trapType) {
		*outDx = 0;
		*outDy = -1;
		return trap;
	}

	trap = GetTrapAt(xPos, yPos + 1);
	if (trap && trap->type == trapType) {
		*outDx = 0;
		*outDy = 1;
		return trap;
	}

	trap = GetTrapAt(xPos - 1, yPos);
	if (trap && trap->type == trapType) {
		*outDx = -1;
		*outDy = 0;
		return trap;
	}

	trap = GetTrapAt(xPos + 1, yPos);
	if (trap && trap->type == trapType) {
		*outDx = 1;
		*outDy = 0;
		return trap;
	}

	return NULL;
}

static struct Trap *GetAdjacentTorch(int xPos, int yPos)
{
	int dx = 0;
	int dy = 0;

	return GetAdjacentTrap(xPos, yPos, TRAP_TOGGLE_TORCH, &dx, &dy);
}

static bool DoesTerrainBlockBoulder(int terrain)
{
	switch (terrain) {
	case TERRAIN_VILLAGE_03:
	case TERRAIN_VILLAGE_04:
	case TERRIAN_HOUSE:
	case TERRAIN_ARMORY:
	case TERRAIN_VENDOR:
	case TERRAIN_ARENA_08:
	case TERRAIN_ARENA_30:
	case TERRAIN_CHURCH:
	case TERRAIN_INN:
	case TERRAIN_WALL_REGULAR:
	case TERRAIN_WALL_DAMAGED:
	case TERRAIN_RUBBLE:
	case TERRAIN_PILLAR:
	case TERRAIN_DOOR:
	case TERRAIN_CHEST_20:
	case TERRAIN_CHEST_21:
	case TERRAIN_ROOF:
	case TERRAIN_CLIFF:
	case TERRAIN_SNAG:
	case TERRAIN_FENCE_32:
	case TERRAIN_BARREL:
	case TERRAIN_BONE:
	case TERRAIN_RIVER:
	case TERRAIN_WATER:
	case TERRAIN_SEA:
	case TERRAIN_LAKE:
	case TERRAIN_DEEPS:
	case TERRAIN_SKY:
		return true;

	default:
		return false;
	}
}

static bool CanBoulderOccupyTile(int x, int y)
{
	if (!IsPositionValid(x, y))
		return false;

	if (gBmMapUnit[y][x])
		return false;

	if (gBmMapHidden[y][x] & HIDDEN_BIT_UNIT)
		return false;

	if (GetTrapAt(x, y))
		return false;

	if (DoesTerrainBlockBoulder(gBmMapTerrain[y][x]))
		return false;

	return true;
}

static int GetBoulderPushDistance(struct Unit *unit, const struct Trap *trap, int dx, int dy)
{
	int step;
	int maxPush = GetUnitPower(unit) / 10;
	int bestDistance = 0;

	if (maxPush <= 0)
		return 0;

	for (step = 1; step <= maxPush; ++step) {
		int destX = trap->xPos + dx * step;
		int destY = trap->yPos + dy * step;

		if (!CanBoulderOccupyTile(destX, destY))
			break;

		bestDistance = step;
	}

	return bestDistance;
}

static struct Trap *GetPushableAdjacentBoulder(int xPos, int yPos, int *outDx, int *outDy, int *outDistance)
{
	struct Trap *trap;

	trap = GetAdjacentTrap(xPos, yPos, TRAP_BOULDER_TILE, outDx, outDy);
	if (!trap)
		return NULL;

	*outDistance = GetBoulderPushDistance(gActiveUnit, trap, *outDx, *outDy);
	if (*outDistance <= 0)
		return NULL;

	return trap;
}

/**
 * Interact command: toggle adjacent torches
 */
u8 InteractCommandUsability(const struct MenuItemDef *def, int number)
{
	int dx = 0;
	int dy = 0;
	int pushDistance = 0;

	if (gActiveUnit->state & US_CANTOING)
		return MENU_NOTSHOWN;

	if (GetAdjacentTorch(gActiveUnit->xPos, gActiveUnit->yPos))
		return MENU_ENABLED;

	if (GetPushableAdjacentBoulder(gActiveUnit->xPos, gActiveUnit->yPos, &dx, &dy, &pushDistance))
		return MENU_ENABLED;

	return MENU_NOTSHOWN;
}

u8 InteractCommandEffect(struct MenuProc *menu, struct MenuItemProc *menuItem)
{
	int dx = 0;
	int dy = 0;
	int pushDistance = 0;
	struct Trap *trap;

	trap = GetAdjacentTorch(gActiveUnit->xPos, gActiveUnit->yPos);
	if (trap && trap->type == TRAP_TOGGLE_TORCH) {
		int defaultDuration = trap->data[TRAP_EXTDATA_TOGGLE_TORCH_DURATION];

		if (defaultDuration <= 0)
			defaultDuration = 3;

		trap->extra = (trap->extra > 0) ? 0 : defaultDuration;

		gActionData.unitActionType = UNIT_ACTION_WAIT;
		RefreshEntityBmMaps();
		RenderBmMap();
		RefreshUnitSprites();

		PlaySoundEffect(0x6A);
		return MENU_ACT_SKIPCURSOR | MENU_ACT_END | MENU_ACT_SND6A | MENU_ACT_CLEAR;
	}

	trap = GetPushableAdjacentBoulder(gActiveUnit->xPos, gActiveUnit->yPos, &dx, &dy, &pushDistance);
	if (trap && trap->type == TRAP_BOULDER_TILE) {
		trap->xPos += dx * pushDistance;
		trap->yPos += dy * pushDistance;

		gActionData.unitActionType = UNIT_ACTION_WAIT;
		RefreshEntityBmMaps();
		RenderBmMap();
		RefreshUnitSprites();

		PlaySoundEffect(0x6A);
		return MENU_ACT_SKIPCURSOR | MENU_ACT_END | MENU_ACT_SND6A | MENU_ACT_CLEAR;
	}

	return MENU_ACT_SND6B;
}

LYN_REPLACE_CHECK(PickCommandUsability);
u8 PickCommandUsability(const struct MenuItemDef *def, int number)
{
	struct Trap *trap;

	if (gActiveUnit->state & US_CANTOING)
		return MENU_NOTSHOWN;

	trap = GetTrapAt(gActiveUnit->xPos, gActiveUnit->yPos);
	if (trap && trap->type == TRAP_TOGGLE_TORCH)
		return MENU_ENABLED;

#if !CHAX
	if (gActiveUnit->pClassData->number != CLASS_ROGUE)
		return MENU_NOTSHOWN;
#endif

	MakeTargetListForPick(gActiveUnit);
	if (GetSelectTargetCount() == 0)
		return MENU_NOTSHOWN;

	return MENU_ENABLED;
}

LYN_REPLACE_CHECK(PickCommandEffect);
u8 PickCommandEffect(struct MenuProc *menu, struct MenuItemProc *menuItem)
{
	struct Trap *trap;

	if (menuItem->availability == MENU_DISABLED) {
		MenuFrozenHelpBox(menu, 0x856);
		return MENU_ACT_SND6B;
	}

	trap = GetTrapAt(gActiveUnit->xPos, gActiveUnit->yPos);
	if (trap && trap->type == TRAP_TOGGLE_TORCH) {
		int defaultDuration = trap->data[TRAP_EXTDATA_TOGGLE_TORCH_DURATION];

		if (defaultDuration <= 0)
			defaultDuration = 3;

		trap->extra = (trap->extra > 0) ? 0 : defaultDuration;

		gActionData.unitActionType = UNIT_ACTION_WAIT;
		RefreshEntityBmMaps();
		RenderBmMap();
		RefreshUnitSprites();

		PlaySoundEffect(0x6A);
		return MENU_ACT_SKIPCURSOR | MENU_ACT_END | MENU_ACT_SND6A | MENU_ACT_CLEAR;
	}

	ClearBg0Bg1();
	MakeTargetListForPick(gActiveUnit);
	BmMapFill(gBmMapMovement, -1);
	NewTargetSelection(&gSelectInfo_Pick);

	PlaySoundEffect(0x6A);
	return MENU_ACT_SKIPCURSOR | MENU_ACT_END | MENU_ACT_SND6A;
}
