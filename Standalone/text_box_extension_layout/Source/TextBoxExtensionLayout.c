#include "global.h"
#include "helpbox.h"
#include "statscreen.h"
#include "fontgrp.h"
#include "hardware.h"
#include "ctc.h"
#include "bmlib.h"
#include "variables.h"
#include "scene.h"
#include "uiutils.h"
#include "soundwrapper.h"

#include <stddef.h>

extern const u8 TextBoxExtensionLayout;
extern u8 gUnknown_08A02274[];

enum {
	HELP_BOX_MODE_VANILLA = 0,
	HELP_BOX_MODE_EXTENDED = 1,
	HELP_BOX_MODE_PAGED = 2,
	HELP_BOX_PAGE_NUM_OBJPAL = 10
};

struct HelpBoxPageState {
	u8 page;
	u8 page_count;
	u8 lines_per_page;
	u8 pretext_lines;
	u8 desc_lines;
	u8 page0_desc_lpp;
	u8 later_desc_lpp;
	u8 indicator_px;
};

extern struct HelpBoxPageState sHelpBoxPageState;
extern struct Text sHelpBoxExtraText[2];

void ClearHelpBoxText_Layout(void);
void ApplyHelpBoxContentSize_Layout(struct HelpBoxProc *proc, int width, int height);

_Static_assert(offsetof(struct HelpBoxSt, text[0]) == 0x18, "help box text offset");

/* Vanilla gHelpBoxSt.oam2_base. The CLib header's text[5] moves that field. */
static u16 *HelpBoxOam2(void)
{
	return (u16 *)((char *)&gHelpBoxSt + 0x30);
}

static int LayoutMode(void)
{
	return TextBoxExtensionLayout;
}

static int ModeExtended(void)
{
	return LayoutMode() == HELP_BOX_MODE_EXTENDED;
}

static int ModePaged(void)
{
	return LayoutMode() == HELP_BOX_MODE_PAGED;
}

static void HelpBoxResetPageState(void)
{
	sHelpBoxPageState.page = 0;
	sHelpBoxPageState.page_count = 1;
	sHelpBoxPageState.lines_per_page = 3;
	sHelpBoxPageState.pretext_lines = 0;
	sHelpBoxPageState.desc_lines = 0;
	sHelpBoxPageState.page0_desc_lpp = 3;
	sHelpBoxPageState.later_desc_lpp = 3;
	sHelpBoxPageState.indicator_px = 0;
}

static int HelpBoxCountDescLines(const char *str)
{
	int lines = 0;

	if (str == NULL || *str == CHFE_L_X)
		return 0;

	while (*str != CHFE_L_X) {
		lines++;
		str = GetStringLineEnd((char *)str);
		if (*str == CHFE_L_NL)
			str++;
		else
			break;
	}

	return lines;
}

static int HelpBoxLinesPerPageForPretext(int pretext)
{
	int lines = 3 - pretext;

	if (lines < 1)
		lines = 1;

	return lines;
}

static int HelpBoxCalcPageCount(void)
{
	int remaining = sHelpBoxPageState.desc_lines;
	int pages = 0;

	if (remaining <= 0)
		return 1;

	remaining -= sHelpBoxPageState.page0_desc_lpp;
	pages = 1;

	while (remaining > 0) {
		remaining -= sHelpBoxPageState.later_desc_lpp;
		pages++;
		if (pages > 16)
			break;
	}

	return pages;
}

static void HelpBoxFinalizePageState(int pretextLines)
{
	int lines;

	if (pretextLines < 0)
		pretextLines = 0;
	if (pretextLines > 2)
		pretextLines = 2;

	lines = HelpBoxLinesPerPageForPretext(pretextLines);
	sHelpBoxPageState.pretext_lines = pretextLines;
	sHelpBoxPageState.lines_per_page = lines;

	if (!ModePaged()) {
		sHelpBoxPageState.page_count = 1;
		return;
	}

	if (sHelpBoxPageState.page == 0) {
		sHelpBoxPageState.page0_desc_lpp = lines;
		if (sHelpBoxPageState.page0_desc_lpp > sHelpBoxPageState.desc_lines)
			sHelpBoxPageState.page0_desc_lpp = sHelpBoxPageState.desc_lines;
		sHelpBoxPageState.later_desc_lpp = lines;
		sHelpBoxPageState.page_count = HelpBoxCalcPageCount();
	}

	if (sHelpBoxPageState.page_count < 1)
		sHelpBoxPageState.page_count = 1;
	if (sHelpBoxPageState.page >= sHelpBoxPageState.page_count)
		sHelpBoxPageState.page = 0;
}

static int HelpBoxDescLinesToSkip(void)
{
	int page = sHelpBoxPageState.page;

	if (!ModePaged() || page == 0)
		return 0;

	return sHelpBoxPageState.page0_desc_lpp +
		(page - 1) * sHelpBoxPageState.later_desc_lpp;
}

static const char *HelpBoxSkipDescLines(const char *str, int linesToSkip)
{
	if (str == NULL)
		return str;

	while (linesToSkip > 0 && *str != CHFE_L_X) {
		str = GetStringLineEnd((char *)str);
		if (*str == CHFE_L_NL) {
			str++;
			linesToSkip--;
		} else {
			break;
		}
	}

	return str;
}

static const u16 sHelpBoxPageNumPal[16] = {
	0x67F8, 0x47DF, 0x6F37, 0x4E30,
	0x212A, 0x3BBE, 0x1656, 0x19B1,
	0x11FF, 0x7DE2, 0x7715, 0x6238,
	0x0AE3, 0x4BEF, 0x594A, 0x0000
};

static void HelpBoxDrawPageIndicator(void)
{
	if (!ModePaged() || sHelpBoxPageState.page_count <= 1) {
		sHelpBoxPageState.indicator_px = 0;
		return;
	}

	ApplyPalette(sHelpBoxPageNumPal, HELP_BOX_PAGE_NUM_OBJPAL + 0x10);
	sHelpBoxPageState.indicator_px = 22;
}

static void HelpBoxPutPageIndicatorSprites(int boxX, int boxY, int boxW)
{
	int x;
	int oam2;
	int cur;
	int tot;

	if (!ModePaged() || sHelpBoxPageState.page_count <= 1)
		return;

	cur = sHelpBoxPageState.page;
	tot = sHelpBoxPageState.page_count;
	x = boxX + boxW - 24;
	if (x < boxX + 48)
		x = boxX + 48;

	oam2 = TILEREF(0x289, HELP_BOX_PAGE_NUM_OBJPAL);
	PutSprite(0, x, boxY - 0xB, gObject_8x8, oam2 + cur + 1);
	PutSprite(0, x + 7, boxY - 0xB, gObject_8x8, oam2);
	PutSprite(0, x + 14, boxY - 0xB, gObject_8x8, oam2 + tot);
}

static void HelpBoxEnsurePageNumGfx(void)
{
	if (!ModePaged())
		return;

	Decompress(gUnknown_08A02274, (void *)(OBJ_VRAM0 + 0x240 * TILE_SIZE_4BPP));
}

static void HelpBoxStoreDescLineCount(int descHeightPx)
{
	int lines = descHeightPx / 0x10;

	if (lines < 0)
		lines = 0;

	sHelpBoxPageState.desc_lines = lines;
}

static void CapPagedHeight(int *height)
{
	if (ModePaged() && *height > 0x30)
		*height = 0x30;
}

static int HelpBoxTryAdvancePage(void)
{
	struct HelpBoxProc *hb;
	int item;
	int mid;

	if (!ModePaged() || sHelpBoxPageState.page_count <= 1 ||
		!(gKeyStatusPtr->newKeys & A_BUTTON))
		return 0;

	hb = Proc_Find(gProcScr_HelpBox);
	if (hb == NULL)
		hb = Proc_Find(ProcScr_Helpbox_bug_08A01678);
	if (hb == NULL)
		return 0;

	item = hb->item;
	mid = hb->mid;
	sHelpBoxPageState.page++;
	if (sHelpBoxPageState.page >= sHelpBoxPageState.page_count)
		sHelpBoxPageState.page = 0;

	PlaySoundEffect(0x67);
	ClearHelpBoxText_Layout();
	StartHelpBoxTextInit(item, mid);
	return 1;
}

void LoadHelpBoxGfx_Layout(void *vram, int palId)
{
	if (vram == NULL)
		vram = (void *)0x06013000;

	if (palId < 0)
		palId = 5;

	palId = (palId & 0xF) + 0x10;

	Decompress(gGfx_HelpTextBox, (char *)vram + 0x360);
	Decompress(gGfx_HelpTextBox2, (char *)vram + 0x760);
	Decompress(gGfx_HelpTextBox3, (char *)vram + 0xB60);
	Decompress(gGfx_HelpTextBox4, (char *)vram + 0xF60);
	Decompress(gGfx_HelpTextBox5, (char *)vram + 0x1360);

	InitSpriteTextFont(&gHelpBoxSt.font, vram, palId);
	InitSpriteText(&gHelpBoxSt.text[0]);
	InitSpriteText(&gHelpBoxSt.text[1]);
	InitSpriteText(&gHelpBoxSt.text[2]);

	if (ModeExtended()) {
		InitSpriteText(&sHelpBoxExtraText[0]);
		InitSpriteText(&sHelpBoxExtraText[1]);
	}

	SetTextFont(0);
	ApplyPalette(Pal_HelpBox, palId);
	*HelpBoxOam2() = (((u32)vram << 0x11) >> 0x16) + (palId & 0xF) * 0x1000;
}

void DisplayHelpBoxObj_Layout(int x, int y, int w, int h, int unk)
{
	s8 flag;
	s8 flag_;
	s8 anotherFlag;
	int xCount;
	int yCount;
	int xPx;
	int yPx;
	int iy;
	int ix;
	u16 oam2 = *HelpBoxOam2();

	flag = (w + 7) & 0x10;
	anotherFlag = w & 0xF;

	if (w < 0x20)
		w = 0x20;
	if (w > 0xC0)
		w = 0xC0;
	if (h < 0x10)
		h = 0x10;

	if (ModeExtended()) {
		if (h > 0x50)
			h = 0x50;
	} else if (h > 0x30) {
		h = 0x30;
	}

	xCount = (w + 0x1F) / 0x20;
	yCount = (h + 0x0F) / 0x10;
	flag_ = flag;

	for (ix = xCount - 1; ix >= 0; ix--) {
		for (iy = yCount; iy >= 0; iy--) {
			yPx = (iy + 1) * 0x10;
			if (yPx > h)
				yPx = h;
			yPx -= 0x10;

			xPx = (ix + 1) * 0x20;
			if (flag_ != 0) {
				xPx -= 0x20;
				PutSprite(0, x + xPx, y + yPx, gObject_16x16,
					oam2 + ix * 4 + iy * 0x40);
			} else {
				if (xPx > w)
					xPx = w;
				xPx -= 0x20;
				PutSprite(0, x + xPx, y + yPx, gObject_32x16,
					oam2 + ix * 4 + iy * 0x40);
			}
		}
		flag_ = 0;
	}

	flag_ = flag;
	for (ix = xCount - 1; ix >= 0; ix--) {
		xPx = (ix + 1) * 0x20;
		if (flag_ != 0) {
			xPx -= 0x20;
			PutSprite(0, x + xPx, y - 8, gObject_16x8, oam2 + 0x1B);
			PutSprite(0, x + xPx, y + h, gObject_16x8, oam2 + 0x3B);
			flag_ = 0;
		} else {
			if (xPx > w)
				xPx = w;
			xPx -= 0x20;
			PutSprite(0, x + xPx, y - 8, gObject_32x8, oam2 + 0x1B);
			PutSprite(0, x + xPx, y + h, gObject_32x8, oam2 + 0x3B);
		}
	}

	for (iy = yCount; iy >= 0; iy--) {
		yPx = (iy + 1) * 0x10;
		if (yPx > h)
			yPx = h;
		yPx -= 0x10;

		PutSprite(0, x - 8, y + yPx, gObject_8x16, oam2 + 0x5F);
		PutSprite(0, x + w, y + yPx, gObject_8x16, oam2 + 0x1F);
		if (anotherFlag != 0)
			PutSprite(0, x + w - 8, y + yPx, gObject_8x16, oam2 + 0x1A);
	}

	PutSprite(0, x - 8, y - 8, gObject_8x8, oam2 + 0x5B);
	PutSprite(0, x + w, y - 8, gObject_8x8, oam2 + 0x5C);
	PutSprite(0, x - 8, y + h, gObject_8x8, oam2 + 0x5D);
	PutSprite(0, x + w, y + h, gObject_8x8, oam2 + 0x5E);

	if (anotherFlag != 0) {
		PutSprite(0, x + w - 8, y - 8, gObject_8x8, oam2 + 0x1B);
		PutSprite(0, x + w - 8, y + h, gObject_8x8, oam2 + 0x3B);
	}

	if (unk == 0)
		PutSprite(0, x, y - 0xB, gObject_32x16, (0x3FF & oam2) + 0x7B);

	HelpBoxPutPageIndicatorSprites(x, y, w);
}

void HelpBoxTextScroll_OnLoop_Layout(struct HelpBoxScrollProc *proc)
{
	int i;
	int maxLine;
	int textLimit;

	proc->step--;
	if (proc->step > 0)
		return;

	proc->step = proc->speed;
	SetTextFont(proc->font);
	SetTextFontGlyphs(1);

	textLimit = ModeExtended() ? 5 : 3;
	maxLine = textLimit;
	if (ModePaged()) {
		maxLine = proc->unk_64 + sHelpBoxPageState.lines_per_page;
		if (maxLine > 3)
			maxLine = 3;
	}

	for (i = 0; i < proc->chars_per_step; i++) {
		switch (*proc->string) {
		case CHFE_L_X:
			Proc_Break(proc);
			goto scroll_end;

		case CHFE_L_NL:
			proc->string++;
			proc->pretext_lines++;
			if (proc->pretext_lines >= maxLine) {
				Proc_Break(proc);
				goto scroll_end;
			}
			continue;

		case CHFE_L_Pause8:
			proc->string++;
			continue;

		default:
			if (proc->pretext_lines < 0 || proc->pretext_lines >= textLimit ||
				proc->texts[proc->pretext_lines] == NULL) {
				Proc_Break(proc);
				goto scroll_end;
			}
			proc->string = Text_DrawCharacter(proc->texts[proc->pretext_lines], proc->string);
			continue;
		}
	}

scroll_end:
	SetTextFont(0);
}

void HelpBoxIntroDrawTexts_Layout(struct ProcHelpBoxIntro *proc)
{
	struct HelpBoxScrollProc *otherProc;
	int textSpeed;
	const char *string;

	SetTextFont(&gHelpBoxSt.font);
	SetTextFontGlyphs(1);
	Text_SetColor(&gHelpBoxSt.text[0], 6);
	Text_SetColor(&gHelpBoxSt.text[1], 6);
	Text_SetColor(&gHelpBoxSt.text[2], 6);

	if (ModeExtended()) {
		Text_SetColor(&sHelpBoxExtraText[0], 6);
		Text_SetColor(&sHelpBoxExtraText[1], 6);
	}

	GetStringFromIndex(proc->msg);
	string = StringInsertSpecialPrefixByCtrl();

	if (ModePaged() && sHelpBoxPageState.page == 0)
		sHelpBoxPageState.desc_lines = HelpBoxCountDescLines(string);

	HelpBoxFinalizePageState(proc->pretext_lines);
	HelpBoxDrawPageIndicator();

	SetTextFont(&gHelpBoxSt.font);
	SetTextFontGlyphs(1);
	Text_SetCursor(&gHelpBoxSt.text[0], 0);
	Text_SetCursor(&gHelpBoxSt.text[1], 0);
	Text_SetCursor(&gHelpBoxSt.text[2], 0);
	Text_SetColor(&gHelpBoxSt.text[0], 6);
	Text_SetColor(&gHelpBoxSt.text[1], 6);
	Text_SetColor(&gHelpBoxSt.text[2], 6);
	if (ModeExtended()) {
		Text_SetCursor(&sHelpBoxExtraText[0], 0);
		Text_SetCursor(&sHelpBoxExtraText[1], 0);
		Text_SetColor(&sHelpBoxExtraText[0], 6);
		Text_SetColor(&sHelpBoxExtraText[1], 6);
	}

	Proc_EndEach(gProcScr_HelpBoxTextScroll);
	otherProc = Proc_Start(gProcScr_HelpBoxTextScroll, PROC_TREE_3);
	otherProc->font = &gHelpBoxSt.font;
	otherProc->texts[0] = &gHelpBoxSt.text[0];
	otherProc->texts[1] = &gHelpBoxSt.text[1];
	otherProc->texts[2] = &gHelpBoxSt.text[2];
	otherProc->texts[3] = ModeExtended() ? &sHelpBoxExtraText[0] : NULL;
	otherProc->texts[4] = ModeExtended() ? &sHelpBoxExtraText[1] : NULL;
	otherProc->pretext_lines = proc->pretext_lines;
	otherProc->unk_64 = proc->pretext_lines;

	if (ModePaged())
		string = HelpBoxSkipDescLines(string, HelpBoxDescLinesToSkip());

	otherProc->string = string;
	otherProc->chars_per_step = 1;
	otherProc->step = 0;

	textSpeed = gPlaySt.config.textSpeed;
	switch (gPlaySt.config.textSpeed) {
	case 0:
		otherProc->speed = 2;
		break;
	case 1:
		otherProc->speed = textSpeed;
		break;
	case 2:
		otherProc->speed = 1;
		otherProc->chars_per_step = textSpeed;
		break;
	case 3:
		otherProc->speed = 0;
		otherProc->chars_per_step = 0x7F;
		break;
	}

	SetTextFont(0);
}

void ClearHelpBoxText_Layout(void)
{
	SetTextFont(&gHelpBoxSt.font);
	SpriteText_DrawBackground(&gHelpBoxSt.text[0]);
	SpriteText_DrawBackground(&gHelpBoxSt.text[1]);
	SpriteText_DrawBackground(&gHelpBoxSt.text[2]);

	if (ModeExtended()) {
		SpriteText_DrawBackground(&sHelpBoxExtraText[0]);
		SpriteText_DrawBackground(&sHelpBoxExtraText[1]);
	}

	Proc_EndEach(gProcScr_HelpBoxTextScroll);
	Proc_EndEach(ProcScr_HelpBoxIntro);
	SetTextFont(0);
}

void StartHelpBoxExt_Layout(const struct HelpBoxInfo *info, int unk)
{
	struct HelpBoxProc *proc;
	int wContent, hContent;

	proc = (void *)Proc_Find(gProcScr_HelpBox);
	if (proc == NULL) {
		proc = (void *)Proc_Start(gProcScr_HelpBox, PROC_TREE_3);
		proc->unk52 = unk;
		SetHelpBoxInitPosition(proc, info->xDisplay, info->yDisplay);
		ResetHelpBoxInitSize(proc);
	} else {
		proc->xBoxInit = proc->xBox;
		proc->yBoxInit = proc->yBox;
		proc->wBoxInit = proc->wBox;
		proc->hBoxInit = proc->hBox;
	}

	proc->info = info;
	proc->timer = 0;
	proc->timerMax = 12;
	proc->item = 0;
	proc->mid = info->mid;

	HelpBoxResetPageState();
	HelpBoxEnsurePageNumGfx();

	if (proc->info->populate)
		proc->info->populate(proc);

	SetTextFontGlyphs(1);
	GetStringTextBox(GetStringFromIndex(proc->mid), &wContent, &hContent);
	SetTextFontGlyphs(0);
	HelpBoxStoreDescLineCount(hContent);
	CapPagedHeight(&hContent);

	ApplyHelpBoxContentSize_Layout(proc, wContent, hContent);
	ApplyHelpBoxPosition(proc, info->xDisplay, info->yDisplay);
	ClearHelpBoxText_Layout();
	StartHelpBoxTextInit(proc->item, proc->mid);
	sLastHbi = info;
}

void sub_808A200_Layout(const struct HelpBoxInfo *info)
{
	int wTextBox;
	int hTextBox;
	struct HelpBoxProc *proc = Proc_Find(ProcScr_Helpbox_bug_08A01678);

	if (proc == NULL) {
		proc = Proc_Start(ProcScr_Helpbox_bug_08A01678, PROC_TREE_3);
		PlaySoundEffect(0x70);
		sub_808A43C(proc, info->xDisplay, info->yDisplay);
		SetHelpBoxDefaultRect(proc);
	} else {
		proc->xBoxInit = proc->xBox;
		proc->yBoxInit = proc->yBox;
		proc->wBoxInit = proc->wBoxFinal;
		proc->hBoxInit = proc->hBoxFinal;
	}

	proc->info = info;
	proc->timer = 0;
	proc->timerMax = 12;
	proc->mid = info->mid;

	HelpBoxResetPageState();
	HelpBoxEnsurePageNumGfx();

	SetTextFontGlyphs(1);
	GetStringTextBox(GetStringFromIndex(proc->mid), &wTextBox, &hTextBox);
	SetTextFontGlyphs(0);
	HelpBoxStoreDescLineCount(hTextBox);
	CapPagedHeight(&hTextBox);

	sub_808A384(proc, wTextBox, hTextBox);
	if (ModePaged() && proc->hBoxFinal > 0x30)
		proc->hBoxFinal = 0x30;

	sub_808A3C4(proc, info->xDisplay, info->yDisplay);
	ClearHelpBoxText_Layout();
	StartHelpBoxTextInit(proc->item, proc->mid);
	gpHelpBoxCurrentInfo = info;
}

void ApplyHelpBoxContentSize_Layout(struct HelpBoxProc *proc, int width, int height)
{
	width = 0xF0 & (width + 15);

	switch (GetHelpBoxItemInfoKind(proc->item)) {
	case 1:
		if (width < 0x90)
			width = 0x90;
		if (GetStringTextLen(GetStringFromIndex(proc->mid)) > 8)
			height += 0x20;
		else
			height += 0x10;
		break;
	case 2:
		if (width < 0x60)
			width = 0x60;
		height += 0x10;
		break;
	case 3:
		width = 0x80;
		height += 0x10;
		break;
	}

	CapPagedHeight(&height);
	proc->wBoxFinal = width;
	proc->hBoxFinal = height;
}

void HbMoveCtrl_OnIdle_Layout(struct HelpBoxProc *proc)
{
	u8 boxMoved = FALSE;

	DisplayUiHand(sHbOrigin.x * 8 + proc->info->xDisplay,
		sHbOrigin.y * 8 + proc->info->yDisplay);

	if (gKeyStatusPtr->repeatedKeys & DPAD_UP)
		boxMoved |= TryRelocateHbUp(proc);
	if (gKeyStatusPtr->repeatedKeys & DPAD_DOWN)
		boxMoved |= TryRelocateHbDown(proc);
	if (gKeyStatusPtr->repeatedKeys & DPAD_LEFT)
		boxMoved |= TryRelocateHbLeft(proc);
	if (gKeyStatusPtr->repeatedKeys & DPAD_RIGHT)
		boxMoved |= TryRelocateHbRight(proc);

	if (gKeyStatusPtr->newKeys & (B_BUTTON | R_BUTTON)) {
		HelpBoxResetPageState();
		Proc_Break((void *)proc);
		return;
	}

	if (HelpBoxTryAdvancePage())
		return;

	if (boxMoved) {
		HelpBoxResetPageState();
		PlaySoundEffect(0x67);
		Proc_Goto((void *)proc, 0);
	}
}
