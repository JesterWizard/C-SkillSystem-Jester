#include "../../_shared/headers.h"
#include "../../_shared/empty-event-lists.h"
#include "redas.h"
#include "units.h"
#include "events.h"

const struct ChapterEventGroup Chapter02Event = {
    .turnBasedEvents               = EventListScr_Ch02_Turn,
    .characterBasedEvents          = EventListScr_Ch02_Character,
    .locationBasedEvents           = EventListScr_Ch02_Location,
    .miscBasedEvents               = EventListScr_Ch02_Misc,
    .specialEventsWhenUnitSelected = EventListScr_Empty,
    .specialEventsWhenDestSelected = EventListScr_Empty,
    .specialEventsAfterUnitMoved   = EventListScr_Empty,
    .tutorialEvents                = EventListScr_Tutorial_None,

    .traps            = TrapData_None,
    .extraTrapsInHard = TrapData_None,

    .playerUnitsChoice1InEncounter = NULL,
    .playerUnitsChoice2InEncounter = NULL,
    .playerUnitsChoice3InEncounter = NULL,

    .enemyUnitsChoice1InEncounter = NULL,
    .enemyUnitsChoice2InEncounter = NULL,
    .enemyUnitsChoice3InEncounter = NULL,

    .beginningSceneEvents = EventScr_Ch02_Opening,
    .endingSceneEvents    = EventScr_Ch02_Ending,
};