#include "common-chax.h"
#include "constants/faces.h"
#include "constants/skills.h"
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
    WM_SKILL_SHOP_VISIBLE_COUNT = 5,
    WM_SKILL_SHOP_TEXT_COUNT = 2 + WM_SKILL_SHOP_VISIBLE_COUNT,
    WM_SKILL_SHOP_TEXT_BASE = 0x0C,
    WM_SKILL_SHOP_NODE_COUNT = 2,
    WM_SKILL_SHOP_ITEM_COUNT = 7,
};

struct WorldMapSkillShopProc {
    PROC_HEADER;

    u8 unitId;
    u8 savedX;
    u8 savedY;
    u8 cursor;
    u8 listTop;
    u8 handEnabled;
    u8 itemCount;
    u8 skillCount;
    u8 scrollEnabled;
    s8 nodeIndex;
};

static const u8 sWorldMapSkillShopNodeIds[WM_SKILL_SHOP_NODE_COUNT] = {
    NODE_IDE,
    NODE_SERAFEW,
};

static const u16 sWorldMapSkillShopSkillIds[WM_SKILL_SHOP_NODE_COUNT][WM_SKILL_SHOP_ITEM_COUNT] = {
    {
        SID_Absolve,
        SID_Astra,
        SID_Fury,
        SID_Counter,
    },
    {
        SID_Fury,
        SID_FuryPlus,
        SID_FortressDef,
        SID_FortressRes,
        SID_BlowDarting,
        SID_BlowDeath,
        SID_BlowArmored,
    },
};

static const u8 sWorldMapSkillShopSkillCosts[WM_SKILL_SHOP_NODE_COUNT][WM_SKILL_SHOP_ITEM_COUNT] = {
    {
        10,
        20,
        30,
        40,
    },
    {
        5,
        10,
        15,
        20,
        25,
        30,
        35,
    },
};

static u8 WorldMapSkillShop_GetSkillCount(int nodeIndex)
{
    int i;
    u8 count = 0;

    if (nodeIndex < 0)
        return 0;

    for (i = 0; i < WM_SKILL_SHOP_ITEM_COUNT; ++i) {
        if (sWorldMapSkillShopSkillIds[nodeIndex][i] != 0)
            ++count;
    }

    return count;
}

static bool WorldMapSkillShop_IsBlankSlot(int nodeIndex, int skillIndex)
{
    return nodeIndex < 0 || skillIndex < 0 || skillIndex >= WM_SKILL_SHOP_ITEM_COUNT || sWorldMapSkillShopSkillIds[nodeIndex][skillIndex] == 0;
}

static bool WorldMapSkillShop_MoveCursorToNextSkill(struct WorldMapSkillShopProc *proc, int direction)
{
    int cursor;

    if (proc->nodeIndex < 0)
        return false;

    cursor = proc->cursor;

    if (direction > 0) {
        for (cursor++; cursor < proc->itemCount; ++cursor) {
            if (!WorldMapSkillShop_IsBlankSlot(proc->nodeIndex, cursor)) {
                proc->cursor = cursor;
                return true;
            }
        }
    } else {
        for (cursor--; cursor >= 0; --cursor) {
            if (!WorldMapSkillShop_IsBlankSlot(proc->nodeIndex, cursor)) {
                proc->cursor = cursor;
                return true;
            }
        }
    }

    return false;
}

static void WorldMapSkillShop_UpdateHandCursor(struct WorldMapSkillShopProc *proc)
{
    if (proc->handEnabled)
        ShowSysHandCursor(28, 72 + ((proc->cursor - proc->listTop) * 16), 0x0, 0x800);
}

static int WorldMapSkillShop_GetNodeIndex(u8 nodeId)
{
    unsigned i;

    for (i = 0; i < WM_SKILL_SHOP_NODE_COUNT; ++i) {
        if (sWorldMapSkillShopNodeIds[i] == nodeId)
            return i;
    }

    return -1;
}

bool WorldMapSkillShop_HasNodeShop(u8 nodeId)
{
    return WorldMapSkillShop_GetNodeIndex(nodeId) >= 0;
}

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

static bool WorldMapSkillShop_HelpBoxActive(void)
{
    return Proc_Find(gProcScr_HelpBox) != NULL;
}

static void WorldMapSkillShop_ShowHelp(struct WorldMapSkillShopProc *proc)
{
    u16 sid;
    int row = proc->cursor - proc->listTop;

    if (proc->nodeIndex < 0 || proc->cursor >= proc->itemCount || WorldMapSkillShop_IsBlankSlot(proc->nodeIndex, proc->cursor))
        return;

    sid = sWorldMapSkillShopSkillIds[proc->nodeIndex][proc->cursor];

    LoadHelpBoxGfx(NULL, 2);
    StartHelpBox(2 * 8, (9 + (row * 2)) * 8, GetSkillDescMsg(sid));
    StartUiGoldBox_New(160, 45, 4, proc);
}

static bool WorldMapSkillShop_IsSkillListFull(struct Unit *unit)
{
    if (!UNIT_IS_VALID(unit))
        return false;

    return GetFreeSkillSlot(unit) == -1;
}

static void WorldMapSkillShop_RefreshHelp(struct WorldMapSkillShopProc *proc)
{
    if (WorldMapSkillShop_HelpBoxActive())
        WorldMapSkillShop_ShowHelp(proc);
}

static void WorldMapSkillShop_ClampCursor(struct WorldMapSkillShopProc *proc)
{
    if (proc->itemCount == 0) {
        proc->cursor = 0;
        proc->listTop = 0;
        return;
    }

    if (proc->cursor >= proc->itemCount)
        proc->cursor = proc->itemCount - 1;

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
    int visibleCount = proc->itemCount < WM_SKILL_SHOP_VISIBLE_COUNT ? proc->itemCount : WM_SKILL_SHOP_VISIBLE_COUNT;

    SetTextFont(0);
    InitSystemTextFont();

    TileMap_FillRect(TILEMAP_LOCATED(gBG0TilemapBuffer, 3, 8), 23, 12, 0);
    BG_Fill(gBG1TilemapBuffer, 0);

    for (i = 0; i < WM_SKILL_SHOP_TEXT_COUNT; ++i)
        ClearText(&gPrepUnitTexts[WM_SKILL_SHOP_TEXT_BASE + i]);

    DrawUiFrame2(3, 8, 23, 12, 0);
    DrawUiFrame2(0, 0, 8, 8, 2);

    PutDrawText(&gPrepUnitTexts[WM_SKILL_SHOP_TEXT_BASE + 0], TILEMAP_LOCATED(gBG0TilemapBuffer, 21, 6), TEXT_COLOR_SYSTEM_BLUE, 0, 0, "SP:");
    PutNumber(TILEMAP_LOCATED(gBG0TilemapBuffer, 26, 6), TEXT_COLOR_SYSTEM_BLUE, skillPoints);

    if (proc->scrollEnabled)
        UpdateMenuScrollBarConfig(8, proc->listTop * 16, proc->skillCount, WM_SKILL_SHOP_VISIBLE_COUNT);

    for (i = 0; i < visibleCount; ++i) {
        int skillIndex = proc->listTop + i;
        int y = 9 + (i * 2);
        u16 sid;
        u8 cost;
        int textColor = TEXT_COLOR_SYSTEM_WHITE;

        if (skillIndex >= proc->itemCount || WorldMapSkillShop_IsBlankSlot(proc->nodeIndex, skillIndex))
            continue;

        sid = sWorldMapSkillShopSkillIds[proc->nodeIndex][skillIndex];
        cost = sWorldMapSkillShopSkillCosts[proc->nodeIndex][skillIndex];

        if (unit && SkillTester(unit, sid))
            textColor = TEXT_COLOR_SYSTEM_GRAY;
        else if (WorldMapSkillShop_IsSkillListFull(unit))
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

    WorldMapSkillShop_UpdateHandCursor(proc);

    BG_EnableSyncByMask(BG0_SYNC_BIT | BG1_SYNC_BIT);
}

static void WorldMapSkillShop_OpenHelp(struct WorldMapSkillShopProc *proc)
{
    WorldMapSkillShop_ShowHelp(proc);
}

static int WorldMapSkillShop_TryPurchase(struct WorldMapSkillShopProc *proc)
{
    struct Unit *unit = WorldMapSkillShop_GetUnit(proc);
    struct NewBwl *bwl;
    u16 sid;
    u8 cost;

    if (proc->nodeIndex < 0 || proc->cursor >= proc->itemCount || WorldMapSkillShop_IsBlankSlot(proc->nodeIndex, proc->cursor))
        return 0;

    sid = sWorldMapSkillShopSkillIds[proc->nodeIndex][proc->cursor];
    cost = sWorldMapSkillShopSkillCosts[proc->nodeIndex][proc->cursor];

    if (!UNIT_IS_VALID(unit))
        return 0;

    if (!CheckHasBwl(UNIT_CHAR_ID(unit)))
        return 0;

    bwl = GetNewBwl(UNIT_CHAR_ID(unit));
    if (!bwl)
        return 0;

    if (SkillTester(unit, sid))
        return 0;

    if (bwl->skillPoints < cost) {
        StartShopDialogue(MSG_WM_SKILL_SHOP_NO_FUNDS, (struct ProcShop *)proc);
        Proc_Goto(proc, 1);
        return -1;
    }

    if (WorldMapSkillShop_IsSkillListFull(unit)) {
        StartShopDialogue(MSG_WM_SKILL_SHOP_NO_SPACE, (struct ProcShop *)proc);
        Proc_Goto(proc, 1);
        return -1;
    }

    if (AddSkill(unit, sid) != 0)
        return 0;

    bwl->skillPoints -= cost;
    return 1;
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
    if (proc->scrollEnabled) {
        StartMenuScrollBar(proc);
        PutMenuScrollBarAt(2 * 8, 76);
        InitMenuScrollBarImg(0x7A60, 5);
    }

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
    WorldMapSkillShop_UpdateHandCursor(proc);
    proc->handEnabled = 1;

    StartShopDialogue(0x8A3, (struct ProcShop *)proc);
    ShowSysHandCursor(28, 72 + ((proc->cursor - proc->listTop) * 16), 0x0, 0x800);
}

static void WorldMapSkillShop_Init(struct WorldMapSkillShopProc *proc)
{
    proc->nodeIndex = WorldMapSkillShop_GetNodeIndex(gGMData.units[0].location);
    proc->itemCount = proc->nodeIndex >= 0 ? WM_SKILL_SHOP_ITEM_COUNT : 0;
    proc->skillCount = WorldMapSkillShop_GetSkillCount(proc->nodeIndex);
    proc->scrollEnabled = proc->skillCount >= 6;
    proc->unitId = gGMData.units[0].id;
    proc->savedX = gGMData.xCamera;
    proc->savedY = gGMData.yCamera;
    proc->cursor = 0;
    proc->listTop = 0;
    proc->handEnabled = 0;

    if (proc->nodeIndex < 0) {
        Proc_End(proc);
        return;
    }

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
            if (WorldMapSkillShop_MoveCursorToNextSkill(proc, -1)) {
                WorldMapSkillShop_ClampCursor(proc);
                if (proc->scrollEnabled)
                    WorldMapSkillShop_Draw(proc);
                else
                    WorldMapSkillShop_UpdateHandCursor(proc);
                WorldMapSkillShop_RefreshHelp(proc);
                PlaySoundEffect(SONG_SE_SYS_CURSOR_UD1);
            }
        }
    }

    if (gKeyStatusPtr->newKeys & DPAD_DOWN) {
        if (proc->cursor + 1 < proc->itemCount) {
            if (WorldMapSkillShop_MoveCursorToNextSkill(proc, 1)) {
                WorldMapSkillShop_ClampCursor(proc);
                if (proc->scrollEnabled)
                    WorldMapSkillShop_Draw(proc);
                else
                    WorldMapSkillShop_UpdateHandCursor(proc);
                WorldMapSkillShop_RefreshHelp(proc);
                PlaySoundEffect(SONG_SE_SYS_CURSOR_UD1);
            }
        }
    }

    if (gKeyStatusPtr->newKeys & R_BUTTON)
        WorldMapSkillShop_OpenHelp(proc);

    if (gKeyStatusPtr->newKeys & A_BUTTON) {
        bool helpWasActive = WorldMapSkillShop_HelpBoxActive();
        int purchaseResult;

        WorldMapSkillShop_CloseHelp();
        purchaseResult = WorldMapSkillShop_TryPurchase(proc);

        if (purchaseResult > 0) {
            WorldMapSkillShop_Draw(proc);
            if (helpWasActive)
                WorldMapSkillShop_RefreshHelp(proc);
            PlaySoundEffect(SONG_SE_SYS_WINDOW_SELECT1);
        } else if (purchaseResult == 0) {
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
