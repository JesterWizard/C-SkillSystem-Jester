#include "../Headers.h"
#include "Source/ASMCs.h"
#include "Source/Redas.h"
#include "Source/Units.h"
#include "Source/Events.h"
#include "Source/Traps.h"

const struct ChapterEventGroup Chapter06Event = {
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

    .playerUnitsInNormal = CH6_PLAYER_UNITS,
    .playerUnitsInHard   = CH6_PLAYER_UNITS,

    .playerUnitsChoice1InEncounter = UnitDef_088B6540,
    .playerUnitsChoice2InEncounter = UnitDef_088B6838,
    .playerUnitsChoice3InEncounter = UnitDef_088B6B30,

    .enemyUnitsChoice1InEncounter = UnitDef_088B6608,
    .enemyUnitsChoice2InEncounter = UnitDef_088B6900,
    .enemyUnitsChoice3InEncounter = UnitDef_088B6BF8,

    .beginningSceneEvents = EventScr_Beginning,
    .endingSceneEvents    = EventScr_Ending_Chapter_06,
};