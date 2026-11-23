#include "../../_shared/headers.h"
#include "../../_shared/empty-event-lists.h"

/*
 * Extras > Tutorials' map (Documentation/Features/Tutorials.md, "The maps"). The opening puts the chosen tutorial's
 * units on the map and runs its set-up (TutorialMap_Setup, Kernel/Wizardry/Tutorials/Source/TutorialMap.c), then
 * explains the mechanic; the player then tries it. Defeating every enemy ends the map; so do losing Eirika and the
 * map menu's Retreat. Each goes back to Extras > Tutorials (TutorialMap_PostChapterSwitch).
 */

/* A message in the middle of the screen, as the in-map tutorials show theirs (TutorialsEvents.c); -1: slot 2's */
#define TUTMAP_TEXT(msg) \
    EvtTextStartType5 \
    SVAL(EVT_SLOT_B, 0xFFFFFFFF) \
    TEXTSHOW(msg) \
    TEXTEND \
    REMA

/*
 * Leave the map, back to Tutorials (TutorialMap_PostChapterSwitch, at the game control's label 9), as the map menu's
 * Retreat does (vanilla's EventScr_SkirmishRetreat): MNCH -1, then StartRetreatProcessing, which ends the map. A win or
 * a game over alone would not leave it: in tutorial mode vanilla starts the map again.
 */
extern void StartRetreatProcessing(void);

#define TUTMAP_LEAVE \
    FADE_TO_BLACK(16) \
    NEXT_CHAPTER_WITH_MAP(-1) \
    ASMC(StartRetreatProcessing) \
    ENDA

extern void TutorialMap_Setup(void);
extern void TutorialMap_AfterIntro(void);
extern void TutorialMap_CheckIntro(void);

static const EventScr EventScr_TutorialMap_Opening[] = {
    /* The screen is black until the opening ends (as a district's, PortKiris/events/events.c): fade in first */
    ASMC(TutorialMap_Setup)
    FADE_FROM_BLACK(16)
    STAL(20)
    TUTMAP_TEXT(-1)
    ASMC(TutorialMap_AfterIntro)
    STAL(30)
    NOFADE
    ENDA
};

static const EventScr EventScr_TutorialMap_Win[] = {
    TUTMAP_TEXT(MSG_TUTMAP_CLEAR)
    TUTMAP_LEAVE
};

/* Eirika lost: no game over (in tutorial mode vanilla would retry the map), back to Tutorials */
static const EventScr EventScr_TutorialMap_Lose[] = {
    TUTMAP_TEXT(MSG_TUTMAP_LOST)
    TUTMAP_LEAVE
};

static const EventScr EventScr_TutorialMap_Ending[] = {
    ENDA
};

static const EventListScr EventListScr_TutorialMap_Misc[] = {
    DEFEAT_ALL(EventScr_TutorialMap_Win)
    AFEV(0, EventScr_TutorialMap_Lose, EVFLAG_GAMEOVER)
    END_MAIN
};

/* A district's opening calls it: in tutorial mode, the tutorial's explanation */
const EventScr EventScr_TutorialMap_DistrictIntro[] = {
    ASMC(TutorialMap_CheckIntro)
    BEQ(1, EVT_SLOT_C, EVT_SLOT_0)
    TUTMAP_TEXT(-1)
    LABEL(1)
    ENDA
};

const struct ChapterEventGroup TutorialMapEvent = {
    .turnBasedEvents               = EventListScr_Empty,
    .characterBasedEvents          = EventListScr_Empty,
    .locationBasedEvents           = EventListScr_Empty,
    .miscBasedEvents               = EventListScr_TutorialMap_Misc,
    .specialEventsWhenUnitSelected = EventListScr_Empty,
    .specialEventsWhenDestSelected = EventListScr_Empty,
    .specialEventsAfterUnitMoved   = EventListScr_Empty,
    .tutorialEvents                = EventListScr_Tutorial_None,
    .traps            = TrapData_None,
    .extraTrapsInHard = TrapData_None,
    .playerUnitsInNormal = NULL,
    .playerUnitsInHard   = NULL,
    .playerUnitsChoice1InEncounter = NULL,
    .playerUnitsChoice2InEncounter = NULL,
    .playerUnitsChoice3InEncounter = NULL,
    .enemyUnitsChoice1InEncounter = NULL,
    .enemyUnitsChoice2InEncounter = NULL,
    .enemyUnitsChoice3InEncounter = NULL,
    .beginningSceneEvents = EventScr_TutorialMap_Opening,
    .endingSceneEvents    = EventScr_TutorialMap_Ending,
};
