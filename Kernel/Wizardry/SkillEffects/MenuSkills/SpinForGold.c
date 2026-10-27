#include "common-chax.h"
#include "skill-system.h"
#include "constants/skills.h"
#include "constants/texts.h"
#include "constants/gfx.h"

#if defined(SID_SpinForGold) && (COMMON_SKILL_VALID(SID_SpinForGold))

enum {
	SPIN_COST = 100,
	SPIN_PAYOUT = 100,
	SPIN_SECTIONS = 8,

	/*
	 * Map OBJ VRAM uses 2D mapping (32 tiles per row), so each 64x32 half
	 * of the wheel takes an 8x4 tile block. Tile rows 4-11 are refreshed
	 * every few frames from gSMSGfxBuffer, so the tiles go into all three
	 * buffer frames. Standing sprites fill rows 8-11 from the right, so
	 * columns 0-17 are normally unused; RefreshUnitSprites() rebuilds the
	 * buffers afterwards in case a sprite was there.
	 */
	SPIN_SMS_FIRST_CHR = 0x80,
	SPIN_SMS_FRAMES = 3,
	SPIN_OBJ_CHR_TOP = 0x100,
	SPIN_OBJ_CHR_BOTTOM = 0x108,
	SPIN_OBJ_CHR_ARROW = 0x110,
	SPIN_OBJ_PAL = 6,
	SPIN_AFFINE = 30,

	/* Window and text, in tiles. */
	SPIN_FRAME_X = 8,
	SPIN_FRAME_Y = 1,
	SPIN_FRAME_W = 14,
	SPIN_FRAME_H = 14,
	SPIN_TEXT_X = 9,
	SPIN_TEXT_Y = 12,
	SPIN_TEXT_W = 12,

	/* Wheel centre and arrow, in pixels. */
	SPIN_WHEEL_X = 120,
	SPIN_WHEEL_Y = 62,
	SPIN_ARROW_X = 112,
	SPIN_ARROW_Y = 18,

	/* Angles: 0x10000 is one clockwise turn. */
	SPIN_TURN = 0x10000,
	SPIN_SECTION_ANGLE = SPIN_TURN / SPIN_SECTIONS,
	SPIN_SPEED = 0xA00,
	SPIN_EXTRA_TURNS = 2,

	SPIN_RESULT_FRAMES = 60,
};

/* Must match SECTIONS and PAL in Contents/Gfx/Scripts/SpinForGold/make_wheel.py. */
static const u8 sSpinForGoldValues[SPIN_SECTIONS] = { 5, 0, 1, 2, 0, 1, 3, 0 };

static const u16 sSpinForGoldPal[16] = {
	0x0000, 0x0865, 0x235F, 0x11F6, 0x18F9, 0x6586, 0x37BF, 0x6BFF,
	0x0E3A, 0x19B4, 0x110D, 0x7FFF, 0x4E52, 0x2508, 0x1093, 0x4D04,
};

static const u16 sSpinObj_HalfWheel[] = { 1, OAM0_SHAPE_64x32, OAM1_SIZE_64x32, 0 };

/* Standing map sprite frames, copied to OBJ tile rows 4-11. */
extern u8 gSMSGfxBuffer[SPIN_SMS_FRAMES][0x2000];

struct SpinForGoldProc {
	PROC_HEADER;
	/* 2C */ u32 angle; /* current rotation, 0 to SPIN_TURN - 1 */
	/* 30 */ u32 start; /* rotation when A was pressed */
	/* 34 */ u32 dist; /* rotation left to cover from start */
	/* 38 */ s16 frames, left;
	/* 3C */ u8 section;
	/* 40 */ struct Text text;
	/* 48 */ u16 savedPal[16];
};

_Static_assert(sizeof(struct SpinForGoldProc) <= 0x6C, "SpinForGoldProc too big");

u8 SpinForGold_Usability(const struct MenuItemDef *def, int number)
{
	if (gActiveUnit->state & US_CANTOING)
		return MENU_NOTSHOWN;

	if ((int)GetPartyGoldAmount() < SPIN_COST)
		return MENU_DISABLED;

	return MENU_ENABLED;
}

u8 SpinForGold_OnSelected(struct MenuProc *menu, struct MenuItemProc *item)
{
	if (item->availability == MENU_DISABLED) {
		MenuFrozenHelpBox(menu, MSG_SKILL_SpinForGold_FRtext);
		return MENU_ACT_SND6B;
	}

	gActionData.unk08 = SID_SpinForGold;
	gActionData.unitActionType = CONFIG_UNIT_ACTION_EXPA_ExecSkill;

	return MENU_ACT_SKIPCURSOR | MENU_ACT_END | MENU_ACT_SND6A | MENU_ACT_CLEAR;
}

/*
 * Section under the arrow. In the wheel art, section i is centred
 * i sections clockwise from the top.
 */
static int SpinForGold_SectionAtArrow(u32 angle)
{
	return ((SPIN_SECTION_ANGLE / 2 - angle) & (SPIN_TURN - 1)) / SPIN_SECTION_ANGLE;
}

static void SpinForGold_DrawText(struct SpinForGoldProc *proc, const char *str, int color)
{
	int x = (SPIN_TEXT_W * 8 - GetStringTextLen(str)) / 2;

	ClearText(&proc->text);
	Text_InsertDrawString(&proc->text, x, color, str);
	PutText(&proc->text, gBG0TilemapBuffer + TILEMAP_INDEX(SPIN_TEXT_X, SPIN_TEXT_Y));
	BG_EnableSyncByMask(BG0_SYNC_BIT);
}

/*
 * The wheel is two 64x32 affine sprites sharing one matrix. Each half is
 * placed where the rotation moves its centre (16 pixels above or below
 * the wheel centre).
 */
static void SpinForGold_DrawWheel(struct SpinForGoldProc *proc)
{
	int phi = (proc->angle >> 8) & 0xFF;
	int s = SIN(phi);
	int c = COS(phi);
	int ox = (s * 16) >> 12;
	int oy = (c * 16) >> 12;
	int oam2 = OAM2_PAL(SPIN_OBJ_PAL) | OAM2_LAYER(0);

	SetObjAffine(SPIN_AFFINE, c >> 4, s >> 4, -s >> 4, c >> 4);

	PutSpriteExt(0,
		OAM1_X(SPIN_WHEEL_X + ox - 64) | OAM1_AFFINE_ID(SPIN_AFFINE),
		OAM0_Y(SPIN_WHEEL_Y - oy - 32) | OAM0_AFFINE_ENABLE | OAM0_DOUBLESIZE,
		sSpinObj_HalfWheel, OAM2_CHR(SPIN_OBJ_CHR_TOP) | oam2);

	PutSpriteExt(0,
		OAM1_X(SPIN_WHEEL_X - ox - 64) | OAM1_AFFINE_ID(SPIN_AFFINE),
		OAM0_Y(SPIN_WHEEL_Y + oy - 32) | OAM0_AFFINE_ENABLE | OAM0_DOUBLESIZE,
		sSpinObj_HalfWheel, OAM2_CHR(SPIN_OBJ_CHR_BOTTOM) | oam2);

	PutSpriteExt(0, OAM1_X(SPIN_ARROW_X), OAM0_Y(SPIN_ARROW_Y), gObject_16x16, OAM2_CHR(SPIN_OBJ_CHR_ARROW) | oam2);
}

/* Copies count tiles to OBJ tile chr, in VRAM and in every standing sprite frame. */
static void SpinForGold_PutTiles(const u8 *src, int chr, int count)
{
	int f;

	CpuFastCopy(src, OBJ_CHR_ADDR(chr), count * CHR_SIZE);

	for (f = 0; f < SPIN_SMS_FRAMES; f++)
		CpuFastCopy(src, gSMSGfxBuffer[f] + (chr - SPIN_SMS_FIRST_CHR) * CHR_SIZE, count * CHR_SIZE);
}

static void SpinForGold_Init(struct SpinForGoldProc *proc)
{
	int i;

	SetPartyGoldAmount(GetPartyGoldAmount() - SPIN_COST);

	proc->angle = 0;
	proc->section = SpinForGold_SectionAtArrow(0);

	/* Wheel art tile rows 0-3 are the top half, rows 4-7 the bottom half. */
	for (i = 0; i < 8; i++) {
		int chr = (i < 4 ? SPIN_OBJ_CHR_TOP : SPIN_OBJ_CHR_BOTTOM) + (i & 3) * 32;

		SpinForGold_PutTiles(GFX_SpinForGold_Wheel + i * 8 * CHR_SIZE, chr, 8);
	}

	SpinForGold_PutTiles(GFX_SpinForGold_Arrow, SPIN_OBJ_CHR_ARROW, 2);
	SpinForGold_PutTiles(GFX_SpinForGold_Arrow + 2 * CHR_SIZE, SPIN_OBJ_CHR_ARROW + 32, 2);

	CpuCopy16(gPaletteBuffer + (0x10 + SPIN_OBJ_PAL) * 16, proc->savedPal, sizeof(proc->savedPal));
	ApplyPalette(sSpinForGoldPal, 0x10 + SPIN_OBJ_PAL);
	EnablePaletteSync();

	ResetTextFont();
	SetTextFontGlyphs(0);
	ResetText();
	ClearBg0Bg1();

	DrawUiFrame(BG_GetMapBuffer(1), SPIN_FRAME_X, SPIN_FRAME_Y, SPIN_FRAME_W, SPIN_FRAME_H, TILEREF(0, 0), 0);

	InitText(&proc->text, SPIN_TEXT_W);
	SpinForGold_DrawText(proc, GetStringFromIndex(MSG_SKILL_SpinForGold_PROMPT), TEXT_COLOR_SYSTEM_WHITE);

	BG_EnableSyncByMask(BG0_SYNC_BIT | BG1_SYNC_BIT);
	SpinForGold_DrawWheel(proc);
}

/* Full speed until A, then plan a stop on a random section. */
static void SpinForGold_Spin(struct SpinForGoldProc *proc)
{
	/* Advance the RNG every frame so the result depends on when A is pressed. */
	NextRN();

	proc->angle = (proc->angle + SPIN_SPEED) & (SPIN_TURN - 1);
	SpinForGold_DrawWheel(proc);

	if (gKeyStatusPtr->newKeys & A_BUTTON) {
		int section = NextRN_N(SPIN_SECTIONS);
		int jitter = NextRN_N(SPIN_SECTION_ANGLE * 3 / 4) - SPIN_SECTION_ANGLE * 3 / 8;
		u32 target = (jitter - section * SPIN_SECTION_ANGLE) & (SPIN_TURN - 1);

		proc->start = proc->angle;
		proc->dist = ((target - proc->angle) & (SPIN_TURN - 1)) + SPIN_EXTRA_TURNS * SPIN_TURN;

		/* Quadratic ease-out whose starting speed is SPIN_SPEED. */
		proc->frames = Div(proc->dist * 2, SPIN_SPEED);
		proc->left = proc->frames;
		proc->section = SpinForGold_SectionAtArrow(proc->angle);

		PlaySoundEffect(0x6A);
		Proc_Break(proc);
	}
}

static void SpinForGold_SlowDown(struct SpinForGoldProc *proc)
{
	int q, section;
	u32 rest;

	proc->left--;

	/* rest = dist * (left / frames)^2, in 12-bit fixed point */
	q = Div(proc->left << 12, proc->frames);
	rest = (((proc->dist * q) >> 12) * q) >> 12;

	proc->angle = (proc->start + proc->dist - rest) & (SPIN_TURN - 1);
	SpinForGold_DrawWheel(proc);

	section = SpinForGold_SectionAtArrow(proc->angle);
	if (section != proc->section) {
		proc->section = section;
		PlaySoundEffect(0x66);
	}

	if (proc->left <= 0)
		Proc_Break(proc);
}

/* Writes n (>= 0) in decimal into buf. */
static char *SpinForGold_NumberToString(int n, char *buf)
{
	char tmp[12];
	int i = 0;

	do {
		tmp[i++] = '0' + DivRem(n, 10);
		n = Div(n, 10);
	} while (n > 0);

	while (i > 0)
		*buf++ = tmp[--i];

	*buf = '\0';
	return buf;
}

static void SpinForGold_ShowResult(struct SpinForGoldProc *proc)
{
	int value = sSpinForGoldValues[proc->section];

	if (value > 0) {
		char str[16];
		char *end;

		str[0] = '+';
		end = SpinForGold_NumberToString(value * SPIN_PAYOUT, str + 1);
		end[0] = 'G';
		end[1] = '!';
		end[2] = '\0';

		SpinForGold_DrawText(proc, str, TEXT_COLOR_SYSTEM_GOLD);
	} else {
		SpinForGold_DrawText(proc, GetStringFromIndex(MSG_SKILL_SpinForGold_LOSE), TEXT_COLOR_SYSTEM_GRAY);
		PlaySoundEffect(0x6C);
	}

	proc->left = SPIN_RESULT_FRAMES;
	SpinForGold_DrawWheel(proc);
}

static void SpinForGold_Hold(struct SpinForGoldProc *proc)
{
	SpinForGold_DrawWheel(proc);

	if (--proc->left <= 0 || (gKeyStatusPtr->newKeys & A_BUTTON))
		Proc_Break(proc);
}

static void SpinForGold_End(struct SpinForGoldProc *proc)
{
	ClearBg0Bg1();
	BG_EnableSyncByMask(BG0_SYNC_BIT | BG1_SYNC_BIT);

	ApplyPalette(proc->savedPal, 0x10 + SPIN_OBJ_PAL);
	EnablePaletteSync();

	RefreshUnitSprites();
}

static void SpinForGold_Payout(struct SpinForGoldProc *proc)
{
	int value = sSpinForGoldValues[proc->section];

	if (value > 0)
		NewPopup_GoldGot(proc, gActiveUnit, value * SPIN_PAYOUT);
}

static const struct ProcCmd ProcScr_SpinForGold[] = {
	PROC_NAME("SpinForGold"),
	PROC_CALL(EndAllMus),
	PROC_SLEEP(1),

	PROC_CALL(SpinForGold_Init),
	PROC_REPEAT(SpinForGold_Spin),
	PROC_REPEAT(SpinForGold_SlowDown),
	PROC_CALL(SpinForGold_ShowResult),
	PROC_REPEAT(SpinForGold_Hold),
	PROC_CALL(SpinForGold_End),

	PROC_CALL(SpinForGold_Payout),
	PROC_SLEEP(1),
	PROC_END,
};

bool Action_SpinForGold(ProcPtr parent)
{
	ShowUnitSprite(gActiveUnit);
	Proc_StartBlocking(ProcScr_SpinForGold, parent);
	return false;
}

#endif
