#include "common-chax.h"
#include "skill-system.h"
#include "skill-wheel.h"
#include "constants/skills.h"
#include "constants/texts.h"
#include "constants/gfx.h"

#if (defined(SID_SpinForGold) && COMMON_SKILL_VALID(SID_SpinForGold)) || \
	(defined(SID_SpinForLuck) && COMMON_SKILL_VALID(SID_SpinForLuck))

enum {
	/*
	 * Map OBJ VRAM uses 2D mapping (32 tiles per row), so each 64x32 half
	 * of the wheel takes an 8x4 tile block. Tile rows 4-11 are refreshed
	 * every few frames from gSMSGfxBuffer, so the tiles go into all three
	 * buffer frames. Standing sprites fill rows 8-11 from the right, so
	 * columns 0-17 are normally unused; RefreshUnitSprites() rebuilds the
	 * buffers afterwards in case a sprite was there.
	 */
	WHEEL_SMS_FIRST_CHR = 0x80,
	WHEEL_SMS_FRAMES = 3,
	WHEEL_OBJ_CHR_TOP = 0x100,
	WHEEL_OBJ_CHR_BOTTOM = 0x108,
	WHEEL_OBJ_CHR_ARROW = 0x110,
	WHEEL_OBJ_PAL = 6,
	WHEEL_AFFINE = 30,

	/* Window and text, in tiles. */
	WHEEL_FRAME_X = 8,
	WHEEL_FRAME_Y = 1,
	WHEEL_FRAME_W = 14,
	WHEEL_FRAME_H = 14,
	WHEEL_TEXT_X = 9,
	WHEEL_TEXT_Y = 12,
	WHEEL_TEXT_W = 12,

	/* Wheel centre and arrow, in pixels. */
	WHEEL_X = 120,
	WHEEL_Y = 62,
	WHEEL_ARROW_X = 112,
	WHEEL_ARROW_Y = 18,

	/* Angles: 0x10000 is one clockwise turn. */
	WHEEL_TURN = 0x10000,
	WHEEL_SECTION_ANGLE = WHEEL_TURN / SKILL_WHEEL_SECTIONS,
	WHEEL_SPEED = 0xA00,
	WHEEL_EXTRA_TURNS = 2,

	WHEEL_RESULT_FRAMES = 60,
};

static const u16 sSkillWheelObj_Half[] = { 1, OAM0_SHAPE_64x32, OAM1_SIZE_64x32, 0 };

/* Standing map sprite frames, copied to OBJ tile rows 4-11. */
extern u8 gSMSGfxBuffer[WHEEL_SMS_FRAMES][0x2000];

struct SkillWheelProc {
	PROC_HEADER;
	/* 2C */ const struct SkillWheelConf *conf;
	/* 30 */ u32 angle; /* current rotation, 0 to WHEEL_TURN - 1 */
	/* 34 */ u32 start; /* rotation when A was pressed */
	/* 38 */ u32 dist; /* rotation left to cover from start */
	/* 3C */ s16 frames, left;
	/* 40 */ struct Text text;
	/* 48 */ u16 savedPal[16];
	/* 68 */ u8 section;
};

_Static_assert(sizeof(struct SkillWheelProc) <= 0x6C, "SkillWheelProc too big");

/* Section under the arrow. */
static int SkillWheel_SectionAtArrow(u32 angle)
{
	return ((WHEEL_SECTION_ANGLE / 2 - angle) & (WHEEL_TURN - 1)) / WHEEL_SECTION_ANGLE;
}

static void SkillWheel_DrawText(struct SkillWheelProc *proc, const char *str, int color)
{
	int x = (WHEEL_TEXT_W * 8 - GetStringTextLen(str)) / 2;

	ClearText(&proc->text);
	Text_InsertDrawString(&proc->text, x, color, str);
	PutText(&proc->text, gBG0TilemapBuffer + TILEMAP_INDEX(WHEEL_TEXT_X, WHEEL_TEXT_Y));
	BG_EnableSyncByMask(BG0_SYNC_BIT);
}

/*
 * The wheel is two 64x32 affine sprites sharing one matrix. Each half is
 * placed where the rotation moves its centre (16 pixels above or below
 * the wheel centre).
 */
static void SkillWheel_Draw(struct SkillWheelProc *proc)
{
	int phi = (proc->angle >> 8) & 0xFF;
	int s = SIN(phi);
	int c = COS(phi);
	int ox = (s * 16) >> 12;
	int oy = (c * 16) >> 12;
	int oam2 = OAM2_PAL(WHEEL_OBJ_PAL) | OAM2_LAYER(0);

	SetObjAffine(WHEEL_AFFINE, c >> 4, s >> 4, -s >> 4, c >> 4);

	PutSpriteExt(0,
		OAM1_X(WHEEL_X + ox - 64) | OAM1_AFFINE_ID(WHEEL_AFFINE),
		OAM0_Y(WHEEL_Y - oy - 32) | OAM0_AFFINE_ENABLE | OAM0_DOUBLESIZE,
		sSkillWheelObj_Half, OAM2_CHR(WHEEL_OBJ_CHR_TOP) | oam2);

	PutSpriteExt(0,
		OAM1_X(WHEEL_X - ox - 64) | OAM1_AFFINE_ID(WHEEL_AFFINE),
		OAM0_Y(WHEEL_Y + oy - 32) | OAM0_AFFINE_ENABLE | OAM0_DOUBLESIZE,
		sSkillWheelObj_Half, OAM2_CHR(WHEEL_OBJ_CHR_BOTTOM) | oam2);

	PutSpriteExt(0, OAM1_X(WHEEL_ARROW_X), OAM0_Y(WHEEL_ARROW_Y), gObject_16x16, OAM2_CHR(WHEEL_OBJ_CHR_ARROW) | oam2);
}

/* Copies count tiles to OBJ tile chr, in VRAM and in every standing sprite frame. */
static void SkillWheel_PutTiles(const u8 *src, int chr, int count)
{
	int f;

	CpuFastCopy(src, OBJ_CHR_ADDR(chr), count * CHR_SIZE);

	for (f = 0; f < WHEEL_SMS_FRAMES; f++)
		CpuFastCopy(src, gSMSGfxBuffer[f] + (chr - WHEEL_SMS_FIRST_CHR) * CHR_SIZE, count * CHR_SIZE);
}

static void SkillWheel_Init(struct SkillWheelProc *proc)
{
	int i;

	proc->angle = 0;
	proc->section = SkillWheel_SectionAtArrow(0);

	/* Wheel art tile rows 0-3 are the top half, rows 4-7 the bottom half. */
	for (i = 0; i < 8; i++) {
		int chr = (i < 4 ? WHEEL_OBJ_CHR_TOP : WHEEL_OBJ_CHR_BOTTOM) + (i & 3) * 32;

		SkillWheel_PutTiles(proc->conf->gfx + i * 8 * CHR_SIZE, chr, 8);
	}

	/* The arrow is shared by every wheel. */
	SkillWheel_PutTiles(GFX_SpinForGold_Arrow, WHEEL_OBJ_CHR_ARROW, 2);
	SkillWheel_PutTiles(GFX_SpinForGold_Arrow + 2 * CHR_SIZE, WHEEL_OBJ_CHR_ARROW + 32, 2);

	CpuCopy16(gPaletteBuffer + (0x10 + WHEEL_OBJ_PAL) * 16, proc->savedPal, sizeof(proc->savedPal));
	ApplyPalette(proc->conf->pal, 0x10 + WHEEL_OBJ_PAL);
	EnablePaletteSync();

	ResetTextFont();
	SetTextFontGlyphs(0);
	ResetText();
	ClearBg0Bg1();

	DrawUiFrame(BG_GetMapBuffer(1), WHEEL_FRAME_X, WHEEL_FRAME_Y, WHEEL_FRAME_W, WHEEL_FRAME_H, TILEREF(0, 0), 0);

	InitText(&proc->text, WHEEL_TEXT_W);
	SkillWheel_DrawText(proc, GetStringFromIndex(proc->conf->prompt), TEXT_COLOR_SYSTEM_WHITE);

	BG_EnableSyncByMask(BG0_SYNC_BIT | BG1_SYNC_BIT);
	SkillWheel_Draw(proc);
}

/* Full speed until A, then plan a stop on a random section. */
static void SkillWheel_Spin(struct SkillWheelProc *proc)
{
	/* Advance the RNG every frame so the result depends on when A is pressed. */
	NextRN();

	proc->angle = (proc->angle + WHEEL_SPEED) & (WHEEL_TURN - 1);
	SkillWheel_Draw(proc);

	if (gKeyStatusPtr->newKeys & A_BUTTON) {
		int section = NextRN_N(SKILL_WHEEL_SECTIONS);
		int jitter = NextRN_N(WHEEL_SECTION_ANGLE * 3 / 4) - WHEEL_SECTION_ANGLE * 3 / 8;
		u32 target = (jitter - section * WHEEL_SECTION_ANGLE) & (WHEEL_TURN - 1);

		proc->start = proc->angle;
		proc->dist = ((target - proc->angle) & (WHEEL_TURN - 1)) + WHEEL_EXTRA_TURNS * WHEEL_TURN;

		/* Quadratic ease-out whose starting speed is WHEEL_SPEED. */
		proc->frames = Div(proc->dist * 2, WHEEL_SPEED);
		proc->left = proc->frames;
		proc->section = SkillWheel_SectionAtArrow(proc->angle);

		PlaySoundEffect(0x6A);
		Proc_Break(proc);
	}
}

static void SkillWheel_SlowDown(struct SkillWheelProc *proc)
{
	int q, section;
	u32 rest;

	proc->left--;

	/* rest = dist * (left / frames)^2, in 12-bit fixed point */
	q = Div(proc->left << 12, proc->frames);
	rest = (((proc->dist * q) >> 12) * q) >> 12;

	proc->angle = (proc->start + proc->dist - rest) & (WHEEL_TURN - 1);
	SkillWheel_Draw(proc);

	section = SkillWheel_SectionAtArrow(proc->angle);
	if (section != proc->section) {
		proc->section = section;
		PlaySoundEffect(0x66);
	}

	if (proc->left <= 0)
		Proc_Break(proc);
}

static void SkillWheel_ShowResult(struct SkillWheelProc *proc)
{
	char buf[32];
	int color = TEXT_COLOR_SYSTEM_WHITE;
	const char *str = proc->conf->describe(proc->section, buf, &color);

	SkillWheel_DrawText(proc, str, color);

	proc->left = WHEEL_RESULT_FRAMES;
	SkillWheel_Draw(proc);
}

static void SkillWheel_Hold(struct SkillWheelProc *proc)
{
	SkillWheel_Draw(proc);

	if (--proc->left <= 0 || (gKeyStatusPtr->newKeys & A_BUTTON))
		Proc_Break(proc);
}

static void SkillWheel_End(struct SkillWheelProc *proc)
{
	ClearBg0Bg1();
	BG_EnableSyncByMask(BG0_SYNC_BIT | BG1_SYNC_BIT);

	ApplyPalette(proc->savedPal, 0x10 + WHEEL_OBJ_PAL);
	EnablePaletteSync();

	RefreshUnitSprites();
}

static void SkillWheel_Apply(struct SkillWheelProc *proc)
{
	proc->conf->apply(proc, proc->section);
}

static const struct ProcCmd ProcScr_SkillWheel[] = {
	PROC_NAME("SkillWheel"),
	PROC_CALL(EndAllMus),
	PROC_SLEEP(1),

	PROC_CALL(SkillWheel_Init),
	PROC_REPEAT(SkillWheel_Spin),
	PROC_REPEAT(SkillWheel_SlowDown),
	PROC_CALL(SkillWheel_ShowResult),
	PROC_REPEAT(SkillWheel_Hold),
	PROC_CALL(SkillWheel_End),

	PROC_CALL(SkillWheel_Apply),
	PROC_SLEEP(1),
	PROC_END,
};

void StartSkillWheel(const struct SkillWheelConf *conf, ProcPtr parent)
{
	struct SkillWheelProc *proc;

	ShowUnitSprite(gActiveUnit);

	/* The script sleeps before SkillWheel_Init, so conf is set in time. */
	proc = Proc_StartBlocking(ProcScr_SkillWheel, parent);
	proc->conf = conf;
}

#endif
