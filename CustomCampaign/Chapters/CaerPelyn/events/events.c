#include "../../_shared/headers.h"
#include "../../_shared/empty-event-lists.h"
#include "asmc.h"
#include "district.h"
#include "hex.h"
#include "units.c"
#include "traps.c"

/**
 * Main events
 */
static const EventScr EventScr_CaerPelyn_Opening[] = {
    /*
     * The screen is still black from the world map (WorldMap_Destruct): set the district up unseen, then one fade in.
     * Before, the map showed half loaded, faded out and in, and the camera then scrolled to Eirika.
     */
    MUSC(SONG_DISTANT_ROADS)
    LOAD_WAIT_PERSIST(Base_CaerPelyn_Eirika)
    LOAD_WAIT(District_CaerPelyn_Npcs)
    MOVE_CAMERA_TO_UNIT(CHARACTER_EIRIKA)
    ASMC(SetGameOptions_DistrictCaerPelyn)
    ASMC(StartDistrictNpcs)
    FADE_FROM_BLACK(16)
    NOFADE
    ENDA
};

/*
 * Runs right after the opening: a district has no enemies, so the "defeat all" check calls the ending at once
 * (RunWaitEvents). A bare ENDA fades the screen out and back in, hence NOFADE.
 */
static const EventScr EventScr_CaerPelyn_Ending[] = {
    NOFADE
    ENDA
};

/**
 * Misc events
 */

/**
 * NPC talk events
 */
static const EventScr EventScr_District_CaerPelyn_HexA[] = {
    TEXT(District_CaerPelyn_HexA)
    SVAL(EVT_SLOT_2, CHARACTER_CITIZEN_YOUNG_MAN)
    ASMC(StartDistrictHexASMC)
    NOFADE
    ENDA
};

static const EventScr EventScr_District_CaerPelyn_HexB[] = {
    TEXT(District_CaerPelyn_HexB)
    SVAL(EVT_SLOT_2, CHARACTER_CITIZEN_TEEN_GIRL)
    ASMC(StartDistrictHexASMC)
    NOFADE
    ENDA
};

static const EventScr EventScr_District_CaerPelyn_Gold[] = {
    TEXT(District_CaerPelyn_Gold)
    GIVE_GOLD(2000)
    NOFADE
    ENDA
};

static const EventScr EventScr_District_CaerPelyn_GoldAfter[] = {
    TEXT(District_CaerPelyn_GoldAfter)
    NOFADE
    ENDA
};

static const EventScr EventScr_District_CaerPelyn_Item[] = {
    TEXT(District_CaerPelyn_Item)
    GIVE_ITEM_TO(ITEM_BOOSTER_RES, CHARACTER_EIRIKA)
    NOFADE
    ENDA
};

static const EventScr EventScr_District_CaerPelyn_ItemAfter[] = {
    TEXT(District_CaerPelyn_ItemAfter)
    NOFADE
    ENDA
};

static const EventScr EventScr_District_CaerPelyn_Hint[] = {
    TEXT(District_CaerPelyn_Hint)
    NOFADE
    ENDA
};

static const EventScr EventScr_District_CaerPelyn_HexShop[] = {
    ASMC(StartDistrictHexShopASMC)
    NOFADE
    ENDA
};

/**
 * Event list
 */

static const EventListScr EventListScr_CaerPelyn_Turn[] = {
    END_MAIN
};

static const EventListScr EventListScr_CaerPelyn_Character[] = {
    CharacterEventBothWays(GLOBAL_FLAG_DISTRICT_CAER_PELYN_GOLD, EventScr_District_CaerPelyn_Gold, CHARACTER_EIRIKA, CHARACTER_CITIZEN)
    CharacterEventBothWays(0, EventScr_District_CaerPelyn_GoldAfter, CHARACTER_EIRIKA, CHARACTER_CITIZEN)
    CharacterEventBothWays(GLOBAL_FLAG_DISTRICT_CAER_PELYN_ITEM, EventScr_District_CaerPelyn_Item, CHARACTER_EIRIKA, CHARACTER_CITIZEN_YOUNG_GIRL)
    CharacterEventBothWays(0, EventScr_District_CaerPelyn_ItemAfter, CHARACTER_EIRIKA, CHARACTER_CITIZEN_YOUNG_GIRL)
    CharacterEventBothWays(0, EventScr_District_CaerPelyn_HexA, CHARACTER_EIRIKA, CHARACTER_CITIZEN_YOUNG_MAN)
    CharacterEventBothWays(0, EventScr_District_CaerPelyn_HexB, CHARACTER_EIRIKA, CHARACTER_CITIZEN_TEEN_GIRL)
    CharacterEventBothWays(0, EventScr_District_CaerPelyn_Hint, CHARACTER_EIRIKA, 0x7F)
    END_MAIN
};

static const EventListScr EventListScr_CaerPelyn_Location[] = {
    HOUSE(0, EventScr_District_CaerPelyn_HexShop, 11, 8)
    END_MAIN
};

static const EventListScr EventListScr_CaerPelyn_Misc[] = {
    END_MAIN
};

const struct ChapterEventGroup CaerPelynEvent = {
    .turnBasedEvents               = EventListScr_CaerPelyn_Turn,
    .characterBasedEvents          = EventListScr_CaerPelyn_Character,
    .locationBasedEvents           = EventListScr_CaerPelyn_Location,
    .miscBasedEvents               = EventListScr_CaerPelyn_Misc,
    .specialEventsWhenUnitSelected = EventListScr_Empty,
    .specialEventsWhenDestSelected = EventListScr_Empty,
    .specialEventsAfterUnitMoved   = EventListScr_Empty,
    .tutorialEvents                = EventListScr_Tutorial_None,

    .traps            = TrapData_CaerPelyn,
    .extraTrapsInHard = TrapData_None,

    .playerUnitsInNormal = Base_CaerPelyn_Eirika,
    .playerUnitsInHard = Base_CaerPelyn_Eirika,

    .playerUnitsChoice1InEncounter = NULL,
    .playerUnitsChoice2InEncounter = NULL,
    .playerUnitsChoice3InEncounter = NULL,

    .enemyUnitsChoice1InEncounter = NULL,
    .enemyUnitsChoice2InEncounter = NULL,
    .enemyUnitsChoice3InEncounter = NULL,

    .beginningSceneEvents = EventScr_CaerPelyn_Opening,
    .endingSceneEvents    = EventScr_CaerPelyn_Ending,
};
