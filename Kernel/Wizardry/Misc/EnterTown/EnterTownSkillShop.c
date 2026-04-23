#include "common-chax.h"
#include "constants/faces.h"
#include "constants/texts.h"
#include "bwl.h"
#include "gamecontrol.h"
#include "icon-rework.h"
#include "jester_headers/custom-arrays.h"
#include "jester_headers/custom-functions.h"
#include "jester_headers/custom-structs.h"
#include "jester_headers/procs.h"
#include "kernel-lib.h"
#include "kernel/prep-skill.h"
#include "popup.h"
#include "skill-system.h"
#include "worldmap.h"

enum {
    WM_SKILL_SHOP_ITEM_COUNT = 7,
    WM_SKILL_SHOP_VISIBLE_COUNT = 5,
    WM_SKILL_SHOP_TEXT_COUNT = 2 + WM_SKILL_SHOP_VISIBLE_COUNT,
    WM_SKILL_SHOP_TEXT_BASE = 0x0C,
};

struct WorldMapSkillShopProc {
    PROC_HEADER;

    u8 unitId;
    u8 savedX;
    u8 savedY;
    u8 cursor;
    u8 listTop;
    u8 handEnabled;
};

static const u16 sWorldMapSkillShopSkillIds[WM_SKILL_SHOP_ITEM_COUNT] = {
    1, 2, 3, 4, 5, 6, 7,
};

static const u8 sWorldMapSkillShopSkillCosts[WM_SKILL_SHOP_ITEM_COUNT] = {
    5, 10, 15, 20, 25, 30, 35,
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
    {
        CloseHelpBox();
    }
}

static bool WorldMapSkillShop_HelpBoxActive(void)
{
    return Proc_Find(gProcScr_HelpBox) != NULL;
}

static void WorldMapSkillShop_ShowHelp(struct WorldMapSkillShopProc *proc)
{
    u16 sid = sWorldMapSkillShopSkillIds[proc->cursor];
    int row = proc->cursor - proc->listTop;

    LoadHelpBoxGfx(NULL, 2);
    StartHelpBox(2 * 8, (9 + (row * 2)) * 8, GetSkillDescMsg(sid));
}

static void WorldMapSkillShop_RefreshHelp(struct WorldMapSkillShopProc *proc)
{
    if (!WorldMapSkillShop_HelpBoxActive())
        return;

    WorldMapSkillShop_ShowHelp(proc);
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

    SetTextFont(0);
    InitSystemTextFont();

    TileMap_FillRect(TILEMAP_LOCATED(gBG0TilemapBuffer, 3, 8), 23, 12, 0);
    BG_Fill(gBG1TilemapBuffer, 0);

    for (i = 0; i < WM_SKILL_SHOP_TEXT_COUNT; ++i)
        ClearText(&gPrepUnitTexts[WM_SKILL_SHOP_TEXT_BASE + i]);

    DrawUiFrame2(3, 8, 23, 12, 0);

    PutDrawText(&gPrepUnitTexts[WM_SKILL_SHOP_TEXT_BASE + 0], TILEMAP_LOCATED(gBG0TilemapBuffer, 21, 6), TEXT_COLOR_SYSTEM_BLUE, 0, 0, "SP:");
    PutNumber(TILEMAP_LOCATED(gBG0TilemapBuffer, 26, 6), TEXT_COLOR_SYSTEM_BLUE, skillPoints);

    UpdateMenuScrollBarConfig(8, proc->listTop * 16, WM_SKILL_SHOP_ITEM_COUNT, WM_SKILL_SHOP_VISIBLE_COUNT);

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
            &gPrepUnitTexts[WM_SKILL_SHOP_TEXT_BASE + 1 + i],
            TILEMAP_LOCATED(gBG0TilemapBuffer, 7, y),
            textColor,
            0,
            14,
            GetSkillNameStr(sid));

        PutNumber(TILEMAP_LOCATED(gBG0TilemapBuffer, 22, y), TEXT_COLOR_SYSTEM_BLUE, cost);
    }

    if (proc->handEnabled)
        ShowSysHandCursor(28, 72 + ((proc->cursor - proc->listTop) * 16), 0x0, 0x800);

    BG_EnableSyncByMask(BG0_SYNC_BIT | BG1_SYNC_BIT);
}

static void WorldMapSkillShop_OpenHelp(struct WorldMapSkillShopProc *proc)
{
    WorldMapSkillShop_ShowHelp(proc);
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
    // SetPopupUnit(unit);
    // SetPopupItem(sid);
    // NewPopup_Simple(PopupScr_LearnSkill, SONG_SE_UPDATE, 0x00, proc);

    return true;
}

static void WorldMapSkillShop_EntryDialogue(struct WorldMapSkillShopProc *proc)
{
    SetInitTalkTextFont();
    ClearTalkText();

    StartTalkExt(8, 2, GetStringFromIndex(MSG_WM_SKILL_SHOP_DIALOGUE), proc);
    SetTalkPrintColor(0);
    SetTalkFlag(TALK_FLAG_INSTANTSHIFT);
    SetTalkFlag(TALK_FLAG_NOBUBBLE);
    SetTalkFlag(TALK_FLAG_NOSKIP);
    SetActiveTalkFace(1);
}

static void WorldMapSkillShop_StartShopUi(struct WorldMapSkillShopProc *proc)
{
    StartMenuScrollBar(proc);
    PutMenuScrollBarAt(2 * 8, 76);
    InitMenuScrollBarImg(0x7A60, 5);
    StartUiGoldBox_New(160, 45, 4, proc);

    WorldMapSkillShop_Draw(proc);
}

static void WorldMapSkillShop_HandleEntryChoice(struct WorldMapSkillShopProc *proc)
{
    if (GetTalkChoiceResult() != TALK_CHOICE_YES) {
        Proc_End(proc);
        return;
    }

    ResetSysHandCursor(proc);
    StartUiCursorHand(proc);
    DisplaySysHandCursorTextShadow(0x600, false);
    ShowSysHandCursor(28, 72 + ((proc->cursor - proc->listTop) * 16), 0x0, 0x800);
    proc->handEnabled = 1;

    StartShopDialogue(0x8A3, (struct ProcShop *)proc);
}

static void WorldMapSkillShop_Init(struct WorldMapSkillShopProc *proc)
{
    proc->unitId = gGMData.units[0].id;
    proc->savedX = gGMData.xCamera;
    proc->savedY = gGMData.yCamera;
    proc->cursor = 0;
    proc->listTop = 0;
    proc->handEnabled = 0;

    StartBgm(SONG_SHOPS, 0);
    InitTalk(0x200, 2, 0);
    SetPrimaryHBlankHandler(PrepItemSupply_OnHBlank);

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
    StartTalkFace(FID_SHOP_VENDOR, 32, 8, 3, 1);
    ApplyPalette(Pal_TalkBubble, 3);

    ApplyPalette(Pal_CommGameBgScreenInShop, BGPAL_SHOP_MAINBG);
    Decompress(Img_CommGameBgScreen, (void *)BG_VRAM + GetBackgroundTileDataOffset(BG_3));
    CallARM_FillTileRect(gBG3TilemapBuffer, Tsa_CommGameBgScreenInShop, OBJ_PALETTE(BGPAL_SHOP_MAINBG));

    for (int i = 0; i < WM_SKILL_SHOP_TEXT_COUNT; ++i)
        InitText(&gPrepUnitTexts[WM_SKILL_SHOP_TEXT_BASE + i], 16);
}

static void WorldMapSkillShop_Loop(struct WorldMapSkillShopProc *proc)
{
    if (gKeyStatusPtr->newKeys & B_BUTTON) {
        if (WorldMapSkillShop_HelpBoxActive()) {
            WorldMapSkillShop_CloseHelp();
            return;
        }

        Proc_Break(proc);
        PlaySoundEffect(SONG_SE_SYS_WINDOW_CANSEL1);
        return;
    }

    if (gKeyStatusPtr->newKeys & DPAD_UP) {
        if (proc->cursor > 0) {
            proc->cursor--;
            WorldMapSkillShop_ClampCursor(proc);
            WorldMapSkillShop_Draw(proc);
            WorldMapSkillShop_RefreshHelp(proc);
            PlaySoundEffect(SONG_SE_SYS_CURSOR_UD1);
        }
    }

    if (gKeyStatusPtr->newKeys & DPAD_DOWN) {
        if (proc->cursor + 1 < WM_SKILL_SHOP_ITEM_COUNT) {
            proc->cursor++;
            WorldMapSkillShop_ClampCursor(proc);
            WorldMapSkillShop_Draw(proc);
            WorldMapSkillShop_RefreshHelp(proc);
            PlaySoundEffect(SONG_SE_SYS_CURSOR_UD1);
        }
    }

    if (gKeyStatusPtr->newKeys & R_BUTTON) {
        WorldMapSkillShop_OpenHelp(proc);
    }

    if (gKeyStatusPtr->newKeys & A_BUTTON) {
        bool helpWasActive = WorldMapSkillShop_HelpBoxActive();

        WorldMapSkillShop_CloseHelp();

        if (WorldMapSkillShop_TryPurchase(proc)) {
            WorldMapSkillShop_Draw(proc);
            if (helpWasActive)
                WorldMapSkillShop_RefreshHelp(proc);
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
    ResetDialogueScreen();

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
    PROC_CALL(WorldMapSkillShop_StartShopUi),
    PROC_CALL(WorldMapSkillShop_EntryDialogue),
    PROC_WHILE(IsTalkActive),
    PROC_CALL(WorldMapSkillShop_HandleEntryChoice),
    PROC_REPEAT(WorldMapSkillShop_Loop),
    PROC_CALL_ARG(NewFadeOut, 0x10),
    PROC_WHILE(FadeOutExists),
    PROC_END,
};

static void StartWMNodeSkillShop(struct MenuProc *menuProc)
{
    Proc_StartBlocking(ProcScr_WMNodeSkillShop, Proc_Find(ProcScr_WorldMapMain));
}

u8 WMMenu_OnSkillShopSelected(struct MenuProc *menuProc, struct MenuItemProc *menuItemProc)
{
    gGMData.xCamera = 0;
    gGMData.yCamera = 0;
    gGMData.unk_cd = menuProc->itemCurrent;
    StartWMNodeSkillShop(menuProc);
    return MENU_ACT_SKIPCURSOR | MENU_ACT_END | MENU_ACT_SND6A | MENU_ACT_CLEAR;
}