#include "../../_shared/headers.h"
#include "../../_shared/empty-event-lists.h"
#include "redas.h"
#include "units.h"
#include "events.h"

const struct ChapterEventGroup Chapter04Event = {
    .turnBasedEvents               = EventListScr_Ch04_Turn,
    .characterBasedEvents          = EventListScr_Ch04_Character,
    .locationBasedEvents           = EventListScr_Ch04_Location,
    .miscBasedEvents               = EventListScr_Ch04_Misc,
    .specialEventsWhenUnitSelected = EventListScr_Empty,
    .specialEventsWhenDestSelected = EventListScr_Empty,
    .specialEventsAfterUnitMoved   = EventListScr_Empty,
    .tutorialEvents                = EventListScr_Tutorial_None,

    .traps            = TrapData_None,
    .extraTrapsInHard = TrapData_None,

    .playerUnitsInNormal = CH4_PLAYER_UNITS,
    .playerUnitsInHard   = CH4_PLAYER_UNITS,

    .playerUnitsChoice1InEncounter = UnitDef_088B4DB4,
    .playerUnitsChoice2InEncounter = UnitDef_Ch4SkirmishAlly2,
    .playerUnitsChoice3InEncounter = UnitDef_088B528C,

    .enemyUnitsChoice1InEncounter = UnitDef_088B4E68,
    .enemyUnitsChoice2InEncounter = UnitDef_088B50D4,
    .enemyUnitsChoice3InEncounter = UnitDef_088B5340,

    .beginningSceneEvents = EventScr_Ch04_Opening,
    .endingSceneEvents    = EventScr_Ch04_Ending,
};