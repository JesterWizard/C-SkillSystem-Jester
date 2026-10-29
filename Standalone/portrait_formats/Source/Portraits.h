#pragma once

/*
 * Half-body portraits and 32-colour portraits for FE8U.
 *
 * Both formats are recognised from the portrait table entry, so normal
 * portraits keep vanilla behaviour:
 *  - half-body: imgChibi == img + 0x2644 (or + 0x2648), see setMugEntry_Halfbody
 *  - 32-colour: imgCard == pal + 16 colours, see setMugEntry_32Color
 *
 * A face in one of these formats needs a second 0x1000-byte block of OBJ
 * VRAM right after its own (the half-body's lower half, or the 32-colour
 * overlay), and a 32-colour face also needs the OBJ palette after its own.
 * Portraits.c hands those out per face slot (see ChooseResources).
 */

#include "gbafe.h"

/* Display kind used by the patched map menus and level-up screens. */
#define PORTRAITS_KIND_MENU 6

enum {
	PORTRAIT_MODE_NORMAL,
	PORTRAIT_MODE_HALFBODY,
	PORTRAIT_MODE_COLOR32,
};

struct PortraitSlot {
	/* 00 */ u8 mode;
	/* 01 */ u8 resSlot;     /* sFaceConfig entry the face draws from */
	/* 02 */ u8 extPal;      /* COLOR32: overlay OBJ palette (0-15) */
	/* 03 */ u8 hasMouth;    /* mouth/blink procs were started for it */
	/* 04 */ u16 extChr;     /* COLOR32: overlay tile number */
	/* 06 */ u16 savedPal[16];
	/* 26 */ u16 pad;
};

extern struct PortraitSlot sPortraitSlots[4];

/* For the dialogue code (vanilla talk here, or the text engine). */
bool Portraits_IsHalfBodyFace(struct FaceProc *face);
bool Portraits_TalkFaceWillBeHalfBody(int fid, int disp);
void HalfBody_OnTalkFaceClear(struct FaceProc *face);
s32 Portrait32_GetOverlayOam2(struct FaceProc *face);
