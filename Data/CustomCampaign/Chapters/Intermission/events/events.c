#include "../../_shared/headers.h"
#include "../../_shared/empty-event-lists.h"
#include "redas.h"
#include "units.h"
#include "events.h"

const struct ChapterEventGroup IntermissionEvent = {
    .turnBasedEvents               = EventListScr_Intermission_Turn,
    .characterBasedEvents          = EventListScr_Intermission_Character,
    .locationBasedEvents           = EventListScr_Intermission_Location,
    .miscBasedEvents               = EventListScr_Intermission_Misc,
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

    .beginningSceneEvents = EventScr_Intermission_Opening,
    .endingSceneEvents    = EventScr_Intermission_Ending,
};
