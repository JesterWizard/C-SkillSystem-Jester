#include "gbafe.h"
#include "common-chax.h"
#include "kernel-lib.h"
#include "constants/texts.h"
#include "event-rework.h"

// Arena Limits. Hack by Zeta/Gilgamesh
// Requires FE-CLIB
// Free to use/modify

#define ARENA_WIN 0x1
#define ARENA_LOSS 0x2
#define ARENA_OK 0x1
#define CANT_ARENA 0x3
#define CENTERTUTORIALTEXTBOX SVAL(0xB,0xFFFFFFFF)

STATIC_DECLAR const EventScr EventScr_ArenaClosed[] = {
    EVBIT_MODIFY(0x4)
    TUTORIALTEXTBOXSTART
	CENTERTUTORIALTEXTBOX
    TEXTSHOW(MSG_ArenaClosedString) // Display chosen text ID
    TEXTEND
    NOFADE
    ENDA
};

STATIC_DECLAR const EventScr EventScr_ArenaLevelLimit[] = {
    EVBIT_MODIFY(0x4)
    TUTORIALTEXTBOXSTART
	CENTERTUTORIALTEXTBOX
    TEXTSHOW(MSG_ArenaLevelLimitString) // Display chosen text ID
    TEXTEND
    NOFADE
    ENDA
};

STATIC_DECLAR const EventScr EventScr_UnpromotedOnly[] = {
    EVBIT_MODIFY(0x4)
    TUTORIALTEXTBOXSTART
	CENTERTUTORIALTEXTBOX
    TEXTSHOW(MSG_UnpromotedOnlyString) // Display chosen text ID
    TEXTEND
    NOFADE
    ENDA
};

struct _ArenaLimitTable
{
    u8 ChapterID;
    u8 LevelLimit;
    u8 TurnLimit;
};

extern struct _ArenaLimitTable ArenaLimitTable[];

extern char* FreeSpaceRAM;
extern void* ArenaLimitEvent;

extern u8 NonLethalArena;
extern u8 MaxLevel;

u8 CheckArenaLimits(void)
{
    if (gpKernelDesignerConfig->arena_limits != true)
    {
        return ARENA_OK;
    }
    
	ProcPtr playerPhaseProc = Proc_Find(gProcScr_PlayerPhase); 
    for (int x = 0; ArenaLimitTable[x].ChapterID != 0xFF; x++)
    {
        if (ArenaLimitTable[x].ChapterID == gPlaySt.chapterIndex)
        {
            // Turn Limit Check
            if (ArenaLimitTable[x].TurnLimit)
            {
                if (gPlaySt.chapterTurnNumber > ArenaLimitTable[x].TurnLimit)
                {
					gEventSlots[EVT_SLOT_7] = 1;
					KernelCallEvent(EventScr_ArenaClosed, EV_EXEC_CUTSCENE, playerPhaseProc);
                    return CANT_ARENA;
                }
            }
            
            // Level Limit Check
            if (ArenaLimitTable[x].LevelLimit)
            {
                u8 TempLevel = gActiveUnit->level;
                if (gActiveUnit->pClassData->attributes & CA_PROMOTED)
					TempLevel += MaxLevel;
                
                if (TempLevel > ArenaLimitTable[x].LevelLimit)
                {
                    if (TempLevel > MaxLevel && MaxLevel > 20)
						gEventSlots[EVT_SLOT_7] = 3;
                    else if (TempLevel > MaxLevel)
						gEventSlots[EVT_SLOT_7] = 2;
					else
						return ARENA_OK;


					switch (gEventSlots[EVT_SLOT_7]) {
					case 2:
						KernelCallEvent(EventScr_ArenaLevelLimit, EV_EXEC_CUTSCENE, playerPhaseProc);
						break;
					case 4:
						KernelCallEvent(EventScr_UnpromotedOnly, EV_EXEC_CUTSCENE, playerPhaseProc);
						break;
					}
                    
                    return CANT_ARENA;

                }
            }
    
            return ARENA_OK;
        }
    }
    
    return ARENA_OK;
}

// Fixed function using the gArenaState pointer as intended
void KillUnitIfNoHealth(struct Unit* unit)
{
    if (GetUnitCurrentHp(unit) == 0)
    {
        if (gArenaState.result == ARENA_LOSS && NonLethalArena)
        {
            unit->curHP = 1;
        }
    }
}

// Fixed function using the gArenaState pointer as intended
LYN_REPLACE_CHECK(DidUnitDie);
bool DidUnitDie(struct Unit* unit)
{
    if (GetUnitCurrentHp(unit) == 0)
    {
        if (gArenaState.result == ARENA_LOSS && NonLethalArena)
        {
            unit->curHP = 1;
            return TRUE;
        }
        else
            return FALSE;
    }
    else
        return TRUE;
}