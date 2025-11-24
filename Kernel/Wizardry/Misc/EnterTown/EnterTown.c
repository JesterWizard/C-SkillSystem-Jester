#include "common-chax.h"
#include "constants/texts.h"
#include "worldmap.h"
#include "jester_headers/custom-structs.h"

const struct {
    u8 mapNodeId;
    u8 chapterId;
} EnterTownNodes[] = {
    {0x05, 0x3B},
    {0x06, 0x3C},
};

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

u8 WMMenu_IsDistrictAvailable(const struct MenuItemDef * def, int number)
{
    u8 location = *(volatile u8*)0x03005291;

    // Loop through array entries
    for (unsigned i = 0; i < sizeof(EnterTownNodes)/sizeof(EnterTownNodes[0]); i++)
    {
        if (EnterTownNodes[i].mapNodeId == location)
            return MENU_ENABLED;
    }

    return MENU_NOTSHOWN;
}

u8 WMMenu_OnDistrictSelected(struct MenuProc * menuProc, struct MenuItemProc * menuItemProc)
{
    SetFlag(GLOBAL_FLAG_BASE_CHAPTER_INTRO_SKIP); // Set this to skip intro chapter GFX

    // Matches: *(u8*)0x03005266 = 0x36
    *(volatile u8*)0x03005266 = 0x36;

    // Map index we want to load from
    *(volatile u8*)0x03005268 = 0x3B;

    // Find proc at script address 08A3D748
    void * p = Proc_Find(ProcScr_WorldMapMain);

    // Perform state jump to label 0x0E in that proc
    Proc_Goto(p, 0x0E);

    // 0x17 = cursor skip + end menu + sound 6A + clear menu
    return MENU_ACT_SKIPCURSOR | MENU_ACT_END | MENU_ACT_SND6A | MENU_ACT_CLEAR;
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

static struct MenuItemDef const MenuItemDef_WMNodeMenu_NEW[] =
{
#ifdef CONFIG_ENTER_DISTRICT
    {
        .name = "　アイテム整理",
        .nameMsgId = MSG_Enter_District_NAME, // TODO: msgid " Enter District "
        .helpMsgId = MSG_Enter_District_DESC,
        .overrideId = 0,
        .isAvailable = WMMenu_IsDistrictAvailable,
        .onSelected = WMMenu_OnDistrictSelected,
    },
#endif
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
        .name = "　アイテム整理",
        .nameMsgId = 0x0671, // TODO: msgid " Manage Items[.]"
        .helpMsgId = 0x0678,
        .overrideId = 4,
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

    return menuProc;
}

LYN_REPLACE_CHECK(GetROMChapterStruct);
const struct ROMChapterData* GetROMChapterStruct(unsigned chIndex) {
    if (chIndex == 0x7F)
        return gExtraMapInfo->chapter_info;

    return gChapterDataTable + chIndex;
}