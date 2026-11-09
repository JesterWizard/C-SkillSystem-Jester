#pragma once

#include "common-chax.h"

/*
 * District Tower Stack (CONFIG_DISTRICT_TOWER, Kernel/Wizardry/Minigames/Tower): a block-stacking minigame.
 *
 * A Tower Stack spot is a free-movement trap, TRAP_TOWER_SPOT (bmtrick.h), on a walkable district tile. Standing on it
 * and pressing A opens the game. A small tower marks the spot on the map.
 */

#define TOWER_RING 16           /* the last blocks of the tower, enough to fill the screen */
#define TOWER_PARTS 12

struct TowerState {
	/* 00 */ s32 x;                      /* the moving block's left edge, pixels << 8 */
	/* 04 */ s32 vx;                     /* pixels << 8 per frame */
	/* 08 */ s16 dropY;                  /* the moving block's top, scene rows */
	/* 0A */ s16 fallVY;                 /* a falling block's speed, pixels << 4 */
	/* 0C */ s16 camY;                   /* the scene row at the top of the screen (negative above the sky) */
	/* 0E */ s16 xs[TOWER_RING];         /* the left edge of block n is xs[n & 15] */
	/* 2E */ u8 styles[TOWER_RING];      /* its look */
	/* 3E */ u16 height;                 /* blocks in the tower, the base included */
	/* 40 */ u16 score;
	/* 42 */ u16 best;
	/* 44 */ u16 fever;                  /* frames of fever left: missed blocks cost no lives */
	/* 46 */ u16 timer;                  /* state timer */
	/* 48 */ u16 gold;                   /* the payout */
	/* 4A */ u8 lives;
	/* 4B */ u8 chain;                   /* perfect landings in a row (1..10) */
	/* 4C */ u8 state;
	/* 4D */ u8 prevState;
	/* 4E */ u8 style;                   /* the moving block's look */
	/* 4F */ u8 perfect;                 /* the last landing was perfect */
	/* 50 */ u8 popWord;                 /* TOWER_POP_*, 0xFF: none */
	/* 51 */ u8 popTimer;
	/* 52 */ u8 popX;
	/* 53 */ u8 popY;
	/* 54 */ u8 numLen;                  /* the digits pop-up: "+1", "x7", ... */
	/* 55 */ u8 numTimer;
	/* 56 */ u8 numX;
	/* 57 */ u8 numY;
	/* 58 */ u8 num[4];                  /* glyphs: 0-9, 10 "+", 11 "x" */
	/* 5C */ u8 hintKey;
	/* 5D */ u8 hintCount;
	/* 5E */ u8 hintIcon[4];
	/* 62 */ u8 hintX[4];
	/* 66 */ u8 shake;
	/* 67 */ s8 shakeX;
	/* 68 */ u32 line1Sig;
	/* 6C */ struct Text texts[2];       /* the status line and the button labels */
	/* 7C */ struct TowerParticle {
		s16 x, y;      /* screen pixels << 4 */
		s8 vx, vy;     /* pixels << 4 per frame */
		u8 life;
		u8 pad;
	} parts[TOWER_PARTS];
	/* DC */ u8 numRise;                 /* the digits pop-up rises (a plain landing) */
	/* DD */ u8 hitStop;                 /* frames the game is frozen after a perfect landing */
	/* DE */ u8 squash;                  /* frames of the top block's squash left */
	/* DF */ u8 squashBig;               /* the squash is a perfect landing's (deeper) */
	/* E0 */ u8 flash;                   /* frames of the background flash left */
	/* E1 */ u8 pendWord;                /* a pop-up word waiting for the current one to end, 0xFF: none */
	/* E2 */ u8 pendPal;
	/* E3 */ u8 pendNum;                 /* its number ("10 FLOORS!"), 0: none */
	/* E4 */ u8 newBest;                 /* NEW BEST! has been shown this game */
	/* E5 */ u8 shakeAmp;
	/* E6 */ u8 pad[0x1A];
};

/* Saved record: the best score (a save chunk in both save lists, 4 bytes). */
struct TowerRecords {
	/* 00 */ u16 magic;
	/* 02 */ u16 best;
};

extern struct TowerState gTowerState;
extern struct TowerRecords gTowerRecords;

/* Free-movement A-press trap handler (FreeMovement/cFreeMU/HookList.event): starts the game's event. */
void Tower_OnPressA(ProcPtr proc);

/* ASMC: the map fades out, the game runs, the map comes back. */
void StartDistrictTowerASMC(ProcPtr parent);

extern const struct ProcCmd ProcScr_TowerScreen[];

/* District map: the tower on each spot (DistrictNpc.c). */
void Tower_LoadMapMarker(int chr);
void Tower_DrawMapMarkers(int chr, int pal);

/* Records (TowerRecords.c) */
int TowerRecords_Best(void);
void TowerRecords_Add(u32 score);
void TowerRecords_NewGame(void);
void TowerRecords_Save(u8 *dst, const u32 size);
void TowerRecords_Load(u8 *src, const u32 size);
