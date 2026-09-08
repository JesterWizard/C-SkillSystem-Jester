#include "common-chax.h"
#include "icon-rework.h"
#include "map-anims.h"
#include "skill-system.h"
#include "battle-system.h"
#include "constants/skills.h"
#include "constants/texts.h"
#include "debuff.h"
#include "prepscreen.h"
#include "playst-expa.h"
#include "action-expa.h"
#include "jester_headers/miscellaneous.h"
#include "jester_headers/custom-functions.h"
#include "jester_headers/custom-arrays.h"
#include "jester_headers/custom-structs.h"

#ifndef CONFIG_UNIT_ACTION_EXPA_ExecSkill
    #define CONFIG_UNIT_ACTION_EXPA_ExecSkill 20
#endif

extern u16 gUnknown_085A0D4C[];

#define EMERGENCY_EXIT_PLUS_VISIBLE_COUNT 5

/* Fill gEmergencyExitCandidates with undeployed allies (returns count) */
static int CollectUndeployedUnits(void)
{
    memset(gUndeployedUnitCount, 0xFF, sizeof(gUndeployedUnitCount));

    int unitCounter = 0;

    for (int i = FACTION_BLUE; i < MAX_UNDEPLOYED_UNIT_COUNT; ++i) {
        struct Unit* unit = GetUnit(i);

        if (!UNIT_IS_VALID(unit))
            continue;

        if (unit->state & US_NOT_DEPLOYED) {
            gUndeployedUnitCount[unitCounter++] =
                unit->pCharacterData->number;
        }
    }

    gList_Total = unitCounter;
    return unitCounter;
}

#if defined(SID_EmergencyExitPlus) && (COMMON_SKILL_VALID(SID_EmergencyExitPlus))

STATIC_DECLAR const struct MenuItemDef EmergencyExitPlusMenuItems[];
STATIC_DECLAR u8 EmergencyExitPlusMenu_HelpBox(struct MenuProc * menu, struct MenuItemProc * item);
STATIC_DECLAR u8 EmergencyExitPlus_OnCancel(struct MenuProc * menu, struct MenuItemProc * item);
STATIC_DECLAR u8 EmergencyExitPlusMenu_Usability(const struct MenuItemDef * self, int number);
STATIC_DECLAR int EmergencyExitPlusMenu_OnDraw(struct MenuProc * menu, struct MenuItemProc * item);
STATIC_DECLAR u8 EmergencyExitPlusMenu_OnSelected(struct MenuProc * menu, struct MenuItemProc * item);

static bool EmergencyExitPlus_ShouldShowScrollBar(void)
{
    return (gList_Total > EMERGENCY_EXIT_PLUS_VISIBLE_COUNT);
}

static void EmergencyExitPlus_UpdateScrollBar(void)
{
    if (!EmergencyExitPlus_ShouldShowScrollBar())
        return;

    UpdateMenuScrollBarConfig(
        gList_Total,
        gTopVisibleListIndex * 16,
        gList_Total,
        EMERGENCY_EXIT_PLUS_VISIBLE_COUNT
    );
}

static void EmergencyExitPlus_StartScrollBar(struct MenuProc * menu)
{
    if (!EmergencyExitPlus_ShouldShowScrollBar())
        return;

    StartMenuScrollBar(menu);
    PutMenuScrollBarAt(200, 32);
    InitMenuScrollBarImg(0x7A60, 2);
    EmergencyExitPlus_UpdateScrollBar();
}

static void EmergencyExitPlus_EndScrollBar(void)
{
    if (!EmergencyExitPlus_ShouldShowScrollBar())
        return;

    EndMenuScrollBar();
}

static int EmergencyExitPlus_GetVisibleItemCount(void)
{
    if (gList_Total < EMERGENCY_EXIT_PLUS_VISIBLE_COUNT)
        return gList_Total;

    return EMERGENCY_EXIT_PLUS_VISIBLE_COUNT;
}

static int EmergencyExitPlus_GetAbsoluteIndex(int itemNumber)
{
    return gTopVisibleListIndex + itemNumber;
}

static const struct CharacterData * EmergencyExitPlus_GetCharacterForVisibleIndex(int itemNumber)
{
    int index = EmergencyExitPlus_GetAbsoluteIndex(itemNumber);

    if ((index < 0) || (index >= gList_Total))
        return NULL;

    return GetCharacterData(gUndeployedUnitCount[index]);
}

static void EmergencyExitPlus_DrawPortrait(int itemNumber)
{
    const struct CharacterData * undeployedUnit = EmergencyExitPlus_GetCharacterForVisibleIndex(itemNumber);

    if (!undeployedUnit)
        return;

    PutFace80x72_Core(
        gBG0TilemapBuffer + TILEMAP_INDEX(3, 5),
        undeployedUnit->portraitId,
        0x200,
        5
    );

    BG_EnableSyncByMask(BG0_SYNC_BIT);
}

void EmergencyExitPlus_ResetMenuState(void)
{
    gList_Total = 0;
    gTopVisibleListIndex = 0;
}

u8 EmergencyExitPlus_GetCharIdForVisibleIndex(int itemNumber)
{
    int index = EmergencyExitPlus_GetAbsoluteIndex(itemNumber);

    if ((index < 0) || (index >= gList_Total))
        return 0;

    return gUndeployedUnitCount[index];
}

bool EmergencyExitPlus_HandleMenuScroll(struct MenuProc * menu)
{
    int visibleCount = EmergencyExitPlus_GetVisibleItemCount();
    int maxOffset = gList_Total - visibleCount;

    if (visibleCount <= 0)
        return false;

    if (gList_Total <= visibleCount)
        return false;

    if (gKeyStatusPtr->repeatedKeys & DPAD_UP)
    {
        if (menu->itemCurrent == 0)
        {
            if (gTopVisibleListIndex > 0)
            {
                gTopVisibleListIndex--;
                RedrawMenu(menu);
                DrawMenuItemHover(menu, menu->itemCurrent, TRUE);
                EmergencyExitPlus_DrawPortrait(menu->itemCurrent);
                EmergencyExitPlus_UpdateScrollBar();
                return true;
            }

            if (gKeyStatusPtr->repeatedKeys != gKeyStatusPtr->newKeys)
                return true;

            gTopVisibleListIndex = maxOffset;
            menu->itemCurrent = visibleCount - 1;
            RedrawMenu(menu);
            DrawMenuItemHover(menu, menu->itemCurrent, TRUE);
            EmergencyExitPlus_DrawPortrait(menu->itemCurrent);
            EmergencyExitPlus_UpdateScrollBar();
            return true;
        }
    }

    if (gKeyStatusPtr->repeatedKeys & DPAD_DOWN)
    {
        if (menu->itemCurrent == (visibleCount - 1))
        {
            if (gTopVisibleListIndex < maxOffset)
            {
                gTopVisibleListIndex++;
                RedrawMenu(menu);
                DrawMenuItemHover(menu, menu->itemCurrent, TRUE);
                EmergencyExitPlus_DrawPortrait(menu->itemCurrent);
                EmergencyExitPlus_UpdateScrollBar();
                return true;
            }

            if (gKeyStatusPtr->repeatedKeys != gKeyStatusPtr->newKeys)
                return true;

            gTopVisibleListIndex = 0;
            menu->itemCurrent = 0;
            RedrawMenu(menu);
            DrawMenuItemHover(menu, menu->itemCurrent, TRUE);
            EmergencyExitPlus_DrawPortrait(menu->itemCurrent);
            EmergencyExitPlus_UpdateScrollBar();
            return true;
        }
    }

    return false;
}

u8 EmergencyExitPlus_Usability(const struct MenuItemDef * def, int number)
{
    if (gActiveUnit->state & US_CANTOING)
        return MENU_NOTSHOWN;

    if (gActiveUnit->curHP > (gActiveUnit->maxHP / 2))
        return MENU_NOTSHOWN;

    EmergencyExitPlus_ResetMenuState();

    if (CollectUndeployedUnits() <= 0)
        return MENU_NOTSHOWN;

    return MENU_ENABLED;
}

u8 EmergencyExitPlus_OnSelected(struct MenuProc * menu, struct MenuItemProc * item)
{
    if (item->availability == MENU_DISABLED)
    {
        MenuFrozenHelpBox(menu, MSG_No_Allies);
        return MENU_ACT_SND6B;
    }

    gActionData.unk08 = SID_EmergencyExitPlus;
    gActionData.unitActionType = CONFIG_UNIT_ACTION_EXPA_ExecSkill;

    return MENU_ACT_SKIPCURSOR | MENU_ACT_END | MENU_ACT_SND6A;
}

static void callback_anim(ProcPtr proc)
{
    // Clear UI
    BG_Fill(gBG0TilemapBuffer, 0);
    BG_Fill(gBG1TilemapBuffer, 0);
    BG_EnableSyncByMask(BG0_SYNC_BIT | BG1_SYNC_BIT);

	PlaySoundEffect(0x269);
	Proc_StartBlocking(ProcScr_DanceringAnim, proc);

	BG_SetPosition(
		BG_0,
		-SCREEN_TILE_IX(gActiveUnit->xPos - 1),
		-SCREEN_TILE_IX(gActiveUnit->yPos - 2));
}

#define EmergencyExitPlusMenuItem(i) \
{ \
    .helpMsgId = (i), \
    .isAvailable = EmergencyExitPlusMenu_Usability, \
    .onDraw = EmergencyExitPlusMenu_OnDraw, \
    .onSelected = EmergencyExitPlusMenu_OnSelected, \
}

STATIC_DECLAR const struct MenuItemDef EmergencyExitPlusMenuItems[] =
{
    EmergencyExitPlusMenuItem(0),
    EmergencyExitPlusMenuItem(1),
    EmergencyExitPlusMenuItem(2),
    EmergencyExitPlusMenuItem(3),
    EmergencyExitPlusMenuItem(4),
    { 0 }
};

STATIC_DECLAR const struct MenuDef EmergencyExitPlusMenuDef = {
    {15, 2, 10, 0},
    0,
    EmergencyExitPlusMenuItems,
    0, 0, 0,
    EmergencyExitPlus_OnCancel,
    MenuAutoHelpBoxSelect,
    EmergencyExitPlusMenu_HelpBox
};

STATIC_DECLAR u8 EmergencyExitPlusMenu_HelpBox(struct MenuProc * menu, struct MenuItemProc * item)
{
    const struct CharacterData * undeployedUnit = EmergencyExitPlus_GetCharacterForVisibleIndex(item->itemNumber);

    if (!undeployedUnit)
        return 0;

    StartHelpBox(
        item->xTile * 8,
        item->yTile * 8,
        undeployedUnit->descTextId
    );
    
    return 0;
}

STATIC_DECLAR u8 EmergencyExitPlusMenu_Usability(const struct MenuItemDef * self, int number)
{
    if (number < EmergencyExitPlus_GetVisibleItemCount())
        return MENU_ENABLED;

    return MENU_NOTSHOWN;
}

STATIC_DECLAR int EmergencyExitPlusMenu_OnDraw(struct MenuProc * menu, struct MenuItemProc * item)
{
    const struct CharacterData * undeployedUnit = EmergencyExitPlus_GetCharacterForVisibleIndex(item->itemNumber);

    if (!undeployedUnit)
        return 0;

    CallARM_FillTileRect(gBG1TilemapBuffer + 0x42, gUnknown_085A0D4C, 0x1000);

    ClearText(&item->text);
    Text_SetColor(&item->text, TEXT_COLOR_SYSTEM_WHITE);
    Text_DrawString(&item->text, GetStringFromIndex(undeployedUnit->nameTextId));
    PutText(&item->text, TILEMAP_LOCATED(gBG0TilemapBuffer, item->xTile + 1, item->yTile));

    if (item->itemNumber == menu->itemCurrent)
        EmergencyExitPlus_DrawPortrait(item->itemNumber);

    BG_EnableSyncByMask(BG0_SYNC_BIT);
    return 0;
}

STATIC_DECLAR u8 EmergencyExitPlusMenu_OnSelected(struct MenuProc * menu, struct MenuItemProc * item)
{    
    const u8 charId = EmergencyExitPlus_GetCharIdForVisibleIndex(item->itemNumber);

    struct Unit * unit = GetUnit(gActiveUnit->index);
    HideUnitSprite(unit);
    unit->state |= US_HIDDEN;

    gActiveUnit = GetUnitFromCharId(charId);

    gActionDataExpa.refrain_action = true;
    EndAllMus();

    /* Prevent other menus from freezing because of our little dpad hack in ProcessMenuDpadInput */
    gActionData.unk08 = 0;
    EmergencyExitPlus_EndScrollBar();
    EmergencyExitPlus_ResetMenuState();

    return MENU_ACT_SKIPCURSOR | MENU_ACT_END | MENU_ACT_SND6A | MENU_ACT_CLEAR;
}

STATIC_DECLAR u8 EmergencyExitPlus_OnCancel(struct MenuProc * menu, struct MenuItemProc * item)
{
    /* Reset action */
    gActionData.unitActionType = 0;

    BG_Fill(gBG0TilemapBuffer, 0);
    BG_Fill(gBG1TilemapBuffer, 0);
    BG_Fill(gBG2TilemapBuffer, 0);
    BG_EnableSyncByMask(BG0_SYNC_BIT | BG1_SYNC_BIT | BG2_SYNC_BIT);
    HideMoveRangeGraphics();

    /* Prevent other menus from freezing because of our little dpad hack in ProcessMenuDpadInput */
    gActionData.unk08 = 0;
    EmergencyExitPlus_EndScrollBar();
    EmergencyExitPlus_ResetMenuState();

    gActionDataExpa.refrain_action = true;
    EndAllMus();

    return MENU_ACT_SKIPCURSOR | MENU_ACT_END | MENU_ACT_SND6A;
}

static void callback_exec(ProcPtr proc)
{
    struct MenuProc * menu;

    EmergencyExitPlus_ResetMenuState();

    if (CollectUndeployedUnits() <= 0) {
        MenuFrozenHelpBox(NULL, MSG_No_Allies);
        return;
    }

    menu = StartOrphanMenu(&EmergencyExitPlusMenuDef);
    EmergencyExitPlus_StartScrollBar(menu);

    StartSubtitleHelp(menu, GetStringFromIndex(MSG_SelectUndeployedUnit));
}

bool Action_EmergencyExitPlus(ProcPtr parent)
{
	NewMuSkillAnimOnActiveUnit(gActionData.unk08, callback_anim, callback_exec);
	return true;
}
#endif