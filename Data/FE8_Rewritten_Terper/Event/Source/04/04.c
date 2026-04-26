#include "../Headers.h"
#include "Source/ASMCs.h"
#include "Source/Redas.h"
#include "Source/Units.h"
#include "Source/Events.h"
#include "Source/Traps.h"

const struct ChapterEventGroup Chapter04Event = {
    .turnBasedEvents               = EventListScr_Turn,
    .characterBasedEvents          = EventListScr_Character,
    .locationBasedEvents           = EventListScr_Location,
    .miscBasedEvents               = EventListScr_Misc,
    .specialEventsWhenUnitSelected = EventListScr_SelectUnit,
    .specialEventsWhenDestSelected = EventListScr_SelectDestination,
    .specialEventsAfterUnitMoved   = EventListScr_UnitMove,
    .tutorialEvents                = EventListScr_Tutorial,

    .traps            = TrapData_ThisEvent,
    .extraTrapsInHard = TrapData_ThisEventHard,

    .playerUnitsInNormal = CH4_PLAYER_UNITS,
    .playerUnitsInHard   = CH4_PLAYER_UNITS,

    .playerUnitsChoice1InEncounter = UnitDef_088B4DB4,
    .playerUnitsChoice2InEncounter = UnitDef_Ch4SkirmishAlly2,
    .playerUnitsChoice3InEncounter = UnitDef_088B528C,

    .enemyUnitsChoice1InEncounter = UnitDef_088B4E68,
    .enemyUnitsChoice2InEncounter = UnitDef_088B50D4,
    .enemyUnitsChoice3InEncounter = UnitDef_088B5340,

    .beginningSceneEvents = EventScr_Beginning,
    .endingSceneEvents    = EventScr_Ending_Chapter_04,
};