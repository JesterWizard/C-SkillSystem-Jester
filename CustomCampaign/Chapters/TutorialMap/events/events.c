#include "../../_shared/headers.h"
#include "../../_shared/empty-event-lists.h"
#include "tutorials.h"
#include "event-rework.h"

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
extern void TutorialMap_CheckWin(void);
extern void TutorialMap_Leave(void);
extern void TutorialMap_AfterIntro(void);
extern void TutorialMap_CheckIntro(void);
extern void TutorialMap_QuitCancelled(void);
extern const EventScr *const gTutorialGuideScripts[];  /* the tutorials' guided steps (Tutorials/Source/TutorialGuides.c) */

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
    /*
     * A tutorial with no enemies (the stat screen's, Prestige...) has nothing to win, and one with guided steps ends
     * with its lesson (Tutorials/Source/TutorialGuides.c). Done: straight back, with no box.
     */
    /* No fade at the end: a guided tutorial's map has no enemy left after any action, and this event fading the map
     * out and back in came before the lesson's own way out, a second fade */
    NOFADE
    ASMC(TutorialMap_CheckWin)
    BNE(1, EVT_SLOT_C, EVT_SLOT_0)
    ENDA
    LABEL(1)
    TUTMAP_LEAVE
};

/* Eirika, or the last player unit, lost (TutorialMap_OnUnitKill): no game over, back to Tutorials */
static const EventScr EventScr_TutorialMap_Lose[] = {
    TUTMAP_TEXT(MSG_TUTMAP_LOST)
    TUTMAP_LEAVE
};

static const EventScr EventScr_TutorialMap_Ending[] = {
    ENDA
};

/*
 * Free Movement's map (TutorialsData.c: Garcia and the flags): a village at (8, 2), an armory at (1, 11) (Chapter 4's
 * map's two village tiles) and a talk with Garcia, all reached with A in free movement (FreeMU_Event.c). The village is
 * kept by TUTMAP_FLAG_VILLAGE, set for the other tutorials; the armory and the talk need Free Movement's unit or tile.
 */
static const EventListScr EventScr_TutorialMap_Village[] = {
    HOUSE_EVENT_NO_END(0x2, MSG_TUTFM_VILLAGE)
    CALL(EventScr_RemoveBGIfNeeded)
    GIVE_ITEM_TO(ITEM_VULNERARY, CHARACTER_EVT_ACTIVE)
    NOFADE
    ENDA
};

static const EventListScr EventScr_TutorialMap_Garcia[] = {
    TEXT(MSG_TUTFM_GARCIA)
    NOFADE
    ENDA
};

static const u16 EventListScr_TutorialMap_Armory[] = {
    ITEM_SWORD_IRON,
    ITEM_SWORD_SLIM,
    ITEM_LANCE_IRON,
    ITEM_AXE_IRON,
    ITEM_VULNERARY,
    ITEM_NONE,
};

static const EventListScr EventListScr_TutorialMap_Location[] = {
    VILLAGE(TUTMAP_FLAG_VILLAGE, EventScr_TutorialMap_Village, 8, 2)
    ARMORY(EventListScr_TutorialMap_Armory, 1, 11)
    END_MAIN
};

/*
 * Skill Capacity (TutorialsData.c): Saleh teaches Eirika Luna, more than her capacity has left, so it is learned but not
 * equipped (AddSkillWithinCapacity, SkillEventCmd.c), as after a level-up or a promotion.
 */
static const EventListScr EventScr_TutorialMap_Saleh[] = {
    TEXT(MSG_TUTCAP_SALEH)
    Evt_AddSkill(SID_Luna, CHARACTER_EIRIKA)
    TUTMAP_TEXT(MSG_TUTCAP_LEARNED)
    NOFADE
    ENDA
};

static const EventListScr EventListScr_TutorialMap_Character[] = {
    CharacterEventBothWays(TUTMAP_FLAG_GARCIA, EventScr_TutorialMap_Garcia, CHARACTER_EIRIKA, CHARACTER_GARCIA)
    CharacterEvent(TUTMAP_FLAG_SALEH, EventScr_TutorialMap_Saleh, CHARACTER_EIRIKA, CHARACTER_SALEH)
    END_MAIN
};

static const EventListScr EventListScr_TutorialMap_Misc[] = {
    /*
     * DEFEAT_ALL with its own entry flag (TUTMAP_FLAG_NO_WIN, not EVFLAG_WIN): a guided tutorial sets it at the opening
     * (TutorialMap_Setup), so the event never starts. It ends with its lesson; this event's start, even ending at
     * once, dimmed the map and lit it again after any action on a map with no enemy (Staff EXP).
     */
    AFEV(TUTMAP_FLAG_NO_WIN, EventScr_TutorialMap_Win, EVFLAG_DEFEAT_ALL)
    AFEV(0, EventScr_TutorialMap_Lose, EVFLAG_GAMEOVER)
    END_MAIN
};

/* A district's opening calls it: in tutorial mode, the tutorial's explanation */
const EventScr EventScr_TutorialMap_DistrictIntro[] = {
    NOFADE  /* the district is already up: no fade out and in after the explanation */
    ASMC(TutorialMap_CheckIntro)
    BEQ(1, EVT_SLOT_C, EVT_SLOT_0)
    TUTMAP_TEXT(-1)
    LABEL(1)
    ENDA
};

/* SELECT on a tutorial's map or district (TutorialMap_QuitWatch): leave it, back to Tutorials */
const EventScr EventScr_TutorialMap_Quit[] = {
    TUTMAP_TEXT(MSG_TUTMAP_QUIT)
    SVAL(EVT_SLOT_1, 1)
    BEQ(1, EVT_SLOT_C, EVT_SLOT_1)
    /* No: back to the guided step, if any, with the keys it ignored */
    ASMC(TutorialMap_QuitCancelled)
    ENDA
    LABEL(1)
    FADE_TO_BLACK(16)
    ASMC(TutorialMap_Leave)
    /* Wait for it: the map ends from TutorialMap_Leave's proc */
    LABEL(2)
    STAL(1)
    GOTO(2)
    ENDA
};

const struct ChapterEventGroup TutorialMapEvent = {
    .turnBasedEvents               = EventListScr_Empty,
    .characterBasedEvents          = EventListScr_TutorialMap_Character,
    .locationBasedEvents           = EventListScr_TutorialMap_Location,
    .miscBasedEvents               = EventListScr_TutorialMap_Misc,
    .specialEventsWhenUnitSelected = EventListScr_Empty,
    .specialEventsWhenDestSelected = EventListScr_Empty,
    .specialEventsAfterUnitMoved   = EventListScr_Empty,
    .tutorialEvents                = gTutorialGuideScripts,
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
