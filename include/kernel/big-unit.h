#pragma once

#include "common-chax.h"

struct MuProc;

/*
 * 2x2 units (Kernel/Wizardry/BigUnits, Documentation/Features/BigUnits.md): an enemy, NPC or fourth-allegiance unit
 * whose character or class is in gBigUnitCharacters or gBigUnitClasses loads as four units, one per tile of a 2x2 block. Each corner has its own HP and
 * is attacked on its own; a corner at 0 HP breaks and its tile empties, and the unit is gone when all four are.
 * The block is drawn once, the map sprite at twice its size, and acts once per phase.
 */

enum {
	BIG_CORNER_TL,
	BIG_CORNER_TR,
	BIG_CORNER_BL,
	BIG_CORNER_BR,
	BIG_CORNERS,
};

#define BIG_UNIT_GROUPS 8

/* A 2x2 unit: its corners' unit ids (0 once broken) and its top-left tile. Saved in the suspend save. */
struct BigUnitGroup {
	u8 uid[BIG_CORNERS];
	u8 x, y;
};

extern struct BigUnitGroup gBigUnitGroups[BIG_UNIT_GROUPS];

/* Designer tables (Data/BigUnits/BigUnits.c), each ending with 0 */
extern const u8 gBigUnitCharacters[];
extern const u8 gBigUnitClasses[];

extern const s8 gBigUnitCornerOffsets[BIG_CORNERS][2];

/* The unit's group, and in *corner which corner it is; NULL for a 1x1 unit */
struct BigUnitGroup *BigUnit_GetGroup(struct Unit *unit, int *corner);
bool BigUnit_IsBig(struct Unit *unit);
struct Unit *BigUnit_GetCorner(struct BigUnitGroup *group, int corner);

/* The end of LoadUnit: a listed unit gets its three other corners */
void BigUnit_OnLoadUnit(struct Unit *unit);

/*
 * Each frame on the map (PutUnitSpritesOam) and before each AI decision: the corner that moved takes the others along,
 * and once the active corner has acted the others are grayed too.
 */
void BigUnit_SyncAll(void);

/*
 * The AI (BigUnit.c, from CpDecide_Main): a 2x2 unit acts from its best placed corner, deciding with the other corners
 * off the unit map. The block's footprint in a corner's movement map.
 */
void BigUnit_AiDecide(void);
void BigUnit_MarkFootprintBarriers(struct Unit *unit);

/* The range shown for a selected 2x2 enemy: every tile the block can cover (gBmMapMovement, display only) */
void BigUnit_ExpandMovementMap(struct Unit *unit);

/*
 * Sprites (BigUnitDraw.c). RefreshUnitSprites tags each corner's sprite handle (SMSHandle._u0A, whose bit 0 is the
 * mirror flag): PutUnitSpritesOam skips the corners' own sprites and draws the block once, at its shown corner.
 */
#define BIG_SMS_CORNER 0x02
#define BIG_SMS_SHOWN 0x04
#define BIG_SMS_GROUP(tag) ((tag) >> 4)

int BigUnit_SmsTag(struct Unit *unit);
bool BigUnit_HandleShown(struct SMSHandle *handle);
void BigUnit_PutBlockSprite(struct SMSHandle *handle);

/* A corner's MU (its walk) drawn as the whole block at twice its size; false for a 1x1 unit */
bool BigUnit_PutMu(struct MuProc *proc, int x, int y, int oam0);

/* Defeat quotes and flags wait for a 2x2 unit's last corner */
bool BigUnit_PidStillStanding(int pid);

/* Hooks */
void BigUnit_OnClearUnit(struct Unit *unit);
void BigUnit_OnUnitKill(struct Unit *unit);
void BigUnit_ChapterEnd(void);
void BigUnit_Save(u8 *dst, const u32 size);
void BigUnit_Load(u8 *src, const u32 size);
