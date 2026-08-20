#include "../../_shared/headers.h"
#include "../../_shared/empty-event-lists.h"
#include "redas.h"
#include "units.h"
#include "events.h"

const struct ChapterEventGroup Chapter05xEvent = {
    .turnBasedEvents               = EventListScr_Ch05x_Turn,
    .characterBasedEvents          = EventListScr_Ch05x_Character,
    .locationBasedEvents           = EventListScr_Ch05x_Location,
    .miscBasedEvents               = EventListScr_Ch05x_Misc,
    .specialEventsWhenUnitSelected = EventListScr_Empty,
    .specialEventsWhenDestSelected = EventListScr_Empty,
    .specialEventsAfterUnitMoved   = EventListScr_Empty,
    .tutorialEvents                = EventListScr_Tutorial_None,

    .traps            = TrapData_None,
    .extraTrapsInHard = TrapData_None,

    .playerUnitsInNormal = CH5X_PLAYER_UNITS,
    .playerUnitsInHard   = CH5X_PLAYER_UNITS,

    .playerUnitsChoice1InEncounter = NULL,
    .playerUnitsChoice2InEncounter = NULL,
    .playerUnitsChoice3InEncounter = NULL,

    .enemyUnitsChoice1InEncounter = NULL,
    .enemyUnitsChoice2InEncounter = NULL,
    .enemyUnitsChoice3InEncounter = NULL,

    .beginningSceneEvents = EventScr_Ch05x_Opening,
    .endingSceneEvents    = EventScr_Ch05x_Ending,
};