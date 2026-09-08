#include "common-chax.h"
#include "utf8.h"
#include "kernel-lib.h"
#include "constants/texts.h"
#include "popup.h"
#include "prep-skill.h"
#include "jester_headers/maps.h"
#include "jester_headers/macros.h"
#include "jester_headers/custom-functions.h"
#include "jester_headers/custom-structs.h"

#define BASE_VISIBLE_COUNT 5

struct ConversationPair {
    int titleTextId;
    int bodyTextId;
    int songId;
    int backgroundId;
    int itemId;
    int flagId;
};

struct ChapterConversations {
    struct ConversationPair conversations[10];
};

// Single master array with inline nested data
// Index corresponds to chapter number
static const struct ChapterConversations gBaseConversationTable[] = {
    // CHAPTER_00
    { .conversations = {{0, 0, 0, 0, 0, 0}} },
    
    // CHAPTER_01
    { .conversations = {{0, 0, 0, 0, 0, 0}} },
    
    // CHAPTER_02
    { .conversations = {{0, 0, 0, 0, 0, 0}} },
    
    // CHAPTER_03
    { .conversations = {{0, 0, 0, 0, 0, 0}} },
    
    // CHAPTER_04
    { .conversations = {
        {MSG_BASE_CH4_01_TITLE, MSG_BASE_CH4_01_TEXT, SONG_POWERFUL_FOE, BG_BURNING_CASTLE, ITEM_ANTITOXIN, 0},
        {MSG_BASE_CH4_02_TITLE, MSG_BASE_CH4_02_TEXT, SONG_POWERFUL_FOE, BG_BURNING_CASTLE, ITEM_ANTITOXIN, 1},
        {MSG_BASE_CH4_03_TITLE, MSG_BASE_CH4_03_TEXT, SONG_POWERFUL_FOE, BG_BURNING_CASTLE, ITEM_ANTITOXIN, 2},
        {MSG_BASE_CH4_04_TITLE, MSG_BASE_CH4_04_TEXT, SONG_POWERFUL_FOE, BG_BURNING_CASTLE, ITEM_ANTITOXIN, 3},
        {MSG_BASE_CH4_05_TITLE, MSG_BASE_CH4_05_TEXT, SONG_POWERFUL_FOE, BG_BURNING_CASTLE, ITEM_ANTITOXIN, 4},
        {MSG_BASE_CH4_06_TITLE, MSG_BASE_CH4_06_TEXT, SONG_POWERFUL_FOE, BG_BURNING_CASTLE, ITEM_ANTITOXIN, 5},
        {MSG_BASE_CH4_07_TITLE, MSG_BASE_CH4_07_TEXT, SONG_POWERFUL_FOE, BG_BURNING_CASTLE, ITEM_ANTITOXIN, 6},
        {MSG_BASE_CH4_08_TITLE, MSG_BASE_CH4_08_TEXT, SONG_POWERFUL_FOE, BG_BURNING_CASTLE, ITEM_ANTITOXIN, 7},
        {MSG_BASE_CH4_09_TITLE, MSG_BASE_CH4_09_TEXT, SONG_POWERFUL_FOE, BG_BURNING_CASTLE, ITEM_ANTITOXIN, 8},
        {MSG_BASE_CH4_10_TITLE, MSG_BASE_CH4_10_TEXT, SONG_POWERFUL_FOE, BG_BURNING_CASTLE, ITEM_ANTITOXIN, 9}
    }},
};

#define FLAG_BYTE(i)  ((i) / 8)
#define FLAG_MASK(i)  (1 << ((i) % 8))

// Set a flag
static inline void SetConversationFlag(int flag)
{
    gBaseConversations_Flags[FLAG_BYTE(flag)] |= FLAG_MASK(flag);
}

// Clear a flag
static inline void ClearConversationFlag(int flag)
{
    gBaseConversations_Flags[FLAG_BYTE(flag)] &= ~FLAG_MASK(flag);
}

// Check a flag
static inline int ConversationFlagSet(int flag)
{
    return (gBaseConversations_Flags[FLAG_BYTE(flag)] & FLAG_MASK(flag)) != 0;
}


static int NumberOfChapterBaseConversations() {
    int lengthOfList = 0;
    for (int i = 0; i < (int)ARRAY_COUNT(gBaseConversationTable[gPlaySt.chapterIndex].conversations); i++)
        lengthOfList += 1;

    return lengthOfList;
}

static void DrawBaseConversations(int x, int y) {

    int color;

    for (int i = 0; i < BASE_VISIBLE_COUNT; i++) 
    {
        ClearText(&PrepItemSuppyTexts.th[i+2]);

        if (ConversationFlagSet(gBaseConversationTable[gPlaySt.chapterIndex].conversations[i + gTopVisibleListIndex].flagId))
            color = TEXT_COLOR_SYSTEM_GRAY;
        else
            color = TEXT_COLOR_SYSTEM_WHITE;

        TileMap_FillRect(TILEMAP_LOCATED(gBG0TilemapBuffer, x - 1, y + (i * 2)), 1, 2, 0);
        PutNumber(TILEMAP_LOCATED(gBG0TilemapBuffer, x, y + (i * 2)), color, (i + 1 + gTopVisibleListIndex));
        PutDrawText(&PrepItemSuppyTexts.th[i+2], TILEMAP_LOCATED(gBG0TilemapBuffer, x + 2, y + (i * 2)), 
                    color, 0, 0, GetStringFromIndex(gBaseConversationTable[gPlaySt.chapterIndex].conversations[i + gTopVisibleListIndex].titleTextId));
    }

    BG_EnableSyncByMask(BG0_SYNC_BIT);
}

static void PrepInitGfx_BASE(struct ProcPrepUnit * proc) {

    StartBgm(SONG_LAUGHTER, 0);

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
    LoadIconPalettes(4);
    RestartMuralBackground();
    DrawUiFrame2(6, 7, 18, 12, 0);
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
    for (int i = 0; i < 8; i++)
        InitText(PrepItemSuppyTexts.th + i, 20);

    sub_8097668();
    BG_EnableSyncByMask(4);
    SetBlendConfig(1, 0xe, 4, 0);
    SetBlendTargetA(0, 0, 0, 0, 0);
    SetBlendTargetB(0, 0, 0, 1, 0);

    PutDrawText(&PrepItemSuppyTexts.th[0], TILEMAP_LOCATED(gBG0TilemapBuffer, 6, 3), TEXT_COLOR_SYSTEM_WHITE, 6, 0, GetStringFromIndex(MSG_BASE_CONVERSATIONS_INSTRUCTION));
    PutDrawText(&PrepItemSuppyTexts.th[1], TILEMAP_LOCATED(gBG0TilemapBuffer, 1, 0), TEXT_COLOR_SYSTEM_WHITE, 4, 0, GetStringFromIndex(MSG_PREP_SCREEN_TITLE_BASE_CONVERSATIONS));

    StartSysBrownBox(0x0, 0x7080, 0xf, 0xc00, 0x400, proc);
    EnableSysBrownBox(0, -40, -1, 1);
    StartMenuScrollBar(proc); 
    PutMenuScrollBarAt(42, 64); 
    InitMenuScrollBarImg(0x7A60, 2); 

    gList_Total = NumberOfChapterBaseConversations();
    UpdateMenuScrollBarConfig(gList_Total, gTopVisibleListIndex * 16, gList_Total, BASE_VISIBLE_COUNT);
    DrawBaseConversations(8, 8);
}

static void PrepLoop_MainKeyHandler_BASE(struct ProcPrepUnit * proc) {
    bool hasScrolled = false;

    if (gKeyStatusPtr->newKeys & A_BUTTON) {
        Proc_Goto(proc, PL_BASE_CONVERSATIONS_EVENT);
        return;
    }

    if (gKeyStatusPtr->newKeys & B_BUTTON) {
        SetPrimaryHBlankHandler(NULL);
        Proc_Goto(proc, PL_BASE_CONVERSATIONS_PRESS_B);
    }

    if (gKeyStatusPtr->newKeys & DPAD_UP) {
        if (proc->list_num_cur > 0) {
            proc->list_num_cur--;
            if (proc->list_num_cur < gTopVisibleListIndex)
                gTopVisibleListIndex--;
            hasScrolled = true;
        }
    }

    if (gKeyStatusPtr->newKeys & DPAD_DOWN) {
        if (proc->list_num_cur < gList_Total - 1) {
            proc->list_num_cur++;
            if (proc->list_num_cur >= gTopVisibleListIndex + BASE_VISIBLE_COUNT)
                gTopVisibleListIndex++;
            hasScrolled = true;
        }
    }

    if (hasScrolled) {
        ShowSysHandCursor(56, 64 + ((proc->list_num_cur - gTopVisibleListIndex) * 16), 15, 0x800);
        PlaySoundEffect(SONG_SE_SYS_CURSOR_UD1);
        DrawBaseConversations(8, 8);
        UpdateMenuScrollBarConfig(gList_Total, gTopVisibleListIndex * 16, gList_Total, BASE_VISIBLE_COUNT);
    }
}

void CallBaseConversationEvent(struct ProcPrepUnit * proc) {
    SetInitTalkTextFont();
    ClearTalkText();
    ClearTalkFaceRefs();

    /* Check if a song ID is set for this conversation, otherwise choose a random one */
    if (gBaseConversationTable[gPlaySt.chapterIndex].conversations[proc->list_num_cur].songId == 0)
        StartBgm(NextRN_N(0x46), 0);
    else
        StartBgm(gBaseConversationTable[gPlaySt.chapterIndex].conversations[proc->list_num_cur].songId, 0);

    /* Check if a background ID is set for this conversation, otherwise choose a random one */
    if (gBaseConversationTable[gPlaySt.chapterIndex].conversations[proc->list_num_cur].backgroundId == 0)
        EventShowTextBgDirect(1, NextRN_N(0x34)); 
    else
        EventShowTextBgDirect(1, gBaseConversationTable[gPlaySt.chapterIndex].conversations[proc->list_num_cur].backgroundId);

    StartTalkExt(0, 0, GetStringFromIndex(gBaseConversationTable[gPlaySt.chapterIndex].conversations[proc->list_num_cur].bodyTextId), proc);
    SetTalkPrintColor(1);
    ApplyPalette(Pal_TalkBubble, 3);
}

static int WaitForBaseConversation(struct ProcPrepUnit * proc) {
    return IsTalkActive();
}

static struct PopupInstruction const BasePopup[] = {
    POPUP_SOUND(SONG_SE_UPDATE),
	POPUP_COLOR(TEXT_COLOR_SYSTEM_WHITE),
    POPUP_SPACE(3),
    POPUP_MSG(MSG_Obtained),
    POPUP_COLOR(TEXT_COLOR_SYSTEM_BLUE),
    POPUP_SPACE(2),
    POPUP_ITEM_STR,
    POPUP_SPACE(2),
    POPUP_ITEM_ICON,
    POPUP_COLOR(TEXT_COLOR_SYSTEM_WHITE),
    POPUP_SPACE(1),
    POPUP_MSG(0x022),                   /* .[.] */
    POPUP_END
};

/* Here we check for two things: that we have an item to give, and we haven't already viewed the conversation */
static void ShowPopup(struct ProcPrepUnit * proc) {

    u16 item = gBaseConversationTable[gPlaySt.chapterIndex].conversations[proc->list_num_cur].itemId;
    bool flagSet = ConversationFlagSet(gBaseConversationTable[gPlaySt.chapterIndex].conversations[proc->list_num_cur].flagId);

    if (item != 0 && !flagSet)
    {
        SetPopupItem(item);
        NewPopup_Simple(BasePopup, 0x60, 0x00, proc);
        AddItemToConvoy(MakeNewItem(item));
        SetConversationFlag(gBaseConversationTable[gPlaySt.chapterIndex].conversations[proc->list_num_cur].flagId);
    }
}

/* Keep the proc on hold until the popup has finished */
static bool WaitForPopup(struct ProcPrepUnit * proc) {
    if (Proc_Find(ProcScr_Popup))
        return true;
    else
        return false;
}

/* Start the combat prep song again when we exit this menu */
static void PrepItemList_OnEnd_BASE(struct ProcPrepUnit * proc) {
    struct ProcAtMenu *pproc = proc->proc_parent;
    pproc->state = 1;
    EndMuralBackground_();
    ClearBg0Bg1();

    StartBgm(SONG_COMBAT_PREPARATION, 0);
}

/* Disable all backgrounds and TSAs before switching to the conversation event */
static void DisablePrepScreenDisplay(struct ProcPrepUnit * proc) {
    SetPrimaryHBlankHandler(NULL);
    SetBlendConfig(0, 0, 0, 0);
    ClearBg0Bg1();
    ResetFaces();
    ResetText();
    EndMenuScrollBar();
    EndSysBrownBox();
    EndMuralBackground_();
    HideSysHandCursor();
}

static void ResetScrollerBarVariables(struct ProcPrepUnit *proc) {
    gTopVisibleListIndex = 0;
    proc->list_num_cur = 0;
}

struct ProcCmd const ProcScr_PrepItemListScreen_BASE[] = {
    PROC_NAME("PrepScreen_BASE"),
    PROC_YIELD,
    PROC_SET_END_CB(PrepItemList_OnEnd_BASE),
    PROC_CALL(ResetScrollerBarVariables),

PROC_LABEL(PL_BASE_CONVERSATIONS_INIT),
    PROC_CALL(PrepInitGfx_BASE),
	PROC_CALL_ARG(NewFadeIn, 16),
    PROC_WHILE(FadeInExists),

PROC_LABEL(PL_BASE_CONVERSATIONS_IDLE),
    PROC_REPEAT(PrepLoop_MainKeyHandler_BASE),

PROC_LABEL(PL_BASE_CONVERSATIONS_EVENT),
    PROC_CALL(DisablePrepScreenDisplay),
    PROC_CALL_ARG(NewFadeOut, 16),
    PROC_WHILE(FadeOutExists),
    PROC_CALL_ARG(NewFadeIn, 16),
    PROC_WHILE(FadeInExists),
    PROC_CALL(CallBaseConversationEvent),
    PROC_WHILE(WaitForBaseConversation), 
    PROC_CALL(ShowPopup),
    PROC_WHILE(WaitForPopup),
    PROC_CALL_ARG(NewFadeOut, 16),
    PROC_WHILE(FadeOutExists),
    PROC_CALL_ARG(NewFadeIn, 16),
    PROC_WHILE(FadeInExists),
    PROC_GOTO(PL_BASE_CONVERSATIONS_INIT),

PROC_LABEL(PL_BASE_CONVERSATIONS_PRESS_B),
    PROC_CALL_ARG(NewFadeOut, 16),
    PROC_WHILE(FadeOutExists),

PROC_LABEL(PL_BASE_CONVERSATIONS_END),
    PROC_END
};

void StartBaseScreen_FromPrep(struct ProcAtMenu *pproc) {
    Proc_StartBlocking(ProcScr_PrepItemListScreen_BASE, pproc);
}