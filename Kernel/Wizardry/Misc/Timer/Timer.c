#include "common-chax.h"
#include "constants/texts.h"
#include "kernel-lib.h"
#include "jester_headers/custom-structs.h"
#include "jester_headers/custom-functions.h"

//! FE8U = 0x0808D784
LYN_REPLACE_CHECK(GoalDisplay_Loop_Display);
void GoalDisplay_Loop_Display(struct PlayerInterfaceProc *proc)
{
    /* --- NEW: redraw timer every frame --- */

    ClearText(&proc->texts[1]);
    DrawTimeHMS(&proc->texts[1], 2, gChapterTimerSeconds);
    BG_EnableSyncByMask(BG0_SYNC_BIT);

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

    /* Choose color once */
    color = (seconds <= gChapterTimerSeconds_Initial / 2)
        ? TEXT_COLOR_SYSTEM_GOLD
        : TEXT_COLOR_SYSTEM_BLUE;

    h = seconds / 3600;
    m = k_umod((seconds / 60), 60);
    s = k_umod(seconds, 60);

    /* Hours */
    Text_InsertDrawNumberOrBlank(text, x + 3,      color, h / 10);
    Text_InsertDrawNumberOrBlank(text, x + 12,  color, k_umod(h, 10));

    Text_InsertDrawString       (text, x + 21, TEXT_COLOR_SYSTEM_WHITE, ":");

    /* Minutes */
    Text_InsertDrawNumberOrBlank(text, x + 26, color, m / 10);
    Text_InsertDrawNumberOrBlank(text, x + 35, color, k_umod(m, 10));

    Text_InsertDrawString       (text, x + 44, TEXT_COLOR_SYSTEM_WHITE, ":");

    /* Seconds */
    Text_InsertDrawNumberOrBlank(text, x + 49, color, s / 10);
    Text_InsertDrawNumberOrBlank(text, x + 58, color, k_umod(s, 10));

}

const struct ProcCmd ProcScr_ChapterTimer[] =
{
    PROC_YIELD,
    PROC_REPEAT(ChapterTimer_OnTick),
    PROC_END,
};

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

void StartChapterTimer(int seconds)
{
    /* If already started, then we return early */
    if (Proc_Find(ProcScr_ChapterTimer) || seconds > 65000)
        return;

    struct ChapterTimerProc *proc;

    gChapterTimerSeconds = seconds;
    gChapterTimerSeconds_Initial = seconds;

    proc = Proc_Start(ProcScr_ChapterTimer, PROC_TREE_3);
    proc->frameClock = 0;
}

//! FE8U = 0x0808D288
LYN_REPLACE_CHECK(GoalDisplay_Init);
void GoalDisplay_Init(struct PlayerInterfaceProc *proc)
{
    const bool isSkirmish = (GetBattleMapKind() == BATTLEMAP_KIND_SKIRMISH);
    const struct ROMChapterData *chapter = isSkirmish ? NULL : GetROMChapterStruct(gPlaySt.chapterIndex);

    int goalTextId;
    int goalWindowType;
    int turnNumber;
    int lastTurnNumber;
    char *str;
    struct Text *text;

    proc->showHideClock = 0;
    proc->isRetracting  = false;
    proc->cursorQuadrant = 0;
    proc->windowQuadrant = -1;

    InitText(&proc->texts[0], 9);
    InitText(&proc->texts[1], 9);

    StartGreenText(proc);

    ClearText(&proc->texts[0]);
    ClearText(&proc->texts[1]);

    if (GetBattleMapKind() != BATTLEMAP_KIND_SKIRMISH)
        goalTextId = GetROMChapterStruct(gPlaySt.chapterIndex)->goalWindowTextId;
    else
        goalTextId = MSG_019E; // "Defeat enemy"

    str = GetStringFromIndex(goalTextId);

    if (GetBattleMapKind() != BATTLEMAP_KIND_SKIRMISH)
        goalWindowType = GetROMChapterStruct(gPlaySt.chapterIndex)->goalWindowDataType;
    else
        goalWindowType = GOAL_TYPE_DEFEAT_ALL;

    goalWindowType = isSkirmish ? GOAL_TYPE_DEFEAT_ALL : chapter->goalWindowDataType;

    /* Since we're adding custom objectives now I'm putting this forced text string under a condition */
    if (goalWindowType < GOAL_TYPE_TIMER)
        Text_InsertDrawString(&proc->texts[0], GetStringTextCenteredPos(64, str), TEXT_COLOR_SYSTEM_WHITE, str);

    switch (goalWindowType)
    {
    case GOAL_TYPE_SEIZE:
    case GOAL_TYPE_DEFEAT_BOSS:
    case GOAL_TYPE_SPECIAL:
        proc->unitClock = 0;
        return;

    case GOAL_TYPE_DEFEAT_ALL:
        Text_InsertDrawString(
            &proc->texts[1], 16, TEXT_COLOR_SYSTEM_WHITE,
            GetStringFromIndex(MSG_01C1)           // "Left"
        );

        if (gPlaySt.chapterVisionRange != 0)
        {
            Text_InsertDrawString(
                &proc->texts[1], 40, TEXT_COLOR_SYSTEM_GRAY,
                GetStringFromIndex(MSG_0535)
            );
        }
        else
        {
            Text_InsertDrawNumberOrBlank(
                &proc->texts[1], 48, TEXT_COLOR_SYSTEM_BLUE,
                CountUnitsByFaction(FACTION_RED)
            );
        }
        break;

    case GOAL_TYPE_DEFENSE:
        turnNumber = gPlaySt.chapterTurnNumber;
        lastTurnNumber = isSkirmish
            ? -1
            : chapter->goalWindowEndTurnNumber - 1;

        if (turnNumber >= lastTurnNumber)
        {
            str = GetStringFromIndex(MSG_01C3);    // "Last Turn."
            Text_InsertDrawString(
                &proc->texts[1],
                GetStringTextCenteredPos(64, str),
                TEXT_COLOR_SYSTEM_GREEN,
                str
            );
            break;
        }

        text = &proc->texts[1];

        Text_InsertDrawNumberOrBlank(
            text, 10, TEXT_COLOR_SYSTEM_BLUE, turnNumber
        );
        Text_InsertDrawString(
            text, 18, TEXT_COLOR_SYSTEM_WHITE,
            GetStringFromIndex(MSG_0539)           // "/."
        );
        Text_InsertDrawNumberOrBlank(
            text, 34, TEXT_COLOR_SYSTEM_BLUE, lastTurnNumber
        );
        Text_InsertDrawString(
            text, 42, TEXT_COLOR_SYSTEM_WHITE,
            GetStringFromIndex(MSG_01C2)           // "Turn"
        );
        break;

    case GOAL_TYPE_TIMER:
        Text_InsertDrawString(
             &proc->texts[0], 
             GetStringTextCenteredPos(64, "Remaining:"),
             TEXT_COLOR_SYSTEM_WHITE,
             "Remaining:"
        );

        /* If the proc hasn't already begun then start it here to assure several seconds aren't lost on initialization */
        if (!Proc_Find(ProcScr_ChapterTimer))
            StartChapterTimer(gChapterTimerSeconds);

        DrawTimeHMS(&proc->texts[1], 2, gChapterTimerSeconds);

        proc->unitClock = 1;
        break;


    default:
        return;
    }

    proc->unitClock = 1;
}