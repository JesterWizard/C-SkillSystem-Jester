#pragma once

#define TEXT(msg) \
    TEXTSTART \
    TEXTSHOW(msg) \
    TEXTEND \
    REMA

#define TEXT_CG(bg, msg) \
    SVAL(EVT_SLOT_2, (bg)) \
    SVAL(EVT_SLOT_3, (msg)) \
    CALL(EventScr_SetBackground) \
    SADD(EVT_SLOT_2, EVT_SLOT_3, EVT_SLOT_0) \
    CGTEXTBOXSTART \
    TEXTSHOW(-1) \
    TEXTEND \
    CALL(EventScr_TextShowWithFadeIn)

#define TEXT_CONSECUTIVE(msg) \
    EvtTextShow2(msg) \
    TEXTEND

#define TEXT_BG(bg, msg) \
    SVAL(EVT_SLOT_2, (bg)) \
    SVAL(EVT_SLOT_3, (msg)) \
    CALL(Event_TextWithBG)

#define SET_BACKGROUND(bg) \
    SVAL(EVT_SLOT_2, bg) \
    CALL(EventScr_SetBackground)
