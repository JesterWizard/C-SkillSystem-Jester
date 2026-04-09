#include "common-chax.h"
#include "worldmap.h"
#include "kernel-lib.h"
#include "prep-skill.h"
#include "kernel/debug-kit.h"
#include "player_interface.h"
#include "jester_headers/custom-functions.h"

extern int sub_80B955C(struct WorldMapMainProc * proc, int nodeId);
extern void sub_80B961C(struct WorldMapMainProc * proc, int nodeId);
extern void sub_80B8FEC(struct WorldMapMainProc * proc);
extern void sub_80B90CC(struct WorldMapMainProc * proc);
extern void sub_80BEF20(struct GMapPIProc * proc, int nodeId);
extern void sub_80BE638(struct GMapPIProc * proc, struct Unit * unit);
extern void sub_80BE5F8(u16 * src, struct Unit * unit);
extern void PutGMapPICharName(struct GMapPIProc * proc, int pid);
extern void PutGMapPIFace(struct GMapPIProc * proc);
extern void DrawUnitMapUi(struct PlayerInterfaceProc * proc, struct Unit * unit);

extern s8 IsUnitInCurrentRoster(struct Unit * unit);
extern struct ProcCmd ProcScr_GMapPlayerInterface[];

void WorldMap_LoopExt(struct WorldMapMainProc * proc);
extern struct ProcCmd gProcScr_HelpBox[];

static u8 GetNextWorldMapRosterUnitId(u8 currentCharId)
{
    int i;
    int rosterCount;
    int currentIndex;

    ReorderPlayerUnitsBasedOnDeployment();
    MakePrepUnitList();
    rosterCount = PrepGetUnitAmount();

    Debugf("rosterCount=%d currentCharId=%d", rosterCount, currentCharId);

    currentIndex = UnitGetIndexInPrepList(currentCharId);

    Debugf("currentIndex=%d", currentIndex);

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

        Debugf("advance idx=%d cycleIndex=%d prepUnit=%p pid=%d", i, cycleIndex, prepUnit, prepUnit ? prepUnit->pCharacterData->number : 0);

        if (!UNIT_IS_VALID(prepUnit))
            continue;

        nextCharId = prepUnit->pCharacterData->number;
        unit = GetUnitFromCharId(nextCharId);

        Debugf("resolved unit=%p valid=%d pid=%d", unit, UNIT_IS_VALID(unit), unit ? unit->pCharacterData->number : 0);

        if (!UNIT_IS_VALID(unit))
            continue;

        if (!IsUnitInCurrentRoster(unit))
            if (!UNIT_IS_VALID(prepUnit))
            continue;

        return nextCharId;
    }

    return currentCharId;
}

typedef struct {
    u8 chapterIndex;
    u8 * const bubble[2];
} WorldMapThoughtBubbleEntryGraphics;

static const WorldMapThoughtBubbleEntryGraphics WorldMapThoughtBubble[] = {
    {
        .chapterIndex = CHAPTER_L_2,
        .bubble = {
            Gfx_Chapter_02_Thought_Bubble_Eirika_Left,
            Gfx_Chapter_02_Thought_Bubble_Eirika_Right,
        },
    },
    {
        .chapterIndex = CHAPTER_L_3,
        .bubble = {
            Gfx_Chapter_03_Thought_Bubble_Eirika_Left,
            Gfx_Chapter_03_Thought_Bubble_Eirika_Right,
        },
    },
    {
        .chapterIndex = CHAPTER_L_4,
        .bubble = {
            Gfx_Chapter_04_Thought_Bubble_Eirika_Left,
            Gfx_Chapter_04_Thought_Bubble_Eirika_Right,
        },
    },
    {
        .chapterIndex = CHAPTER_L_5,
        .bubble = {
            Gfx_Chapter_05_Thought_Bubble_Eirika_Left,
            Gfx_Chapter_05_Thought_Bubble_Eirika_Right,
        },
    },
    {
        .chapterIndex = CHAPTER_L_6,
        .bubble = {
            Gfx_Chapter_06_Thought_Bubble_Eirika_Left,
            Gfx_Chapter_06_Thought_Bubble_Eirika_Right,
        },
    },
    {
        .chapterIndex = CHAPTER_L_7,
        .bubble = {
            Gfx_Chapter_07_Thought_Bubble_Eirika_Left,
            Gfx_Chapter_07_Thought_Bubble_Eirika_Right,
        },
    },
    {
        .chapterIndex = CHAPTER_E_9,
        .bubble = {
            Gfx_Chapter_09_Thought_Bubble_Eirika_Left,
            Gfx_Chapter_09_Thought_Bubble_Eirika_Right,
        },
    },
};

static void WorldMapThoughtBubble_Init(struct MenuProc * menuProc)
{
    unsigned i;

    for (i = 0; i < ARRAY_COUNT(WorldMapThoughtBubble); ++i)
    {
        if (WorldMapThoughtBubble[i].chapterIndex != gPlaySt.chapterIndex)
            continue;

        Decompress(WorldMapThoughtBubble[i].bubble[0], gGenericBuffer);
        Copy2dChr(gGenericBuffer, (void *)0x6015880, 8, 8);

        Decompress(WorldMapThoughtBubble[i].bubble[1], gGenericBuffer);
        Copy2dChr(gGenericBuffer, (void *)0x6015980, 8, 8);
        break;
    }
}

static void WorldMapThoughtBubble_Loop(struct MenuProc * menuProc)
{
    unsigned i;

    for (i = 0; i < ARRAY_COUNT(WorldMapThoughtBubble); ++i)
    {
        if (WorldMapThoughtBubble[i].chapterIndex != gPlaySt.chapterIndex)
            continue;

        PutSprite(4, 10, 10, gObject_64x64, TILEREF(0x2C4, 0x0));
        PutSprite(4, 74, 10, gObject_64x64, TILEREF(0x2CC, 0x0));
        break;
    }
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

            Debugf("R pressed: currentUnitId=%d nextUnitId=%d node=%d ix=%d iy=%d", currentUnitId, nextUnitId, gGMData.units[0].location, gGMData.ix, gGMData.iy);

            if (nextUnitId != 0)
            {
                struct GMapPIProc * playerInterfaceProc;
                struct Unit * nextUnit = GetUnitFromCharId(nextUnitId);

                Debugf("nextUnit lookup: unit=%p valid=%d pid=%d level=%d faction=%d", nextUnit, UNIT_IS_VALID(nextUnit), nextUnit ? nextUnit->pCharacterData->number : 0, nextUnit ? nextUnit->level : 0, nextUnit ? UNIT_FACTION(nextUnit) : 0);

                gGMData.units[0].id = nextUnitId;
                sub_80B8FEC(proc);
                sub_80B90CC(proc);
                // PutFaceChibi(nextUnitId, gUnknown_0201B7DA, 0x220, 4, 0);

                playerInterfaceProc = Proc_Find(ProcScr_GMapPlayerInterface);

                Debugf("player interface proc=%p", playerInterfaceProc);

                if (playerInterfaceProc != NULL && UNIT_IS_VALID(nextUnit))
                {
                    Debugf("refreshing PI: oldPid=%d oldJid=%d nodeId=%d", playerInterfaceProc->pid, playerInterfaceProc->jid, playerInterfaceProc->nodeId);
                    Debugf("PI buffers: unk_40=%p unk_44=%04x", playerInterfaceProc->unk_40, playerInterfaceProc->unk_44);
                    playerInterfaceProc->pid = nextUnitId;
                    playerInterfaceProc->jid = 0;
                    playerInterfaceProc->interfaceKind = 1;

                    Debugf("calling face/name/level redraw for pid=%d", playerInterfaceProc->pid);
                    PutGMapPICharName(playerInterfaceProc, playerInterfaceProc->pid);
                    PutGMapPIFace(playerInterfaceProc);
                    Debugf("writing level=%d via sub_80BE5F8 to %p", nextUnit->level, playerInterfaceProc->unk_40);
                    sub_80BE5F8(playerInterfaceProc->unk_40, nextUnit);
                    Debugf("level write complete for pid=%d level=%d", nextUnitId, nextUnit->level);
                    sub_80BE638(playerInterfaceProc, nextUnit);
                    BG_EnableSyncByMask(BG0_SYNC_BIT);
                }
                else
                {
                    Debugf("PI refresh skipped: proc=%p nextUnitValid=%d", playerInterfaceProc, UNIT_IS_VALID(nextUnit));
                }
            }
            else
            {
                Debug("R pressed but no next roster unit was found");
            }
        }
    }

    if (gKeyStatusPtr->newKeys & SELECT_BUTTON)
    {
        if (gGMData.state.bits.state_2)
        {
            gGMData.state.bits.state_2 = 0;
        }
        else
        {
            gGMData.state.bits.state_2 = 1;
        }
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