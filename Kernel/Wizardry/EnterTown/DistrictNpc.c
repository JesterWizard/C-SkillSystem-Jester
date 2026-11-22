#include "common-chax.h"
#include "constants/terrains.h"
#include "district.h"
#include "skill-system.h"	/* MAPANIMFX_PAL for the Hex Card icon and the gate arrows */
#include "fishing.h"
#include "crossing.h"
#include "tower.h"
#include "guild.h"

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
 * Each NPC takes a step about once a second on its own timer, whatever the player does: the player's free movement
 * only waits for its own MU (FreeMU_Core.c), and an NPC never steps next to the player. A walk is a bare MU, and
 * vanilla has 4 MU slots, so at most DISTRICT_NPC_WALKS walk at once and the player always has one.
 */
#define DISTRICT_NPC_WALKS 2
#define DISTRICT_NPC_STEP_FRAMES 60

/* Proc state. PROC_HEADER is 0x29 bytes; the whole struct must fit in 0x6C. */
struct DistrictNpcProc {
	PROC_HEADER;
	u8 count;
	u8 unitId[DISTRICT_NPC_MAX];
	s8 homeX[DISTRICT_NPC_MAX];
	s8 homeY[DISTRICT_NPC_MAX];
	u8 radius[DISTRICT_NPC_MAX];
	u8 timer[DISTRICT_NPC_MAX];	/* frames until this NPC's next step */
	u8 hexMask;			/* bit i: row i has DISTRICT_NPC_HEX */
	u8 iconsLoaded;			/* Hex Card icon graphics are in OBJ VRAM */
	u8 gatesLoaded;			/* Gate arrow graphics are in OBJ VRAM */
	u8 walkIdx[DISTRICT_NPC_WALKS];	/* NPC index of each walk (only valid while its walkMu is set) */
	struct MuProc *walkMu[DISTRICT_NPC_WALKS];	/* bare MU of each walking NPC, NULL when free */
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
	p->hexMask = 0;
	p->iconsLoaded = 0;
	p->gatesLoaded = 0;
	for (id = 0; id < DISTRICT_NPC_WALKS; id++) {
		p->walkMu[id] = NULL;
		p->walkIdx[id] = 0;
	}
	p->lastClock = GetGameClock();

	/* This story chapter's places (District life); the NPCs then wander around them */
	District_PlaceNpcs();

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
		/* Spread out: each starts its one-a-second steps at its own moment */
		p->timer[p->count] = 20 + NextRN_N(DISTRICT_NPC_STEP_FRAMES);
		if (row->flags & DISTRICT_NPC_HEX)
			p->hexMask |= 1 << p->count;
		p->count++;
	}
}

static bool DistrictNpc_IsDoorTerrain(u8 terrain)
{
	return terrain == TERRAIN_VILLAGE_03 || terrain == TERRIAN_HOUSE ||
	       terrain == TERRAIN_ARMORY || terrain == TERRAIN_VENDOR ||
	       terrain == TERRAIN_ARENA_08;
}

/*
 * True while walk w's MU is a live MU proc of its NPC. A MU that ends by itself frees its proc slot, and
 * DeleteProcessRecursive clears proc_script on that slot; a new MU (the player's) can take the slot, hence the unit.
 */
static bool DistrictNpc_WalkMuLive(struct DistrictNpcProc *p, int w)
{
	struct MuProc *mu = p->walkMu[w];

	return mu && mu->proc_script == ProcScr_Mu && mu->unit == GetUnit(p->unitId[p->walkIdx[w]]);
}

/* The walk NPC i is on, or -1 */
static int DistrictNpc_WalkOf(struct DistrictNpcProc *p, int i)
{
	int w;

	for (w = 0; w < DISTRICT_NPC_WALKS; w++)
		if (p->walkMu[w] && p->walkIdx[w] == i)
			return w;
	return -1;
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

/* In the district's own OBJ tiles (district.h): MAPANIMFX_CHR_R, row 12, is the walking units' frames */
#define GATE_CHR_DOWN (DISTRICT_GATE_CHR)
#define GATE_CHR_RIGHT (DISTRICT_GATE_CHR + 1)
/* District exit markers: two gold chevrons on each gate tile, pointing off the nearest map edge. */
static void DistrictNpc_LoadGates(void)
{
	int i;

	CpuCopy16(sGateArrowTiles, OBJ_VRAM0 + GATE_CHR_DOWN * 0x20, sizeof(sGateArrowTiles));
	LoadIconPalette(0, 0x10 + MAPANIMFX_PAL);

	/* The spots' icons: each 16x16 one's top row of tiles, then its bottom row 32 tiles on (2D OBJ mapping) */
	for (i = 0; i < DISTRICT_ICON_MAX; i++) {
		u8 *vram = OBJ_VRAM0 + DISTRICT_ICON_CHR_OF(i) * 0x20;

		CpuCopy16(gDistrictIconTiles[i][0], vram, 0x40);
		CpuCopy16(gDistrictIconTiles[i][2], vram + 0x20 * 0x20, 0x40);
	}
	ApplyPalette(gDistrictIconPal, 0x10 + DISTRICT_ICON_PAL);
}

/* A spot's icon floats over the tile (x, y), bobbing 1 pixel every 16 frames, under the units and the windows */
void District_PutSpotIcon(int x, int y, int icon)
{
	int sx = x * 16 - gBmSt.camera.x;
	int sy = y * 16 - gBmSt.camera.y - 12 - ((GetGameClock() >> 4) & 1);

	if (sx <= -16 || sx >= 240 || sy <= -16 || sy >= 160)
		return;

	PutSprite(4,
		  OAM1_X(sx & 0x1FF),
		  OAM0_Y(sy & 0xFF),
		  gObject_16x16,
		  OAM2_PAL(DISTRICT_ICON_PAL) + OAM2_LAYER(0b10) + OAM2_CHR(DISTRICT_ICON_CHR_OF(icon)));
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
 * Hex Card icon floating over each Hex player. Its tiles are the district's own
 * (DISTRICT_HEX_ICON_CHR, district.h: the skill map animations' MAPANIMFX_CHR_L
 * is in the walking units' frames, which a walk overwrote, so the icon blinked
 * out), its palette the skill map animations' (MAPANIMFX_PAL), unused in
 * districts. Events overwrite them, so iconsLoaded is cleared while one runs
 * and the icon is reloaded afterwards.
 */
static void DistrictNpc_DrawIcons(struct DistrictNpcProc *p)
{
	struct Unit *unit;
	int i, w, bob, px, py, sx, sy, walking;

	if (!p->iconsLoaded) {
		/* Item icon palette 0, as in AchievementsMenu.c */
		LoadIconObjectGraphics(GetItemIconId(CONFIG_ITEM_INDEX_HEX_CARD), DISTRICT_HEX_ICON_CHR);
		LoadIconPalette(0, 0x10 + MAPANIMFX_PAL);
		p->iconsLoaded = 1;
	}

	/* 1 px bob, flips every 16 frames */
	bob = (GetGameClock() >> 4) & 1;

	for (i = 0; i < p->count; i++) {
		if (!(p->hexMask & (1 << i)))
			continue;

		unit = GetUnit(p->unitId[i]);
		if (!unit || !unit->pCharacterData)
			continue;

		/* While this NPC walks, its standing sprite is hidden: allow that */
		w = DistrictNpc_WalkOf(p, i);
		walking = w >= 0 && DistrictNpc_WalkMuLive(p, w);
		if (unit->state & (US_DEAD | US_NOT_DEPLOYED))
			continue;
		if (!walking && (unit->state & US_HIDDEN))
			continue;

		/* While this NPC walks, its standing sprite is replaced by a MU: follow that */
		if (walking) {
			px = p->walkMu[w]->x_q4 >> 4;
			py = p->walkMu[w]->y_q4 >> 4;
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
			OAM2_PAL(MAPANIMFX_PAL) + OAM2_LAYER(0b01) + OAM2_CHR(DISTRICT_HEX_ICON_CHR));
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
static void DistrictNpc_FinishWalk(struct DistrictNpcProc *p, int w)
{
	struct Unit *unit;

	if (!p->walkMu[w])
		return;

	if (DistrictNpc_WalkMuLive(p, w))
		EndMu(p->walkMu[w]);
	p->walkMu[w] = NULL;

	unit = GetUnit(p->unitId[p->walkIdx[w]]);
	if (unit && unit->pCharacterData) {
		ShowUnitSprite(unit);
		RefreshEntityBmMaps();
		/* The standing sprite list keeps each sprite's tile: without this it came back on the tile it left */
		RefreshUnitSprites();
	}
}

/* NPC idx takes one step in a random free direction, as walk w. False if every way is shut. */
static bool DistrictNpc_Step(struct DistrictNpcProc *p, int idx, int w)
{
	struct Unit *unit = GetUnit(p->unitId[idx]);
	struct Trap *trap;
	struct MuProc *mu;
	const u8 *script;
	int dir, tries, nx, ny, dx, dy;

	if (!unit || !unit->pCharacterData)
		return false;
	if (unit->state & (US_DEAD | US_NOT_DEPLOYED | US_HIDDEN))
		return false;

	dir = NextRN_N(4);
	for (tries = 0; tries < 4; tries++, dir = (dir + 1) & 3) {
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
		if (nx < p->homeX[idx] - p->radius[idx] || nx > p->homeX[idx] + p->radius[idx])
			continue;
		if (ny < p->homeY[idx] - p->radius[idx] || ny > p->homeY[idx] + p->radius[idx])
			continue;

		/*
		 * Never next to the player: their next step goes to a tile next to them, and an NPC there would stop it
		 * (or walk into them while their position is still the tile they left)
		 */
		if (gActiveUnit) {
			dx = nx - gActiveUnit->xPos;
			dy = ny - gActiveUnit->yPos;
			if (dx >= -1 && dx <= 1 && dy >= -1 && dy <= 1)
				continue;
		}

		if (!FMU_CanUnitBeOnPos(unit, nx, ny))
			continue;

		/* Keep off traps (the district gate) and the door tiles */
		trap = GetTrapAt(nx, ny);
		if (trap && trap->type != TRAP_NONE)
			continue;
		if (DistrictNpc_IsDoorTerrain(gBmMapTerrain[ny][nx]))
			continue;

		/* Bare MU, no camera: start it on the old tile, then move the logical position */
		mu = StartMu(unit);
		if (!mu)
			return false;
		DisableMuCamera(mu);
		SetMuMoveScript(mu, script);

		unit->xPos = nx;
		unit->yPos = ny;
		HideUnitSprite(unit);
		RefreshEntityBmMaps();

		p->walkMu[w] = mu;
		p->walkIdx[w] = idx;
		return true;
	}
	return false;
}

static void DistrictNpc_Loop(ProcPtr proc)
{
	struct DistrictNpcProc *p = proc;
	int i, w;
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
	 * A walk ends when its MU proc is gone or stops moving, or when an event
	 * starts. The proc check comes first, so a freed slot is never read for state.
	 */
	for (w = 0; w < DISTRICT_NPC_WALKS; w++)
		if (p->walkMu[w] && (EventEngineExists() || !DistrictNpc_WalkMuLive(p, w) ||
				     p->walkMu[w]->state != MU_STATE_MOVEMENT))
			DistrictNpc_FinishWalk(p, w);

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
	Fishing_DrawMapMarkers();
#endif
#ifdef CONFIG_DISTRICT_CROSSING
	Crossing_DrawMapMarkers();
#endif
#ifdef CONFIG_DISTRICT_TOWER
	Tower_DrawMapMarkers();
#endif
#ifdef CONFIG_DISTRICT_GUILD
	Guild_DrawMapMarkers();
#endif

#ifdef CONFIG_HEX_CARDS
	DistrictNpc_DrawIcons(p);
#endif

	/* Each NPC on its own clock; one whose step is due waits for a free walk */
	for (i = 0; i < p->count; i++) {
		/* The clock runs during the step too, so a step starts about once a second */
		if (p->timer[i] > 0)
			p->timer[i]--;
		if (p->timer[i] > 0 || p->radius[i] == 0 || DistrictNpc_WalkOf(p, i) >= 0)
			continue;

		for (w = 0; w < DISTRICT_NPC_WALKS; w++)
			if (!p->walkMu[w])
				break;
		if (w == DISTRICT_NPC_WALKS)
			continue;

		/* Boxed in this time: try again a bit later */
		p->timer[i] = DistrictNpc_Step(p, i, w) ? DISTRICT_NPC_STEP_FRAMES - 10 + NextRN_N(9) : 15;
	}
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
	int w;

	if (p)
		for (w = 0; w < DISTRICT_NPC_WALKS; w++)
			DistrictNpc_FinishWalk(p, w);
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
