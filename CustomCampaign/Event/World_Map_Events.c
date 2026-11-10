#include "common-chax.h"
#include "kernel-lib.h"
#include "../Chapters/_shared/worldmap-scripts.h"
#include "wm-story-chapter.h"
#include "jester_headers/maps.h"

/*
 * Where each story chapter starts on the world map. The chapter comes from
 * the event that ends the previous one (NEXT_CHAPTER_WITH_MAP), so several
 * chapters can use the same node. Chapters played without the world map
 * (NEXT_CHAPTER_WITHOUT_MAP, e.g. Chapter 1 and 5x) need no row.
 */
const struct WmStoryChapterNode gWmStoryChapterNodes[] = {
    { 0x00,          WM_NODE_BorderMulan },   /* Prologue */
    { CASTLE_FRELIA, WM_NODE_CastleFrelia },  /* Intermission */
    { CHAPTER_02,    WM_NODE_Ide },
    { CHAPTER_03,    WM_NODE_BorgoRidge },
    { CHAPTER_04,    WM_NODE_ZahaWoods },
    { CHAPTER_05,    WM_NODE_Serafew },
    { CHAPTER_06,    WM_NODE_AdlasPlains },
    { CHAPTER_07,    WM_NODE_Renvall1 },
    { CHAPTER_08,    WM_NODE_Renvall2 },
    { CHAPTER_09,    WM_NODE_PortKiris },
    { CHAPTER_10,    WM_NODE_TerazPlateau },
    /* Stub chapters (Chapters/Stubs) from here: the vanilla Eirika route's nodes */
    { CHAPTER_11,    WM_NODE_CaerPelyn },
    { CHAPTER_12,    WM_NODE_CaerPelyn },
    { CHAPTER_13,    WM_NODE_HamillCanyon },
    { CHAPTER_14,    WM_NODE_JehannaHall },
    { CHAPTER_15,    WM_NODE_JehannaHall },   /* not vanilla's second Jehanna node: it sits on the first */
    { CHAPTER_16,    WM_NODE_RenaisCastle },
    { CHAPTER_17,    WM_NODE_NarubeRiver },
    { CHAPTER_18,    WM_NODE_NelerasPeak },
    { CHAPTER_19,    WM_NODE_RaustenCourt },
    { CHAPTER_20,    WM_NODE_BlackTemple },
    { WM_STORY_CHAPTER_NONE, 0 },
};

const u16 * const EventScrWM_SET_NODE[] = {
    NULL,
    (const u16 *)EventScrWM_Prologue_SET_NODE,
    NULL,
    (const u16 *)EventScrWM_Ch2_SET_NODE,
    (const u16 *)EventScrWM_Ch3_SET_NODE,
    (const u16 *)EventScrWM_Ch4_SET_NODE,
    NULL,
    (const u16 *)EventScrWM_Ch5_SET_NODE,
    (const u16 *)EventScrWM_Ch6_SET_NODE,
    (const u16 *)EventScrWM_Ch7_SET_NODE,
    (const u16 *)EventScrWM_Ch8_SET_NODE,
    (const u16 *)EventScrWM_Ch9_SET_NODE,
    (const u16 *)EventScrWM_Ch10_SET_NODE,
    (const u16 *)EventScrWM_Ch12_SET_NODE,   /* 0x0D */
    (const u16 *)EventScrWM_Ch13_SET_NODE,
    (const u16 *)EventScrWM_Ch14_SET_NODE,
    (const u16 *)EventScrWM_Ch15_SET_NODE,
    (const u16 *)EventScrWM_Ch16_SET_NODE,   /* 0x11 */
    (const u16 *)EventScrWM_Ch17_SET_NODE,
    (const u16 *)EventScrWM_Ch18_SET_NODE,
    (const u16 *)EventScrWM_Ch19_SET_NODE,
    (const u16 *)EventScrWM_Ch20_SET_NODE,   /* 0x15 */
    NULL,                                    /* 0x16, 0x17: the last two chapters come without the world map */
    NULL,
    (const u16 *)EventScrWM_Ch11_SET_NODE,   /* 0x18: Chapter 11's gmapEventId (vanilla 0x39, Stubs/Stubs.event) */
};

const u16 * const EventScrWM_TRAVEL_TO_NODE[] = {
    NULL,
    (const u16 *)EventScrWM_Prologue_TRAVEL_TO_NODE,
    NULL,
    (const u16 *)EventScrWM_Ch2_TRAVEL_TO_NODE,
    (const u16 *)EventScrWM_Ch3_TRAVEL_TO_NODE,
    (const u16 *)EventScrWM_Ch4_TRAVEL_TO_NODE,
    NULL,
    (const u16 *)EventScrWM_Ch5_TRAVEL_TO_NODE,
    (const u16 *)EventScrWM_Ch6_TRAVEL_TO_NODE,
    (const u16 *)EventScrWM_Ch7_TRAVEL_TO_NODE,
    (const u16 *)EventScrWM_Ch8_TRAVEL_TO_NODE,
    (const u16 *)EventScrWM_Ch9_TRAVEL_TO_NODE,
    (const u16 *)EventScrWM_Ch10_TRAVEL_TO_NODE,
    (const u16 *)EventScrWM_Ch12_TRAVEL_TO_NODE,   /* 0x0D */
    (const u16 *)EventScrWM_Ch13_TRAVEL_TO_NODE,
    (const u16 *)EventScrWM_Ch14_TRAVEL_TO_NODE,
    (const u16 *)EventScrWM_Ch15_TRAVEL_TO_NODE,
    (const u16 *)EventScrWM_Ch16_TRAVEL_TO_NODE,   /* 0x11 */
    (const u16 *)EventScrWM_Ch17_TRAVEL_TO_NODE,
    (const u16 *)EventScrWM_Ch18_TRAVEL_TO_NODE,
    (const u16 *)EventScrWM_Ch19_TRAVEL_TO_NODE,
    (const u16 *)EventScrWM_Ch20_TRAVEL_TO_NODE,   /* 0x15 */
    NULL,
    NULL,
    (const u16 *)EventScrWM_Ch11_TRAVEL_TO_NODE,   /* 0x18 */
};

//! FE8U = 0x080BA334
LYN_REPLACE_CHECK(WorldMap_CallBeginningEvent);
void WorldMap_CallBeginningEvent(struct WorldMapMainProc* proc)
{
    int chIndex;
    int node_next;
    int eventId;
    const struct ROMChapterData * chapterData;

    Sound_FadeOutBGM(4);

    if ((gGMData.state.bits.monster_merged) || (gPlaySt.chapterStateBits & PLAY_FLAG_POSTGAME))
    {
        sub_80BA008(proc->timer);
    }
    else
    {
        int loc = gGMData.units[0].location;

        /* The chapter the last NEXT_CHAPTER_WITH_MAP named decides where the story goes next */
        WmStoryChapter_Begin(gPlaySt.chapterIndex);

        gGMData.current_node = loc;
        node_next = WMLoc_GetNextLocId(loc);
        NoCashGBAPrintf("[WM] CallBeginning loc=%d node_next=%d mode=%d merged=%d\n",
                        loc,
                        node_next,
                        gPlaySt.chapterModeIndex,
                        gGMData.state.bits.monster_merged);

        if (node_next > -1 && node_next < NODE_MAX)
        {
            chIndex = WMLoc_GetChapterId(node_next);
            NoCashGBAPrintf("[WM] CallBeginning chapter=%d node_next=%d\n", chIndex, node_next);

            chapterData = chIndex < 0 ? NULL : GetROMChapterStruct(chIndex);

            if (chapterData != NULL)
            {
                gPlaySt.chapterIndex = chIndex;

                eventId = chapterData->gmapEventId;
                NoCashGBAPrintf("[WM] CallBeginning gmapEventId=%d chapter=%d\n", eventId, chIndex);

                ResetGmStoryNode();
                proc->gm_icon->merge_next_node = false;

                /**
                 * New event list helper
                 */
                if (eventId == 55)
                    CallEvent((const u16*)EventScrWM_Ch1_ENDING, 0);
                else if (eventId == 1)
                {
                    if (gpKernelDesignerConfig->skip_intro == true)
                        CallEvent((const u16*)EventScrWM_PrologueSkip, 0);
                    else
                        CallEvent((const u16*)EventScrWM_Prologue_SET_NODE, 0);
                }
                else if (eventId >= 0 && eventId < (int)(sizeof(EventScrWM_SET_NODE) / sizeof(EventScrWM_SET_NODE[0])) && EventScrWM_SET_NODE[eventId] != NULL)
                    CallEvent(EventScrWM_SET_NODE[eventId], 0);
                else
                    NoCashGBAPrintf("[WM] CallBeginning no script chapter=%d eventId=%d\n", chIndex, eventId);
            }
        }
    }

    /* These hand the world map back to the player. Skipping them for a chapter
     * with no beginning script leaves the map faded out with nothing to finish
     * the transition. */
    StartWMFaceCtrl(proc);
    StartGmapMuEntry(NULL);
}

//! FE8U = 0x080BA3D4
LYN_REPLACE_CHECK(CallChapterWMIntroEvents);
void CallChapterWMIntroEvents(ProcPtr proc)
{
    int chapterIndex;
    int eventId;
    const u16 * eventScript;
    const struct ROMChapterData * chapterData;
    const int maxSafeEventId = 12;

    chapterIndex = gPlaySt.chapterIndex;

    eventScript = NULL;
    chapterData = chapterIndex < 0 ? NULL : GetROMChapterStruct(chapterIndex);

    if (chapterData != NULL)
    {
        /**
         * New event list helper
         */
        eventId = chapterData->gmapEventId;
        NoCashGBAPrintf("[WM] CallIntro chapter=%d gmapEventId=%d\n", chapterIndex, eventId);

        if (eventId >= 0 && eventId < (int)(sizeof(EventScrWM_TRAVEL_TO_NODE) / sizeof(EventScrWM_TRAVEL_TO_NODE[0])) && EventScrWM_TRAVEL_TO_NODE[eventId] != NULL)
        {
            NoCashGBAPrintf("[WM] CallIntro using travel script idx=%d\n", eventId);
            eventScript = EventScrWM_TRAVEL_TO_NODE[eventId];
        }
        else if (eventId != 2 && eventId != 6 && eventId >= 0 && eventId < maxSafeEventId)
        {
            NoCashGBAPrintf("[WM] CallIntro using intro script idx=%d\n", eventId);
            eventScript = (const u16 *)Events_WM_ChapterIntro[eventId];
        }
        else
        {
            NoCashGBAPrintf("[WM] CallIntro no script chapter=%d eventId=%d\n", chapterIndex, eventId);
        }
    }

    if (eventScript != NULL)
        CallEvent(eventScript, 0);

    /* These hand the world map back to the player. Skipping them for a chapter
     * with no intro script (0x38 reports gmapEventId 55, which is outside every
     * table here) leaves the map faded out with nothing to finish the transition. */
    StartWMFaceCtrl(proc);
    StartGmapMuEntry(NULL);
}

LYN_REPLACE_CHECK(Event97_WmInitNextStoryNode);
//! FE8U = 0x0800C2DC
u8 Event97_WmInitNextStoryNode(struct EventEngineProc* proc)
{
    // struct WorldMapMainProc * worldMapProc;

    int nodeId = WMLoc_GetNextLocId(gGMData.current_node);

    if (nodeId < 0 || nodeId >= NODE_MAX)
    {
        return EVC_ADVANCE_CONTINUE;
    }

    if (EVENT_IS_SKIPPING(proc))
    {
        ResetGmStoryNode();
        gGMData.nodes[nodeId].state |= 1;
        gGMData.nodes[nodeId].state |= 2;

        GM_ICON->nodeId = nodeId;
        GM_ICON->merge_next_node = true;
    }
    else
    {
        if (!(gGMData.nodes[nodeId].state & 1))
        {
            GM_ICON->nodeId = nodeId;
            StartGmBaseEntry(nodeId, 0, NULL);
            ResetGmStoryNode();
            gGMData.nodes[nodeId].state |= 2;
        }
    }

    return EVC_ADVANCE_CONTINUE;
};

//! FE8U = 0x08010968
LYN_REPLACE_CHECK(Event3E_PrepScreenCall);
u8 Event3E_PrepScreenCall(struct EventEngineProc* proc)
{
    HideAllUnits();
    ClearFlag(0x84);
    Proc_StartBlocking(gProcScr_SALLYCURSOR, proc);

    return EVC_ADVANCE_YIELD;
}

/* EnterTown's district chapter (Kernel/Wizardry/WorldMap/WorldMap.c, WMLoc_GetChapterId) */
extern u8 sWmChapterIdOverride;

//! FE8U = 0x080B9B38
LYN_REPLACE_CHECK(WorldMap_CallIntroEvent);
void WorldMap_CallIntroEvent(struct WorldMapMainProc* proc)
{
    int nodeId;
    int chapterId;

    GmMu_80BE108(proc->gm_mu, 0, 0);

    if (gGMData.units[0].location[gWMNodeData].placementFlag != GMAP_NODE_PLACEMENT_DUNGEON)
    {
        nodeId = proc->unk_3e;
        gGMData.state.bits.monster_merged = false;
    }
    else
    {
        nodeId = gGMData.units[0].location;
    }

    /*
     * Enter District (EnterTown.c) comes here without picking a node, so unk_3e is whatever the last trip left in it,
     * often out of range: the early return below then kept the story chapter the beginning event had set, and the
     * district never loaded (#672). With a district override, the lord's node is the node.
     */
    if (sWmChapterIdOverride != 0)
        nodeId = gGMData.units[0].location;

    if (nodeId < 0 || nodeId >= NODE_MAX)
        return;

    chapterId = WMLoc_GetChapterId(nodeId);
    NoCashGBAPrintf("[WM] CallIntro node=%d chapter=%d mode=%d dungeon=%d\n",
                    nodeId,
                    chapterId,
                    gPlaySt.chapterModeIndex,
                    gGMData.units[0].location[gWMNodeData].placementFlag == GMAP_NODE_PLACEMENT_DUNGEON);

    if (chapterId < 0)
        return;

    gPlaySt.chapterIndex = chapterId;

    CallChapterWMIntroEvents(proc);

    gGMData.sprite_disp = 0;

    WmRemoveRandomMonsters();
}

extern struct ProcCmd CONST_DATA gProcScr_OpSubtitle[];

//! FE8U = 0x080C541C
LYN_REPLACE_CHECK(StartIntroMonologue);
void StartIntroMonologue(ProcPtr proc) {

    if (gpKernelDesignerConfig->skip_intro == true)
        return;

    Proc_StartBlocking(gProcScr_OpSubtitle, proc);
    
    return;
}
