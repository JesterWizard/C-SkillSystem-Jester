#include "../../_shared/headers.h"
#include "../../_shared/empty-event-lists.h"
#include "asmc.h"
#include "redas.h"
#include "units.h"
#include "events.h"

const struct ChapterEventGroup SerafewEvent = {
    .turnBasedEvents               = EventListScr_Serafew_Turn,
    .characterBasedEvents          = EventListScr_Serafew_Character,
    .locationBasedEvents           = EventListScr_Serafew_Location,
    .miscBasedEvents               = EventListScr_Serafew_Misc,
    .specialEventsWhenUnitSelected = EventListScr_Empty,
    .specialEventsWhenDestSelected = EventListScr_Empty,
    .specialEventsAfterUnitMoved   = EventListScr_Empty,
    .tutorialEvents                = EventListScr_Tutorial_None,

    .traps            = TrapData_None,
    .extraTrapsInHard = TrapData_None,

    .playerUnitsInNormal = Base_Chapter_5_Eirika,
    .playerUnitsInHard = Base_Chapter_5_Eirika,

    .playerUnitsChoice1InEncounter = NULL,
    .playerUnitsChoice2InEncounter = NULL,
    .playerUnitsChoice3InEncounter = NULL,

    .enemyUnitsChoice1InEncounter = NULL,
    .enemyUnitsChoice2InEncounter = NULL,
    .enemyUnitsChoice3InEncounter = NULL,

    .beginningSceneEvents = EventScr_Serafew_Opening,
    .endingSceneEvents    = EventScr_Serafew_Ending,
};