#include "common-chax.h"
#include "constants/texts.h"
#include "kernel-lib.h"
#include "jester_headers/custom-structs.h"
#include "jester_headers/custom-functions.h"
#include "jester_headers/procs.h"


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
    if (gpKernelDesignerConfig->goal_timer == true && GetChapterTimerConfigSeconds() > 0)
        goalWindowType = GOAL_TYPE_TIMER;

    switch (goalWindowType)
    {
    case GOAL_TYPE_SEIZE:
        Text_InsertDrawString(&proc->texts[0], GetStringTextCenteredPos(64, GetStringFromIndex(MSG_019F)), TEXT_COLOR_SYSTEM_WHITE, GetStringFromIndex(MSG_019F));
        proc->unitClock = 0;
        return;
    case GOAL_TYPE_DEFEAT_BOSS:
        Text_InsertDrawString(&proc->texts[0], GetStringTextCenteredPos(64, GetStringFromIndex(MSG_019D)), TEXT_COLOR_SYSTEM_WHITE, GetStringFromIndex(MSG_019D));
        proc->unitClock = 0;
        return;
    case GOAL_TYPE_SPECIAL:
        proc->unitClock = 0;
        return;

    case GOAL_TYPE_DEFEAT_ALL:
        Text_InsertDrawString(&proc->texts[0], GetStringTextCenteredPos(64, str), TEXT_COLOR_SYSTEM_WHITE, str);
        Text_InsertDrawString(&proc->texts[1], 16, TEXT_COLOR_SYSTEM_WHITE, GetStringFromIndex(MSG_01C1));

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

        /*
         * Always seed from the chapter table unless we are clearly resuming
         * with a remaining time in range (suspend LoadTimer path).
         */
        if (!Proc_Find(ProcScr_ChapterTimer))
        {
            u16 configured = GetChapterTimerConfigSeconds();
            u16 remaining = gChapterTimerSeconds;

            if (configured == 0)
            {
                gChapterTimerSeconds = 0;
            }
            else
            {
                if (remaining == 0 || remaining > configured)
                    remaining = configured;

                gChapterTimerSeconds = remaining;
                StartChapterTimer(remaining);
            }
        }

        DrawTimeHMS(&proc->texts[1], 2, gChapterTimerSeconds);

        proc->unitClock = 1;
        break;

    case GOAL_TYPE_ESCAPE:
        Text_InsertDrawString(&proc->texts[0], GetStringTextCenteredPos(64, "Escape"), TEXT_COLOR_SYSTEM_WHITE, "Escape");
        Text_InsertDrawString(&proc->texts[1], GetStringTextCenteredPos(64, "With all"), TEXT_COLOR_SYSTEM_WHITE, "With all");
        break;

    default:
        return;
    }

    proc->unitClock = 1;
}