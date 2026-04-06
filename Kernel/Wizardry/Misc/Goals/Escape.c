#include "common-chax.h"
#include "event-rework.h"
#include "kernel-lib.h"
#include "action-expa.h"
#include "constants/texts.h"
#include "jester_headers/custom-functions.h"
#include "jester_headers/custom-arrays.h"
#include "jester_headers/macros.h"
#include "jester_headers/procs.h"

STATIC_DECLAR const EventScr EventScr_PostAction_Escape[] = {
    ASMC(IsEventTileASMC)
    SVAL(EVT_SLOT_7, 1)
    BNE(0x0, EVT_SLOT_C, EVT_SLOT_7)
    TEXT(Chapter_00_Scene_03_Convo_05)
    ASMC(RemoveActiveUnitASMC)
    CHECK_PLAYERS
    SVAL(EVT_SLOT_7, 0)
    BNE(0x0, EVT_SLOT_C, EVT_SLOT_7)
    CALL(EventScr_Ending_Chapter_00)

LABEL(0x0)
    NOFADE
    ENDA
};

bool HasEscapeObjective(int chapterIndex)
{
    switch (chapterIndex)
    {
    case CHAPTER_L_PROLOGUE:
    case CHAPTER_L_1:
    case CHAPTER_L_2:
    case CHAPTER_L_3:
    case CHAPTER_L_4:
    case CHAPTER_L_5:
    case CHAPTER_L_5X:
    case CHAPTER_L_6:
    case CHAPTER_L_7:
    case CHAPTER_L_8:
    case CHAPTER_E_9:
    case CHAPTER_E_10:
    case CHAPTER_E_11:
    case CHAPTER_E_12:
    case CHAPTER_E_13:
    case CHAPTER_E_14:
    case CHAPTER_E_15:
    case CHAPTER_E_16:
    case CHAPTER_E_17:
    case CHAPTER_E_18:
        return true;

    default:
        return false;
    }
}

bool IsEscapeTile(int chapterIndex, int x, int y)
{
    switch (chapterIndex)
    {
    case CHAPTER_L_PROLOGUE:
    case CHAPTER_L_1:
    case CHAPTER_L_2:
    case CHAPTER_L_3:
    case CHAPTER_L_4:
    case CHAPTER_L_5:
    case CHAPTER_L_5X:
    case CHAPTER_L_6:
    case CHAPTER_L_7:
    case CHAPTER_L_8:
    case CHAPTER_E_9:
    case CHAPTER_E_10:
    case CHAPTER_E_11:
    case CHAPTER_E_12:
    case CHAPTER_E_13:
    case CHAPTER_E_14:
    case CHAPTER_E_15:
    case CHAPTER_E_16:
    case CHAPTER_E_17:
    case CHAPTER_E_18:
        return x == 3 && y == 3;

    default:
        return false;
    }
}

void CheckPlayersRemainingASMC(void)
{
    int count = 0;

    FOR_UNITS_VALID_FACTION(FACTION_BLUE, unit, {
        if (unit->state & (US_HIDDEN | US_NOT_DEPLOYED))
            continue;

        count++;
    })

    gEventSlots[EVT_SLOT_C] = count;
}

void RemoveActiveUnitASMC(void)
{
    if (gActiveUnit == NULL)
        return;

    struct Unit * unit = GetUnit(gActiveUnit->index);
    gBmMapUnit[unit->yPos][unit->xPos] = 0;
    HideUnitSprite(unit);
    unit->state |= US_HIDDEN | US_NOT_DEPLOYED | US_UNSELECTABLE;
    unit->state &= ~US_DEAD;
    unit->xPos = -1;
    unit->yPos = -1;
    int faction = UNIT_FACTION(unit);
    int i, amount = GetFactionUnitAmount(faction);

	for (i = 1; i <= amount; i++)
    {
		struct Unit * testUnit = GetUnit(faction + i);

		if (testUnit->pCharacterData != NULL && !(testUnit->state &= US_HIDDEN))
        {
            if (testUnit->pCharacterData->number != gActiveUnit->pCharacterData->number)
            {
			    gActiveUnit = testUnit;
                break;
            }
        }
	}

    gActionData.xMove = gActiveUnit->xPos;
    gActionData.yMove = gActiveUnit->yPos;

    gActionDataExpa.refrain_action = true;
    EndAllMus();
}

void IsEventTileASMC(void)
{
    struct Unit *unit = GetUnit(gActionData.subjectIndex);

    if (unit == NULL)
        unit = gActiveUnit;

    if (unit != NULL && IsEscapeTile(gPlaySt.chapterIndex, unit->xPos, unit->yPos))
        gEventSlots[EVT_SLOT_C] = 1;
    else
        gEventSlots[EVT_SLOT_C] = 0;
}

bool PostAction_Escape(ProcPtr proc)
{
    if (!UNIT_IS_VALID(gActiveUnit))
        return false;

    if (gActionData.unitActionType != UNIT_ACTION_WAIT && gActionData.unitActionType != UNIT_ACTION_FORCE_WAIT)
        return false;

    if (!IsEscapeTile(gPlaySt.chapterIndex, gActiveUnit->xPos, gActiveUnit->yPos))
        return false;

    KernelCallEvent(EventScr_PostAction_Escape, EV_EXEC_CUTSCENE, proc);
    return true;
}