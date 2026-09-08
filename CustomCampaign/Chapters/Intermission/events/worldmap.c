#include "../../_shared/worldmap-include.h"

const EventScr EventScrWM_Intermission_Renvall_To_Serafew[] = {
    EVBIT_MODIFY(0x1)
    WM_SPAWNLORD(WM_MU_0, CHARACTER_EIRIKA, WM_NODE_Renvall1)
    WM_CENTERCAMONLORD(WM_MU_0)
    WmEvtSetNodeStateNot2(WM_NODE_Renvall1) // ENOSUPP in EAstdlib
    WM_FADEOUT(0)
    WM_TEXTDECORATE // WaitFade
    EVBIT_MODIFY(0x0)
    WmEvtSetUnitOnNode(WM_MU_0, WM_NODE_Renvall1) // ENOSUPP in EAstdlib
    WM_MAKELORDVISIBLE(WM_MU_0)
    WM_MOVESPRITETO(WM_MU_0, 0x0004, WM_NODE_Renvall1, WM_NODE_AdlasPlains, -2, 0)
    WM_WAITFORSPRITES(WM_MU_0)
    WM_MOVESPRITETO(WM_MU_0, 0x0004, WM_NODE_AdlasPlains, WM_NODE_Serafew, -2, 0)
    WM_WAITFORSPRITES(WM_MU_0)
    WmEvtSetUnitOnNode(WM_MU_0, WM_NODE_Serafew) // ENOSUPP in EAstdlib
    SKIPWN
    ENDA
};

const EventScr EventScrWM_Intermission_Serafew_To_Frelia[] = {
    EVBIT_MODIFY(0x1)
    WM_SPAWNLORD(WM_MU_0, CHARACTER_EIRIKA, WM_NODE_Serafew)
    WM_CENTERCAMONLORD(WM_MU_0)
    WmEvtSetNodeStateNot2(WM_NODE_Serafew) // ENOSUPP in EAstdlib
    WM_FADEOUT(0)
    WM_TEXTDECORATE // WaitFade
    EVBIT_MODIFY(0x0)
    WmEvtSetUnitOnNode(WM_MU_0, WM_NODE_Serafew) // ENOSUPP in EAstdlib
    WM_MAKELORDVISIBLE(WM_MU_0)
    WM_MOVESPRITETO(WM_MU_0, 0x0004, WM_NODE_Serafew, WM_NODE_ZahaWoods, -2, 0)
    WM_WAITFORSPRITES(WM_MU_0)
    WM_MOVESPRITETO(WM_MU_0, 0x0004, WM_NODE_ZahaWoods, WM_NODE_BorgoRidge, -2, 0)
    WM_MOVECAMTO(-1, -1, WM_NODE_CastleFrelia, 120, 0)
    WM_WAITFORSPRITES(WM_MU_0)
    WM_MOVESPRITETO(WM_MU_0, 0x0004, WM_NODE_BorgoRidge, WM_NODE_Ide, -2, 0)
    WM_WAITFORSPRITES(WM_MU_0)
    WM_MOVESPRITETO(WM_MU_0, 0x0004, WM_NODE_Ide, WM_NODE_CastleFrelia, -2, 0)
    WM_WAITFORSPRITES(WM_MU_0)
    WM_WAITFORCAM
    WmEvtSetUnitOnNode(WM_MU_0, WM_NODE_CastleFrelia) // ENOSUPP in EAstdlib
    SKIPWN
    ENDA
};
