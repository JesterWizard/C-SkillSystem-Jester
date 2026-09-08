#include "../../_shared/worldmap-include.h"

const EventScr EventScrWM_Ch8_SET_NODE[] = {
    EVBIT_MODIFY(0x1)
    WM_SPAWNLORD(WM_MU_0, CHARACTER_EIRIKA, WM_NODE_Renvall2)
    WM_CENTERCAMONLORD(WM_MU_0)
    WM_REMOVELOCATION(WM_NODE_Renvall1)
    WM_REMOVEPATH(WM_PATH_06)
    WM_LOADLOCATION2(WM_NODE_Renvall2) // Don't show the blue light effect
    WM_SETDESTINATION(WM_NODE_Renvall2)
    WM_DRAWPATH2(WM_PATH_07)
    WmEvtSetUnitOnNode(WM_MU_0, WM_NODE_Renvall2) // ENOSUPP in EAstdlib
    SKIPWN
    ENDA
};

const EventScr EventScrWM_Ch8_TRAVEL_TO_NODE[] = {
    EVBIT_MODIFY(0x1)
    ENUT(136) // Set departure flag
    ENDA
};
