#include "../../_shared/headers.h"
#include "../../_shared/empty-event-lists.h"
#include "redas.h"
#include "units.h"
#include "events.h"

const struct ChapterEventGroup Chapter06Event = {
    .turnBasedEvents               = EventListScr_Ch06_Turn,
    .characterBasedEvents          = EventListScr_Ch06_Character,
    .locationBasedEvents           = EventListScr_Ch06_Location,
    .miscBasedEvents               = EventListScr_Ch06_Misc,
    .specialEventsWhenUnitSelected = EventListScr_Empty,
    .specialEventsWhenDestSelected = EventListScr_Empty,
    .specialEventsAfterUnitMoved   = EventListScr_Empty,
    .tutorialEvents                = EventListScr_Tutorial_None,

    .traps            = TrapData_None,
    .extraTrapsInHard = TrapData_None,

    .playerUnitsInNormal = CH6_PLAYER_UNITS,
    .playerUnitsInHard   = CH6_PLAYER_UNITS,

    .playerUnitsChoice1InEncounter = UnitDef_088B6540,
    .playerUnitsChoice2InEncounter = UnitDef_088B6838,
    .playerUnitsChoice3InEncounter = UnitDef_088B6B30,

    .enemyUnitsChoice1InEncounter = UnitDef_088B6608,
    .enemyUnitsChoice2InEncounter = UnitDef_088B6900,
    .enemyUnitsChoice3InEncounter = UnitDef_088B6BF8,

    .beginningSceneEvents = EventScr_Ch06_Opening,
    .endingSceneEvents    = EventScr_Ch06_Ending,
};