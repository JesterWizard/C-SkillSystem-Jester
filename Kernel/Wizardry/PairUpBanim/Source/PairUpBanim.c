#include "common-chax.h"
#include "kernel-lib.h"
#include "ekrbattle.h"
#include "pair-up.h"
#include "banim-codec.h"
#include "feb-compat.h"

/*
 * Kernel/Wizardry/PairUpBanim: in a battle animation, a pair-up leader's support stands behind the leader in its own
 * standing pose (Documentation/Features/PairUpBanim.md). The pose does not move.
 *
 * The support's standing frame is taken from its own animation (script, right-facing OAM, sheet and palette) once a
 * battle, when the battle sets up its animations (gProc_ekrBattle's first step, PairUpBanim_ekrBattle_Init). The
 * leaders' animation buffers are borrowed for the unpacking, and UpdateBanimFrame unpacks the leaders' data into them
 * again before vanilla's ekrBattle_Init reads it.
 *
 * The frame is drawn into a canvas, cut into 16x16 pieces (8x8 where a piece is mostly empty), and the pieces go to
 * the OBJ tiles that nothing in a battle uses: OBJ VRAM is in 2D mapping there (32 tiles a row), spells load their
 * sprites at tiles 64-319, the "xN" badge at 320-323 and 352-355 (DoubleBadge.c), the HP bars and their text at 384
 * and up, and the two animations' sheets at 512-1023. So rows 0-1 (tiles 0-63) and rows 10-11 from column 4 (tiles
 * 324-351 and 356-383) are free: 60 columns of two tiles.
 *
 * The pose is a frozen struct Anim with the lowest draw-layer value of the battle: the animation system writes higher
 * values to OAM first, so the pose comes after every other animation, behind the leader, the opponent and the effects.
 * It shows while either of the leader's animations does, and through a spell (whose code draws the caster itself).
 */

#define PUB_CANVAS_TW   16 /* the canvas: 128x96 pixels, in 4bpp tiles */
#define PUB_CANVAS_TH   12
#define PUB_MAX_OBJS    62 /* both sides' pieces; each side's list also ends with a terminator */
#define PUB_COLUMNS     60 /* tile columns of two rows: 32 in rows 0-1, 28 in rows 10-11 */
#define PUB_LAYER       1 /* AnimCreate's draw layer: lower than the battle's own (0x28-0x64), so drawn last */
#define PUB_DX          24 /* the support stands this many pixels behind the leader... */
#define PUB_DY          (-4) /* ...and a little up the platform */

/* OBJ palettes no battle sprite uses: 8 is the left side's Triangle Attack palette, which UpdateBanimFrame writes */
#define PUB_OBJPAL_L    8
#define PUB_OBJPAL_R    1

struct PairUpBanimSide {
	u16 pal[16];
	u16 first; /* the side's first entry in oam[] */
	u8 valid;
	u8 objpal;
};

struct PairUpBanimState {
	u32 magic; /* PUB_MAGIC once prepared for the battle of ids[0-1] (PairUpBanim_IsCurrent) */
	u8 ids[4]; /* the left and right leaders, then their supports (unit index, 0 for none) */
	struct PairUpBanimSide side[2];
	u8 used_objs;
	u8 next_col;
	u8 stop; /* the level-up box drew over the tiles, or an OAM reached the state: no pose until the battle ends */
	u8 low_row; /* the tile row of the pieces' first 32 columns: 0 in the battle, 8 in the zooms (PairUpBanim_MoveLow) */
	/* struct Anim's sprite data: { attr0 | attr1 << 16, attr2 | x << 16, y }, a list ends with 1 */
	u32 oam[(PUB_MAX_OBJS + 2) * 3];
};

/*
 * No kernel RAM is free for it, and it is only needed during the battle animation, so the state lives in the end of
 * gBanimOamr2 (the right animation's OAM, 0x5800 bytes; vanilla keeps a word at +0x57F0). The largest OAM of the ROM's
 * animations (0xE1, 0x5094 bytes) stops well before it, and every OAM unpacked there is checked against it
 * (PairUpBanim_OamFits).
 */
#define PUB_MAGIC       0x50554231 /* "PUB1" */
#define PUB_STATE_END   0x57F0
#define PUB_STATE_OFF   (PUB_STATE_END - sizeof(struct PairUpBanimState))
#define gPairUpBanim    (*(struct PairUpBanimState *)((u8 *)gBanimOamr2 + PUB_STATE_OFF))

struct ProcPairUpBanim {
	PROC_HEADER;
	struct Anim *anim[2];
};

/* ProcScr_ekrUnitKakudai's fields: the units zooming in from the map as the battle opens, or back out as it ends */
#define KAKUDAI_S16(proc, off)  (*(s16 *)((u8 *)(proc) + (off)))
#define KAKUDAI_S32(proc, off)  (*(s32 *)((u8 *)(proc) + (off)))
#define KAKUDAI_TIMER           0x2C
#define KAKUDAI_END             0x2E
#define KAKUDAI_MAP_X(pos)      (0x32 + 2 * (pos)) /* the unit's place on the map... */
#define KAKUDAI_X(pos)          (0x36 + 2 * (pos)) /* ...and on the platform */
#define KAKUDAI_MAP_Y(pos)      (0x3A + 2 * (pos))
#define KAKUDAI_Y               0x58
#define KAKUDAI_OUT             0x44 /* 0: in, at the start; else out, at the end */
#define KAKUDAI_SHOWN(pos)      (0x4C + 4 * (pos))
#define KAKUDAI_SCALE_SMALL     0x250 /* BanimUpdateSpriteRotScale's scale on the map (larger is smaller) */
#define KAKUDAI_SCALE_FULL      0x100

extern const u8 *p_gAnimCharaPalConfig; /* BanimSync_Internal.event: the tables' pointers in vanilla's literal pools */
extern const u8 *p_gAnimCharaPalIt;

void ekrBattle_Init(struct ProcEkrBattle *proc);
void PairUpBanim_ekrBattle_Init(struct ProcEkrBattle *proc);
void PairUpBanim_KakudaiPrepare(struct ProcUnitKakudai *proc);
void PairUpBanim_KakudaiLoop(struct ProcUnitKakudai *proc);
void PairUpBanim_ekrBattleEnding(struct ProcEkrBattleEnding *proc);

/* Only the pointer is read: the pose's anim is frozen, so the script never runs */
static const u32 sPairUpBanimScr[] = { 0x80000000 };

/* 8x8 tile counts of the OBJ shapes, [shape][size] = { width, height } */
static const u8 sObjSize[3][4][2] = {
	{ { 1, 1 }, { 2, 2 }, { 4, 4 }, { 8, 8 } },
	{ { 2, 1 }, { 4, 1 }, { 4, 2 }, { 8, 4 } },
	{ { 1, 2 }, { 1, 4 }, { 2, 4 }, { 4, 8 } },
};

/*
 * The borrowed buffers: the support's script in gBanimScrLeft (0x2A00) and its OAM in gBanimOamr2, and in gBanimOaml
 * (0x5800) its sheet (0x2000), its palettes (0x100) and the canvas (0x1800).
 */
#define PUB_SCRIPT      ((u8 *)gBanimScrLeft)
#define PUB_OAM         ((u8 *)gBanimOamr2)
#define PUB_SHEET       ((u8 *)gBanimOaml)
#define PUB_PAL         ((u16 *)((u8 *)gBanimOaml + 0x2000))
#define PUB_CANVAS      ((u8 *)gBanimOaml + 0x2100)

/* An OAM unpacked into gBanimOamr2 stops before the state; the header's size is the same for LZ77 and BanimCodec's */
static bool PairUpBanim_OamFits(int idx)
{
	const u32 *oam;

	if (idx < 0)
		return true;

	oam = (const u32 *)banim_data[idx].oam_r;
	return (oam[0] >> 8) <= PUB_STATE_OFF;
}

static int PairUpBanim_ColumnChr(int col)
{
	return col < 32 ? gPairUpBanim.low_row * 32 + col : 320 + 4 + (col - 32);
}

/*
 * The first 32 columns of pieces are in tile rows 8-9 (tiles 256-319) while the units zoom in or out, and in rows 0-1
 * (tiles 0-63) during the battle. The turning platform of the zooms (NewEkrBaseKaiten) unpacks its sprites over tiles
 * 0-255; in the battle, spells use tiles 64-319 and the damage numbers 256 and up.
 */
static void PairUpBanim_MoveLow(int row)
{
	struct PairUpBanimState *st = &gPairUpBanim;
	int from = st->low_row * 32, to = row * 32, pos;

	if (st->low_row == row)
		return;

	CpuFastCopy((u8 *)OBJ_VRAM0 + from * 0x20, (u8 *)OBJ_VRAM0 + to * 0x20, 64 * 0x20);

	for (pos = 0; pos < 2; pos++) {
		u32 *ent;

		if (!st->side[pos].valid)
			continue;

		for (ent = st->oam + st->side[pos].first * 3; ent[0] != 1; ent += 3) {
			int chr = ent[1] & 0x3FF;

			if (chr >= from && chr < from + 64)
				ent[1] += to - from;
		}
	}
	st->low_row = row;
}

static u8 *PairUpBanim_ColumnTile(int col)
{
	return (u8 *)OBJ_VRAM0 + PairUpBanim_ColumnChr(col) * 0x20;
}

/* The 8 pixels of a 4bpp tile row in the other order */
static u32 PairUpBanim_FlipRow(u32 row)
{
	row = ((row >> 4) & 0x0F0F0F0F) | ((row & 0x0F0F0F0F) << 4);
	return (row >> 24) | ((row >> 8) & 0xFF00) | ((row << 8) & 0xFF0000) | (row << 24);
}

/* Draws a tile row of 8 pixels at canvas pixel (x, y); its colour-0 pixels leave the canvas as it is */
static void PairUpBanim_PutRow(int x, int y, u32 row)
{
	u32 *dst = (u32 *)(PUB_CANVAS + ((y >> 3) * PUB_CANVAS_TW + (x >> 3)) * 0x20) + (y & 7);
	u32 mask = row | (row >> 1) | (row >> 2) | (row >> 3);
	int shift = (x & 7) * 4;

	mask = (mask & 0x11111111) * 0xF;

	*dst = (*dst & ~(mask << shift)) | (row << shift);

	/* The rest goes in the next tile on the right */
	if (shift && (x >> 3) + 1 < PUB_CANVAS_TW) {
		dst += 8;
		*dst = (*dst & ~(mask >> (32 - shift))) | (row >> (32 - shift));
	}
}

/* The standing frame's sheet and OAM, from the support's script */
static const u32 *PairUpBanim_FindFrame(int idx, const void **sheet)
{
	const struct BattleAnim *banim = &banim_data[idx];
	int mode = BanimDefaultModeConfig[BanimDefaultStandingTypes[gEkrDistanceType] * 4];
	const u32 *cmd;
	int i;

	if (!PairUpBanim_OamFits(idx))
		return NULL;

	BanimUnComp(banim->script, PUB_SCRIPT);
	cmd = (const u32 *)(PUB_SCRIPT + ((const u32 *)banim->modes)[mode]);

	/* Commands (one word) may come before the first frame (0x86: the command, the sheet and the OAM offset) */
	for (i = 0; i < 16; i++, cmd++) {
		if ((*cmd >> 24) == 0x86) {
			*sheet = (const void *)cmd[1];
			BanimUnComp(banim->oam_r, PUB_OAM);
			return (const u32 *)(PUB_OAM + cmd[2]);
		}
		if ((*cmd >> 24) != 0x85)
			break;
	}
	return NULL;
}

/* Skips the frame's affine header, if any: its entries hold the rotation parameters */
static const u32 *PairUpBanim_SkipAffine(const u32 *ent)
{
	if ((ent[0] & 0xFFFF0000) == 0xFFFF0000)
		ent += 3 * (ent[0] & 0xFFFF);
	return ent;
}

static void PairUpBanim_DrawObj(const u32 *ent, int pos, int x0, int y0)
{
	int shape = (ent[0] >> 14) & 3, size = ent[0] >> 30;
	int x = (s16)(ent[1] >> 16), y = (s16)ent[2];
	int tw = sObjSize[shape][size][0], th = sObjSize[shape][size][1];
	int hflip = (ent[0] >> 28) & 1, vflip = (ent[0] >> 29) & 1;
	int chr = ent[1] & 0x3FF;
	const u32 *sheet = (const u32 *)PUB_SHEET;
	int py, c;

	/* The left side faces the other way: the OAM's x mirrors around the anim's position */
	if (pos == EKR_POS_L) {
		x = -x - tw * 8;
		hflip = !hflip;
	}

	for (py = 0; py < th * 8; py++) {
		int sy = vflip ? th * 8 - 1 - py : py;

		for (c = 0; c < tw; c++) {
			int tile = chr + (sy >> 3) * 32 + (hflip ? tw - 1 - c : c);
			u32 row;

			if (tile >= 0x100) /* past the 256x64 sheet */
				continue;

			row = sheet[tile * 8 + (sy & 7)];
			if (!row)
				continue;

			PairUpBanim_PutRow(x + c * 8 - x0, y + py - y0, hflip ? PairUpBanim_FlipRow(row) : row);
		}
	}
}

static bool PairUpBanim_SkipObj(const u32 *ent)
{
	return ((ent[0] >> 14) & 3) == 3 || (ent[0] & 0x100); /* not a shape, or affine */
}

/* Draws the frame into the canvas; returns false if it is larger than the canvas */
static bool PairUpBanim_Compose(const u32 *frame, int pos, int *ox, int *oy)
{
	const u32 *start = PairUpBanim_SkipAffine(frame);
	const u32 *ent;
	int x0 = 0x7FFF, y0 = 0x7FFF, x1 = -0x7FFF, y1 = -0x7FFF;

	for (ent = start; ent[0] != 1; ent += 3) {
		int shape = (ent[0] >> 14) & 3, size = ent[0] >> 30;
		int x = (s16)(ent[1] >> 16), y = (s16)ent[2];
		int w, h;

		if (PairUpBanim_SkipObj(ent))
			continue;

		w = sObjSize[shape][size][0] * 8;
		h = sObjSize[shape][size][1] * 8;
		if (pos == EKR_POS_L)
			x = -x - w;

		x0 = x < x0 ? x : x0;
		y0 = y < y0 ? y : y0;
		x1 = x + w > x1 ? x + w : x1;
		y1 = y + h > y1 ? y + h : y1;
	}

	if (x0 > x1 || x1 - x0 > PUB_CANVAS_TW * 8 || y1 - y0 > PUB_CANVAS_TH * 8)
		return false;

	CpuFill16(0, PUB_CANVAS, PUB_CANVAS_TW * PUB_CANVAS_TH * 0x20);

	/* The first entry is on top: draw from the last */
	while (ent != start) {
		ent -= 3;
		if (!PairUpBanim_SkipObj(ent))
			PairUpBanim_DrawObj(ent, pos, x0, y0);
	}

	*ox = x0;
	*oy = y0;
	return true;
}

static bool PairUpBanim_TileUsed(int tx, int ty)
{
	const u32 *t = (const u32 *)(PUB_CANVAS + (ty * PUB_CANVAS_TW + tx) * 0x20);
	int i;

	for (i = 0; i < 8; i++)
		if (t[i])
			return true;
	return false;
}

static void PairUpBanim_CopyTile(int tx, int ty, u8 *dst)
{
	CpuCopy32(PUB_CANVAS + (ty * PUB_CANVAS_TW + tx) * 0x20, dst, 0x20);
}

static void PairUpBanim_AddObj(u32 attr01, int chr, int x, int y)
{
	u32 *o = gPairUpBanim.oam + gPairUpBanim.used_objs++ * 3;

	o[0] = attr01;
	o[1] = OAM2_CHR(chr) | ((u16)x << 16);
	o[2] = (u16)y;
}

/* A 16x16 piece: two columns side by side, in the same tile rows */
static bool PairUpBanim_PlaceBlock(int bx, int by, int ox, int oy)
{
	struct PairUpBanimState *st = &gPairUpBanim;
	u8 *dst;

	if (st->next_col == 31)
		st->next_col = 32; /* a piece can't span rows 0-1 and 10-11 */
	if (st->next_col + 2 > PUB_COLUMNS || st->used_objs >= PUB_MAX_OBJS)
		return false;

	dst = PairUpBanim_ColumnTile(st->next_col);
	PairUpBanim_CopyTile(bx * 2, by * 2, dst);
	PairUpBanim_CopyTile(bx * 2 + 1, by * 2, dst + 0x20);
	PairUpBanim_CopyTile(bx * 2, by * 2 + 1, dst + 32 * 0x20);
	PairUpBanim_CopyTile(bx * 2 + 1, by * 2 + 1, dst + 33 * 0x20);

	PairUpBanim_AddObj(OAM1_SIZE_16x16 << 16, PairUpBanim_ColumnChr(st->next_col), ox + bx * 16, oy + by * 16);
	st->next_col += 2;
	return true;
}

/* An 8x8 piece: the bottom tile of the column the last one started, or the top tile of a new column */
static bool PairUpBanim_PlaceTile(int tx, int ty, int ox, int oy, int *spare)
{
	struct PairUpBanimState *st = &gPairUpBanim;
	int chr;

	if (st->used_objs >= PUB_MAX_OBJS)
		return false;

	if (*spare >= 0) {
		chr = PairUpBanim_ColumnChr(*spare) + 32;
		*spare = -1;
	} else {
		if (st->next_col >= PUB_COLUMNS)
			return false;
		chr = PairUpBanim_ColumnChr(st->next_col);
		*spare = st->next_col++;
	}

	PairUpBanim_CopyTile(tx, ty, (u8 *)OBJ_VRAM0 + chr * 0x20);
	PairUpBanim_AddObj(0, chr, ox + tx * 8, oy + ty * 8);
	return true;
}

/*
 * Cuts the canvas into pieces and puts them in the free columns: a 16x16 piece where three or four of its tiles have
 * pixels, 8x8 pieces for the rest (one tile of a column each).
 */
static bool PairUpBanim_Place(int ox, int oy)
{
	int bx, by, i, j;
	int spare = -1; /* a column with its bottom tile free for an 8x8 piece */

	for (by = 0; by < PUB_CANVAS_TH / 2; by++) {
		for (bx = 0; bx < PUB_CANVAS_TW / 2; bx++) {
			int n = 0;

			for (j = 0; j < 2; j++)
				for (i = 0; i < 2; i++)
					n += PairUpBanim_TileUsed(bx * 2 + i, by * 2 + j);

			if (n >= 3) {
				if (!PairUpBanim_PlaceBlock(bx, by, ox, oy))
					return false;
				continue;
			}

			for (j = 0; j < 2; j++)
				for (i = 0; i < 2; i++)
					if (PairUpBanim_TileUsed(bx * 2 + i, by * 2 + j) &&
					    !PairUpBanim_PlaceTile(bx * 2 + i, by * 2 + j, ox, oy, &spare))
						return false;
		}
	}
	return true;
}

static int PairUpBanim_GetBanimIdx(struct Unit *unit, const struct BattleAnimDef *def)
{
	u32 unused;
	u16 weapon = GetUnitEquippedWeapon(unit);
	int idx = -1;

	if (!weapon)
		weapon = unit->items[0];
	if (weapon)
		idx = (s16)GetBattleAnimationId(unit, def, weapon, &unused);
	if (idx < 0)
		idx = (s16)GetBattleAnimationId(unit, def, 0, &unused);
	return idx;
}

static void PairUpBanim_LoadPalette(struct PairUpBanimSide *side, struct Unit *unit,
				    const struct BattleAnimDef *def, int idx)
{
	int faction = GetBanimFactionPalette(unit->index & 0xC0);
	int pid = unit->pCharacterData->number, jid = unit->pClassData->number;
	int i;

	BanimUnComp(banim_data[idx].pal, PUB_PAL);

	/* A character palette is laid out for its class's animations, as in PrepareBattleGraphicsMaybe */
	if (def == unit->pClassData->pBattleAnimDef) {
		for (i = 0; i < 7; i++) {
			if (p_gAnimCharaPalConfig[(pid - 1) * 7 + i] == jid) {
				int uniq = p_gAnimCharaPalIt[(pid - 1) * 7 + i] - 1;

				if (uniq >= 0)
					BanimUnComp(character_battle_animation_palette_table[uniq].pal, PUB_PAL);
				break;
			}
		}
	}

	CpuCopy16(PUB_PAL + faction * 16, side->pal, sizeof(side->pal));
}

static struct Unit *PairUpBanim_GetLeader(int pos)
{
	struct BattleUnit *bu = pos == EKR_POS_L ? gpEkrBattleUnitLeft : gpEkrBattleUnitRight;

	if (!gBanimValid[pos] || !bu)
		return NULL;
	return GetUnit(bu->unit.index);
}

/*
 * Who stands on the battle's two sides: the leaders, from the battle's own copies (a unit that dies in the battle is
 * cleared from the unit array), then their supports (unit index, 0 for none)
 */
static void PairUpBanim_GetIds(u8 *ids)
{
	int pos;

	for (pos = 0; pos < 2; pos++) {
		struct BattleUnit *bu = pos == EKR_POS_L ? gpEkrBattleUnitLeft : gpEkrBattleUnitRight;
		struct Unit *leader = PairUpBanim_GetLeader(pos);
		struct Unit *support = leader ? PairUp_GetSupport(leader) : NULL;

		ids[pos] = bu ? (u8)bu->unit.index : 0; /* even once its unit died (gBanimValid is 0 then) */
		ids[2 + pos] = support ? support->index : 0;
	}
}

/*
 * The support whose name tag stacks under its leader's in the battle's top bar (VeslyUI's EfxPrepareScreenFx and
 * ekrDispUPMain), or NULL: as for the poses, none in promotions, the arena or the link arena. The tag doesn't need the
 * pose (pair_up_banim).
 */
struct Unit *PairUpBanim_GetTagSupport(int pos)
{
	struct Unit *leader;

	if (gEkrDistanceType == EKR_DISTANCE_PROMOTION || (gBattleStats.config & BATTLE_CONFIG_ARENA))
		return NULL;
	if (gBmSt.gameStateBits & BM_FLAG_LINKARENA)
		return NULL;

	leader = PairUpBanim_GetLeader(pos);
	return leader ? PairUp_GetSupport(leader) : NULL;
}

static bool PairUpBanim_Enabled(void)
{
	if (!gpKernelDesignerConfig->pair_up_banim)
		return false;

	if (gEkrDistanceType == EKR_DISTANCE_PROMOTION || (gBattleStats.config & BATTLE_CONFIG_ARENA))
		return false;

	if (gBmSt.gameStateBits & BM_FLAG_LINKARENA)
		return false;

	/* The state lives past the right animation's OAM */
	return !gBanimValid[EKR_POS_R] || PairUpBanim_OamFits(gBanimIdx[EKR_POS_R]);
}

/* The state was prepared for this battle (as the units zoomed in), for these units */
static bool PairUpBanim_IsCurrent(void)
{
	u8 ids[4];

	if (!PairUpBanim_Enabled() || gPairUpBanim.magic != PUB_MAGIC)
		return false;

	/* The leaders only: a support's pairing ends if its leader dies, and the zoom out still shows the other side's */
	PairUpBanim_GetIds(ids);
	return ids[0] == gPairUpBanim.ids[0] && ids[1] == gPairUpBanim.ids[1];
}

static void PairUpBanim_PrepareSide(int pos)
{
	struct PairUpBanimState *st = &gPairUpBanim;
	struct PairUpBanimSide *side = &st->side[pos];
	struct Unit *leader = PairUpBanim_GetLeader(pos);
	struct Unit *support = leader ? PairUp_GetSupport(leader) : NULL;
	const struct BattleAnimDef *def;
	const void *sheet;
	const u32 *frame;
	int idx, ox, oy, first_obj, first_col;

	if (!support)
		return;

	def = GetUnitBattleAnimDef(support);
	idx = PairUpBanim_GetBanimIdx(support, def);
	if (idx < 0)
		return;

	frame = PairUpBanim_FindFrame(idx, &sheet);
	if (!frame)
		return;

	BanimUnComp(sheet, PUB_SHEET);
	if (!PairUpBanim_Compose(frame, pos, &ox, &oy))
		return;

	/* The pieces' places are from the leader's: the support stands behind it, and scales with it in the zoom */
	ox += pos == EKR_POS_L ? -PUB_DX : PUB_DX;
	oy += PUB_DY;

	first_obj = st->used_objs;
	first_col = st->next_col;
	if (!PairUpBanim_Place(ox, oy)) {
		/* Too large for what is left: no pose for this side */
		st->used_objs = first_obj;
		st->next_col = first_col;
		return;
	}

	/* The side's list ends here */
	st->oam[st->used_objs * 3] = 1;
	st->used_objs++;

	PairUpBanim_LoadPalette(side, support, def, idx);
	side->first = first_obj;
	side->objpal = pos == EKR_POS_L ? PUB_OBJPAL_L : PUB_OBJPAL_R;
	side->valid = true;
}

static void PairUpBanim_SyncPalette(struct PairUpBanimSide *side)
{
	u16 *pal = gPaletteBuffer + 0x100 + side->objpal * 16;
	int i;

	/* UpdateBanimFrame writes the left side's over again when an animation switches */
	for (i = 0; i < 16; i++) {
		if (pal[i] != side->pal[i]) {
			CpuCopy16(side->pal, pal, sizeof(side->pal));
			EnablePaletteSync();
			return;
		}
	}
}

/*
 * Returns whether either side has a pose. restore: the leaders' animations are already unpacked, and are unpacked again
 * (UpdateBanimFrame, about 1.7 frames) where the supports' were. Before vanilla unpacks them, it isn't needed.
 */
static bool PairUpBanim_Prepare(bool restore)
{
	struct PairUpBanimState *st = &gPairUpBanim;
	int pos;

	if (!PairUpBanim_Enabled())
		return false;

	CpuFill16(0, st, sizeof(*st));
	PairUpBanim_GetIds(st->ids);
	st->magic = PUB_MAGIC;
	st->low_row = 8; /* free while the units zoom in */

	if (!st->ids[2] && !st->ids[3])
		return false;

	/* Two large poses may not both fit in the free tiles: the player's side goes first */
	pos = gBanimPositionIsEnemy[EKR_POS_L] ? EKR_POS_R : EKR_POS_L;
	PairUpBanim_PrepareSide(pos);
	PairUpBanim_PrepareSide(1 - pos);

	if (!st->side[EKR_POS_L].valid && !st->side[EKR_POS_R].valid)
		return false;

	/* Put the leaders' scripts and OAM back where the support's were unpacked (it writes OBJ palette 8 too) */
	if (restore)
		UpdateBanimFrame();

	for (pos = 0; pos < 2; pos++)
		if (st->side[pos].valid)
			PairUpBanim_SyncPalette(&st->side[pos]);
	return true;
}

static bool PairUpBanim_IsOwn(struct Anim *anim)
{
	return anim && (anim->state & ANIM_BIT_ENABLED) && anim->pScrStart == sPairUpBanimScr;
}

static bool PairUpBanim_Shown(struct Anim *anim)
{
	if (!anim || !(anim->state & ANIM_BIT_ENABLED))
		return false;

	/* While a spell plays, its code hides the caster's animations from the animation system and draws them itself */
	return !(anim->state & ANIM_BIT_HIDDEN) || gEfxSpellAnimExists;
}

static void PairUpBanim_Loop(struct ProcPairUpBanim *proc)
{
	struct PairUpBanimState *st = &gPairUpBanim;
	int pos;

	/*
	 * An animation switched in mid-battle may unpack over the state (PairUpBanim_IsCurrent is false from then): the
	 * poses go (PairUpBanim_End)
	 */
	if (gBanimValid[EKR_POS_R] && !PairUpBanim_OamFits(gBanimIdx[EKR_POS_R])) {
		Proc_Break(proc);
		return;
	}

	/* The level-up box draws over the tiles */
	if (Proc_Find(ProcScr_EkrLevelup))
		st->stop = true;

	/*
	 * The zoom in is over: the pieces leave the tiles the spells use. Once the battle ends, they go back before the
	 * zoom out's platform is unpacked over them (a frame early: this frame's OAM is already made).
	 */
	if (Proc_Find(ProcScr_ekrBattleEnding))
		PairUpBanim_MoveLow(8);
	else if (!Proc_Find(ProcScr_EkrBaseKaiten))
		PairUpBanim_MoveLow(0);

	for (pos = 0; pos < 2; pos++) {
		struct Anim *anim = proc->anim[pos];
		struct Anim *leader = gAnims[pos * 2];
		struct Anim *back = gAnims[pos * 2 + 1];

		if (!PairUpBanim_IsOwn(anim)) {
			proc->anim[pos] = NULL;
			continue;
		}

		/* The pose goes with its leader's anim (ekrBattleEnding_8056228 clears them all as the zoom out starts) */
		if (!leader || !(leader->state & ANIM_BIT_ENABLED)) {
			AnimDelete(anim);
			proc->anim[pos] = NULL;
			continue;
		}

		if (st->stop || !leader || (!PairUpBanim_Shown(leader) && !PairUpBanim_Shown(back))) {
			anim->state |= ANIM_BIT_HIDDEN;
			continue;
		}

		anim->xPosition = leader->xPosition;
		anim->yPosition = leader->yPosition;
		anim->state &= ~ANIM_BIT_HIDDEN;

		PairUpBanim_SyncPalette(&st->side[pos]);
	}

	if (!proc->anim[EKR_POS_L] && !proc->anim[EKR_POS_R])
		Proc_Break(proc);
}

static void PairUpBanim_End(struct ProcPairUpBanim *proc)
{
	int pos;

	for (pos = 0; pos < 2; pos++) {
		if (PairUpBanim_IsOwn(proc->anim[pos]))
			AnimDelete(proc->anim[pos]);
		proc->anim[pos] = NULL;
	}
}

static const struct ProcCmd ProcScr_PairUpBanim[] = {
	PROC_NAME("PairUpBanim"),
	PROC_SET_END_CB(PairUpBanim_End),
	PROC_REPEAT(PairUpBanim_Loop),
	PROC_END,
};

static void PairUpBanim_Start(void)
{
	struct ProcPairUpBanim *proc;
	int pos;

	/* Not the battle's child: it ends before its last frames are drawn */
	proc = Proc_Start(ProcScr_PairUpBanim, PROC_TREE_3);

	for (pos = 0; pos < 2; pos++) {
		struct PairUpBanimSide *side = &gPairUpBanim.side[pos];
		struct Anim *anim = NULL;

		if (side->valid)
			anim = AnimCreate(sPairUpBanimScr, PUB_LAYER);

		proc->anim[pos] = anim;
		if (!anim)
			continue;

		anim->state |= ANIM_BIT_FROZEN | ANIM_BIT_HIDDEN;
		anim->oam2Base = OAM2_PAL(side->objpal) | OAM2_LAYER(2);
		anim->pSpriteData = gPairUpBanim.oam + side->first * 3;
	}
}

/* gProc_ekrBattle's first step (PairUpBanim_Installer.event): vanilla's, then the poses' anims */
void PairUpBanim_ekrBattle_Init(struct ProcEkrBattle *proc)
{
	/* The poses are made as the units zoom in; if that step did not run, make them now */
	bool any = PairUpBanim_IsCurrent() ? (gPairUpBanim.side[0].valid || gPairUpBanim.side[1].valid)
					    : PairUpBanim_Prepare(true);

	ekrBattle_Init(proc);

	if (any)
		PairUpBanim_Start();
}

/*
 * ProcScr_ekrUnitKakudai's first step: the poses, then vanilla's, which unpacks the leaders' animations into the buffers
 * the poses were made in
 */
void PairUpBanim_KakudaiPrepare(struct ProcUnitKakudai *proc)
{
	/* A new battle; while a map unit still moves, the poses are made later in the zoom (PairUpBanim_KakudaiLoop) */
	if (!KAKUDAI_S32(proc, KAKUDAI_OUT) && PairUpBanim_Enabled()) {
		gPairUpBanim.magic = 0;
		if (!MuExists())
			PairUpBanim_Prepare(false);
	}

	UnitKakudaiPrepareAnimScript(proc);
}

/* Each side's pose, scaled and placed as vanilla does the leader's in this frame of the zoom */
static void PairUpBanim_KakudaiDraw(struct ProcUnitKakudai *proc, int pos, int t)
{
	struct PairUpBanimSide *side = &gPairUpBanim.side[pos];
	int end = KAKUDAI_S16(proc, KAKUDAI_END);
	int map_x = KAKUDAI_S16(proc, KAKUDAI_MAP_X(pos)), x = KAKUDAI_S16(proc, KAKUDAI_X(pos));
	int map_y = KAKUDAI_S16(proc, KAKUDAI_MAP_Y(pos)), y = KAKUDAI_Y;
	int scale;
	struct Anim anim;
	u32 buf[(PUB_MAX_OBJS + 2) * 3 + 12 + 1];

	if (!side->valid || KAKUDAI_S32(proc, KAKUDAI_SHOWN(pos)) != 1)
		return;

	if (!KAKUDAI_S32(proc, KAKUDAI_OUT)) {
		scale = Interpolate(0, KAKUDAI_SCALE_SMALL, KAKUDAI_SCALE_FULL, t, end);
		x = Interpolate(0, map_x, x, t, end);
		y = Interpolate(0, map_y, y, t, end);
	} else {
		scale = Interpolate(0, KAKUDAI_SCALE_FULL, KAKUDAI_SCALE_SMALL, t, end);
		x = Interpolate(0, x, map_x, t, end);
		y = Interpolate(0, y, map_y, t, end);
	}

	BanimUpdateSpriteRotScale(gPairUpBanim.oam + side->first * 3, (void *)buf, scale, scale, pos);
	PairUpBanim_SyncPalette(side);

	CpuFill16(0, &anim, sizeof(anim));
	anim.xPosition = x;
	anim.yPosition = y;
	anim.oam2Base = OAM2_PAL(side->objpal);
	anim.pSpriteData = buf;
	AnimDisplay(&anim);
}

/* ProcScr_ekrUnitKakudai's loop: vanilla's (it draws the leaders), then the poses, which go behind them in OAM */
void PairUpBanim_KakudaiLoop(struct ProcUnitKakudai *proc)
{
	int t = KAKUDAI_S16(proc, KAKUDAI_TIMER);
	int pos;

	UnitKakudaiMain(proc);

	/* The leaders' OAM, which the zoom reads, is unpacked again before this returns */
	if (!KAKUDAI_S32(proc, KAKUDAI_OUT) && !PairUpBanim_IsCurrent() && !MuExists())
		PairUpBanim_Prepare(true);

	if (!PairUpBanim_IsCurrent() || gPairUpBanim.stop)
		return;

	if (t >= KAKUDAI_S16(proc, KAKUDAI_END)) {
		/* The zoom is over; after the one back to the map, the battle is */
		if (KAKUDAI_S32(proc, KAKUDAI_OUT)) {
			gPairUpBanim.magic = 0;
			return;
		}

		/*
		 * This frame's OAM was made before the poses' anims were shown (PairUpBanim_Loop): draw them once more, in
		 * their places, or they blink out for a frame
		 */
		t = KAKUDAI_S16(proc, KAKUDAI_END) - 1;
	}

	for (pos = 0; pos < 2; pos++)
		PairUpBanim_KakudaiDraw(proc, pos, t + 1);
}

/*
 * ProcScr_ekrBattleEnding's step that starts the zoom back to the map: vanilla's, after moving the poses' tiles out of
 * the turning platform's (PairUpBanim_MoveLow)
 */
void PairUpBanim_ekrBattleEnding(struct ProcEkrBattleEnding *proc)
{
	if (PairUpBanim_IsCurrent() && !gPairUpBanim.stop)
		PairUpBanim_MoveLow(8);

	ekrBattleEnding_8056228(proc);
}
