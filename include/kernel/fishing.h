#pragma once

#include "common-chax.h"

/*
 * District fishing (CONFIG_DISTRICT_FISHING, Kernel/Wizardry/Minigames/Fishing).
 *
 * A fishing spot is a free-movement trap, TRAP_FISHING_SPOT (bmtrick.h), on a walkable tile next to water. Its subtype
 * is the side the water is on (FISHING_DIR_*), which places the bobber marker on the map. Standing on the spot and
 * pressing A opens the fishing screen.
 */

enum {
	FISHING_DIR_LEFT = 0,
	FISHING_DIR_RIGHT = 1,
	FISHING_DIR_UP = 2,
	FISHING_DIR_DOWN = 3,
};

/* The fish, in prize table order */
enum {
	FISH_SMALL = 0,
	FISH_MEDIUM = 1,
	FISH_LARGE = 2,
	FISH_HUGE = 3,
	FISH_GOLDEN = 4,
	FISH_TYPES = 5,
};

/*
 * The waters: one for each district (chosen by the chapter). Each has five species, one for each kind of fish above,
 * so a species is water * FISH_TYPES + kind. The journal records them.
 */
enum {
	FISHING_WATER_SERAFEW = 0,
	FISHING_WATER_PORT_KIRIS = 1,
	FISHING_WATER_CAER_PELYN = 2,
	FISHING_WATER_JEHANNA = 3,
	FISHING_WATERS = 4,
};

#define FISHING_JOURNAL_SPECIES (FISHING_WATERS * FISH_TYPES)

enum {
	BAIT_WORM = 0,
	BAIT_SHRIMP = 1,
	BAIT_LURE = 2,
	BAIT_TYPES = 3,
	BAIT_NONE = 0xFF,
};

#define FISHING_FISH_MAX 8

struct FishingFish {
	/* 00 */ s32 x;         /* left edge, pixels << 8 */
	/* 04 */ s16 vx;        /* pixels << 8 per frame; > 0 swims right (the art faces right) */
	/* 06 */ s16 y;         /* centre row in the scene, with the bob */
	/* 08 */ s16 baseY;     /* centre row without the bob */
	/* 0A */ u8 type;       /* FISH_* */
	/* 0B */ u8 mode;       /* FISHM_* (FishingScreen.c) */
	/* 0C */ u16 timer;
	/* 0E */ u8 pad[2];
};

struct FishingState {
	/* 00 */ struct FishingFish fish[FISHING_FISH_MAX];
	/* 80 */ s16 hookY;      /* scene row of the hook */
	/* 82 */ s16 targetY;    /* the depth the player set */
	/* 84 */ s16 camY;       /* first scene row on screen */
	/* 86 */ s16 reelStartY;
	/* 88 */ s16 tension;    /* 0..FISHING_TENSION_MAX */
	/* 8A */ s16 progress;   /* frames spent in the green zone */
	/* 8C */ u16 timer;      /* state timer */
	/* 8E */ u16 reelFrames; /* frames since the fish was hooked */
	/* 90 */ u16 slack;      /* frames at zero tension */
	/* 92 */ u16 burst;      /* frames left of the fish's run */
	/* 94 */ s16 unitX;      /* the unit's sprite x (it walks in from the right) */
	/* 96 */ u16 msg;        /* message of FS_MESSAGE */
	/* 98 */ u32 payout;     /* gold of the last catch */
	/* 9C */ u32 line1Sig;   /* what the window's first line shows (redrawn when it changes) */
	/* A0 */ s8 biter;       /* the fish on (or at) the hook, -1: none */
	/* A1 */ u8 state;       /* FS_* (FishingScreen.c) */
	/* A2 */ u8 prevState;   /* the state the prize table returns to */
	/* A3 */ u8 bait;        /* the selected bait, BAIT_* */
	/* A4 */ u8 baited;      /* the bait on the hook, BAIT_NONE: none */
	/* A5 */ u8 hookQ;       /* hook bonus, quarters */
	/* A6 */ u8 landQ;       /* landing bonus, quarters */
	/* A7 */ u8 splash;      /* frames of splash left */
	/* A8 */ s16 nibbleY;    /* hookY when the nibble began */
	/* AA */ u8 unitFrame;   /* frame of the unit's walking sheet on screen */
	/* AB */ u8 bonusTimer;  /* frames the hook bonus line stays up */
	/* AC */ u8 smsPal;      /* the unit's OBJ palette bank */
	/* AD */ u8 line2Key;    /* which controls the window's second line shows */
	/* AE */ u8 hintCount;   /* button icons on the second line */
	/* AF */ u8 lastType;    /* the type of the last catch */
	/* B0 */ u16 biteClock;  /* frames since the bite */
	/* B2 */ u8 hintIcon[4];
	/* B6 */ u8 hintX[4];
	/* BA */ struct Text texts[8]; /* the window's two lines and the prize table's six rows */
	/* FA */ s16 rodAngle;   /* the rod's angle, 256 to a turn, << 8 */
	/* FC */ s8 shakeX;      /* screen shake offset */
	/* FD */ s8 shakeY;
	/* FE */ u8 shake;       /* frames of shake left */
	/* FF */ u8 popWord;     /* FISHING_POP_*, or 0xFF: no pop-up */
	/* 100 */ u8 popTimer;
	/* 101 */ u8 popX;       /* the pop-up's centre on screen */
	/* 102 */ u8 popY;
	/* 103 */ u8 pad103;
	/* 104 */ struct FishingParticle {
		s16 x, y;      /* screen pixels << 4 */
		s8 vx, vy;     /* pixels << 4 per frame */
		u8 life;       /* frames left, 0: free */
		u8 kind;       /* 0 spark, 1 coin */
	} parts[16];
	/* 184 */ u8 water;        /* FISHING_WATER_* of this district */
	/* 185 */ u8 species;      /* the last catch */
	/* 186 */ u8 cm;           /* its size */
	/* 187 */ u8 prevBest;     /* the species' best size before it (0: a first catch) */
	/* 188 */ u8 stamp;        /* FISHING_POP_NEW or FISHING_POP_RECORD on the catch card, 0xFF: none */
	/* 189 */ u8 cardT;        /* frames since the catch card opened, 0: closed */
	/* 18A */ u8 page;         /* the journal's water */
	/* 18B */ u8 pad18B[0x15];
};

/*
 * Saved records, the fish journal: how many of each species were landed and the biggest (a save chunk in both save
 * lists, 0x2C bytes). A species with caught 0 is not found yet.
 */
struct FishingRecords {
	/* 00 */ u16 magic;
	/* 02 */ u8 caught[FISHING_JOURNAL_SPECIES]; /* up to 255 */
	/* 16 */ u8 best[FISHING_JOURNAL_SPECIES];   /* centimetres */
	/* 2A */ u16 pad;
};

/* What a catch added to the journal (FishingRecords_Add) */
enum {
	FISHING_JOURNAL_SAME = 0,
	FISHING_JOURNAL_NEW = 1,    /* the first of its species */
	FISHING_JOURNAL_RECORD = 2, /* bigger than the best so far */
};

extern struct FishingState gFishingState;
extern struct FishingRecords gFishingRecords;

/* Free-movement A-press trap handler (FreeMovement/cFreeMU/HookList.event): starts the fishing event. */
void Fishing_OnPressA(ProcPtr proc);

/* ASMC: the map fades out, the fishing screen runs, the map comes back. */
void StartDistrictFishingASMC(ProcPtr parent);

extern const struct ProcCmd ProcScr_FishingScreen[];

/* District map: the bobber markers on the water next to each fishing spot (DistrictNpc.c). */
void Fishing_LoadMapMarker(int chr);
void Fishing_DrawMapMarkers(int chr, int pal);

/* Records (FishingRecords.c) */
int FishingRecords_Add(int species, int cm);
int FishingRecords_Found(void);
int FishingRecords_Caught(int species);
int FishingRecords_Best(int species);
void FishingRecords_NewGame(void);
void FishingRecords_Save(u8 *dst, const u32 size);
void FishingRecords_Load(u8 *src, const u32 size);
