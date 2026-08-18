#include "common-chax.h"
#include "constants/texts.h"
#include "kernel-lib.h"
#include "jester_headers/custom-structs.h"
#include "jester_headers/custom-functions.h"
#include "jester_headers/procs.h"

const TimerAmount chapter_timers[] = {
    {CHAPTER_L_PROLOGUE,    30},
    {CHAPTER_L_1,           0},
    {CHAPTER_L_2,           60},
    {CHAPTER_L_3,           60},
    {CHAPTER_L_4,           60},
    {CHAPTER_L_5X,          60},
    {CHAPTER_L_5,           60},
    {CHAPTER_L_6,           60},
    {CHAPTER_L_7,           60},
    {CHAPTER_L_8,           60},
    {CHAPTER_E_9,           60},
    {CHAPTER_E_10,          60},
    {CHAPTER_E_12,          60},
    {CHAPTER_E_13,          60},
    {CHAPTER_E_14,          60},
    {CHAPTER_E_15,          60},
    {CHAPTER_E_16,          60},
    {CHAPTER_E_17,          60},
    {CHAPTER_E_18,          60},
};

#define CHAPTER_TIMERS_COUNT (sizeof(chapter_timers) / sizeof(chapter_timers[0]))

u16 GetChapterTimerConfigSeconds(void)
{
    u8 chapter = gPlaySt.chapterIndex;

    if (chapter >= CHAPTER_TIMERS_COUNT)
        return 0;

    if (chapter_timers[chapter].chapter_id != chapter)
        return 0;

    return chapter_timers[chapter].time_seconds;
}

/* Fresh chapter only (not suspend). Suspend restores remaining time via LoadTimer. */
void ChapterInit_ResetChapterTimer(ProcPtr proc)
{
    (void)proc;

    Proc_EndEach(ProcScr_ChapterTimer);

    if (gpKernelDesignerConfig->goal_timer != true)
    {
        gChapterTimerSeconds = 0;
        return;
    }

    gChapterTimerSeconds = GetChapterTimerConfigSeconds();
}

//! FE8U = 0x0808D784
LYN_REPLACE_CHECK(GoalDisplay_Loop_Display);
void GoalDisplay_Loop_Display(struct PlayerInterfaceProc *proc)
{
    /* --- NEW: redraw timer every frame --- */

    if (gpKernelDesignerConfig->goal_timer == true && GetChapterTimerConfigSeconds() > 0)
    {
        ClearText(&proc->texts[1]);
        DrawTimeHMS(&proc->texts[1], 2, gChapterTimerSeconds);
        BG_EnableSyncByMask(BG0_SYNC_BIT);
    }

    /* --- Vanilla cursor / retract logic below --- */

    proc->xCursorPrev = proc->xCursor;
    proc->yCursorPrev = proc->yCursor;

    proc->xCursor = gBmSt.playerCursor.x;
    proc->yCursor = gBmSt.playerCursor.y;

    if (proc->xCursor == proc->xCursorPrev &&
        proc->yCursor == proc->yCursorPrev)
    {
        return;
    }

    if (Proc_Find(ProcScr_CamMove) == NULL)
    {
        int cursorQuadrant = GetCursorQuadrant();
        int quadrant = proc->cursorQuadrant;

        if (cursorQuadrant == quadrant)
            return;

        if ((sPlayerInterfaceConfigLut[cursorQuadrant].xGoal ==
             sPlayerInterfaceConfigLut[quadrant].xGoal) &&
            (sPlayerInterfaceConfigLut[cursorQuadrant].yGoal ==
             sPlayerInterfaceConfigLut[quadrant].yGoal))
        {
            return;
        }
    }

    proc->isRetracting = true;
    Proc_Break(proc);
}

void DrawTimeHMS(struct Text *text, int x, int seconds)
{
    int h, m, s;
    int color;

    if (seconds < 0)
        seconds = 0;

    /* If we're at half remaining time, switch to yellow colored text */
    color = (seconds <= GetChapterTimerConfigSeconds() / 2)
        ? TEXT_COLOR_SYSTEM_GOLD
        : TEXT_COLOR_SYSTEM_BLUE;

    h = seconds / 3600;
    m = k_umod((seconds / 60), 60);
    s = k_umod(seconds, 60);

    /* Hours */
    Text_InsertDrawNumberOrBlank(text, x + 3,  color, h / 10);
    Text_InsertDrawNumberOrBlank(text, x + 12, color, k_umod(h, 10));
    Text_InsertDrawString       (text, x + 21, TEXT_COLOR_SYSTEM_WHITE, ":");

    /* Minutes */
    Text_InsertDrawNumberOrBlank(text, x + 26, color, m / 10);
    Text_InsertDrawNumberOrBlank(text, x + 35, color, k_umod(m, 10));
    Text_InsertDrawString       (text, x + 44, TEXT_COLOR_SYSTEM_WHITE, ":");

    /* Seconds */
    Text_InsertDrawNumberOrBlank(text, x + 49, color, s / 10);
    Text_InsertDrawNumberOrBlank(text, x + 58, color, k_umod(s, 10));

}

void ChapterTimer_OnTick(struct ChapterTimerProc *proc)
{
    if (gChapterTimerSeconds <= 0)
        return;

    /* Freeze timer when battle animations are on */
    if (Proc_Find(ProcScr_efxHPBarColorChange))
        return;

    proc->frameClock++;

    if (proc->frameClock >= 60)
    {
        proc->frameClock = 0;
        gChapterTimerSeconds--;

        /* We've won now so we reset the timer and exit early to prevent the game over screen triggering */
        if (CheckFlag(EVFLAG_WIN))
        {
            Proc_End(proc);
            gChapterTimerSeconds = 0;
            return;
        }

        if (gChapterTimerSeconds == 0)
        {
            /* STOP this proc */
            Proc_End(proc);

            /* Queue game over safely */
            BG_Fill(gBG0TilemapBuffer, 0);
            BG_Fill(gBG1TilemapBuffer, 0);
            BG_Fill(gBG2TilemapBuffer, 0);
            BG_EnableSyncByMask(BG0_SYNC_BIT | BG1_SYNC_BIT | BG2_SYNC_BIT);
            Proc_End(Proc_Find(gProcScr_UnitDisplay_MinimugBox));
            CallGameOverEvent();
        }
    }
}

const struct ProcCmd ProcScr_ChapterTimer[] =
{
    PROC_YIELD,
    PROC_REPEAT(ChapterTimer_OnTick),
    PROC_END,
};

void StartChapterTimer(int seconds)
{
    if (gpKernelDesignerConfig->goal_timer != true)
        return;

    if (seconds <= 0)
        return;

    /* If already started, then we return early */
    if (Proc_Find(ProcScr_ChapterTimer))
        return;

    struct ChapterTimerProc *proc;

    gChapterTimerSeconds = seconds;

    proc = Proc_Start(ProcScr_ChapterTimer, PROC_TREE_3);
    proc->frameClock = 0;
}

LYN_REPLACE_CHECK(StartMuSpeedUpAnim);
void StartMuSpeedUpAnim(struct MuProc * proc)
{
    proc->sprite_anim->frameTimer    = 0;
    proc->sprite_anim->frameInterval = 0x40;

    CallDelayedArg(MuSlowDownAnimFreezeFunc, (int) proc->sprite_anim, 10);
}

static const MuStateFunc sMuStateFuncs[] = {
    [MU_STATE_NONE]       = Mu_OnStateNone,
    [MU_STATE_INACTIVE]   = Mu_OnStateDoNothing,
    [MU_STATE_MOVEMENT]   = Mu_OnStateMovement,
    [MU_STATE_SLEEPING]   = Mu_OnStateSleeping,
    [MU_STATE_UNK4]       = Mu_OnStateUnk4,
    [MU_STATE_BUMPING]    = Mu_OnStateBump,
    [MU_STATE_DISPLAY_UI] = Mu_OnStateDoNothing,
    [MU_STATE_DEATHFADE]  = Mu_OnStateDoNothing,
};

LYN_REPLACE_CHECK(Mu_OnLoop);
void Mu_OnLoop(struct MuProc * proc)
{
    // If our timer hack is active and we're under half the remainin time, increase the active unit animations speed
    if (gpKernelDesignerConfig->goal_timer == true && gChapterTimerSeconds <= GetChapterTimerConfigSeconds() / 2) {
        proc->sprite_anim->frameInterval = 0x60; // Quad speed (Default is 0x100)
    }

    if (proc->state)
    {
        if (proc->move_clock_q4 == 0)
            if (proc->state == MU_STATE_SLEEPING || proc->state == MU_STATE_MOVEMENT)
                RunMuMoveScript(proc);

        sMuStateFuncs[proc->state](proc);
    }

    if (proc->facing == MU_FACING_STANDING)
        PutMuSMS(proc);
    else
        PutMu(proc);
}

u8 EWRAM_DATA gSMSGfxBuffer[3][8*0x20*0x20] = {};

LYN_REPLACE_CHECK(SyncUnitSpriteSheet);
void SyncUnitSpriteSheet(void)
{
    int clock = GetGameClock();

    if (gpKernelDesignerConfig->goal_timer == true && gChapterTimerSeconds <= GetChapterTimerConfigSeconds() / 2) {
        clock *= 4;
    }

    int frame = k_umod(clock, 72);

    if (frame == 0)
        CpuFastCopy(gSMSGfxBuffer[0], (void*)0x06011000, sizeof(gSMSGfxBuffer[0]));

    if (frame == 32)
        CpuFastCopy(gSMSGfxBuffer[1], (void*)0x06011000, sizeof(gSMSGfxBuffer[1]));

    if (frame == 36)
        CpuFastCopy(gSMSGfxBuffer[2], (void*)0x06011000, sizeof(gSMSGfxBuffer[2]));

    if (frame == 68)
        CpuFastCopy(gSMSGfxBuffer[1], (void*)0x06011000, sizeof(gSMSGfxBuffer[1]));
}
