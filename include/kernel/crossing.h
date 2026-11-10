#pragma once

#include "common-chax.h"

/*
 * District River Crossing (CONFIG_DISTRICT_CROSSING, Kernel/Wizardry/Minigames/Crossing): a lane-crossing minigame.
 *
 * A crossing spot is a free-movement trap, TRAP_CROSSING_SPOT (bmtrick.h), on a walkable district tile. Standing on it
 * and pressing A opens the game. A flag marks the spot on the map.
 */

#define CROSSING_LANES 6
#define CROSSING_LEVELS 4
#define CROSSING_PARTS 8

struct CrossingState {
	/* 00 */ s32 laneX[CROSSING_LANES];  /* lane offsets, pixels << 8, in [0, period) */
	/* 18 */ s32 px;                     /* the unit's x (left edge of its cell), pixels << 8 */
	/* 1C */ u32 score;
	/* 20 */ s16 py;                     /* the unit's y on screen (top of its cell) */
	/* 22 */ u16 timer;                  /* frames left for this life */
	/* 24 */ u16 stateTimer;
	/* 26 */ u16 best;                   /* this district's best score, shown on the status line */
	/* 28 */ u32 line1Sig;
	/* 2C */ u8 row;                     /* the unit's row (9 start .. 1 the docks) */
	/* 2D */ u8 hopFrames;               /* frames left of a hop */
	/* 2E */ s8 hopDX;                   /* the hop's direction */
	/* 2F */ s8 hopDY;
	/* 30 */ u8 facing;                  /* 0 left, 1 right, 2 down, 3 up */
	/* 31 */ u8 lives;
	/* 32 */ u8 round;
	/* 33 */ u8 state;
	/* 34 */ u8 prevState;
	/* 35 */ u8 docks;                   /* filled docks, a bit each */
	/* 36 */ u8 maxRow;                  /* the furthest row this life (scored once) */
	/* 37 */ u8 level;                   /* the district's course, 0..CROSSING_LEVELS-1 */
	/* 38 */ u8 smsPal;
	/* 39 */ u8 riderPal;
	/* 3A */ u8 hintKey;
	/* 3B */ u8 hintCount;
	/* 3C */ u8 hintIcon[4];
	/* 40 */ u8 hintX[4];
	/* 44 */ u8 popWord;
	/* 45 */ u8 popTimer;
	/* 46 */ u8 popX;
	/* 47 */ u8 popY;
	/* 48 */ u8 death;                   /* what ended the life: CROSSING_DEATH_* (CrossingScreen.c) */
	/* 49 */ u8 shake;
	/* 4A */ s8 shakeX;
	/* 4B */ u8 splashX;
	/* 4C */ u8 splashY;
	/* 4D */ u8 splash;                  /* frames of splash left */
	/* 4E */ u16 gold;                   /* the payout of the game */
	/* 50 */ struct Text texts[3];       /* the status line and the button labels */
	/* 68 */ struct CrossingParticle {
		s16 x, y;      /* screen pixels << 4 */
		s8 vx, vy;     /* pixels << 4 per frame */
		u8 life;
		u8 kind;       /* 0 a spark, 1 a water droplet */
	} parts[CROSSING_PARTS];
	/* A8 */ u8 hitStop;                 /* frames the game stands still after a trampling */
	/* A9 */ u8 chain;                   /* docks filled in a row without losing a life (the note climbs) */
	/* AA */ u8 roundDeaths;             /* lives lost this round (none: PERFECT!) */
	/* AB */ u8 nearCool;                /* frames before another near miss counts */
	/* AC */ u8 squash;                  /* frames of the landing squash left */
	/* AD */ u8 dustT;                   /* frames of the landing dust left */
	/* AE */ u8 dustX;
	/* AF */ u8 dustY;
	/* B0 */ u8 rippleT;                 /* frames of the raft ripple left */
	/* B1 */ u8 sinkT;                   /* frames of the sinking left (drowned) */
	/* B2 */ u16 shownScore;             /* the final score as it counts up */
	/* B4 */ u8 countDone;
	/* B5 */ u8 newBest;
	/* B6 */ u8 numTimer;                /* frames of the score pop-up ("+10") left */
	/* B7 */ u8 numLen;
	/* B8 */ u8 num[6];                  /* its glyphs: 0-9, 10 "+", 11 "x" */
	/* BE */ u8 numX;
	/* BF */ u8 numY;
	/* C0 */ u8 shakeAmp;
	/* C1 */ u8 pendPop;                 /* a pop-up word waiting for the one on screen, 0xFF: none */
	/* C2 */ u8 pad[0x3E];
};

/* Saved records: the best score of each district's course (a save chunk in both save lists, 0x0C bytes). */
struct CrossingRecords {
	/* 00 */ u16 magic;
	/* 02 */ u16 best[CROSSING_LEVELS];
	/* 0A */ u16 pad;
};

extern struct CrossingState gCrossingState;
extern struct CrossingRecords gCrossingRecords;

/* Free-movement A-press trap handler (FreeMovement/cFreeMU/HookList.event): starts the game's event. */
void Crossing_OnPressA(ProcPtr proc);

/* ASMC: the map fades out, the game runs, the map comes back. */
void StartDistrictCrossingASMC(ProcPtr parent);

extern const struct ProcCmd ProcScr_CrossingScreen[];

/* District map: the flag on each crossing spot (DistrictNpc.c). */
void Crossing_LoadMapMarker(int chr);
void Crossing_DrawMapMarkers(int chr, int pal);

/* Records (CrossingRecords.c) */
int CrossingRecords_Best(int level);
void CrossingRecords_Add(int level, u32 score);
void CrossingRecords_NewGame(void);
void CrossingRecords_Save(u8 *dst, const u32 size);
void CrossingRecords_Load(u8 *src, const u32 size);
