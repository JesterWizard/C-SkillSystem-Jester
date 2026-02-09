#include "common-chax.h"
#include "utf8.h"
#include "kernel-lib.h"
#include "constants/texts.h"
#include "popup.h"
#include "prep-skill.h"
#include "jester_headers/custom-functions.h"
#include "jester_headers/custom-structs.h"

#define BEXP_VISIBLE_COUNT 5

static void DrawUnits_BEXP(int x, int y)
{
    int i;
    struct Unit *unit;
    int unitCount = PrepGetUnitAmount();

    // Clear old unit sprites
    ClearSprites();

    for (i = 0; i < BEXP_VISIBLE_COUNT; i++) {
        int unitIndex = gBEXP[3] + i;

        // Clear the text for this slot
        ClearText(&gPrepUnitTexts[i + 5]);

        // Stop if we've run out of units
        if (unitIndex >= unitCount)
            continue;

        unit = GetUnitFromPrepList(unitIndex);

        // Draw sprite at visual position 'i' (not unitIndex)
        PutUnitSprite(
            0,
            ((x - 1) * 8) + 4,
            ((y + (i * 2)) * 8) + 4,
            unit
        );

        // Draw name at visual position 'i' (not unitIndex)
        PutDrawText(
            &gPrepUnitTexts[i + 5],
            TILEMAP_LOCATED(gBG0TilemapBuffer, x + 2, y + (i * 2)),
            TEXT_COLOR_SYSTEM_WHITE,
            0,
            0,
            GetStringFromIndex(unit->pCharacterData->nameTextId)
        );
    }

    RefreshUnitSprites();
    SyncUnitSpriteSheet();
    BG_EnableSyncByMask(BG0_SYNC_BIT);
}

/* X and Y are tilemap coordinates (8x8) */
/* Draw only the minimug and level/exp - called when cursor moves */
static void DrawUnitMinimugAndLevel(struct Unit *unit, int x, int y)
{
    // Clear only the minimug and level/exp area (rows y to y+3)
    TileMap_FillRect(TILEMAP_LOCATED(gBG0TilemapBuffer, x, y), 4, 4, 0);
    
    PutFaceChibi(GetUnitPortraitId(unit), TILEMAP_LOCATED(gBG0TilemapBuffer, x, y), 0x270, 2, 0);

    PutTwoSpecialChar(TILEMAP_LOCATED(gBG0TilemapBuffer, x + 4, y + 2), TEXT_COLOR_SYSTEM_GOLD, TEXT_SPECIAL_LV_A, TEXT_SPECIAL_LV_B);
    PutNumberOrBlank(TILEMAP_LOCATED(gBG0TilemapBuffer, x + 7, y + 2), TEXT_COLOR_SYSTEM_BLUE, unit->level);
    
    PutSpecialChar(TILEMAP_LOCATED(gBG0TilemapBuffer, x + 8, y + 2), TEXT_COLOR_SYSTEM_GOLD, TEXT_SPECIAL_E);
    PutNumberOrBlank(TILEMAP_LOCATED(gBG0TilemapBuffer, x + 11, y + 2), TEXT_COLOR_SYSTEM_BLUE, unit->exp);

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
    ShowSysHandCursor(12, 66, 0, 0x800);

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

    char * experience_string = GetStringFromIndex(MSG_PREP_SCREEN_TITLE_BEXP);
    int leftPosition = ((8 * ITEM_PANEL_LEFT_Y) - GetStringTextLen(experience_string)) / 2;
    PutDrawText(NULL, gBG1TilemapBuffer + TILEMAP_INDEX(0, 0), 0, leftPosition, ITEM_PANEL_LEFT_Y, experience_string);
    SetBlendConfig(1, 0xe, 4, 0);
    SetBlendTargetA(0, 0, 0, 0, 0);
    SetBlendTargetB(0, 0, 0, 1, 0);

    InitText(&PrepItemSuppyTexts.th[0], 22);
    PutDrawText(&PrepItemSuppyTexts.th[0], TILEMAP_LOCATED(gBG0TilemapBuffer, 3, 3), TEXT_COLOR_SYSTEM_WHITE, 8, 0, Utf8ToNarrowFonts(GetStringFromIndex(MSG_BEXP_INSTRUCTION)));
    
    // Initialize text buffers for unit names
    for (i = 0; i < BEXP_VISIBLE_COUNT; i++) {
        InitText(&gPrepUnitTexts[i + 5], 10);
        ClearText(&gPrepUnitTexts[i + 5]);
    }

    // Initialize scroll position and cursor
    gBEXP[3] = 0;  // First visible unit index
    proc->list_num_cur = 0;  // Current cursor position

    ApplyUnitSpritePalettes();

    // Draw static BEXP UI elements (these don't change) - DRAWN ONCE
    gBEXP[1] = 1000;
    int bexp_color = gBEXP[1] == 1000 ? TEXT_COLOR_SYSTEM_GREEN : TEXT_COLOR_SYSTEM_WHITE;

    InitText(&PrepItemSuppyTexts.th[1], 10);
    PutDrawText(&PrepItemSuppyTexts.th[1], TILEMAP_LOCATED(gBG0TilemapBuffer, 15, 14), TEXT_COLOR_SYSTEM_GOLD, 0, 0, Utf8ToNarrowFonts(GetStringFromIndex(MSG_BEXP_AMOUNT_TITLE)));
    PutNumber(TILEMAP_LOCATED(gBG0TilemapBuffer, 26, 14), bexp_color, 1000);

    // Multiplier label and value (next to minimug)
    InitText(&PrepItemSuppyTexts.th[2], 6);
    PutDrawText(&PrepItemSuppyTexts.th[2], TILEMAP_LOCATED(gBG0TilemapBuffer, 19, 8), TEXT_COLOR_SYSTEM_GOLD, 0, 0, Utf8ToNarrowFonts(GetStringFromIndex(MSG_BEXP_MULTIPLIER_TITLE)));

    InitText(&PrepItemSuppyTexts.th[3], 4);
    PutDrawText(&PrepItemSuppyTexts.th[3], TILEMAP_LOCATED(gBG0TilemapBuffer, 24, 8), TEXT_COLOR_SYSTEM_GRAY, 0, 0, Utf8ToNarrowFonts(GetStringFromIndex(MSG_BEXP_MULTIPLIER_1_00)));

    // Applied EXP label and value
    InitText(&PrepItemSuppyTexts.th[4], 10);
    PutDrawText(&PrepItemSuppyTexts.th[4], TILEMAP_LOCATED(gBG0TilemapBuffer, 15, 16), TEXT_COLOR_SYSTEM_GOLD, 0, 0, Utf8ToNarrowFonts(GetStringFromIndex(MSG_BEXP_APPLIED)));
    PutNumber(TILEMAP_LOCATED(gBG0TilemapBuffer, 24, 16), TEXT_COLOR_SYSTEM_WHITE, 50);

    // Draw initial unit list and minimug
    DrawUnits_BEXP(3, 8);
    DrawUnitMinimugAndLevel(GetUnitFromPrepList(0), 15, 8);

    StartSysBrownBox(0xd, 0xe00, 0xf, 0xc00, 0x400, proc);
    EnableSysBrownBox(0, -20, -1, 1);
}

static void PrepLoop_MainKeyHandler_BEXP(struct ProcPrepUnit * proc)
{
    int unitCount = PrepGetUnitAmount();

    // Redraw units every frame (handles scrolling)
    DrawUnits_BEXP(3, 8);

    if (gKeyStatusPtr->newKeys & B_BUTTON) {
        SetPrimaryHBlankHandler(NULL);
        Proc_Goto(proc, PL_BEXP_PRESS_B);
        PlaySoundEffect(SONG_SE_SYS_WINDOW_CANSEL1);
        return;
    }

    if (gKeyStatusPtr->newKeys & DPAD_UP) { 
        if (proc->list_num_cur > 0) {
            proc->list_num_cur--;
            
            // Scroll window up if cursor moves above visible area
            if (proc->list_num_cur < gBEXP[3]) {
                gBEXP[3]--;
            }
            
            // Update cursor position (visual position relative to scroll)
            ShowSysHandCursor(12, 64 + ((proc->list_num_cur - gBEXP[3]) * 16), 0, 0x800);
            
            // Update minimug for new unit
            DrawUnitMinimugAndLevel(GetUnitFromPrepList(proc->list_num_cur), 15, 8);
            PlaySoundEffect(SONG_SE_SYS_CURSOR_UD1);
        }
        return;
    }

    if (gKeyStatusPtr->newKeys & DPAD_DOWN) { 
        if (proc->list_num_cur < unitCount - 1) {
            proc->list_num_cur++;
            
            // Scroll window down if cursor moves below visible area
            if (proc->list_num_cur >= gBEXP[3] + BEXP_VISIBLE_COUNT) {
                gBEXP[3]++;
            }
            
            // Update cursor position (visual position relative to scroll)
            ShowSysHandCursor(12, 64 + ((proc->list_num_cur - gBEXP[3]) * 16), 0, 0x800);
            
            // Update minimug for new unit
            DrawUnitMinimugAndLevel(GetUnitFromPrepList(proc->list_num_cur), 15, 8);
            PlaySoundEffect(SONG_SE_SYS_CURSOR_UD1);
        }
        return;
    }

    if (gKeyStatusPtr->newKeys & R_BUTTON) {
        SetPrimaryHBlankHandler(NULL);
        PlaySoundEffect(SONG_SE_SYS_WINDOW_SELECT1);
        Proc_Goto(proc, PL_BEXP_PRESS_R);
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
    PROC_NAME("PrepItemListScreen_BEXP"),
    PROC_YIELD,
    PROC_SET_END_CB(PrepItemList_OnEnd_BEXP),

PROC_LABEL(PL_BEXP_INIT),
    PROC_CALL(PrepInitGfx_BEXP),
	PROC_CALL_ARG(NewFadeIn, 0x10),
    PROC_WHILE(FadeInExists),

PROC_LABEL(PL_BEXP_IDLE),
    PROC_REPEAT(PrepLoop_MainKeyHandler_BEXP),

PROC_LABEL(PL_BEXP_REFRESH_VIEW),
    PROC_CALL_ARG(NewFadeOut, 0x10),
    PROC_WHILE(FadeOutExists),
    PROC_CALL(PrepItemList_OnEnd_BEXP),
    PROC_SLEEP(0),
    PROC_GOTO(PL_BEXP_IDLE),

PROC_LABEL(PL_BEXP_PRESS_R),
    PROC_CALL(PrepUnitDisableDisp),
    PROC_SLEEP(0x2),
    PROC_CALL(sub_809B014),
    PROC_CALL(sub_809B504),
    PROC_YIELD,
    PROC_CALL(sub_809B520),
    PROC_CALL(ProcPrepUnit_InitScreen),
    PROC_SLEEP(0x2),
    PROC_CALL(PrepUnitEnableDisp),

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