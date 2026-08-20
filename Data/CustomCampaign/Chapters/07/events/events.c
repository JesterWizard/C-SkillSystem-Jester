#include "../../_shared/headers.h"
#include "../../_shared/empty-event-lists.h"
#include "redas.h"
#include "units.h"
#include "events.h"
#include "traps.h"

const struct ChapterEventGroup Chapter07Event = {
    .turnBasedEvents               = EventListScr_Ch07_Turn,
    .characterBasedEvents          = EventListScr_Ch07_Character,
    .locationBasedEvents           = EventListScr_Ch07_Location,
    .miscBasedEvents               = EventListScr_Ch07_Misc,
    .specialEventsWhenUnitSelected = EventListScr_Empty,
    .specialEventsWhenDestSelected = EventListScr_Empty,
    .specialEventsAfterUnitMoved   = EventListScr_Empty,
    .tutorialEvents                = EventListScr_Tutorial_None,

    .traps            = TrapData_Ch07,
    .extraTrapsInHard = TrapData_None,

    .playerUnitsInNormal = CH7_PLAYER_UNITS,
    .playerUnitsInHard   = CH7_PLAYER_UNITS,

    .playerUnitsChoice1InEncounter = NULL,
    .playerUnitsChoice2InEncounter = NULL,
    .playerUnitsChoice3InEncounter = NULL,

    .enemyUnitsChoice1InEncounter = NULL,
    .enemyUnitsChoice2InEncounter = NULL,
    .enemyUnitsChoice3InEncounter = NULL,

    .beginningSceneEvents = EventScr_Ch07_Opening,
    .endingSceneEvents    = EventScr_Ch07_Ending,
};