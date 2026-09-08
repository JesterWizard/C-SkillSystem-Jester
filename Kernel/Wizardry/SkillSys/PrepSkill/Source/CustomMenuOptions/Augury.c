#include "common-chax.h"
#include "utf8.h"
#include "kernel-lib.h"
#include "constants/texts.h"
#include "jester_headers/custom-functions.h"
#include "jester_headers/custom-structs.h"

#define BASE_VISIBLE_COUNT 5

static const struct Auguries gAuguryTable[] = {
    {MSG_AUGURY_DEFAULT, 0},
    {MSG_AUGURY_DEFAULT, 0},
    {MSG_AUGURY_DEFAULT, 0},
    {MSG_AUGURY_DEFAULT, 0},
    {MSG_AUGURY_CHAPTER_04, SONG_LAUGHTER},
    {MSG_AUGURY_DEFAULT, 0}, // Chapter 5x
    {MSG_AUGURY_DEFAULT, 0}, // Chapter 5
};

static void PrepInitGfx_AUGURY(struct ProcPrepUnit * proc) {
    SetupBackgrounds(NULL);
    BG_Fill(BG_GetMapBuffer(0), 0);
    BG_Fill(BG_GetMapBuffer(1), 0);
    BG_Fill(BG_GetMapBuffer(2), 0);
    RestartMuralBackground();
    BG_EnableSyncByMask(4);
}

void CallAuguryEvent(struct ProcPrepUnit * proc) {
    SetInitTalkTextFont();

    /* Check if a song ID is set for this conversation, otherwise choose a random one */
    if (gAuguryTable[gPlaySt.chapterIndex].songId == 0)
        StartBgm(NextRN_N(0x46), 0);
    else
        StartBgm(gAuguryTable[gPlaySt.chapterIndex].songId, 0);

    StartTalkExt(0, 0, GetStringFromIndex(gAuguryTable[gPlaySt.chapterIndex].bodyTextId), proc);
    SetTalkPrintColor(1);
    ApplyPalette(Pal_TalkBubble, 3);
}

static int WaitForBaseConversation(struct ProcPrepUnit * proc) {
    return IsTalkActive();
}

/* Start the combat prep song again when we exit this menu */
static void PrepItemList_OnEnd_AUGURY(struct ProcPrepUnit * proc) {
    struct ProcAtMenu *pproc = proc->proc_parent;
    pproc->state = 1;
    ResetFaces();
    EndMuralBackground_();
    ClearBg0Bg1();
    StartBgm(SONG_COMBAT_PREPARATION, 0);
}

struct ProcCmd const ProcScr_PrepItemListScreen_AUGURY[] = {
    PROC_NAME("PrepScreen_AUGURY"),
    PROC_YIELD,
    PROC_SET_END_CB(PrepItemList_OnEnd_AUGURY),

PROC_LABEL(PL_AUGURY_INIT),
    PROC_CALL(PrepInitGfx_AUGURY),
	PROC_CALL_ARG(NewFadeIn, 16),
    PROC_WHILE(FadeInExists),

PROC_LABEL(PL_AUGURY_EVENT),
    PROC_CALL(CallAuguryEvent),
    PROC_WHILE(WaitForBaseConversation),
    PROC_CALL_ARG(NewFadeOut, 16),
    PROC_WHILE(FadeOutExists),

PROC_LABEL(PL_AUGURY_END),
    PROC_END
};

void StartAuguryScreen_FromPrep(struct ProcAtMenu *pproc) {
    Proc_StartBlocking(ProcScr_PrepItemListScreen_AUGURY, pproc);
}