#include "common-chax.h"
#include "weapon-range.h"
#include "mokha-aoe.h"
#include "constants/texts.h"

STATIC_DECLAR const struct MenuDef sGambitSelectMenuDef;
STATIC_DECLAR const struct MenuItemDef sGambitSelectMenuItems[];

STATIC_DECLAR u8 GambitSelectMenu_OnCancel(struct MenuProc *menu, struct MenuItemProc *item);
STATIC_DECLAR int GambitSelectMenu_OnDraw(struct MenuProc *menu, struct MenuItemProc *item);
STATIC_DECLAR void Gambit_ShowAttackRange(int attackIndex);
STATIC_DECLAR bool Gambit_AttackHasTargets(int attackIndex);

extern const struct SelectInfo gSelectInfo_Gambit;

STATIC_DECLAR bool Gambit_AttackHasTargets(int attackIndex)
{
	const struct MokhaAoeAttackInfo *info = GetMokhaAoeAttackInfo(attackIndex);

	if (!info)
		return false;

	MakeTargetListFor_SubGambitMenu(gActiveUnit, info->range);
	return GetSelectTargetCount() != 0;
}

STATIC_DECLAR void Gambit_ShowAttackRange(int attackIndex)
{
	const struct MokhaAoeAttackInfo *info = GetMokhaAoeAttackInfo(attackIndex);
	u8 range = info ? info->range : 2;

	HideMoveRangeGraphics();
	BmMapFill(gBmMapMovement, -1);
	BmMapFill(gBmMapRange, 0);
	gWorkingBmMap = gBmMapRange;
	FillRangeMapForHover(gActiveUnit, range);
	DisplayMoveRangeGraphics(MOVLIMITV_RMAP_GREEN);
}

u8 Gambit_UpperMenu_Usability(const struct MenuItemDef *def, int number)
{
	int i;

	(void)def;
	(void)number;

	if (gActiveUnit->state & (US_CANTOING | US_HAS_MOVED | US_IN_BALLISTA))
		return MENU_NOTSHOWN;

	if (!IsUnitMokhaAoeEligible(gActiveUnit))
		return MENU_NOTSHOWN;

	for (i = 0; i < MOKHA_AOE_ATK_COUNT; i++) {
		if (Gambit_AttackHasTargets(i))
			return MENU_ENABLED;
	}

	return MENU_NOTSHOWN;
}

u8 Gambit_UpperMenu_Effect(struct MenuProc *menu, struct MenuItemProc *item)
{
	(void)menu;
	(void)item;

	ResetIconGraphics();
	LoadIconPalettes(4);
	StartOrphanMenu(&sGambitSelectMenuDef);

	return MENU_ACT_SKIPCURSOR | MENU_ACT_END | MENU_ACT_SND6A | MENU_ACT_CLEAR;
}

int Gambit_UpperMenu_Hover(struct MenuProc *menu, struct MenuItemProc *item)
{
	(void)menu;
	(void)item;

	Gambit_ShowAttackRange(MOKHA_AOE_ATK_DEFAULT);
	return 0;
}

int Gambit_UpperMenu_Unhover(struct MenuProc *menu, struct MenuItemProc *item)
{
	(void)menu;
	(void)item;

	GambitResetMaps();
	return 0;
}

u8 GambitSelectMenu_Usability(const struct MenuItemDef *def, int number)
{
	(void)def;

	if (!IsUnitMokhaAoeEligible(gActiveUnit))
		return MENU_NOTSHOWN;

	if (!Gambit_AttackHasTargets(number))
		return MENU_DISABLED;

	return MENU_ENABLED;
}

STATIC_DECLAR int GambitSelectMenu_OnDraw(struct MenuProc *menu, struct MenuItemProc *item)
{
	const struct MokhaAoeAttackInfo *info = GetMokhaAoeAttackInfo(item->itemNumber);
	int color = TEXT_COLOR_SYSTEM_WHITE;

	if (item->availability == MENU_DISABLED)
		color = TEXT_COLOR_SYSTEM_GRAY;

	Text_SetColor(&item->text, color);
	Text_DrawString(&item->text, GetStringFromIndex(info->nameMsg));
	PutText(
		&item->text,
		TILEMAP_LOCATED(BG_GetMapBuffer(menu->frontBg), item->xTile, item->yTile));

	return 0;
}

u8 GambitSelectMenu_Effect(struct MenuProc *menu, struct MenuItemProc *item)
{
	const struct MokhaAoeAttackInfo *info = GetMokhaAoeAttackInfo(item->itemNumber);

	if (item->availability == MENU_DISABLED) {
		MenuFrozenHelpBox(menu, info ? info->descMsg : MSG_SKILL_CommonFail);
		return MENU_ACT_SND6B;
	}

	sGambitSelectedAttack = item->itemNumber;
	gActionData.unk08 = item->itemNumber;
	gActionData.itemSlotIndex = 0;

	ClearBg0Bg1();
	MakeTargetListFor_SubGambitMenu(gActiveUnit, info->range);
	NewTargetSelection(&gSelectInfo_Gambit);

	return MENU_ACT_SKIPCURSOR | MENU_ACT_END | MENU_ACT_SND6A | MENU_ACT_CLEAR;
}

int GambitSelectMenu_Hover(struct MenuProc *menu, struct MenuItemProc *item)
{
	(void)menu;

	sGambitSelectedAttack = item->itemNumber;
	Gambit_ShowAttackRange(item->itemNumber);
	return 0;
}

int GambitSelectMenu_Unhover(struct MenuProc *menu, struct MenuItemProc *item)
{
	(void)item;

	if (!(menu->state & MENU_STATE_ENDING))
		GambitResetMaps();

	return 0;
}

STATIC_DECLAR u8 GambitSelectMenu_OnCancel(struct MenuProc *menu, struct MenuItemProc *item)
{
	(void)item;

	GambitResetMaps();
	return ItemMenu_ButtonBPressed(menu, item);
}

void RebuildGambitSelectMenu(void)
{
	ResetTextFont();
	BG_Fill(gBG2TilemapBuffer, 0);
	BG_EnableSyncByMask(BG2_SYNC_BIT);

	ResetIconGraphics();
	LoadIconPalettes(4);
	StartOrphanMenu(&sGambitSelectMenuDef);
}

STATIC_DECLAR const struct MenuItemDef sGambitSelectMenuItems[] = {
	{"", MSG_MokhaAoe_Default_NAME, MSG_MokhaAoe_Default_DESC, TEXT_COLOR_SYSTEM_WHITE, 0, GambitSelectMenu_Usability, GambitSelectMenu_OnDraw, GambitSelectMenu_Effect, 0, GambitSelectMenu_Hover, GambitSelectMenu_Unhover},
	{"", MSG_MokhaAoe_Blaze_NAME, MSG_MokhaAoe_Blaze_DESC, TEXT_COLOR_SYSTEM_WHITE, 1, GambitSelectMenu_Usability, GambitSelectMenu_OnDraw, GambitSelectMenu_Effect, 0, GambitSelectMenu_Hover, GambitSelectMenu_Unhover},
	{"", MSG_MokhaAoe_Absorption_NAME, MSG_MokhaAoe_Absorption_DESC, TEXT_COLOR_SYSTEM_WHITE, 2, GambitSelectMenu_Usability, GambitSelectMenu_OnDraw, GambitSelectMenu_Effect, 0, GambitSelectMenu_Hover, GambitSelectMenu_Unhover},
	{"", MSG_MokhaAoe_Fusillade_NAME, MSG_MokhaAoe_Fusillade_DESC, TEXT_COLOR_SYSTEM_WHITE, 3, GambitSelectMenu_Usability, GambitSelectMenu_OnDraw, GambitSelectMenu_Effect, 0, GambitSelectMenu_Hover, GambitSelectMenu_Unhover},
	{"", MSG_MokhaAoe_FireArrows_NAME, MSG_MokhaAoe_FireArrows_DESC, TEXT_COLOR_SYSTEM_WHITE, 4, GambitSelectMenu_Usability, GambitSelectMenu_OnDraw, GambitSelectMenu_Effect, 0, GambitSelectMenu_Hover, GambitSelectMenu_Unhover},
	{"", MSG_MokhaAoe_GroupLance_NAME, MSG_MokhaAoe_GroupLance_DESC, TEXT_COLOR_SYSTEM_WHITE, 5, GambitSelectMenu_Usability, GambitSelectMenu_OnDraw, GambitSelectMenu_Effect, 0, GambitSelectMenu_Hover, GambitSelectMenu_Unhover},
	{0},
};

STATIC_DECLAR const struct MenuDef sGambitSelectMenuDef = {
	{1, 1, 0x10, 0},
	0,
	sGambitSelectMenuItems,
	0, 0, 0,
	GambitSelectMenu_OnCancel,
	MenuAutoHelpBoxSelect,
	MenuStdHelpBox,
};
