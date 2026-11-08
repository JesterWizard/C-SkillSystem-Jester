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
static const EventScr EventScr_Serafew_Opening[] = {
    // CHECK_EVENTID(136) // Flag 0x88
    // BEQ(0x1, EVT_SLOT_C, EVT_SLOT_0)
    // CALL(EventScr_NEW_JOURNEY)
    // GOTO(0x2)
    STAL(15)
    MUSC(SONG_DISTANT_ROADS)
    LOAD_WAIT_PERSIST(Base_Chapter_5_Eirika)
    MOVE_WAIT(0, CHARACTER_EIRIKA, 11, 16)
    LOAD_WAIT(District_Serafew_Npcs)
    ASMC(SetGameOptions_BaseChapter5)
    ASMC(StartDistrictNpcs)
    NOFADE
    ENDA
};

static const EventScr EventScr_Serafew_Ending[] = {
    ENDA
};

/**
 * Misc events
 */

static const EventListScr EventListScr_HOUSE_1_TALK[] = {
    HOUSE_EVENT_NO_END(0x0, Chapter_05_House_01)
    CALL(EventScr_RemoveBGIfNeeded) // This is vital, the game crashes without it for this event
    GIVE_ITEM_TO(ITEM_SWORD_ARMORSLAYER, CHARACTER_EVT_ACTIVE)
    NOFADE
    ENDA
};
static const EventListScr EventListScr_HOUSE_2_TALK[] = {
    HOUSE_EVENT_NO_END(0x0, Chapter_05_House_02)
    CALL(EventScr_RemoveBGIfNeeded) // This is vital, the game crashes without it for this event
    GIVE_ITEM_TO(ITEM_BOOSTER_DEF, CHARACTER_EVT_ACTIVE)
    NOFADE
    ENDA
};
static const EventListScr EventListScr_HOUSE_3_TALK[] = {
    HOUSE_EVENT_NO_END(0x0, Chapter_05_House_03)
    CALL(EventScr_RemoveBGIfNeeded) // This is vital, the game crashes without it for this event
    GIVE_ITEM_TO(ITEM_BOOSTER_SKL, CHARACTER_EVT_ACTIVE)
    NOFADE
    ENDA
};
static const EventListScr EventListScr_HOUSE_4_TALK[] = {
    HOUSE_EVENT_NO_END(0x0, Chapter_05_House_04)
    CALL(EventScr_RemoveBGIfNeeded) // This is vital, the game crashes without it for this event
    GIVE_ITEM_TO(ITEM_TORCH, CHARACTER_EVT_ACTIVE)
    NOFADE
    ENDA
};

static const u16 EventListScr_CH5_ARMORY[] = {
    ITEM_SWORD_SLIM,
    ITEM_SWORD_IRON,
    ITEM_SWORD_STEEL,
    ITEM_LANCE_SLIM,
    ITEM_LANCE_IRON,
    ITEM_LANCE_STEEL,
    ITEM_AXE_IRON,
    ITEM_AXE_STEEL,
    ITEM_BOW_IRON,
    ITEM_BOW_STEEL,
    ITEM_NONE,
};

static const u16 EventListScr_CH5_VENDOR[] = {
    ITEM_ANIMA_FIRE,
    ITEM_LIGHT_LIGHTNING,
    ITEM_STAFF_HEAL,
    ITEM_VULNERARY,
    ITEM_NONE,
};


/**
 * NPC talk events
 */
static const EventScr EventScr_District_Serafew_HexA[] = {
    TEXT(District_Serafew_HexA)
    SVAL(EVT_SLOT_2, CHARACTER_CITIZEN_YOUNG_MAN)
    ASMC(StartDistrictHexASMC)
    NOFADE
    ENDA
};

static const EventScr EventScr_District_Serafew_HexB[] = {
    TEXT(District_Serafew_HexB)
    SVAL(EVT_SLOT_2, CHARACTER_CITIZEN_TEEN_GIRL)
    ASMC(StartDistrictHexASMC)
    NOFADE
    ENDA
};

static const EventScr EventScr_District_Serafew_Gold[] = {
    TEXT(District_Serafew_Gold)
    GIVE_GOLD(1000)
    NOFADE
    ENDA
};

static const EventScr EventScr_District_Serafew_GoldAfter[] = {
    TEXT(District_Serafew_GoldAfter)
    NOFADE
    ENDA
};

static const EventScr EventScr_District_Serafew_Item[] = {
    TEXT(District_Serafew_Item)
    GIVE_ITEM_TO(ITEM_ELIXIR, CHARACTER_EIRIKA)
    NOFADE
    ENDA
};

static const EventScr EventScr_District_Serafew_ItemAfter[] = {
    TEXT(District_Serafew_ItemAfter)
    NOFADE
    ENDA
};

static const EventScr EventScr_District_Serafew_Hint[] = {
    TEXT(District_Serafew_Hint)
    NOFADE
    ENDA
};

static const EventScr EventScr_District_Serafew_HexShop[] = {
    ASMC(StartDistrictHexShopASMC)
    NOFADE
    ENDA
};

/**
 * Event list
 */

static const EventListScr EventListScr_Serafew_Turn[] = {
    END_MAIN
};

static const EventListScr EventListScr_Serafew_Character[] = {
    CharacterEventBothWays(GLOBAL_FLAG_DISTRICT_SERAFEW_GOLD, EventScr_District_Serafew_Gold, CHARACTER_EIRIKA, CHARACTER_CITIZEN)
    CharacterEventBothWays(0, EventScr_District_Serafew_GoldAfter, CHARACTER_EIRIKA, CHARACTER_CITIZEN)
    CharacterEventBothWays(GLOBAL_FLAG_DISTRICT_SERAFEW_ITEM, EventScr_District_Serafew_Item, CHARACTER_EIRIKA, CHARACTER_CITIZEN_YOUNG_GIRL)
    CharacterEventBothWays(0, EventScr_District_Serafew_ItemAfter, CHARACTER_EIRIKA, CHARACTER_CITIZEN_YOUNG_GIRL)
    CharacterEventBothWays(0, EventScr_District_Serafew_HexA, CHARACTER_EIRIKA, CHARACTER_CITIZEN_YOUNG_MAN)
    CharacterEventBothWays(0, EventScr_District_Serafew_HexB, CHARACTER_EIRIKA, CHARACTER_CITIZEN_TEEN_GIRL)
    CharacterEventBothWays(0, EventScr_District_Serafew_Hint, CHARACTER_EIRIKA, 0x7F)
    END_MAIN
};

static const EventListScr EventListScr_Serafew_Location[] = {
    ARMORY(EventListScr_CH5_ARMORY, 3, 3)
    VENDOR(EventListScr_CH5_VENDOR, 8, 3)
    VILLAGE(EVFLAG_TMP(10), EventListScr_HOUSE_1_TALK, 15, 3)
    VILLAGE(EVFLAG_TMP(11), EventListScr_HOUSE_2_TALK, 4, 10)
    VILLAGE(EVFLAG_TMP(12), EventListScr_HOUSE_3_TALK, 8, 15)
    VILLAGE(EVFLAG_TMP(13), EventListScr_HOUSE_4_TALK, 16, 15)
    HOUSE(0, EventScr_District_Serafew_HexShop, 19, 10)
    END_MAIN
};

static const EventListScr EventListScr_Serafew_Misc[] = {
    END_MAIN
};

const struct ChapterEventGroup SerafewEvent = {
    .turnBasedEvents               = EventListScr_Serafew_Turn,
    .characterBasedEvents          = EventListScr_Serafew_Character,
    .locationBasedEvents           = EventListScr_Serafew_Location,
    .miscBasedEvents               = EventListScr_Serafew_Misc,
    .specialEventsWhenUnitSelected = EventListScr_Empty,
    .specialEventsWhenDestSelected = EventListScr_Empty,
    .specialEventsAfterUnitMoved   = EventListScr_Empty,
    .tutorialEvents                = EventListScr_Tutorial_None,

    .traps            = TrapData_Serafew,
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
