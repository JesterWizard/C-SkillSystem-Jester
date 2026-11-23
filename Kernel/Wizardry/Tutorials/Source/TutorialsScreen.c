#include "common-chax.h"
#include "kernel-lib.h"
#include "savemenu.h"
#include "hardware.h"
#include "fontgrp.h"
#include "uiutils.h"
#include "ctc.h"
#include "bmshop.h"
#include "tutorials.h"
#include "TutorialTitles.h"
#include "constants/songs.h"
#include "constants/texts.h"

/*
 * Extras > Tutorials (Documentation/Features/Tutorials.md): Select Mode's screen (vanilla's InitDifficultySelectScreen
 * and DrawDifficultySprites_Loop, rebuilt here), with the tutorials' titles on its banners, in the Extras labels'
 * letters (Tools/tutorial-titles/titles.py), and the chosen one's text in its box. Up and down choose a tutorial (three banners show; the list scrolls under them, with arrows
 * when there is more, and goes around at either end). A text longer than the box shows a page at a time: A or right
 * shows the next page, left the previous one. B goes back to Extras.
 */

/* Vanilla's Select Mode data (the rest is in FE-CLib: the palettes, the banner, the hand, sub_80AC4F8 the glow) */
extern const u8 Img_DifficultyMenuObjs[];
extern const u16 Pal_DifficultyMenuObjs[];
extern const u8 gUnknown_08A29558[];    /* the box's tilemap */
extern const u16 gSprite_DifficultyMenuSelectModeBg[];

/* The Extras labels, for "Tutorials" on the header (Kernel/Wizardry/NewGamePlus) */
extern const u8 Img_NgpMenuObjs[];

#define VISIBLE         3
#define BANNER_X        8
#define BANNER_Y        48
#define BANNER_STEP     32
/*
 * The banners in the home menu's colours (the save menu's main buttons): the chosen one in the highlighted button's
 * palette (Pal_SaveScreenSprits' first, its outline pulsing as SaveMenuUpdateCursorPalette does), the others in the
 * plain buttons' (its fifth). Select Mode's banner uses the same colour slots.
 */
#define BANNER_PAL_ON   13
#define BANNER_PAL_OFF  14
#define TITLE_CHR       0x2C0   /* the three titles' images (128x16 each, two rows of 16 tiles), under the labels' sheet */
#define TITLE_ROW_BYTES (16 * 0x20)
#define BOX_LINES       5
#define BOX_TILES       14
#define PAGE_LINES      4       /* a text of pages: the box's last line is the page count */
#define TEXT_PX         88      /* the box's inside, from its first text column */
#define TUT_MAX_LINES       48      /* a tutorial's lines, 12 pages */

#define LABEL_CHR       0x200   /* the Extras labels' sheet, as the Extras list loads it */
#define LABEL_PAL       11      /* OBJ palette for the header's label: Select Mode leaves 11-15 free */
#define ARROW_CHR       0x180   /* the scroll arrows (UnpackUiVArrowGfx): OBJ tiles Select Mode leaves free */
#define ARROW_PAL       12
#define ARROW_X         (BANNER_X + 60)

struct TutorialsScreen {
	struct Text lines[BOX_LINES];
	u16 count;
	u16 cursor;
	u16 top;
	u8 page;
	u8 pages;
	u8 lineCount;
	u16 lineStart[TUT_MAX_LINES];   /* where each wrapped line starts in the tutorial's text */
	u8 clock;
	s8 handFrom;    /* the slot the hand slides from */
	u8 handTime;    /* frames of the slide left */
	bool launch;    /* A: the chosen tutorial's map, after the fade */
};

extern u8 gBestiaryScreen[];
#define TS ((struct TutorialsScreen *)gBestiaryScreen)
_Static_assert(sizeof(struct TutorialsScreen) <= 0x160, "the screen fits in gBestiaryScreen (0x160 bytes)");

struct ProcTutorials {
	PROC_HEADER;
	struct DispCnt dispBak;
};

enum { L_TUT_MAIN = 1, L_TUT_EXIT };

/* "Tutorials", the Extras list's label (Sprite_TutorialsExtrasLabel), centred on the header (8 down, as Select Mode's) */
static const u16 Sprite_TutorialsHeaderLabel[] = {
	3,
	0x4000, 0x8000 + 32, 0x0A1A,
	0x0000, 0x4000 + 64, 0x0A1E,
	0x0000, 0x4000 + 80, 0x0A4C,
};

/* A title: 128x16, four 32x16 pieces, over the banner (priority 1) */
static const u16 Sprite_TutorialTitle[] = {
	4,
	0x4000, 0x8000 + 0, 0x0400 + 0,
	0x4000, 0x8000 + 32, 0x0400 + 4,
	0x4000, 0x8000 + 64, 0x0400 + 8,
	0x4000, 0x8000 + 96, 0x0400 + 12,
};

/* The visible titles' images, into their sprites' tiles (2D mapping: a slot's second row is 32 tiles on) */
static void Tutorials_DrawTitles(void)
{
	int slot, i;

	for (slot = 0; slot < VISIBLE; slot++) {
		u8 *dst = (u8 *)OBJ_VRAM0 + (TITLE_CHR + slot * 0x40) * 0x20;

		i = TS->top + slot;
		if (i < TS->count) {
			const u8 *img = gTutorials[Tutorials_Index(i)].img;

			CpuFastCopy(img, dst, TITLE_ROW_BYTES);
			CpuFastCopy(img + TITLE_ROW_BYTES, dst + 32 * 0x20, TITLE_ROW_BYTES);
		} else {
			CpuFastFill(0, dst, TITLE_ROW_BYTES);
			CpuFastFill(0, dst + 32 * 0x20, TITLE_ROW_BYTES);
		}
	}
}

/* The tutorial's text, decoded, with control bytes ([LF] and the like) as spaces */
static const char *Tutorials_Text(void)
{
	static char *const buf = (char *)gGenericBuffer;
	const char *src = GetStringFromIndex(gTutorials[Tutorials_Index(TS->cursor)].text);
	int i;

	for (i = 0; src[i] != '\0' && i < 0x7FF; i++)
		buf[i] = (u8)src[i] < 0x20 ? ' ' : src[i];
	buf[i] = '\0';
	return buf;
}

/* The width of src's first n bytes */
static int Tutorials_Width(const char *src, int n)
{
	char line[0x60];

	if (n > (int)sizeof(line) - 1)
		n = sizeof(line) - 1;
	memcpy(line, src, n);
	line[n] = '\0';
	return GetStringTextLen(line);
}

/* Where each line starts: words go on a line while it fits in TEXT_PX */
static void Tutorials_Wrap(void)
{
	const char *text = Tutorials_Text();
	int pos = 0, start, end, next;

	TS->lineCount = 0;

	while (text[pos] != '\0' && TS->lineCount < TUT_MAX_LINES) {
		while (text[pos] == ' ')
			pos++;
		if (text[pos] == '\0')
			break;

		start = end = pos;
		for (;;) {
			next = end;
			while (text[next] == ' ')
				next++;
			while (text[next] != '\0' && text[next] != ' ')
				next++;
			if (end != start && Tutorials_Width(text + start, next - start) > TEXT_PX)
				break;
			end = next;
			if (text[end] == '\0')
				break;
		}

		TS->lineStart[TS->lineCount++] = start;
		pos = end;
	}

	/* Four lines a page: the box's last line is "A: Try it", and the page count if there are more pages */
	for (TS->pages = 0, start = 0; start < TS->lineCount; start += PAGE_LINES)
		TS->pages++;
	if (TS->pages == 0)
		TS->pages = 1;
}

/* The page's four lines; the last line is "A: Try it", and the page count if there are more pages */
static void Tutorials_DrawPage(void)
{
	const char *text = Tutorials_Text();
	char buf[0x60];
	char num[8];
	int line, i, from, to, n;
	int per = PAGE_LINES;

	for (line = 0; line < BOX_LINES; line++)
		ClearText(&TS->lines[line]);

	for (line = 0; line < per; line++) {
		i = TS->page * per + line;
		if (i >= TS->lineCount)
			break;
		from = TS->lineStart[i];
		to = i + 1 < TS->lineCount ? TS->lineStart[i + 1] : from + (int)strlen(text + from);
		while (to > from && text[to - 1] == ' ')
			to--;
		n = to - from < (int)sizeof(buf) - 1 ? to - from : (int)sizeof(buf) - 1;
		memcpy(buf, text + from, n);
		buf[n] = '\0';
		Text_InsertDrawString(&TS->lines[line], 0, TEXT_COLOR_SYSTEM_WHITE, buf);
	}

	Text_InsertDrawString(&TS->lines[PAGE_LINES], 0, TEXT_COLOR_SYSTEM_GOLD, GetStringFromIndex(MSG_TUT_TRY));

	if (TS->pages > 1) {
		i = 0;
		num[i++] = '1' + TS->page;
		num[i++] = '/';
		num[i++] = '0' + TS->pages;
		num[i] = 0;
		Text_InsertDrawString(&TS->lines[PAGE_LINES], TEXT_PX - GetStringTextLen(num), TEXT_COLOR_SYSTEM_GOLD, num);
	}

	for (line = 0; line < BOX_LINES; line++)
		PutText(&TS->lines[line], gBG0TilemapBuffer + TILEMAP_INDEX(18, 7 + line * 2));
	BG_EnableSyncByMask(BG0_SYNC_BIT);
}

static void Tutorials_Select(int i)
{
	TS->cursor = i;
	TS->page = 0;
	Tutorials_Wrap();
	Tutorials_DrawPage();
}

static void Tutorials_Init(struct ProcTutorials *proc)
{
	int i;

	proc->dispBak = gLCDControlBuffer.dispcnt;
	CpuFill16(0, TS, sizeof(*TS));
	TS->count = Tutorials_Count();
	/* Back from a tutorial's map: on that tutorial (the list shows only this game's: Tutorials_Shown) */
	if (Tutorials_Shown(TutorialMap_Played()) >= 0)
		TS->cursor = Tutorials_Shown(TutorialMap_Played());
	if (TS->cursor > VISIBLE - 1)
		TS->top = TS->cursor - (VISIBLE - 1);
	TS->handFrom = -1;

	/* As vanilla's InitDifficultySelectScreen */
	ResetTextFont();
	LoadUiFrameGraphics();
	ResetText();
	for (i = 0; i < BOX_LINES; i++)
		InitText(&TS->lines[i], BOX_TILES);

	CopyToPaletteBuffer(Pal_SaveMenuBG, 0x100, 0x100);
	CopyToPaletteBuffer(Pal_MainMenuBgFog, 0xE0, 0x20);
	CopyToPaletteBuffer(Pal_SaveScreenSprits, 0x240, 0x100);
	CopyToPaletteBuffer(Pal_08A295B4, 0x40, 0x20);
	Decompress(Img_DifficultyMenuObjs, (void *)0x06010800);
	CopyToPaletteBuffer(Pal_DifficultyMenuObjs, 0x220, 0x140);

	/* The header's label: the Extras labels' sheet where the Extras list has it, with its palette */
	Decompress(Img_NgpMenuObjs, (void *)(0x06010000 + LABEL_CHR * 0x20));
	CopyToPaletteBuffer(Pal_SaveScreenSprits + 0x20, 0x200 + LABEL_PAL * 0x20, 0x20); /* its third, OBJ 4 there */
	UnpackUiVArrowGfx(ARROW_CHR, ARROW_PAL);
	CopyToPaletteBuffer(Pal_SaveScreenSprits, 0x200 + BANNER_PAL_ON * 0x20, 0x20);
	CopyToPaletteBuffer(Pal_SaveScreenSprits + 4 * 0x10, 0x200 + BANNER_PAL_OFF * 0x20, 0x20);
	EnablePaletteSync();

	BG_Fill(gBG0TilemapBuffer, 0);
	BG_Fill(gBG1TilemapBuffer, 0);
	Decompress(gUnknown_08A29558, gGenericBuffer);
	CallARM_FillTileRect(gBG1TilemapBuffer + 0xD1, (void *)gGenericBuffer, 0x1000);

	gLCDControlBuffer.dispcnt.obj1dMap = 0;
	SetDispEnable(1, 1, 1, 1, 1);

	Tutorials_Select(TS->cursor);
	Tutorials_DrawTitles();
	BG_EnableSyncByMask(BG0_SYNC_BIT | BG1_SYNC_BIT);
}

static void Tutorials_PutSprites(void)
{
	int slot, y, handY, from, to;

	TS->clock++;

	PutSpriteExt(4, 56, 4, gSprite_DifficultyMenuSelectModeBg, OAM2_PAL(2));
	PutSpriteExt(4, 56, 4 + 8, Sprite_TutorialsHeaderLabel, OAM2_PAL(LABEL_PAL));

	/* The banners, in the home menu buttons' colours: only the chosen one lit */
	for (slot = 0; slot < VISIBLE && TS->top + slot < TS->count; slot++) {
		y = BANNER_Y + slot * BANNER_STEP;
		PutSpriteExt(4, BANNER_X, y, gSprite_08A20AC2,
			     OAM2_PAL(TS->top + slot == TS->cursor ? BANNER_PAL_ON : BANNER_PAL_OFF));
		PutSpriteExt(4, BANNER_X, y + 8, Sprite_TutorialTitle,
			     OAM2_CHR(TITLE_CHR + slot * 0x40) + OAM2_PAL(LABEL_PAL));
	}

	/* The lit banner's outline pulses, as the home menu's chosen button (SaveMenuUpdateCursorPalette's colours) */
	PAL_OBJ_COLOR(BANNER_PAL_ON, 1) = Pal_08A28088[(TS->clock & 0x3F) >> 2];
	EnablePaletteSync();

	/* The hand, sliding to its slot in 4 frames as vanilla's, and bobbing */
	to = (TS->cursor - TS->top) * BANNER_STEP;
	handY = to;
	if (TS->handTime != 0 && TS->handFrom >= 0) {
		from = TS->handFrom * BANNER_STEP;
		handY = to + (from - to) * TS->handTime / 4;
		TS->handTime--;
	}
	handY += BANNER_Y - 16 + (((TS->clock >> 3) & 7) < 4 ? ((TS->clock >> 3) & 7) : 7 - ((TS->clock >> 3) & 7));
	PutSpriteExt(4, 0, handY & 0xFF, gSprite_08A20A98, OAM2_PAL(3));

	/* More tutorials above or below the banners */
	if (TS->top > 0)
		DisplayUiVArrow(ARROW_X, BANNER_Y - 9, OAM2_CHR(ARROW_CHR) + OAM2_PAL(ARROW_PAL), 1);
	if (TS->top + VISIBLE < TS->count)
		DisplayUiVArrow(ARROW_X, BANNER_Y + VISIBLE * BANNER_STEP + 1, OAM2_CHR(ARROW_CHR) + OAM2_PAL(ARROW_PAL), 0);
}

static void Tutorials_Move(int to)
{
	int slot = TS->cursor - TS->top;

	if (to < 0 || to >= TS->count || to == TS->cursor)
		return;

	PlaySoundEffect(SONG_SE_SYS_CURSOR_UD1);

	if (to < TS->top)
		TS->top = to;
	else if (to > TS->top + VISIBLE - 1)
		TS->top = to - (VISIBLE - 1);

	TS->handFrom = slot;
	TS->handTime = 4;
	Tutorials_Select(to);
	Tutorials_DrawTitles();
}

static void Tutorials_Flip(int step)
{
	if (TS->pages <= 1)
		return;

	PlaySoundEffect(SONG_SE_SYS_CURSOR_LR1);
	/* No %: the division runtime's remainder is broken here (CLAUDE.md) */
	if (step > 0)
		TS->page = TS->page + 1 < TS->pages ? TS->page + 1 : 0;
	else
		TS->page = TS->page > 0 ? TS->page - 1 : TS->pages - 1;
	Tutorials_DrawPage();
}

static void Tutorials_Loop(struct ProcTutorials *proc)
{
	u16 press = gKeyStatusPtr->newKeys, repeat = gKeyStatusPtr->repeatedKeys;

	Tutorials_PutSprites();

	if (press & B_BUTTON) {
		PlaySoundEffect(SONG_SE_SYS_WINDOW_CANSEL1);
		Proc_Goto(proc, L_TUT_EXIT);
		return;
	}

	if (press & A_BUTTON) {
		PlaySoundEffect(SONG_SE_SYS_WINDOW_SELECT1);
		TS->launch = true;
		Proc_Goto(proc, L_TUT_EXIT);
		return;
	}

	if (press & DPAD_RIGHT) {
		Tutorials_Flip(1);
		return;
	}

	if (press & DPAD_LEFT) {
		Tutorials_Flip(-1);
		return;
	}

	/* Around the ends on a press, not while held */
	if (repeat & DPAD_DOWN) {
		if (TS->cursor + 1 < TS->count)
			Tutorials_Move(TS->cursor + 1);
		else if (press & DPAD_DOWN)
			Tutorials_Move(0);
	} else if (repeat & DPAD_UP) {
		if (TS->cursor > 0)
			Tutorials_Move(TS->cursor - 1);
		else if (press & DPAD_UP)
			Tutorials_Move(TS->count - 1);
	}
}

/* A fade, with the sprites kept on screen */
static void Tutorials_FadeWait(struct ProcTutorials *proc)
{
	Tutorials_PutSprites();

	if (!FadeInExists() && !FadeOutExists())
		Proc_Break(proc);
}

/* After the fade: A started the chosen tutorial's map (the save menu ends, and the game control starts it) */
static void Tutorials_Launch(struct ProcTutorials *proc)
{
	if (TS->launch)
		TutorialMap_Launch(proc->proc_parent, Tutorials_Index(TS->cursor));
}

static void Tutorials_OnEnd(struct ProcTutorials *proc)
{
	gLCDControlBuffer.dispcnt = proc->dispBak;
	BG_Fill(gBG0TilemapBuffer, 0);
	BG_Fill(gBG1TilemapBuffer, 0);
	BG_EnableSyncByMask(BG0_SYNC_BIT | BG1_SYNC_BIT);
}

static const struct ProcCmd ProcScr_Tutorials[] = {
	PROC_NAME("Tutorials"),
	PROC_SET_END_CB(Tutorials_OnEnd),
	PROC_YIELD,
	PROC_CALL(Tutorials_Init),
	PROC_CALL_ARG(NewFadeIn, 8),
	PROC_REPEAT(Tutorials_FadeWait),

	PROC_LABEL(L_TUT_MAIN),
	PROC_REPEAT(Tutorials_Loop),

	PROC_LABEL(L_TUT_EXIT),
	PROC_CALL_ARG(NewFadeOut, 8),
	PROC_REPEAT(Tutorials_FadeWait),
	PROC_CALL(Tutorials_Launch),
	PROC_END,
};

ProcPtr StartTutorialsScreen(ProcPtr parent)
{
	return Proc_StartBlocking(ProcScr_Tutorials, parent);
}

bool TutorialsScreen_Exists(void)
{
	return Proc_Find(ProcScr_Tutorials) != NULL;
}
