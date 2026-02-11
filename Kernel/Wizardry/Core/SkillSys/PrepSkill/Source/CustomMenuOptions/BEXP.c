#include "common-chax.h"
#include "utf8.h"
#include "kernel-lib.h"
#include "constants/texts.h"
#include "popup.h"
#include "prep-skill.h"
#include "jester_headers/custom-functions.h"
#include "jester_headers/custom-structs.h"
#include "jester_headers/custom-arrays.h"

#define BEXP_VISIBLE_COUNT 5

static int LevelUpProcExists(struct ProcPrepUnit* proc) {
    return Proc_Find(ProcScr_ManimLevelUp) || Proc_Find(ProcScr_ManimLevelUp_UnitComment);
}

// Only draws the sprites (Face/Class icons). Run this every frame.
static void DrawUnitSprites_BEXP(int x, int y)
{
    int i;
    struct Unit *unit;
    int unitCount = PrepGetUnitAmount();

    // Clear old unit sprites (OAM)
    ClearSprites();

    for (i = 0; i < BEXP_VISIBLE_COUNT; i++) {
        int unitIndex = gBEXP_TopVisibleIndex + i;

        if (unitIndex >= unitCount)
            continue;

        unit = GetUnitFromPrepList(unitIndex);

        // Draw sprite at visual position 'i'
        PutUnitSprite(0, ((x - 1) * 8) + 4, ((y + (i * 2)) * 8) + 4, unit);
    }
    
    // Push OAM to GPU
    RefreshUnitSprites();
    SyncUnitSpriteSheet();
}

// Only draws the text names. Run this ONLY when scrolling.
static void DrawUnitText_BEXP(int x, int y)
{
    int i;
    struct Unit *unit;
    int unitCount = PrepGetUnitAmount();

    for (i = 0; i < BEXP_VISIBLE_COUNT; i++) {
        int unitIndex = gBEXP_TopVisibleIndex + i;

        // Clear the text handle for this slot to prevent ghosting
        ClearText(&gPrepUnitTexts[i + 5]);

        if (unitIndex >= unitCount)
            continue;

        unit = GetUnitFromPrepList(unitIndex);

        // Draw name at visual position 'i'
        PutDrawText(&gPrepUnitTexts[i + 5], TILEMAP_LOCATED(gBG0TilemapBuffer, x + 2, y + (i * 2)), TEXT_COLOR_SYSTEM_WHITE, 0, 0, GetStringFromIndex(unit->pCharacterData->nameTextId));
    }
    
    // Enable BG0 Sync to show the new text
    BG_EnableSyncByMask(BG0_SYNC_BIT);
}

/* X and Y are tilemap coordinates (8x8) */
/* Draw only the minimug and level/exp - called when cursor moves */
static void DrawUnitMinimugAndLevel(struct Unit *unit, int x, int y)
{
    // Strangely the unit's EXP value doesn't update on the background if I don't clear the area it occupies beforehand
    TileMap_FillRect(TILEMAP_LOCATED(gBG0TilemapBuffer, 0x19, 0xA), 2, 2, 0);
    
    PutFaceChibi(GetUnitPortraitId(unit), TILEMAP_LOCATED(gBG0TilemapBuffer, x, y), 0x2A0, 2, 0);

    PutTwoSpecialChar(TILEMAP_LOCATED(gBG0TilemapBuffer, x + 4, y + 2), TEXT_COLOR_SYSTEM_GOLD, TEXT_SPECIAL_LV_A, TEXT_SPECIAL_LV_B);
    PutNumberOrBlank(TILEMAP_LOCATED(gBG0TilemapBuffer, x + 7, y + 2), TEXT_COLOR_SYSTEM_BLUE, unit->level);
    
    PutSpecialChar(TILEMAP_LOCATED(gBG0TilemapBuffer, x + 8, y + 2), TEXT_COLOR_SYSTEM_GOLD, TEXT_SPECIAL_E);
    TileMap_FillRect(TILEMAP_LOCATED(gBG0TilemapBuffer, 26, 10), 3, 2, 0);

    /* Change the text color of the unit's EXP if they've hit 100 */
    int exp_color = ((unit->exp + gBEXP_Applied) == 100) ? TEXT_COLOR_SYSTEM_GREEN : TEXT_COLOR_SYSTEM_BLUE;

    PutNumberOrBlank(TILEMAP_LOCATED(gBG0TilemapBuffer, x + 11, y + 2), exp_color, unit->exp + gBEXP_Applied);

    int multiplier_index = 0;
    int unit_level = unit->level;

    if (UNIT_CATTRIBUTES(unit) & CA_PROMOTED)
        unit_level += 20;

    if (unit_level < 4)
        multiplier_index = MSG_BEXP_MULTIPLIER_1_00;
    else if (unit_level < 8)
        multiplier_index = MSG_BEXP_MULTIPLIER_1_25;
    else if (unit_level < 12)
        multiplier_index = MSG_BEXP_MULTIPLIER_1_50;
    else if (unit_level < 16)
        multiplier_index = MSG_BEXP_MULTIPLIER_1_75;
    else if (unit_level < 20)
        multiplier_index = MSG_BEXP_MULTIPLIER_2_00;
    else if (unit_level < 24)
        multiplier_index = MSG_BEXP_MULTIPLIER_2_25;
    else if (unit_level < 28)
        multiplier_index = MSG_BEXP_MULTIPLIER_2_50;
    else if (unit_level < 32)
        multiplier_index = MSG_BEXP_MULTIPLIER_2_75;
    else if (unit_level < 36)
        multiplier_index = MSG_BEXP_MULTIPLIER_3_00;

    ClearText(&PrepItemSuppyTexts.th[3]);
    PutDrawText(&PrepItemSuppyTexts.th[3], TILEMAP_LOCATED(gBG0TilemapBuffer, 24, 8), TEXT_COLOR_SYSTEM_GRAY, 0, 0, Utf8ToNarrowFonts(GetStringFromIndex(multiplier_index)));

    BG_EnableSyncByMask(BG0_SYNC_BIT);
}

/* X and Y are tilemap coordinates (8x8) */
/* Draw only the minimug and level/exp - called when cursor moves */
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
    ShowSysHandCursor(14, 66, 0x8, 0x800);

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

    StartMenuScrollBar(proc); 
    PutMenuScrollBarAt(2, 66); 
    InitMenuScrollBarImg(0x7A60, 2); 

    /* Initial configuration to set the bar size/pos */
    UpdateMenuScrollBarConfig(
        PrepGetUnitAmount(),        // Total Height (max items)
        gBEXP_TopVisibleIndex * 16, // Current Top Pixel (Index * 16px per row)
        PrepGetUnitAmount(),        // Total Rows
        BEXP_VISIBLE_COUNT          // Visible Rows
    );

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
    PutDrawText(&PrepItemSuppyTexts.th[0], TILEMAP_LOCATED(gBG2TilemapBuffer, 3, 3), TEXT_COLOR_SYSTEM_WHITE, 8, 0, Utf8ToNarrowFonts(GetStringFromIndex(MSG_BEXP_INSTRUCTION)));
    
    // Initialize text buffers for unit names
    for (i = 0; i < BEXP_VISIBLE_COUNT; i++) {
        InitText(&gPrepUnitTexts[i + 5], 10);
        ClearText(&gPrepUnitTexts[i + 5]);
    }

    ApplyUnitSpritePalettes();

    // Draw static BEXP UI elements (these don't change) - DRAWN ONCE
    gBEXP_Total = 1000;
    int bexp_color = gBEXP_Total == 1000 ? TEXT_COLOR_SYSTEM_GREEN : TEXT_COLOR_SYSTEM_WHITE;

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
    PutDrawText(&PrepItemSuppyTexts.th[4], TILEMAP_LOCATED(gBG0TilemapBuffer, 15, 16), TEXT_COLOR_SYSTEM_GOLD, 0, 0, Utf8ToNarrowFonts(GetStringFromIndex(MSG_BEXP_APPLIED_TITLE)));
    PutNumber(TILEMAP_LOCATED(gBG0TilemapBuffer, 26, 16), TEXT_COLOR_SYSTEM_WHITE, 0);

    // Initialize scroll position and cursor
    gBEXP_TopVisibleIndex = 0;  // First visible unit index
    proc->list_num_cur = 0;  // Current cursor position

    // Draw initial unit list and minimug
    DrawUnitText_BEXP(3, 8);    // Draw Text Once
    DrawUnitSprites_BEXP(3, 8); // Draw Sprites
    DrawUnitMinimugAndLevel(GetUnitFromPrepList(proc->list_num_cur), 15, 8);

    StartSysBrownBox(0xd, 0xe00, 0xf, 0xc00, 0x400, proc);
    EnableSysBrownBox(0, -20, -1, 1);
}

// Rolls the stats and sets up the animation pointers
void SetupBexpLevelUp(struct ProcPrepUnit * proc)
{
    struct Unit* unit = GetUnitFromPrepList(proc->list_num_cur);
    InitBattleUnit(&gBattleActor, unit);

    gBattleActor.unit.exp = unit->exp;
    gBattleActor.expGain = 100 - unit->exp;

    CheckBattleUnitLevelUp(&gBattleActor);

    gManimSt.actor[0].unit = unit;
    gManimSt.actor[0].bu = &gBattleActor;
    gManimSt.actor[0].mu = NULL; 
    
    // FIX: Tells the engine which unit is gaining stats for text/speech
    SetPopupUnit(unit);

    gBEXP_Applied = 0;
}

// Commits the stats to the real unit after animation finishes
void ApplyBexpLevelUpOutcome(struct ProcPrepUnit * proc)
{
    struct Unit* unit = GetUnitFromPrepList(proc->list_num_cur);
    
    // Save the changes from the dummy battle unit to the real unit
    UpdateUnitFromBattle(unit, &gBattleActor);
}

// Wrapper to start the blocking proc from the Proc Script
void CallLevelUpProc(struct ProcPrepUnit * proc)
{
    struct ManimLevelUpProc* child;

    // if (gpKernelDesignerConfig->talk_on_level_up == true) {
    //     child = Proc_StartBlocking(ProcScr_ManimLevelUp_UnitComment, proc);
    // } else {
    //     child = Proc_StartBlocking(ProcScr_ManimLevelUp, proc);
    // }

    child = Proc_StartBlocking(ProcScr_ManimLevelUp, proc);
    
    child->actor_id = 0; // Essential for gManimSt index
}

// 2. Triggers the slide-out and wipes the text
static void StartLevelUpExitAndCleanup(struct ProcPrepUnit* proc) {
    // Commit stats to the unit
    ApplyBexpLevelUpOutcome(proc);

    // Clear the level up text on BG0 immediately so it doesn't linger during slide
    ClearManimLevelUpWindow(); 
    
    // Play the cleanup sound if desired
    PlaySoundEffect(SONG_SE_SYS_WINDOW_CANSEL1);
}

// Restores graphics but preserves the cursor position
void RestoreBexpGraphics(struct ProcPrepUnit * proc)
{
    int saved_cursor = proc->list_num_cur;
    int saved_scroll = gBEXP_TopVisibleIndex;

    // Fully clear VRAM to prevent "ghost" tiles from Level Up
    SetPrimaryHBlankHandler(NULL);
    ClearBg0Bg1();
    BG_Fill(gBG2TilemapBuffer, 0);

    // Re-init the UI
    PrepInitGfx_BEXP(proc);

    proc->list_num_cur = saved_cursor;
    gBEXP_TopVisibleIndex = saved_scroll;
    
    DrawUnitText_BEXP(3, 8);
    DrawUnitSprites_BEXP(3, 8);
    
    EndUiCursorHand();
    ShowSysHandCursor(184, 128, 0x0, 0x000);
}

static void PrepLoop_MainKeyHandler_BEXP(struct ProcPrepUnit * proc)
{
    int unitCount = PrepGetUnitAmount();
    int hasScrolled = 0; // Flag to check if we need to update the scrollbar

    // Redraw unit sprites every frame
    DrawUnitSprites_BEXP(3, 8);

    if (gKeyStatusPtr->newKeys & A_BUTTON) {

        if (gBEXP_State == BEXP_STATE_LIST)
        {
            gBEXP_State = BEXP_STATE_RTEXT;
            EndUiCursorHand();
            ShowSysHandCursor(112, 112, 0x0, 0x000);
            PlaySoundEffect(SONG_SE_SYS_WINDOW_SELECT1);
        }
        else if (gBEXP_State == BEXP_STATE_RTEXT)
        {
            gBEXP_State = BEXP_STATE_APPLY;
            EndUiCursorHand();
            ShowSysHandCursor(184, 128, 0x0, 0x000);
            PlaySoundEffect(SONG_SE_SYS_WINDOW_SELECT1);
        }
        else if (gBEXP_State == BEXP_STATE_APPLY && GetUnitFromPrepList(proc->list_num_cur)->exp + gBEXP_Applied == 100)
        {
            HideSysHandCursor();
            // Prepare the data (Roll stats)
            SetupBexpLevelUp(proc);
            
            // Break the loop and jump to the Level Up sequence
            Proc_Goto(proc, PL_BEXP_LEVELUP);
            return;
        }
    
        return;
    }

    if (gKeyStatusPtr->newKeys & B_BUTTON) {

        if (Proc_Find(gProcScr_HelpBox)) 
        {
            CloseHelpBox();
        }
        else if (gBEXP_State == BEXP_STATE_LIST)
        {
            SetPrimaryHBlankHandler(NULL);
            Proc_Goto(proc, PL_BEXP_PRESS_B);
        }
        else if (gBEXP_State == BEXP_STATE_RTEXT)
        {
            EndUiCursorHand();
            ShowSysHandCursor(14, 64 + ((proc->list_num_cur - gBEXP_TopVisibleIndex) * 16), 0x8, 0x800);
            gBEXP_State = BEXP_STATE_LIST;
        }
        else if (gBEXP_State == BEXP_STATE_APPLY)
        {
            gBEXP_Applied = 0;
            TileMap_FillRect(TILEMAP_LOCATED(gBG0TilemapBuffer, 24, 16), 3, 2, 0);
            PutNumber(TILEMAP_LOCATED(gBG0TilemapBuffer, 26, 16), TEXT_COLOR_SYSTEM_WHITE, gBEXP_Applied);

            EndUiCursorHand();
            ShowSysHandCursor(112, 112, 0x0, 0x800);
            gBEXP_State = BEXP_STATE_RTEXT;
        }

        PlaySoundEffect(SONG_SE_SYS_WINDOW_CANSEL1);
        return;
    }

    if (gKeyStatusPtr->newKeys & DPAD_UP) { 
        if (gBEXP_State == BEXP_STATE_LIST)
        {
            if (proc->list_num_cur > 0) {
                proc->list_num_cur--;
                
                // Scroll window up if cursor moves above visible area
                if (proc->list_num_cur < gBEXP_TopVisibleIndex) {
                    gBEXP_TopVisibleIndex--;
                }
                
                // Update cursor position (visual position relative to scroll)
                ShowSysHandCursor(14, 64 + ((proc->list_num_cur - gBEXP_TopVisibleIndex) * 16), 0x8, 0x800);
                
                // Update minimug for new unit
                DrawUnitMinimugAndLevel(GetUnitFromPrepList(proc->list_num_cur), 15, 8);
                PlaySoundEffect(SONG_SE_SYS_CURSOR_UD1);
                hasScrolled = 1;
            }
        }

        if (gBEXP_State == BEXP_STATE_APPLY)
        {
            if (gBEXP_Applied + GetUnitFromPrepList(proc->list_num_cur)->exp < 96)
            {
                gBEXP_Applied += 5;
                TileMap_FillRect(TILEMAP_LOCATED(gBG0TilemapBuffer, 24, 16), 3, 2, 0);
                PutNumber(TILEMAP_LOCATED(gBG0TilemapBuffer, 26, 16), TEXT_COLOR_SYSTEM_WHITE, gBEXP_Applied);
                DrawUnitMinimugAndLevel(GetUnitFromPrepList(proc->list_num_cur), 15, 8);
            }
        }
    }

    if (gKeyStatusPtr->newKeys & DPAD_DOWN) { 
        if (gBEXP_State == BEXP_STATE_LIST)
        {
            if (proc->list_num_cur < unitCount - 1) {
                proc->list_num_cur++;
                
                // Scroll window down if cursor moves below visible area
                if (proc->list_num_cur >= gBEXP_TopVisibleIndex + BEXP_VISIBLE_COUNT) {
                    gBEXP_TopVisibleIndex++;
                }
                
                // Update cursor position (visual position relative to scroll)
                ShowSysHandCursor(14, 64 + ((proc->list_num_cur - gBEXP_TopVisibleIndex) * 16), 0x8, 0x800);
                
                // Update minimug for new unit
                DrawUnitMinimugAndLevel(GetUnitFromPrepList(proc->list_num_cur), 15, 8);
                PlaySoundEffect(SONG_SE_SYS_CURSOR_UD1);
                hasScrolled = 1;
            }
        }

        if (gBEXP_State == BEXP_STATE_APPLY)
        {
            if (gBEXP_Applied > 4)
            {
                gBEXP_Applied -= 5;
                TileMap_FillRect(TILEMAP_LOCATED(gBG0TilemapBuffer, 24, 16), 3, 2, 0);
                PutNumber(TILEMAP_LOCATED(gBG0TilemapBuffer, 26, 16), TEXT_COLOR_SYSTEM_WHITE, gBEXP_Applied);
                DrawUnitMinimugAndLevel(GetUnitFromPrepList(proc->list_num_cur), 15, 8);
            }
        }
    }

    // Update the physical scroll bar if we moved
    if (hasScrolled) {
        DrawUnitText_BEXP(3, 8); // <--- Only called when necessary
        
        UpdateMenuScrollBarConfig(
            unitCount,                  // Max Index
            gBEXP_TopVisibleIndex * 16, // Current top pixel
            unitCount,                  // Total items
            BEXP_VISIBLE_COUNT          // Visible items
        );
    }

    if (gKeyStatusPtr->newKeys & R_BUTTON) {
        if (gBEXP_State == BEXP_STATE_LIST)
        {
            SetPrimaryHBlankHandler(NULL);
            PlaySoundEffect(SONG_SE_SYS_WINDOW_SELECT1);
            Proc_Goto(proc, PL_BEXP_PRESS_R);
        }
        else if (gBEXP_State == BEXP_STATE_RTEXT)
        {
            LoadHelpBoxGfx(NULL, -1);
            StartHelpBox(112, 112, MSG_BEXP_AMOUNT_DESC);
        }
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

PROC_LABEL(PL_BEXP_LEVELUP),
    // 1. Prepare data
    PROC_CALL(SetupBexpLevelUp),
    
    // 2. Start the animation proc (Non-blocking call here, we handle blocking via WHILE)
    PROC_CALL(CallLevelUpProc), 
    PROC_WHILE(LevelUpProcExists),

    // 3. Start the scroll-out animation and clear text
    PROC_CALL(StartLevelUpExitAndCleanup),

    // 4. Hard-restore the BEXP interface
    PROC_CALL(RestoreBexpGraphics),
    PROC_CALL(ManimLevelUp_RestoreBgm),
    
    PROC_GOTO(PL_BEXP_IDLE),

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