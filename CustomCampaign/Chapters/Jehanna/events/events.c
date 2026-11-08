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
static const EventScr EventScr_Jehanna_Opening[] = {
    STAL(15)
    MUSC(SONG_DISTANT_ROADS)
    LOAD_WAIT_PERSIST(Base_Jehanna_Eirika)
    MOVE_WAIT(0, CHARACTER_EIRIKA, 11, 14)
    LOAD_WAIT(District_Jehanna_Npcs)
    ASMC(SetGameOptions_DistrictJehanna)
    ASMC(StartDistrictNpcs)
    NOFADE
    ENDA
};

static const EventScr EventScr_Jehanna_Ending[] = {
    ENDA
};

/**
 * Misc events
 */

static const u16 EventListScr_Jehanna_VENDOR[] = {
    ITEM_ANIMA_THUNDER,
    ITEM_ANIMA_ELFIRE,
    ITEM_LIGHT_LIGHTNING,
    ITEM_LIGHT_SHINE,
    ITEM_LIGHT_DIVINE,
    ITEM_DARK_FLUX,
    ITEM_DARK_LUNA,
    ITEM_STAFF_MEND,
    ITEM_STAFF_RESTORE,
    ITEM_STAFF_BARRIER,
    ITEM_VULNERARY,
    ITEM_ELIXIR,
    ITEM_PUREWATER,
    ITEM_ANTITOXIN,
    ITEM_NONE,
};


/**
 * NPC talk events
 */
static const EventScr EventScr_District_Jehanna_HexA[] = {
    TEXT(District_Jehanna_HexA)
    SVAL(EVT_SLOT_2, CHARACTER_CITIZEN_YOUNG_MAN)
    ASMC(StartDistrictHexASMC)
    NOFADE
    ENDA
};

static const EventScr EventScr_District_Jehanna_HexB[] = {
    TEXT(District_Jehanna_HexB)
    SVAL(EVT_SLOT_2, CHARACTER_CITIZEN_TEEN_GIRL)
    ASMC(StartDistrictHexASMC)
    NOFADE
    ENDA
};

static const EventScr EventScr_District_Jehanna_Gold[] = {
    TEXT(District_Jehanna_Gold)
    GIVE_GOLD(3000)
    NOFADE
    ENDA
};

static const EventScr EventScr_District_Jehanna_GoldAfter[] = {
    TEXT(District_Jehanna_GoldAfter)
    NOFADE
    ENDA
};

static const EventScr EventScr_District_Jehanna_Item[] = {
    TEXT(District_Jehanna_Item)
    GIVE_ITEM_TO(ITEM_BOOSTER_POW, CHARACTER_EIRIKA)
    NOFADE
    ENDA
};

static const EventScr EventScr_District_Jehanna_ItemAfter[] = {
    TEXT(District_Jehanna_ItemAfter)
    NOFADE
    ENDA
};

static const EventScr EventScr_District_Jehanna_Hint[] = {
    TEXT(District_Jehanna_Hint)
    NOFADE
    ENDA
};

static const EventScr EventScr_District_Jehanna_HexShop[] = {
    ASMC(StartDistrictHexShopASMC)
    NOFADE
    ENDA
};

/**
 * Event list
 */

static const EventListScr EventListScr_Jehanna_Turn[] = {
    END_MAIN
};

static const EventListScr EventListScr_Jehanna_Character[] = {
    CharacterEventBothWays(GLOBAL_FLAG_DISTRICT_JEHANNA_GOLD, EventScr_District_Jehanna_Gold, CHARACTER_EIRIKA, CHARACTER_CITIZEN)
    CharacterEventBothWays(0, EventScr_District_Jehanna_GoldAfter, CHARACTER_EIRIKA, CHARACTER_CITIZEN)
    CharacterEventBothWays(GLOBAL_FLAG_DISTRICT_JEHANNA_ITEM, EventScr_District_Jehanna_Item, CHARACTER_EIRIKA, CHARACTER_CITIZEN_YOUNG_GIRL)
    CharacterEventBothWays(0, EventScr_District_Jehanna_ItemAfter, CHARACTER_EIRIKA, CHARACTER_CITIZEN_YOUNG_GIRL)
    CharacterEventBothWays(0, EventScr_District_Jehanna_HexA, CHARACTER_EIRIKA, CHARACTER_CITIZEN_YOUNG_MAN)
    CharacterEventBothWays(0, EventScr_District_Jehanna_HexB, CHARACTER_EIRIKA, CHARACTER_CITIZEN_TEEN_GIRL)
    CharacterEventBothWays(0, EventScr_District_Jehanna_Hint, CHARACTER_EIRIKA, 0x7F)
    END_MAIN
};

static const EventListScr EventListScr_Jehanna_Location[] = {
    VENDOR(EventListScr_Jehanna_VENDOR, 8, 4)
    HOUSE(0, EventScr_District_Jehanna_HexShop, 15, 10)
    END_MAIN
};

static const EventListScr EventListScr_Jehanna_Misc[] = {
    END_MAIN
};

const struct ChapterEventGroup JehannaEvent = {
    .turnBasedEvents               = EventListScr_Jehanna_Turn,
    .characterBasedEvents          = EventListScr_Jehanna_Character,
    .locationBasedEvents           = EventListScr_Jehanna_Location,
    .miscBasedEvents               = EventListScr_Jehanna_Misc,
    .specialEventsWhenUnitSelected = EventListScr_Empty,
    .specialEventsWhenDestSelected = EventListScr_Empty,
    .specialEventsAfterUnitMoved   = EventListScr_Empty,
    .tutorialEvents                = EventListScr_Tutorial_None,

    .traps            = TrapData_Jehanna,
    .extraTrapsInHard = TrapData_None,

    .playerUnitsInNormal = Base_Jehanna_Eirika,
    .playerUnitsInHard = Base_Jehanna_Eirika,

    .playerUnitsChoice1InEncounter = NULL,
    .playerUnitsChoice2InEncounter = NULL,
    .playerUnitsChoice3InEncounter = NULL,

    .enemyUnitsChoice1InEncounter = NULL,
    .enemyUnitsChoice2InEncounter = NULL,
    .enemyUnitsChoice3InEncounter = NULL,

    .beginningSceneEvents = EventScr_Jehanna_Opening,
    .endingSceneEvents    = EventScr_Jehanna_Ending,
};
