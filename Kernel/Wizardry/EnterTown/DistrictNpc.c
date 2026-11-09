#include "common-chax.h"
#include "constants/terrains.h"
#include "district.h"
#include "skill-system.h"	/* MAPANIMFX_CHR_L / MAPANIMFX_PAL for the Hex Card icon */
#include "fishing.h"
#include "crossing.h"
#include "tower.h"

/* Free movement (Kernel/Wizardry/FreeMovement/cFreeMU/src/FreeMU.h, not on the include path) */
extern bool FMU_CanUnitBeOnPos(struct Unit *unit, s8 x, s8 y);
extern u8 GetFreeMovementState(void);

#define DISTRICT_NPC_MAX 8

/*
 * Green NPCs (units 0x41..0x7F) that wander inside their district.
 * Rows are { chapter, character id, wander radius, flags }.
 */
const struct DistrictNpc gDistrictNpcs[] = {
	/* Serafew (0x3B) */
	{ 0x3B, 0xFA, 2, DISTRICT_NPC_HEX },
	{ 0x3B, 0xFB, 2, DISTRICT_NPC_HEX },
	{ 0x3B, 0xFC, 2, 0 },
	{ 0x3B, 0xF9, 2, 0 },
	{ 0x3B, 0x7F, 3, 0 },

	/* Port Kiris (0x3C) */
	{ 0x3C, 0xFA, 2, DISTRICT_NPC_HEX },
	{ 0x3C, 0xFB, 2, DISTRICT_NPC_HEX },
	{ 0x3C, 0xFC, 2, 0 },
	{ 0x3C, 0xF9, 2, 0 },
	{ 0x3C, 0x7F, 3, 0 },

	/* Caer Pelyn (0x41) */
	{ 0x41, 0xFA, 2, DISTRICT_NPC_HEX },
	{ 0x41, 0xFB, 2, DISTRICT_NPC_HEX },
	{ 0x41, 0xFC, 2, 0 },
	{ 0x41, 0xF9, 2, 0 },
	{ 0x41, 0x7F, 3, 0 },

	/* Jehanna (0x42) */
	{ 0x42, 0xFA, 2, DISTRICT_NPC_HEX },
	{ 0x42, 0xFB, 2, DISTRICT_NPC_HEX },
	{ 0x42, 0xFC, 2, 0 },
	{ 0x42, 0xF9, 2, 0 },
	{ 0x42, 0x7F, 3, 0 },

	{ 0xFF, 0, 0, 0 },
};

const struct DistrictNpc *GetDistrictNpc(int chapter, int pid)
{
	const struct DistrictNpc *it;

	for (it = gDistrictNpcs; it->chapter != 0xFF; it++)
		if (it->chapter == chapter && it->pid == pid)
			return it;

	return NULL;
}

/*
 * Proc state. PROC_HEADER is 0x29 bytes; the whole struct is 0x64 bytes with
 * padding (budget 0x6C).
 */
struct DistrictNpcProc {
	PROC_HEADER;
	u8 count;
	u8 next;		/* index of the NPC that moves on the next tick */
	u16 timer;		/* frames until the next NPC move */
	u8 cooldown;		/* frames to wait after the player stops moving */
	s8 playerX;		/* last seen position of gActiveUnit */
	s8 playerY;
	u8 unitId[DISTRICT_NPC_MAX];
	s8 homeX[DISTRICT_NPC_MAX];
	s8 homeY[DISTRICT_NPC_MAX];
	u8 radius[DISTRICT_NPC_MAX];
	u8 hex[DISTRICT_NPC_MAX];	/* 1 if the row has DISTRICT_NPC_HEX */
	u8 iconsLoaded;			/* Hex Card icon graphics are in OBJ VRAM */
	u8 gatesLoaded;			/* Gate arrow graphics are in OBJ VRAM */
	u8 walkIdx;			/* NPC index of walkMu (only valid while walkMu is set) */
	struct MuProc *walkMu;		/* bare MU for the NPC that is walking, NULL when idle */
	u32 lastClock;			/* GetGameClock() at the last tick, to spot a gap (the map was suspended) */
};

static void DistrictNpc_Init(ProcPtr proc)
{
	struct DistrictNpcProc *p = proc;
	const struct DistrictNpc *row;
	struct Unit *unit;
	int chapter = gPlaySt.chapterIndex;
	int id;

	p->count = 0;
	p->next = 0;
	p->timer = 60;
	p->cooldown = 0;
	p->playerX = 0;
	p->playerY = 0;
	p->iconsLoaded = 0;
	p->gatesLoaded = 0;
	p->walkMu = NULL;
	p->walkIdx = 0;
	p->lastClock = GetGameClock();

	for (id = 0x41; id <= 0x7F && p->count < DISTRICT_NPC_MAX; id++) {
		unit = GetUnit(id);
		if (!unit || !unit->pCharacterData)
			continue;

		row = GetDistrictNpc(chapter, unit->pCharacterData->number);
		if (!row)
			continue;

		p->unitId[p->count] = id;
		p->homeX[p->count] = unit->xPos;
		p->homeY[p->count] = unit->yPos;
		p->radius[p->count] = row->radius;
		p->hex[p->count] = (row->flags & DISTRICT_NPC_HEX) ? 1 : 0;
		p->count++;
	}

	if (gActiveUnit) {
		p->playerX = gActiveUnit->xPos;
		p->playerY = gActiveUnit->yPos;
	}
}

static bool DistrictNpc_IsDoorTerrain(u8 terrain)
{
	return terrain == TERRAIN_VILLAGE_03 || terrain == TERRIAN_HOUSE ||
	       terrain == TERRAIN_ARMORY || terrain == TERRAIN_VENDOR ||
	       terrain == TERRAIN_ARENA_08;
}

/*
 * True while walkMu is a live MU proc. A MU that ends by itself frees its proc
 * slot, and DeleteProcessRecursive clears proc_script on that slot.
 */
static bool DistrictNpc_WalkMuLive(struct DistrictNpcProc *p)
{
	return p->walkMu && p->walkMu->proc_script == ProcScr_Mu;
}

/*
 * Gate exit arrows: two 8x8 4bpp chevrons, gold (palette index 5) with a dark
 * outline (4). Tile 0 points DOWN, tile 1 points RIGHT (the transpose of tile 0).
 * Each byte is two pixels, left pixel in the low nibble.
 *
 * Tile 0 (DOWN)        Tile 1 (RIGHT)
 *   454..4f4             44......
 *   415445f4             514.....
 *   .4555f4.             4554....
 *   ..45f4..             .4554...
 *   ...44...             .45f4...
 *   ........             45f4....
 *   ........             ff4.....
 *   ........             44......
 */
static const u8 sGateArrowTiles[64] = {
	0x54, 0x04, 0x40, 0x4F, 0x14, 0x45, 0x54, 0x4F,
	0x40, 0x55, 0xF5, 0x04, 0x00, 0x54, 0x4F, 0x00,
	0x00, 0x40, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x44, 0x00, 0x00, 0x00, 0x15, 0x04, 0x00, 0x00,
	0x54, 0x45, 0x00, 0x00, 0x40, 0x55, 0x04, 0x00,
	0x40, 0xF5, 0x04, 0x00, 0x54, 0x4F, 0x00, 0x00,
	0xFF, 0x04, 0x00, 0x00, 0x44, 0x00, 0x00, 0x00,
};

#define GATE_CHR_DOWN (MAPANIMFX_CHR_R)
#define GATE_CHR_RIGHT (MAPANIMFX_CHR_R + 1)
/* The fishing-spot bobber: the tile under the gate arrows in the 2D map OBJ layout (free in districts, as above) */
#define FISHING_CHR_MAP_BOBBER (MAPANIMFX_CHR_R + 0x20)
/* The River Crossing flag: the tile after the bobber */
#define CROSSING_CHR_MAP_FLAG (MAPANIMFX_CHR_R + 0x21)
/* The Tower Stack tower: the tile before the Hex Card icon (title-screen leftovers there, unused on the map) */
#define TOWER_CHR_MAP_TOWER (MAPANIMFX_CHR_L - 1)

/* District exit markers: two gold chevrons on each gate tile, pointing off the nearest map edge. */
static void DistrictNpc_LoadGates(void)
{
	CpuCopy16(sGateArrowTiles, OBJ_VRAM0 + GATE_CHR_DOWN * 0x20, sizeof(sGateArrowTiles));
#ifdef CONFIG_DISTRICT_FISHING
	Fishing_LoadMapMarker(FISHING_CHR_MAP_BOBBER);
#endif
#ifdef CONFIG_DISTRICT_CROSSING
	Crossing_LoadMapMarker(CROSSING_CHR_MAP_FLAG);
#endif
#ifdef CONFIG_DISTRICT_TOWER
	Tower_LoadMapMarker(TOWER_CHR_MAP_TOWER);
#endif
	LoadIconPalette(0, 0x10 + MAPANIMFX_PAL);
}

static void DistrictNpc_DrawGates(void)
{
	struct Trap *trap;
	int slide = (GetGameClock() >> 3) & 3;
	int x, y, chr, flip, dx, dy, i, off;

	for (trap = GetTrap(0); trap->type != TRAP_NONE; trap++) {
		if (trap->type != TRAP_DISTRICT_GATE)
			continue;

		x = trap->xPos * 16 - gBmSt.camera.x;
		y = trap->yPos * 16 - gBmSt.camera.y;
		if (x <= -16 || x >= 240 || y <= -16 || y >= 160)
			continue;

		if (trap->yPos == 0) {
			chr = GATE_CHR_DOWN;
			flip = OAM1_VFLIP;
			dx = 0;
			dy = -1;
		} else if (trap->xPos == 0) {
			chr = GATE_CHR_RIGHT;
			flip = OAM1_HFLIP;
			dx = -1;
			dy = 0;
		} else if (trap->xPos == gBmMapSize.x - 1) {
			chr = GATE_CHR_RIGHT;
			flip = 0;
			dx = 1;
			dy = 0;
		} else {
			chr = GATE_CHR_DOWN;
			flip = 0;
			dx = 0;
			dy = 1;
		}

		for (i = 0; i < 2; i++) {
			off = i * 5 + slide - 4;
			PutSprite(4,
				  OAM1_X((x + 4 + dx * off) & 0x1FF) | flip,
				  OAM0_Y((y + 4 + dy * off) & 0xFF),
				  gObject_8x8,
				  OAM2_PAL(MAPANIMFX_PAL) + OAM2_LAYER(0b10) + OAM2_CHR(chr));
		}
	}
}

#ifdef CONFIG_HEX_CARDS
/*
 * Hex Card icon floating over each Hex player. The OBJ chr and palette slots
 * are the skill map animation ones (MAPANIMFX_*). Those animations never run
 * in districts, so the slots are free here. Events overwrite them, so
 * iconsLoaded is cleared while one runs and the icon is reloaded afterwards.
 */
static void DistrictNpc_DrawIcons(struct DistrictNpcProc *p)
{
	struct Unit *unit;
	int i, bob, px, py, sx, sy, walking;

	if (!p->iconsLoaded) {
		/* Item icon palette 0, as in AchievementsMenu.c */
		LoadIconObjectGraphics(GetItemIconId(CONFIG_ITEM_INDEX_HEX_CARD), MAPANIMFX_CHR_L);
		LoadIconPalette(0, 0x10 + MAPANIMFX_PAL);
		p->iconsLoaded = 1;
	}

	/* 1 px bob, flips every 16 frames */
	bob = (GetGameClock() >> 4) & 1;

	for (i = 0; i < p->count; i++) {
		if (!p->hex[i])
			continue;

		unit = GetUnit(p->unitId[i]);
		if (!unit || !unit->pCharacterData)
			continue;

		/* While this NPC walks, its standing sprite is hidden: allow that */
		walking = DistrictNpc_WalkMuLive(p) && p->walkIdx == i;
		if (unit->state & (US_DEAD | US_NOT_DEPLOYED))
			continue;
		if (!walking && (unit->state & US_HIDDEN))
			continue;

		/* While this NPC walks, its standing sprite is replaced by a MU: follow that */
		if (walking) {
			px = p->walkMu->x_q4 >> 4;
			py = p->walkMu->y_q4 >> 4;
		} else {
			px = unit->xPos * 16;
			py = unit->yPos * 16;
		}

		sx = px - gBmSt.camera.x;
		sy = py - gBmSt.camera.y - 14 - bob;

		if (sx <= -16 || sx >= 240 || sy <= -16 || sy >= 160)
			continue;

		PutSprite(
			4,
			OAM1_X(sx & 0x1FF),
			OAM0_Y(sy & 0xFF),
			gObject_16x16,
			OAM2_PAL(MAPANIMFX_PAL) + OAM2_LAYER(0b01) + OAM2_CHR(MAPANIMFX_CHR_L));
	}
}
#endif

/* Walk MU for each direction: one step, then halt. Rest is zero, past HALT. */
static const u8 sNpcWalkLeft[MOVE_CMD_MAX_COUNT] = { MOVE_CMD_MOVE_LEFT, MOVE_CMD_HALT };
static const u8 sNpcWalkRight[MOVE_CMD_MAX_COUNT] = { MOVE_CMD_MOVE_RIGHT, MOVE_CMD_HALT };
static const u8 sNpcWalkUp[MOVE_CMD_MAX_COUNT] = { MOVE_CMD_MOVE_UP, MOVE_CMD_HALT };
static const u8 sNpcWalkDown[MOVE_CMD_MAX_COUNT] = { MOVE_CMD_MOVE_DOWN, MOVE_CMD_HALT };

/*
 * Ends the walk and puts the NPC's standing sprite back at its tile. A MU that
 * already ended on its own has a freed slot, so it is not passed to EndMu.
 */
static void DistrictNpc_FinishWalk(struct DistrictNpcProc *p)
{
	struct Unit *unit;

	if (!p->walkMu)
		return;

	if (DistrictNpc_WalkMuLive(p))
		EndMu(p->walkMu);
	p->walkMu = NULL;

	unit = GetUnit(p->unitId[p->walkIdx]);
	if (unit && unit->pCharacterData) {
		ShowUnitSprite(unit);
		RefreshEntityBmMaps();
	}
}

static void DistrictNpc_Loop(ProcPtr proc)
{
	struct DistrictNpcProc *p = proc;
	struct Unit *unit;
	struct Trap *trap;
	struct MuProc *mu;
	const u8 *script;
	int idx, dir, nx, ny, homeX, homeY, radius;
	u32 clock;

	/*
	 * A skipped frame means the map was suspended (e.g. a Hex match), and OBJ
	 * VRAM and palettes may have been overwritten: reload the icons.
	 */
	clock = GetGameClock();
	if (clock != p->lastClock + 1) {
		p->iconsLoaded = 0;
		p->gatesLoaded = 0;
	}
	p->lastClock = clock;

	/*
	 * The walk ends when its MU proc is gone or stops moving, or when an event
	 * starts. The proc check comes first, so a freed slot is never read for state.
	 */
	if (p->walkMu && (EventEngineExists() || !DistrictNpc_WalkMuLive(p) ||
			  p->walkMu->state != MU_STATE_MOVEMENT))
		DistrictNpc_FinishWalk(p);

	/* Only while free movement is on and no event is running */
	if (!GetFreeMovementState() || EventEngineExists()) {
		p->iconsLoaded = 0;
		p->gatesLoaded = 0;
		return;
	}

	if (!p->gatesLoaded) {
		DistrictNpc_LoadGates();
		p->gatesLoaded = 1;
	}
	DistrictNpc_DrawGates();
#ifdef CONFIG_DISTRICT_FISHING
	Fishing_DrawMapMarkers(FISHING_CHR_MAP_BOBBER, MAPANIMFX_PAL);
#endif
#ifdef CONFIG_DISTRICT_CROSSING
	Crossing_DrawMapMarkers(CROSSING_CHR_MAP_FLAG, MAPANIMFX_PAL);
#endif
#ifdef CONFIG_DISTRICT_TOWER
	Tower_DrawMapMarkers(TOWER_CHR_MAP_TOWER, MAPANIMFX_PAL);
#endif

#ifdef CONFIG_HEX_CARDS
	/* Drawn every frame, also on the ticks below that return early */
	DistrictNpc_DrawIcons(p);
#endif

	/* One walk at a time */
	if (p->walkMu)
		return;

	/* The player is moving: wait for them to stop */
	if (gActiveUnit && (gActiveUnit->xPos != p->playerX || gActiveUnit->yPos != p->playerY)) {
		p->playerX = gActiveUnit->xPos;
		p->playerY = gActiveUnit->yPos;
		p->cooldown = 30;
		return;
	}
	if (p->cooldown > 0) {
		p->cooldown--;
		return;
	}

	if (gKeyStatusPtr->heldKeys & DPAD_ANY)
		return;
	if (MuExists())
		return;

	if (p->timer > 0) {
		p->timer--;
		return;
	}
	p->timer = 20 + NextRN_N(30);

	if (p->count == 0)
		return;

	idx = p->next;
	p->next++;
	if (p->next >= p->count)
		p->next = 0;

	/* Stands still, or idles this turn */
	if (p->radius[idx] == 0 || NextRN_N(3) == 0)
		return;

	unit = GetUnit(p->unitId[idx]);
	if (!unit || !unit->pCharacterData)
		return;
	if (unit->state & (US_DEAD | US_NOT_DEPLOYED | US_HIDDEN))
		return;

	dir = NextRN_N(4);
	nx = unit->xPos;
	ny = unit->yPos;
	if (dir == 0) {
		nx--;
		script = sNpcWalkLeft;
	} else if (dir == 1) {
		nx++;
		script = sNpcWalkRight;
	} else if (dir == 2) {
		ny--;
		script = sNpcWalkUp;
	} else {
		ny++;
		script = sNpcWalkDown;
	}

	/* Stay inside the wander box (compares only, no abs or %) */
	homeX = p->homeX[idx];
	homeY = p->homeY[idx];
	radius = p->radius[idx];
	if (nx < homeX - radius || nx > homeX + radius)
		return;
	if (ny < homeY - radius || ny > homeY + radius)
		return;

	if (!FMU_CanUnitBeOnPos(unit, nx, ny))
		return;

	/* Keep off traps (the district gate) and the door tiles */
	trap = GetTrapAt(nx, ny);
	if (trap && trap->type != TRAP_NONE)
		return;
	if (DistrictNpc_IsDoorTerrain(gBmMapTerrain[ny][nx]))
		return;

	/* Bare MU, no camera: start it on the old tile, then move the logical position */
	mu = StartMu(unit);
	if (!mu)
		return;
	DisableMuCamera(mu);
	SetMuMoveScript(mu, script);

	unit->xPos = nx;
	unit->yPos = ny;
	HideUnitSprite(unit);
	RefreshEntityBmMaps();

	p->walkMu = mu;
	p->walkIdx = idx;
}

const struct ProcCmd ProcScr_DistrictNpc[] = {
	PROC_NAME("DistrictNpc"),
	PROC_CALL(DistrictNpc_Init),
	PROC_REPEAT(DistrictNpc_Loop),
	PROC_END,
};

/*
 * A walking NPC is a MU, which keeps drawing itself from OBJ VRAM. The NPC proc is not run while an event is, so a
 * minigame screen that takes over OBJ VRAM (fishing, River Crossing, Tower Stack) ends the walk first, or a garbled MU
 * shows.
 */
void DistrictNpc_EndWalk(void)
{
	struct DistrictNpcProc *p = Proc_Find(ProcScr_DistrictNpc);

	if (p)
		DistrictNpc_FinishWalk(p);
}

/* ASMC from the district opening script. Does nothing if already running. */
void StartDistrictNpcs(void)
{
	ProcPtr parent;

	if (Proc_Find(ProcScr_DistrictNpc))
		return;

	parent = Proc_Find(gProc_BMapMain);
	if (!parent)
		parent = PROC_TREE_3;

	Proc_Start(ProcScr_DistrictNpc, parent);
}
