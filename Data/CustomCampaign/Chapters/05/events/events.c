#include "../../_shared/headers.h"
#include "../../_shared/empty-event-lists.h"
#include "redas.h"
#include "units.h"
#include "events.h"

const struct ChapterEventGroup Chapter05Event = {
    .turnBasedEvents               = EventListScr_Ch05_Turn,
    .characterBasedEvents          = EventListScr_Ch05_Character,
    .locationBasedEvents           = EventListScr_Ch05_Location,
    .miscBasedEvents               = EventListScr_Ch05_Misc,
    .specialEventsWhenUnitSelected = EventListScr_Empty,
    .specialEventsWhenDestSelected = EventListScr_Empty,
    .specialEventsAfterUnitMoved   = EventListScr_Empty,
    .tutorialEvents                = EventListScr_Tutorial_None,

    .traps            = TrapData_None,
    .extraTrapsInHard = TrapData_None,

    .playerUnitsInNormal = CH5_PLAYER_UNITS,
    .playerUnitsInHard   = CH5_PLAYER_UNITS,

    .playerUnitsChoice1InEncounter = NULL,
    .playerUnitsChoice2InEncounter = NULL,
    .playerUnitsChoice3InEncounter = NULL,

    .enemyUnitsChoice1InEncounter = NULL,
    .enemyUnitsChoice2InEncounter = NULL,
    .enemyUnitsChoice3InEncounter = NULL,

    .beginningSceneEvents = EventScr_Ch05_Opening,
    .endingSceneEvents    = EventScr_Ch05_Ending,
};