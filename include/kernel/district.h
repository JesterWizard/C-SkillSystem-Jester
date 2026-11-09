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
