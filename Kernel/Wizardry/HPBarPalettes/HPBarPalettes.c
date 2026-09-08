#include "common-chax.h"
#include "kernel-lib.h"
#include "battle-system.h"
#include "jester_headers/custom-arrays.h"

/* Helper function to determine HP bar palette based on current HP */
static u16* GetHpBarPalette(u8 current_hp)
{
    if (!gpKernelDesignerConfig->expanded_hp)
    {
        /* Default behavior: use green palette for HP <= 80, purple otherwise */
        return (current_hp <= 80) ? gPalEfxHpBarGreen : gPalEfxHpBarPurple;
    }
    else
    {
        /* Expanded HP mode with multiple color thresholds */
        if (current_hp <= 80)
            return gPalEfxHpBarGreen;
        else if (current_hp <= 160)
            return gPalEfxHpBarYellow;
        else if (current_hp <= 240)
            return gPalEfxHpBarRed;
        else /* 241-254 */
            return gPalEfxHpBarBlue;
    }
}

/* Helper function to check if HP is in low threshold (for special effects) */
static bool IsLowHpThreshold(u8 current_hp)
{
    if (!gpKernelDesignerConfig->expanded_hp)
        return (current_hp <= 80);
    else
        return (current_hp <= 80);
}

/* This temporarily changes the HP color palette in the frames/frames the unit is struck */
LYN_REPLACE_CHECK(EfxFlashHPBarMain1);
void EfxFlashHPBarMain1(struct ProcEfxFlashing * proc)
{
    u16* palette;

    if (GetAnimPosition(proc->anim) == EKR_POS_L)
    {
        palette = GetHpBarPalette(gEkrGaugeHp[EKR_POS_L]);
        CpuCopy16(palette, PAL_OBJ(OBPAL_EFXHPBAR_L), 0x20);
    }
    else
    {
        palette = GetHpBarPalette(gEkrGaugeHp[EKR_POS_R]);
        CpuCopy16(palette, PAL_OBJ(OBPAL_EFXHPBAR_R), 0x20);
    }

    EnablePaletteSync();

    if (++proc->timer >= proc->terminator2)
        Proc_Break(proc);
}

/* Presumably this restores the palette that was temporarily changed in EfxFlashHPBarMain1 */
LYN_REPLACE_CHECK(EfxFlashHPBarRestorePal);
void EfxFlashHPBarRestorePal(struct ProcEfxFlashing * proc)
{
    u16* palette;

    if (GetAnimPosition(proc->anim) == EKR_POS_L)
    {
        if (IsLowHpThreshold(gEkrGaugeHp[EKR_POS_L]))
            CpuCopy16(&PAL_BUF_COLOR(gPalEfxHpBarGreen, gBanimFactionPal[EKR_POS_L], 0),
                      PAL_OBJ(OBPAL_EFXHPBAR_L), 0x20);
        else
        {
            palette = GetHpBarPalette(gEkrGaugeHp[EKR_POS_L]);
            CpuCopy16(palette, PAL_OBJ(OBPAL_EFXHPBAR_L), 0x20);
        }
    }
    else
    {
        if (IsLowHpThreshold(gEkrGaugeHp[EKR_POS_R]))
            CpuCopy16(&PAL_BUF_COLOR(gPalEfxHpBarGreen, gBanimFactionPal[EKR_POS_R], 0),
                      PAL_OBJ(OBPAL_EFXHPBAR_R), 0x20);
        else
        {
            palette = GetHpBarPalette(gEkrGaugeHp[EKR_POS_R]);
            CpuCopy16(palette, PAL_OBJ(OBPAL_EFXHPBAR_R), 0x20);
        }
    }

    EnablePaletteSync();
    Proc_Break(proc);
}

/* This is the function that holds the idle color state of the HP bars */
LYN_REPLACE_CHECK(EfxHPBarColorChangeMain);
void EfxHPBarColorChangeMain(struct ProcEfxHpBarColorChange * proc)
{
    int ret;
    u8 *buf1, *buf2;
    s16 *buf3;
    u16* palette;

    if (proc->disabled == true)
        return;

    ret = EfxAdvanceFrameLut(&proc->timer1, (s16 *)&proc->frame1, (s16 *)proc->frame_lut1);
    if (ret >= 0)
        proc->unk54 = ret;

    ret = EfxAdvanceFrameLut(&proc->timer2, (s16 *)&proc->frame2, (s16 *)proc->frame_lut2);
    if (ret >= 0)
        proc->unk58 = ret;

    /* Left HP bar */
    if (IsLowHpThreshold(gEkrGaugeHp[EKR_POS_L]))
    {
        buf1 = gEfxSplitedColorBufA;
        buf2 = gEfxSplitedColorBufB;
        buf3 = gEfxSplitedColorBufC;

        EfxDecodeSplitedPalette(
            PAL_OBJ(OBPAL_EFXHPBAR_L),
            (s8 *)gEfxSplitedColorBufA,
            (s8 *)gEfxSplitedColorBufB,
            (s16 *)gEfxSplitedColorBufC,
            0x10,
            proc->unk54,
            5);
    }
    else
    {
        palette = GetHpBarPalette(gEkrGaugeHp[EKR_POS_L]);
        /* Remove flashing effect if expanded_hp is enabled */
        if (gpKernelDesignerConfig->expanded_hp)
            CpuFastCopy(palette, PAL_OBJ(OBPAL_EFXHPBAR_L), 0x20);
        else
            CpuFastCopy(palette + proc->unk58 * 0x10, PAL_OBJ(OBPAL_EFXHPBAR_L), 0x20);
    }

    /* Right HP bar */
    if (IsLowHpThreshold(gEkrGaugeHp[EKR_POS_R]))
    {
        buf1 = gEfxSplitedColorBufD;
        buf2 = gEfxSplitedColorBufE;
        buf3 = gEfxSplitedColorBufF;

        EfxDecodeSplitedPalette(
            PAL_OBJ(OBPAL_EFXHPBAR_R),
            (s8 *)buf1,
            (s8 *)buf2,
            (s16 *)buf3,
            0x10, proc->unk54,
            5);
    }
    else
    {
        palette = GetHpBarPalette(gEkrGaugeHp[EKR_POS_R]);
        /* Remove flashing effect if expanded_hp is enabled */
        if (gpKernelDesignerConfig->expanded_hp)
            CpuFastCopy(palette, PAL_OBJ(OBPAL_EFXHPBAR_R), 0x20);
        else
            CpuFastCopy(palette + proc->unk58 * 0x10, PAL_OBJ(OBPAL_EFXHPBAR_R), 0x20);
    }

    EnablePaletteSync();
}

/* This displays the initial color of the HP bar as it's being initialized (with other graphics), before switching to EfxHPBarColorChangeMain */
LYN_REPLACE_CHECK(NewEkrGauge);
void NewEkrGauge(void)
{
    u32 i, j;
    u16* palette_left;
    u16* palette_right;

    gpProcEkrGauge = Proc_Start(ProcScr_ekrGauge, PROC_TREE_1);

    EkrGauge_Setup44(0);
    EkrGauge_Clr4C50();
    DisableEkrGauge();
    EkrGauge_ClrInitFlag();
    EkrGauge_Clr323A(gEkrBg0QuakeVec.x, gEkrBg0QuakeVec.y);

    /* Determine palette for left side */
    if (!gpKernelDesignerConfig->expanded_hp)
    {
        /* Original logic had the condition inverted - fixing it here */
        if (gEkrGaugeHp[0] <= 80)
            palette_left = gPalEfxHpBarGreen;
        else
            palette_left = gPalEfxHpBarPurple + gBanimFactionPal[POS_L] * 0x10;
    }
    else
    {
        palette_left = GetHpBarPalette(gEkrGaugeHp[0]);
    }
    CpuCopy16(palette_left, PAL_OBJ(0xB), 0x10 * sizeof(u16));

    /* Determine palette for right side */
    if (!gpKernelDesignerConfig->expanded_hp)
    {
        /* Original logic had the condition inverted - fixing it here */
        if (gEkrGaugeHp[1] <= 80)
            palette_right = gPalEfxHpBarGreen;
        else
            palette_right = gPalEfxHpBarPurple + gBanimFactionPal[POS_R] * 0x10;
    }
    else
    {
        palette_right = GetHpBarPalette(gEkrGaugeHp[1]);
    }
    CpuCopy16(palette_right, PAL_OBJ(0xC), 0x10 * sizeof(u16));

    gEkrGaugeHpBak[0] = -1;
    gEkrGaugeHpBak[1] = -1;

    LZ77UnCompVram(Img_EfxSideHitDmgCrit, (void *)0x6013800);
    LZ77UnCompVram(Img_EfxWTAArrow1, (void *)0x6013940);
    LZ77UnCompVram(Img_EfxWTAArrow2, (void *)0x6013D40);

    /* These are the palette colors for the NO DAMAGE, MISS etc messages */
    CpuFastCopy(gUnknown_08802884 + gBanimFactionPal[POS_L] * 0x10, PAL_OBJ(0x5), 0x10 * sizeof(u16));
    CpuFastCopy(gUnknown_08802884 + gBanimFactionPal[POS_R] * 0x10, PAL_OBJ(0x6), 0x10 * sizeof(u16));

    EnablePaletteSync();

    /* decode value to number for display: 998 --> 9 9 8 */
    ModDec(gEkrGaugeHit[0], &gEkrGaugeDecoder[0x0]);
    ModDec(gEkrGaugeDmg[0], &gEkrGaugeDecoder[0x3]);
    ModDec(gEkrGaugeCrt[0], &gEkrGaugeDecoder[0x6]);

    ModDec(gEkrGaugeHit[1], &gEkrGaugeDecoder[0x9]);
    ModDec(gEkrGaugeDmg[1], &gEkrGaugeDecoder[0xC]);
    ModDec(gEkrGaugeCrt[1], &gEkrGaugeDecoder[0xF]);

    CpuFastFill(0, gObjBuf_EkrSideHitDmgCrit, 0x400);

    /* value of hit & dmg & crit */
    for (i = 0; i < 6; i++) {
        for (j = 0; j < 3; j++) {
            int r4 = i * 0x40 + j * 0x10;

            CpuCopy16(
                gUnknown_088026E4 + gEkrGaugeDecoder[i * 3 + j] * 0x10,
                gObjBuf_EkrSideHitDmgCrit + r4,
                0x10 * sizeof(u16));
        }
    }

    /* left side of hit & dmg & crit */
    RegisterDataMove(gObjBuf_EkrSideHitDmgCrit, (void *)0x6013A00, 0xC0 * sizeof(u16));

    /* right side of hit & dmg & crit */
    RegisterDataMove(gObjBuf_EkrSideHitDmgCrit + 0xC0, (void *)0x6013E00, 0xC0 * sizeof(u16));

    ResetIconGraphics_();
    LoadIconPalette(0, 0x1D);
    LoadIconPalette(0, 0x1E);
    LoadIconObjectGraphics(GetItemIconId(gpEkrBattleUnitLeft->weaponBefore), 0x1DC);
    LoadIconObjectGraphics(GetItemIconId(gpEkrBattleUnitRight->weaponBefore), 0x1DE);
    ApplyPalette(gPal_MiscUiGraphics, 0x10);
}

//Original
LYN_REPLACE_CHECK(ekrGaugeMain);
void ekrGaugeMain(struct ProcEkrGauge * proc)
{
    struct Anim AStack_130;
    u16 auStack_e8[12];
    u16 local_d0[6];  // Changed from [4] to [6] to support 3 digits per side
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
    u8 hp_threshold = 99;
    bool expanded_hp = gpKernelDesignerConfig->expanded_hp;
    u16 player_hp_tilemap_index = expanded_hp ? 0x1E1 : 0x1CE;
    u16 enemy_hp_tilemap_index = expanded_hp ? 0x1E5 : 0x1EE;
    s32 hp_hundreds_digit[2] = {0, 0};  // For enemy 0 and player 1
    u8 * hp_digits_oam_array = gUnknown_085B940C;

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
            EfxTmCpyBG(gUnknown_08802274 + r8, &gBG0TilemapBuffer[r7 + r9], 15, r6, -1, -1); /* Jester - gBG0TilemapBuffer2D is not respected so I removed the macro */
            sub_8070D04(&gBG0TilemapBuffer[r7 + r9], 15, r6, 2, 0x80); /* Jester - gBG0TilemapBuffer2D is not respected so I removed the macro */
        }

        if (0 == proc->unk50) {
            void *ptr;

            if (0 == proc->unk4C)
                ptr = gUnknown_08802348 + r8;
            else
                ptr = gUnknown_08802428 + r8;

            EfxTmCpyBG(ptr, &gBG0TilemapBuffer[r7 + spDC], 16, r6, -1, -1); /* Jester - gBG0TilemapBuffer2D is not respected so I removed the macro */
            sub_8070D04(&gBG0TilemapBuffer[r7 + spDC], 16, r6, 3, 128); /* Jester - gBG0TilemapBuffer2D is not respected so I removed the macro */
        }

        BG_EnableSyncByMask(1);
    }

    if (gEkrGaugeHpBak[0] != gEkrGaugeHp[0])
        hp_changed = 1;

    if (gEkrGaugeHpBak[1] != gEkrGaugeHp[1])
        hp_changed = 1;

    gEkrGaugeHpBak[0] = gEkrGaugeHp[0];
    gEkrGaugeHpBak[1] = gEkrGaugeHp[1];

    r7_ = gEkrGaugeHp[0];
    r6_ = gBanimMaxHP[0];
    r8_ = gEkrGaugeHp[1];
    sp_d4 = gBanimMaxHP[1];

    switch (gEkrDistanceType) {
        case 3:
            if (gBanimValid[POS_L] == 1) {
                x = proc->unk32 + 0x38;
            } else {
                x = proc->unk32 - 0x38;
            }
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


    if (proc->battle_init == 0) {
        y = proc->unk3A & 0xFFF8;
    } else {
        y = proc->unk3A;
    }

    // Calculate HP digits based on expanded_hp setting
    if (expanded_hp) {
        // Calculate hundreds digit for enemy
        if (gEkrGaugeHp[0] >= 200) {
            hp_hundreds_digit[0] = 2;
            local_d0[0] = Div(gEkrGaugeHp[0] - 200, 10);
            local_d0[1] = (gEkrGaugeHp[0] - 200) - local_d0[0] * 10;
        } else if (gEkrGaugeHp[0] >= 100) {
            hp_hundreds_digit[0] = 1;
            local_d0[0] = Div(gEkrGaugeHp[0] - 100, 10);
            local_d0[1] = (gEkrGaugeHp[0] - 100) - local_d0[0] * 10;
        } else {
            hp_hundreds_digit[0] = 0;
            local_d0[0] = Div(gEkrGaugeHp[0], 10);
            local_d0[1] = gEkrGaugeHp[0] - local_d0[0] * 10;
        }
        
        // Hide leading zero for tens place
        if (local_d0[0] == 0 && hp_hundreds_digit[0] == 0) {
            local_d0[0] = 0xb;
        }

        // Calculate hundreds digit for player
        if (gEkrGaugeHp[1] >= 200) {
            hp_hundreds_digit[1] = 2;
            local_d0[2] = Div(gEkrGaugeHp[1] - 200, 10);
            local_d0[3] = (gEkrGaugeHp[1] - 200) - local_d0[2] * 10;
        } else if (gEkrGaugeHp[1] >= 100) {
            hp_hundreds_digit[1] = 1;
            local_d0[2] = Div(gEkrGaugeHp[1] - 100, 10);
            local_d0[3] = (gEkrGaugeHp[1] - 100) - local_d0[2] * 10;
        } else {
            hp_hundreds_digit[1] = 0;
            local_d0[2] = Div(gEkrGaugeHp[1], 10);
            local_d0[3] = gEkrGaugeHp[1] - local_d0[2] * 10;
        }
        
        // Hide leading zero for tens place
        if (local_d0[2] == 0 && hp_hundreds_digit[1] == 0) {
            local_d0[2] = 0xb;
        }
    } else {
        // Original 2-digit support
        local_d0[0] = Div(gEkrGaugeHp[0], 10);
        local_d0[1] = gEkrGaugeHp[0] - local_d0[0] * 10;

        if (local_d0[0] == 0) {
            local_d0[0] = 0xb;
        }

        local_d0[2] = Div(gEkrGaugeHp[1], 10);
        local_d0[3] = gEkrGaugeHp[1] - local_d0[2] * 10;

        if (local_d0[2] == 0) {
            local_d0[2] = 0xb;
        }

        /* If the unit's HP is over the threshold, display "?" (0xc) */
        if (gEkrGaugeHp[0] > hp_threshold) {
            local_d0[0] = 0xc;
            local_d0[1] = 0xc;
        }

        if (gEkrGaugeHp[1] > hp_threshold) {
            local_d0[2] = 0xc;
            local_d0[3] = 0xc;
        }
    }

    if (hp_changed == 1) {
        s32 i;
        s32 j;

        CpuFastFill(0, gUnk_Banim_02016DC8, 0x80);

        for (i = 0; i < 2; i++) {
            for (j = 0; j < 2; j++) {  // Always use 2 digits for the main display
                CpuCopy16(
                    gUnknown_088026E4 + local_d0[i * 2 + j] * 0x10,
                    (u16 *)gUnk_Banim_02016DC8 + ((i * 0x20) + (j * 0x10)),
                    0x20
                );
            }
        }

        RegisterDataMove(gUnk_Banim_02016DC8, (void *)(0x06010000 + (player_hp_tilemap_index * 0x20)), 0x40);
        RegisterDataMove((u16 *)gUnk_Banim_02016DC8 + 0x20, (void *)(0x06010000 + (enemy_hp_tilemap_index * 0x20)), 0x40);
        
        // Handle hundreds digit sprites if expanded_hp is enabled
        if (expanded_hp) {
            if (hp_hundreds_digit[0] > 0) {
                // Point directly to the digit graphic in ROM
                void* pSrc = (void*)(gUnknown_088026E4 + (hp_hundreds_digit[0] * 0x10));
                void* pDest = (void*)(0x06010000 + ((player_hp_tilemap_index - 1) * 0x20));
                
                RegisterDataMove(pSrc, pDest, 0x20);
            }
            
            if (hp_hundreds_digit[1] > 0) {
                // Point directly to the digit graphic in ROM
                void* pSrc = (void*)(gUnknown_088026E4 + (hp_hundreds_digit[1] * 0x10));
                void* pDest = (void*)(0x06010000 + ((enemy_hp_tilemap_index - 1) * 0x20));
                
                RegisterDataMove(pSrc, pDest, 0x20);
            }
        }
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

    /* Display enemy HP digits */
    if (proc->unk4C == 0) {
        AnimDisplay(&AStack_130);
        
        // Display hundreds digit if needed
        if (expanded_hp && hp_hundreds_digit[0] > 0) {
            AStack_130.oam2Base = 0x0000B000 + (player_hp_tilemap_index - 1);
            AStack_130.oam2Base |= proc->unk44;
            AStack_130.xPosition = x + 1;  // Position before the other digits
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

    /* Display player HP digits */
    if (proc->unk50 == 0) {
        AnimDisplay(&AStack_130);
        
        // Display hundreds digit if needed
        if (expanded_hp && hp_hundreds_digit[1] > 0) {
            AStack_130.oam2Base = 0x0000C000 + (enemy_hp_tilemap_index - 1);
            AStack_130.oam2Base |= proc->unk44;
            AStack_130.xPosition = x + 0x79;  // Position before player digits
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

    if (uVar15 > 0x28) {
        uVar15 = 0x28;
    }

    if (uVar8 > 0x28) {
        uVar8 = 0x28;
    }

    if ((uVar15) < 0) {
        uVar15 = 0;
    }

    if ((uVar8) < 0) {
        uVar8 = 0;
    }

    if ((sVar16) > 0x28) {
        sVar16 = 0x28;
    }

    if (sVar5 > 0x28) {
        sVar5 = 0x28;
    }

    AStack_130.oam2Base = 0xb000;
    AStack_130.oam2Base |= proc->unk44;

    AStack_130.oamBase = 0;
    AStack_130.xPosition = x + 0x1d;
    AStack_130.pSpriteData = gUnknown_085B93D0;

    if (proc->unk4C == 0) {
        if (uVar8 != 0) {
            sub_8071068(auStack_e8, uVar15, uVar8);
            if (hp_changed == 1) {
                sub_8050E40(auStack_e8, gUnk_Banim_02016E48);
            }

            AStack_130.yPosition = y + 0x8e;
            AStack_130.oam2Base &= 0xfc00;
            AStack_130.oam2Base |= 0;
            AStack_130.state2 = 0;

            /* Display unit names */
            AnimDisplay(&AStack_130);
        }

        sub_8071068(auStack_e8, sVar16, sVar5);

        if (hp_changed == 1) {
            sub_8050E40(auStack_e8, gUnk_Banim_02017248);
        }

        if (uVar8 != 0) {
            AStack_130.yPosition = y + 0x95;
        } else {
            AStack_130.yPosition = y + 0x91;
        }

        AStack_130.oam2Base &= 0xfc00;
        AStack_130.oam2Base |= 0x20;
        AStack_130.state2 = 0;

        /* Display enemy HP bar */
          AnimDisplay(&AStack_130);
    }

    uVar15 = (r8_ - 0x28);
    uVar8 = (sp_d4 - 0x28);
    sVar16 = (r8_);
    sVar5 = (sp_d4);

    if (uVar15 > 0x28) {
        uVar15 = 0x28;
    }

    if (uVar8 > 0x28) {
        uVar8 = 0x28;
    }

    if ((uVar15) < 0) {
        uVar15 = 0;
    }

    if ((uVar8) < 0) {
        uVar8 = 0;
    }

    if (sVar16 > 0x28) {
        sVar16 = 0x28;
    }

    if (sVar5 > 0x28) {
        sVar5 = 0x28;
    }

    AStack_130.oam2Base = 0xc000;
    AStack_130.oam2Base |= proc->unk44;

    AStack_130.oamBase = 0;
    AStack_130.xPosition = x + 0x95;
    AStack_130.pSpriteData = gUnknown_085B93D0;

    if (proc->unk50 == 0) {
        if (uVar8 != 0) {
            sub_8071068(auStack_e8, uVar15, uVar8);
            if (hp_changed == 1) {
                sub_8050E40(auStack_e8, gUnk_Banim_02017048);
            }

            AStack_130.yPosition = y + 0x8e;
            AStack_130.oam2Base &= 0xfc00;
            AStack_130.oam2Base |= 0x10;
            AStack_130.state2 = 0;

            /* Display second player HP bar */
            AnimDisplay(&AStack_130);
        }

        sub_8071068(auStack_e8, sVar16, sVar5);

        if (hp_changed == 1) {
            sub_8050E40(auStack_e8, gUnk_Banim_02017448);
        }

        if (uVar8 != 0) {
            AStack_130.yPosition = y + 0x95;
        } else {
            AStack_130.yPosition = y + 0x91;
        }

        AStack_130.oam2Base &= 0xfc00;
        AStack_130.oam2Base |= 0x30;
        AStack_130.state2 = 0;

        /* Display first player HP bar */
        AnimDisplay(&AStack_130);
    }

    if (hp_changed == 1) {
        RegisterDataMove((void *)gUnk_Banim_02016E48, (void *)0x06013000, 0x800);
    }

    if (proc->unk4C == 0) {
        AStack_130.oamBase = 0;
        AStack_130.pSpriteData = gUnknown_085B9424;
        AStack_130.oam2Base = 0x0000B1D0;
        AStack_130.oam2Base |= proc->unk44;

        AStack_130.xPosition = x + 0x12;
        AStack_130.yPosition = y + 0x70;
        AStack_130.state2 = 0;
        AnimDisplay(&AStack_130); // Display enemy stat numbers
        AStack_130.oamBase = 0;

        AStack_130.pSpriteData = gUnknown_085B949C;
        AStack_130.oam2Base = 0x0000B1C0;
        AStack_130.oam2Base |= proc->unk44;

        AStack_130.xPosition = x + 0x65;
        AStack_130.yPosition = y + 0x78;
        AStack_130.state2 = 0;
        AnimDisplay(&AStack_130); // Display enemy stat labels (hit, dmg, crt)
    }

    if (proc->unk50 == 0) {
        AStack_130.oamBase = 0;
        AStack_130.pSpriteData = gUnknown_085B9424;
        AStack_130.oam2Base = 0x0000C1F0;
        AStack_130.oam2Base |= proc->unk44;

        AStack_130.xPosition = x + 0xd8;
        AStack_130.yPosition = y + 0x70;
        AStack_130.state2 = 0;
        AnimDisplay(&AStack_130); // Display player stat numbers

        AStack_130.oamBase = 0;
        AStack_130.pSpriteData = gUnknown_085B94F0;
        AStack_130.oam2Base = 0x0000C1C0;
        AStack_130.oam2Base |= proc->unk44;

        AStack_130.xPosition = x + 0x87;
        AStack_130.yPosition = y + 0x78;
        AStack_130.state2 = 0;
        AnimDisplay(&AStack_130); // Display player stat labels (hit, dmg, crt)
    }

    if (proc->unk4C == 0) {
        AStack_130.oamBase = 0;
        if (gBanimWtaBonus[0] != 0) {
            sub_8051238((void*)&AStack_130, gBanimWtaBonus[0], clk);
            AStack_130.oam2Base = 0x1ca;
            AStack_130.oam2Base |= proc->unk44;

            AStack_130.xPosition = x + 0x36;
            AStack_130.yPosition = y + 0x79;
            AStack_130.state2 = 0;
            AnimDisplay(&AStack_130); // Doesn't seem to be anything I can see? Maybe the flash/effectiveness arrow on enemy weapons?
        }

        AStack_130.pSpriteData = gUnknown_085B9544;
        AStack_130.oam2Base = 0x0000D1DC;
        AStack_130.oam2Base |= proc->unk44;

        AStack_130.xPosition = x + 0x2c;
        AStack_130.yPosition = y + 0x79;
        AStack_130.state2 = 0;
        AnimDisplay(&AStack_130); // Display enemy weapon icon
    }

    if (proc->unk50 == 0) {
        AStack_130.oamBase = 0;
        if (gBanimWtaBonus[1] != 0) {

            sub_8051238((void*)&AStack_130, gBanimWtaBonus[1], clk);
            AStack_130.oam2Base = 0x1ca;
            AStack_130.oam2Base |= proc->unk44;

            AStack_130.xPosition = x + 0x85;
            AStack_130.yPosition = y + 0x79;
            AStack_130.state2 = 0;
            AnimDisplay(&AStack_130); // Doesn't seem to be anything I can see? Maybe the flash/effectiveness arrow on player weapons?
        }

        AStack_130.pSpriteData = gUnknown_085B9544;
        AStack_130.oam2Base = 0x0000E1DE;
        AStack_130.oam2Base |= proc->unk44;

        AStack_130.xPosition = x + 0x7b;
        AStack_130.yPosition = y + 0x79;
        AStack_130.state2 = 0;
        AnimDisplay(&AStack_130); // Display player weapon icon
    }

    return;
}

/* Apprently if I don't hook this, then the expanded HP caps at the signed value of 127 and then goes negative :/ */
LYN_REPLACE_CHECK(PrepareBattleGraphicsMaybe);
bool PrepareBattleGraphicsMaybe(void)
{
    u16 i;
    u16 pid, jid;
    void * zero;
    struct Unit * unit_bu1;
    struct Unit * unit_bu2;
    struct BattleUnit * bu1;
    struct BattleUnit * bu2;
    const struct CharacterData * pinfo1;
    const struct CharacterData * pinfo2;
    int usrdefined_enable;
    const void * animdef1;
    const void * animdef2;
    s16 valid_l;
    s16 valid_r;
    u32 animid1, animid2;

    int char_cnt = 1;

    ResetEkrDragonStatus();

    if (!(gBattleStats.config & BATTLE_CONFIG_ARENA))
        SetBanimArenaFlag(false);
    else
        SetBanimArenaFlag(true);

    if (!(gBmSt.gameStateBits & BM_FLAG_LINKARENA))
        SetBanimLinkArenaFlag(false);
    else
        SetBanimLinkArenaFlag(true);

    if (gBattleStats.config & BATTLE_CONFIG_PROMOTION)
        gEkrDistanceType = EKR_DISTANCE_PROMOTION;
    else
        gEkrDistanceType = EKR_DISTANCE_CLOSE;

    if (gEkrDistanceType == EKR_DISTANCE_PROMOTION)
    {
        bu1 = gpEkrBattleUnitLeft = &gBattleActor;
        bu2 = gpEkrBattleUnitRight = &gBattleTarget;

        gBanimPositionIsEnemy[POS_L] = gBanimPositionIsEnemy[POS_R] = 0;
        gBanimValid[EKR_POS_R] = gBanimValid[EKR_POS_L] = true;
    }
    else
    {
        u8 i1 = -0x40 & gBattleActor.unit.index;
        u16 faction1 = GetBanimFactionPalette(i1);
        u8 i2 = -0x40 & gBattleTarget.unit.index;
        u16 faction2 = GetBanimFactionPalette(i2);

        if (gBattleStats.config & BATTLE_CONFIG_REFRESH)
            char_cnt = 2;
        else if (gBattleActor.weaponBefore == ITEM_NONE)
            char_cnt = 2;
        else
            char_cnt = GetSpellAssocCharCount(GetItemIndex(gBattleActor.weaponBefore));

        gBanimValid[EKR_POS_L] = gBanimValid[EKR_POS_R] = true;

        if (EKR_POS_R == GetBanimAllyPosition(faction1, faction2))
        {
            bu1 = gpEkrBattleUnitLeft = &gBattleTarget;
            bu2 = gpEkrBattleUnitRight = &gBattleActor;

            gBanimPositionIsEnemy[POS_L] = true;
            gBanimPositionIsEnemy[POS_R] = false;

            if (char_cnt == 1)
                gBanimValid[EKR_POS_L] = false;
        }
        else
        {
            bu1 = gpEkrBattleUnitLeft = &gBattleActor;
            bu2 = gpEkrBattleUnitRight = &gBattleTarget;

            gBanimPositionIsEnemy[POS_L] = false;
            gBanimPositionIsEnemy[POS_R] = true;

            if (char_cnt == 1)
                gBanimValid[EKR_POS_R] = false;
        }
    }

    unit_bu1 = &bu1->unit;
    unit_bu2 = &bu2->unit;

    pinfo1 = unit_bu1->pCharacterData;
    pinfo2 = unit_bu2->pCharacterData;

    animdef1 = animdef2 = 0;

    valid_l = gBanimValid[POS_L];
    valid_r = gBanimValid[POS_R];

    if (valid_l)
        animdef1 = unit_bu1->pClassData->pBattleAnimDef;

    if (valid_r)
        animdef2 = unit_bu2->pClassData->pBattleAnimDef;

    if (valid_l)
    {
        gEkrBmLocation[POS_L] = (16 * unit_bu1->xPos - gBmSt.camera.x) >> 4;
        gEkrBmLocation[POS_R] = (16 * unit_bu1->yPos - gBmSt.camera.y) >> 4;
    }

    if (valid_r)
    {
        gEkrBmLocation[2] = (16 * unit_bu2->xPos - gBmSt.camera.x) >> 4;
        gEkrBmLocation[3] = (16 * unit_bu2->yPos - gBmSt.camera.y) >> 4;
    }

    if (gEkrDistanceType != EKR_DISTANCE_PROMOTION)
    {
        if (GetItemAttributes(gBattleActor.weaponBefore) & IA_UNCOUNTERABLE)
            gEkrDistanceType = EKR_DISTANCE_FARFAR;
        else
        {
            gEkrDistanceType = EKR_DISTANCE_MONOCOMBAT;

            if (valid_l + valid_r == 2)
            {
                s16 x_distance, y_distance;
                x_distance = ABS(gEkrBmLocation[POS_L] - gEkrBmLocation[2]);
                y_distance = ABS(gEkrBmLocation[1] - gEkrBmLocation[3]);

                if (x_distance + y_distance <= 1)
                {
                    gEkrDistanceType = EKR_DISTANCE_CLOSE;
                }
                else if (x_distance + y_distance <= 3)
                {
                    gEkrDistanceType = EKR_DISTANCE_FAR;
                }
                else
                {
                    gEkrDistanceType = EKR_DISTANCE_FARFAR;
                }
            }
        }
    }

    if (gEkrDistanceType == EKR_DISTANCE_PROMOTION)
    {
        gBanimIdx[POS_L] = gBanimIdx_bak[POS_L] = GetBattleAnimationId(unit_bu1, animdef1, bu1->weapon, &animid1);
        gBanimIdx[POS_R] = gBanimIdx_bak[POS_R] = GetBattleAnimationId(unit_bu2, animdef2, bu2->weapon, &animid2);
    }
    else
    {
        if (valid_l)
        {
            gBanimIdx[POS_L] = gBanimIdx_bak[POS_L] = GetBattleAnimationId(unit_bu1, animdef1, bu1->weaponBefore, &animid1);
        }

        if (valid_r)
        {
            gBanimIdx[POS_R] = gBanimIdx_bak[POS_R] = GetBattleAnimationId(unit_bu2, animdef2, bu2->weaponBefore, &animid2);
        }
    }

    pid = unit_bu1->pCharacterData->number - 1;
    jid = unit_bu1->pClassData->number;

    if (valid_l)
        gBanimUniquePal[POS_L] = -1;

    for (i = 0; i < 7; i++)
    {
        if (gAnimCharaPalConfig[pid][i] == jid && valid_l)
        {
            gBanimUniquePal[POS_L] = gAnimCharaPalIt[pid][i] - 1;
            break;
        }
    }

    pid = unit_bu2->pCharacterData->number - 1;
    jid = unit_bu2->pClassData->number;

    if (valid_r)
        gBanimUniquePal[POS_R] = -1;

    for (i = 0; i < 7; i++)
    {
        if (gAnimCharaPalConfig[pid][i] == jid && valid_r)
        {
            gBanimUniquePal[POS_R] = gAnimCharaPalIt[pid][i] - 1;
            break;
        }
    }

    if (valid_l)
        gBanimTriAtkPalettes[POS_L] = (void *)FilterBattleAnimCharacterPalette(gBanimIdx[POS_L], bu1->weaponBefore);

    if (valid_r)
        gBanimTriAtkPalettes[POS_R] = (void *)FilterBattleAnimCharacterPalette(gBanimIdx[POS_R], bu2->weaponBefore);

    gBanimTerrain[POS_L] = bu1->terrainId;
    gBanimTerrain[POS_R] = bu2->terrainId;

    gBanimFloorfx[POS_L] = gBanimFloorfx[POS_R] = -1;

    if (valid_l)
        gBanimFloorfx[POS_L] =
            GetBanimTerrainGround(bu1->terrainId, GetROMChapterStruct(gPlaySt.chapterIndex)->battleTileSet);

    if (valid_r)
        gBanimFloorfx[POS_R] =
            GetBanimTerrainGround(bu2->terrainId, GetROMChapterStruct(gPlaySt.chapterIndex)->battleTileSet);

    if (gBmSt.gameStateBits & BM_FLAG_LINKARENA)
    {
        gBanimTerrain[POS_R] = gBanimTerrain[POS_L] = TERRAIN_ARENA_30;

        if (valid_l)
            gBanimFloorfx[POS_L] =
                GetBanimTerrainGround(gBanimTerrain[POS_L], GetROMChapterStruct(gPlaySt.chapterIndex)->battleTileSet);

        if (valid_r)
            gBanimFloorfx[POS_R] =
                GetBanimTerrainGround(gBanimTerrain[POS_R], GetROMChapterStruct(gPlaySt.chapterIndex)->battleTileSet);
    }

    if (CheckBanimHensei() == true)
    {
        gBanimFloorfx[POS_L] = gBanimFloorfx[POS_R] = 20;
        gBanimTerrain[POS_L] = gBanimTerrain[POS_R] = TERRAIN_ARENA_30;
    }

    switch (gEkrDistanceType)
    {
        case EKR_DISTANCE_CLOSE:
        case EKR_DISTANCE_FAR:
        case EKR_DISTANCE_FARFAR:
        case EKR_DISTANCE_MONOCOMBAT:
            break;

        case EKR_DISTANCE_PROMOTION:
            gBanimFloorfx[POS_L] = gBanimFloorfx[POS_R];
            break;
    }

    switch (gPlaySt.chapterWeatherId)
    {
        case WEATHER_SNOW:
        case WEATHER_SNOWSTORM:
            gEkrSnowWeather = 1;
            break;

        default:
            gEkrSnowWeather = 0;
            break;
    }

    if (valid_l)
        gBanimCon[POS_L] = unit_bu1->pClassData->baseCon;

    if (valid_r)
        gBanimCon[POS_R] = unit_bu2->pClassData->baseCon;

    if (valid_l)
    {
        gEkrGaugeHp[POS_L] = bu1->hpInitial;
        gBanimMaxHP[POS_L] = unit_bu1->maxHP;
    }

    if (valid_r)
    {
        gEkrGaugeHp[POS_R] = bu2->hpInitial;
        gBanimMaxHP[POS_R] = unit_bu2->maxHP;
    }

    ParseBattleHitToBanimCmd();

    if (gEkrDistanceType == EKR_DISTANCE_PROMOTION)
    {
        gEkrSpellAnimIndex[POS_R] = 1;
        gEkrSpellAnimIndex[POS_L] = 1;
    }
    else
    {
        if (valid_l)
            gEkrSpellAnimIndex[POS_L] = GetSpellAnimId(unit_bu1->pClassData->number, bu1->weaponBefore);

        if (valid_r)
            gEkrSpellAnimIndex[POS_R] = GetSpellAnimId(unit_bu2->pClassData->number, bu2->weaponBefore);

        if (gBattleStats.config & BATTLE_CONFIG_REFRESH)
            if (!IsItemDisplayedInBattle(bu2->weaponBefore))
                if (unit_bu2->pClassData->number == CLASS_DANCER)
                    gEkrSpellAnimIndex[POS_R] = 0xF;
    }

    if (valid_l)
        UnsetMapStaffAnim(&gEkrSpellAnimIndex[POS_L], 0, bu1->weaponBefore);

    if (valid_r)
        UnsetMapStaffAnim(&gEkrSpellAnimIndex[POS_R], 1, bu2->weaponBefore);

    switch (gEkrDistanceType)
    {
        case EKR_DISTANCE_CLOSE:
        case EKR_DISTANCE_FAR:
        case EKR_DISTANCE_FARFAR:
            switch (unit_bu1->pClassData->number)
            {
                case CLASS_DRACO_ZOMBIE:
                    SetEkrDragonStatusType(gAnims[POS_L], EKRDRGON_TYPE_DRACO_ZOMBIE);
                    break;

                case CLASS_DEMON_KING:
                    SetEkrDragonStatusType(gAnims[POS_L], EKRDRGON_TYPE_DEMON_KING);
                    break;
            }

            break;

        case EKR_DISTANCE_MONOCOMBAT:
        case EKR_DISTANCE_PROMOTION:
            break;

        default:
            break;
    }

    if (valid_l)
    {
        u8 i1 = -0x40 & unit_bu1->index;
        gBanimFactionPal[POS_L] = GetBanimFactionPalette(i1);
    }

    if (valid_r)
    {
        u8 i2 = -0x40 & unit_bu2->index;
        gBanimFactionPal[POS_R] = GetBanimFactionPalette(i2);
    }

    gEkrPids[POS_R] = 0;
    gEkrPids[POS_L] = 0;

    if (valid_l)
        gEkrPids[POS_L] = pinfo1->number;

    if (valid_r)
        gEkrPids[POS_R] = pinfo2->number;

    if (valid_l)
        gEkrGaugeHit[POS_L] = GetDisplayedTrueHitRate(bu1->battleEffectiveHitRate);

    if (valid_r)
        gEkrGaugeHit[POS_R] = GetDisplayedTrueHitRate(bu2->battleEffectiveHitRate);

    if (gEkrGaugeHit[POS_L] == 0xFF)
        gEkrGaugeHit[POS_L] = -1;

    if (gEkrGaugeHit[POS_R] == 0xFF)
        gEkrGaugeHit[POS_R] = -1;

    if (valid_l)
    {
        gEkrGaugeDmg[POS_L] = bu1->battleAttack - bu2->battleDefense;
        if (gEkrGaugeDmg[POS_L] < 0)
            gEkrGaugeDmg[POS_L] = 0;

        if (bu1->battleAttack == 0xFF)
            gEkrGaugeDmg[POS_L] = -1;

        if (GetItemIndex(bu1->weapon) == ITEM_MONSTER_STONE)
            gEkrGaugeDmg[POS_L] = -1;
    }

    if (valid_r)
    {
        gEkrGaugeDmg[POS_R] = bu2->battleAttack - bu1->battleDefense;
        if (gEkrGaugeDmg[POS_R] < 0)
            gEkrGaugeDmg[POS_R] = 0;

        if (bu2->battleAttack == 0xFF)
            gEkrGaugeDmg[POS_R] = -1;

        if (GetItemIndex(bu2->weapon) == ITEM_MONSTER_STONE)
            gEkrGaugeDmg[POS_R] = -1;
    }

    if (valid_l)
        gEkrGaugeCrt[POS_L] = bu1->battleEffectiveCritRate;

    if (valid_r)
        gEkrGaugeCrt[POS_R] = bu2->battleEffectiveCritRate;

    if (gEkrGaugeCrt[POS_L] == 0xFF)
        gEkrGaugeCrt[POS_L] = -1;

    if (gEkrGaugeCrt[POS_R] == 0xFF)
        gEkrGaugeCrt[POS_R] = -1;

    if (GetItemIndex(bu1->weapon) == ITEM_MONSTER_STONE)
        gEkrGaugeCrt[POS_L] = -1;

    if (GetItemIndex(bu2->weapon) == ITEM_MONSTER_STONE)
        gEkrGaugeCrt[POS_R] = -1;

    if (gEkrDistanceType == EKR_DISTANCE_PROMOTION)
    {
        gEkrGaugeHit[POS_R] = -1;
        gEkrGaugeDmg[POS_R] = -1;
        gEkrGaugeCrt[POS_R] = -1;
    }

    if (valid_l)
        gBanimExpPrevious[POS_L] = (s8)bu1->expPrevious; // needed explicit casts

    if (valid_r)
        gBanimExpPrevious[POS_R] = (s8)bu2->expPrevious; // needed explicit casts

    if (valid_l)
        gBanimExpGain[POS_L] = bu1->expGain;

    if (valid_r)
        gBanimExpGain[POS_R] = bu2->expGain;

    gBanimWtaBonus[POS_R] = 0;
    gBanimWtaBonus[POS_L] = 0;

    if (gEkrDistanceType != EKR_DISTANCE_PROMOTION)
    {
        if (valid_l)
            gBanimWtaBonus[POS_L] = bu1->wTriangleHitBonus;

        if (valid_r)
            gBanimWtaBonus[POS_R] = bu2->wTriangleHitBonus;

        if (valid_l)
            gBanimEffectiveness[POS_L] = IsUnitEffectiveAgainst(unit_bu1, unit_bu2);

        if (valid_r)
            gBanimEffectiveness[POS_R] = IsUnitEffectiveAgainst(unit_bu2, unit_bu1);

        if (!gBanimEffectiveness[POS_L] && valid_l)
            gBanimEffectiveness[POS_L] = IsItemEffectiveAgainst(bu1->weapon, unit_bu2);

        if (!gBanimEffectiveness[POS_R] && valid_r)
            gBanimEffectiveness[POS_R] = IsItemEffectiveAgainst(bu2->weapon, unit_bu1);
    }

    gBanimForceUnitChgDebug[POS_L] = gBanimForceUnitChgDebug[POS_R] = zero = 0;

    if (valid_l)
        (void)GetItemIndex(bu1->weaponBefore);

    if (valid_r)
        (void)GetItemIndex(bu2->weaponBefore);

    if (GetBanimLinkArenaFlag() == true || gPlaySt.config.unitColor)
    {
        gBanimUniquePaletteDisabled[POS_L] = gBanimUniquePaletteDisabled[POS_R] = 1;
    }
    else
    {
        gBanimUniquePaletteDisabled[POS_L] = gBanimUniquePaletteDisabled[POS_R] = 0;
    }

    ++zero; --zero; // :/

    gBanimBG = 0;

    if (GetBattleAnimPreconfType() == PLAY_ANIMCONF_ON_UNIQUE_BG)
    {
        if (gBanimValid[POS_L] != false)
            gBanimBG =
                GetBanimBackgroundIndex(gBanimTerrain[POS_L], GetROMChapterStruct(gPlaySt.chapterIndex)->battleTileSet);
        else
            gBanimBG =
                GetBanimBackgroundIndex(gBanimTerrain[POS_R], GetROMChapterStruct(gPlaySt.chapterIndex)->battleTileSet);
    }

    if (CheckBanimHensei() == 1)
        gBanimBG = 0x3C;

    usrdefined_enable = false;
    if (GetBattleAnimPreconfType() == PLAY_ANIMCONF_ON)
        usrdefined_enable = true;
    if (GetBattleAnimPreconfType() == PLAY_ANIMCONF_ON_UNIQUE_BG)
        usrdefined_enable = true;
    if (GetBattleAnimPreconfType() == PLAY_ANIMCONF_OFF)
    {
        /**
         * Banim can also display regardless used-configuration
         * in the following case:
         *
         * 1. promotion
         * 2. arena
         * 3. scripted battle
         */
        if (gEkrDistanceType == EKR_DISTANCE_PROMOTION)
            usrdefined_enable = true;
        if (GetBattleAnimArenaFlag() == true)
            usrdefined_enable = true;
        if ((CheckBattleScripted() == true))
            usrdefined_enable = true;
    }

    SetBattleUnscripted();

    if (gEkrDistanceType != EKR_DISTANCE_PROMOTION)
    {
        if (unit_bu1->state & US_IN_BALLISTA)
            return false;

        if (unit_bu2->state & US_IN_BALLISTA)
            return false;
    }

    if (unit_bu1->pClassData->number == CLASS_MANAKETE_MYRRH && GetItemIndex(bu2->weaponBefore) == ITEM_STAFF_SLEEP)
        return false;

    if (unit_bu2->pClassData->number == CLASS_MANAKETE_MYRRH && GetItemIndex(bu1->weaponBefore) == ITEM_STAFF_SLEEP)
        return false;

    if (unit_bu1->pClassData->number == CLASS_MANAKETE_MYRRH && GetItemIndex(bu2->weaponBefore) == ITEM_STAFF_BERSERK)
        return false;

    if (unit_bu2->pClassData->number == CLASS_MANAKETE_MYRRH && GetItemIndex(bu1->weaponBefore) == ITEM_STAFF_BERSERK)
        return false;

    if (unit_bu1->pClassData->number == CLASS_MANAKETE_MYRRH && GetItemIndex(bu2->weaponBefore) == ITEM_STAFF_SILENCE)
        return false;

    if (unit_bu2->pClassData->number == CLASS_MANAKETE_MYRRH && GetItemIndex(bu1->weaponBefore) == ITEM_STAFF_SILENCE)
        return false;

    if (unit_bu1->pClassData->number == CLASS_MANAKETE_MYRRH && GetItemIndex(bu2->weaponBefore) == ITEM_MONSTER_STONE)
        return false;

    if (unit_bu2->pClassData->number == CLASS_MANAKETE_MYRRH && GetItemIndex(bu1->weaponBefore) == ITEM_MONSTER_STONE)
        return false;

    if (char_cnt != 1 && unit_bu1->pClassData->number == CLASS_DEMON_KING && GetItemIndex(bu1->weaponBefore) != ITEM_NIGHTMARE &&
        unit_bu2->pClassData->number != CLASS_PHANTOM && unit_bu2->pClassData->number != CLASS_DRACO_ZOMBIE)
        return true;

    if (usrdefined_enable == false)
        return false;

    if (gBanimValid[POS_L] == true)
    {
        if (unit_bu1->statusIndex == UNIT_STATUS_BERSERK)
            return false;

        if (gBanimIdx[POS_L] == -1)
            return false;

        if (gEkrSpellAnimIndex[POS_L] == -2)
            return false;

        if (gBanimFloorfx[POS_L] == -1)
            return false;

        if (gBanimTerrain[POS_L] == TERRAIN_WALL_DAMAGED)
            return false;

        if (gBanimTerrain[POS_L] == TERRAIN_SNAG)
            return false;
    }

    if (gBanimValid[POS_R] == true)
    {
        if (unit_bu2->statusIndex == UNIT_STATUS_BERSERK)
            return false;

        if (gBanimIdx[POS_R] == -1)
            return false;

        if (gEkrSpellAnimIndex[POS_R] == -2)
            return false;

        if (gBanimFloorfx[POS_R] == -1)
            return false;

        if (gBanimTerrain[POS_R] == TERRAIN_WALL_DAMAGED)
            return false;

        if (gBanimTerrain[POS_R] == TERRAIN_SNAG)
            return false;
    }

    return true;
}