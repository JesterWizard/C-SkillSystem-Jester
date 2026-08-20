#include "../../_shared/headers.h"
#include "../../_shared/empty-event-lists.h"
#include "redas.h"
#include "units.h"
#include "events.h"
#include "traps.h"

const struct ChapterEventGroup Chapter01Event = {
    .turnBasedEvents               = EventListScr_Ch01_Turn,
    .characterBasedEvents          = EventListScr_Ch01_Character,
    .locationBasedEvents           = EventListScr_Ch01_Location,
    .miscBasedEvents               = EventListScr_Ch01_Misc,
    .specialEventsWhenUnitSelected = EventListScr_Empty,
    .specialEventsWhenDestSelected = EventListScr_Empty,
    .specialEventsAfterUnitMoved   = EventListScr_Empty,
    .tutorialEvents                = EventListScr_Tutorial_None,

    .traps            = TrapData_Ch01,
    .extraTrapsInHard = TrapData_None,

    .playerUnitsChoice1InEncounter = NULL,
    .playerUnitsChoice2InEncounter = NULL,
    .playerUnitsChoice3InEncounter = NULL,

    .enemyUnitsChoice1InEncounter = NULL,
    .enemyUnitsChoice2InEncounter = NULL,
    .enemyUnitsChoice3InEncounter = NULL,

    .beginningSceneEvents = EventScr_Ch01_Opening,
    .endingSceneEvents    = EventScr_Ch01_Ending,
};