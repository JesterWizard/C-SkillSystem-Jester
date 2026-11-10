#include "../../_shared/worldmap-include.h"

/*
 * The world map before each stub story chapter (CustomCampaign/Chapters/Stubs/events/events.c): the lord on the last
 * chapter's node, then the next chapter's node shown, set as the destination and joined to the lord by a road
 * (WM_REVEAL_DEST, as Chapters 2-5 do), so the lord can travel there. The road is a vanilla path, so the two nodes
 * must be neighbours on the vanilla map (gWMPathData), and the lord starts on such a neighbour; and travelling there, which puts the lord on the
 * node. Chapter 10's are in Chapters/10. EventScrWM_SET_NODE and EventScrWM_TRAVEL_TO_NODE (World_Map_Events.c) list
 * them by the chapters' gmapEventId.
 */
#define STUB_SET_NODE(name, from, to) \
    const EventScr EventScrWM_##name##_SET_NODE[] = { \
        WM_OPEN_MAP(CHARACTER_EIRIKA, from) \
        WM_SETUNITONNODE(WM_MU_0, from) \
        WM_REVEAL_DEST(to) \
        WM_CLOSE_SET_NODE() \
    }; \
    /* Travelling there: no scene, the lord is put on the node (the written chapters walk the lord in theirs) */ \
    const EventScr EventScrWM_##name##_TRAVEL_TO_NODE[] = { \
        WM_SETUNITONNODE(WM_MU_0, to) \
        STAL(20) \
        ENDA \
    };

/*           chapter  from (the last chapter's node)  to (this chapter's node) */
STUB_SET_NODE(Ch11, WM_NODE_TerazPlateau, WM_NODE_CaerPelyn)
STUB_SET_NODE(Ch12, WM_NODE_CaerPelyn,    WM_NODE_CaerPelyn)
STUB_SET_NODE(Ch13, WM_NODE_CaerPelyn,    WM_NODE_HamillCanyon)
STUB_SET_NODE(Ch14, WM_NODE_HamillCanyon, WM_NODE_JehannaHall)
STUB_SET_NODE(Ch15, WM_NODE_JehannaHall,  WM_NODE_JehannaHall)
/* Renais Castle and Narube River join the map at Jehanna Hall's second node (19, drawn where Jehanna Hall is) */
STUB_SET_NODE(Ch16, WM_NODE_JehannaHall2, WM_NODE_RenaisCastle)
STUB_SET_NODE(Ch17, WM_NODE_JehannaHall2, WM_NODE_NarubeRiver)
STUB_SET_NODE(Ch18, WM_NODE_NarubeRiver,  WM_NODE_NelerasPeak)
STUB_SET_NODE(Ch19, WM_NODE_NelerasPeak,  WM_NODE_RaustenCourt)
STUB_SET_NODE(Ch20, WM_NODE_RaustenCourt, WM_NODE_BlackTemple)
