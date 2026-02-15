#include "common-chax.h"
#include "utf8.h"
#include "kernel-lib.h"
#include "constants/texts.h"
#include "popup.h"
#include "prep-skill.h"
#include "jester_headers/macros.h"
#include "jester_headers/custom-functions.h"
#include "jester_headers/custom-structs.h"

#define BASE_VISIBLE_COUNT 5

static int NumberOfChapterBaseConversations()
{
    return 10;
}

static void DrawBaseConversations(int x, int y)
{
    for (int i = 0; i < BASE_VISIBLE_COUNT; i++)
    {
        PutDrawText(&PrepItemSuppyTexts.th[i+2], TILEMAP_LOCATED(gBG0TilemapBuffer, x, y + (i * 2)), TEXT_COLOR_SYSTEM_WHITE, 0, 0, GetStringFromIndex(MSG_BEXP_APPLIED_TITLE));
    }
}

static void PrepInitGfx_BASE(struct ProcPrepUnit * proc)
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

    /* Draw the background for the list of base conversations */
    /* Start 0x7, 0x4  */
    DrawUiFrame2(6, 7, 18, 12, 0);

    /* Display the transparent black banner behind the text */
    SetPrimaryHBlankHandler(PrepItemSupply_OnHBlank);

    BG_EnableSyncByMask(7);
    StartUiCursorHand(proc);
    ResetSysHandCursor(proc);
    DisplaySysHandCursorTextShadow(0x600, 1);
    ShowSysHandCursor(56, 64 + ((proc->list_num_cur - gTopVisibleListIndex) * 16), 15, 0x800);

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
    InitText(PrepItemSuppyTexts.th + 0, 6);
    InitText(PrepItemSuppyTexts.th + 1, 5);
    InitText(PrepItemSuppyTexts.th + 15, 4);

    for (i = 0; i < 8; i++) {
        InitTextDb(PrepItemSuppyTexts.th + 7 + i, 7);
    }

    sub_8097668();

    BG_EnableSyncByMask(4);
    SetBlendConfig(1, 0xe, 4, 0);
    SetBlendTargetA(0, 0, 0, 0, 0);
    SetBlendTargetB(0, 0, 0, 1, 0);

    InitText(&PrepItemSuppyTexts.th[0], 22);
    PutDrawText(&PrepItemSuppyTexts.th[0], TILEMAP_LOCATED(gBG0TilemapBuffer, 6, 3), TEXT_COLOR_SYSTEM_WHITE, 6, 0, GetStringFromIndex(MSG_BASE_CONVERSATIONS_INSTRUCTION));
    InitText(&PrepItemSuppyTexts.th[1], 6);
    PutDrawText(&PrepItemSuppyTexts.th[1], TILEMAP_LOCATED(gBG0TilemapBuffer, 1, 0), TEXT_COLOR_SYSTEM_WHITE, 4, 0, GetStringFromIndex(MSG_PREP_SCREEN_TITLE_BASE_CONVERSATIONS));

    StartSysBrownBox(0x0, 0x7080, 0xf, 0xc00, 0x400, proc);
    EnableSysBrownBox(0, -40, -1, 1);

    StartMenuScrollBar(proc); 
    PutMenuScrollBarAt(42, 64); 
    InitMenuScrollBarImg(0x7A60, 2); 

    gBaseConversations_Total = NumberOfChapterBaseConversations();

    /* Initial configuration to set the bar size/pos */
    UpdateMenuScrollBarConfig(
        gBaseConversations_Total,  // Total Height (max items)
        gTopVisibleListIndex * 16, // Current Top Pixel (Index * 16px per row)
        gBaseConversations_Total,  // Total Rows
        BASE_VISIBLE_COUNT         // Visible Rows
    );

    InitText(&PrepItemSuppyTexts.th[2], 12);
    InitText(&PrepItemSuppyTexts.th[3], 12);
    InitText(&PrepItemSuppyTexts.th[4], 12);
    InitText(&PrepItemSuppyTexts.th[5], 12);
    InitText(&PrepItemSuppyTexts.th[6], 12);

    DrawBaseConversations(8, 8);
}

static void PrepLoop_MainKeyHandler_BASE(struct ProcPrepUnit * proc)
{
    bool hasScrolled = false; // Flag to check if we need to update the scrollbar

    if (gKeyStatusPtr->newKeys & A_BUTTON)
    {
        /* Set the text ID to play in the base conversation before calling a blocking proc */
        gEventSlots[EVT_SLOT_2] = MSG_BASE_CONVERSATION_01_CHAPTER_04;
        Proc_Goto(proc, PL_BASE_CONVERSATIONS_EVENT);
        return;
    }

    if (gKeyStatusPtr->newKeys & B_BUTTON) 
    {
        SetPrimaryHBlankHandler(NULL);
        Proc_Goto(proc, PL_BASE_CONVERSATIONS_PRESS_B);
    }

    if (gKeyStatusPtr->newKeys & DPAD_UP)
    {
        if (proc->list_num_cur > 0) 
        {
            proc->list_num_cur--;

            if (proc->list_num_cur < gTopVisibleListIndex) {
                gTopVisibleListIndex--;
            }       

            ShowSysHandCursor(56, 64 + ((proc->list_num_cur - gTopVisibleListIndex) * 16), 15, 0x800);
            PlaySoundEffect(SONG_SE_SYS_CURSOR_UD1);
            hasScrolled = true;
        }
    }

    if (gKeyStatusPtr->newKeys & DPAD_DOWN)
    {
        if (proc->list_num_cur < NumberOfChapterBaseConversations() - 1)
        {
            proc->list_num_cur++;

            if (proc->list_num_cur >= gTopVisibleListIndex + BASE_VISIBLE_COUNT) {
                gTopVisibleListIndex++;
            }

            ShowSysHandCursor(56, 64 + ((proc->list_num_cur - gTopVisibleListIndex) * 16), 15, 0x800);
            PlaySoundEffect(SONG_SE_SYS_CURSOR_UD1);
            hasScrolled = true;
        }
    }

    // Update the physical scroll bar if we moved
    if (hasScrolled) 
    {
        DrawBaseConversations(8, 8);

        UpdateMenuScrollBarConfig(
            gBaseConversations_Total,  // Total Height (max items)
            gTopVisibleListIndex * 16, // Current Top Pixel (Index * 16px per row)
            gBaseConversations_Total,  // Total Rows
            BASE_VISIBLE_COUNT         // Visible Rows
        );
    }

    return;
}

static const EventScr EventScr_BaseConversation[] = {
    SET_BACKGROUND(0x2)
    FADE_FROM_BLACK(16)
    TEXTSTART
    TEXTSHOW(0xFFFF) // 0xFFFF reads from EVT_SLOT_2
    TEXTEND
    REMA
    ENDA
};

void CallBaseConversationEvent(struct ProcPrepUnit * proc) {
    // Calls text with random background (support viewer?)
    //CallEvent((u16 *)EventScr_SupportViewerConversation, EV_EXEC_QUIET);
    gEventSlots[EVT_SLOT_2] = MSG_BASE_CONVERSATION_01_CHAPTER_04;
    KernelCallEvent(EventScr_BaseConversation, EV_EXEC_CUTSCENE, proc);
}

static void PrepItemList_OnEnd_BASE(struct ProcPrepUnit * proc)
{
    struct ProcAtMenu *pproc = proc->proc_parent;
    pproc->state = 1;

    EndAllParallelWorkers();
    EndAllProcChildren(proc);
    EndFaceById(0);
    EndMuralBackground_();
    ClearBg0Bg1();
}

static void DisablePrepScreenDisplay(struct ProcPrepUnit * proc)
{
    SetPrimaryHBlankHandler(NULL);

    gLCDControlBuffer.dispcnt.win0_on = 0;
    gLCDControlBuffer.dispcnt.win1_on = 0;
    gLCDControlBuffer.dispcnt.objWin_on = 0;

    SetBlendConfig(0, 0, 0, 0);

    EndMuralBackground_();
}

static void ResetScrollerBarVariables(struct ProcPrepUnit *proc) {
    gTopVisibleListIndex = 0;  // First visible unit index
    proc->list_num_cur = 0;  // Current cursor position
}

struct ProcCmd const ProcScr_PrepItemListScreen_BASE[] = {
    PROC_NAME("PrepItemListScreen_BASE"),
    PROC_YIELD,
    PROC_SET_END_CB(PrepItemList_OnEnd_BASE),
    PROC_CALL(ResetScrollerBarVariables),

PROC_LABEL(PL_BASE_CONVERSATIONS_INIT),
    PROC_CALL(PrepInitGfx_BASE),
	PROC_CALL_ARG(NewFadeIn, 0x10),
    PROC_WHILE(FadeInExists),

PROC_LABEL(PL_BASE_CONVERSATIONS_IDLE),
    PROC_REPEAT(PrepLoop_MainKeyHandler_BASE),

PROC_LABEL(PL_BASE_CONVERSATIONS_EVENT),
    PROC_CALL_ARG(NewFadeOut, 0x10),
    PROC_WHILE(FadeOutExists),
    PROC_CALL(DisablePrepScreenDisplay),
    PROC_CALL(CallBaseConversationEvent),
    PROC_WHILE(EventEngineExists), 
    PROC_GOTO(PL_BASE_CONVERSATIONS_INIT),

PROC_LABEL(PL_BASE_CONVERSATIONS_PRESS_B),
    PROC_CALL_ARG(NewFadeOut, 0x10),
    PROC_WHILE(FadeOutExists),

PROC_LABEL(PL_BASE_CONVERSATIONS_END),
    
    PROC_END
};

void StartBaseScreen_FromPrep(struct ProcAtMenu *pproc)
{
    Proc_StartBlocking(ProcScr_PrepItemListScreen_BASE, pproc);
}