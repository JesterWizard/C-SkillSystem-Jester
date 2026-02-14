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

// Initialize the struct with your specific values
const struct BexpGains gBexpGainConstants = {
    .normal = 2,
    .boss = 10
};

static int round_up_mul(int value, int numerator, int denominator)
{
    int product = value * numerator;

    // Round up the division
    return (product + denominator - 1) / denominator;
}

static int GetBexpCost(struct Unit* unit, int expToGain) {
    int cost = expToGain;
    int level = unit->level;

    if (UNIT_CATTRIBUTES(unit) & CA_PROMOTED)
        level += 20;

    if (level < 4)
        return cost * 1;
    else if (level < 8)
        return round_up_mul(cost, 5, 4);  // 1.25x
    else if (level < 12)
        return round_up_mul(cost, 6, 4);  // 1.50x
    else if (level < 16)
        return round_up_mul(cost, 7, 4);  // 1.75x
    else if (level < 20)
        return cost * 2;                  // 2.00x
    else if (level < 24)
        return round_up_mul(cost, 9, 4);  // 2.25x
    else if (level < 28)
        return round_up_mul(cost, 10, 4); // 2.50x
    else if (level < 32)
        return round_up_mul(cost, 11, 4); // 2.75x
    else if (level < 36)
        return cost * 3;                  // 3.00x
    
    // Fallback for very high levels (optional, keeping consistent with max case)
    return cost * 3;
}

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
    TileMap_FillRect(TILEMAP_LOCATED(gBG0TilemapBuffer, 24, 10), 3, 2, 0);

    /* Clear the space use for the unit's level */
    TileMap_FillRect(TILEMAP_LOCATED(gBG0TilemapBuffer, 21, 10), 2, 2, 0);
    
    PutFaceChibi(GetUnitPortraitId(unit), TILEMAP_LOCATED(gBG0TilemapBuffer, x, y), 0x2A0, 2, 0);

    PutTwoSpecialChar(TILEMAP_LOCATED(gBG0TilemapBuffer, x + 4, y + 2), TEXT_COLOR_SYSTEM_GOLD, TEXT_SPECIAL_LV_A, TEXT_SPECIAL_LV_B);
    PutNumber(TILEMAP_LOCATED(gBG0TilemapBuffer, x + 7, y + 2), TEXT_COLOR_SYSTEM_BLUE, unit->level);
    
    PutSpecialChar(TILEMAP_LOCATED(gBG0TilemapBuffer, x + 8, y + 2), TEXT_COLOR_SYSTEM_GOLD, TEXT_SPECIAL_E);
    TileMap_FillRect(TILEMAP_LOCATED(gBG0TilemapBuffer, 26, 10), 3, 2, 0);

    /* Change the text color of the unit's EXP if they've hit 100 */
    int bexp_applied_color = ((unit->exp + gBEXP_Applied) == 100) ? TEXT_COLOR_SYSTEM_GREEN : TEXT_COLOR_SYSTEM_BLUE;
    PutNumber(TILEMAP_LOCATED(gBG0TilemapBuffer, x + 11, y + 2), bexp_applied_color, unit->exp + gBEXP_Applied);

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
    ShowSysHandCursor(14, 64 + ((proc->list_num_cur - gBEXP_TopVisibleIndex) * 16), 0x8, 0x800);

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
    PutDrawText(NULL, gBG0TilemapBuffer + TILEMAP_INDEX(0, 0), 0, leftPosition, ITEM_PANEL_LEFT_Y, experience_string);
    SetBlendConfig(1, 0xe, 4, 0);
    SetBlendTargetA(0, 0, 0, 0, 0);
    SetBlendTargetB(0, 0, 0, 1, 0);

    InitText(&PrepItemSuppyTexts.th[0], 22);

    /* This is needed to be drawn on both background 0 and 2 so that it persists when the level up proc is called */
    PutDrawText(&PrepItemSuppyTexts.th[0], TILEMAP_LOCATED(gBG0TilemapBuffer, 3, 3), TEXT_COLOR_SYSTEM_WHITE, 8, 0, Utf8ToNarrowFonts(GetStringFromIndex(MSG_BEXP_INSTRUCTION)));
    PutDrawText(&PrepItemSuppyTexts.th[0], TILEMAP_LOCATED(gBG2TilemapBuffer, 3, 3), TEXT_COLOR_SYSTEM_WHITE, 8, 0, Utf8ToNarrowFonts(GetStringFromIndex(MSG_BEXP_INSTRUCTION)));
    
    // Initialize text buffers for unit names
    for (i = 0; i < BEXP_VISIBLE_COUNT; i++) {
        InitText(&gPrepUnitTexts[i + 5], 10);
        ClearText(&gPrepUnitTexts[i + 5]);
    }

    ApplyUnitSpritePalettes();

    // Draw static BEXP UI elements (these don't change) - DRAWN ONCE
    InitText(&PrepItemSuppyTexts.th[1], 10);
    PutDrawText(&PrepItemSuppyTexts.th[1], TILEMAP_LOCATED(gBG0TilemapBuffer, 15, 14), TEXT_COLOR_SYSTEM_GOLD, 0, 0, Utf8ToNarrowFonts(GetStringFromIndex(MSG_BEXP_AMOUNT_TITLE)));

    // Multiplier label and value (next to minimug)
    InitText(&PrepItemSuppyTexts.th[2], 6);
    PutDrawText(&PrepItemSuppyTexts.th[2], TILEMAP_LOCATED(gBG0TilemapBuffer, 19, 8), TEXT_COLOR_SYSTEM_GOLD, 0, 0, Utf8ToNarrowFonts(GetStringFromIndex(MSG_BEXP_MULTIPLIER_TITLE)));

    InitText(&PrepItemSuppyTexts.th[3], 4);
    PutDrawText(&PrepItemSuppyTexts.th[3], TILEMAP_LOCATED(gBG0TilemapBuffer, 24, 8), TEXT_COLOR_SYSTEM_GRAY, 0, 0, Utf8ToNarrowFonts(GetStringFromIndex(MSG_BEXP_MULTIPLIER_1_00)));

    // Applied EXP label and value
    InitText(&PrepItemSuppyTexts.th[4], 10);
    PutDrawText(&PrepItemSuppyTexts.th[4], TILEMAP_LOCATED(gBG0TilemapBuffer, 15, 16), TEXT_COLOR_SYSTEM_GOLD, 0, 0, Utf8ToNarrowFonts(GetStringFromIndex(MSG_BEXP_APPLIED_TITLE)));
    PutNumber(TILEMAP_LOCATED(gBG0TilemapBuffer, 26, 16), TEXT_COLOR_SYSTEM_WHITE, 0);

    int bexp_color = gBEXP_Total == 1000 ? TEXT_COLOR_SYSTEM_GREEN : TEXT_COLOR_SYSTEM_WHITE;
    PutNumber(TILEMAP_LOCATED(gBG0TilemapBuffer, 26, 14), bexp_color, gBEXP_Total);

    // Draw initial unit list and minimug
    DrawUnitText_BEXP(3, 8);    // Draw Text Once
    DrawUnitSprites_BEXP(3, 8); // Draw Sprites
    DrawUnitMinimugAndLevel(GetUnitFromPrepList(proc->list_num_cur), 15, 8);

    StartSysBrownBox(0x0, 0x7080, 0xf, 0xc00, 0x400, proc);
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

    child = Proc_StartBlocking(ProcScr_ManimLevelUp, proc);
    child->actor_id = 0; // Essential for gManimSt index
}

// Triggers the slide-out and wipes the text
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
        else if (gBEXP_State == BEXP_STATE_APPLY)
        {
            struct Unit* currentUnit = GetUnitFromPrepList(proc->list_num_cur);
            
            // Check if ready to level up
            if ((currentUnit->exp + gBEXP_Applied) >= 100)
            {
                EndMenuScrollBar();
                EndSysBrownBox();
                ClearBg0Bg1();
                HideSysHandCursor();
                SetupBexpLevelUp(proc);
                Proc_Goto(proc, PL_BEXP_LEVELUP);
            }
            else
            {
                // Commit the applied EXP (without leveling up)
                currentUnit->exp += gBEXP_Applied;
                gBEXP_Applied = 0;   
                
                // Redraw UI
                TileMap_FillRect(TILEMAP_LOCATED(gBG0TilemapBuffer, 24, 16), 3, 2, 0);
                // Color logic: if exactly 100 (ready to level), Green. Else Blue.
                // Note: logic slightly adjusted to handle the exact moment of commitment
                int bexp_applied_color = TEXT_COLOR_SYSTEM_WHITE; 
                PutNumber(TILEMAP_LOCATED(gBG0TilemapBuffer, 26, 16), bexp_applied_color, gBEXP_Applied);
                DrawUnitMinimugAndLevel(currentUnit, 15, 8);
            }
            return;
        }
        return;
    }

    // --- B BUTTON ---
    if (gKeyStatusPtr->newKeys & B_BUTTON) 
    {
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
            // Refund everything applied so far
            gBEXP_Total += GetBexpCost(GetUnitFromPrepList(proc->list_num_cur), gBEXP_Applied);
            
            gBEXP_Applied = 0;
            TileMap_FillRect(TILEMAP_LOCATED(gBG0TilemapBuffer, 23, 16), 3, 2, 0);
            PutNumber(TILEMAP_LOCATED(gBG0TilemapBuffer, 26, 16), TEXT_COLOR_SYSTEM_WHITE, gBEXP_Applied);

            // Redraw Total
            int bexp_color = gBEXP_Total == 1000 ? TEXT_COLOR_SYSTEM_GREEN : TEXT_COLOR_SYSTEM_WHITE;
            TileMap_FillRect(TILEMAP_LOCATED(gBG0TilemapBuffer, 23, 14), 4, 2, 0);
            PutNumber(TILEMAP_LOCATED(gBG0TilemapBuffer, 26, 14), bexp_color, gBEXP_Total);

            EndUiCursorHand();
            ShowSysHandCursor(112, 112, 0x0, 0x800);
            gBEXP_State = BEXP_STATE_RTEXT;
        }

        PlaySoundEffect(SONG_SE_SYS_WINDOW_CANSEL1);
        return;
    }

    // --- DPAD UP (Add EXP) ---
    if (gKeyStatusPtr->newKeys & DPAD_UP) { 
        if (gBEXP_State == BEXP_STATE_LIST)
        {
            if (proc->list_num_cur > 0) {
                proc->list_num_cur--;
                if (proc->list_num_cur < gBEXP_TopVisibleIndex) {
                    gBEXP_TopVisibleIndex--;
                }
                TileMap_FillRect(TILEMAP_LOCATED(gBG0TilemapBuffer, 24, 16), 3, 2, 0);
                PutNumber(TILEMAP_LOCATED(gBG0TilemapBuffer, 26, 16), TEXT_COLOR_SYSTEM_WHITE, gBEXP_Applied);
                ShowSysHandCursor(14, 64 + ((proc->list_num_cur - gBEXP_TopVisibleIndex) * 16), 0x8, 0x800);
                DrawUnitMinimugAndLevel(GetUnitFromPrepList(proc->list_num_cur), 15, 8);
                PlaySoundEffect(SONG_SE_SYS_CURSOR_UD1);
                hasScrolled = 1;
            }
        }

        if (gBEXP_State == BEXP_STATE_APPLY)
        {
            struct Unit* unit = GetUnitFromPrepList(proc->list_num_cur);
            int currentExp = unit->exp + gBEXP_Applied;

            // Stop if already at 100
            if (currentExp < 100)
            {
                // 1. Determine maximum possible add (Targeting 5, but capped by 100)
                int amountToAdd = 5;
                if (currentExp + amountToAdd > 100) {
                    amountToAdd = 100 - currentExp;
                }

                // 2. Calculate Cost
                int cost = GetBexpCost(unit, amountToAdd);

                // 3. Handle Low BEXP reserves
                // If we can't afford the amount, reduce amountToAdd until we can
                // This handles the "User has less than 5 Bonus EXP" case by adding whatever they can afford
                while (cost > gBEXP_Total && amountToAdd > 0) {
                    amountToAdd--;
                    cost = GetBexpCost(unit, amountToAdd);
                }

                // 4. Apply changes if valid
                if (amountToAdd > 0) {
                    gBEXP_Total -= cost;
                    gBEXP_Applied += amountToAdd;

                    // UI Updates
                    int bexp_color = gBEXP_Total == 1000 ? TEXT_COLOR_SYSTEM_GREEN : TEXT_COLOR_SYSTEM_WHITE;
                    TileMap_FillRect(TILEMAP_LOCATED(gBG0TilemapBuffer, 23, 14), 4, 2, 0);
                    PutNumber(TILEMAP_LOCATED(gBG0TilemapBuffer, 26, 14), bexp_color, gBEXP_Total);

                    TileMap_FillRect(TILEMAP_LOCATED(gBG0TilemapBuffer, 24, 16), 3, 2, 0);
                    int bexp_applied_color = (unit->exp + gBEXP_Applied) == 100 ? TEXT_COLOR_SYSTEM_GREEN : TEXT_COLOR_SYSTEM_WHITE;
                    PutNumber(TILEMAP_LOCATED(gBG0TilemapBuffer, 26, 16), bexp_applied_color, gBEXP_Applied);

                    DrawUnitMinimugAndLevel(unit, 15, 8);
                    
                    // Optional: Sound effect for adding EXP
                    // PlaySoundEffect(SONG_SE_SYS_CURSOR_UD1); 
                }
            }
        }
    }

    // --- DPAD DOWN (Remove EXP) ---
   if (gKeyStatusPtr->newKeys & DPAD_DOWN) { 
        if (gBEXP_State == BEXP_STATE_LIST)
        {
            if (proc->list_num_cur < unitCount - 1) {
                proc->list_num_cur++;
                if (proc->list_num_cur >= gBEXP_TopVisibleIndex + BEXP_VISIBLE_COUNT) {
                    gBEXP_TopVisibleIndex++;
                }
                TileMap_FillRect(TILEMAP_LOCATED(gBG0TilemapBuffer, 24, 16), 3, 2, 0);
                PutNumber(TILEMAP_LOCATED(gBG0TilemapBuffer, 26, 16), TEXT_COLOR_SYSTEM_WHITE, gBEXP_Applied);
                ShowSysHandCursor(14, 64 + ((proc->list_num_cur - gBEXP_TopVisibleIndex) * 16), 0x8, 0x800);
                DrawUnitMinimugAndLevel(GetUnitFromPrepList(proc->list_num_cur), 15, 8);
                PlaySoundEffect(SONG_SE_SYS_CURSOR_UD1);
                hasScrolled = 1;
            }
        }
        else if (gBEXP_State == BEXP_STATE_APPLY)
        {
            if (gBEXP_Applied > 0)
            {
                struct Unit * unit = GetUnitFromPrepList(proc->list_num_cur);
                
                // Default removal is 5
                int amountToRemove = 5;

                // Handle partial removals (CASE 3)
                // If we have 2 EXP applied: remainder = 2. Remove 2.
                // If we have 12 EXP applied: remainder = 2. Remove 2 (down to 10).
                // If we have 10 EXP applied: remainder = 0. Remove 5 (down to 5).
                int remainder = gBEXP_Applied % 5;
                if (remainder != 0) {
                    amountToRemove = remainder;
                }
                
                // Safety check: Don't remove more than we have applied
                if (amountToRemove > gBEXP_Applied) {
                    amountToRemove = gBEXP_Applied;
                }

                // Calculate proper refund based on the multiplier (CASE 2)
                int refund = GetBexpCost(unit, amountToRemove);

                gBEXP_Total += refund;
                gBEXP_Applied -= amountToRemove;

                // UI Updates
                int bexp_color = gBEXP_Total == 1000 ? TEXT_COLOR_SYSTEM_GREEN : TEXT_COLOR_SYSTEM_WHITE;
                TileMap_FillRect(TILEMAP_LOCATED(gBG0TilemapBuffer, 23, 14), 4, 2, 0);
                PutNumber(TILEMAP_LOCATED(gBG0TilemapBuffer, 26, 14), bexp_color, gBEXP_Total);

                TileMap_FillRect(TILEMAP_LOCATED(gBG0TilemapBuffer, 24, 16), 3, 2, 0);
                int bexp_applied_color = (unit->exp + gBEXP_Applied) == 100 ? TEXT_COLOR_SYSTEM_GREEN : TEXT_COLOR_SYSTEM_WHITE;
                PutNumber(TILEMAP_LOCATED(gBG0TilemapBuffer, 26, 16), bexp_applied_color, gBEXP_Applied);

                DrawUnitMinimugAndLevel(unit, 15, 8);
                
                // Sound effect for decrementing
                PlaySoundEffect(SONG_SE_SYS_CURSOR_UD1);
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

static void ResetScrollerBarVariables(struct ProcPrepUnit *proc) {
    gBEXP_TopVisibleIndex = 0;  // First visible unit index
    proc->list_num_cur = 0;  // Current cursor position
}

const struct PopupInstruction BEXPPopup[] = {
    POPUP_SOUND(SONG_SE_UPDATE),
	POPUP_COLOR(TEXT_COLOR_SYSTEM_WHITE),
    POPUP_SPACE(3),
    POPUP_MSG(MSG_BEXP_POPUP_TEXT_1),
    POPUP_SPACE(2),
    POPUP_COLOR(TEXT_COLOR_SYSTEM_BLUE),
    POPUP_NUM,
    POPUP_SPACE(4),
    POPUP_COLOR(TEXT_COLOR_SYSTEM_WHITE),
    POPUP_MSG(MSG_BEXP_POPUP_TEXT_2),
    POPUP_MSG(0x022),                   /* .[.] */
    POPUP_END
};

void GrantBEXP_Loop(struct ProcGrantBEXP* proc)
{
    if (gBEXP_MapGain > 0)
    {
        gBEXP_Total += gBEXP_MapGain;
        SetPopupNumber(gBEXP_MapGain);
        NewPopup_Simple(BEXPPopup, 0x60, 0x00, proc);
    }

    gBEXP_Applied = 0;
}

bool PopupProcExists()
{
    if (Proc_Find(ProcScr_Popup))
        return true;
    else
        return false;
}

const struct ProcCmd ProcScr_GrantBEXP[] = {
    PROC_CALL(GrantBEXP_Loop),
    PROC_WHILE(PopupProcExists),
    PROC_END,
};

void GrantBEXP(ProcPtr parent)
{
    Proc_StartBlocking(ProcScr_GrantBEXP, parent);
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

const struct ProcCmd ProcScr_PrepItemListScreen_BEXP[] = {
    PROC_NAME("PrepItemListScreen_BEXP"),
    PROC_YIELD,
    PROC_SET_END_CB(PrepItemList_OnEnd_BEXP),
    PROC_CALL(ResetScrollerBarVariables),

PROC_LABEL(PL_BEXP_INIT),
    PROC_CALL(PrepInitGfx_BEXP),
	PROC_CALL_ARG(NewFadeIn, 0x10),
    PROC_WHILE(FadeInExists),

PROC_LABEL(PL_BEXP_IDLE),
    PROC_REPEAT(PrepLoop_MainKeyHandler_BEXP),

PROC_LABEL(PL_BEXP_LEVELUP),
    PROC_CALL(SetupBexpLevelUp),
    PROC_CALL(CallLevelUpProc), 
    PROC_WHILE(LevelUpProcExists),
    PROC_CALL(StartLevelUpExitAndCleanup),
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
    PROC_CALL(sub_809B014),
    PROC_CALL(sub_809B504),
    PROC_YIELD,
    PROC_CALL(sub_809B520),
    PROC_CALL_ARG(NewFadeOut, 0x10),
    PROC_WHILE(FadeOutExists),
    PROC_GOTO(PL_BEXP_INIT),

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