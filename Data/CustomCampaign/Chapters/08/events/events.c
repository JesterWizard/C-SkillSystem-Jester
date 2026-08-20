#include "../../_shared/headers.h"
#include "../../_shared/empty-event-lists.h"
#include "asmc.h"
#include "redas.h"
#include "units.h"
#include "events.h"

const struct ChapterEventGroup Chapter08Event = {
    .turnBasedEvents               = EventListScr_Ch08_Turn,
    .characterBasedEvents          = EventListScr_Ch08_Character,
    .locationBasedEvents           = EventListScr_Ch08_Location,
    .miscBasedEvents               = EventListScr_Ch08_Misc,
    .specialEventsWhenUnitSelected = EventListScr_Empty,
    .specialEventsWhenDestSelected = EventListScr_Empty,
    .specialEventsAfterUnitMoved   = EventListScr_Empty,
    .tutorialEvents                = EventListScr_Tutorial_None,

    .traps            = TrapData_None,
    .extraTrapsInHard = TrapData_None,

    .playerUnitsInNormal = CH8_PLAYER_UNITS,
    .playerUnitsInHard   = CH8_PLAYER_UNITS,

    .playerUnitsChoice1InEncounter = NULL,
    .playerUnitsChoice2InEncounter = NULL,
    .playerUnitsChoice3InEncounter = NULL,

    .enemyUnitsChoice1InEncounter = NULL,
    .enemyUnitsChoice2InEncounter = NULL,
    .enemyUnitsChoice3InEncounter = NULL,

    .beginningSceneEvents = EventScr_Ch08_Opening,
    .endingSceneEvents    = EventScr_Ch08_Ending,
};