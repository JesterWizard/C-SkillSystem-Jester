#include "global.h"
#include "bksel.h"
#include "bmbattle.h"
#include "bmitem.h"
#include "bmlib.h"
#include "constants/items.h"
#include "efxbattle.h"
#include "ekrbattle.h"
#include "fontgrp.h"
#include "hardware.h"
#include "icon.h"
#include "proc.h"
#include "variables.h"

extern const u8 ShowTrue2RNEnabled;

/**
 * Rounded true-hit probabilities for GBA 2RN:
 * Roll2RN succeeds when ((RN1 + RN2) / 2) < displayed.
 * Values match Serenes Forest, rounded to nearest percent.
 */
static const u8 sTrue2RNTable[101] = {
	  0,   0,   0,   0,   0,   1,   1,   1,   1,   2,
	  2,   3,   3,   4,   4,   5,   5,   6,   7,   7,
	  8,   9,  10,  11,  12,  13,  14,  15,  16,  17,
	 18,  20,  21,  22,  23,  25,  26,  28,  29,  31,
	 32,  34,  36,  37,  39,  41,  43,  45,  47,  49,
	 51,  52,  54,  56,  58,  60,  62,  63,  65,  67,
	 68,  70,  72,  73,  74,  76,  77,  79,  80,  81,
	 82,  83,  85,  86,  87,  88,  89,  90,  91,  91,
	 92,  93,  94,  94,  95,  96,  96,  97,  97,  98,
	 98,  98,  99,  99,  99, 100, 100, 100, 100, 100,
	100,
};

static int GetDisplayedTrueHitRate(int rate)
{
	if (!ShowTrue2RNEnabled)
		return rate;

	if (rate < 0 || rate > 100)
		return rate;

	return sTrue2RNTable[rate];
}

static void PutTrueHitNumber(u16 *dest, int color, int rate)
{
	PutNumberTwoChr(dest, color, GetDisplayedTrueHitRate(rate));
}

/**
 * Vanilla DrawBattleForecastContentsStandard with HIT shown as true 2RN.
 */
void DrawBattleForecastContentsStandard_TrueHit(struct BattleForecastProc *proc)
{
	int damage;
	int critRate;

	CallARM_FillTileRect(gUiTmScratchB, gTSA_BattleForecastStandard, 0x1000);

	TileMap_FillRect(gUiTmScratchA, 10, 15, 0);

	PutBattleForecastUnitName(gUiTmScratchA + 0x23, &proc->unitNameTextA, &gBattleActor.unit);

	PutBattleForecastUnitName(gUiTmScratchA + 0x161, &proc->unitNameTextA, &gBattleTarget.unit);

	PutBattleForecastItemName(gUiTmScratchA + 0x1A1, &proc->itemNameText, gBattleTarget.weaponBefore);

	if ((gBattleTarget.weapon == 0) && (gBattleTarget.weaponBroke == 0)) {
		damage = -1;

		gBattleTarget.battleEffectiveHitRate = 0xFF;
		gBattleTarget.battleEffectiveCritRate = 0xFF;
	} else {
		damage = gBattleTarget.battleAttack - gBattleActor.battleDefense;

		if (damage < 0)
			damage = 0;
	}

	if (gBattleTarget.hpInitial > 99)
		PutNumberTwoChr(gUiTmScratchA + 0x62, 2, 0xFF);
	else
		PutNumberTwoChr(gUiTmScratchA + 0x62, 2, gBattleTarget.hpInitial);

	PutNumberTwoChr(gUiTmScratchA + 0xA2, 2, damage);
	PutTrueHitNumber(gUiTmScratchA + 0xA2 + 0x40, 2, gBattleTarget.battleEffectiveHitRate);
	PutNumberTwoChr(gUiTmScratchA + 0xA2 + 0x80, 2, gBattleTarget.battleEffectiveCritRate);

	damage = gBattleActor.battleAttack - gBattleTarget.battleDefense;

	if (GetItemIndex(gBattleActor.weapon) == ITEM_MONSTER_STONE)
		damage = 0xFF;

	if (damage < 0)
		damage = 0;

	critRate = gBattleActor.battleEffectiveCritRate;

	if (GetItemIndex(gBattleActor.weapon) == ITEM_MONSTER_STONE)
		critRate = 0xFF;

	if (critRate < 0)
		critRate = 0;

	if (gBattleActor.hpInitial > 99)
		PutNumberTwoChr(gUiTmScratchA + 0xA8 - 0x40, 2, 0xFF);
	else
		PutNumberTwoChr(gUiTmScratchA + 0xA8 - 0x40, 2, gBattleActor.hpInitial);

	PutNumberTwoChr(gUiTmScratchA + 0xA8, 2, damage);
	PutTrueHitNumber(gUiTmScratchA + 0xA8 + 0x40, 2, gBattleActor.battleEffectiveHitRate);
	PutNumberTwoChr(gUiTmScratchA + 0xA8 + 0x80, 2, critRate);

	PutTwoSpecialChar(gUiTmScratchA + 0xA8 - 0x44, TEXT_COLOR_SYSTEM_GOLD, TEXT_SPECIAL_HP_A, TEXT_SPECIAL_HP_B);

	PutText(gaBattleForecastTextStructs, gUiTmScratchA + 0xA8 - 5);
	PutText(gaBattleForecastTextStructs + 1, gUiTmScratchA + 0xA8 + 0x3B);
	PutText(gaBattleForecastTextStructs + 2, gUiTmScratchA + 0xA8 + 0x7B);

	DrawIcon(gUiTmScratchA + 0xA8 + 0xBF, GetItemIconId(gBattleTarget.weaponBefore), 0x4000);
	DrawIcon(gUiTmScratchA + 0xA8 - 0x87, GetItemIconId(gBattleActor.weaponBefore), 0x3000);
}

/**
 * Vanilla DrawBattleForecastContentsExtended with HIT shown as true 2RN.
 */
void DrawBattleForecastContentsExtended_TrueHit(struct BattleForecastProc *proc)
{
	CallARM_FillTileRect(gUiTmScratchB, gTSA_BattleForecastExtended, 0x1000);

	TileMap_FillRect(gUiTmScratchA, 10, 19, 0);

	PutBattleForecastUnitName(gUiTmScratchA + 0x23, &proc->unitNameTextA, &gBattleActor.unit);
	PutBattleForecastUnitName(gUiTmScratchA + 0x1E1, &proc->unitNameTextA, &gBattleTarget.unit);

	PutBattleForecastItemName(gUiTmScratchA + 0x221, &proc->itemNameText, gBattleTarget.weaponBefore);

	if ((gBattleTarget.weapon == 0) && (!gBattleTarget.weaponBroke)) {
		gBattleTarget.battleAttack = 0xFF;
		gBattleTarget.battleEffectiveHitRate = 0xFF;
		gBattleTarget.battleEffectiveCritRate = 0xFF;
	}

	if (gBattleTarget.hpInitial > 99)
		PutNumberTwoChr(gUiTmScratchA + 0x62, 2, 0xFF);
	else
		PutNumberTwoChr(gUiTmScratchA + 0x62, 2, gBattleTarget.hpInitial);

	PutNumberTwoChr(gUiTmScratchA + 0xA2, 2, gBattleTarget.battleAttack);
	PutNumberTwoChr(gUiTmScratchA + 0xA2 + 0x40, 2, gBattleTarget.battleDefense);
	PutTrueHitNumber(gUiTmScratchA + 0xA2 + 0x80, 2, gBattleTarget.battleEffectiveHitRate);
	PutNumberTwoChr(gUiTmScratchA + 0xA2 + 0xC0, 2, gBattleTarget.battleEffectiveCritRate);
	PutNumberTwoChr(gUiTmScratchA + 0xA2 + 0x100, 2, gBattleTarget.battleSpeed);

	if (gBattleActor.hpInitial > 99)
		PutNumberTwoChr(gUiTmScratchA + 0xA2 - 0x3A, 2, 0xFF);
	else
		PutNumberTwoChr(gUiTmScratchA + 0xA2 - 0x3A, 2, gBattleActor.hpInitial);

	PutNumberTwoChr(gUiTmScratchA + 0xA8, 2, gBattleActor.battleAttack);
	PutNumberTwoChr(gUiTmScratchA + 0xA8 + 0x40, 2, gBattleActor.battleDefense);
	PutTrueHitNumber(gUiTmScratchA + 0xA8 + 0x80, 2, gBattleActor.battleEffectiveHitRate);
	PutNumberTwoChr(gUiTmScratchA + 0xA8 + 0xC0, 2, gBattleActor.battleEffectiveCritRate);
	PutNumberTwoChr(gUiTmScratchA + 0xA8 + 0x100, 2, gBattleActor.battleSpeed);

	PutTwoSpecialChar(gUiTmScratchA + 0xA8 - 0x44, TEXT_COLOR_SYSTEM_GOLD, TEXT_SPECIAL_HP_A, TEXT_SPECIAL_HP_B);

	PutText(gaBattleForecastTextStructs + 3, gUiTmScratchA + 0xA8 - 5);
	PutText(gaBattleForecastTextStructs + 4, gUiTmScratchA + 0xA8 + 0x3B);
	PutText(gaBattleForecastTextStructs + 1, gUiTmScratchA + 0xA8 + 0x7B);
	PutText(gaBattleForecastTextStructs + 2, gUiTmScratchA + 0xA8 + 0xBB);
	PutText(gaBattleForecastTextStructs + 5, gUiTmScratchA + 0xA8 + 0xFB);

	DrawIcon(gUiTmScratchA + 0xA8 + 0x13F, GetItemIconId(gBattleTarget.weaponBefore), 0x4000);
	DrawIcon(gUiTmScratchA + 0xA8 - 0x87, GetItemIconId(gBattleActor.weaponBefore), 0x3000);
}

/**
 * Vanilla NewEkrGauge with HIT shown as true 2RN.
 */
void NewEkrGauge_TrueHit(void)
{
	u32 i, j;

	gpProcEkrGauge = Proc_Start(ProcScr_ekrGauge, PROC_TREE_1);

	EkrGauge_Setup44(0);
	EkrGauge_Clr4C50();
	DisableEkrGauge();
	EkrGauge_ClrInitFlag();
	EkrGauge_Clr323A(gEkrBg0QuakeVec.x, gEkrBg0QuakeVec.y);

	if (gEkrGaugeHp[0] > 0x50)
		CpuCopy16(gPalEfxHpBarPurple, PAL_OBJ(0xB), 0x10 * sizeof(u16));
	else
		CpuCopy16(gPalEfxHpBarGreen + gBanimFactionPal[POS_L] * 0x10, PAL_OBJ(0xB), 0x10 * sizeof(u16));

	if (gEkrGaugeHp[1] > 0x50)
		CpuCopy16(gPalEfxHpBarPurple, PAL_OBJ(0xC), 0x10 * sizeof(u16));
	else
		CpuCopy16(gPalEfxHpBarGreen + gBanimFactionPal[POS_R] * 0x10, PAL_OBJ(0xC), 0x10 * sizeof(u16));

	gEkrGaugeHpBak[0] = -1;
	gEkrGaugeHpBak[1] = -1;

	LZ77UnCompVram(Img_EfxSideHitDmgCrit, (void *)0x6013800);
	LZ77UnCompVram(Img_EfxWTAArrow1, (void *)0x6013940);
	LZ77UnCompVram(Img_EfxWTAArrow2, (void *)0x6013D40);

	CpuFastCopy(gUnknown_08802884 + gBanimFactionPal[POS_L] * 0x10, PAL_OBJ(0x5), 0x10 * sizeof(u16));
	CpuFastCopy(gUnknown_08802884 + gBanimFactionPal[POS_R] * 0x10, PAL_OBJ(0x6), 0x10 * sizeof(u16));

	EnablePaletteSync();

	ModDec(GetDisplayedTrueHitRate(gEkrGaugeHit[0]), &gEkrGaugeDecoder[0x0]);
	ModDec(gEkrGaugeDmg[0], &gEkrGaugeDecoder[0x3]);
	ModDec(gEkrGaugeCrt[0], &gEkrGaugeDecoder[0x6]);

	ModDec(GetDisplayedTrueHitRate(gEkrGaugeHit[1]), &gEkrGaugeDecoder[0x9]);
	ModDec(gEkrGaugeDmg[1], &gEkrGaugeDecoder[0xC]);
	ModDec(gEkrGaugeCrt[1], &gEkrGaugeDecoder[0xF]);

	CpuFastFill(0, gObjBuf_EkrSideHitDmgCrit, 0x400);

	for (i = 0; i < 6; i++) {
		for (j = 0; j < 3; j++) {
			int r4 = i * 0x40 + j * 0x10;

			CpuCopy16(
				gUnknown_088026E4 + gEkrGaugeDecoder[i * 3 + j] * 0x10,
				gObjBuf_EkrSideHitDmgCrit + r4,
				0x10 * sizeof(u16));
		}
	}

	RegisterDataMove(gObjBuf_EkrSideHitDmgCrit, (void *)0x6013A00, 0xC0 * sizeof(u16));
	RegisterDataMove(gObjBuf_EkrSideHitDmgCrit + 0xC0, (void *)0x6013E00, 0xC0 * sizeof(u16));

	ResetIconGraphics_();
	LoadIconPalette(0, 0x1D);
	LoadIconPalette(0, 0x1E);
	LoadIconObjectGraphics(GetItemIconId(gpEkrBattleUnitLeft->weaponBefore), 0x1DC);
	LoadIconObjectGraphics(GetItemIconId(gpEkrBattleUnitRight->weaponBefore), 0x1DE);
	ApplyPalette(gPal_MiscUiGraphics, 0x10);
}
