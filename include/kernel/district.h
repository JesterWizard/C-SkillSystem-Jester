#pragma once

#include "common-chax.h"

/*
 * Districts: base chapters entered from a world-map node with "Enter District".
 * Table in Kernel/Wizardry/EnterTown/District.c.
 */

#define DISTRICT_NODE_END 0xFF

struct DistrictNode {
	u8 node;		/* world-map node (NODE_*) */
	u8 chapter;		/* base chapter loaded inside the district */
	u8 storyChapter;	/* story chapter whose clear unlocks it */
	u8 pad;
	u16 unlockFlag;		/* global flag set when storyChapter is cleared */
};

extern const struct DistrictNode gDistrictNodes[];

const struct DistrictNode *GetDistrictAtNode(int node);
bool District_IsUnlocked(const struct DistrictNode *district);
void District_MarkStoryChapterCleared(int storyChapter);

/* Free-movement auto trap (HookList.event): stepping on a district gate asks to leave */
void DistrictGate_OnStep(ProcPtr proc);

enum { DISTRICT_NPC_HEX = 1 << 0 };

/* Wandering district NPC (Kernel/Wizardry/EnterTown/DistrictNpc.c) */
struct DistrictNpc {
	u8 chapter;
	u8 pid;
	u8 radius;	/* wander box: home +/- radius tiles; 0 = stands still */
	u8 flags;	/* DISTRICT_NPC_* */
};

extern const struct DistrictNpc gDistrictNpcs[];	/* ends with chapter 0xFF */

const struct DistrictNpc *GetDistrictNpc(int chapter, int pid);
/* ASMC: no arguments (the event engine calls it bare, like EnableFreeMovementASMC) */
void StartDistrictNpcs(void);

/* Ends an NPC walk in progress (the NPC proc does not run while an event does). For screens that take over OBJ VRAM. */
void DistrictNpc_EndWalk(void);

/*
 * District life (Kernel/Wizardry/EnterTown/DistrictLife.c, tables in Data/District/DistrictLife.c). Both follow the
 * world-map story: a row counts once the story has reached its chapter, in story order (WmStoryReached).
 *
 * Time of day: a district map's palettes are tinted by the time of day, the time of the last gDistrictTimes row whose
 * story chapter has come. The tint is applied whenever the map's palette loads (UnpackChapterMapPalette, replaced).
 */
enum district_time {
	DISTRICT_TIME_DAY,
	DISTRICT_TIME_EVENING,
	DISTRICT_TIME_NIGHT,
	DISTRICT_TIME_MORNING,
	DISTRICT_TIMES,
};

#define DISTRICT_LIFE_END 0xFF

struct DistrictTime {
	u8 storyChapter;  /* from this story chapter on (gDistrictTimes, in story order, ends with DISTRICT_LIFE_END) */
	u8 time;          /* DISTRICT_TIME_* */
};

/* A tint: each colour channel (0-31) becomes channel * mul / 16 + add, at most 31 */
struct DistrictTint {
	u8 mul[3];        /* red, green, blue */
	u8 add[3];
};

/*
 * Townsfolk's places: the last gDistrictNpcSpots row for a district and an NPC whose story chapter has come puts that
 * NPC there when the district opens, if the tile is free (it then wanders around it). Without one, the NPC stands
 * where the district's events load it.
 */
struct DistrictNpcSpot {
	u8 chapter;       /* the district chapter (gDistrictNpcSpots ends with DISTRICT_LIFE_END) */
	u8 pid;
	u8 storyChapter;  /* from this story chapter on */
	s8 x, y;
	u8 pad;
};

extern const struct DistrictTime gDistrictTimes[];
extern const struct DistrictTint gDistrictTints[DISTRICT_TIMES];
extern const struct DistrictNpcSpot gDistrictNpcSpots[];

bool District_IsDistrictChapter(int chapter);
bool District_StartFreeMovement(ProcPtr proc);

/* World map, while idle: a pennant over each shown node with an unlocked district (DistrictMarker.c) */
void District_DrawWorldMapMarkers(void);
int District_TimeOfDay(void);
void District_TintMapPalettes(void);
void District_PlaceNpcs(void);
