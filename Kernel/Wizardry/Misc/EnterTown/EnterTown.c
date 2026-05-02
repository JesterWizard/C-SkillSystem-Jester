#include "common-chax.h"
#include "constants/texts.h"
#include "worldmap.h"
#include "gamecontrol.h"
#include "kernel-lib.h"
#include "kernel/prep-skill.h"
#include "bwl.h"
#include "icon-rework.h"
#include "skill-system.h"
#include "popup.h"
#include "constants/texts.h"
#include "constants/faces.h"
#include "jester_headers/procs.h"
#include "jester_headers/custom-functions.h"
#include "jester_headers/custom-structs.h"
#include "jester_headers/custom-arrays.h"

extern u8 MapMenu_IsGuideCommandAvailable(const struct MenuItemDef * def, int number);
extern void sub_80B5D3C(void);
extern struct MenuRect gMenuRect_WMGeneralMenuRect;
extern struct ProcCmd CONST_DATA ProcScr_OpAnim[]; // intro cutscene
extern struct ProcCmd CONST_DATA ProcScr_WorldMapWrapper[];
extern void StartWorldMapThoughtBubble(struct MenuProc * menuProc);
extern void StartWMNodeSkillMenuTransition(struct MenuProc *menuProc);
extern u8 WMMenu_OnSkillShopSelected(struct MenuProc * menuProc, struct MenuItemProc * menuItemProc);
extern bool WorldMapSkillShop_HasNodeShop(u8 nodeId);

typedef struct {
    u8 mapNodeId;
    u8 chapterId;
} EnterTownNode;

typedef struct {
    u8 chapterId;
    u16 textId;
} WorldMapThoughtBubbleEntry;

static const EnterTownNode EnterTownNodes[] = {
    { NODE_SERAFEW,      0x50 },
    // { NODE_ADLAS_PLAINS, CHAPTER_3C },
};

u8 WMMenu_IsDistrictAvailable(const struct MenuItemDef * def, int number)
{

    if (gpKernelDesignerConfig->base_chapters == false)
        return MENU_NOTSHOWN;

    u8 location = *(volatile u8*)0x03005291;

    // Loop through array entries
    for (unsigned i = 0; i < ARRAY_COUNT(EnterTownNodes); i++)
    {
        if (EnterTownNodes[i].mapNodeId == location)
            return MENU_ENABLED;
    }

    return MENU_NOTSHOWN;
}

u8 WMMenu_OnDistrictSelected(struct MenuProc * menuProc, struct MenuItemProc * menuItemProc)
{
    SetFlag(GLOBAL_FLAG_BASE_CHAPTER_INTRO_SKIP); // Set this to skip intro chapter GFX

    // Matches: *(u8*)0x03005266 = 0x36 not sure what it's for
    *(volatile u8*)0x03005266 = 0x36;

    // Map index we want to load from
    *(volatile u8*)0x03005268 = 0x3B;

    void * p = Proc_Find(ProcScr_WorldMapMain);

    // Perform state jump to label 0x0E in that proc
    Proc_Goto(p, 0x0E);

    return MENU_ACT_SKIPCURSOR | MENU_ACT_END | MENU_ACT_SND6A | MENU_ACT_CLEAR;
}

/* JESTER - This originally had an int type so I made a new function which is void to make it compile */
//! FE8U = 0x080BC634
static void WMNodeMenu_OnInit_VOID(struct MenuProc * menu)
{
    BG_EnableSyncByMask(BG0_SYNC_BIT);
}

/* JESTER - This originally had an int type so I made a new function which is void to make it compile */
//! FE8U = 0x080BC644
static void WMNodeMenu_OnEnd_VOID(struct MenuProc * menu)
{
    EndAllProcChildren(menu);
    ClearBg0Bg1();
}

//! FE8U = 0x080BC674
LYN_REPLACE_CHECK(WMMenu_IsArmoryAvailable);
u8 WMMenu_IsArmoryAvailable(const struct MenuItemDef * def, int number)
{
    if (gGMData.nodes[gGMData.units[0].location].state & 2)
    {
        return MENU_NOTSHOWN;
    }

    if ((gGMData.units[0].location[gWMNodeData].armory[0]) == 0)
    {
        return MENU_NOTSHOWN;
    }

    return MENU_ENABLED;
}

//! FE8U = 0x080BC6AC
LYN_REPLACE_CHECK(WMMenu_IsVendorAvailable);
u8 WMMenu_IsVendorAvailable(const struct MenuItemDef * def, int number)
{
    if (gGMData.nodes[gGMData.units[0].location].state & 2)
    {
        return MENU_NOTSHOWN;
    }

    if ((gGMData.units[0].location[gWMNodeData].vendor[0]) == 0)
    {
        return MENU_NOTSHOWN;
    }

    return MENU_ENABLED;
}

//! FE8U = 0x080BC6E4
LYN_REPLACE_CHECK(WMMenu_IsSecretShopAvailable);
u8 WMMenu_IsSecretShopAvailable(const struct MenuItemDef * def, int number)
{
    if (gGMData.nodes[gGMData.units[0].location].state & 2)
    {
        return MENU_NOTSHOWN;
    }

    if ((gGMData.units[0].location[gWMNodeData].secretShop[0]) == 0)
    {
        return MENU_NOTSHOWN;
    }

    if (!(gPlaySt.chapterStateBits & PLAY_FLAG_POSTGAME))
    {
        return MENU_NOTSHOWN;
    }

    return MENU_ENABLED;
}

//! FE8U = 0x080BC77C
LYN_REPLACE_CHECK(WMMenu_OnArmorySelected);
u8 WMMenu_OnArmorySelected(struct MenuProc * menuProc, struct MenuItemProc * menuItemProc)
{
    gGMData.unk_cd = menuProc->itemCurrent;
    Proc_Goto(GM_MAIN, 19);
    return MENU_ACT_SKIPCURSOR | MENU_ACT_END | MENU_ACT_SND6A | MENU_ACT_CLEAR;
}

//! FE8U = 0x080BC7A4
LYN_REPLACE_CHECK(WMMenu_OnVendorSelected);
u8 WMMenu_OnVendorSelected(struct MenuProc * menuProc, struct MenuItemProc * menuItemProc)
{
    gGMData.unk_cd = menuProc->itemCurrent;
    Proc_Goto(GM_MAIN, 20);
    return MENU_ACT_SKIPCURSOR | MENU_ACT_END | MENU_ACT_SND6A | MENU_ACT_CLEAR;
}

//! FE8U = 0x080BC7CC
LYN_REPLACE_CHECK(WMMenu_OnSecretShopSelected);
u8 WMMenu_OnSecretShopSelected(struct MenuProc * menuProc, struct MenuItemProc * menuItemProc)
{
    gGMData.unk_cd = menuProc->itemCurrent;
    Proc_Goto(GM_MAIN, 21);
    return MENU_ACT_SKIPCURSOR | MENU_ACT_END | MENU_ACT_SND6A | MENU_ACT_CLEAR;
}

//! FE8U = 0x080BC7F4
LYN_REPLACE_CHECK(WMMenu_OnManageItemsSelected);
u8 WMMenu_OnManageItemsSelected(struct MenuProc * menuProc, struct MenuItemProc * menuItemProc)
{
    gGMData.unk_cd = menuProc->itemCurrent;
    Proc_Goto(GM_MAIN, 22);
    return MENU_ACT_SKIPCURSOR | MENU_ACT_END | MENU_ACT_SND6A | MENU_ACT_CLEAR;
}

u8 WMMenu_IsSkillShopAvailable(const struct MenuItemDef * def, int number)
{
    if (gpKernelDesignerConfig->skill_shop == false)
        return MENU_NOTSHOWN;

    if (gGMData.nodes[gGMData.units[0].location].state & 2)
        return MENU_NOTSHOWN;

    if (!WorldMapSkillShop_HasNodeShop(gGMData.units[0].location))
        return MENU_NOTSHOWN;

    return MENU_ENABLED;
}

u8 WMMenu_OnManageSkillsSelected(struct MenuProc * menuProc, struct MenuItemProc * menuItemProc)
{
    gGMData.unk_cd = menuProc->itemCurrent;
    StartWMNodeSkillMenuTransition(menuProc);
    return MENU_ACT_SKIPCURSOR | MENU_ACT_END | MENU_ACT_SND6A | MENU_ACT_CLEAR;
}

static struct MenuItemDef const MenuItemDef_WMNodeMenu_NEW[] =
{
    {
        .name = "　アイテム整理",
        .nameMsgId = MSG_Enter_District_NAME, // TODO: msgid " Enter District "
        .helpMsgId = MSG_Enter_District_DESC,
        .overrideId = 0,
        .isAvailable = WMMenu_IsDistrictAvailable,
        .onSelected = WMMenu_OnDistrictSelected,
    },
    {
        .name = "　武器屋に入る",
        .nameMsgId = 0x066E, // TODO: msgid " Enter Armory[.]"
        .helpMsgId = 0x06CF,
        .overrideId = 1,
        .isAvailable = WMMenu_IsArmoryAvailable,
        .onSelected = WMMenu_OnArmorySelected,
    },

    {
        .name = "　道具屋に入る",
        .nameMsgId = 0x066F, // TODO: msgid " Enter Shop[.]"
        .helpMsgId = 0x06D0,
        .overrideId = 2,
        .isAvailable = WMMenu_IsVendorAvailable,
        .onSelected = WMMenu_OnVendorSelected,
    },

    {
        .name = "　秘密店に入る",
        .nameMsgId = 0x0670, // TODO: msgid " Enter ? Shop[.]"
        .helpMsgId = 0x06D1,
        .overrideId = 3,
        .isAvailable = WMMenu_IsSecretShopAvailable,
        .onSelected = WMMenu_OnSecretShopSelected,
    },

    {
        .name = " Skill Shop",
        .nameMsgId = MSG_WM_SKILL_SHOP_NAME,
        .helpMsgId = MSG_WM_SKILL_SHOP_DESC,
        .overrideId = 4,
        .isAvailable = WMMenu_IsSkillShopAvailable,
        .onSelected = WMMenu_OnSkillShopSelected,
    },

    {
        .name = "　特技変更",
        .nameMsgId = MSG_WM_MANAGE_SKILLS_NAME,
        .helpMsgId = MSG_WM_MANAGE_SKILLS_DESC,
        .overrideId = 5,
        .isAvailable = MenuAlwaysEnabled,
        .onSelected = WMMenu_OnManageSkillsSelected,
    },

    {
        .name = "　アイテム整理",
        .nameMsgId = 0x0671, // TODO: msgid " Manage Items[.]"
        .helpMsgId = 0x0678,
        .overrideId = 6,
        .isAvailable = MenuAlwaysEnabled,
        .onSelected = WMMenu_OnManageItemsSelected,
    },

    { 0 }, // end
};

static struct MenuDef const gMenu_WMNodeMenu_NEW =
{
    .rect = { 20, 10, 8, 0 },
    .menuItems = MenuItemDef_WMNodeMenu_NEW,
    .onInit = WMNodeMenu_OnInit_VOID,
    .onEnd = WMNodeMenu_OnEnd_VOID,
    .onBPress = WMNodeMenu_OnCancel,
    .onRPress = MenuAutoHelpBoxSelect,
    .onHelpBox = MenuStdHelpBox,
};

static void WMGeneralMenu_OnInit_VOID(struct MenuProc * menu)
{
    BG_EnableSyncByMask(BG0_SYNC_BIT);
}

static void WMGeneralMenu_OnEnd_VOID(struct MenuProc * menu)
{
    ClearBg0Bg1();
}

static u8 WMGeneralMenu_OnCancel_NEW(struct MenuProc * menuProc, struct MenuItemProc * menuItemProc)
{
    Proc_Goto(GM_MAIN, 3);
    return MENU_ACT_SKIPCURSOR | MENU_ACT_END | MENU_ACT_SND6B | MENU_ACT_CLEAR;
}

static u8 WMMenu_OnUnitSelected_NEW(struct MenuProc * menuProc, struct MenuItemProc * menuItemProc)
{
    Proc_Goto(GM_MAIN, 9);
    return MENU_ACT_SKIPCURSOR | MENU_ACT_END | MENU_ACT_SND6A | MENU_ACT_CLEAR;
}

static u8 WMMenu_OnStatusSelected_NEW(struct MenuProc * menuProc, struct MenuItemProc * menuItemProc)
{
    Proc_Goto(GM_MAIN, 12);
    return MENU_ACT_SKIPCURSOR | MENU_ACT_END | MENU_ACT_SND6A | MENU_ACT_CLEAR;
}

static int WMMenu_OnGuideDraw_NEW(struct MenuProc * menuProc, struct MenuItemProc * menuItemProc)
{
    if (!(menuProc->state & MENU_STATE_NOTSHOWN))
    {
        if (!BmGuideTextShowGreenOrNormal())
            Text_SetColor(&menuItemProc->text, TEXT_COLOR_SYSTEM_GREEN);

        if (menuItemProc->availability == MENU_DISABLED)
            Text_SetColor(&menuItemProc->text, TEXT_COLOR_SYSTEM_GRAY);

        Text_DrawString(&menuItemProc->text, GetStringFromIndex(menuItemProc->def->nameMsgId));
        PutText(&menuItemProc->text,
            BG_GetMapBuffer(menuProc->frontBg) + TILEMAP_INDEX(menuItemProc->xTile, menuItemProc->yTile));
    }

    return 0;
}

static u8 WMMenu_OnGuideSelected_NEW(struct MenuProc * menuProc, struct MenuItemProc * menuItemProc)
{
    Proc_Goto(GM_MAIN, 10);
    return MENU_ACT_SKIPCURSOR | MENU_ACT_END | MENU_ACT_SND6A | MENU_ACT_CLEAR;
}

static u8 WMMenu_OnOptionsSelected_NEW(struct MenuProc * menuProc, struct MenuItemProc * menuItemProc)
{
    Proc_Goto(GM_MAIN, 11);
    return MENU_ACT_SKIPCURSOR | MENU_ACT_END | MENU_ACT_SND6A | MENU_ACT_CLEAR;
}

static u8 WMMenu_OnSaveSelected_NEW(struct MenuProc * menuProc, struct MenuItemProc * menuItemProc)
{
    Proc_Goto(GM_MAIN, 13);
    return MENU_ACT_SKIPCURSOR | MENU_ACT_END | MENU_ACT_SND6A | MENU_ACT_CLEAR;
}

static u8 WMMenu_IsHomeAvailable_NEW(const struct MenuItemDef * def, int number)
{
    if (gpKernelDesignerConfig->quality_of_life_fixes == true)
        return MENU_ENABLED;

    return MENU_NOTSHOWN;
}

static int WMMenu_OnHomeDraw_NEW(struct MenuProc * menuProc, struct MenuItemProc * menuItemProc)
{
    if (!(menuProc->state & MENU_STATE_NOTSHOWN))
    {
        if (menuItemProc->availability == MENU_DISABLED)
            Text_SetColor(&menuItemProc->text, TEXT_COLOR_SYSTEM_GRAY);

        Text_DrawString(&menuItemProc->text, GetStringFromIndex(menuItemProc->def->nameMsgId));
        PutText(&menuItemProc->text,
            BG_GetMapBuffer(menuProc->frontBg) + TILEMAP_INDEX(menuItemProc->xTile, menuItemProc->yTile));
    }

    return 0;
}

static u8 WMMenu_OnHomeSelected_NEW(struct MenuProc * menuProc, struct MenuItemProc * menuItemProc)
{
    struct Proc * gameCtrl = Proc_Find(gProcScr_GameControl);
    ProcPtr wmProc = Proc_Find(ProcScr_WorldMapMain);
    ProcPtr wmWrapperProc = Proc_Find(ProcScr_WorldMapWrapper);

    StartFastFadeToBlack();

    if (wmProc != NULL)
        EndWM(NULL);

    if (wmWrapperProc != NULL)
        Proc_End(wmWrapperProc);

    if (gameCtrl != NULL)
        Proc_Goto(gameCtrl, LGAMECTRL_TITLE_DIRECT);
    else
        StartTitleScreen_FlagFalse(NULL);

    return MENU_ACT_SKIPCURSOR | MENU_ACT_END | MENU_ACT_SND6A | MENU_ACT_CLEAR;
}

u8 WMMenu_IsSuspendAvailable(const struct MenuItemDef * def, int number)
{
    if (gPlaySt.chapterStateBits & PLAY_FLAG_TUTORIAL)
        return MENU_DISABLED;

    if (gBmSt.gameStateBits & BM_FLAG_LINKARENA)
        return MENU_DISABLED;

    return MENU_ENABLED;
}

u8 WMMenu_OnSuspendSelected(struct MenuProc * menuProc, struct MenuItemProc * menuItemProc)
{
    if (menuItemProc->availability == MENU_DISABLED)
    {
        MenuFrozenHelpBox(menuProc, 0x864);
        return MENU_ACT_SND6B;
    }

    sub_80B5D3C();
    return MENU_ACT_SKIPCURSOR | MENU_ACT_END | MENU_ACT_SND6A | MENU_ACT_CLEAR;
}

static struct MenuItemDef const MenuItemDef_WMGeneralMenu_NEW[] =
{
    {
        .name = "　部隊",
        .nameMsgId = 0x0645,
        .helpMsgId = 0x06DF,
        .overrideId = 0,
        .isAvailable = MenuAlwaysEnabled,
        .onSelected = WMMenu_OnUnitSelected_NEW,
    },
    {
        .name = "　状況",
        .nameMsgId = 0x0646,
        .helpMsgId = 0x06E0,
        .overrideId = 1,
        .isAvailable = MenuAlwaysEnabled,
        .onSelected = WMMenu_OnStatusSelected_NEW,
    },
    {
        .name = "　辞書",
        .nameMsgId = 0x0647,
        .helpMsgId = 0x06E5,
        .overrideId = 2,
        .isAvailable = MapMenu_IsGuideCommandAvailable,
        .onDraw = WMMenu_OnGuideDraw_NEW,
        .onSelected = WMMenu_OnGuideSelected_NEW,
    },
    {
        .name = "　設定",
        .nameMsgId = 0x0648,
        .helpMsgId = 0x06E1,
        .overrideId = 3,
        .isAvailable = MenuAlwaysEnabled,
        .onSelected = WMMenu_OnOptionsSelected_NEW,
    },
    {
        .name = "　記録",
        .nameMsgId = 0x0649,
        .helpMsgId = 0x0679,
        .overrideId = 4,
        .isAvailable = MenuAlwaysEnabled,
        .onSelected = WMMenu_OnSaveSelected_NEW,
    },
    {
        .name = " Home",
        .nameMsgId = MSG_WM_HOME_NAME,
        .helpMsgId = MSG_WM_HOME_DESC,
        .overrideId = 5,
        .isAvailable = WMMenu_IsHomeAvailable_NEW,
        .onDraw = WMMenu_OnHomeDraw_NEW,
        .onSelected = WMMenu_OnHomeSelected_NEW,
    },
    { 0 },
};

static struct MenuDef const gMenu_WMGeneralMenu_NEW =
{
    .rect = { 1, 1, 6, 0 },
    .menuItems = MenuItemDef_WMGeneralMenu_NEW,
    .onInit = WMGeneralMenu_OnInit_VOID,
    .onEnd = WMGeneralMenu_OnEnd_VOID,
    .onBPress = WMGeneralMenu_OnCancel_NEW,
    .onRPress = MenuAutoHelpBoxSelect,
    .onHelpBox = MenuStdHelpBox,
};

LYN_REPLACE_CHECK(StartWMGeneralMenu);
struct MenuProc * StartWMGeneralMenu(ProcPtr parent)
{
    gGMData.sprite_disp = 0;
    InitTextFont(&gFont_0201AFC0, (void *)0x06001000, 0x80, 0);
    return StartMenuAt(&gMenu_WMGeneralMenu_NEW, gMenuRect_WMGeneralMenuRect, parent);
}

LYN_REPLACE_CHECK(StartWMNodeMenu);
struct MenuProc * StartWMNodeMenu(struct WorldMapMainProc * parent)
{
    struct MenuProc * menuProc;

    gGMData.sprite_disp = 0;

    InitTextFont(&gFont_0201AFC0, (void *)0x06001000, 0x80, 0);

    if ((gGMData.ix >> 8) - gGMData.xCamera < 152)
    {
        menuProc = StartMenuAt(&gMenu_WMNodeMenu_NEW, gMenuRect_WMNodeMenuRectA, parent);
    }
    else
    {
        menuProc = StartMenuAt(&gMenu_WMNodeMenu_NEW, gMenuRect_WMNodeMenuRectB, parent);
    }

    if (gGMData.unk_cd < menuProc->itemCount)
    {
        menuProc->itemCurrent = gGMData.unk_cd;
    }
    else
    {
        menuProc->itemCurrent = menuProc->itemCount - 1;
    }

    StartWorldMapThoughtBubble(menuProc);

    return menuProc;
}