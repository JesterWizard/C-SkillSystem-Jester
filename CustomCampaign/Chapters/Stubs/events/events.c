#include "../../_shared/headers.h"
#include "../../_shared/empty-event-lists.h"

/*
 * Stub story chapters: the chapters of the Eirika route not written yet (Documentation/Features/StoryStubs.md).
 *
 * Each uses its vanilla chapter row (map, tileset, title card) and its own event group here: the opening says the
 * chapter is a placeholder and moves the story on at once, to the world map on the next story chapter's node (its
 * path drawn by Stubs/events/worldmap.c), or, for the last chapters, straight on and then to the game's ending. No unit
 * is loaded and no battle is fought.
 *
 * To write one of them: give it its own folder like the other chapters, point its event slot at the new group in
 * CustomCampaign/Event/Event_Installer.event (NewEventPointerTable_Event), and delete its STUB_CHAPTER line here.
 */

static const struct UnitDefinition StubUnits_None[] = {
    {},
};

static const EventScr EventScr_Stub_Ending[] = {
    ENDA
};

static const EventListScr EventListScr_Stub_Misc[] = {
    CAUSE_GAME_OVER_IF_LORD_DIES
    END_MAIN
};

#define STUB_CHAPTER(name, end) \
    static const EventScr EventScr_##name##_Opening[] = { \
        STAL(30) \
        TEXT(MSG_STORY_STUB_CHAPTER) \
        end \
        ENDA \
    }; \
    const struct ChapterEventGroup name##Event = { \
        .turnBasedEvents               = EventListScr_Empty, \
        .characterBasedEvents          = EventListScr_Empty, \
        .locationBasedEvents           = EventListScr_Empty, \
        .miscBasedEvents               = EventListScr_Stub_Misc, \
        .specialEventsWhenUnitSelected = EventListScr_Empty, \
        .specialEventsWhenDestSelected = EventListScr_Empty, \
        .specialEventsAfterUnitMoved   = EventListScr_Empty, \
        .tutorialEvents                = EventListScr_Tutorial_None, \
        .traps            = TrapData_None, \
        .extraTrapsInHard = TrapData_None, \
        .playerUnitsInNormal = StubUnits_None, \
        .playerUnitsInHard   = StubUnits_None, \
        .playerUnitsChoice1InEncounter = NULL, \
        .playerUnitsChoice2InEncounter = NULL, \
        .playerUnitsChoice3InEncounter = NULL, \
        .enemyUnitsChoice1InEncounter = NULL, \
        .enemyUnitsChoice2InEncounter = NULL, \
        .enemyUnitsChoice3InEncounter = NULL, \
        .beginningSceneEvents = EventScr_##name##_Opening, \
        .endingSceneEvents    = EventScr_Stub_Ending, \
    };

/* Story chapter      what comes next */
STUB_CHAPTER(StubChapter10, NEXT_CHAPTER_WITH_MAP(CHAPTER_11))
STUB_CHAPTER(StubChapter11, NEXT_CHAPTER_WITH_MAP(CHAPTER_12))
STUB_CHAPTER(StubChapter12, NEXT_CHAPTER_WITH_MAP(CHAPTER_13))
STUB_CHAPTER(StubChapter13, NEXT_CHAPTER_WITH_MAP(CHAPTER_14))
STUB_CHAPTER(StubChapter14, NEXT_CHAPTER_WITH_MAP(CHAPTER_15))
STUB_CHAPTER(StubChapter15, NEXT_CHAPTER_WITH_MAP(CHAPTER_16))
STUB_CHAPTER(StubChapter16, NEXT_CHAPTER_WITH_MAP(CHAPTER_17))
STUB_CHAPTER(StubChapter17, NEXT_CHAPTER_WITH_MAP(CHAPTER_18))
STUB_CHAPTER(StubChapter18, NEXT_CHAPTER_WITH_MAP(CHAPTER_19))
STUB_CHAPTER(StubChapter19, NEXT_CHAPTER_WITH_MAP(CHAPTER_20))
/* The last chapters follow each other without the world map, as in vanilla, then the ending */
STUB_CHAPTER(StubChapter20, NEXT_CHAPTER_WITHOUT_MAP(CHAPTER_21))
STUB_CHAPTER(StubChapter21, NEXT_CHAPTER_WITHOUT_MAP(CHAPTER_22))
STUB_CHAPTER(StubChapter22, MNC4)
