#include "common-chax.h"
#include "utf8.h"
#include "kernel-lib.h"
#include "constants/texts.h"
#include "popup.h"
#include "prep-skill.h"
#include "jester_headers/custom-functions.h"
#include "jester_headers/custom-structs.h"

static void DrawUnits_BEXP(struct ProcPrepUnit *proc, int x, int y)
{
    struct Unit *unit;

    for (int i = 0; i < 5; i++) 
    {
        unit = GetUnitFromPrepList(i);

        PutUnitSprite(0, ((x - 1) * 8), ((y + (i * 2)) * 8), unit);
        PutDrawText(&gPrepUnitTexts[i+4], TILEMAP_LOCATED(gBG0TilemapBuffer, (x + 2), y + (i * 2)), TEXT_COLOR_SYSTEM_WHITE, 0, 0, GetStringFromIndex(unit->pCharacterData->nameTextId));
    }

    RefreshUnitSprites();
    SyncUnitSpriteSheet();
    BG_EnableSyncByMask(BG0_SYNC_BIT); // With this the unit name text will display
}


/* X and Y are tilemap coordinates (8x8) */
static void DrawUnitMinimugAndLevel(struct Unit *unit, int x, int y)
{
    TileMap_FillRect(TILEMAP_LOCATED(gBG0TilemapBuffer, x + 4, x + 2), x + 5, y, 0);
    PutFaceChibi(GetUnitPortraitId(unit), TILEMAP_LOCATED(gBG0TilemapBuffer, x, y), 0x270, 2, 0);
    ClearText(&gPrepUnitTexts[0x13]);
    PutDrawText(
        &gPrepUnitTexts[0x13],
        TILEMAP_LOCATED(gBG0TilemapBuffer, x + 4, y),
        TEXT_COLOR_SYSTEM_WHITE,
        GetStringTextCenteredPos(0x38, GetStringFromIndex(unit->pCharacterData->nameTextId)),
        0,
        GetStringFromIndex(unit->pCharacterData->nameTextId)
    );

    PutTwoSpecialChar(TILEMAP_LOCATED(gBG0TilemapBuffer, x + 4, y + 2), TEXT_COLOR_SYSTEM_GOLD, TEXT_SPECIAL_LV_A, TEXT_SPECIAL_LV_B);
    PutSpecialChar(TILEMAP_LOCATED(gBG0TilemapBuffer, x + 8, y + 2), TEXT_COLOR_SYSTEM_GOLD, TEXT_SPECIAL_E);

    PutNumberOrBlank(TILEMAP_LOCATED(gBG0TilemapBuffer, x + 7, y + 2), TEXT_COLOR_SYSTEM_BLUE, unit->level);
    PutNumberOrBlank(TILEMAP_LOCATED(gBG0TilemapBuffer, x + 10, y + 2), TEXT_COLOR_SYSTEM_BLUE, unit->exp);

    InitText(&PrepItemSuppyTexts.th[2], 6);
    PutDrawText(&PrepItemSuppyTexts.th[2], TILEMAP_LOCATED(gBG0TilemapBuffer, 19, 8), TEXT_COLOR_SYSTEM_GOLD, 8, 0, Utf8ToNarrowFonts(GetStringFromIndex(MSG_BEXP_MULTIPLIER)));
    PutNumber(TILEMAP_LOCATED(gBG0TilemapBuffer, 25, 8), TEXT_COLOR_SYSTEM_GRAY, 1);

    InitText(&PrepItemSuppyTexts.th[3], 10);
    PutDrawText(&PrepItemSuppyTexts.th[3], TILEMAP_LOCATED(gBG0TilemapBuffer, 14, 16), TEXT_COLOR_SYSTEM_GOLD, 8, 0, Utf8ToNarrowFonts(GetStringFromIndex(MSG_BEXP_APPLIED)));
    PutNumber(TILEMAP_LOCATED(gBG0TilemapBuffer, 25, 16), TEXT_COLOR_SYSTEM_WHITE, 50);

    BG_EnableSyncByMask(BG0_SYNC_BIT);
}

static void PrepInitGfx_BEXP(struct ProcPrepUnit * proc)
{
    int i;

    gLCDControlBuffer.dispcnt.mode = 0;
    SetupBackgrounds(NULL);

    BG_Fill(BG_GetMapBuffer(0), 0);
    BG_Fill(BG_GetMapBuffer(1), 0);
    BG_Fill(BG_GetMapBuffer(2), 0);

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

    LoadHelpBoxGfx((void*)0x06012000, -1);
    LoadIconPalettes(4);

    RestartMuralBackground();

    /* Draws the left side frame that will hold the unit list */
    DrawUiFrame2(1, 7, 12, 13, 0);

    /* Draw the right side frame to hold the minimug */
    DrawUiFrame2(13, 7, 16, 6, 0);

    /* Draw the right side frame to hold the BEXP adding */
    DrawUiFrame2(13, 13, 16, 7, 0);

    BG_EnableSyncByMask(7);
    StartUiCursorHand(proc);
    ResetSysHandCursor(proc);
    DisplaySysHandCursorTextShadow(0x600, 1);
    ShowSysHandCursor(10, 64, 0, 0x800);

    gLCDControlBuffer.dispcnt.win0_on = 1;
    gLCDControlBuffer.dispcnt.win1_on = 0;
    gLCDControlBuffer.dispcnt.objWin_on = 0;

    gLCDControlBuffer.win0_left = 128;
    gLCDControlBuffer.win0_top = 40;
    gLCDControlBuffer.win0_right = 224;
    gLCDControlBuffer.win0_bottom = 152;

    gLCDControlBuffer.wincnt.win0_enableBg0 = 1;
    gLCDControlBuffer.wincnt.win0_enableBg1 = 1;
    gLCDControlBuffer.wincnt.win0_enableBg2 = 1;
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
    InitText(PrepItemSuppyTexts.th + 0, 6);
    InitText(PrepItemSuppyTexts.th + 1, 5);
    InitText(PrepItemSuppyTexts.th + 15, 4);

    for (i = 0; i < 8; i++) {
        InitTextDb(PrepItemSuppyTexts.th + 7 + i, 7);
    }

    /* Display the transparent black banner behind the text */
    SetPrimaryHBlankHandler(PrepItemSupply_OnHBlank);
    StartMenuScrollBarExt(proc, 225, 47, 0x5800, 9);
    UpdateMenuScrollBarConfig(0xA, proc->yDiff_cur, (PrepGetUnitAmount() - 1) / 2 + 1, 6);
    sub_8097668();
    BG_EnableSyncByMask(4);
    // StartUiSpinningArrows(proc);
    // LoadUiSpinningArrowGfx(0, 0x280, 2);
    // SetUiSpinningArrowPositions(0x78, 0x18, 0xe9, 0x18);
    // SetUiSpinningArrowConfig(3);
    // StartParallelWorker(List_PutHighlightedCategorySprites_INFUSE, proc);
    char * experience_string = GetStringFromIndex(MSG_PREP_SCREEN_TITLE_BEXP);
    int leftPosition = ((8 * ITEM_PANEL_LEFT_Y) - GetStringTextLen(experience_string)) / 2;
    PutDrawText(NULL, gBG1TilemapBuffer + TILEMAP_INDEX(0, 0), 0, leftPosition, ITEM_PANEL_LEFT_Y, experience_string);
    SetBlendConfig(1, 0xe, 4, 0);
    SetBlendTargetA(0, 0, 0, 0, 0);
    SetBlendTargetB(0, 0, 0, 1, 0);

  //  PrepUnit_DrawLeftUnitName(GetUnitFromPrepList(proc->list_num_cur));
    DrawUnitMinimugAndLevel(GetUnitFromPrepList(proc->list_num_cur), 15, 8);

    // for (i = 0; i < 6; i++)
    //     PrepUnit_DrawUnitListNames(proc, proc->yDiff_cur / 0x10 + i);

    InitText(&PrepItemSuppyTexts.th[0], 22);
    PutDrawText(&PrepItemSuppyTexts.th[0], TILEMAP_LOCATED(gBG0TilemapBuffer, 3, 3), TEXT_COLOR_SYSTEM_WHITE, 8, 0, Utf8ToNarrowFonts(GetStringFromIndex(MSG_BEXP_INSTRUCTION)));
    
    InitText(&PrepItemSuppyTexts.th[1], 9);
    PutDrawText(&PrepItemSuppyTexts.th[1], TILEMAP_LOCATED(gBG0TilemapBuffer, 14, 14), TEXT_COLOR_SYSTEM_GOLD, 8, 0, Utf8ToNarrowFonts(GetStringFromIndex(MSG_BEXP_AMOUNT_TITLE)));
    
    
    gBEXP = 1000;
    int bexp_color = gBEXP == 1000 ? TEXT_COLOR_SYSTEM_GREEN : TEXT_COLOR_SYSTEM_WHITE;

    PutNumber(TILEMAP_LOCATED(gBG0TilemapBuffer, 27, 14), bexp_color, gBEXP);

    InitText(&gPrepUnitTexts[4], 10);
    InitText(&gPrepUnitTexts[5], 10);
    InitText(&gPrepUnitTexts[6], 10);
    InitText(&gPrepUnitTexts[7], 10);
    InitText(&gPrepUnitTexts[8], 10);

    ClearText(&gPrepUnitTexts[4]);
    ClearText(&gPrepUnitTexts[5]);
    ClearText(&gPrepUnitTexts[6]);
    ClearText(&gPrepUnitTexts[7]);
    ClearText(&gPrepUnitTexts[8]);

    ApplyUnitSpritePalettes();

    StartSysBrownBox(0xd, 0xe00, 0xf, 0xc00, 0x400, proc);
    EnableSysBrownBox(0, -20, -1, 1);
    // CpuFastFill(0, PAL_OBJ(0x0B), 0x20);
    // ForceSyncUnitSpriteSheet();
}

static void PrepLoop_MainKeyHandler_BEXP(struct ProcPrepUnit * proc)
{
  //  int idx = proc->idxPerPage[proc->currentPage];
    // int i;
    // struct Unit *unit;

    // for (i = 0; i < PrepGetUnitAmount(); i++) {
    //     unit = GetUnitFromPrepList(i);

    //     ClearText(&gPrepUnitTexts[i]);
    //     Text_DrawString(&gPrepUnitTexts[i],  GetStringFromIndex(unit->pCharacterData->nameTextId));
    //     PutText(&gPrepUnitTexts[i], gBG0TilemapBuffer + TILEMAP_INDEX(2, 5 + i));
    // }

    DrawUnits_BEXP(proc, 3, 8);

    if (gKeyStatusPtr->newKeys & B_BUTTON) {
        SetPrimaryHBlankHandler(NULL); // Remove black banner
        Proc_Goto(proc, PL_BEXP_PRESS_B);
        PlaySoundEffect(SONG_SE_SYS_WINDOW_CANSEL1);
        return;
    }

    return;
}

static void PrepItemList_OnEnd_BEXP(struct ProcPrepUnit * proc)
{
    struct ProcAtMenu *pproc = proc->proc_parent;
    pproc->state = 1;

    EndAllParallelWorkers();
    EndAllProcChildren(proc);
    EndFaceById(0);
    EndMuralBackground_();
    ClearBg0Bg1();
}

struct ProcCmd const ProcScr_PrepItemListScreen_BEXP[] = {
    PROC_NAME("PrepItemListScreen_INFUSE"),
    PROC_YIELD,
    PROC_SET_END_CB(PrepItemList_OnEnd_BEXP),

PROC_LABEL(PL_BEXP_INIT),
    PROC_CALL(PrepInitGfx_BEXP),
	PROC_CALL_ARG(NewFadeIn, 0x10),
    PROC_WHILE(FadeInExists),

// PROC_LABEL(PL_BEXP_SHOW_CURSOR),
//     PROC_CALL(sub_809F5F4),

PROC_LABEL(PL_BEXP_IDLE),
    PROC_REPEAT(PrepLoop_MainKeyHandler_BEXP),

PROC_LABEL(PL_BEXP_REFRESH_VIEW),
    PROC_CALL_ARG(NewFadeOut, 0x10),
    PROC_WHILE(FadeOutExists),
    PROC_CALL(PrepItemList_OnEnd_BEXP),
    PROC_SLEEP(0),
    PROC_GOTO(PL_BEXP_IDLE),

PROC_LABEL(PL_BEXP_PRESS_B),
    PROC_CALL_ARG(NewFadeOut, 0x10),
    PROC_WHILE(FadeOutExists),

PROC_LABEL(PL_BEXP_END),
    PROC_END
};

void StartBEXPScreen_FromPrep(struct ProcAtMenu *pproc)
{
    Proc_StartBlocking(ProcScr_PrepItemListScreen_BEXP, pproc);
}