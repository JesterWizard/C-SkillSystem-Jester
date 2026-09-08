/**
 * EventReplay.c
 *
 * Prep screen "Replay" option — two-level menu that lets the player
 * rewatch any conversation from any chapter reached so far.
 *
 * Level 1: Chapter list  (only chapters up to gPlaySt.chapterIndex)
 * Level 2: Numbered conversation list for the selected chapter
 *
 * HOW TO ADD CONVERSATIONS
 * ------------------------
 * 1. Add an entry to the appropriate gReplay_ChXX[] array below with the
 *    message ID of the conversation text (from Contents/Texts/build/msgs.h).
 * 2. Rebuild.
 */

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

#define EVENT_REPLAY_VISIBLE_COUNT 5

/* =========================================================================
 * Data structures
 * ========================================================================= */

struct EventReplayConv {
    int bodyTextId; /* message ID of the conversation to play */
    int songId;     /* background music; 0 = pick randomly    */
    int bgId;       /* background image;  0 = pick randomly   */
};

struct EventReplayChapterDef {
    int chapterTitleTextId;              /* label shown in the chapter list */
    int chapterIndex;                    /* compared against gPlaySt.chapterIndex */
    const struct EventReplayConv *convs; /* per-chapter conversation list   */
    int convCount;                       /* number of entries in convs      */
};

/* =========================================================================
 * Per-chapter conversation lists
 * Add entries here to populate a chapter's replay menu.
 * ========================================================================= */

/* --- Chapter 00 --- */
static const struct EventReplayConv gReplay_Ch00[] = {
    { Chapter_00_Scene_01_Convo_01,  0, 0 },
    { Chapter_00_Scene_01_Convo_02,  0, 0 },
    { Chapter_00_Scene_01_Convo_03,  0, 0 },
    { Chapter_00_Scene_01_Convo_04,  0, 0 },
    { Chapter_00_Scene_02_Convo_01,  0, 0 },
    { Chapter_00_Scene_02_Convo_02,  0, 0 },
    { Chapter_00_Scene_02_Convo_03,  0, 0 },
    { Chapter_00_Scene_02_Convo_04_1, 0, 0 },
    { Chapter_00_Scene_02_Convo_04_2, 0, 0 },
    { Chapter_00_Scene_02_Convo_05,  0, 0 },
    { Chapter_00_Scene_03_Convo_01,  0, 0 },
    { Chapter_00_Scene_03_Convo_02,  0, 0 },
    { Chapter_00_Scene_03_Convo_03,  0, 0 },
    { Chapter_00_Scene_03_Convo_04,  0, 0 },
    { Chapter_00_Scene_03_Convo_05,  0, 0 },
    { Chapter_00_ONeill_Agro,        0, 0 },
    { Chapter_00_Seth_Warning,       0, 0 },
    { Chapter_00_Scene_03_Convo_06,  0, 0 },
};

/* --- Chapter 01 --- */
static const struct EventReplayConv gReplay_Ch01[] = {
    { Chapter_01_Scene_01_Convo_01,  0, 0 },
    { Chapter_01_Scene_02_Convo_01,  0, 0 },
    { Chapter_01_Scene_02_Convo_02,  0, 0 },
    { Chapter_01_Scene_03_Convo_01,  0, 0 },
    { Chapter_01_Scene_04_Convo_01,  0, 0 },
    { Chapter_01_Scene_05_Convo_01,  0, 0 },
    { Chapter_01_Scene_06_Convo_01,  0, 0 },
    { Chapter_01_Scene_07_Convo_01,  0, 0 },
    { Chapter_01_Scene_08_Convo_01,  0, 0 },
    { Chapter_01_Scene_09_Convo_01,  0, 0 },
    { Chapter_01_Scene_10_Convo_01,  0, 0 },
    { Chapter_01_Scene_11_Convo_01,  0, 0 },
    { Chapter_01_Scene_11_Convo_02,  0, 0 },
    { Chapter_01_Scene_11_Convo_03,  0, 0 },
    { Chapter_01_Scene_12_Convo_01,  0, 0 },
    { Chapter_01_Scene_12_Convo_02,  0, 0 },
    { Chapter_01_Scene_13_Convo_01,  0, 0 },
};

/* --- Chapter 02 --- */
static const struct EventReplayConv gReplay_Ch02[] = {
    { Chapter_02_Scene_01_Convo_01,  0, 0 },
    { Chapter_02_Scene_02_Convo_01,  0, 0 },
    { Chapter_02_Scene_03_Convo_01,  0, 0 },
    { Chapter_02_Scene_04_Convo_01,  0, 0 },
    { Chapter_02_Scene_05_Convo_01,  0, 0 },
    { Chapter_02_Scene_06_Convo_01,  0, 0 },
    { Chapter_02_Scene_06_Convo_02,  0, 0 },
    { Chapter_02_Scene_06_Convo_03,  0, 0 },
    { Chapter_02_Ross_Eirika,        0, 0 },
    { Chapter_02_Ross_Garcia,        0, 0 },
    { Chapter_02_Scene_07_Convo_01,  0, 0 },
    { Chapter_02_Scene_08_Convo_01,  0, 0 },
    { Chapter_02_Scene_09_Convo_01,  0, 0 },
    { Chapter_02_Scene_10_Convo_01,  0, 0 },
    { Chapter_02_Scene_11_Convo_01,  0, 0 },
    { Chapter_02_Scene_12_Convo_01,  0, 0 },
};

/* --- Chapter 03 --- */
static const struct EventReplayConv gReplay_Ch03[] = {
    { Chapter_03_Scene_01_Convo_01,  0, 0 },
    { Chapter_03_Scene_02_Convo_01,  0, 0 },
    { Chapter_03_Scene_03_Convo_01,  0, 0 },
    { Chapter_03_Scene_03_Convo_02,  0, 0 },
    { Chapter_03_Colm_Appears,       0, 0 },
    { Chapter_03_Colm_Neimi,         0, 0 },
    { Chapter_03_Ross_Appears,       0, 0 },
    { Chapter_03_Ross_Eirika,        0, 0 },
    { Chapter_03_Scene_04_Convo_01,  0, 0 },
    { Chapter_03_Scene_05_Convo_01,  0, 0 },
    { Chapter_03_Scene_06_Convo_01,  0, 0 },
    { Chapter_03_Scene_07_Convo_01,  0, 0 },
    { Chapter_03_Scene_07_Convo_02,  0, 0 },
};

/* --- Chapter 04 --- */
static const struct EventReplayConv gReplay_Ch04[] = {
    { Chapter_04_Scene_01_Convo_01,  0, 0 },
    { Chapter_04_Scene_02_Convo_01,  0, 0 },
    { Chapter_04_Scene_03_Convo_01,  0, 0 },
    { Chapter_04_Scene_04_Convo_01,  0, 0 },
    { Chapter_04_Scene_04_Convo_02,  0, 0 },
    { Chapter_04_Scene_04_Convo_03,  0, 0 },
    { Chapter_04_Scene_05_Convo_01,  0, 0 },
    { Chapter_04_ROSS,               0, 0 },
    { Chapter_04_GARCIA,             0, 0 },
    { Chapter_04_ROSS_GARCIA,        0, 0 },
    { Chapter_04_LARACHEL,           0, 0 },
    { Chapter_04_Scene_06_Convo_01,  0, 0 },
    { Chapter_04_Scene_07_Convo_01,  0, 0 },
    { Chapter_04_Scene_08_Convo_01,  0, 0 },
    { Chapter_04_Scene_09_Convo_01,  0, 0 },
    { Chapter_04_Scene_10_Convo_01,  0, 0 },
};

/* --- Chapter 05 --- */
static const struct EventReplayConv gReplay_Ch05[] = {
    { Chapter_05_Scene_01_Convo_01,  0, 0 },
    { Chapter_05_Scene_01_Convo_02,  0, 0 },
    { Chapter_05_Scene_02_Convo_01,  0, 0 },
    { Chapter_05_Scene_02_Convo_02,  0, 0 },
    { Chapter_05_Scene_03_Convo_01,  0, 0 },
    { Chapter_05_Scene_03_Convo_02,  0, 0 },
    { Chapter_05_Scene_04_Convo_01,  0, 0 },
    { Chapter_05_Scene_04_Convo_02,  0, 0 },
    { Chapter_05_Scene_04_Convo_03,  0, 0 },
    { Chapter_05_Scene_04_Convo_04,  0, 0 },
    { Chapter_05_Scene_04_Convo_05,  0, 0 },
    { Chapter_05_Scene_04_Convo_06,  0, 0 },
    { Chapter_05_BANDIT,             0, 0 },
    { Chapter_05_Scene_05_Convo_01,  0, 0 },
    { Chapter_05_Scene_06_Convo_01,  0, 0 },
};

/* --- Chapter 06 --- */
static const struct EventReplayConv gReplay_Ch06[] = {
    { Chapter_06_Pre_Scene_01_Convo_01,  0, 0 },
    { Chapter_06_Pre_Scene_02_Convo_01,  0, 0 },
    { Chapter_06_Pre_Scene_02_Convo_02,  0, 0 },
    { Chapter_06_Pre_Scene_03_Convo_01,  0, 0 },
    { Chapter_06_Pre_Scene_03_Convo_02,  0, 0 },
    { Chapter_06_Post_Scene_01_Convo_01, 0, 0 },
    { Chapter_06_Post_Scene_02_Convo_01, 0, 0 },
    { Chapter_06_Post_Scene_03_Convo_01, 0, 0 },
    { Chapter_06_Post_Scene_04_Convo_01, 0, 0 },
    { Chapter_06_Post_Scene_05_Convo_01, 0, 0 },
    { Chapter_06_Post_Scene_06_Convo_01, 0, 0 },
};

/* --- Chapter 07 --- */
static const struct EventReplayConv gReplay_Ch07[] = {
    { Chapter_07_Pre_Scene_01_Convo_01,          0, 0 },
    { Chapter_07_Pre_Scene_Franz,                0, 0 },
    { Chapter_07_Pre_Scene_Gilliam_Moulder_Tana, 0, 0 },
    { Chapter_07_Pre_Scene_Ross_Garcia,          0, 0 },
    { Chapter_07_Pre_Scene_Neimi_Colm,           0, 0 },
    { Chapter_07_Pre_Scene_Lute_Artur,           0, 0 },
    { Chapter_07_Pre_Scene_Joshua,               0, 0 },
    { Chapter_07_Pre_Scene_Natasha,              0, 0 },
    { Chapter_07_Pre_Scene_02_Convo_01,          0, 0 },
    { Chapter_07_Post_Scene_01_Convo_01,         0, 0 },
    { Chapter_07_Post_Scene_02_Convo_01,         0, 0 },
};

/* --- Chapter 08 --- */
static const struct EventReplayConv gReplay_Ch08[] = {
    { Chapter_08_Pre_Scene_01_Convo_01,    0, 0 },
    { Chapter_08_Pre_Scene_01_Convo_02,    0, 0 },
    { Chapter_08_Pre_Scene_01_Convo_03,    0, 0 },
    { Chapter_08_Pre_Scene_02_Convo_01,    0, 0 },
    { Chapter_08_Pre_Scene_03_Convo_01,    0, 0 },
    { Chapter_08_In_Scene_Ephraim_Escapes, 0, 0 },
    { Chapter_08_In_Scene_Ephraim_Eirika,  0, 0 },
    { Chapter_08_In_Scene_Seth_Kyle,       0, 0 },
    { Chapter_08_In_Scene_Forde_Franz,     0, 0 },
    { Chapter_08_Post_Scene_01_Convo_01,   0, 0 },
    { Chapter_08_Post_Scene_01_Convo_02,   0, 0 },
    { Chapter_08_Post_Scene_01_Convo_03,   0, 0 },
};

/* --- Chapter 09 --- */
static const struct EventReplayConv gReplay_Ch09[] = {
    { Chapter_09_Pre_Scene_01_Convo_01,   0, 0 },
    { Chapter_09_Pre_Scene_01_Convo_02,   0, 0 },
    { Chapter_09_Pre_Scene_01_Convo_03,   0, 0 },
    { Chapter_09_Pre_Scene_02_Convo_01,   0, 0 },
    { Chapter_09_Pre_Scene_03_Convo_01,   0, 0 },
    { Chapter_09_Pre_Scene_04_Convo_01,   0, 0 },
    { Chapter_09_Pre_Scene_05_Convo_01,   0, 0 },
    { Chapter_09_Pre_Scene_06_Convo_01,   0, 0 },
    { Chapter_09_In_Scene_Amelia_Arrives, 0, 0 },
    { Chapter_09_In_Scene_Ewan,           0, 0 },
    { Chapter_09_In_Scene_Amelia_Eirika,  0, 0 },
    { Chapter_09_In_Scene_Amelia_Franz,   0, 0 },
    { Chapter_09_Post_Scene_01_Convo_01,  0, 0 },
    { Chapter_09_Post_Scene_01_Convo_02,  0, 0 },
    { Chapter_09_Post_Scene_01_Convo_03,  0, 0 },
    { Chapter_09_Post_Scene_02_Convo_01,  0, 0 },
    { Chapter_09_Post_Scene_02_Convo_02,  0, 0 },
};

/* =========================================================================
 * Master catalog — one entry per chapter, ordered by chapterIndex.
 * Only chapters where chapterIndex <= gPlaySt.chapterIndex are shown.
 * ========================================================================= */
static const struct EventReplayChapterDef gEventReplayTable[] = {
    { MSG_CHAPTER_00_TITLE, 0, gReplay_Ch00, ARRAY_COUNT(gReplay_Ch00) },
    { MSG_CHAPTER_01_TITLE, 1, gReplay_Ch01, ARRAY_COUNT(gReplay_Ch01) },
    { MSG_CHAPTER_02_TITLE, 2, gReplay_Ch02, ARRAY_COUNT(gReplay_Ch02) },
    { MSG_CHAPTER_03_TITLE, 3, gReplay_Ch03, ARRAY_COUNT(gReplay_Ch03) },
    { MSG_CHAPTER_04_TITLE, 4, gReplay_Ch04, ARRAY_COUNT(gReplay_Ch04) },
    { MSG_CHAPTER_05_TITLE, 5, gReplay_Ch05, ARRAY_COUNT(gReplay_Ch05) },
    { MSG_CHAPTER_06_TITLE, 6, gReplay_Ch06, ARRAY_COUNT(gReplay_Ch06) },
    { MSG_CHAPTER_07_TITLE, 7, gReplay_Ch07, ARRAY_COUNT(gReplay_Ch07) },
    { MSG_CHAPTER_08_TITLE, 8, gReplay_Ch08, ARRAY_COUNT(gReplay_Ch08) },
    { MSG_CHAPTER_09_TITLE, 9, gReplay_Ch09, ARRAY_COUNT(gReplay_Ch09) },
};

/* =========================================================================
 * Chapter filtering helpers
 * ========================================================================= */

/* Count how many chapters are unlocked (chapterIndex <= gPlaySt.chapterIndex). */
static int CountUnlockedChapters(void)
{
    int n = 0, i;
    for (i = 0; i < (int)ARRAY_COUNT(gEventReplayTable); i++) {
        if (gEventReplayTable[i].chapterIndex <= (int)gPlaySt.chapterIndex)
            n++;
    }
    return n;
}

/* Return the nth unlocked chapter entry (0-based displayIdx). */
static const struct EventReplayChapterDef *GetUnlockedChapterAt(int displayIdx)
{
    int count = 0, i;
    for (i = 0; i < (int)ARRAY_COUNT(gEventReplayTable); i++) {
        if (gEventReplayTable[i].chapterIndex <= (int)gPlaySt.chapterIndex) {
            if (count == displayIdx)
                return &gEventReplayTable[i];
            count++;
        }
    }
    return NULL;
}

static int CountConvsInChapter(int displayIdx)
{
    const struct EventReplayChapterDef *def = GetUnlockedChapterAt(displayIdx);
    return def ? def->convCount : 0;
}

/* =========================================================================
 * Draw helpers
 * ========================================================================= */

/* Draw the chapter list (level 1). Uses gEventReplay_ChapterScrollIndex. */
static void EventReplay_DrawChapterList(int x, int y)
{
    int total = CountUnlockedChapters();
    int i;

    for (i = 0; i < EVENT_REPLAY_VISIBLE_COUNT; i++) {
        ClearText(&PrepItemSuppyTexts.th[i + 2]);

        int idx = i + gEventReplay_ChapterScrollIndex;
        if (idx >= total)
            break;

        const struct EventReplayChapterDef *def = GetUnlockedChapterAt(idx);
        TileMap_FillRect(TILEMAP_LOCATED(gBG0TilemapBuffer, x - 1, y + (i * 2)), 1, 2, 0);
        PutNumber(TILEMAP_LOCATED(gBG0TilemapBuffer, x, y + (i * 2)),
                  TEXT_COLOR_SYSTEM_WHITE, idx + 1);
        PutDrawText(&PrepItemSuppyTexts.th[i + 2],
                    TILEMAP_LOCATED(gBG0TilemapBuffer, x + 2, y + (i * 2)),
                    TEXT_COLOR_SYSTEM_WHITE, 0, 0,
                    GetStringFromIndex(def->chapterTitleTextId));
    }

    BG_EnableSyncByMask(BG0_SYNC_BIT);
}

/* Draw the conversation list (level 2). Uses gTopVisibleListIndex. */
static void EventReplay_DrawConvList(int x, int y)
{
    const struct EventReplayChapterDef *chDef = GetUnlockedChapterAt(gEventReplay_SelectedChapter);
    int total = chDef ? chDef->convCount : 0;
    int i;

    for (i = 0; i < EVENT_REPLAY_VISIBLE_COUNT; i++) {
        ClearText(&PrepItemSuppyTexts.th[i + 2]);

        int convIdx = i + gTopVisibleListIndex;
        if (convIdx >= total)
            break;

        TileMap_FillRect(TILEMAP_LOCATED(gBG0TilemapBuffer, x - 1, y + (i * 2)), 1, 2, 0);
        PutDrawText(&PrepItemSuppyTexts.th[i + 2],
                    TILEMAP_LOCATED(gBG0TilemapBuffer, x, y + (i * 2)),
                    TEXT_COLOR_SYSTEM_WHITE, 0, 0,
                    GetStringFromIndex(MSG_EVENT_REPLAY_CONV_LABEL));
        PutNumber(TILEMAP_LOCATED(gBG0TilemapBuffer, x + 9, y + (i * 2)),
                  TEXT_COLOR_SYSTEM_WHITE, convIdx + 1);
    }

    BG_EnableSyncByMask(BG0_SYNC_BIT);
}

/* =========================================================================
 * Shared GFX initialisation (called at top of both chapter and conv init)
 * ========================================================================= */

static void EventReplay_InitGfx_Common(struct ProcPrepUnit *proc)
{
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

    gLCDControlBuffer.dispcnt.win0_on   = 1;
    gLCDControlBuffer.dispcnt.win1_on   = 0;
    gLCDControlBuffer.dispcnt.objWin_on = 0;
    gLCDControlBuffer.win0_left         = 128;
    gLCDControlBuffer.win0_top          = 40;
    gLCDControlBuffer.win0_right        = 224;
    gLCDControlBuffer.win0_bottom       = 152;
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

    StartSysBrownBox(0x0, 0x7080, 0xf, 0xc00, 0x400, proc);
    EnableSysBrownBox(0, -40, -1, 1);
    StartMenuScrollBar(proc);
    PutMenuScrollBarAt(42, 64);
    InitMenuScrollBarImg(0xE00, 2);
}

/* Level 1 — chapter list */
static void EventReplay_InitGfx_Chapters(struct ProcPrepUnit *proc)
{
    EventReplay_InitGfx_Common(proc);

    ShowSysHandCursor(56,
        64 + ((proc->list_num_cur - gEventReplay_ChapterScrollIndex) * 16),
        15, 0x800);

    PutDrawText(&PrepItemSuppyTexts.th[0],
                TILEMAP_LOCATED(gBG0TilemapBuffer, 9, 3),
                TEXT_COLOR_SYSTEM_WHITE, 6, 0,
                GetStringFromIndex(MSG_EVENT_REPLAY_SELECT_CHAPTER_INSTRUCTION));
    PutDrawText(&PrepItemSuppyTexts.th[1],
                TILEMAP_LOCATED(gBG0TilemapBuffer, 1, 0),
                TEXT_COLOR_SYSTEM_WHITE, 4, 0,
                GetStringFromIndex(MSG_PREP_SCREEN_TITLE_EVENT_REPLAY));

    gList_Total = (u8)CountUnlockedChapters();
    UpdateMenuScrollBarConfig(10,
                              gEventReplay_ChapterScrollIndex * 16,
                              gList_Total,
                              EVENT_REPLAY_VISIBLE_COUNT);
    EventReplay_DrawChapterList(8, 8);
}

/* Level 2 — conversation list */
static void EventReplay_InitGfx_Convs(struct ProcPrepUnit *proc)
{
    EventReplay_InitGfx_Common(proc);

    ShowSysHandCursor(56,
        64 + ((proc->list_num_cur - gTopVisibleListIndex) * 16),
        15, 0x800);

    PutDrawText(&PrepItemSuppyTexts.th[0],
                TILEMAP_LOCATED(gBG0TilemapBuffer, 5, 3),
                TEXT_COLOR_SYSTEM_WHITE, 6, 0,
                GetStringFromIndex(MSG_EVENT_REPLAY_SELECT_CONV_INSTRUCTION));
    PutDrawText(&PrepItemSuppyTexts.th[1],
                TILEMAP_LOCATED(gBG0TilemapBuffer, 1, 0),
                TEXT_COLOR_SYSTEM_WHITE, 4, 0,
                GetStringFromIndex(MSG_PREP_SCREEN_TITLE_EVENT_REPLAY));

    gList_Total = (u8)CountConvsInChapter(gEventReplay_SelectedChapter);
    UpdateMenuScrollBarConfig(10,
                              gTopVisibleListIndex * 16,
                              gList_Total,
                              EVENT_REPLAY_VISIBLE_COUNT);
    EventReplay_DrawConvList(8, 8);
}

/* =========================================================================
 * Key handlers
 * ========================================================================= */

static void EventReplay_KeyHandler_Chapters(struct ProcPrepUnit *proc)
{
    bool hasScrolled = false;
    int  total       = CountUnlockedChapters();

    if (gKeyStatusPtr->newKeys & A_BUTTON) {
        /* Save chapter-list position before entering conversation list */
        gEventReplay_SelectedChapter    = (u8)proc->list_num_cur;
        gEventReplay_ChapterScrollIndex = gTopVisibleListIndex;
        /* Reset scroll/cursor for conversation list */
        proc->list_num_cur   = 0;
        gTopVisibleListIndex = 0;
        PlaySoundEffect(SONG_SE_SYS_WINDOW_SELECT1);
        Proc_Goto(proc, PL_EVENT_REPLAY_CHAPTER_TO_CONV);
        return;
    }

    if (gKeyStatusPtr->newKeys & B_BUTTON) {
        SetPrimaryHBlankHandler(NULL);
        PlaySoundEffect(SONG_SE_SYS_WINDOW_CANSEL1);
        Proc_Goto(proc, PL_EVENT_REPLAY_EXIT);
        return;
    }

    if (gKeyStatusPtr->newKeys & DPAD_UP) {
        if (proc->list_num_cur > 0) {
            proc->list_num_cur--;
            if (proc->list_num_cur < gEventReplay_ChapterScrollIndex)
                gEventReplay_ChapterScrollIndex--;
            hasScrolled = true;
        }
    }

    if (gKeyStatusPtr->newKeys & DPAD_DOWN) {
        if (proc->list_num_cur < total - 1) {
            proc->list_num_cur++;
            if (proc->list_num_cur >= gEventReplay_ChapterScrollIndex + EVENT_REPLAY_VISIBLE_COUNT)
                gEventReplay_ChapterScrollIndex++;
            hasScrolled = true;
        }
    }

    if (hasScrolled) {
        ShowSysHandCursor(56,
            64 + ((proc->list_num_cur - gEventReplay_ChapterScrollIndex) * 16),
            15, 0x800);
        PlaySoundEffect(SONG_SE_SYS_CURSOR_UD1);
        EventReplay_DrawChapterList(8, 8);
        UpdateMenuScrollBarConfig(10,
                                  gEventReplay_ChapterScrollIndex * 16,
                                  gList_Total,
                                  EVENT_REPLAY_VISIBLE_COUNT);
    }
}

static void EventReplay_KeyHandler_Convs(struct ProcPrepUnit *proc)
{
    bool hasScrolled = false;
    int  total       = CountConvsInChapter(gEventReplay_SelectedChapter);

    if (gKeyStatusPtr->newKeys & A_BUTTON) {
        PlaySoundEffect(SONG_SE_SYS_WINDOW_SELECT1);
        Proc_Goto(proc, PL_EVENT_REPLAY_PLAY);
        return;
    }

    if (gKeyStatusPtr->newKeys & B_BUTTON) {
        SetPrimaryHBlankHandler(NULL);
        PlaySoundEffect(SONG_SE_SYS_WINDOW_CANSEL1);
        Proc_Goto(proc, PL_EVENT_REPLAY_CONV_TO_CHAPTER);
        return;
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
        if (proc->list_num_cur < total - 1) {
            proc->list_num_cur++;
            if (proc->list_num_cur >= gTopVisibleListIndex + EVENT_REPLAY_VISIBLE_COUNT)
                gTopVisibleListIndex++;
            hasScrolled = true;
        }
    }

    if (hasScrolled) {
        ShowSysHandCursor(56,
            64 + ((proc->list_num_cur - gTopVisibleListIndex) * 16),
            15, 0x800);
        PlaySoundEffect(SONG_SE_SYS_CURSOR_UD1);
        EventReplay_DrawConvList(8, 8);
        UpdateMenuScrollBarConfig(10,
                                  gTopVisibleListIndex * 16,
                                  gList_Total,
                                  EVENT_REPLAY_VISIBLE_COUNT);
    }
}

/* =========================================================================
 * Conversation playback
 * ========================================================================= */

static void EventReplay_PlayConversation(struct ProcPrepUnit *proc)
{
    const struct EventReplayChapterDef *chDef = GetUnlockedChapterAt(gEventReplay_SelectedChapter);
    const struct EventReplayConv *conv = &chDef->convs[proc->list_num_cur];

    SetInitTalkTextFont();
    ClearTalkText();
    ClearTalkFaceRefs();

    if (conv->songId == 0)
        StartBgm(NextRN_N(0x46), 0);
    else
        StartBgm(conv->songId, 0);

    if (conv->bgId == 0)
        EventShowTextBgDirect(1, NextRN_N(0x34));
    else
        EventShowTextBgDirect(1, conv->bgId);

    StartTalkExt(0, 0, GetStringFromIndex(conv->bodyTextId), proc);
    SetTalkPrintColor(1);
    ApplyPalette(Pal_TalkBubble, 3);
}

static int EventReplay_WaitForConversation(struct ProcPrepUnit *proc)
{
    (void)proc;
    return IsTalkActive();
}

/* =========================================================================
 * State transitions
 * ========================================================================= */

/* Clean up display layers before a fade or conversation. */
static void EventReplay_DisableDisplay(struct ProcPrepUnit *proc)
{
    (void)proc;
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

/* Restore chapter-list cursor and scroll when navigating back from level 2. */
static void EventReplay_RestoreChapterState(struct ProcPrepUnit *proc)
{
    proc->list_num_cur   = gEventReplay_SelectedChapter;
    gTopVisibleListIndex = gEventReplay_ChapterScrollIndex;
}

/* Called when the proc ends; restore the prep screen music. */
static void EventReplay_OnEnd(struct ProcPrepUnit *proc)
{
    struct ProcAtMenu *pproc = proc->proc_parent;
    pproc->state = 1;
    EndMuralBackground_();
    ClearBg0Bg1();
    StartBgm(SONG_COMBAT_PREPARATION, 0);
}

/* Reset all state on first entry. */
static void EventReplay_ResetVars(struct ProcPrepUnit *proc)
{
    proc->list_num_cur              = 0;
    gTopVisibleListIndex            = 0;
    gEventReplay_SelectedChapter    = 0;
    gEventReplay_ChapterScrollIndex = 0;
}

/* =========================================================================
 * Proc script
 * ========================================================================= */

struct ProcCmd const ProcScr_EventReplay[] = {
    PROC_NAME("EventReplay"),
    PROC_YIELD,
    PROC_SET_END_CB(EventReplay_OnEnd),
    PROC_CALL(EventReplay_ResetVars),

PROC_LABEL(PL_EVENT_REPLAY_CHAPTER_INIT),
    PROC_CALL(EventReplay_InitGfx_Chapters),
    PROC_CALL_ARG(NewFadeIn, 16),
    PROC_WHILE(FadeInExists),

PROC_LABEL(PL_EVENT_REPLAY_CHAPTER_IDLE),
    PROC_REPEAT(EventReplay_KeyHandler_Chapters),

PROC_LABEL(PL_EVENT_REPLAY_CHAPTER_TO_CONV),
    PROC_CALL(EventReplay_DisableDisplay),
    PROC_CALL_ARG(NewFadeOut, 16),
    PROC_WHILE(FadeOutExists),
    PROC_GOTO(PL_EVENT_REPLAY_CONV_INIT),

PROC_LABEL(PL_EVENT_REPLAY_CONV_INIT),
    PROC_CALL(EventReplay_InitGfx_Convs),
    PROC_CALL_ARG(NewFadeIn, 16),
    PROC_WHILE(FadeInExists),

PROC_LABEL(PL_EVENT_REPLAY_CONV_IDLE),
    PROC_REPEAT(EventReplay_KeyHandler_Convs),

PROC_LABEL(PL_EVENT_REPLAY_PLAY),
    PROC_CALL(EventReplay_DisableDisplay),
    PROC_CALL_ARG(NewFadeOut, 16),
    PROC_WHILE(FadeOutExists),
    PROC_CALL_ARG(NewFadeIn, 16),
    PROC_WHILE(FadeInExists),
    PROC_CALL(EventReplay_PlayConversation),
    PROC_WHILE(EventReplay_WaitForConversation),
    PROC_CALL_ARG(NewFadeOut, 16),
    PROC_WHILE(FadeOutExists),
    PROC_CALL_ARG(NewFadeIn, 16),
    PROC_WHILE(FadeInExists),
    PROC_GOTO(PL_EVENT_REPLAY_CONV_INIT),

PROC_LABEL(PL_EVENT_REPLAY_CONV_TO_CHAPTER),
    PROC_CALL(EventReplay_DisableDisplay),
    PROC_CALL(EventReplay_RestoreChapterState),
    PROC_CALL_ARG(NewFadeOut, 16),
    PROC_WHILE(FadeOutExists),
    PROC_GOTO(PL_EVENT_REPLAY_CHAPTER_INIT),

PROC_LABEL(PL_EVENT_REPLAY_EXIT),
    PROC_CALL_ARG(NewFadeOut, 16),
    PROC_WHILE(FadeOutExists),

PROC_LABEL(PL_EVENT_REPLAY_END),
    PROC_END
};

void StartEventReplayScreen_FromPrep(struct ProcAtMenu *pproc)
{
    Proc_StartBlocking(ProcScr_EventReplay, pproc);
}
