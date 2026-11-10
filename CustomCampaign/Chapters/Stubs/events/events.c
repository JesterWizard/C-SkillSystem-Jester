#include "../../_shared/headers.h"
#include "../../_shared/empty-event-lists.h"

/*
 * Stub story chapters: the chapters of the Eirika route not written yet (Documentation/Features/StoryStubs.md).
 *
 * Each uses its vanilla chapter row (map, tileset, title card) and its own event group here. The opening says the
 * chapter is a placeholder, puts the first few enemies of vanilla's own army for the chapter on the map (their boss
 * first, at vanilla's levels), and opens the preparations screen, which deploys the party on vanilla's starting tiles
 * (playerUnitsInNormal: vanilla's player list). Defeating every enemy runs the win script, which moves the story on:
 * to the world map on the next story chapter's node (its path drawn by Stubs/events/worldmap.c), or, for the last
 * chapters, straight on and then to the game's ending. Vanilla's unit lists are read in place, from the ROM.
 *
 * To write one of them, run Tools/chapter/new_chapter.py <number>: it gives the chapter its own folder, points its event
 * slot at the new group in CustomCampaign/Event/Event_Installer.event, and deletes its STUB_CHAPTER line here.
 */

/*
 * ASMC: loads the first gEventSlots[2] enemies (red units) of the unit list in gEventSlots[1], on their starting tiles
 * (vanilla's movement after loading, REDAs, is skipped).
 */
void StubChapter_LoadEnemies(void)
{
    const struct UnitDefinition *def = (const struct UnitDefinition *)gEventSlots[1];
    int left = gEventSlots[2];

    for (; def->charIndex && left > 0; def++) {
        if (def->allegiance != FACTION_ID_RED)
            continue;
        LoadUnit(def);
        left--;
    }
    RefreshEntityBmMaps();
    RefreshUnitSprites();
}

/*
 * The chapters' ending scene: empty. What comes next is in each chapter's win script (DEFEAT_ALL), as the stubs before
 * the battles had it in their opening: the game's ending (after MNC4) runs the last chapter's ending scene again, so
 * MNC4 there played Chapter 22 once more.
 */
static const EventScr EventScr_Stub_Ending[] = {
    ENDA
};

/*
 * ASMC: slot C is 1 once the game is complete. The game's ending (after the last chapter's MNC4) plays that chapter's
 * map again, with its opening: there the opening skips the battle and goes straight on, as the stubs before the
 * battles did.
 */
void StubChapter_CheckGameComplete(void)
{
    gEventSlots[0xC] = (gPlaySt.chapterStateBits & PLAY_FLAG_COMPLETE) ? 1 : 0;
}

#define STUB_CHAPTER(name, end, players, enemies, count) \
    static const EventScr EventScr_##name##_Opening[] = { \
        STAL(30) \
        ASMC(StubChapter_CheckGameComplete) \
        BNE(1, EVT_SLOT_C, EVT_SLOT_0) \
        TEXT(MSG_STORY_STUB_CHAPTER) \
        SVAL(EVT_SLOT_1, (enemies)) \
        SVAL(EVT_SLOT_2, (count)) \
        ASMC(StubChapter_LoadEnemies) \
        PREP_ALT \
        ENDA \
        LABEL(1) \
        end \
        ENDA \
    }; \
    static const EventScr EventScr_##name##_Win[] = { \
        end \
        ENDA \
    }; \
    static const EventListScr EventListScr_##name##_Misc[] = { \
        DEFEAT_ALL(EventScr_##name##_Win) \
        CAUSE_GAME_OVER_IF_LORD_DIES \
        END_MAIN \
    }; \
    const struct ChapterEventGroup name##Event = { \
        .turnBasedEvents               = EventListScr_Empty, \
        .characterBasedEvents          = EventListScr_Empty, \
        .locationBasedEvents           = EventListScr_Empty, \
        .miscBasedEvents               = EventListScr_##name##_Misc, \
        .specialEventsWhenUnitSelected = EventListScr_Empty, \
        .specialEventsWhenDestSelected = EventListScr_Empty, \
        .specialEventsAfterUnitMoved   = EventListScr_Empty, \
        .tutorialEvents                = EventListScr_Tutorial_None, \
        .traps            = TrapData_None, \
        .extraTrapsInHard = TrapData_None, \
        .playerUnitsInNormal = (const struct UnitDefinition *)(players), \
        .playerUnitsInHard   = (const struct UnitDefinition *)(players), \
        .playerUnitsChoice1InEncounter = NULL, \
        .playerUnitsChoice2InEncounter = NULL, \
        .playerUnitsChoice3InEncounter = NULL, \
        .enemyUnitsChoice1InEncounter = NULL, \
        .enemyUnitsChoice2InEncounter = NULL, \
        .enemyUnitsChoice3InEncounter = NULL, \
        .beginningSceneEvents = EventScr_##name##_Opening, \
        .endingSceneEvents    = EventScr_Stub_Ending, \
    };

/*
 * Story chapter, what comes next, vanilla's player list (starting tiles) and enemy list for the chapter, and how many of
 * its enemies to load. The lists are vanilla's (fe8.gba's event group for the chapter's slot: playerUnitsInNormal, and
 * the largest enemy list its opening loads).
 */
STUB_CHAPTER(StubChapter10, NEXT_CHAPTER_WITH_MAP(CHAPTER_11), 0x088B8540, 0x088B8644, 5)
STUB_CHAPTER(StubChapter11, NEXT_CHAPTER_WITH_MAP(CHAPTER_12), 0x088B9894, 0x088B9984, 5)
STUB_CHAPTER(StubChapter12, NEXT_CHAPTER_WITH_MAP(CHAPTER_13), 0x088BA170, 0x088BA274, 5)
STUB_CHAPTER(StubChapter13, NEXT_CHAPTER_WITH_MAP(CHAPTER_14), 0x088BA948, 0x088BAA74, 5)
STUB_CHAPTER(StubChapter14, NEXT_CHAPTER_WITH_MAP(CHAPTER_15), 0x088BBBA8, 0x088BBCAC, 6)
STUB_CHAPTER(StubChapter15, NEXT_CHAPTER_WITH_MAP(CHAPTER_16), 0x088BC610, 0x088BC714, 6)
STUB_CHAPTER(StubChapter16, NEXT_CHAPTER_WITH_MAP(CHAPTER_17), 0x088BCE58, 0x088BCF5C, 6)
STUB_CHAPTER(StubChapter17, NEXT_CHAPTER_WITH_MAP(CHAPTER_18), 0x088BDE58, 0x088BE31C, 6)
STUB_CHAPTER(StubChapter18, NEXT_CHAPTER_WITH_MAP(CHAPTER_19), 0x088BEFE8, 0x088BF0EC, 7)
STUB_CHAPTER(StubChapter19, NEXT_CHAPTER_WITH_MAP(CHAPTER_20), 0x088C0290, 0x088C0984, 7)
/* The last chapters follow each other without the world map, as in vanilla, then the ending */
STUB_CHAPTER(StubChapter20, NEXT_CHAPTER_WITHOUT_MAP(CHAPTER_21), 0x088C0D74, 0x088C0EF0, 7)
STUB_CHAPTER(StubChapter21, NEXT_CHAPTER_WITHOUT_MAP(CHAPTER_22), 0x088C181C, 0x088C1920, 7)
STUB_CHAPTER(StubChapter22, MNC4, 0x088C1F44, 0x088C2098, 1)
