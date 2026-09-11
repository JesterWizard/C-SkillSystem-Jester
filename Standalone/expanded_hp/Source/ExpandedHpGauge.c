#include "global.h"
#include "hardware.h"
#include "bmlib.h"
#include "proc.h"
#include "anime.h"
#include "ekrbattle.h"
#include "efxbattle.h"
#include "bmbattle.h"
#include "bmitem.h"
#include "icon.h"
#include "variables.h"
#include "constants/video-banim.h"

extern u16 gPalEfxHpBarYellow[];
extern u16 gPalEfxHpBarRed[];
extern u16 gPalEfxHpBarBlue[];

static u16 *GetHpBarPalette(u8 current_hp)
{
	if (current_hp <= 80)
		return gPalEfxHpBarGreen;
	if (current_hp <= 160)
		return gPalEfxHpBarYellow;
	if (current_hp <= 240)
		return gPalEfxHpBarRed;

	return gPalEfxHpBarBlue;
}

static bool IsLowHpThreshold(u8 current_hp)
{
	return current_hp <= 80;
}

/* Vanilla battle-init may ldrsb HP into s16; recover unsigned 128-254. */
static int GaugeHpUnsigned(s16 hp)
{
	int value = hp;

	if (value < 0)
		value += 256;

	if (value < 0)
		value = 0;

	if (value > 254)
		value = 254;

	return value;
}

static void SplitGaugeHpDigits(int hp, s32 *hundreds, u16 *tens, u16 *ones)
{
	hp = GaugeHpUnsigned(hp);

	if (hp >= 200) {
		*hundreds = 2;
		hp -= 200;
	} else if (hp >= 100) {
		*hundreds = 1;
		hp -= 100;
	} else {
		*hundreds = 0;
	}

	*tens = Div(hp, 10);
	*ones = hp - *tens * 10;

	if (*tens == 0 && *hundreds == 0)
		*tens = 0xb;
}

void EfxFlashHPBarMain1_ExpandedHp(struct ProcEfxFlashing *proc)
{
	u16 *palette;

	if (GetAnimPosition(proc->anim) == EKR_POS_L) {
		palette = GetHpBarPalette(gEkrGaugeHp[EKR_POS_L]);
		CpuCopy16(palette, PAL_OBJ(OBPAL_EFXHPBAR_L), 0x20);
	} else {
		palette = GetHpBarPalette(gEkrGaugeHp[EKR_POS_R]);
		CpuCopy16(palette, PAL_OBJ(OBPAL_EFXHPBAR_R), 0x20);
	}

	EnablePaletteSync();

	if (++proc->timer >= proc->terminator2)
		Proc_Break(proc);
}

void EfxFlashHPBarRestorePal_ExpandedHp(struct ProcEfxFlashing *proc)
{
	u16 *palette;

	if (GetAnimPosition(proc->anim) == EKR_POS_L) {
		if (IsLowHpThreshold(gEkrGaugeHp[EKR_POS_L])) {
			CpuCopy16(&PAL_BUF_COLOR(gPalEfxHpBarGreen, gBanimFactionPal[EKR_POS_L], 0),
				PAL_OBJ(OBPAL_EFXHPBAR_L), 0x20);
		} else {
			palette = GetHpBarPalette(gEkrGaugeHp[EKR_POS_L]);
			CpuCopy16(palette, PAL_OBJ(OBPAL_EFXHPBAR_L), 0x20);
		}
	} else {
		if (IsLowHpThreshold(gEkrGaugeHp[EKR_POS_R])) {
			CpuCopy16(&PAL_BUF_COLOR(gPalEfxHpBarGreen, gBanimFactionPal[EKR_POS_R], 0),
				PAL_OBJ(OBPAL_EFXHPBAR_R), 0x20);
		} else {
			palette = GetHpBarPalette(gEkrGaugeHp[EKR_POS_R]);
			CpuCopy16(palette, PAL_OBJ(OBPAL_EFXHPBAR_R), 0x20);
		}
	}

	EnablePaletteSync();
	Proc_Break(proc);
}

void EfxHPBarColorChangeMain_ExpandedHp(struct ProcEfxHpBarColorChange *proc)
{
	int ret;
	u8 *buf1;
	s16 *buf3;
	u16 *palette;

	if (proc->disabled == true)
		return;

	ret = EfxAdvanceFrameLut(&proc->timer1, (s16 *)&proc->frame1, (s16 *)proc->frame_lut1);
	if (ret >= 0)
		proc->unk54 = ret;

	ret = EfxAdvanceFrameLut(&proc->timer2, (s16 *)&proc->frame2, (s16 *)proc->frame_lut2);
	if (ret >= 0)
		proc->unk58 = ret;

	if (IsLowHpThreshold(gEkrGaugeHp[EKR_POS_L])) {
		EfxDecodeSplitedPalette(
			PAL_OBJ(OBPAL_EFXHPBAR_L),
			(s8 *)gEfxSplitedColorBufA,
			(s8 *)gEfxSplitedColorBufB,
			(s16 *)gEfxSplitedColorBufC,
			0x10,
			proc->unk54,
			5);
	} else {
		palette = GetHpBarPalette(gEkrGaugeHp[EKR_POS_L]);
		CpuFastCopy(palette, PAL_OBJ(OBPAL_EFXHPBAR_L), 0x20);
	}

	if (IsLowHpThreshold(gEkrGaugeHp[EKR_POS_R])) {
		buf1 = gEfxSplitedColorBufD;
		buf3 = gEfxSplitedColorBufF;

		EfxDecodeSplitedPalette(
			PAL_OBJ(OBPAL_EFXHPBAR_R),
			(s8 *)buf1,
			(s8 *)gEfxSplitedColorBufE,
			(s16 *)buf3,
			0x10, proc->unk54,
			5);
	} else {
		palette = GetHpBarPalette(gEkrGaugeHp[EKR_POS_R]);
		CpuFastCopy(palette, PAL_OBJ(OBPAL_EFXHPBAR_R), 0x20);
	}

	EnablePaletteSync();
}

void NewEkrGauge_ExpandedHp(void)
{
	u32 i, j;
	u16 *palette_left;
	u16 *palette_right;

	gpProcEkrGauge = Proc_Start(ProcScr_ekrGauge, PROC_TREE_1);

	EkrGauge_Setup44(0);
	EkrGauge_Clr4C50();
	DisableEkrGauge();
	EkrGauge_ClrInitFlag();
	EkrGauge_Clr323A(gEkrBg0QuakeVec.x, gEkrBg0QuakeVec.y);

	gEkrGaugeHp[0] = gpEkrBattleUnitLeft->hpInitial;
	gEkrGaugeHp[1] = gpEkrBattleUnitRight->hpInitial;
	gBanimMaxHP[0] = gpEkrBattleUnitLeft->unit.maxHP;
	gBanimMaxHP[1] = gpEkrBattleUnitRight->unit.maxHP;

	palette_left = GetHpBarPalette(gEkrGaugeHp[0]);
	CpuCopy16(palette_left, PAL_OBJ(0xB), 0x10 * sizeof(u16));

	palette_right = GetHpBarPalette(gEkrGaugeHp[1]);
	CpuCopy16(palette_right, PAL_OBJ(0xC), 0x10 * sizeof(u16));

	gEkrGaugeHpBak[0] = -1;
	gEkrGaugeHpBak[1] = -1;

	LZ77UnCompVram(Img_EfxSideHitDmgCrit, (void *)0x6013800);
	LZ77UnCompVram(Img_EfxWTAArrow1, (void *)0x6013940);
	LZ77UnCompVram(Img_EfxWTAArrow2, (void *)0x6013D40);

	CpuFastCopy(gUnknown_08802884 + gBanimFactionPal[POS_L] * 0x10, PAL_OBJ(0x5), 0x10 * sizeof(u16));
	CpuFastCopy(gUnknown_08802884 + gBanimFactionPal[POS_R] * 0x10, PAL_OBJ(0x6), 0x10 * sizeof(u16));

	EnablePaletteSync();

	ModDec(gEkrGaugeHit[0], &gEkrGaugeDecoder[0x0]);
	ModDec(gEkrGaugeDmg[0], &gEkrGaugeDecoder[0x3]);
	ModDec(gEkrGaugeCrt[0], &gEkrGaugeDecoder[0x6]);

	ModDec(gEkrGaugeHit[1], &gEkrGaugeDecoder[0x9]);
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

void ekrGaugeMain_ExpandedHp(struct ProcEkrGauge *proc)
{
	struct Anim AStack_130;
	u16 auStack_e8[12];
	u16 local_d0[4];
	struct AnimSpriteData auStack_c8[8];
	s16 r4;
	s32 r6;
	s32 r7;
	s32 r8;
	s32 r9;
	s16 r7_;
	s16 r6_;
	s16 r8_;
	s16 sp_d4;
	s32 hp_changed;
	s32 spDC;
	s32 x;
	s32 y;
	s32 clk;
	s16 uVar8;
	s16 sVar16;
	s16 sVar5;
	s16 uVar15;
	u16 player_hp_tilemap_index;
	u16 enemy_hp_tilemap_index;
	s32 hp_hundreds_digit[2];
	u8 *hp_digits_oam_array = gUnknown_085B940C;

	hp_changed = 0;
	clk = DivRem(GetGameClock() / 8, 3);

	if (proc->valid == 1)
		return;

	if (proc->battle_init == 0) {
		r4 = proc->unk3A >> 3;
		r7 = (r4 << 5) + 0x1A0;

		if (r7 < 0)
			r7 = 0;

		r6 = r4 + 7;
		if (r6 > 7)
			r6 = 7;

		r8 = (7 - r6) * 30;

		switch (gEkrDistanceType) {
		case 0:
		case 1:
		case 2:
			r9 = 0;
			spDC = 15;
			break;
		case 3:
		case 4:
		default:
			spDC = 8;
			r9 = 8;
			break;
		}

		FillBGRect(gBG0TilemapBuffer + 0x1A0, 30, 8, 0, 0x80);

		if (0 == proc->unk4C) {
			EfxTmCpyBG(gUnknown_08802274 + r8, &gBG0TilemapBuffer[r7 + r9], 15, r6, -1, -1);
			sub_8070D04(&gBG0TilemapBuffer[r7 + r9], 15, r6, 2, 0x80);
		}

		if (0 == proc->unk50) {
			void *ptr;

			if (0 == proc->unk4C)
				ptr = gUnknown_08802348 + r8;
			else
				ptr = gUnknown_08802428 + r8;

			EfxTmCpyBG(ptr, &gBG0TilemapBuffer[r7 + spDC], 16, r6, -1, -1);
			sub_8070D04(&gBG0TilemapBuffer[r7 + spDC], 16, r6, 3, 128);
		}

		BG_EnableSyncByMask(1);
	}

	if (gEkrGaugeHpBak[0] != gEkrGaugeHp[0])
		hp_changed = 1;

	if (gEkrGaugeHpBak[1] != gEkrGaugeHp[1])
		hp_changed = 1;

	gEkrGaugeHpBak[0] = gEkrGaugeHp[0];
	gEkrGaugeHpBak[1] = gEkrGaugeHp[1];

	r7_ = GaugeHpUnsigned(gEkrGaugeHp[0]);
	r6_ = gpEkrBattleUnitLeft->unit.maxHP;
	r8_ = GaugeHpUnsigned(gEkrGaugeHp[1]);
	sp_d4 = gpEkrBattleUnitRight->unit.maxHP;

	switch (gEkrDistanceType) {
	case 3:
		if (gBanimValid[POS_L] == 1)
			x = proc->unk32 + 0x38;
		else
			x = proc->unk32 - 0x38;
		break;
	case 0:
	case 1:
	case 2:
		x = proc->unk32;
		break;
	case 4:
	default:
		x = proc->unk32 - 0x38;
		break;
	}

	if (proc->battle_init == 0)
		y = proc->unk3A & 0xFFF8;
	else
		y = proc->unk3A;

	/* CSS tiles: hundreds + tens/ones are consecutive (0x1E0-0x1E2 / 0x1E4-0x1E6). */
	player_hp_tilemap_index = 0x1E1;
	enemy_hp_tilemap_index = 0x1E5;

	SplitGaugeHpDigits(gEkrGaugeHp[0], &hp_hundreds_digit[0], &local_d0[0], &local_d0[1]);
	SplitGaugeHpDigits(gEkrGaugeHp[1], &hp_hundreds_digit[1], &local_d0[2], &local_d0[3]);

	if (hp_changed == 1) {
		u16 *digit_buf = gObjBuf_EkrSideHitDmgCrit;

		CpuCopy16(gUnknown_088026E4 + hp_hundreds_digit[0] * 0x10, digit_buf + 0x00, 0x20);
		CpuCopy16(gUnknown_088026E4 + local_d0[0] * 0x10, digit_buf + 0x10, 0x20);
		CpuCopy16(gUnknown_088026E4 + local_d0[1] * 0x10, digit_buf + 0x20, 0x20);
		CpuCopy16(gUnknown_088026E4 + hp_hundreds_digit[1] * 0x10, digit_buf + 0x30, 0x20);
		CpuCopy16(gUnknown_088026E4 + local_d0[2] * 0x10, digit_buf + 0x40, 0x20);
		CpuCopy16(gUnknown_088026E4 + local_d0[3] * 0x10, digit_buf + 0x50, 0x20);

		RegisterDataMove(digit_buf, (void *)(0x06010000 + ((player_hp_tilemap_index - 1) * 0x20)), 0x60);
		RegisterDataMove(digit_buf + 0x30, (void *)(0x06010000 + ((enemy_hp_tilemap_index - 1) * 0x20)), 0x60);
	}

	AStack_130.oam2Base = 0x0000B000 + player_hp_tilemap_index;
	AStack_130.oam2Base |= proc->unk44;
	AStack_130.xPosition = x + 9;
	AStack_130.yPosition = y + 0x91;
	AStack_130.state2 = 0;

	if (CheckEkrHitNow(POS_L) != 1) {
		AStack_130.pSpriteData = hp_digits_oam_array;
		AStack_130.oamBase = 0;
	} else {
		AStack_130.pSpriteData = auStack_c8;
		AStack_130.oamBase = 0x200;
		AStack_130.xPosition = AStack_130.xPosition - 8;
		AStack_130.yPosition = AStack_130.yPosition - 8;
		BanimUpdateSpriteRotScale(hp_digits_oam_array, auStack_c8, 0x100, 0x80, 1);
	}

	if (proc->unk4C == 0) {
		AnimDisplay(&AStack_130);

		if (hp_hundreds_digit[0] > 0) {
			AStack_130.oam2Base = 0x0000B000 + (player_hp_tilemap_index - 1);
			AStack_130.oam2Base |= proc->unk44;
			AStack_130.xPosition = x + 1;
			AStack_130.yPosition = y + 0x91;
			AStack_130.state2 = 0;
			AStack_130.pSpriteData = hp_digits_oam_array;
			AStack_130.oamBase = 0;
			AnimDisplay(&AStack_130);
		}
	}

	AStack_130.oamBase = 0;
	AStack_130.oam2Base = 0x0000C000 + enemy_hp_tilemap_index;
	AStack_130.oam2Base |= proc->unk44;
	AStack_130.xPosition = x + 0x81;
	AStack_130.yPosition = y + 0x91;
	AStack_130.state2 = 0;

	if (CheckEkrHitNow(POS_R) != 1) {
		AStack_130.pSpriteData = hp_digits_oam_array;
		AStack_130.oamBase = 0;
	} else {
		AStack_130.pSpriteData = auStack_c8;
		AStack_130.oamBase = 0x200;
		AStack_130.xPosition = AStack_130.xPosition - 8;
		AStack_130.yPosition = AStack_130.yPosition - 8;
		BanimUpdateSpriteRotScale(hp_digits_oam_array, auStack_c8, 0x100, 0x80, 1);
	}

	if (proc->unk50 == 0) {
		AnimDisplay(&AStack_130);

		if (hp_hundreds_digit[1] > 0) {
			AStack_130.oam2Base = 0x0000C000 + (enemy_hp_tilemap_index - 1);
			AStack_130.oam2Base |= proc->unk44;
			AStack_130.xPosition = x + 0x79;
			AStack_130.yPosition = y + 0x91;
			AStack_130.state2 = 0;
			AStack_130.pSpriteData = hp_digits_oam_array;
			AStack_130.oamBase = 0;
			AnimDisplay(&AStack_130);
		}
	}

	uVar15 = (r7_ - 0x28);
	uVar8 = (r6_ - 0x28);
	sVar16 = (r7_);
	sVar5 = (r6_);

	if (uVar15 > 0x28)
		uVar15 = 0x28;
	if (uVar8 > 0x28)
		uVar8 = 0x28;
	if (uVar15 < 0)
		uVar15 = 0;
	if (uVar8 < 0)
		uVar8 = 0;
	if (sVar16 > 0x28)
		sVar16 = 0x28;
	if (sVar5 > 0x28)
		sVar5 = 0x28;

	AStack_130.oam2Base = 0xb000;
	AStack_130.oam2Base |= proc->unk44;
	AStack_130.oamBase = 0;
	AStack_130.xPosition = x + 0x1d;
	AStack_130.pSpriteData = gUnknown_085B93D0;

	if (proc->unk4C == 0) {
		if (uVar8 != 0) {
			sub_8071068(auStack_e8, uVar15, uVar8);
			if (hp_changed == 1)
				sub_8050E40(auStack_e8, gUnk_Banim_02016E48);

			AStack_130.yPosition = y + 0x8e;
			AStack_130.oam2Base &= 0xfc00;
			AStack_130.oam2Base |= 0;
			AStack_130.state2 = 0;
			AnimDisplay(&AStack_130);
		}

		sub_8071068(auStack_e8, sVar16, sVar5);

		if (hp_changed == 1)
			sub_8050E40(auStack_e8, gUnk_Banim_02017248);

		if (uVar8 != 0)
			AStack_130.yPosition = y + 0x95;
		else
			AStack_130.yPosition = y + 0x91;

		AStack_130.oam2Base &= 0xfc00;
		AStack_130.oam2Base |= 0x20;
		AStack_130.state2 = 0;
		AnimDisplay(&AStack_130);
	}

	uVar15 = (r8_ - 0x28);
	uVar8 = (sp_d4 - 0x28);
	sVar16 = (r8_);
	sVar5 = (sp_d4);

	if (uVar15 > 0x28)
		uVar15 = 0x28;
	if (uVar8 > 0x28)
		uVar8 = 0x28;
	if (uVar15 < 0)
		uVar15 = 0;
	if (uVar8 < 0)
		uVar8 = 0;
	if (sVar16 > 0x28)
		sVar16 = 0x28;
	if (sVar5 > 0x28)
		sVar5 = 0x28;

	AStack_130.oam2Base = 0xc000;
	AStack_130.oam2Base |= proc->unk44;
	AStack_130.oamBase = 0;
	AStack_130.xPosition = x + 0x95;
	AStack_130.pSpriteData = gUnknown_085B93D0;

	if (proc->unk50 == 0) {
		if (uVar8 != 0) {
			sub_8071068(auStack_e8, uVar15, uVar8);
			if (hp_changed == 1)
				sub_8050E40(auStack_e8, gUnk_Banim_02017048);

			AStack_130.yPosition = y + 0x8e;
			AStack_130.oam2Base &= 0xfc00;
			AStack_130.oam2Base |= 0x10;
			AStack_130.state2 = 0;
			AnimDisplay(&AStack_130);
		}

		sub_8071068(auStack_e8, sVar16, sVar5);

		if (hp_changed == 1)
			sub_8050E40(auStack_e8, gUnk_Banim_02017448);

		if (uVar8 != 0)
			AStack_130.yPosition = y + 0x95;
		else
			AStack_130.yPosition = y + 0x91;

		AStack_130.oam2Base &= 0xfc00;
		AStack_130.oam2Base |= 0x30;
		AStack_130.state2 = 0;
		AnimDisplay(&AStack_130);
	}

	if (hp_changed == 1)
		RegisterDataMove((void *)gUnk_Banim_02016E48, (void *)0x06013000, 0x800);

	if (proc->unk4C == 0) {
		AStack_130.oamBase = 0;
		AStack_130.pSpriteData = gUnknown_085B9424;
		AStack_130.oam2Base = 0x0000B1D0;
		AStack_130.oam2Base |= proc->unk44;
		AStack_130.xPosition = x + 0x12;
		AStack_130.yPosition = y + 0x70;
		AStack_130.state2 = 0;
		AnimDisplay(&AStack_130);

		AStack_130.oamBase = 0;
		AStack_130.pSpriteData = gUnknown_085B949C;
		AStack_130.oam2Base = 0x0000B1C0;
		AStack_130.oam2Base |= proc->unk44;
		AStack_130.xPosition = x + 0x65;
		AStack_130.yPosition = y + 0x78;
		AStack_130.state2 = 0;
		AnimDisplay(&AStack_130);
	}

	if (proc->unk50 == 0) {
		AStack_130.oamBase = 0;
		AStack_130.pSpriteData = gUnknown_085B9424;
		AStack_130.oam2Base = 0x0000C1F0;
		AStack_130.oam2Base |= proc->unk44;
		AStack_130.xPosition = x + 0xd8;
		AStack_130.yPosition = y + 0x70;
		AStack_130.state2 = 0;
		AnimDisplay(&AStack_130);

		AStack_130.oamBase = 0;
		AStack_130.pSpriteData = gUnknown_085B94F0;
		AStack_130.oam2Base = 0x0000C1C0;
		AStack_130.oam2Base |= proc->unk44;
		AStack_130.xPosition = x + 0x87;
		AStack_130.yPosition = y + 0x78;
		AStack_130.state2 = 0;
		AnimDisplay(&AStack_130);
	}

	if (proc->unk4C == 0) {
		AStack_130.oamBase = 0;
		if (gBanimWtaBonus[0] != 0) {
			sub_8051238((void *)&AStack_130, gBanimWtaBonus[0], clk);
			AStack_130.oam2Base = 0x1ca;
			AStack_130.oam2Base |= proc->unk44;
			AStack_130.xPosition = x + 0x36;
			AStack_130.yPosition = y + 0x79;
			AStack_130.state2 = 0;
			AnimDisplay(&AStack_130);
		}

		AStack_130.pSpriteData = gUnknown_085B9544;
		AStack_130.oam2Base = 0x0000D1DC;
		AStack_130.oam2Base |= proc->unk44;
		AStack_130.xPosition = x + 0x2c;
		AStack_130.yPosition = y + 0x79;
		AStack_130.state2 = 0;
		AnimDisplay(&AStack_130);
	}

	if (proc->unk50 == 0) {
		AStack_130.oamBase = 0;
		if (gBanimWtaBonus[1] != 0) {
			sub_8051238((void *)&AStack_130, gBanimWtaBonus[1], clk);
			AStack_130.oam2Base = 0x1ca;
			AStack_130.oam2Base |= proc->unk44;
			AStack_130.xPosition = x + 0x85;
			AStack_130.yPosition = y + 0x79;
			AStack_130.state2 = 0;
			AnimDisplay(&AStack_130);
		}

		AStack_130.pSpriteData = gUnknown_085B9544;
		AStack_130.oam2Base = 0x0000E1DE;
		AStack_130.oam2Base |= proc->unk44;
		AStack_130.xPosition = x + 0x7b;
		AStack_130.yPosition = y + 0x79;
		AStack_130.state2 = 0;
		AnimDisplay(&AStack_130);
	}
}
