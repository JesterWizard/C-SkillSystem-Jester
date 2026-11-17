#include "common-chax.h"
#include "kernel-lib.h"
#include "big-unit.h"
#include "skill-system.h"
#include "status-getter.h"
#include "weapon-range.h"
#include "map-movement.h"
#include "threat-arrows.h"
#include "constants/skills.h"

/*
 * Threat arrows: while the player picks where to move a unit, every enemy that could attack a unit on the cursor's
 * tile in the next enemy phase shows a flashing red arrow from its tile to the cursor's.
 *
 * DrawUpdatedPathArrow (RemoveMovePath.c) runs each frame of a move selection; it calls ThreatArrows_OnMoveSelect,
 * which starts ProcScr_ThreatArrows as a child of the player phase proc and marks it seen. The proc ends on the first
 * frame the move selection is not running (A, B, the stat screen) and puts the tiles and colours back.
 *
 * Once, when the move selection starts, each enemy's attack range is worked out the way the game shows it when the
 * enemy is selected (DisplayUnitEffectRange: movement map, 2x2 blocks, seal map, complete attack range), and each tile
 * of the selected unit's move range gets a bit per enemy that reaches it. A cursor move only looks its mask up.
 *
 * The stat getters and the flood are slow (an enemy's MovGetter alone is about a quarter of a frame on a busy map), so
 * the work is one step a frame: an enemy's stats, its movement flood, its attack range. The arrows show once all
 * enemies are done.
 */

enum {
	/* OBJ tiles that the system sheet (LoadObjUIGfx) leaves blank and nothing else uses on the map */
	THREAT_CHR_DOT = 0x70,
	THREAT_CHR_HEAD_E = 0x71,
	THREAT_CHR_HEAD_S = 0x7A,
	THREAT_CHR_HEAD_SE = 0x7B,

	/* OBJ palette 6 (vanilla's first face palette): no portrait is up during a move selection */
	THREAT_OBJ_PAL = 6,

	THREAT_ARROWS_MAX = 8,
	THREAT_SPRITES_MAX = 56,
	THREAT_DOT_SPACING = 7,
	/* Pushed after the cursor (layer 4), the moving unit and the move path (11), before the map sprites */
	THREAT_SPRITE_LAYER = 12,

	/* Dots start this far from the enemy's centre; the head's centre is this far from the cursor tile's centre */
	THREAT_START_GAP = 9,
	THREAT_HEAD_GAP = 10,
	THREAT_HEAD_CLEAR = 7,

	/*
	 * The enemies' maps, borrowed from gGenericBuffer within one step: 0 .. 0x800 is the skill list's scratch
	 * (GenerateSkillListExt), so the range map goes where the movement flood keeps its pioneer map (0x800).
	 */
	THREAT_ALT_RANGE = 0x800,
	THREAT_ALT_MOVE = 0x1000,
	THREAT_ALT_OTHER = 0x1800,
	THREAT_ALT_SIZE = 0x800,

	/*
	 * Another step starts only while the scanlines used since the frame started (the main loop starts at VBlank, line
	 * 160) are under this. A step is at most about 140 lines on a busy map and the rest of the frame needs about 140,
	 * so a frame of the one-time work takes at most two VBlanks: a tap of a key (two VBlanks) is still read.
	 */
	THREAT_LINES_BUDGET = 100,
};

enum {
	THREAT_SEAL,		/* once: whether any unit's Physical Seal marks the map */
	THREAT_STATS,		/* an enemy: whether it can fight, and could reach the move range at all */
	THREAT_FLOOD,		/* its movement map, kept as a bitmap */
	THREAT_RANGE,		/* its attack range, OR'd into the masks */
	THREAT_READY,
};

/* The proc's own fields start at 0x29; it must fit in 0x6C bytes */
struct ThreatArrowsProc {
	PROC_HEADER;

	u8 seen;
	u8 state;
	u8 next;
	u8 count;
	u8 by;
	u8 bh;
	u8 mov;
	u8 sealed;
	s8 curX;
	s8 curY;
	u16 savedPal[2];
	u8 uids[THREAT_ENEMY_MAX];
	u32 mask;
};

_Static_assert(sizeof(struct ThreatArrowsProc) <= 0x6C, "ThreatArrowsProc fits in a proc");
_Static_assert(sizeof(struct ThreatArrowsData) == 0x780, "gThreatArrows is 0x780 bytes (include/link/config-memmap.s)");

/* What a step borrows: the maps and flood state that the move selection needs back exactly as they were */
struct ThreatArrowsBorrow {
	struct Unit *activeUnit;
	u8 **moveMap, **rangeMap, **otherMap, **workingMap, **barrierMap, **pioneerMap;
	u32 moveFlags;
	struct MovMapFillState fillState;
	u8 origin;
	u8 workingCosts[TERRAIN_COUNT];
};

static const u32 sThreatTiles[4][8] = {
	/* dot */
	{ 0x00000000, 0x00000000, 0x00022000, 0x00211200, 0x00211200, 0x00022000, 0x00000000, 0x00000000 },
	/* head pointing right (left with HFLIP) */
	{ 0x00000000, 0x00000200, 0x00022100, 0x02211100, 0x02211100, 0x00022100, 0x00000200, 0x00000000 },
	/* head pointing down (up with VFLIP) */
	{ 0x00000000, 0x00000000, 0x02111120, 0x00211200, 0x00211200, 0x00022000, 0x00022000, 0x00000000 },
	/* head pointing down-right (the other diagonals with flips) */
	{ 0x00000000, 0x02000000, 0x02200000, 0x02120000, 0x02112000, 0x02111200, 0x02222220, 0x00000000 },
};

static const u8 sThreatTileChr[4] = {
	THREAT_CHR_DOT, THREAT_CHR_HEAD_E, THREAT_CHR_HEAD_S, THREAT_CHR_HEAD_SE,
};

/* Global (not static) so that gbarun's prof can time them by name (Tools/cloud/README.md) */
void ThreatArrows_Init(struct ThreatArrowsProc *proc);
void ThreatArrows_Loop(struct ThreatArrowsProc *proc);
bool ThreatArrows_Step(struct ThreatArrowsProc *proc);
static void ThreatArrows_OnEnd(struct ThreatArrowsProc *proc);

const struct ProcCmd ProcScr_ThreatArrows[] = {
	PROC_NAME("ThreatArrows"),
	PROC_SET_END_CB(ThreatArrows_OnEnd),
	PROC_CALL(ThreatArrows_Init),
	PROC_REPEAT(ThreatArrows_Loop),
	PROC_END,
};

static bool ThreatArrows_Enabled(void)
{
	return gpKernelDesignerConfig->threat_arrows && gPlaySt.config.threat_arrows_on;
}

static void ThreatArrows_PutTiles(bool clear)
{
	int i;

	for (i = 0; i < 4; i++) {
		u32 *dst = (u32 *)(OBJ_VRAM0 + sThreatTileChr[i] * 0x20);

		CpuFill32(0, dst, 0x20);
		if (!clear)
			CpuCopy32(sThreatTiles[i], dst, 0x20);
	}
}

/* Row y of the selected unit's move range: its first tile in *x0 and the span to its last, 0 if none */
static int ThreatArrows_RowSpan(int y, int *x0)
{
	int x, lo = -1, hi = -1;

	for (x = 0; x < gBmMapSize.x; x++) {
		if (gBmMapMovement[y][x] > MAP_MOVEMENT_MAX)
			continue;

		if (lo < 0)
			lo = x;
		hi = x;
	}

	*x0 = lo;
	return lo < 0 ? 0 : hi - lo + 1;
}

/* The rows of the move range, outward from the unit's own: at most THREAT_ROW_MAX rows and THREAT_TILE_MAX tiles */
static void ThreatArrows_BuildRows(struct ThreatArrowsProc *proc)
{
	int uy = gActiveUnit->yPos;
	int lo = uy, hi = uy, total, len, x0, k, y, r, off;
	bool up = true, down = true;

	total = ThreatArrows_RowSpan(uy, &x0);

	for (k = 1; (up || down) && (hi - lo + 1) < THREAT_ROW_MAX; k++) {
		if (up) {
			y = uy - k;
			len = (y >= 0) ? ThreatArrows_RowSpan(y, &x0) : 0;

			if (len == 0 || total + len > THREAT_TILE_MAX) {
				up = false;
			} else {
				lo = y;
				total += len;
			}
		}

		if (down && (hi - lo + 1) < THREAT_ROW_MAX) {
			y = uy + k;
			len = (y < gBmMapSize.y) ? ThreatArrows_RowSpan(y, &x0) : 0;

			if (len == 0 || total + len > THREAT_TILE_MAX) {
				down = false;
			} else {
				hi = y;
				total += len;
			}
		}
	}

	proc->by = lo;
	proc->bh = hi - lo + 1;

	off = 0;
	for (r = 0; r < proc->bh; r++) {
		len = ThreatArrows_RowSpan(lo + r, &x0);
		gThreatArrows.rowX0[r] = len ? x0 : 0;
		gThreatArrows.rowLen[r] = len;
		gThreatArrows.rowOff[r] = off;
		off += len;
	}

	CpuFill32(0, gThreatArrows.masks, sizeof(gThreatArrows.masks));
}

/* Whether the enemy counts: on the map and shown, by the danger zone's rules (GenerateDangerZoneRange). Cheap. */
static bool ThreatArrows_IsShownEnemy(struct Unit *unit)
{
	struct BigUnitGroup *group;
	int corner, c;

	if (!UNIT_IS_VALID(unit))
		return false;

	if (unit->state & (US_HIDDEN | US_UNAVAILABLE | US_RESCUED | US_UNDER_A_ROOF))
		return false;

	if (unit->xPos < 0 || unit->yPos < 0 || unit->xPos >= gBmMapSize.x || unit->yPos >= gBmMapSize.y)
		return false;

	if (AreUnitsAllied(gActiveUnit->index, unit->index))
		return false;

	if (gPlaySt.chapterVisionRange && gBmMapFog[unit->yPos][unit->xPos] == 0)
		return false;

	/* A 2x2 unit is counted once, from its first standing corner: its expanded range is the block's */
	group = BigUnit_GetGroup(unit, &corner);
	if (group) {
		for (c = 0; c < corner; c++)
			if (BigUnit_GetCorner(group, c))
				return false;
	}

	return true;
}

/* The shown enemies, nearest to the selected unit first; the nearest THREAT_ENEMY_MAX are kept */
static void ThreatArrows_PickEnemies(struct ThreatArrowsProc *proc)
{
	u8 dist[THREAT_ENEMY_MAX];
	int i, j, d;
	struct Unit *unit;

	proc->count = 0;

	for (i = FACTION_GREEN + 1; i < 0x100; i++) {
		unit = GetUnit(i);

		if (!ThreatArrows_IsShownEnemy(unit))
			continue;

		d = RECT_DISTANCE(unit->xPos, unit->yPos, gActiveUnit->xPos, gActiveUnit->yPos);

		for (j = proc->count; j > 0 && dist[j - 1] > d; j--) {
			if (j < THREAT_ENEMY_MAX) {
				dist[j] = dist[j - 1];
				proc->uids[j] = proc->uids[j - 1];
			}
		}

		if (j < THREAT_ENEMY_MAX) {
			dist[j] = d;
			proc->uids[j] = i;
			if (proc->count < THREAT_ENEMY_MAX)
				proc->count++;
		}
	}
}

void ThreatArrows_Init(struct ThreatArrowsProc *proc)
{
	u16 *pal = gPaletteBuffer + (0x10 + THREAT_OBJ_PAL) * 0x10;

	proc->seen = 1;
	proc->state = THREAT_SEAL;
	proc->next = 0;
	proc->sealed = false;
	proc->curX = -1;
	proc->curY = -1;
	proc->mask = 0;

	proc->savedPal[0] = pal[1];
	proc->savedPal[1] = pal[2];

	ThreatArrows_PutTiles(false);
	ThreatArrows_BuildRows(proc);
	ThreatArrows_PickEnemies(proc);

	/* A map larger than the borrowed space gets no arrows */
	if (proc->count == 0 || (gBmMapSize.y + 4) * (gBmMapSize.x + 2 + 4) > THREAT_ALT_SIZE)
		proc->state = THREAT_READY;
}

static void ThreatArrows_OnEnd(struct ThreatArrowsProc *proc)
{
	u16 *pal = gPaletteBuffer + (0x10 + THREAT_OBJ_PAL) * 0x10;

	ThreatArrows_PutTiles(true);

	pal[1] = proc->savedPal[0];
	pal[2] = proc->savedPal[1];
	EnablePaletteSync();
}

/* A queued VBlank copy (RegisterDataMove) out of gGenericBuffer: its data must not be overwritten this frame */
static bool ThreatArrows_GenericBufferQueued(void)
{
	int i;

	for (i = 0; i < gFrameTmRegisterConfig.count && i < 32; i++) {
		u32 src = (u32)gFrameTmRegister[i].src;

		if (src >= (u32)gGenericBuffer && src < (u32)gGenericBuffer + sizeof(gGenericBuffer))
			return true;
	}
	return false;
}

/*
 * Borrow the maps for a step: gBmMapMovement and gBmMapOther point at maps in gGenericBuffer, and the selected unit
 * leaves its tile, so it does not block the enemies. ThreatArrows_Return puts everything back.
 */
static void ThreatArrows_Borrow(struct ThreatArrowsBorrow *b)
{
	u8 **map;

	b->activeUnit = gActiveUnit;
	b->moveMap = gBmMapMovement;
	b->rangeMap = gBmMapRange;
	b->otherMap = gBmMapOther;
	b->workingMap = gWorkingBmMap;
	b->barrierMap = KernelExtMoveBarrierMap;
	b->pioneerMap = KernelExtMovePioneerMap;
	b->moveFlags = KernelMoveMapFlags;
	memcpy(&b->fillState, &gMovMapFillState, sizeof(b->fillState));
	memcpy(b->workingCosts, gWorkingTerrainMoveCosts, TERRAIN_COUNT);

	b->origin = gBmMapUnit[gActiveUnit->yPos][gActiveUnit->xPos];
	gBmMapUnit[gActiveUnit->yPos][gActiveUnit->xPos] = 0;

	BmMapInit(gGenericBuffer + THREAT_ALT_MOVE, &map, gBmMapSize.x, gBmMapSize.y);
	gBmMapMovement = map;
	BmMapInit(gGenericBuffer + THREAT_ALT_OTHER, &map, gBmMapSize.x, gBmMapSize.y);
	gBmMapOther = map;
	BmMapFill(map, 0);
}

static void ThreatArrows_Return(struct ThreatArrowsBorrow *b)
{
	gActiveUnit = b->activeUnit;
	gBmMapUnit[gActiveUnit->yPos][gActiveUnit->xPos] = b->origin;

	gBmMapMovement = b->moveMap;
	gBmMapRange = b->rangeMap;
	gBmMapOther = b->otherMap;
	gWorkingBmMap = b->workingMap;
	KernelExtMoveBarrierMap = b->barrierMap;
	KernelExtMovePioneerMap = b->pioneerMap;
	KernelMoveMapFlags = b->moveFlags;
	memcpy(&gMovMapFillState, &b->fillState, sizeof(b->fillState));
	memcpy(gWorkingTerrainMoveCosts, b->workingCosts, TERRAIN_COUNT);
}

/* The seal map that DisplayUnitEffectRange gives an enemy (Physical Seal), into the borrowed gBmMapOther */
static void ThreatArrows_SealMap(void)
{
#if defined(SID_PhysicalSeal) && (COMMON_SKILL_VALID(SID_PhysicalSeal))
	GeneratePhysicalSealMap(1);
#endif
}

static bool ThreatArrows_StepSeal(void)
{
	int x, y;

	ThreatArrows_SealMap();

	for (y = 0; y < gBmMapSize.y; y++)
		for (x = 0; x < gBmMapSize.x; x++)
			if (gBmMapOther[y][x])
				return true;

	return false;
}

/* Whether the enemy can attack at all (DisplayUnitEffectRange's weapon range), and could reach the move range */
static bool ThreatArrows_StepStats(struct ThreatArrowsProc *proc, struct Unit *unit)
{
	int r, x0, x1, bx0 = 0, bx1 = -1, by0 = proc->by, by1 = proc->by + proc->bh - 1;
	int dx, dy, max, mov;

	for (r = 0; r < proc->bh; r++) {
		if (!gThreatArrows.rowLen[r])
			continue;

		x0 = gThreatArrows.rowX0[r];
		x1 = x0 + gThreatArrows.rowLen[r] - 1;
		if (bx1 < 0 || x0 < bx0)
			bx0 = x0;
		if (x1 > bx1)
			bx1 = x1;
	}

	dx = unit->xPos < bx0 ? bx0 - unit->xPos : (unit->xPos > bx1 ? unit->xPos - bx1 : 0);
	dy = unit->yPos < by0 ? by0 - unit->yPos : (unit->yPos > by1 ? unit->yPos - by1 : 0);

	/* Every step costs at least 1; 2 more for a 2x2 block's other corners */
	mov = MovGetter(unit);
	max = GetUnitMaxRange(unit);
	if (dx + dy > mov + (max > 1 ? max : 1) + 2)
		return false;

	proc->mov = mov;

	if (GetUnitWeaponUsabilityBits(unit) & UNIT_USEBIT_WEAPON)
		return true;

#if defined(SID_UnarmedCombat) && (COMMON_SKILL_VALID(SID_UnarmedCombat))
	if (SkillTester(unit, SID_UnarmedCombat) && !GetUnitEquippedWeapon(unit))
		return true;
#endif

	return false;
}

/* The enemy's movement map (the shown one: 2x2 blocks expanded), kept in gThreatArrows.reach as a bitmap */
static void ThreatArrows_StepFlood(struct ThreatArrowsProc *proc, struct Unit *unit)
{
	int x, y, i = 0;

	gActiveUnit = unit;
	GenerateUnitMovementMapExt(unit, proc->mov);
	BigUnit_ExpandMovementMap(unit);

	CpuFill32(0, gThreatArrows.reach, sizeof(gThreatArrows.reach));

	for (y = 0; y < gBmMapSize.y; y++) {
		for (x = 0; x < gBmMapSize.x; x++, i++) {
			if (gBmMapMovement[y][x] <= MAP_MOVEMENT_MAX)
				gThreatArrows.reach[i >> 3] |= 1 << (i & 7);
		}
	}
}

/* The enemy's complete attack range from its movement map, OR'd into the masks of the move range's tiles */
static void ThreatArrows_StepRange(struct ThreatArrowsProc *proc, struct Unit *unit, u8 **playerMove)
{
	u8 **range;
	u8 saved;
	int r, x, y, x0, len, i = 0;
	u32 bit = 1 << proc->next, *masks;

	for (y = 0; y < gBmMapSize.y; y++) {
		for (x = 0; x < gBmMapSize.x; x++, i++)
			gBmMapMovement[y][x] = (gThreatArrows.reach[i >> 3] & (1 << (i & 7))) ? 0 : 0xFF;
	}

	if (proc->sealed)
		ThreatArrows_SealMap();

	gActiveUnit = unit;
	saved = gBmMapUnit[unit->yPos][unit->xPos];
	gBmMapUnit[unit->yPos][unit->xPos] = 0;

	/* The flood's pioneer map was here; no flood runs now */
	BmMapInit(gGenericBuffer + THREAT_ALT_RANGE, &range, gBmMapSize.x, gBmMapSize.y);
	gBmMapRange = range;
	SetWorkingBmMap(range);
	GenerateUnitCompleteAttackRange(unit);

	gBmMapUnit[unit->yPos][unit->xPos] = saved;

	for (r = 0; r < proc->bh; r++) {
		y = proc->by + r;
		x0 = gThreatArrows.rowX0[r];
		len = gThreatArrows.rowLen[r];
		masks = gThreatArrows.masks + gThreatArrows.rowOff[r];

		/* Only the tiles the selected unit can stop on: in its range, with no other unit there */
		for (x = 0; x < len; x++) {
			if (range[y][x0 + x] && playerMove[y][x0 + x] <= MAP_MOVEMENT_MAX && !gBmMapUnit[y][x0 + x])
				masks[x] |= bit;
		}
	}
}

/* One step of the one-time work, at most about 0.4 frames on a busy map; false if it has to wait for a later frame */
NOINLINE bool ThreatArrows_Step(struct ThreatArrowsProc *proc)
{
	struct ThreatArrowsBorrow b;
	struct Unit *unit = GetUnit(proc->uids[proc->next]);
	bool done = false;

	if (proc->state == THREAT_STATS) {
		if (ThreatArrows_StepStats(proc, unit))
			proc->state = THREAT_FLOOD;
		else
			done = true;
	} else {
		if (ThreatArrows_GenericBufferQueued())
			return false;

		ThreatArrows_Borrow(&b);

		switch (proc->state) {
		case THREAT_SEAL:
			proc->sealed = ThreatArrows_StepSeal();
			proc->state = THREAT_STATS;
			break;

		case THREAT_FLOOD:
			ThreatArrows_StepFlood(proc, unit);
			proc->state = THREAT_RANGE;
			break;

		case THREAT_RANGE:
			ThreatArrows_StepRange(proc, unit, b.moveMap);
			proc->state = THREAT_STATS;
			done = true;
			break;
		}

		ThreatArrows_Return(&b);
	}

	if (done) {
		proc->next++;
		if (proc->next >= proc->count)
			proc->state = THREAT_READY;
	}
	return true;
}

/*
 * Scanlines since the VBlank (line 160 of 228) that started the frame of the first call, 228 for each VBlank since:
 * the game clock ticks at each VBlank. The first call sets *clock0 to the clock.
 */
static int ThreatArrows_Lines(u32 *clock0)
{
	u32 clock, v;

	do {
		clock = GetGameClock();
		v = REG_VCOUNT;
	} while (clock != GetGameClock());

	if (*clock0 == 0)
		*clock0 = clock;

	return (clock - *clock0) * 228 + (v >= 160 ? v - 160 : v + 228 - 160);
}

static u32 ThreatArrows_LookUp(struct ThreatArrowsProc *proc, int x, int y)
{
	int r = y - proc->by;
	int i;

	if (r < 0 || r >= proc->bh)
		return 0;

	i = x - gThreatArrows.rowX0[r];
	if (i < 0 || i >= gThreatArrows.rowLen[r])
		return 0;

	return gThreatArrows.masks[gThreatArrows.rowOff[r] + i];
}

static int ThreatArrows_Sqrt(u32 n)
{
	u32 res = 0, bit = 1 << 30;

	while (bit > n)
		bit >>= 2;

	while (bit) {
		if (n >= res + bit) {
			n -= res + bit;
			res = (res >> 1) + bit;
		} else {
			res >>= 1;
		}
		bit >>= 2;
	}
	return res;
}

static int ThreatArrows_Scale(int v, int num, int den)
{
	u32 q = k_udiv((u32)(v < 0 ? -v : v) * num, den);

	return v < 0 ? -(int)q : (int)q;
}

static void ThreatArrows_Put(int x, int y, int flip, int chr)
{
	if (x <= -8 || x >= DISPLAY_WIDTH || y <= -8 || y >= DISPLAY_HEIGHT)
		return;

	PutSprite(THREAT_SPRITE_LAYER,
		  OAM1_X(x & 0x1FF) | flip,
		  OAM0_Y(y & 0xFF),
		  gObject_8x8,
		  OAM2_PAL(THREAT_OBJ_PAL) + OAM2_LAYER(2) + OAM2_CHR(chr));
}

/* One arrow: dots from the enemy's centre, then a head pointing at the cursor tile's centre */
static void ThreatArrows_DrawArrow(int ex, int ey, int cx, int cy, int maxDots)
{
	int dx = cx - ex, dy = cy - ey;
	int ax = dx < 0 ? -dx : dx, ay = dy < 0 ? -dy : dy;
	int len = ThreatArrows_Sqrt(dx * dx + dy * dy);
	int ux, uy, seg, n, step, i, s, hx, hy, chr, flip;

	if (len == 0)
		return;

	/* The direction, in 1/256 of a pixel per pixel */
	ux = ThreatArrows_Scale(dx, 256, len);
	uy = ThreatArrows_Scale(dy, 256, len);

	seg = len - THREAT_START_GAP - THREAT_HEAD_GAP - THREAT_HEAD_CLEAR;
	if (seg >= 0) {
		n = k_udiv(seg, THREAT_DOT_SPACING) + 1;
		step = THREAT_DOT_SPACING << 8;

		if (n > maxDots) {
			n = maxDots;
			step = n > 1 ? (int)k_udiv(seg << 8, n - 1) : 0;
		}

		for (i = 0; i < n; i++) {
			s = (THREAT_START_GAP << 8) + step * i;
			ThreatArrows_Put(ex + ((ux * s) >> 16) - 4, ey + ((uy * s) >> 16) - 4, 0, THREAT_CHR_DOT);
		}
	}

	/* The head: straight when one axis is under 0.4 of the other, else diagonal */
	if (ay * 5 < ax * 2) {
		chr = THREAT_CHR_HEAD_E;
		flip = dx < 0 ? OAM1_HFLIP : 0;
	} else if (ax * 5 < ay * 2) {
		chr = THREAT_CHR_HEAD_S;
		flip = dy < 0 ? OAM1_VFLIP : 0;
	} else {
		chr = THREAT_CHR_HEAD_SE;
		flip = (dx < 0 ? OAM1_HFLIP : 0) | (dy < 0 ? OAM1_VFLIP : 0);
	}

	hx = cx - ((ux * THREAT_HEAD_GAP) >> 8);
	hy = cy - ((uy * THREAT_HEAD_GAP) >> 8);
	ThreatArrows_Put(hx - 4, hy - 4, flip, chr);
}

/* A slow pulse of the two reds (32 frames), a warning rather than a strobe */
static void ThreatArrows_Pulse(void)
{
	u16 *pal = gPaletteBuffer + (0x10 + THREAT_OBJ_PAL) * 0x10;
	int t = GetGameClock() & 31;
	int level = t < 16 ? t : 31 - t;

	pal[1] = RGB(17 + ((level * 14) >> 4), level >> 2, level >> 2);
	pal[2] = RGB(8 + (level >> 2), 0, 0);
	EnablePaletteSync();
}

static void ThreatArrows_Draw(struct ThreatArrowsProc *proc)
{
	int k, arrows = 0, maxDots, cx, cy;
	u32 mask = proc->mask;
	struct Unit *unit;

	if (!mask)
		return;

	for (k = 0; k < proc->count; k++)
		if (mask & (1 << k))
			arrows++;

	if (arrows > THREAT_ARROWS_MAX)
		arrows = THREAT_ARROWS_MAX;

	maxDots = k_udiv(THREAT_SPRITES_MAX - arrows, arrows);

	cx = proc->curX * 16 + 8 - gBmSt.camera.x;
	cy = proc->curY * 16 + 8 - gBmSt.camera.y;

	ThreatArrows_Pulse();

	/* Nearest enemies first (the bits are in that order) */
	for (k = 0; k < proc->count && arrows > 0; k++) {
		if (!(mask & (1 << k)))
			continue;

		unit = GetUnit(proc->uids[k]);
		ThreatArrows_DrawArrow(unit->xPos * 16 + 8 - gBmSt.camera.x, unit->yPos * 16 + 8 - gBmSt.camera.y,
				       cx, cy, maxDots);
		arrows--;
	}
}

void ThreatArrows_Loop(struct ThreatArrowsProc *proc)
{
	if (!proc->seen || !ThreatArrows_Enabled()) {
		Proc_End(proc);
		return;
	}
	proc->seen = 0;

	if (proc->state != THREAT_READY) {
		u32 clock0 = 0;

		ThreatArrows_Lines(&clock0);
		while (ThreatArrows_Step(proc) && proc->state != THREAT_READY) {
			if (ThreatArrows_Lines(&clock0) >= THREAT_LINES_BUDGET)
				break;
		}
		return;
	}

	if (gBmSt.playerCursor.x != proc->curX || gBmSt.playerCursor.y != proc->curY) {
		proc->curX = gBmSt.playerCursor.x;
		proc->curY = gBmSt.playerCursor.y;
		proc->mask = ThreatArrows_LookUp(proc, proc->curX, proc->curY);
	}

	ThreatArrows_Draw(proc);
}

void ThreatArrows_OnMoveSelect(void)
{
	struct ThreatArrowsProc *proc;
	ProcPtr parent;

	if (!ThreatArrows_Enabled())
		return;

	proc = Proc_Find(ProcScr_ThreatArrows);
	if (!proc) {
		parent = Proc_Find(gProcScr_PlayerPhase);
		if (!parent || !gActiveUnit)
			return;

		proc = Proc_Start(ProcScr_ThreatArrows, parent);
	}

	proc->seen = 1;
}
