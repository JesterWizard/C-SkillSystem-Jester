#include "common-chax.h"
#include "constants/texts.h"
#include "worldmap.h"

const struct {
    u8 mapNodeId;
    u8 chapterId;
} EnterTownNodes[] = {
    {0x01, 0x3B},
    {0x02, 0x3C},
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

// //! FE8U = 0x080BD068
// LYN_REPLACE_CHECK(GetBattleMapKind);
// u32 GetBattleMapKind(void)
// {
//     int i;
//     u32 chapterId = gPlaySt.chapterIndex;

//     switch (chapterId)
//     {
//         case CHAPTER_L_2:
//         case CHAPTER_L_3:
//         case CHAPTER_L_4:
//         case CHAPTER_L_5:
//         case CHAPTER_L_6:
//         case CHAPTER_L_7:
//         case CHAPTER_L_8:
//         case CHAPTER_E_9:
//         case CHAPTER_E_10:
//         case CHAPTER_E_13:
//         case CHAPTER_E_14:
//         case CHAPTER_E_15:
//         case CHAPTER_E_16:
//         case CHAPTER_E_17:
//         case CHAPTER_E_18:
//         case CHAPTER_E_19:
//         case CHAPTER_E_20:
//         case CHAPTER_I_9:
//         case CHAPTER_I_10:
//         case CHAPTER_I_13:
//         case CHAPTER_I_14:
//         case CHAPTER_I_15:
//         case CHAPTER_I_16:
//         case CHAPTER_I_17:
//         case CHAPTER_I_18:
//         case CHAPTER_I_19:
//         case CHAPTER_I_20:
//         case CHAPTER_T_01:
//         case CHAPTER_T_02:
//         case CHAPTER_T_03:
//         case CHAPTER_T_04:
//         case CHAPTER_T_05:
//         case CHAPTER_T_06:
//         case CHAPTER_T_07:
//         case CHAPTER_T_08:
//         case CHAPTER_2C:
//         case CHAPTER_2D:
//         case CHAPTER_R_01:
//         case CHAPTER_R_02:
//         case CHAPTER_R_03:
//         case CHAPTER_R_04:
//         case CHAPTER_R_05:
//         case CHAPTER_R_06:
//         case CHAPTER_R_07:
//         case CHAPTER_R_08:
//         case CHAPTER_R_09:
//         case CHAPTER_R_10:
//         case CHAPTER_MALKAEN_COAST:
//         case CHAPTER_3A:
//         case CHAPTER_E_11:
//         case CHAPTER_I_11:
//         default:
//             if (chapterId - CHAPTER_T_02 < 9)
//             {
//                 chapterId = CHAPTER_T_01;
//             }
//             else if (chapterId - CHAPTER_R_02 < 9)
//             {
//                 chapterId = CHAPTER_R_01;
//             }

//             for (i = 0; i < NODE_MAX; i++)
//             {
//                 if (chapterId == (u32)WMLoc_GetChapterId(i))
//                 {
//                     if (!(gGMData.nodes[i].state & GM_NODE_STATE_CLEARED))
//                     {
//                         if ((u8)i[gWMNodeData].encounters != 3)
//                         {
//                             break;
//                         }
//                     }
//                     else if (i[gWMNodeData].placementFlag != GMAP_NODE_PLACEMENT_DUNGEON)
//                     {
//                         return BATTLEMAP_KIND_STORY;
//                     }

//                     return BATTLEMAP_KIND_DUNGEON;
//                 }
//             }

//             break;

//         case CHAPTER_L_PROLOGUE:
//         case CHAPTER_L_1:
//         case CHAPTER_L_5X:
//         case CHAPTER_E_12:
//         case CHAPTER_E_21:
//         case CHAPTER_E_21X:
//         case CHAPTER_I_12:
//         case CHAPTER_I_21:
//         case CHAPTER_I_21X:
//         case CHAPTER_CASTLE_FRELIA:
//         case CHAPTER_3B:
//         case CHAPTER_3C:
//         case CHAPTER_3F:
//         case CHAPTER_40:
//         case CHAPTER_41:
//         case CHAPTER_42:
//         case CHAPTER_43:
//         case CHAPTER_44:
//         case CHAPTER_45:
//         case CHAPTER_46:
//         case CHAPTER_47:
//             return BATTLEMAP_KIND_STORY;
//     }

//     return BATTLEMAP_KIND_SKIRMISH;
// }