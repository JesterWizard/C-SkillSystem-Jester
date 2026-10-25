#include "common-chax.h"
#include "kernel-lib.h"
#include "gamecontrol.h"
#include "bmsave.h"
#include "language-select.h"

/**
 * Boot language selector.
 *
 * Runs after the health & safety / Nintendo / IS logos and goes straight to
 * the title screen. Six flags in a 2x3 grid: the focused flag is full colour
 * with a glowing gold frame, grows and floats; the others are darkened.
 * The choice is kept in the global save header (GlobalSaveInfo.unk10, unused
 * by vanilla) and preselected on the next boot.
 */

extern struct ProcCmd CONST_DATA ProcScr_GameEarlyStartUI[];
extern const u8 Img_LanguageSelectFlags[];

/* lang + 1; 0 = not read from SRAM yet */
extern u8 gKernelLanguage;

enum {
	LANGSEL_COLS = 3,
	LANGSEL_ROWS = 2,

	LANGSEL_LABEL_END = 99,

	/* Flags sheet is 256x128 (2D OBJ mapping), one 64x64 cell per flag */
	LANGSEL_OBJ_CHR = 0,
	LANGSEL_OBJ_PAL = 0, /* one OBJ palette per flag */
	LANGSEL_AFFINE = 0, /* one affine slot per flag */

	LANGSEL_SCALE_IDLE = 0x100,
	LANGSEL_SCALE_FOCUS = 0x140,
	LANGSEL_SCALE_CONFIRM = 0x168,

	LANGSEL_LIGHT_MAX = 16,
	LANGSEL_FADE_FRAMES = 16,
	LANGSEL_CONFIRM_FRAMES = 24,

	/* Flag sheet palette slots for the two-pixel frame */
	LANGSEL_COLOR_FRAME_OUTER = 1,
	LANGSEL_COLOR_FRAME_INNER = 2,
};

struct LanguageSelectProc {
	PROC_HEADER;

	u8 cursor;
	u8 light[KERNEL_LANG_MAX];
	s16 scale[KERNEL_LANG_MAX];
	s16 clock;
	s16 timer;
};

static const u8 sFlagCenterX[LANGSEL_COLS] = { 48, 120, 192 };
static const u8 sFlagCenterY[LANGSEL_ROWS] = { 52, 108 };

/* Matches the indexed palette of Img_LanguageSelectFlags.png */
static const u16 sFlagPalette[0x10] = {
	RGB(31, 0, 31), /* transparent */
	RGB(3, 3, 4),   /* outer frame */
	RGB(7, 7, 8),   /* inner frame */
	RGB(31, 31, 31),
	RGB(27, 4, 6),  /* red */
	RGB(2, 5, 14),  /* navy */
	RGB(0, 9, 22),  /* blue */
	RGB(3, 3, 3),   /* black */
	RGB(31, 25, 0), /* gold */
	RGB(0, 17, 9),  /* green */
	RGB(20, 3, 5),  /* coat red */
	RGB(25, 17, 3), /* coat gold */
	RGB(22, 22, 23), /* silver */
	RGB(0, 0, 0),
	RGB(0, 0, 0),
	RGB(0, 0, 0),
};

#define FRAME_GLOW_LO RGB(31, 25, 6)
#define FRAME_GLOW_HI RGB(31, 31, 26)
#define FRAME_GLOW_INNER RGB(20, 13, 1)

int GetKernelLanguage(void)
{
	struct GlobalSaveInfo info;
	int lang;

	if (gKernelLanguage == 0 || gKernelLanguage > KERNEL_LANG_MAX) {
		lang = KERNEL_LANG_ENGLISH;

		if (ReadGlobalSaveInfo(&info) && info.unk10 >= 1 && info.unk10 <= KERNEL_LANG_MAX)
			lang = info.unk10 - 1;

		gKernelLanguage = lang + 1;
	}

	return gKernelLanguage - 1;
}

void SetKernelLanguage(int lang)
{
	struct GlobalSaveInfo info;

	if (lang < 0 || lang >= KERNEL_LANG_MAX)
		lang = KERNEL_LANG_ENGLISH;

	gKernelLanguage = lang + 1;

	if (!ReadGlobalSaveInfo(&info))
		return;

	if (info.unk10 == lang + 1)
		return;

	info.unk10 = lang + 1;
	WriteGlobalSaveInfo(&info);
}

static int MixColor(int a, int b, int t, int t_max)
{
	int r = RED_VALUE(a) + (RED_VALUE(b) - RED_VALUE(a)) * t / t_max;
	int g = GREEN_VALUE(a) + (GREEN_VALUE(b) - GREEN_VALUE(a)) * t / t_max;
	int bl = BLUE_VALUE(a) + (BLUE_VALUE(b) - BLUE_VALUE(a)) * t / t_max;

	return RGB(r, g, bl);
}

/* level: 0x100 = untouched */
static int ShadeColor(int color, int level)
{
	return RGB(
		RED_VALUE(color) * level >> 8,
		GREEN_VALUE(color) * level >> 8,
		BLUE_VALUE(color) * level >> 8);
}

static void LanguageSelect_UpdatePalettes(struct LanguageSelectProc *proc)
{
	int i, j;

	/* 0..0x1000, pulses the focused frame between gold and near-white */
	int glow = (SIN(proc->clock * 4) + 0x1000) / 2;

	for (i = 0; i < KERNEL_LANG_MAX; i++) {
		u16 *pal = gPaletteBuffer + 0x100 + (LANGSEL_OBJ_PAL + i) * 0x10;
		int light = proc->light[i];
		int level = 0x80 + light * 8; /* 50% when idle, 100% when focused */

		for (j = 1; j < 0x10; j++)
			pal[j] = ShadeColor(sFlagPalette[j], level);

		pal[LANGSEL_COLOR_FRAME_OUTER] = ShadeColor(
			MixColor(
				sFlagPalette[LANGSEL_COLOR_FRAME_OUTER],
				MixColor(FRAME_GLOW_LO, FRAME_GLOW_HI, glow, 0x1000),
				light, LANGSEL_LIGHT_MAX),
			level);

		pal[LANGSEL_COLOR_FRAME_INNER] = ShadeColor(
			MixColor(sFlagPalette[LANGSEL_COLOR_FRAME_INNER], FRAME_GLOW_INNER, light, LANGSEL_LIGHT_MAX),
			level);
	}

	EnablePaletteSync();
}

static void LanguageSelect_DrawFlag(struct LanguageSelectProc *proc, int i)
{
	int scale = proc->scale[i];
	int row = i / LANGSEL_COLS;
	int x = sFlagCenterX[i - row * LANGSEL_COLS];
	int y = sFlagCenterY[row];
	int inv = 0x10000 / scale;
	int chr = LANGSEL_OBJ_CHR + (i >> 2) * 0x100 + (i & 3) * 8;

	/* Lift with the zoom, then float gently while focused */
	y -= (scale - LANGSEL_SCALE_IDLE) / 0x10;
	y += (SIN(proc->clock * 3) * 2 * proc->light[i]) >> 16;

	int oam2 = OAM2_CHR(chr) | OAM2_PAL(LANGSEL_OBJ_PAL + i) | OAM2_LAYER(1);

	/*
	 * A double-size affine 64x64 costs ~4x the OBJ line time of a plain one.
	 * Six of them overflow the per-scanline budget and drop flags, so only
	 * flags that are actually zoomed go through the affine path.
	 */
	if (scale == LANGSEL_SCALE_IDLE) {
		PutSpriteExt(4, OAM1_X(x - 32), OAM0_Y(y - 32), gObject_64x64, oam2);
		return;
	}

	SetObjAffine(LANGSEL_AFFINE + i, inv, 0, 0, inv);

	/* Double-size box is 128x128, centred on the flag */
	PutSpriteExt(
		4,
		OAM1_X(x - 64) | OAM1_AFFINE_ID(LANGSEL_AFFINE + i),
		OAM0_Y(y - 64) | OAM0_AFFINE_ENABLE | OAM0_DOUBLESIZE,
		gObject_64x64,
		oam2);
}

static void LanguageSelect_Animate(struct LanguageSelectProc *proc)
{
	int i;

	proc->clock++;

	for (i = 0; i < KERNEL_LANG_MAX; i++) {
		bool focused = (i == proc->cursor);
		int target = focused ? LANGSEL_SCALE_FOCUS : LANGSEL_SCALE_IDLE;
		int diff;

		/* Confirm: a quick pop before settling back */
		if (focused && proc->timer > LANGSEL_CONFIRM_FRAMES - 6)
			target = LANGSEL_SCALE_CONFIRM;

		diff = target - proc->scale[i];

		if (diff / 3 != 0)
			proc->scale[i] += diff / 3;
		else if (diff != 0)
			proc->scale[i] += diff > 0 ? 1 : -1;

		if (focused && proc->light[i] < LANGSEL_LIGHT_MAX)
			proc->light[i] += 2;
		else if (!focused && proc->light[i] > 0)
			proc->light[i] -= 2;
	}

	LanguageSelect_UpdatePalettes(proc);

	/* Focused flag last so it sits on top of its neighbours */
	for (i = 0; i < KERNEL_LANG_MAX; i++)
		if (i != proc->cursor)
			LanguageSelect_DrawFlag(proc, i);

	LanguageSelect_DrawFlag(proc, proc->cursor);
}

static void LanguageSelect_Init(struct LanguageSelectProc *proc)
{
	int i;

	if (!gpKernelDesignerConfig->language_select) {
		Proc_Goto(proc, LANGSEL_LABEL_END);
		return;
	}

	SetupBackgrounds(NULL);
	SetDispEnable(0, 0, 0, 1, 1);
	gLCDControlBuffer.dispcnt.obj1dMap = 0;

	Decompress(Img_LanguageSelectFlags, OBJ_CHR_ADDR(LANGSEL_OBJ_CHR));

	/* Sliding wall used by the save and sound room menus, dimmed so the flags stand out */
	StartMuralBackground(proc, NULL, 0xE);

	for (i = 0xE * 0x10; i < 0x10 * 0x10; i++)
		gPaletteBuffer[i] = ShadeColor(gPaletteBuffer[i], 0x70);

	BG_EnableSyncByMask(BG3_SYNC_BIT);

	SetBlendTargetA(1, 1, 1, 1, 1);
	SetBlendBackdropA(1);
	SetBlendDarken(0x10);

	proc->cursor = GetKernelLanguage();
	proc->clock = 0;
	proc->timer = 0;

	for (i = 0; i < KERNEL_LANG_MAX; i++) {
		proc->light[i] = (i == proc->cursor) ? LANGSEL_LIGHT_MAX : 0;
		proc->scale[i] = (i == proc->cursor) ? LANGSEL_SCALE_FOCUS : LANGSEL_SCALE_IDLE;
	}

	LanguageSelect_Animate(proc);
}

static void LanguageSelect_FadeIn(struct LanguageSelectProc *proc)
{
	LanguageSelect_Animate(proc);

	proc->timer++;
	SetBlendDarken(LANGSEL_FADE_FRAMES - proc->timer);

	if (proc->timer >= LANGSEL_FADE_FRAMES) {
		SetBlendNone();
		proc->timer = 0;
		Proc_Break(proc);
	}
}

static void LanguageSelect_Idle(struct LanguageSelectProc *proc)
{
	u16 keys = gKeyStatusPtr->repeatedKeys;
	int row = proc->cursor / LANGSEL_COLS;
	int col = proc->cursor - row * LANGSEL_COLS;

	if (gKeyStatusPtr->newKeys & (A_BUTTON | START_BUTTON)) {
		PlaySoundEffect(SONG_SE_SYS_WINDOW_SELECT1);
		proc->timer = LANGSEL_CONFIRM_FRAMES;
		LanguageSelect_Animate(proc);
		Proc_Break(proc);
		return;
	}

	/* No '%': the kernel links __aeabi_idivmod to vanilla __modsi3, which drops the remainder in r1 */
	if (keys & DPAD_LEFT)
		col = (col == 0) ? LANGSEL_COLS - 1 : col - 1;
	else if (keys & DPAD_RIGHT)
		col = (col == LANGSEL_COLS - 1) ? 0 : col + 1;
	else if (keys & (DPAD_UP | DPAD_DOWN))
		row = (row == 0) ? LANGSEL_ROWS - 1 : row - 1;

	if (row * LANGSEL_COLS + col != proc->cursor) {
		proc->cursor = row * LANGSEL_COLS + col;
		PlaySoundEffect(SONG_SE_SYS_CURSOR_UD1);
	}

	LanguageSelect_Animate(proc);
}

static void LanguageSelect_Confirm(struct LanguageSelectProc *proc)
{
	proc->timer--;
	LanguageSelect_Animate(proc);

	if (proc->timer <= 0)
		Proc_Break(proc);
}

static void LanguageSelect_FadeOut(struct LanguageSelectProc *proc)
{
	proc->timer++;
	SetBlendDarken(proc->timer);
	LanguageSelect_Animate(proc);

	if (proc->timer >= LANGSEL_FADE_FRAMES)
		Proc_Break(proc);
}

static void LanguageSelect_End(struct LanguageSelectProc *proc)
{
	SetKernelLanguage(proc->cursor);

	/* Skip the opening cutscene on boot; the title screen still plays it when left idle */
	Proc_Goto(proc->proc_parent, LGAMECTRL_TITLE_DIRECT);
}

/* Replaces ProcScr_GameEarlyStartUI as the first child of gProcScr_GameControl */
const struct ProcCmd ProcScr_LanguageSelect[] = {
	PROC_NAME("LANGSEL"),
	PROC_START_CHILD_BLOCKING(ProcScr_GameEarlyStartUI),

	PROC_CALL(LanguageSelect_Init),
	PROC_REPEAT(LanguageSelect_FadeIn),
	PROC_REPEAT(LanguageSelect_Idle),
	PROC_REPEAT(LanguageSelect_Confirm),
	PROC_REPEAT(LanguageSelect_FadeOut),
	PROC_CALL(LanguageSelect_End),

PROC_LABEL(LANGSEL_LABEL_END),
	PROC_END,
};
