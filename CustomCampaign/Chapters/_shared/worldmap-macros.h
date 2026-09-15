#pragma once

/**
 * EA-friendly world map event authoring macros.
 * Each macro expands to the same EventScr opcodes the vanilla engine expects.
 */

#include "EAstdlib.h"
#include "event.h"

/* Aliases for Mokha-only WmEvt* commands not exposed in EAstdlib.h */
#define WM_NOFADE WmEvtNOFADE
#define WM_SETNODESTATENOT2 WmEvtSetNodeStateNot2
#define WM_SETUNITONNODE WmEvtSetUnitOnNode
#define WM_SETCAMTONODE WmEvtSetCamToNode
#define WM_SETNEXTSTORYNODE WmEvtSetNextStoryNode
#define WM_MOVECAMTOUNIT WmEvtMoveCamToUnit

/* Portrait placement on the world-map text box */
#define WM_FACE_LEFT  0x02BC
#define WM_FACE_RIGHT 0x0534

/* WM_CLEARPORTRAIT slide-out values paired with each side */
#define WM_FACE_CLEAR_LEFT  0x0100
#define WM_FACE_CLEAR_RIGHT 0x0200
#define WM_FACE_SLIDE_LEFT  0x01BC
#define WM_FACE_SLIDE_RIGHT 0x0634

/* Voice-acted narration beat during WM_TEXT */
#define WM_VOICE_LINE(song) \
    SOUN(song) \
    TEXTCONT \
    TEXTEND

/* Shared SET_NODE prelude: spawn lord, fade map, wait for fade */
#define WM_OPEN_MAP(pid, node) \
    EVBIT_MODIFY(0x1) \
    WM_SPAWNLORD(WM_MU_0, pid, node) \
    WM_CENTERCAMONLORD(WM_MU_0) \
    WM_FADEOUT(0) \
    WM_TEXTDECORATE \
    EVBIT_MODIFY(0x0) \
    STAL(20)

/* Reveal the next destination node and draw a dynamically generated road */
#define WM_REVEAL_DEST(node) \
    WM_LOADLOCATION3(node) \
    WM_SETDESTINATION(node) \
    WM_WAITFORFX \
    STAL(40) \
    ASMC(WmDrawPathFromCurrentToDest) \
    STAL(70)

/* Shared SET_NODE epilogue: show lord and hand control back to the player */
#define WM_CLOSE_SET_NODE() \
    WM_MAKELORDVISIBLE(WM_MU_0) \
    EVBIT_MODIFY(0x1) \
    CALL(EventScr_RemoveBGIfNeeded) \
    ENDA

/* Show a portrait without nation highlight */
#define WM_SHOW_FACE(slot, face, side) \
    WM_SHOWPORTRAIT(slot, face, side, 0) \
    STAL(6)

#define WM_SHOW_FACE_WAIT(slot, face, side, wait) \
    WM_SHOW_FACE(slot, face, side) \
    STAL(wait)

/* Hide a portrait; pass WM_FACE_CLEAR_* or WM_FACE_SLIDE_* for clearSlide */
#define WM_HIDE_FACE(slot, clearSlide) \
    WM_CLEARPORTRAIT(slot, clearSlide, 0) \
    STAL(46)

#define WM_HIDE_FACE_WAIT(slot, clearSlide, wait) \
    WM_CLEARPORTRAIT(slot, clearSlide, 0) \
    STAL(wait)

/* Nation highlight sequence used in prologue-style map narration */
#define WM_NATION_ON(face, side, nation) \
    WM_SHOWPORTRAIT(0, face, side, 0) \
    STAL(6) \
    STAL(26) \
    WM_HIGHLIGHT(nation)

#define WM_NATION_OFF_CORE(nation, clearSlide) \
    WM_HIGHLIGHTCLEAR1(nation) \
    WM_HIGHLIGHTCLEAR2(nation) \
    WM_CLEARPORTRAIT(0, clearSlide, 0)

#define WM_NATION_OFF(nation, clearSlide) \
    WM_NATION_OFF_CORE(nation, clearSlide) \
    STAL(32)
