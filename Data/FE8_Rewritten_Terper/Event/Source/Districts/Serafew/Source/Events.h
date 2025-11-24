/**
 * Main events
 */
static const EventScr EventScr_Beginning[] = {
    // CHECK_EVENTID(136) // Flag 0x88
    // BEQ(0x1, EVT_SLOT_C, EVT_SLOT_0)
    // CALL(EventScr_NEW_JOURNEY)
    // GOTO(0x2)
    STAL(15)
    MUSC(SONG_DISTANT_ROADS)
    LOAD_WAIT_PERSIST(Base_Chapter_5_Eirika)
    MOVE_WAIT(0, CHARACTER_EIRIKA, 6, 17)
    ASMC(SetGameOptions_BaseChapter5)
    NOFADE
    ENDA
};

static const EventScr EventScr_Ending[] = {
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
 * Event list
 */

static const EventListScr EventListScr_Turn[] = {
    END_MAIN
};

static const EventListScr EventListScr_Character[] = {
    END_MAIN
};

static const EventListScr EventListScr_Location[] = {
    ARMORY(EventListScr_CH5_ARMORY, 2, 1)
    VENDOR(EventListScr_CH5_VENDOR, 6, 10)
    VILLAGE(EVFLAG_TMP(10), EventListScr_HOUSE_1_TALK, 12, 10)
    VILLAGE(EVFLAG_TMP(11), EventListScr_HOUSE_2_TALK, 12, 19)
    VILLAGE(EVFLAG_TMP(12), EventListScr_HOUSE_3_TALK, 5, 6)
    VILLAGE(EVFLAG_TMP(13), EventListScr_HOUSE_4_TALK, 5, 1)
    END_MAIN
};

static const EventListScr EventListScr_Misc[] = {
    END_MAIN
};

static const EventListScr EventListScr_SelectUnit[] = {
    END_MAIN
};

static const EventListScr EventListScr_SelectDestination[] = {
    END_MAIN
};

static const EventListScr EventListScr_UnitMove[] = {
    END_MAIN
};

static void const * const EventListScr_Tutorial[] = {
    NULL
};