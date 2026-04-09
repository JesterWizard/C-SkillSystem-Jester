#include "common-chax.h"
#include "worldmap.h"
#include "kernel-lib.h"
#include "prep-skill.h"
#include "kernel/debug-kit.h"
#include "player_interface.h"
#include "jester_headers/thought_bubbles.h"

static u8 GetNextWorldMapRosterUnitId(u8 currentCharId)
{
    int i;
    int rosterCount;
    int currentIndex;

    ReorderPlayerUnitsBasedOnDeployment();
    MakePrepUnitList();
    rosterCount = PrepGetUnitAmount();

    currentIndex = UnitGetIndexInPrepList(currentCharId);

    if (currentIndex < 0 || currentIndex >= rosterCount)
        currentIndex = -1;

    for (i = 1; i <= rosterCount; ++i)
    {
        int cycleIndex = currentIndex + i;
        struct Unit * prepUnit;
        u8 nextCharId;
        struct Unit * unit;

        if (cycleIndex >= rosterCount)
            cycleIndex -= rosterCount;

        prepUnit = GetUnitFromPrepList(cycleIndex);

        if (!UNIT_IS_VALID(prepUnit))
            continue;

        nextCharId = prepUnit->pCharacterData->number;
        unit = GetUnitFromCharId(nextCharId);

        if (!UNIT_IS_VALID(unit))
            continue;

        if (!IsUnitInCurrentRoster(unit))
            if (!UNIT_IS_VALID(prepUnit))
            continue;

        return nextCharId;
    }

    return currentCharId;
}

static const WorldMapThoughtBubbleEntryGraphics * GetWorldMapThoughtBubbleForUnit(int chapterIndex, int unitId)
{
    if (chapterIndex < 0)
        return NULL;

    switch (unitId)
    {
        case CHARACTER_EIRIKA:
            return &WorldMapThoughtBubbleEirika[chapterIndex];

        case CHARACTER_SETH:
            return &WorldMapThoughtBubbleSeth[chapterIndex];

        default:
            return NULL;
    }
}

static void WorldMapThoughtBubble_Init(struct MenuProc * menuProc)
{
    const WorldMapThoughtBubbleEntryGraphics * bubbleEntry;

    bubbleEntry = GetWorldMapThoughtBubbleForUnit(gPlaySt.chapterIndex, gGMData.units[0].id);

    if (bubbleEntry == NULL)
        return;

    Decompress(bubbleEntry->bubble[0], gGenericBuffer);
    Copy2dChr(gGenericBuffer, (void *)0x6015880, 8, 8);

    Decompress(bubbleEntry->bubble[1], gGenericBuffer);
    Copy2dChr(gGenericBuffer, (void *)0x6015980, 8, 8);
}

static void WorldMapThoughtBubble_Loop(struct MenuProc * menuProc)
{
    if (GetWorldMapThoughtBubbleForUnit(gPlaySt.chapterIndex, gGMData.units[0].id) == NULL)
        return;

    PutSprite(4, 10, 10, gObject_64x64, TILEREF(0x2C4, 0x0));
    PutSprite(4, 74, 10, gObject_64x64, TILEREF(0x2CC, 0x0));
}

struct ProcCmd const sProcScr_WorldMapThoughtBubble[] = {
    PROC_NAME("WorldMapThoughtBubble"),
    PROC_CALL(WorldMapThoughtBubble_Init),
    PROC_REPEAT(WorldMapThoughtBubble_Loop),
    PROC_END,
};

void StartWorldMapThoughtBubble(struct MenuProc * menuProc)
{
    Proc_Start(sProcScr_WorldMapThoughtBubble, menuProc);
}

extern u16 gUnknown_0201B7DA[];

//! FE8U = 0x080B96F8
LYN_REPLACE_CHECK(WorldMap_LoopExt);
void WorldMap_LoopExt(struct WorldMapMainProc * proc)
{
    NoCashGBAPrintf("gPlaySt.chapterIndex: %d\n", gPlaySt.chapterIndex);
    int nodeId;

    int x = gGMData.ix;
    int y = gGMData.iy;

    if (gpKernelDesignerConfig->world_map_thought_bubbles == true)
    {
        if (gKeyStatusPtr->newKeys & R_BUTTON)
        {
            u8 currentUnitId = (u8)gGMData.units[0].id;
            u8 nextUnitId = GetNextWorldMapRosterUnitId(currentUnitId);

            if (nextUnitId != 0)
            {
                gGMData.units[0].id = nextUnitId;
                sub_80B8FEC(proc);
                sub_80B90CC(proc);

                Proc_EndEach(ProcScr_GMapPlayerInterface);
                StartWorldMapPlayerInterface((struct Proc *)proc);
            }
        }
    }

    if (gKeyStatusPtr->newKeys & SELECT_BUTTON)
    {
        if (gGMData.state.bits.state_2)
            gGMData.state.bits.state_2 = 0;
        else
            gGMData.state.bits.state_2 = 1;
    }

    nodeId = GetNodeAtPosition(proc->gm_icon, x >> 8, y >> 8, 0, 0);
    if (nodeId >= 0)
    {
        if (gKeyStatusPtr->newKeys & A_BUTTON)
        {
            if (sub_80B92D0(proc, nodeId) != 0)
            {
                PlaySoundEffect(SONG_SE_SYS_WINDOW_SELECT1);
                return;
            }
        }
        else if (gKeyStatusPtr->newKeys & L_BUTTON)
        {
            nodeId = sub_80B955C(proc, nodeId);
            if (nodeId >= 0)
            {
                sub_80B961C(proc, nodeId);
                return;
            }
        }
    }
    else
    {
        if (gKeyStatusPtr->newKeys & L_BUTTON)
        {
            sub_80B961C(proc, sub_80B95B0());
            return;
        }
    }

    if (gKeyStatusPtr->newKeys & A_BUTTON)
    {
        Proc_Goto(proc, 5);
    }
    else
    {
        WmMain_MoveCursor(proc);
        gGMData.ix = x;
        gGMData.iy = y;
        WmMain_MoveCamera(proc);
    }
    return;
}