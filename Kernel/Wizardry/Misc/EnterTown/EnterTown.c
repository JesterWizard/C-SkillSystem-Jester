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

enum {
    WM_SKILL_SHOP_ITEM_COUNT = 7,
    WM_SKILL_SHOP_VISIBLE_COUNT = 5,
    WM_SKILL_SHOP_TEXT_COUNT = 2 + WM_SKILL_SHOP_VISIBLE_COUNT,
};

struct WorldMapSkillShopProc {
    PROC_HEADER;

    u8 unitId;
    u8 savedX;
    u8 savedY;
    u8 cursor;
    u8 listTop;
};

static const u16 sWorldMapSkillShopSkillIds[WM_SKILL_SHOP_ITEM_COUNT] = {
    1, 2, 3, 4, 5,
    6, 7
};

static const u8 sWorldMapSkillShopSkillCosts[WM_SKILL_SHOP_ITEM_COUNT] = {
    5, 10, 15, 20, 25,
    30, 35
};

static struct Unit *WorldMapSkillShop_GetUnit(struct WorldMapSkillShopProc *proc)
{
    return GetUnitFromCharId(proc->unitId);
}

static u8 WorldMapSkillShop_GetUnitSkillPoints(struct Unit *unit)
{
    struct NewBwl *bwl;

    if (!UNIT_IS_VALID(unit))
        return 0;

    if (!CheckHasBwl(UNIT_CHAR_ID(unit)))
        return 0;

    bwl = GetNewBwl(UNIT_CHAR_ID(unit));
    if (!bwl)
        return 0;

    return bwl->skillPoints;
}

static void WorldMapSkillShop_CloseHelp(void)
{
    if (Proc_Find(gProcScr_HelpBox) != NULL)
        CloseHelpBox();
}

static void WorldMapSkillShop_ClampCursor(struct WorldMapSkillShopProc *proc)
{
    if (proc->cursor >= WM_SKILL_SHOP_ITEM_COUNT)
        proc->cursor = WM_SKILL_SHOP_ITEM_COUNT - 1;

    if (proc->cursor < proc->listTop)
        proc->listTop = proc->cursor;

    if (proc->cursor >= proc->listTop + WM_SKILL_SHOP_VISIBLE_COUNT)
        proc->listTop = proc->cursor - (WM_SKILL_SHOP_VISIBLE_COUNT - 1);
}

static void WorldMapSkillShop_Draw(struct WorldMapSkillShopProc *proc)
{
    int i;
    struct Unit *unit = WorldMapSkillShop_GetUnit(proc);
    u8 skillPoints = WorldMapSkillShop_GetUnitSkillPoints(unit);

    BG_Fill(gBG0TilemapBuffer, 0);

    for (i = 0; i < WM_SKILL_SHOP_TEXT_COUNT; ++i)
        ClearText(&gPrepUnitTexts[i]);

    StartUiGoldBox(proc);

    UpdateMenuScrollBarConfig(
        8,
		proc->listTop * 16,
        WM_SKILL_SHOP_ITEM_COUNT,
        WM_SKILL_SHOP_VISIBLE_COUNT
    );

    DrawUiFrame2(3, 8, 23, 12, 0);
    PutDrawText(&gPrepUnitTexts[1], TILEMAP_LOCATED(gBG0TilemapBuffer, 23, 6), TEXT_COLOR_SYSTEM_BLUE, 0, 0, "SP:");
    PutNumber(TILEMAP_LOCATED(gBG0TilemapBuffer, 27, 6), TEXT_COLOR_SYSTEM_BLUE, skillPoints);

    for (i = 0; i < WM_SKILL_SHOP_VISIBLE_COUNT; ++i) {
        int skillIndex = proc->listTop + i;
        int y = 9 + (i * 2);
        u16 sid;
        u8 cost;
        int textColor = TEXT_COLOR_SYSTEM_WHITE;

        if (skillIndex >= WM_SKILL_SHOP_ITEM_COUNT)
            continue;

        sid = sWorldMapSkillShopSkillIds[skillIndex];
        cost = sWorldMapSkillShopSkillCosts[skillIndex];

        if (unit && SkillTester(unit, sid))
            textColor = TEXT_COLOR_SYSTEM_GRAY;
        else if (skillPoints < cost)
            textColor = TEXT_COLOR_SYSTEM_GRAY;

        DrawIcon(
            TILEMAP_LOCATED(gBG0TilemapBuffer, 4, y),
            SKILL_ICON(sid),
            TILEREF(0, STATSCREEN_BGPAL_ITEMICONS + GetSkillIconPal(sid)));

        PutDrawText(
            &gPrepUnitTexts[2 + i],
            TILEMAP_LOCATED(gBG0TilemapBuffer, 7, y),
            textColor,
            0,
            14,
            GetSkillNameStr(sid));

        PutNumber(TILEMAP_LOCATED(gBG0TilemapBuffer, 22, y), TEXT_COLOR_SYSTEM_BLUE, cost);
    }

    ShowSysHandCursor(28, 72 + ((proc->cursor - proc->listTop) * 16), 0x0, 0x800);
    BG_EnableSyncByMask(BG0_SYNC_BIT);
}

static void WorldMapSkillShop_OpenHelp(struct WorldMapSkillShopProc *proc)
{
    u16 sid = sWorldMapSkillShopSkillIds[proc->cursor];
    int row = proc->cursor - proc->listTop;

    StartHelpBox(2 * 8, (9 + (row * 2)) * 8, GetSkillDescMsg(sid));
}

static bool WorldMapSkillShop_TryPurchase(struct WorldMapSkillShopProc *proc)
{
    struct Unit *unit = WorldMapSkillShop_GetUnit(proc);
    struct NewBwl *bwl;
    u16 sid = sWorldMapSkillShopSkillIds[proc->cursor];
    u8 cost = sWorldMapSkillShopSkillCosts[proc->cursor];

    if (!UNIT_IS_VALID(unit))
        return false;

    if (!CheckHasBwl(UNIT_CHAR_ID(unit)))
        return false;

    bwl = GetNewBwl(UNIT_CHAR_ID(unit));
    if (!bwl)
        return false;

    if (SkillTester(unit, sid))
        return false;

    if (bwl->skillPoints < cost)
        return false;

    if (AddSkill(unit, sid) != 0)
        return false;

    bwl->skillPoints -= cost;
    SetPopupUnit(unit);
    SetPopupItem(sid);
    NewPopup_Simple(PopupScr_LearnSkill, SONG_SE_UPDATE, 0x00, proc);

    return true;
}

static void WorldMapSkillShop_Init(struct WorldMapSkillShopProc *proc)
{
    proc->unitId = gGMData.units[0].id;
    proc->savedX = gGMData.xCamera;
    proc->savedY = gGMData.yCamera;
    proc->cursor = 0;
    proc->listTop = 0;

    StartBgm(SONG_SHOPS, 0);

    HideGmUnit(0);
    SetDispEnable(1, 1, 1, 1, 1);
    gLCDControlBuffer.dispcnt.mode = 0;

	CpuFastFill16(0, (void *)0x06011000, 0xFE0);
    CpuFastFill16(0, (void *)0x06012800, 0x5E0);

    SetupBackgrounds(NULL);
    BG_Fill(gBG0TilemapBuffer, 0);
    BG_Fill(gBG1TilemapBuffer, 0);
    BG_Fill(gBG2TilemapBuffer, 0);
    BG_Fill(gBG3TilemapBuffer, 0);

    gLCDControlBuffer.bg0cnt.priority = 0;
    gLCDControlBuffer.bg1cnt.priority = 2;
    gLCDControlBuffer.bg2cnt.priority = 1;
    gLCDControlBuffer.bg3cnt.priority = 3;

    ResetFaces();
    ResetText();
    ResetUnitSprites();
    ResetIconGraphics_();
    LoadUiFrameGraphics();
    LoadObjUIGfx();
    LoadHelpBoxGfx((void *)0x06012000, -1);
    LoadIconPalettes(4);

    StartUiCursorHand(proc);
    ResetSysHandCursor(proc);
    DisplaySysHandCursorTextShadow(0x600, false);

    StartMenuScrollBar(proc);
    PutMenuScrollBarAt(2 * 8, 76);
    InitMenuScrollBarImg(0x7A60, 0);

    ApplyPalette(Pal_CommGameBgScreenInShop, BGPAL_SHOP_MAINBG);
    Decompress(Img_CommGameBgScreen, (void *)BG_VRAM + GetBackgroundTileDataOffset(BG_3));
    CallARM_FillTileRect(gBG3TilemapBuffer, Tsa_CommGameBgScreenInShop, OBJ_PALETTE(BGPAL_SHOP_MAINBG));

    for (int i = 0; i < WM_SKILL_SHOP_TEXT_COUNT; ++i)
        InitText(&gPrepUnitTexts[i], 16);

    WorldMapSkillShop_Draw(proc);
}

static void WorldMapSkillShop_Loop(struct WorldMapSkillShopProc *proc)
{
    if (gKeyStatusPtr->newKeys & B_BUTTON) {
        WorldMapSkillShop_CloseHelp();
        Proc_Break(proc);
        PlaySoundEffect(SONG_SE_SYS_WINDOW_CANSEL1);
        return;
    }

    if (gKeyStatusPtr->newKeys & DPAD_UP) {
        if (proc->cursor > 0) {
            proc->cursor--;
            WorldMapSkillShop_ClampCursor(proc);
            WorldMapSkillShop_CloseHelp();
            WorldMapSkillShop_Draw(proc);
            PlaySoundEffect(SONG_SE_SYS_CURSOR_UD1);
        }
    }

    if (gKeyStatusPtr->newKeys & DPAD_DOWN) {
        if (proc->cursor + 1 < WM_SKILL_SHOP_ITEM_COUNT) {
            proc->cursor++;
            WorldMapSkillShop_ClampCursor(proc);
            WorldMapSkillShop_CloseHelp();
            WorldMapSkillShop_Draw(proc);
            PlaySoundEffect(SONG_SE_SYS_CURSOR_UD1);
        }
    }

    if (gKeyStatusPtr->newKeys & R_BUTTON) {
        WorldMapSkillShop_CloseHelp();
        WorldMapSkillShop_OpenHelp(proc);
    }

    if (gKeyStatusPtr->newKeys & A_BUTTON) {
        WorldMapSkillShop_CloseHelp();

        if (WorldMapSkillShop_TryPurchase(proc)) {
            WorldMapSkillShop_Draw(proc);
            PlaySoundEffect(SONG_SE_SYS_WINDOW_SELECT1);
        } else {
            PlaySoundEffect(SONG_SE_SYS_WINDOW_CANSEL1);
        }
    }
}

static void WorldMapSkillShop_OnEnd(struct WorldMapSkillShopProc *proc)
{
    ProcPtr wmProc;

    Proc_EndEach(ProcScr_SlidingWallBg);
    WorldMapSkillShop_CloseHelp();
    EndAllProcChildren(proc);

    WorldMap_Init(GM_MAIN);
    gGMData.units[0].id = proc->unitId;
    gGMData.sprite_disp = 1;
    gGMData.xCamera = proc->savedX;
    gGMData.yCamera = proc->savedY;

    ClearBg0Bg1();
    SetDefaultColorEffects();

    wmProc = Proc_Find(ProcScr_WorldMapMain);
    if (wmProc != NULL)
        NewFadeIn(0x10, wmProc);
}

static const struct ProcCmd ProcScr_WMNodeSkillShop[] = {
    PROC_NAME("WMNodeSkillShop"),
    PROC_YIELD,
    PROC_SET_END_CB(WorldMapSkillShop_OnEnd),
    PROC_CALL_ARG(NewFadeOut, 0x10),
    PROC_WHILE(FadeOutExists),
    PROC_CALL(WorldMapSkillShop_Init),
    PROC_CALL_ARG(NewFadeIn, 0x10),
    PROC_WHILE(FadeInExists),
    PROC_REPEAT(WorldMapSkillShop_Loop),
    PROC_CALL_ARG(NewFadeOut, 0x10),
    PROC_WHILE(FadeOutExists),
    PROC_END,
};

static void StartWMNodeSkillShop(struct MenuProc *menuProc)
{
    Proc_StartBlocking(ProcScr_WMNodeSkillShop, Proc_Find(ProcScr_WorldMapMain));
}

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

u8 WMMenu_OnManageSkillsSelected(struct MenuProc * menuProc, struct MenuItemProc * menuItemProc)
{
    gGMData.unk_cd = menuProc->itemCurrent;
    StartWMNodeSkillMenuTransition(menuProc);
    return MENU_ACT_SKIPCURSOR | MENU_ACT_END | MENU_ACT_SND6A | MENU_ACT_CLEAR;
}

u8 WMMenu_OnSkillShopSelected(struct MenuProc * menuProc, struct MenuItemProc * menuItemProc)
{
	gGMData.xCamera = 0;
	gGMData.yCamera = 0;
    gGMData.unk_cd = menuProc->itemCurrent;
    StartWMNodeSkillShop(menuProc);
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
        .name = "　特技変更",
        .nameMsgId = MSG_WM_MANAGE_SKILLS_NAME,
        .helpMsgId = MSG_WM_MANAGE_SKILLS_DESC,
        .overrideId = 4,
        .isAvailable = MenuAlwaysEnabled,
        .onSelected = WMMenu_OnManageSkillsSelected,
    },

    {
        .name = " Skill Shop",
        .nameMsgId = MSG_WM_SKILL_SHOP_NAME,
        .helpMsgId = MSG_WM_SKILL_SHOP_DESC,
        .overrideId = 5,
        .isAvailable = MenuAlwaysEnabled,
        .onSelected = WMMenu_OnSkillShopSelected,
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