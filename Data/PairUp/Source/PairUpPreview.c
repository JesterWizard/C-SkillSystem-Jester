#include "common-chax.h"
#include "pair-up.h"
#include "ctc.h"
#include "menu_def.h"
#include "unitinfowindow.h"
#include "uiselecttarget.h"
#include "action-expa.h"
#include "map-anims.h"
#include "constants/texts.h"

extern struct UnitInfoWindowProc *gPairUpPreviewWindows[2];
extern struct Font *gActiveFont;
extern struct Glyph *TextGlyphs_System[];

const u16 gPairUpPreviewLabels[] = {
	0x04FE, /* Str */
	0x04EC, /* Skl */
	0x04ED, /* Spd */
	0x04EF, /* Def */
	0x04F0, /* Res */
	0x04EE, /* Lck */
	0x04FF, /* Mag */
	0x04F6, /* Mov */
};

const u8 gPairUpPreviewStats[] = {
	PAIR_UP_STAT_POW,
	PAIR_UP_STAT_SKL,
	PAIR_UP_STAT_SPD,
	PAIR_UP_STAT_DEF,
	PAIR_UP_STAT_RES,
	PAIR_UP_STAT_LCK,
	PAIR_UP_STAT_MAG,
	PAIR_UP_STAT_MOV,
};

static void PairUpPreview_DrawPlus(struct Text *text, int x)
{
	struct Font font = *gActiveFont;
	struct Font *previousFont = gActiveFont;

	font.glyphs = TextGlyphs_System;
	font.lang = LANG_ENGLISH;
	SetTextFont(&font);
	Text_InsertDrawString(text, x, TEXT_COLOR_SYSTEM_BLUE, "+");
	SetTextFont(previousFont);
}

static void PairUpPreview_InitWindow(
	struct UnitInfoWindowProc **out,
	ProcPtr parent)
{
	int i;

	*out = NewUnitInfoWindow(parent);
	(*out)->unit = gActiveUnit;

	for (i = 0; i < 5; ++i)
		InitTextDb((*out)->lines + i, 14);
}

static void PairUpPreview_Draw(
	struct Unit *leader,
	struct Unit *support)
{
	struct UnitInfoWindowProc *window;
	int x;
	int row;

	if (!leader || !support || !gPairUpPreviewWindows[0]
		|| !gPairUpPreviewWindows[1])
		return;

	x = GetUnitInfoWindowX(leader, 14);

	UnitInfoWindow_DrawBase(
		gPairUpPreviewWindows[0],
		support,
		x,
		0,
		14,
		0
	);

	window = UnitInfoWindow_DrawBase(
		gPairUpPreviewWindows[1],
		leader,
		x,
		4,
		14,
		4
	);

	for (row = 0; row < 4; ++row) {
		int left = row;
		int right = row + 4;
		int y = 4 + 3 + row * 2;
		struct Text *text = window->lines + row;

		ClearText(text);
		Text_InsertDrawString(
			text,
			0,
			TEXT_COLOR_SYSTEM_GOLD,
			GetStringFromIndex(gPairUpPreviewLabels[left])
		);
		PairUpPreview_DrawPlus(text, 24);
		Text_InsertDrawNumberOrBlank(
			text,
			32,
			TEXT_COLOR_SYSTEM_BLUE,
			PairUp_GetStatBonus(leader, support, gPairUpPreviewStats[left])
		);
		Text_InsertDrawString(
			text,
			56,
			TEXT_COLOR_SYSTEM_GOLD,
			GetStringFromIndex(gPairUpPreviewLabels[right])
		);
		PairUpPreview_DrawPlus(text, 80);
		Text_InsertDrawNumberOrBlank(
			text,
			88,
			TEXT_COLOR_SYSTEM_BLUE,
			PairUp_GetStatBonus(leader, support, gPairUpPreviewStats[right])
		);
		PutText(text, gBG0TilemapBuffer + TILEMAP_INDEX(x + 1, y));
	}

	BG_EnableSyncByMask(BG0_SYNC_BIT);
	MoveSpriteRefresher(NULL, (x + 4) * 8, 4 * 8 + 7 - 16);
}

static void PairUpPreview_OnInit(ProcPtr proc)
{
	if (gSelectInfo_PutTrap.onInit)
		gSelectInfo_PutTrap.onInit(proc);

	PairUpPreview_InitWindow(&gPairUpPreviewWindows[0], proc);
	PairUpPreview_InitWindow(&gPairUpPreviewWindows[1], proc);
	StartSpriteRefresher(proc, 2, 0, 0, gObject_16x16_VFlipped, 6);
}

static void PairUpPreview_OnEnd(ProcPtr proc)
{
	if (gSelectInfo_PutTrap.onEnd)
		gSelectInfo_PutTrap.onEnd(proc);

	gPairUpPreviewWindows[0] = NULL;
	gPairUpPreviewWindows[1] = NULL;
	ClearBg0Bg1();
}

static void PairUpPreview_OnUnk08(ProcPtr proc)
{
	if (gSelectInfo_PutTrap.onUnk08)
		gSelectInfo_PutTrap.onUnk08(proc);
}

static u8 PairUpPreview_OnSwitchIn(
	ProcPtr proc,
	struct SelectTarget *target)
{
	u8 result = 0;

	if (gSelectInfo_PutTrap.onSwitchIn)
		result = gSelectInfo_PutTrap.onSwitchIn(proc, target);

	PairUpPreview_Draw(gActiveUnit, GetUnit(target->uid));
	return result;
}

static u8 PairUpPreview_OnSwitchOut(
	ProcPtr proc,
	struct SelectTarget *target)
{
	u8 result = 0;

	if (gSelectInfo_PutTrap.onSwitchOut)
		result = gSelectInfo_PutTrap.onSwitchOut(proc, target);

	ClearBg0Bg1();
	return result;
}

static u8 PairUpPreview_OnCancel(
	ProcPtr proc,
	struct SelectTarget *target)
{
	if (gSelectInfo_PutTrap.onCancel)
		return gSelectInfo_PutTrap.onCancel(proc, target);

	return 0;
}

static u8 PairUpPreview_OnHelp(
	ProcPtr proc,
	struct SelectTarget *target)
{
	if (gSelectInfo_PutTrap.onHelp)
		return gSelectInfo_PutTrap.onHelp(proc, target);

	return 0;
}

const struct SelectInfo gPairUpAttachSelectInfo = {
	.onInit = PairUpPreview_OnInit,
	.onEnd = PairUpPreview_OnEnd,
	.onUnk08 = PairUpPreview_OnUnk08,
	.onSwitchIn = PairUpPreview_OnSwitchIn,
	.onSwitchOut = PairUpPreview_OnSwitchOut,
	.onSelect = NULL,
	.onCancel = PairUpPreview_OnCancel,
	.onHelp = PairUpPreview_OnHelp,
};
