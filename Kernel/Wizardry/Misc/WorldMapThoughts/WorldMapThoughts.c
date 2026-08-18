#include "common-chax.h"
#include "worldmap.h"
#include "kernel-lib.h"
#include "prep-skill.h"
#include "kernel/debug-kit.h"
#include "player_interface.h"
#include "jester_headers/custom-functions.h"
#include "jester_headers/thought_bubbles.h"

#define WMTB_IDX_PROLOGUE 0
#define WMTB_IDX_01 1
#define WMTB_IDX_02 2
#define WMTB_IDX_03 3
#define WMTB_IDX_04 4
#define WMTB_IDX_05X 5
#define WMTB_IDX_05 6
#define WMTB_IDX_06 7
#define WMTB_IDX_07 8
#define WMTB_IDX_08 9
#define WMTB_IDX_09 10
enum {
    WMTB_COUNT = 11,
};

#define WMTB_ENTRY(chapter, unit) [WMTB_IDX_##chapter] = { .bubble = Gfx_Chapter_##chapter##_Thought_Bubble_##unit }
#define WMTB_TABLE(unit) static const WorldMapThoughtBubbleEntryGraphics WorldMapThoughtBubble##unit[WMTB_COUNT] = {

WMTB_TABLE(Artur)
    WMTB_ENTRY(05, Artur),
    WMTB_ENTRY(06, Artur),
    WMTB_ENTRY(07, Artur),
    WMTB_ENTRY(09, Artur),
};

WMTB_TABLE(Colm)
    WMTB_ENTRY(04, Colm),
    WMTB_ENTRY(05, Colm),
    WMTB_ENTRY(06, Colm),
    WMTB_ENTRY(07, Colm),
    WMTB_ENTRY(09, Colm),
};

const WorldMapThoughtBubbleEntryGraphics WorldMapThoughtBubbleEirika[] = {
    WMTB_ENTRY(02, Eirika),
    WMTB_ENTRY(03, Eirika),
    WMTB_ENTRY(04, Eirika),
    WMTB_ENTRY(05, Eirika),
    WMTB_ENTRY(06, Eirika),
    WMTB_ENTRY(07, Eirika),
    WMTB_ENTRY(09, Eirika),
};

WMTB_TABLE(Ephraim)
    WMTB_ENTRY(09, Ephraim),
};

WMTB_TABLE(Forde)
    WMTB_ENTRY(09, Forde),
};

WMTB_TABLE(Franz)
    WMTB_ENTRY(02, Franz),
    WMTB_ENTRY(03, Franz),
    WMTB_ENTRY(04, Franz),
    WMTB_ENTRY(05, Franz),
    WMTB_ENTRY(06, Franz),
    WMTB_ENTRY(07, Franz),
    WMTB_ENTRY(09, Franz),
};

WMTB_TABLE(Garcia)
    WMTB_ENTRY(05, Garcia),
    WMTB_ENTRY(06, Garcia),
    WMTB_ENTRY(07, Garcia),
    WMTB_ENTRY(09, Garcia),
};

WMTB_TABLE(Gilliam)
    WMTB_ENTRY(02, Gilliam),
    WMTB_ENTRY(03, Gilliam),
    WMTB_ENTRY(04, Gilliam),
    WMTB_ENTRY(05, Gilliam),
    WMTB_ENTRY(06, Gilliam),
    WMTB_ENTRY(07, Gilliam),
    WMTB_ENTRY(09, Gilliam),
};

WMTB_TABLE(Joshua)
    WMTB_ENTRY(06, Joshua),
    WMTB_ENTRY(07, Joshua),
    WMTB_ENTRY(09, Joshua),
};

WMTB_TABLE(Kyle)
    WMTB_ENTRY(09, Kyle),
};

WMTB_TABLE(Lute)
    WMTB_ENTRY(05, Lute),
    WMTB_ENTRY(06, Lute),
    WMTB_ENTRY(07, Lute),
    WMTB_ENTRY(09, Lute),
};

WMTB_TABLE(Moulder)
    WMTB_ENTRY(02, Moulder),
    WMTB_ENTRY(03, Moulder),
    WMTB_ENTRY(04, Moulder),
    WMTB_ENTRY(05, Moulder),
    WMTB_ENTRY(06, Moulder),
    WMTB_ENTRY(07, Moulder),
    WMTB_ENTRY(09, Moulder),
};

WMTB_TABLE(Natasha)
    WMTB_ENTRY(06, Natasha),
    WMTB_ENTRY(07, Natasha),
    WMTB_ENTRY(09, Natasha),
};

WMTB_TABLE(Neimi)
    WMTB_ENTRY(04, Neimi),
    WMTB_ENTRY(05, Neimi),
    WMTB_ENTRY(06, Neimi),
    WMTB_ENTRY(07, Neimi),
    WMTB_ENTRY(09, Neimi),
};

WMTB_TABLE(Ross)
    WMTB_ENTRY(04, Ross),
    WMTB_ENTRY(05, Ross),
    WMTB_ENTRY(06, Ross),
    WMTB_ENTRY(07, Ross),
    WMTB_ENTRY(09, Ross),
};

WMTB_TABLE(Seth)
    WMTB_ENTRY(02, Seth),
    WMTB_ENTRY(03, Seth),
    WMTB_ENTRY(04, Seth),
    WMTB_ENTRY(05, Seth),
    WMTB_ENTRY(06, Seth),
    WMTB_ENTRY(07, Seth),
    WMTB_ENTRY(09, Seth),
};

WMTB_TABLE(Tana)
    WMTB_ENTRY(02, Tana),
    WMTB_ENTRY(03, Tana),
    WMTB_ENTRY(04, Tana),
    WMTB_ENTRY(05, Tana),
    WMTB_ENTRY(06, Tana),
    WMTB_ENTRY(07, Tana),
    WMTB_ENTRY(09, Tana),
};

#undef WMTB_TABLE
#undef WMTB_ENTRY
#undef WMTB_IDX_09
#undef WMTB_IDX_08
#undef WMTB_IDX_07
#undef WMTB_IDX_06
#undef WMTB_IDX_05
#undef WMTB_IDX_05X
#undef WMTB_IDX_04
#undef WMTB_IDX_03
#undef WMTB_IDX_02
#undef WMTB_IDX_01
#undef WMTB_IDX_PROLOGUE

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
    const WorldMapThoughtBubbleEntryGraphics *entry;

    if (chapterIndex < 0 || chapterIndex >= WMTB_COUNT)
        return NULL;

    switch (unitId)
    {
        case CHARACTER_ARTUR:
            entry = &WorldMapThoughtBubbleArtur[chapterIndex];
            break;

        case CHARACTER_COLM:
            entry = &WorldMapThoughtBubbleColm[chapterIndex];
            break;

        case CHARACTER_EIRIKA:
            entry = &WorldMapThoughtBubbleEirika[chapterIndex];
            break;

        case CHARACTER_SETH:
            entry = &WorldMapThoughtBubbleSeth[chapterIndex];
            break;

        case CHARACTER_EPHRAIM:
            entry = &WorldMapThoughtBubbleEphraim[chapterIndex];
            break;

        case CHARACTER_FORDE:
            entry = &WorldMapThoughtBubbleForde[chapterIndex];
            break;

        case CHARACTER_FRANZ:
            entry = &WorldMapThoughtBubbleFranz[chapterIndex];
            break;

        case CHARACTER_GARCIA:
            entry = &WorldMapThoughtBubbleGarcia[chapterIndex];
            break;

        case CHARACTER_GILLIAM:
            entry = &WorldMapThoughtBubbleGilliam[chapterIndex];
            break;

        case CHARACTER_JOSHUA:
            entry = &WorldMapThoughtBubbleJoshua[chapterIndex];
            break;

        case CHARACTER_KYLE:
            entry = &WorldMapThoughtBubbleKyle[chapterIndex];
            break;

        case CHARACTER_LUTE:
            entry = &WorldMapThoughtBubbleLute[chapterIndex];
            break;

        case CHARACTER_MOULDER:
            entry = &WorldMapThoughtBubbleMoulder[chapterIndex];
            break;

        case CHARACTER_NATASHA:
            entry = &WorldMapThoughtBubbleNatasha[chapterIndex];
            break;

        case CHARACTER_NEIMI:
            entry = &WorldMapThoughtBubbleNeimi[chapterIndex];
            break;

        case CHARACTER_ROSS:
            entry = &WorldMapThoughtBubbleRoss[chapterIndex];
            break;

        case CHARACTER_TANA:
            entry = &WorldMapThoughtBubbleTana[chapterIndex];
            break;

        default:
            return NULL;
    }

    if (entry->bubble == NULL)
        return NULL;

    return entry;
}

#define WMTB_OBJ_CHR 0x300
#define WMTB_TILE_WIDTH 16
#define WMTB_TILE_HEIGHT 8

static void WorldMapThoughtBubble_Init(struct MenuProc * menuProc)
{
    const WorldMapThoughtBubbleEntryGraphics * bubbleEntry;

    bubbleEntry = GetWorldMapThoughtBubbleForUnit(gPlaySt.chapterIndex, gGMData.units[0].id);

    if (bubbleEntry == NULL)
        return;

    Decompress(bubbleEntry->bubble, gGenericBuffer);
    Copy2dChr(
        gGenericBuffer,
        OBJ_VRAM0 + WMTB_OBJ_CHR * CHR_SIZE,
        WMTB_TILE_WIDTH,
        WMTB_TILE_HEIGHT);
}

static void WorldMapThoughtBubble_Loop(struct MenuProc * menuProc)
{
    if (GetWorldMapThoughtBubbleForUnit(gPlaySt.chapterIndex, gGMData.units[0].id) == NULL)
        return;

    PutSprite(4, 10, 10, gObject_64x64, TILEREF(WMTB_OBJ_CHR, 0x0));
    PutSprite(4, 74, 10, gObject_64x64, TILEREF(WMTB_OBJ_CHR + 8, 0x0));
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
                if (gpKernelDesignerConfig->world_map_thought_bubbles == true)
                {
                    WorldMap_CenterCamera(proc, nodeId);
                }
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

void WorldMap_CenterCamera(ProcPtr proc, int nodeid)
{
    s16 x;
    s16 y;
    int unk;
    s16 xOut;
    s16 yOut;
    s16 xCamera;
    s16 yCamera;

    if (gGMData.units[0].location != nodeid)
        return;

    x = gWMNodeData[nodeid].x;
    y = gWMNodeData[nodeid].y;

    gGMData.ix = x << 8;
    gGMData.iy = y << 8;

    GetWMCenteredCameraPosition(x, y, &xOut, &yOut);

    *&xCamera = gGMData.xCamera;
    *&yCamera = gGMData.yCamera;

    unk = sub_80C0834(xCamera, yCamera, xOut, yOut, 4);
    if (unk < 0)
    {
        unk = unk + 0x1FF;
    }

    unk = (unk >> 9) + 6;
    if (unk > 10)
    {
        unk = 10;
    }

    StartGmScroll(-1, -1, xOut, yOut, unk, 0);

    gGMData.sprite_disp = 0;
}