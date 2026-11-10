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

/* Medals: the district's best score earns one (CrossingRecords_Medal). */
#define CROSSING_MEDAL_BRONZE 500
#define CROSSING_MEDAL_SILVER 1200
#define CROSSING_MEDAL_GOLD 2500

/* The per-chapter challenge: met in round 1 (RESCUE: in the whole game). */
#define CROSSING_CHAL_FAST_SECS 60

enum crossing_pick {
	CROSSING_PICK_NONE,
	CROSSING_PICK_COIN,   /* +50 */
	CROSSING_PICK_POTION, /* +1 life */
};

enum crossing_child {
	CROSSING_CHILD_NONE,
	CROSSING_CHILD_RAFT,    /* riding a raft, waiting */
	CROSSING_CHILD_CARRIED, /* on the unit's back */
};

enum crossing_chal {
	CROSSING_CHAL_ORDER,   /* fill the docks left to right */
	CROSSING_CHAL_FAST,    /* clear round 1 in CROSSING_CHAL_FAST_SECS */
	CROSSING_CHAL_RESCUE,  /* rescue a lost child */
	CROSSING_CHAL_NODEATH, /* clear round 1 without a miss */
	CROSSING_CHAL_COUNT,
};

enum crossing_reward {
	CROSSING_REWARD_NONE,
	CROSSING_REWARD_FIRST,  /* the district's first clear: an Elixir */
	CROSSING_REWARD_ROUND5, /* the district's first round 5: a stat booster */
	CROSSING_REWARD_CHAL,   /* the chapter's challenge: 1000 gold */
};

/* The twists, pickups, challenge and records screen (RAM only, 0x80 bytes). */
struct CrossingExtra {
	/* 00 */ u16 tick;          /* play frames since the game opened */
	/* 02 */ u16 roundFrames;   /* frames into round 1 (the fastest-clear timer) */
	/* 04 */ s16 gullX;         /* the gull's x on screen */
	/* 06 */ u8 gullRow;        /* its row, 0xFF: no gull */
	/* 07 */ u8 gullWarn;       /* frames of the warning left */
	/* 08 */ u16 gullNext;      /* frames before the next gull */
	/* 0A */ u16 stormNext;     /* frames before the next sandstorm */
	/* 0C */ u8 stormLane;      /* the road lane it hides, 0xFF: none */
	/* 0D */ u8 stormT;         /* frames of storm left */
	/* 0E */ u8 slide;          /* an ice slide is under way (no second one) */
	/* 0F */ u8 pickKind;       /* CROSSING_PICK_* */
	/* 10 */ u8 pickLane;
	/* 11 */ u8 pickObj;        /* the raft in its lane */
	/* 12 */ u8 pickCell;       /* the cell of that raft */
	/* 13 */ u8 pickT;          /* the pickup's sparkle timer */
	/* 14 */ u16 pickNext;      /* frames before the next pickup */
	/* 16 */ u8 childState;     /* CROSSING_CHILD_* */
	/* 17 */ u8 childLane;
	/* 18 */ u8 childObj;
	/* 19 */ u8 childCell;
	/* 1A */ u16 childNext;     /* frames before the next lost child */
	/* 1C */ u8 bonusDock;      /* the dock with a star or a skull, 0xFF: none */
	/* 1D */ u8 bonusKind;      /* 0 a star (double points), 1 a skull (a trap) */
	/* 1E */ u16 bonusT;        /* frames it stays */
	/* 20 */ u16 bonusNext;     /* frames before the next one */
	/* 22 */ u8 chalKind;       /* CROSSING_CHAL_* */
	/* 23 */ u8 chalFailed;
	/* 24 */ u8 chalDone;       /* met this game */
	/* 25 */ u8 nextDock;       /* ORDER: the dock to fill next */
	/* 26 */ u8 rewardMsg;      /* CROSSING_REWARD_*, shown at game over */
	/* 27 */ u8 medalBefore;    /* the district's medal before this game */
	/* 28 */ u16 rewardItem;    /* the item given */
	/* 2A */ u8 rewardT;
	/* 2B */ u8 recordsDrawn;
	/* 2C */ u8 rewardQueue;    /* rewards waiting for the status line, a bit per CROSSING_REWARD_* */
	/* 2D */ u8 pad0[0x0B];
	/* 38 */ struct Text texts[6]; /* the records screen */
	/* 68 */ u8 pad[0x18];
};

/* Saved progress (a save chunk in both save lists, 0x10 bytes). */
struct CrossingProgress {
	/* 00 */ u16 magic;
	/* 02 */ u16 fastest[CROSSING_LEVELS];  /* the fastest round 1 clear, seconds, 0: none */
	/* 0A */ u8 milestones;                 /* bits 0-3: first clear, bits 4-7: round 5 (bit = level + 4 * which) */
	/* 0B */ u8 pad;
	/* 0C */ u8 chalStory[CROSSING_LEVELS]; /* the story chapter whose challenge was met, 0xFF: none */
};

extern struct CrossingState gCrossingState;
extern struct CrossingRecords gCrossingRecords;
extern struct CrossingExtra gCrossingExtra;
extern struct CrossingProgress gCrossingProgress;

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
int CrossingRecords_Medal(int level);
int CrossingRecords_Fastest(int level);
void CrossingRecords_SetFastest(int level, int seconds);
bool CrossingRecords_Milestone(int level, int which);
void CrossingRecords_SetMilestone(int level, int which);
bool CrossingRecords_ChalDone(int level, int chapter);
void CrossingRecords_SetChalDone(int level, int chapter);
void CrossingProgress_NewGame(void);
void CrossingProgress_Save(u8 *dst, const u32 size);
void CrossingProgress_Load(u8 *src, const u32 size);
