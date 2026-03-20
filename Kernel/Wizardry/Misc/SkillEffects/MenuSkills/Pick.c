#include "common-chax.h"
#include "skill-system.h"

/**
 * Check for adjacent (4-directional) torches
 */
struct Trap* GetAdjacentTorch(int xPos, int yPos)
{
	struct Trap *trap;

	// Check 4 adjacent tiles (up, down, left, right)
	trap = GetTrapAt(xPos, yPos - 1);
	if (trap && trap->type == TRAP_TOGGLE_TORCH)
		return trap;

	trap = GetTrapAt(xPos, yPos + 1);
	if (trap && trap->type == TRAP_TOGGLE_TORCH)
		return trap;

	trap = GetTrapAt(xPos - 1, yPos);
	if (trap && trap->type == TRAP_TOGGLE_TORCH)
		return trap;

	trap = GetTrapAt(xPos + 1, yPos);
	if (trap && trap->type == TRAP_TOGGLE_TORCH)
		return trap;

	return NULL;
}

/**
 * Interact command: toggle adjacent torches
 */
u8 InteractCommandUsability(const struct MenuItemDef *def, int number)
{
	if (gActiveUnit->state & US_CANTOING)
		return MENU_NOTSHOWN;

	if (GetAdjacentTorch(gActiveUnit->xPos, gActiveUnit->yPos))
		return MENU_ENABLED;

	return MENU_NOTSHOWN;
}

u8 InteractCommandEffect(struct MenuProc *menu, struct MenuItemProc *menuItem)
{
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
