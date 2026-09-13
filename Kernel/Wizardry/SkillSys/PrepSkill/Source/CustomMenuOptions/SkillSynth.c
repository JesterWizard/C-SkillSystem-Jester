#include "common-chax.h"
#include "utf8.h"
#include "kernel-lib.h"
#include "constants/texts.h"
#include "constants/skills.h"
#include "popup.h"
#include "prep-skill.h"
#include "worldmap.h"
#include "skill-system.h"
#include "icon-rework.h"
#include "jester_headers/custom-functions.h"
#include "jester_headers/custom-structs.h"

struct SkillSynthListProc {
    PROC_HEADER;

    struct Unit *unit;
    u8 unitInvIdx;
    s8 scrollAmount;
    u8 unk_32;
    u8 currentPage;
    u16 unk_34;
    u16 unk_36;
    u16 idxPerPage[9];
    u16 yOffsetPerPage[9];

    u8 fromWorldMap;
    u8 state;
    u8 confirmChoice;
    s16 firstIdx;
    s16 secondIdx;
    u8 handX;
    u8 handY;
    u8 wmGfxPaused;
    u8 savedWorldMapNodeIconState;
};

const struct SkillSynthRecipe gSkillSynthRecipeTable[] = {
#if defined(SID_Fury) && defined(SID_FuryPlus)
    { SID_Fury, SID_Fury, SID_FuryPlus },
#endif
#if defined(SID_Counter) && defined(SID_Absolve)
    { SID_Counter, SID_Absolve, SID_Fury },
#endif
    { 0, 0, 0 },
};

static const struct PopupInstruction SkillSynthPopup[] = {
    POPUP_SOUND(SONG_SE_UPDATE),
    POPUP_COLOR(TEXT_COLOR_SYSTEM_WHITE),
    POPUP_MSG(MSG_INFUSED),
    POPUP_COLOR(TEXT_COLOR_SYSTEM_GOLD),
    POPUP_ITEM_STR,
    POPUP_SPACE(1),
    POPUP_ITEM_ICON,
    POPUP_COLOR(TEXT_COLOR_SYSTEM_WHITE),
    POPUP_SPACE(1),
    POPUP_MSG(0x022),
    POPUP_END
};

static bool SkillSynth_NormalizePair(u16 sidA, u16 sidB, u16 *outLo, u16 *outHi)
{
    if (sidA <= sidB) {
        *outLo = sidA;
        *outHi = sidB;
    } else {
        *outLo = sidB;
        *outHi = sidA;
    }

    return *outLo != 0 && *outHi != 0;
}

static u16 SkillSynth_LookupResult(u16 sidA, u16 sidB)
{
    u16 lo;
    u16 hi;
    int i;

    if (!SkillSynth_NormalizePair(sidA, sidB, &lo, &hi))
        return 0;

    for (i = 0; gSkillSynthRecipeTable[i].sid_a != 0 || gSkillSynthRecipeTable[i].sid_b != 0; ++i) {
        u16 recipeLo;
        u16 recipeHi;

        if (!SkillSynth_NormalizePair(
                gSkillSynthRecipeTable[i].sid_a,
                gSkillSynthRecipeTable[i].sid_b,
                &recipeLo,
                &recipeHi))
            continue;

        if (recipeLo == lo && recipeHi == hi)
            return gSkillSynthRecipeTable[i].sid_result;
    }

    return 0;
}

static void SkillSynth_LockWmProc(ProcPtr proc)
{
    struct Proc *p = proc;

    if (p != NULL && p->proc_lockCnt < 127)
        p->proc_lockCnt++;
}

static void SkillSynth_UnlockWmProc(ProcPtr proc)
{
    struct Proc *p = proc;

    if (p != NULL && p->proc_lockCnt > 0)
        p->proc_lockCnt--;
}

static void SkillSynth_PauseWorldMapGfx(struct SkillSynthListProc *proc)
{
    if (!proc->fromWorldMap || proc->wmGfxPaused)
        return;

    if (GM_MAIN == NULL)
        return;

    proc->savedWorldMapNodeIconState = GM_ICON ? GM_ICON->skip : 0;
    SkillSynth_LockWmProc(GM_SCREEN);
    SkillSynth_LockWmProc(GM_ICON);
    SkillSynth_LockWmProc(GM_UNITC);
    SkillSynth_LockWmProc(GM_CURSOR);
    SkillSynth_LockWmProc(GM_MU);

    HideGmUnit(-1);
    gGMData.sprite_disp = 0;
    if (GM_ICON)
        GM_ICON->skip = 0;

    ClearSprites();
    ResetUnitSprites();
    CpuFastFill16(0, (void *)0x06010000, 0x5FE0);
    proc->wmGfxPaused = true;
}

static void SkillSynth_ResumeWorldMapGfx(struct SkillSynthListProc *proc)
{
    if (!proc->fromWorldMap || !proc->wmGfxPaused)
        return;

    if (GM_MAIN != NULL) {
        SkillSynth_UnlockWmProc(GM_MU);
        SkillSynth_UnlockWmProc(GM_CURSOR);
        SkillSynth_UnlockWmProc(GM_UNITC);
        SkillSynth_UnlockWmProc(GM_ICON);
        SkillSynth_UnlockWmProc(GM_SCREEN);
        if (GM_ICON)
            GM_ICON->skip = proc->savedWorldMapNodeIconState;
    }

    proc->wmGfxPaused = false;
}

static bool SkillSynth_GetSidFromIdx(int idx, u16 *outSid)
{
    int sid = 0;

    if (idx < 0 || idx >= gUnknown_02012F56)
        return false;

    if (!TryGetSkillScrollSid(gPrepScreenItemList[idx].item, &sid))
        return false;

    *outSid = sid;
    return true;
}

static void SkillSynth_RemoveAt(const struct PrepScreenItemListEnt *ent)
{
    if (ent->pid == 0) {
        RemoveItemFromConvoy(ent->itemSlot);
        return;
    }

    {
        struct Unit *unit = GetUnitFromCharId(ent->pid);

        if (unit != NULL) {
            unit->items[ent->itemSlot] = 0;
            UnitRemoveInvalidItems(unit);
        }
    }
}

static void SkillSynth_WriteAt(u8 pid, u8 itemSlot, u16 item)
{
    if (pid == 0) {
        u16 *convoy = GetConvoyItemArray();

        convoy[itemSlot] = item;
        return;
    }

    {
        struct Unit *unit = GetUnitFromCharId(pid);

        if (unit != NULL)
            unit->items[itemSlot] = item;
    }
}

static void SkillSynth_BuildScrollList(struct SkillSynthListProc *proc)
{
    struct PrepScreenItemListEnt *dst = gPrepScreenItemList;
    int count = 0;
    int i;
    int j;
    u16 *convoy = GetConvoyItemArray();

#if CHAX
    extern u16 sExpaConvoyItemAmount;

    for (j = 0; j < sExpaConvoyItemAmount; ++j) {
#else
    for (j = 0; j < CONVOY_ITEM_COUNT; ++j) {
#endif
        if (convoy[j] == 0)
            continue;

        if (IsSkillScrollItem(convoy[j])) {
            dst->item = convoy[j];
            dst->pid = 0;
            dst->itemSlot = j;
            dst++;
            count++;
        }
    }

    for (i = FACTION_BLUE + 1; i < FACTION_GREEN; ++i) {
        struct Unit *unit = GetUnit(i);

        if (!UNIT_IS_VALID(unit))
            continue;

        if (unit->state & US_DEAD)
            continue;

        for (j = 0; j < UNIT_ITEM_COUNT; ++j) {
            if (unit->items[j] == 0)
                continue;

            if (IsSkillScrollItem(unit->items[j])) {
                dst->pid = unit->pCharacterData->number;
                dst->item = unit->items[j];
                dst->itemSlot = j;
                dst++;
                count++;
            }
        }
    }

    gUnknown_02012F54 = count;
    gUnknown_02012F56 = count;
    proc->currentPage = 0;
    proc->idxPerPage[0] = 0;
    proc->yOffsetPerPage[0] = 0;
}

#define SKILL_SYNTH_VISIBLE 7
#define SKILL_SYNTH_LIST_X 16
#define SKILL_SYNTH_LIST_Y 5

static char *SkillSynth_GetName(u16 item)
{
    int sid = 0;

    if (TryGetSkillScrollSid(item, &sid))
        return GetSkillNameStr(sid);

    return GetItemName(item);
}

static int SkillSynth_GetIcon(u16 item)
{
    int sid = 0;

    if (TryGetSkillScrollSid(item, &sid))
        return SKILL_ICON(sid);

    return GetItemIconId(item);
}

static void SkillSynth_DrawScrollItems(struct SkillSynthListProc *proc)
{
    int i;
    int top = proc->yOffsetPerPage[0] >> 4;

    proc->currentPage = 0;
    TileMap_FillRect(TILEMAP_LOCATED(gBG0TilemapBuffer, SKILL_SYNTH_LIST_X, SKILL_SYNTH_LIST_Y), 13, 14, 0);

    if (gUnknown_02012F56 == 0) {
        ClearText(PrepItemSuppyTexts.th + 7);
        PutDrawText(
            PrepItemSuppyTexts.th + 7,
            TILEMAP_LOCATED(gBG0TilemapBuffer, SKILL_SYNTH_LIST_X + 2, SKILL_SYNTH_LIST_Y),
            TEXT_COLOR_SYSTEM_GRAY,
            0,
            0,
            GetStringFromIndex(0x5a8)
        );
        BG_EnableSyncByMask(BG0_SYNC_BIT);
        return;
    }

    for (i = 0; i < SKILL_SYNTH_VISIBLE; ++i) {
        int idx = top + i;
        int y = SKILL_SYNTH_LIST_Y + i * 2;
        struct Text *th = PrepItemSuppyTexts.th + 7 + i;

        ClearText(th);

        if (idx >= gUnknown_02012F56)
            continue;

        DrawIcon(
            TILEMAP_LOCATED(gBG0TilemapBuffer, SKILL_SYNTH_LIST_X, y),
            SkillSynth_GetIcon(gPrepScreenItemList[idx].item),
            0x4000
        );
        PutDrawText(
            th,
            TILEMAP_LOCATED(gBG0TilemapBuffer, SKILL_SYNTH_LIST_X + 2, y),
            TEXT_COLOR_SYSTEM_WHITE,
            0,
            0,
            SkillSynth_GetName(gPrepScreenItemList[idx].item)
        );
    }

    BG_EnableSyncByMask(BG0_SYNC_BIT);
}

static void SkillSynth_DrawOwnerText(struct SkillSynthListProc *proc)
{
    int idx = proc->idxPerPage[proc->currentPage];
    u16 *tm = TILEMAP_LOCATED(gBG0TilemapBuffer, 20, 1);

    TileMap_FillRect(tm, 10, 1, 0);
    ClearText(PrepItemSuppyTexts.th + 1);

    if (gUnknown_02012F56 <= idx) {
        PutDrawText(PrepItemSuppyTexts.th + 1, tm, 1, 0, 0, GetStringFromIndex(0x536));
    } else if (gPrepScreenItemList[idx].pid == 0) {
        PutDrawText(PrepItemSuppyTexts.th + 1, tm, 3, 0, 0, GetStringFromIndex(0x598));
    } else {
        PutDrawText(
            PrepItemSuppyTexts.th + 1,
            tm,
            0,
            0,
            0,
            GetStringFromIndex(GetUnitFromCharId(gPrepScreenItemList[idx].pid)->pCharacterData->nameTextId)
        );
    }

    BG_EnableSyncByMask(BG0_SYNC_BIT);
}

static void SkillSynth_DrawPreviewSlot(struct Text *th, int y, u16 item, int color, const char *emptyStr)
{
    ClearText(th);
    TileMap_FillRect(TILEMAP_LOCATED(gBG0TilemapBuffer, 2, y), 12, 2, 0);

    if (item == 0) {
        PutDrawText(
            th,
            TILEMAP_LOCATED(gBG0TilemapBuffer, 2, y),
            TEXT_COLOR_SYSTEM_GRAY,
            4,
            0,
            emptyStr
        );
        return;
    }

    DrawIcon(TILEMAP_LOCATED(gBG0TilemapBuffer, 2, y), SkillSynth_GetIcon(item), 0x4000);
    PutDrawText(
        th,
        TILEMAP_LOCATED(gBG0TilemapBuffer, 4, y),
        color,
        2,
        0,
        SkillSynth_GetName(item)
    );
}

static void SkillSynth_DrawPreview(struct SkillSynthListProc *proc)
{
    int cursor = proc->idxPerPage[proc->currentPage];
    int sidA = 0;
    int sidB = 0;
    u16 hover = 0;
    u16 itemA = 0;
    u16 itemB = 0;
    u16 resultItem = 0;
    const char *resultEmpty = "Nothing";

    TileMap_FillRect(TILEMAP_LOCATED(gBG0TilemapBuffer, 1, 8), 13, 11, 0);
    ClearText(&PrepItemSuppyTexts.th[2]);
    ClearText(&PrepItemSuppyTexts.th[3]);
    ClearText(&PrepItemSuppyTexts.th[4]);

    if (cursor >= 0 && cursor < gUnknown_02012F56)
        hover = gPrepScreenItemList[cursor].item;

    if (proc->firstIdx >= 0)
        itemA = gPrepScreenItemList[proc->firstIdx].item;
    else
        itemA = hover;

    if (proc->secondIdx >= 0)
        itemB = gPrepScreenItemList[proc->secondIdx].item;
    else if (proc->firstIdx >= 0 && cursor != proc->firstIdx)
        itemB = hover;

    if (itemA != 0 && itemB != 0) {
        if (TryGetSkillScrollSid(itemA, &sidA) && TryGetSkillScrollSid(itemB, &sidB)) {
            u16 resultSid = SkillSynth_LookupResult(sidA, sidB);

            if (resultSid != 0)
                resultItem = MakeSkillScrollItem(resultSid);
            else
                resultEmpty = "No recipe";
        } else {
            resultEmpty = "No recipe";
        }
    }

    SkillSynth_DrawPreviewSlot(&PrepItemSuppyTexts.th[2], 9, itemA, TEXT_COLOR_SYSTEM_WHITE, "Nothing");
    SkillSynth_DrawPreviewSlot(&PrepItemSuppyTexts.th[3], 13, itemB, TEXT_COLOR_SYSTEM_WHITE, "Nothing");
    SkillSynth_DrawPreviewSlot(
        &PrepItemSuppyTexts.th[4],
        17,
        resultItem,
        TEXT_COLOR_SYSTEM_GREEN,
        resultEmpty
    );

    BG_EnableSyncByMask(BG0_SYNC_BIT);
}

static void SkillSynth_LoadFrameGraphics(void)
{
    Decompress(Gfx_UI_Frame_One_Line_1, gGenericBuffer);
    Copy2dChr(gGenericBuffer, (void *)0x6015C00, 4, 4);
    Decompress(Gfx_UI_Frame_One_Line_2, gGenericBuffer);
    Copy2dChr(gGenericBuffer, (void *)0x6015C80, 4, 4);
    Decompress(Gfx_UI_Frame_One_Line_3, gGenericBuffer);
    Copy2dChr(gGenericBuffer, (void *)0x6015D00, 1, 4);
    Decompress(Gfx_UI_Frame_One_Line_4, gGenericBuffer);
    Copy2dChr(gGenericBuffer, (void *)0x6015D20, 4, 4);
}

static void SkillSynth_InitTexts(void)
{
    int i;

    InitText(PrepItemSuppyTexts.th + 1, 5);
    InitText(PrepItemSuppyTexts.th + 15, 4);

    for (i = 0; i < 8; ++i)
        InitText(PrepItemSuppyTexts.th + 7 + i, 7);

    InitText(&PrepItemSuppyTexts.th[0], 0xA);
    InitText(&PrepItemSuppyTexts.th[2], 0xA);
    InitText(&PrepItemSuppyTexts.th[3], 0xA);
    InitText(&PrepItemSuppyTexts.th[4], 0xA);
}

static void SkillSynth_DrawHeader(void)
{
    SetTextFont(NULL);
    TileMap_FillRect(TILEMAP_LOCATED(gBG0TilemapBuffer, 5, 1), 15, 4, 0);

    SkillSynth_LoadFrameGraphics();

    PutDrawText(
        &PrepItemSuppyTexts.th[0],
        TILEMAP_LOCATED(gBG0TilemapBuffer, 6, 2),
        TEXT_COLOR_SYSTEM_WHITE,
        2,
        0,
        Utf8ToNarrowFonts(GetStringFromIndex(MSG_PREP_SCREEN_TITLE_SKILL_SYNTH))
    );
    PutFaceChibi(FID_SUPPLY + 1, TILEMAP_LOCATED(gBG0TilemapBuffer, 1, 1), 0x270, 2, 0);
    BG_EnableSyncByMask(BG0_SYNC_BIT);
}

static void SkillSynth_PutBoxSprites(int y)
{
    PutSprite(1, 14, y, gObject_32x32, OAM2_PAL(0) + OAM2_LAYER(3) + OAM2_CHR(0x2E0));
    PutSprite(1, 46, y, gObject_32x32, OAM2_PAL(0) + OAM2_LAYER(3) + OAM2_CHR(0x2E4));
    PutSprite(1, 56, y, gObject_32x32, OAM2_PAL(0) + OAM2_LAYER(3) + OAM2_CHR(0x2E5));
    PutSprite(1, 86, y, gObject_32x32, OAM2_PAL(0) + OAM2_LAYER(3) + OAM2_CHR(0x2E9));
}

static void SkillSynth_DrawSprites(void)
{
    SkillSynth_PutBoxSprites(64);
    SkillSynth_PutBoxSprites(96);
    SkillSynth_PutBoxSprites(128);
}

static void SkillSynth_SpriteWorker(ProcPtr proc)
{
    struct SkillSynthListProc *synth = proc;

    if (synth->fromWorldMap) {
        HideGmUnit(-1);
        gGMData.sprite_disp = 0;
        if (GM_MAIN && GM_ICON)
            GM_ICON->skip = 0;
    }

    SkillSynth_DrawSprites();
    UpdateMenuScrollBarConfig(0xc, synth->yOffsetPerPage[synth->currentPage], gUnknown_02012F56, 7);
}

static void SkillSynth_SetupConfirmSprites(u16 resultSid)
{
    InitSpriteTextFont(&PrepItemSuppyTexts.font, (void *)0x6011000, 0xb);
    ApplyPalette(Pal_Text, 0x1B);
    InitSpriteText(&PrepItemSuppyTexts.th[0xf]);
    SetTextFont(&PrepItemSuppyTexts.font);
    SetTextFontGlyphs(0);
    SpriteText_DrawBackgroundExt(&PrepItemSuppyTexts.th[0xf], 0);
    Text_InsertDrawString(&PrepItemSuppyTexts.th[0xf], 0, TEXT_COLOR_SYSTEM_WHITE, "Yes");
    Text_InsertDrawString(&PrepItemSuppyTexts.th[0xf], 0x40, TEXT_COLOR_SYSTEM_WHITE, "No");
    Text_InsertDrawString(
        &PrepItemSuppyTexts.th[0xf],
        0x84,
        TEXT_COLOR_SYSTEM_WHITE,
        "Synthesize skill?"
    );
    Text_InsertDrawString(
        &PrepItemSuppyTexts.th[0xf],
        0xC0,
        TEXT_COLOR_SYSTEM_BLUE,
        GetSkillNameStr(resultSid)
    );
    SetTextFont(NULL);
}

static void SkillSynth_RedrawList(struct SkillSynthListProc *proc)
{
    proc->currentPage = 0;
    ResetIconGraphics_();
    SkillSynth_DrawOwnerText(proc);
    SkillSynth_DrawPreview(proc);
    SkillSynth_DrawScrollItems(proc);
    BG_EnableSyncByMask(BG0_SYNC_BIT);
}

static void SkillSynth_PlaceCursor(struct SkillSynthListProc *proc)
{
    int top = proc->yOffsetPerPage[0] >> 4;
    int idx = proc->idxPerPage[0];
    int yPos = 40 + (idx - top) * 16;

    ShowSysHandCursor(0x80, yPos, 0xB, 0x800);
    proc->handX = 0x80;
    proc->handY = yPos;
}

static void SkillSynth_EnsureCursorVisible(struct SkillSynthListProc *proc)
{
    int idx = proc->idxPerPage[0];
    int top = proc->yOffsetPerPage[0] >> 4;

    if (idx < top)
        top = idx;
    else if (idx >= top + SKILL_SYNTH_VISIBLE)
        top = idx - (SKILL_SYNTH_VISIBLE - 1);

    if (top < 0)
        top = 0;

    proc->yOffsetPerPage[0] = top * 16;
}

static void SkillSynth_ShowCursor(struct SkillSynthListProc *proc)
{
    SkillSynth_EnsureCursorVisible(proc);
    SkillSynth_RedrawList(proc);
    SkillSynth_PlaceCursor(proc);
}

static void SkillSynth_RefreshListView(struct SkillSynthListProc *proc)
{
    SkillSynth_BuildScrollList(proc);
    SkillSynth_EnsureCursorVisible(proc);
    SkillSynth_RedrawList(proc);
    SkillSynth_PlaceCursor(proc);
}

static void SkillSynth_PerformSynthesis(struct SkillSynthListProc *proc, u16 resultSid)
{
    const struct PrepScreenItemListEnt *ent1 = &gPrepScreenItemList[proc->firstIdx];
    const struct PrepScreenItemListEnt *ent2 = &gPrepScreenItemList[proc->secondIdx];
    u8 resultPid = ent1->pid;
    u8 resultSlot = ent1->itemSlot;
    u16 newItem = MakeSkillScrollItem(resultSid);

    if (ent1->pid == ent2->pid) {
        if (ent2->itemSlot > ent1->itemSlot) {
            SkillSynth_RemoveAt(ent2);
            SkillSynth_RemoveAt(ent1);
        } else {
            SkillSynth_RemoveAt(ent1);
            SkillSynth_RemoveAt(ent2);
        }
    } else {
        SkillSynth_RemoveAt(ent2);
        SkillSynth_RemoveAt(ent1);
    }

    SkillSynth_WriteAt(resultPid, resultSlot, newItem);

    proc->firstIdx = -1;
    proc->secondIdx = -1;
    proc->state = SKILL_SYNTH_STATE_LIST;

    PlaySoundEffect(SONG_SE_UPDATE);
    SetPopupItem(resultSid);
    NewPopup_Simple(SkillSynthPopup, 0x60, 0x00, proc);
    SkillSynth_RefreshListView(proc);
}

static void SkillSynth_InitGfx(struct SkillSynthListProc *proc)
{

    if (proc->fromWorldMap)
        SetDispEnable(0, 0, 0, 0, 0);

    gLCDControlBuffer.dispcnt.mode = 0;
    SetupBackgrounds(NULL);

    if (proc->fromWorldMap) {
        SetDispEnable(0, 0, 0, 0, 0);
        SkillSynth_PauseWorldMapGfx(proc);
    }

    BG_Fill(BG_GetMapBuffer(0), 0);
    BG_Fill(BG_GetMapBuffer(1), 0);
    BG_Fill(BG_GetMapBuffer(2), 0);
    BG_Fill(BG_GetMapBuffer(3), 0);

    gLCDControlBuffer.bg0cnt.priority = 0;
    gLCDControlBuffer.bg1cnt.priority = 2;
    gLCDControlBuffer.bg2cnt.priority = 1;
    gLCDControlBuffer.bg3cnt.priority = 3;

    ResetFaces();
    ResetText();
    ResetIconGraphics_();
    LoadUiFrameGraphics();
    LoadObjUIGfx();

    BG_SetPosition(0, 0, 0);
    BG_SetPosition(1, 0, 0);
    BG_SetPosition(2, 0, 0);

    LoadHelpBoxGfx((void *)0x06012000, -1);
    LoadIconPalettes(4);
    RestartMuralBackground();

    PutImg_PrepItemUseUnk(0x5000, 5);
    PutImg_PrepPopupWindow(0x800, 10);

    Decompress(gUnknown_08A1B9EC, gGenericBuffer);
    CallARM_FillTileRect(gBG1TilemapBuffer, gGenericBuffer, 0x1000);
    TileMap_FillRect(gBG1TilemapBuffer + (0x8 * 32), 14, 12, 0);
    Decompress(gUnknown_08A1BCC0, gGenericBuffer);
    CallARM_FillTileRect(gBG1TilemapBuffer, gGenericBuffer, 0x1000);

    BG_EnableSyncByMask(7);
    StartUiCursorHand(proc);
    ResetSysHandCursor(proc);
    DisplaySysHandCursorTextShadow(0x600, 1);

    gLCDControlBuffer.dispcnt.win0_on = 1;
    gLCDControlBuffer.dispcnt.win1_on = 0;
    gLCDControlBuffer.dispcnt.objWin_on = 0;
    gLCDControlBuffer.win0_left = 128;
    gLCDControlBuffer.win0_top = 40;
    gLCDControlBuffer.win0_right = 224;
    gLCDControlBuffer.win0_bottom = 152;
    gLCDControlBuffer.wincnt.win0_enableBg0 = 1;
    gLCDControlBuffer.wincnt.win0_enableBg1 = 1;
    gLCDControlBuffer.wincnt.win0_enableBg2 = 0;
    gLCDControlBuffer.wincnt.win0_enableBg3 = 1;
    gLCDControlBuffer.wincnt.win0_enableObj = 1;
    gLCDControlBuffer.wincnt.wout_enableBg0 = 1;
    gLCDControlBuffer.wincnt.wout_enableBg1 = 1;
    gLCDControlBuffer.wincnt.wout_enableBg2 = 0;
    gLCDControlBuffer.wincnt.wout_enableBg3 = 1;
    gLCDControlBuffer.wincnt.wout_enableObj = 1;

    SetBlendConfig(0, 0, 0, 8);
    StartGreenText(proc);
    StartHelpPromptSprite(195, 147, 9, proc);

    if (proc->unit == NULL && PrepGetUnitAmount() > 0)
        proc->unit = GetUnitFromPrepList(0);

    SkillSynth_InitTexts();
    SetPrimaryHBlankHandler(PrepItemSupply_OnHBlank);
    StartMenuScrollBarExt(proc, 225, 47, 0x5800, 9);
    sub_8097668();

    proc->firstIdx = -1;
    proc->secondIdx = -1;
    proc->state = SKILL_SYNTH_STATE_LIST;
    proc->confirmChoice = 0;

    SkillSynth_BuildScrollList(proc);

    StartSysBrownBox(0xd, 0xe00, 0xf, 0xc00, 0x400, proc);
    EnableSysBrownBox(1, 0x98, 6, 2);
    SetBlendConfig(1, 0xe, 4, 0);
    SetBlendTargetA(0, 0, 0, 0, 0);
    SetBlendTargetB(0, 0, 0, 1, 0);

    SkillSynth_DrawHeader();
    StartParallelWorker(SkillSynth_SpriteWorker, proc);
    SetDispEnable(1, 1, 0, 1, 1);
    SkillSynth_RedrawList(proc);
}

static void SkillSynth_ExitConfirm(struct SkillSynthListProc *proc)
{
    proc->state = SKILL_SYNTH_STATE_LIST;
    Proc_End(GetParallelWorker(PutGiveTakeBoxSprites));
    EndUiCursorHand();
    HideSysHandCursor();
    ShowSysHandCursor(proc->handX, proc->handY, 0xB, 0x800);
    ClearText(&PrepItemSuppyTexts.th[0]);
    PutDrawText(
        &PrepItemSuppyTexts.th[0],
        TILEMAP_LOCATED(gBG0TilemapBuffer, 6, 2),
        TEXT_COLOR_SYSTEM_WHITE,
        2,
        0,
        Utf8ToNarrowFonts(GetStringFromIndex(MSG_PREP_SCREEN_TITLE_SKILL_SYNTH))
    );
    SkillSynth_DrawPreview(proc);
}

static void SkillSynth_Loop_MainKeyHandler(struct SkillSynthListProc *proc)
{
    int idx;
    int cursorIdx;
    u16 sidA = 0;
    u16 sidB = 0;
    u16 resultSid;

    if (proc->state == SKILL_SYNTH_STATE_POPUP_WAIT) {
        if (!Proc_Find(ProcScr_Popup)) {
            proc->state = SKILL_SYNTH_STATE_LIST;
            ShowSysHandCursor(proc->handX, proc->handY, 0xB, 0x800);
        }
        return;
    }

    proc->currentPage = 0;
    idx = proc->idxPerPage[0];
    cursorIdx = proc->idxPerPage[0];

    if (proc->firstIdx >= 0)
        SkillSynth_GetSidFromIdx(proc->firstIdx, &sidA);
    else
        sidA = 0;

    if (proc->secondIdx >= 0)
        SkillSynth_GetSidFromIdx(proc->secondIdx, &sidB);
    else
        sidB = 0;

    resultSid = (proc->firstIdx >= 0 && proc->secondIdx >= 0)
        ? SkillSynth_LookupResult(sidA, sidB)
        : 0;

    if ((proc->yOffsetPerPage[proc->currentPage] & 0xf) == 0) {
        if ((proc->unk_36 == 0) || (proc->unk_36 == 0xff)) {
            if (gKeyStatusPtr->newKeys & R_BUTTON) {
                if (gUnknown_02012F56 == 0 || proc->state == SKILL_SYNTH_STATE_CONFIRM) {
                    PlaySoundEffect(SONG_6C);
                    return;
                }

                StartItemHelpBox(
                    0x80,
                    proc->idxPerPage[proc->currentPage] * 16 + 40 - proc->yOffsetPerPage[proc->currentPage],
                    gPrepScreenItemList[proc->idxPerPage[proc->currentPage]].item
                );
                proc->unk_36 = 1;
                return;
            }

            if (gKeyStatusPtr->newKeys & A_BUTTON) {
                if (gUnknown_02012F56 == 0) {
                    PlaySoundEffect(SONG_6C);
                    return;
                }

                if (proc->state == SKILL_SYNTH_STATE_CONFIRM) {
                    if (proc->confirmChoice == 0 && resultSid != 0) {
                        SkillSynth_PerformSynthesis(proc, resultSid);
                        proc->state = SKILL_SYNTH_STATE_POPUP_WAIT;
                        Proc_End(GetParallelWorker(PutGiveTakeBoxSprites));
                        EndUiCursorHand();
                        HideSysHandCursor();
                        return;
                    }

                    SkillSynth_ExitConfirm(proc);
                    PlaySoundEffect(SONG_SE_SYS_WINDOW_CANSEL1);
                    return;
                }

                if (proc->firstIdx < 0) {
                    proc->firstIdx = cursorIdx;
                    proc->handX = 0x80;
                    proc->handY = cursorIdx * 16 + 40 - proc->yOffsetPerPage[proc->currentPage];
                    SkillSynth_DrawPreview(proc);
                    PlaySoundEffect(SONG_SE_SYS_WINDOW_SELECT1);
                    return;
                }

                if (proc->secondIdx < 0) {
                    if (cursorIdx == proc->firstIdx) {
                        proc->firstIdx = -1;
                        SkillSynth_DrawPreview(proc);
                        PlaySoundEffect(SONG_SE_SYS_WINDOW_CANSEL1);
                        return;
                    }

                    proc->secondIdx = cursorIdx;
                    SkillSynth_GetSidFromIdx(proc->secondIdx, &sidB);
                    resultSid = SkillSynth_LookupResult(sidA, sidB);

                    if (resultSid == 0) {
                        PlaySoundEffect(SONG_6C);
                        SkillSynth_DrawPreview(proc);
                        return;
                    }

                    proc->state = SKILL_SYNTH_STATE_CONFIRM;
                    proc->confirmChoice = 0;
                    SkillSynth_DrawPreview(proc);
                    SkillSynth_SetupConfirmSprites(resultSid);
                    ClearText(&PrepItemSuppyTexts.th[0]);
                    PutDrawText(
                        &PrepItemSuppyTexts.th[0],
                        TILEMAP_LOCATED(gBG0TilemapBuffer, 6, 2),
                        TEXT_COLOR_SYSTEM_WHITE,
                        2,
                        0,
                        "Synthesize skill?"
                    );
                    StartParallelWorker(PutGiveTakeBoxSprites, proc);
                    EndUiCursorHand();
                    ShowSysHandCursor(68, 36, 0x4, 0x000);
                    PlaySoundEffect(SONG_SE_SYS_WINDOW_SELECT1);
                    return;
                }

                PlaySoundEffect(SONG_6C);
                return;
            }

            if (gKeyStatusPtr->newKeys & B_BUTTON) {
                if (proc->state == SKILL_SYNTH_STATE_CONFIRM) {
                    SkillSynth_ExitConfirm(proc);
                    PlaySoundEffect(SONG_SE_SYS_WINDOW_CANSEL1);
                    return;
                }

                if (proc->secondIdx >= 0) {
                    proc->secondIdx = -1;
                    SkillSynth_DrawPreview(proc);
                    PlaySoundEffect(SONG_SE_SYS_WINDOW_CANSEL1);
                    return;
                }

                if (proc->firstIdx >= 0) {
                    proc->firstIdx = -1;
                    SkillSynth_DrawPreview(proc);
                    PlaySoundEffect(SONG_SE_SYS_WINDOW_CANSEL1);
                    return;
                }

                SetPrimaryHBlankHandler(NULL);
                Proc_Goto(proc, PL_SKILL_SYNTH_PRESS_B);
                PlaySoundEffect(SONG_SE_SYS_WINDOW_CANSEL1);
                proc->unk_36 = 0;
                return;
            }

            if (proc->state == SKILL_SYNTH_STATE_CONFIRM &&
                (gKeyStatusPtr->newKeys & (DPAD_UP | DPAD_DOWN))) {
                proc->confirmChoice ^= 1;
                PlaySoundEffect(SONG_SE_SYS_CURSOR_UD1);
                ShowSysHandCursor(68, proc->confirmChoice == 0 ? 36 : 52, 0x4, 0x000);
                return;
            }

            proc->scrollAmount = 16;

            if ((gKeyStatusPtr->repeatedKeys & DPAD_UP) ||
                ((gKeyStatusPtr->heldKeys & DPAD_UP) && (gKeyStatusPtr->heldKeys & L_BUTTON))) {
                if (proc->idxPerPage[0] != 0)
                    proc->idxPerPage[0]--;
            }

            if ((gKeyStatusPtr->repeatedKeys & DPAD_DOWN) ||
                ((gKeyStatusPtr->heldKeys & DPAD_DOWN) && (gKeyStatusPtr->heldKeys & L_BUTTON))) {
                if (proc->idxPerPage[0] < gUnknown_02012F56 - 1)
                    proc->idxPerPage[0]++;
            }
        } else if (gKeyStatusPtr->newKeys & (R_BUTTON | B_BUTTON)) {
            CloseHelpBox();
            proc->unk_36 = 0;
            return;
        }
    }

    if (idx != proc->idxPerPage[0] && proc->state != SKILL_SYNTH_STATE_CONFIRM) {
        u16 item = gPrepScreenItemList[proc->idxPerPage[0]].item;

        PlaySoundEffect(SONG_SE_SYS_CURSOR_UD1);
        SkillSynth_EnsureCursorVisible(proc);
        SkillSynth_RedrawList(proc);
        SkillSynth_PlaceCursor(proc);

        if (proc->unk_36 != 0)
            StartItemHelpBox(0x80, proc->handY, item);
    }
}

static void SkillSynth_OnEnd(struct SkillSynthListProc *proc)
{
    EndAllParallelWorkers();
    EndAllProcChildren(proc);
    EndFaceById(0);
    EndMuralBackground_();
    ClearBg0Bg1();
    SetPrimaryHBlankHandler(NULL);

    if (proc->fromWorldMap) {
        SkillSynth_ResumeWorldMapGfx(proc);
        gGMData.units[0].id = gSavedWorldMapUnitId;
        gGMData.sprite_disp = 1;
        gGMData.xCamera = gSavedWorldMapXCoordiate;
        gGMData.yCamera = gSavedWorldMapYCoordiate;
        SetDefaultColorEffects();
        returnToWorldMap_External();
        return;
    }

    {
        struct ProcAtMenu *pproc = proc->proc_parent;

        pproc->state = 1;
    }
}

static void SkillSynth_WmEntryAfterFade(struct SkillSynthListProc *proc)
{
    SetDispEnable(0, 0, 0, 0, 0);
    gGMData.sprite_disp = 0;
    HideGmUnit(-1);
    ClearSprites();
    ResetUnitSprites();
    CpuFastFill16(0, (void *)0x06010000, 0x5FE0);
    gGMData.xCamera = 0;
    gGMData.yCamera = 0;
    proc->fromWorldMap = true;
    SkillSynth_PauseWorldMapGfx(proc);
}

struct ProcCmd const ProcScr_PrepItemListScreen_SKILL_SYNTH[] = {
    PROC_NAME("PrepItemListScreen_SKILL_SYNTH"),
    PROC_YIELD,
    PROC_SET_END_CB(SkillSynth_OnEnd),

PROC_LABEL(PL_SKILL_SYNTH_INIT),
    PROC_CALL(PrepItemList_Init),
    PROC_CALL(SkillSynth_InitGfx),
    PROC_CALL_ARG(NewFadeIn, 0x10),
    PROC_WHILE(FadeInExists),

PROC_LABEL(PL_SKILL_SYNTH_SHOW_CURSOR),
    PROC_CALL(SkillSynth_ShowCursor),

PROC_LABEL(PL_SKILL_SYNTH_IDLE),
    PROC_REPEAT(SkillSynth_Loop_MainKeyHandler),

PROC_LABEL(PL_SKILL_SYNTH_PRESS_B),
    PROC_CALL_ARG(NewFadeOut, 0x10),
    PROC_WHILE(FadeOutExists),

PROC_LABEL(PL_SKILL_SYNTH_END),
    PROC_END,

PROC_LABEL(PL_SKILL_SYNTH_WM_ENTRY),
    PROC_CALL_ARG(NewFadeOut, 0x10),
    PROC_WHILE(FadeOutExists),
    PROC_CALL(SkillSynth_WmEntryAfterFade),
    PROC_GOTO(PL_SKILL_SYNTH_INIT),
};

void StartSkillSynthScreen_FromPrep(struct ProcAtMenu *pproc)
{
    struct SkillSynthListProc *proc;

    proc = Proc_StartBlocking(ProcScr_PrepItemListScreen_SKILL_SYNTH, pproc);
    proc->fromWorldMap = false;
    proc->wmGfxPaused = false;
}

void StartSkillSynthScreen_FromWorldMap(void)
{
    struct SkillSynthListProc *proc;

    gSavedWorldMapUnitId = gGMData.units[0].id;
    gSavedWorldMapXCoordiate = gGMData.xCamera;
    gSavedWorldMapYCoordiate = gGMData.yCamera;
    gGMData.sprite_disp = 0;
    HideGmUnit(-1);

    MakePrepUnitList();
    proc = Proc_StartBlocking(ProcScr_PrepItemListScreen_SKILL_SYNTH, Proc_Find(ProcScr_WorldMapMain));
    proc->fromWorldMap = true;
    proc->wmGfxPaused = false;
    Proc_Goto(proc, PL_SKILL_SYNTH_WM_ENTRY);
}
