#include "common-chax.h"
#include "skill-system.h"
#include "status-getter.h"
#include "constants/skills.h"
#include "constants/texts.h"
#include "jester_headers/Forging.h"
#include "jester_headers/custom-functions.h"

extern u16 gUnknown_085A0D4C[];

int GetUnitInfoWindowX(struct Unit *unit, int width);

u8 StealPlus_Usability(const struct MenuItemDef *def, int number)
{
#if !CHAX
    if (!(UNIT_CATTRIBUTES(gActiveUnit) & CA_STEAL))
        return MENU_NOTSHOWN;
#endif

    if (gActiveUnit->state & US_CANTOING)
        return MENU_NOTSHOWN;

    MakeTargetListForSteal(gActiveUnit);
    if (GetSelectTargetCount() == 0)
        return MENU_NOTSHOWN;

    if (GetUnitItemCount(gActiveUnit) == UNIT_ITEM_COUNT)
        return MENU_DISABLED;

    return MENU_ENABLED;
}
extern int _GetUnitCon(struct Unit * unit); 

u8 StealPlus_Effect(struct MenuProc *menu, struct MenuItemProc *menuItem)
{
    if (menuItem->availability == MENU_DISABLED)
    {
        MenuFrozenHelpBox(menu, 0x862); // TODO msgid "Your inventory is full.[.]"
        return MENU_ACT_SND6B;
    }

    ClearBg0Bg1();

    MakeTargetListForSteal(gActiveUnit);

    NewTargetSelection(&gSelectInfo_Steal);

    return MENU_ACT_SKIPCURSOR | MENU_ACT_END | MENU_ACT_SND6A;
}

LYN_REPLACE_CHECK(StealMapSelect_Init);
void StealMapSelect_Init(ProcPtr menu)
{
    StartUnitInventoryInfoWindow(menu);
    StartSubtitleHelp(menu, GetStringFromIndex(0x86D)); // TODO: msgid "Select which unit to steal from."

    return;
}

LYN_REPLACE_CHECK(StealMapSelect_SwitchIn);
u8 StealMapSelect_SwitchIn(ProcPtr proc, struct SelectTarget *target)
{
    ChangeActiveUnitFacing(target->x, target->y);

    RefreshUnitStealInventoryInfoWindow(GetUnit(target->uid));

    return 0; // BUG?
}

LYN_REPLACE_CHECK(StealMapSelect_Select);
u8 StealMapSelect_Select(ProcPtr proc, struct SelectTarget *target)
{
    int pos;

    gActionData.targetIndex = target->uid;

    ResetIconGraphics();
    LoadIconPalettes(4);

    StartOrphanMenu(&gStealItemMenuDef);

    EndTargetSelection(proc);

    CallARM_FillTileRect(gBG1TilemapBuffer + 0x42, gUnknown_085A0D4C, 0x1000);

    pos = (56 - GetStringTextLen(GetStringFromIndex(GetUnit(gActionData.targetIndex)->pCharacterData->nameTextId))) / 2;

    PutDrawText(0, gBG0TilemapBuffer + 0x63, 0, pos, 7, GetStringFromIndex(GetUnit(gActionData.targetIndex)->pCharacterData->nameTextId));

    PutFace80x72_Core(gBG0TilemapBuffer + 0x63 + 0x40, GetUnitPortraitId(GetUnit(gActionData.targetIndex)), 0x200, 5);

    return 0;
}

LYN_REPLACE_CHECK(StealItemMenuCommand_Usability);
u8 StealItemMenuCommand_Usability(const struct MenuItemDef *def, int number)
{
    if (GetUnit(gActionData.targetIndex)->items[number] == 0)
    {
        return MENU_NOTSHOWN;
    }

    if (!IsItemStealable(GetUnit(gActionData.targetIndex)->items[number]))
    {
        return MENU_DISABLED;
    }

    if (GetUnitEquippedWeaponSlot(GetUnit(gActionData.targetIndex)) == number)
    {
        return MENU_DISABLED;
    }

#if defined(SID_StealPlus) && (COMMON_SKILL_VALID(SID_StealPlus))
    if (SkillTesterPlus(gActiveUnit, SID_StealPlus))
    {
        if (_GetUnitCon(gActiveUnit) > GetItemWeight(GetUnit(gActionData.targetIndex)->items[number]) && number != 0)
            return MENU_ENABLED;
    }
#endif

    return MENU_ENABLED;
}

LYN_REPLACE_CHECK(StealItemMenuCommand_Draw);
int StealItemMenuCommand_Draw(struct MenuProc *menu, struct MenuItemProc *menuItem)
{
    int item = GetUnit(gActionData.targetIndex)->items[menuItem->itemNumber];
    s8 isStealable = IsItemStealable(item);

#if defined(SID_StealPlus) && (COMMON_SKILL_VALID(SID_StealPlus))
    if (SkillTesterPlus(gActiveUnit, SID_StealPlus) && isStealable)
    {
        if (_GetUnitCon(gActiveUnit) > GetItemWeight(item) && menuItem->itemNumber != 0)
        {
            if (GetUnitEquippedWeaponSlot(GetUnit(gActionData.targetIndex)) == menuItem->itemNumber)
                isStealable = false;
        }
        else
            isStealable = false;
    }
#endif

    DrawItemMenuLine(&menuItem->text, item, isStealable, gBG0TilemapBuffer + TILEMAP_INDEX(menuItem->xTile, menuItem->yTile));

    return 0; // vanilla returns without a type, likely due to older compilers defaulting to 0
}


LYN_REPLACE_CHECK(IsItemStealable);
s8 IsItemStealable(int item) {

    bool stealable = false;

    if (gActionData.unk08 == 5000) // IER_Effect_Forge sets this. It may need to be revisted if we want to have several steal staves with different restrictions
    {
        if (GetItemAttributes(item) & IA_WEAPON || GetItemAttributes(item) & IA_MAGIC || GetItemAttributes(item) & IA_STAFF)
        {
            return true;
        }
        else
        {
            return false;
        }
    }

#if defined(SID_StealPlus) && (COMMON_SKILL_VALID(SID_StealPlus))
    if (SkillTesterPlus(gActiveUnit, SID_StealPlus))
	{
        stealable = (GetItemType(item) == ITYPE_ITEM  || 
        GetItemType(item) == ITYPE_ANIMA ||
        GetItemType(item) == ITYPE_LIGHT ||
        GetItemType(item) == ITYPE_DARK  ||
        GetItemType(item) == ITYPE_STAFF ||
        GetItemType(item) == ITYPE_AXE   ||
        GetItemType(item) == ITYPE_BOW   ||
        GetItemType(item) == ITYPE_SWORD ||
        GetItemType(item) == ITYPE_LANCE);

		return stealable;
	}
#endif

#if defined(SID_Duplicate) && (COMMON_SKILL_VALID(SID_Duplicate))
    if (SkillTester(gActiveUnit, SID_Duplicate))
	{
		stealable = GetItemType(item) != ITYPE_ITEM;
		return stealable;
	}
#endif

    stealable = GetItemType(item) == ITYPE_ITEM;

    return stealable;

};

extern int _GetUnitCon(struct Unit * unit); 

LYN_REPLACE_CHECK(AddAsTarget_IfCanStealFrom);
void AddAsTarget_IfCanStealFrom(struct Unit* unit) {
    int i;

    if (UNIT_FACTION(unit) != FACTION_RED) {
        return;
    }

    if (gActiveUnit->spd < unit->spd) {
        return;
    }

    for (i = 0; i < UNIT_ITEM_COUNT; i++) {
        u16 item = unit->items[i];

        if (item == 0) {
            return;
        }

	    if (gActionData.unk08 == CONFIG_FORGE_CHECKER)
        {
            AddTarget(unit->xPos, unit->yPos, unit->index, 0);
            return;
        }

        if (!IsItemStealable(item)) {
            continue;
        }

#if defined(SID_StealPlus) && (COMMON_SKILL_VALID(SID_StealPlus))
        if (SkillTesterPlus(gActiveUnit, SID_StealPlus)) 
        {
            if (GetUnitEquippedWeaponSlot(unit) == i) {
                continue;
            }

            if (_GetUnitCon(gActiveUnit) <= GetItemWeight(unit->items[i])) {
                continue;
            }
        }
#endif

#if defined(SID_StickyHold) && (COMMON_SKILL_VALID(SID_StickyHold))
        if (SkillTester(unit, SID_StickyHold)) 
            continue;
#endif

        AddTarget(unit->xPos, unit->yPos, unit->index, 0);
        return;
    }

    return;
}

LYN_REPLACE_CHECK(StealItemMenuCommand_Effect);
u8 StealItemMenuCommand_Effect(struct MenuProc *menu, struct MenuItemProc *menuItem)
{

#if defined(SID_Duplicate) && (COMMON_SKILL_VALID(SID_Duplicate))
    if (gActionData.unk08 == SID_Duplicate)
	{
		gActionData.itemSlotIndex = menuItem->itemNumber;
		gActionData.unitActionType = UNIT_ACTION_STEAL;

		return MENU_ACT_SKIPCURSOR | MENU_ACT_END | MENU_ACT_SND6A | MENU_ACT_CLEAR;
	}
#endif

    if (menuItem->availability == MENU_DISABLED)
    {
        if (gActionData.unk08 == CONFIG_FORGE_CHECKER)
        {
            if (GetItemType(GetUnit(gActionData.targetIndex)->items[menuItem->itemNumber]) == ITYPE_ITEM)
            {
                MenuFrozenHelpBox(menu, MSG_ITEM_CANT_FORGE_ITEM);
                return MENU_ACT_SND6B;
            }
        }
        else
        {
            MenuFrozenHelpBox(menu, MSG_ITEM_CANT_STEAL_PLUS);
            return MENU_ACT_SND6B;
        }
    }

    gActionData.itemSlotIndex = menuItem->itemNumber;

        if (gActionData.unk08 == CONFIG_FORGE_CHECKER)
        {
            gActionData.unitActionType = UNIT_ACTION_STAFF;
            UnitAddItem(gActiveUnit, GetUnit(gActionData.targetIndex)->items[menuItem->itemNumber]);
        }
        else
            gActionData.unitActionType = UNIT_ACTION_STEAL;

    return MENU_ACT_SKIPCURSOR | MENU_ACT_END | MENU_ACT_SND6A | MENU_ACT_CLEAR;
}