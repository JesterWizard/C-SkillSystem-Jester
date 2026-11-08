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
static const EventScr EventScr_PortKiris_Opening[] = {
    STAL(15)
    MUSC(SONG_DISTANT_ROADS)
    LOAD_WAIT_PERSIST(Base_PortKiris_Eirika)
    MOVE_WAIT(0, CHARACTER_EIRIKA, 13, 1)
    LOAD_WAIT(District_PortKiris_Npcs)
    ASMC(SetGameOptions_DistrictPortKiris)
    ASMC(StartDistrictNpcs)
    NOFADE
    ENDA
};

static const EventScr EventScr_PortKiris_Ending[] = {
    ENDA
};

/**
 * Misc events
 */

static const u16 EventListScr_PortKiris_ARMORY[] = {
    ITEM_SWORD_STEEL,
    ITEM_BLADE_IRON,
    ITEM_LANCE_IRON,
    ITEM_LANCE_STEEL,
    ITEM_LANCE_JAVELIN,
    ITEM_AXE_IRON,
    ITEM_AXE_STEEL,
    ITEM_AXE_HAMMER,
    ITEM_AXE_HANDAXE,
    ITEM_BOW_IRON,
    ITEM_BOW_STEEL,
    ITEM_BOW_SHORTBOW,
    ITEM_NONE,
};

static const u16 EventListScr_PortKiris_VENDOR[] = {
    ITEM_ANIMA_FIRE,
    ITEM_ANIMA_THUNDER,
    ITEM_LIGHT_LIGHTNING,
    ITEM_LIGHT_SHINE,
    ITEM_DARK_FLUX,
    ITEM_STAFF_HEAL,
    ITEM_STAFF_MEND,
    ITEM_STAFF_UNLOCK,
    ITEM_CHESTKEY,
    ITEM_VULNERARY,
    ITEM_ELIXIR,
    ITEM_ANTITOXIN,
    ITEM_TORCH,
    ITEM_NONE,
};


/**
 * NPC talk events
 */
static const EventScr EventScr_District_PortKiris_HexA[] = {
    TEXT(District_PortKiris_HexA)
    SVAL(EVT_SLOT_2, CHARACTER_CITIZEN_YOUNG_MAN)
    ASMC(StartDistrictHexASMC)
    NOFADE
    ENDA
};

static const EventScr EventScr_District_PortKiris_HexB[] = {
    TEXT(District_PortKiris_HexB)
    SVAL(EVT_SLOT_2, CHARACTER_CITIZEN_TEEN_GIRL)
    ASMC(StartDistrictHexASMC)
    NOFADE
    ENDA
};

static const EventScr EventScr_District_PortKiris_Gold[] = {
    TEXT(District_PortKiris_Gold)
    GIVE_GOLD(1500)
    NOFADE
    ENDA
};

static const EventScr EventScr_District_PortKiris_GoldAfter[] = {
    TEXT(District_PortKiris_GoldAfter)
    NOFADE
    ENDA
};

static const EventScr EventScr_District_PortKiris_Item[] = {
    TEXT(District_PortKiris_Item)
    GIVE_ITEM_TO(ITEM_BOOSTER_SPD, CHARACTER_EIRIKA)
    NOFADE
    ENDA
};

static const EventScr EventScr_District_PortKiris_ItemAfter[] = {
    TEXT(District_PortKiris_ItemAfter)
    NOFADE
    ENDA
};

static const EventScr EventScr_District_PortKiris_Hint[] = {
    TEXT(District_PortKiris_Hint)
    NOFADE
    ENDA
};

static const EventScr EventScr_District_PortKiris_HexShop[] = {
    ASMC(StartDistrictHexShopASMC)
    NOFADE
    ENDA
};

/**
 * Event list
 */

static const EventListScr EventListScr_PortKiris_Turn[] = {
    END_MAIN
};

static const EventListScr EventListScr_PortKiris_Character[] = {
    CharacterEventBothWays(GLOBAL_FLAG_DISTRICT_PORT_KIRIS_GOLD, EventScr_District_PortKiris_Gold, CHARACTER_EIRIKA, CHARACTER_CITIZEN)
    CharacterEventBothWays(0, EventScr_District_PortKiris_GoldAfter, CHARACTER_EIRIKA, CHARACTER_CITIZEN)
    CharacterEventBothWays(GLOBAL_FLAG_DISTRICT_PORT_KIRIS_ITEM, EventScr_District_PortKiris_Item, CHARACTER_EIRIKA, CHARACTER_CITIZEN_YOUNG_GIRL)
    CharacterEventBothWays(0, EventScr_District_PortKiris_ItemAfter, CHARACTER_EIRIKA, CHARACTER_CITIZEN_YOUNG_GIRL)
    CharacterEventBothWays(0, EventScr_District_PortKiris_HexA, CHARACTER_EIRIKA, CHARACTER_CITIZEN_YOUNG_MAN)
    CharacterEventBothWays(0, EventScr_District_PortKiris_HexB, CHARACTER_EIRIKA, CHARACTER_CITIZEN_TEEN_GIRL)
    CharacterEventBothWays(0, EventScr_District_PortKiris_Hint, CHARACTER_EIRIKA, 0x7F)
    END_MAIN
};

static const EventListScr EventListScr_PortKiris_Location[] = {
    ARMORY(EventListScr_PortKiris_ARMORY, 4, 4)
    VENDOR(EventListScr_PortKiris_VENDOR, 9, 4)
    HOUSE(0, EventScr_District_PortKiris_HexShop, 19, 12)
    END_MAIN
};

static const EventListScr EventListScr_PortKiris_Misc[] = {
    END_MAIN
};

const struct ChapterEventGroup PortKirisEvent = {
    .turnBasedEvents               = EventListScr_PortKiris_Turn,
    .characterBasedEvents          = EventListScr_PortKiris_Character,
    .locationBasedEvents           = EventListScr_PortKiris_Location,
    .miscBasedEvents               = EventListScr_PortKiris_Misc,
    .specialEventsWhenUnitSelected = EventListScr_Empty,
    .specialEventsWhenDestSelected = EventListScr_Empty,
    .specialEventsAfterUnitMoved   = EventListScr_Empty,
    .tutorialEvents                = EventListScr_Tutorial_None,

    .traps            = TrapData_PortKiris,
    .extraTrapsInHard = TrapData_None,

    .playerUnitsInNormal = Base_PortKiris_Eirika,
    .playerUnitsInHard = Base_PortKiris_Eirika,

    .playerUnitsChoice1InEncounter = NULL,
    .playerUnitsChoice2InEncounter = NULL,
    .playerUnitsChoice3InEncounter = NULL,

    .enemyUnitsChoice1InEncounter = NULL,
    .enemyUnitsChoice2InEncounter = NULL,
    .enemyUnitsChoice3InEncounter = NULL,

    .beginningSceneEvents = EventScr_PortKiris_Opening,
    .endingSceneEvents    = EventScr_PortKiris_Ending,
};
