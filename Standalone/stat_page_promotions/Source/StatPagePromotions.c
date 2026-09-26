#include "global.h"
#include "statscreen.h"
#include "helpbox.h"
#include "bmunit.h"
#include "bmudisp.h"
#include "variables.h"
#include "hardware.h"
#include "ctc.h"
#include "fontgrp.h"
#include "constants/classes.h"
#include "constants/characters.h"

extern const u8 StatPagePromotionsEnabled;

/* Vanilla chapter id for Chapter 10. Earlier chapters keep the 3-page stat screen. */
#define PROMO_PAGE_CHAPTER 0x0B
#define PROMO_PAGE_ID 3

struct PromoPageEntry {
	u8 charId;
	u8 classId[3];
};

/* Skill icons from the integrated table are omitted. Edit this list, then run make. */
static const struct PromoPageEntry sPromoPages[] = {
	{ CHARACTER_EIRIKA, { CLASS_EIRIKA_MASTER_LORD, CLASS_EPHRAIM_MASTER_LORD, CLASS_PALADIN } },
	{ 0, { 0, 0, 0 } },
};

static const struct PromoPageEntry *FindPromoPage(int charId)
{
	int i;

	for (i = 0; sPromoPages[i].charId != 0; ++i) {
		if (sPromoPages[i].charId == charId)
			return &sPromoPages[i];
	}

	return NULL;
}

static int PromoPageOpen(void)
{
	return StatPagePromotionsEnabled && gPlaySt.chapterIndex >= PROMO_PAGE_CHAPTER;
}

static void DrawPromoPage(void)
{
	const struct PromoPageEntry *entry;
	static const int y[3] = { 0x1, 0x6, 0xB };
	int i;

	entry = FindPromoPage(UNIT_CHAR_ID(gStatScreen.unit));
	if (entry == NULL) {
		InitText(&gStatScreen.text[STATSCREEN_TEXT_ITEM0], 14);
		PutDrawText(
			&gStatScreen.text[STATSCREEN_TEXT_ITEM0],
			gUiTmScratchA + TILEMAP_INDEX(0x5, 0x8),
			TEXT_COLOR_SYSTEM_GOLD, 0, 0,
			"No promotions");
		return;
	}

	for (i = 0; i < 3; ++i) {
		if (entry->classId[i] == 0)
			continue;

		InitText(&gStatScreen.text[STATSCREEN_TEXT_ITEM0 + i], 12);
		PutDrawText(
			&gStatScreen.text[STATSCREEN_TEXT_ITEM0 + i],
			gUiTmScratchA + TILEMAP_INDEX(0x6, y[i]),
			TEXT_COLOR_SYSTEM_GOLD, 0, 0,
			GetStringFromIndex(GetClassData(entry->classId[i])->nameTextId));
	}
}

void DisplayPage_Promotions(int pageid)
{
	CpuFastFill(0, gUiTmScratchA, sizeof(gUiTmScratchA));
	CpuFastFill(0, gUiTmScratchC, sizeof(gUiTmScratchC));

	switch (pageid) {
	case STATSCREEN_PAGE_0:
		DisplayPage0();
		break;
	case STATSCREEN_PAGE_1:
		DisplayPage1();
		break;
	case STATSCREEN_PAGE_2:
		DisplayPage2();
		break;
	default:
		DrawPromoPage();
		break;
	}
}

void DisplayPageNameSprite_Promotions(int pageid)
{
	int colorid;
	int palPage = pageid;

	if (palPage > STATSCREEN_PAGE_2)
		palPage = STATSCREEN_PAGE_0;

	PutSprite(4,
		111 + gStatScreen.xDispOff, 1 + gStatScreen.yDispOff,
		sSprite_PageNameBack, TILEREF(0x293, 4) + OAM2_LAYER(3));

	if (pageid <= STATSCREEN_PAGE_2) {
		PutSprite(4,
			114 + gStatScreen.xDispOff, 0 + gStatScreen.yDispOff,
			sPageNameSpriteLut[pageid],
			TILEREF(0x240 + sPageNameChrOffsetLut[pageid], 3) + OAM2_LAYER(3));
	}

	colorid = (GetGameClock() / 4) % 16;
	CpuCopy16(gUnknown_08A027FC[palPage] + colorid, PAL_OBJ(3) + 0xE, sizeof(u16));
	EnablePaletteSync();
}

void PageNumCtrl_DisplayMuPlatform_Promotions(struct StatScreenPageNameProc *proc)
{
	const struct PromoPageEntry *entry;

	(void)proc;
	static const int platformY[3] = { 41, 82, 122 };
	static const int spriteY[3] = { 35, 76, 116 };
	int i;

	PutSprite(11,
		gStatScreen.xDispOff + 64,
		gStatScreen.yDispOff + 131,
		gObject_32x16, TILEREF(0x28F, STATSCREEN_OBJPAL_4) + OAM2_LAYER(3));

	if (gStatScreen.page != PROMO_PAGE_ID || !PromoPageOpen())
		return;

	entry = FindPromoPage(UNIT_CHAR_ID(gStatScreen.unit));
	if (entry == NULL)
		return;

	for (i = 0; i < 3; ++i) {
		if (entry->classId[i] == 0)
			continue;

		PutSprite(11, 99, platformY[i], gObject_32x16,
			TILEREF(0x28F, STATSCREEN_OBJPAL_4) + OAM2_LAYER(3));
		PutUnitSpriteForClassId(0, 108, spriteY[i], 0xC800, entry->classId[i]);
	}
}

static void GetPromotedClassDesc(struct HelpBoxProc *proc)
{
	const struct PromoPageEntry *entry;
	int which = -1;

	entry = FindPromoPage(UNIT_CHAR_ID(gStatScreen.unit));
	if (proc->info->yDisplay == 0x20)
		which = 0;
	else if (proc->info->yDisplay == 0x48)
		which = 1;
	else if (proc->info->yDisplay == 0x70)
		which = 2;

	if (entry == NULL || which < 0 || entry->classId[which] == 0) {
		gKeyStatusPtr->newKeys = B_BUTTON;
		return;
	}

	proc->mid = GetClassData(entry->classId[which])->descTextId;
}

static const struct HelpBoxInfo RText_PromoName;
static const struct HelpBoxInfo RText_PromoClass;
static const struct HelpBoxInfo RText_PromoLevel;
static const struct HelpBoxInfo RText_PromoExp;
static const struct HelpBoxInfo RText_PromoHp;
static const struct HelpBoxInfo RText_Promo1;
static const struct HelpBoxInfo RText_Promo2;
static const struct HelpBoxInfo RText_Promo3;

static const struct HelpBoxInfo RText_Promo1 = {
	&RText_Promo3, &RText_Promo2, &RText_PromoName, &RText_Promo2,
	0x6C, 0x20, 0, NULL, GetPromotedClassDesc
};
static const struct HelpBoxInfo RText_Promo2 = {
	&RText_Promo1, &RText_Promo3, &RText_PromoClass, &RText_Promo3,
	0x6C, 0x48, 0, NULL, GetPromotedClassDesc
};
static const struct HelpBoxInfo RText_Promo3 = {
	&RText_Promo2, &RText_Promo1, &RText_PromoHp, &RText_Promo1,
	0x6C, 0x70, 0, NULL, GetPromotedClassDesc
};
static const struct HelpBoxInfo RText_PromoName = {
	&RText_PromoHp, &RText_PromoClass, NULL, &RText_Promo1,
	0x18, 0x50, 0, NULL, HbPopulate_SSCharacter
};
static const struct HelpBoxInfo RText_PromoClass = {
	&RText_PromoName, &RText_PromoLevel, NULL, &RText_Promo2,
	0x06, 0x68, 0, NULL, HbPopulate_SSClass
};
static const struct HelpBoxInfo RText_PromoLevel = {
	&RText_PromoClass, &RText_PromoHp, NULL, &RText_PromoExp,
	0x06, 0x78, 0x542, NULL, NULL
};
static const struct HelpBoxInfo RText_PromoExp = {
	&RText_PromoClass, &RText_PromoHp, &RText_PromoLevel, &RText_Promo2,
	0x26, 0x78, 0x543, NULL, NULL
};
static const struct HelpBoxInfo RText_PromoHp = {
	&RText_PromoLevel, &RText_PromoName, NULL, &RText_Promo3,
	0x06, 0x88, 0x544, NULL, NULL
};

void StartStatScreenHelp_Promotions(int pageid, struct Proc *proc)
{
	LoadHelpBoxGfx(NULL, -1);

	if (!gStatScreen.help) {
		switch (pageid) {
		case STATSCREEN_PAGE_0:
			gStatScreen.help = &gHelpInfo_Ss0Pow;
			break;
		case STATSCREEN_PAGE_1:
			gStatScreen.help = &gHelpInfo_Ss1Item0;
			break;
		case STATSCREEN_PAGE_2:
			gStatScreen.help = &gHelpInfo_Ss2Rank0;
			break;
		default:
			gStatScreen.help = &RText_Promo1;
			break;
		}
	}

	StartMovingHelpBox(gStatScreen.help, proc);
}

void StatScreen_ApplyPromoPageCount(void)
{
	gStatScreen.pageAmt = PromoPageOpen() ? 4 : 3;
	ResetText();
}

/* Replaces `movs r0, #3; strb r0, [r5, #1]; bl ResetText` and returns to the next instruction. */
void __attribute__((naked)) StatScreen_PageAmtHook(void)
{
	asm volatile (
		"push {lr}\n"
		"bl StatScreen_ApplyPromoPageCount\n"
		"pop {r3}\n"
		"ldr r3, =0x08088699\n"
		"bx r3\n"
		".ltorg\n"
	);
}
