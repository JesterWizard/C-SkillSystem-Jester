#include "common-chax.h"
#include "icon-rework.h"
#include "map-anims.h"
#include "skill-system.h"
#include "battle-system.h"
#include "constants/skills.h"
#include "constants/texts.h"
#include "debuff.h"
#include "playst-expa.h"
#include "action-expa.h"
#include "jester_headers/miscellaneous.h"
#include "jester_headers/custom-functions.h"
#include "jester_headers/custom-arrays.h"

#ifndef CONFIG_UNIT_ACTION_EXPA_ExecSkill
    #define CONFIG_UNIT_ACTION_EXPA_ExecSkill 20
#endif

extern u16 gUnknown_085A0D4C[];

/* Fill gEmergencyExitCandidates with undeployed allies (returns count) */
static void CollectUndeployedUnits(void)
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
}

#if defined(SID_EmergencyExitPlus) && (COMMON_SKILL_VALID(SID_EmergencyExitPlus))

STATIC_DECLAR const struct MenuItemDef EmergencyExitPlusMenuItems[];
STATIC_DECLAR u8 EmergencyExitPlusMenu_HelpBox(struct MenuProc * menu, struct MenuItemProc * item);
STATIC_DECLAR u8 EmergencyExitPlus_OnCancel(struct MenuProc * menu, struct MenuItemProc * item);
STATIC_DECLAR u8 EmergencyExitPlusMenu_Usability(const struct MenuItemDef * self, int number);
STATIC_DECLAR int EmergencyExitPlusMenu_OnDraw(struct MenuProc * menu, struct MenuItemProc * item);
STATIC_DECLAR u8 EmergencyExitPlusMenu_OnSelected(struct MenuProc * menu, struct MenuItemProc * item);

u8 EmergencyExitPlus_Usability(const struct MenuItemDef * def, int number)
{
    if (gActiveUnit->state & US_CANTOING)
        return MENU_NOTSHOWN;

    if (gActiveUnit->curHP > (gActiveUnit->maxHP / 2))
        return MENU_NOTSHOWN;

    CollectUndeployedUnits();

    if (gUndeployedUnitCount[0] == 0xFF)
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
    StartHelpBox(
        item->xTile * 8,
        item->yTile * 8,
        GetCharacterData(gUndeployedUnitCount[item->itemNumber])->descTextId
    );
    
    return 0;
}

STATIC_DECLAR u8 EmergencyExitPlusMenu_Usability(const struct MenuItemDef * self, int number)
{
    return MENU_ENABLED;
}

STATIC_DECLAR int EmergencyExitPlusMenu_OnDraw(struct MenuProc * menu, struct MenuItemProc * item)
{
    const struct CharacterData * undeployedUnit = GetCharacterData(gUndeployedUnitCount[item->itemNumber]);

    CallARM_FillTileRect(gBG1TilemapBuffer + 0x42, gUnknown_085A0D4C, 0x1000);

    Text_SetColor(&item->text, TEXT_COLOR_SYSTEM_GOLD);
    Text_DrawString(&item->text, GetStringFromIndex(undeployedUnit->nameTextId));
    PutText(&item->text, TILEMAP_LOCATED(gBG0TilemapBuffer, item->xTile + 1, item->yTile));

    /* Draw portrait only for first item */
    if (item->itemNumber == 0)
    {
        PutFace80x72_Core(
            gBG0TilemapBuffer + TILEMAP_INDEX(3, 5),
            undeployedUnit->portraitId,
            0x200,
            5
        );
    }

    BG_EnableSyncByMask(BG0_SYNC_BIT);
    return 0;
}

STATIC_DECLAR u8 EmergencyExitPlusMenu_OnSelected(struct MenuProc * menu, struct MenuItemProc * item)
{    
    const u8 menuIndex = MENU_SKILL_INDEX(item->def);

    struct Unit * unit = GetUnit(gActiveUnit->index);
    HideUnitSprite(unit);
    unit->state |= US_HIDDEN;

    gActiveUnit = GetUnitFromCharId(gUndeployedUnitCount[menuIndex]);

    gActionDataExpa.refrain_action = true;
    EndAllMus();

    /* Prevent other menus from freezing because of our little dpad hack in ProcessMenuDpadInput */
    gActionData.unk08 = 0;

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

    return MENU_ACT_SKIPCURSOR | MENU_ACT_END | MENU_ACT_SND6A;
}

static void callback_exec(ProcPtr proc)
{
    if (gUndeployedUnitCount[0] == 0xFF) {
        MenuFrozenHelpBox(NULL, MSG_No_Allies);
        return;
    }

    StartSubtitleHelp(
        StartOrphanMenu(&EmergencyExitPlusMenuDef),
        GetStringFromIndex(MSG_SelectUndeployedUnit)
    );
}

bool Action_EmergencyExitPlus(ProcPtr parent)
{
	NewMuSkillAnimOnActiveUnit(gActionData.unk08, callback_anim, callback_exec);
	return true;
}
#endif