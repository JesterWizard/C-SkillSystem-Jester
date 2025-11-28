#include "global.h"

#include "proc.h"
#include "hardware.h"
#include "bm.h"
#include "m4a.h"
#include "soundwrapper.h"
#include "fontgrp.h"
#include "face.h"
#include "uiutils.h"
#include "ctc.h"
#include "savemenu.h"
#include "ekrbattle.h"
#include "bmlib.h"
#include "mu.h"
#include "bmsave.h"
#include "sysutil.h"
#include "efxbattle.h"

#include "constants/faces.h"

#include "common-chax.h"


/*
@ division runtime
fun __divsi3, __aeabi_idiv
fun __modsi3, __aeabi_idivmod
fun __udivsi3, __aeabi_uidiv
fun __umodsi3, __aeabi_uidivmod

dat 0x02000000, gUnk_ModeSelect_02000000
dat 0x02000001, gUnk_ModeSelect_02000001
dat 0x020000A4, gUnk_020000A4
dat 0x0201E8D4, gUnk_0201E8D4
dat 0x0201E97C, gUnk_0201E97C
dat 0x0201E9F4, gUnk_0201E9F4
*/

#define ApplySystemObjectsGraphics LoadObjUIGfx
#define SetBgOffset BG_SetPosition
#define Proc_LockEachMarked Proc_BlockEachMarked
#define ApplyPaletteExt CopyToPaletteBuffer
#define SpawnProc Proc_Start
#define SetFaceConfig SetupFaceGfxData
#define TmApplyTsa_thm CallARM_FillTileRect
#define GetBgChrOffset GetBackgroundTileDataOffset
#define fe7u_func_08063FE0 ResetClassReelSpell
#define fe7u_func_08054E10 sub_805A940
#define fe7u_func_08054C8C sub_805A7B4
#define fe7u_func_08054EF0 sub_805AA28
#define fe7u_func_080A86A0 sub_80AC6AC
#define fe7u_func_08001F3C sub_800154C
#define SetOnHBlankA SetPrimaryHBlankHandler
#define InitBgs SetupBackgrounds
#define EnablePalSync EnablePaletteSync
#define LoadMetaSave ReadGlobalSaveInfo
#define MetaSave_CountCompletedPlaythroughs GetGlobalCompletionCntByInfo
#define fe7u_func_08054E88 NewEfxAnimeDrvProc

#define gBg0Tm gBG0TilemapBuffer
#define gBg1Tm gBG1TilemapBuffer
#define gBg3Tm gBG3TilemapBuffer
#define gPal gPaletteBuffer

struct Unk_020000A4
{
    struct Font font;
    struct Text text[7];
};
extern struct Unk_020000A4 gUnk_020000A4;

struct ModeSelectProc
{
    /* 00 */ PROC_HEADER;
    /* 2C */ s32 unk_2c;
    /* 30 */ u16 unk_30;
    /* 32 */ u16 unk_32;
    /* 34 */ s32 unk_34;
    /* 38 */ void * unk_38;
    /* 3C */ void * unk_3c; // pFaceProc
    /* 40 */ u8 unk_40;
    /* 41 */ u8 unk_41;
    /* 42 */ u8 unk_42;
    /* 43 */ u8 unk_43[3];
    /* 46 */ STRUCT_PAD(0x46, 0x49);
    /* 49 */ u8 unk_49[3];
    /* 4C */ u8 unk_4c;
    /* 50 */ s32 unk_50;
};

extern struct AnimBuffer gUnk_0201E8D4[];

extern struct AnimMagicFxBuffer gUnk_0201E97C[];

extern u8 gUnk_ModeSelect_02000000;
extern u8 gUnk_ModeSelect_02000001;

// ==============
// TODO:
// ==============

void fe7u_func_080AACD8(u16 * param_1, u8 * param_2, u16 param_3)
{
    Decompress(param_2, gGenericBuffer);
    TmApplyTsa_thm(param_1, gGenericBuffer, param_3);
    return;
}

u32 fe7u_func_0809EFBC(void)
{
    return 1;
};

int fe7u_func_0809E9FC(void)
{
    // int cVar1;
    // int iVar2;
    // struct GlobalSaveInfo auStack_70;

    int uVar3 = 0;
/*
    if (LoadMetaSave(&auStack_70))
    {
        iVar2 = MetaSave_CountCompletedPlaythroughs(&auStack_70);

        // FIXME
        cVar1 = fe7u_func_0809EFBC();
        uVar3 = (int)(-cVar1 | cVar1) >> 0x1f & 2;

        if ((auStack_70.completed) != 0)
        {
            if ((auStack_70.Eirk_mode_easy || auStack_70.Eirk_mode_norm) != 0)
            {
                uVar3 = 0xf;
            }

            if ((auStack_70.Eirk_mode_hard) != 0)
            {
                uVar3 = uVar3 | 0x10;
            }

            if (iVar2 > 4)
            {
                uVar3 = uVar3 | 0x10;
            }
        }
    }
*/
    uVar3 = 0x1f;
    return uVar3;
}

// clang-format off

int const gUnk_08CE48C0[4][3] = {
    {
        // 0x000004DE, // Lyn
        // 0x000012B3, // Swordfighter of Sacae[.]
        // 0x000012B8, // Swd[.]
        0x212, // "Eirika"
        0x78B, // "The Valiant"
        0x4FE, // "Str"
    },
    {
        // 0x000004DC, // Eliwood
        // 0x000012B4, // Nobleman of Pherae
        // 0x000012B8, // Swd[.]
        0x220, // "Ephraim"
        0x78B, // "The Valiant"
        0x4FE, // "Str"
    },
    {
        //0x000004DD, // Hector
        //0x000012B5, // Marquess Ostia's brother
        //0x000012B9, // Axe[.]
        0x234, // "Lyon"
        0x78B, // "The Valiant"
        0x4FF, // "Mag"
    },
    {
        // 0x000012B6, // Main character:[.]
        // 0x00000000, 
        // 0x000012B7, // Weapon:[.]
        0x5A1, // "Pick"
        0,
        0x4F2, // "Equip"
    }
};

// clang-format on

void fe7u_func_08054E5C(struct AnimBuffer * pAnimBuf)
{
    pAnimBuf->anim1->state3 |= 8;
    pAnimBuf->anim2->state3 |= 8;
    return;
}

// palette
void fe7u_func_080A73F8(s32 param_1)
{
}

// clang-format off

u16 const BgConfig_08CE3C0C[] = {
    0x0000, 0x6000, 0x0000, 
    0xC000, 0x6800, 0x0000, 
    0x8000, 0x7800, 0x0000, 
    0x8000, 0x7800, 0x0000,
};

// clang-format on

extern u16 Pal_084138F0[]; // pal
extern u16 Pal_0840F9A0[]; // pal

extern u8 Img_08418E44[]; // gfx
extern u8 Img_0840FEB4[]; // gfx

extern u8 Tsa_0840FA00[]; // tsa?
extern u8 Tsa_08411F34[]; // tsa?

// Blend effect on the spell circle

void fe7u_func_080A32D4(void)
{
    u16 vcount = REG_VCOUNT + 1;

    if (vcount > DISPLAY_HEIGHT) {
        vcount = 0;
    }

    if ((vcount & 1) != 0) {
        return;
    }

    if (vcount < gUnk_ModeSelect_02000000) {
        REG_BLDCNT = 0xc1;
        REG_BLDY = (gUnk_ModeSelect_02000000 != 0) 
            ? (gUnk_ModeSelect_02000000 - vcount) * 0x10 / gUnk_ModeSelect_02000000 
            : 0;

    } else {
        REG_BLDCNT = 0x144;
        REG_BLDALPHA = gUnk_ModeSelect_02000001 | 0x1000;
    }

    return;
}


void fe7u_func_080A4E58(void)
{
    InitBgs((u16 *)BgConfig_08CE3C0C);

    gLCDControlBuffer.dispcnt.mode = 1;

    gLCDControlBuffer.bg2cnt.screenSize = 1;
    gLCDControlBuffer.bg2cnt.areaOverflowMode = 0;

    gLCDControlBuffer.bg0cnt.priority = 3;
    gLCDControlBuffer.bg1cnt.priority = 0;
    gLCDControlBuffer.bg2cnt.priority = 2;
    gLCDControlBuffer.bg3cnt.priority = 2;

    EndAllMus();

    SetDispEnable(0, 0, 0, 0, 0);

    gUnk_ModeSelect_02000001 = 10;
    gUnk_ModeSelect_02000000 = 100;

    SetOnHBlankA(fe7u_func_080A32D4);

    ApplyPaletteExt(Pal_084138F0, 0x220, 0x100);
    ApplyPaletteExt(Pal_0840F9A0, 0, 0x60);

    Decompress(Img_08418E44, (void *)(GetBgChrOffset(BG_0) + 0x6000000));
    TmApplyTsa_thm(gBg0Tm, Tsa_0840FA00, 0);

    Decompress(Img_0840FEB4, (void *)(GetBgChrOffset(BG_3) + 0x6000000));
    fe7u_func_08001F3C(gBg3Tm, Tsa_08411F34, 0, 5);

    BG_EnableSyncByMask(BG3_SYNC_BIT);

    return;
}

// =========
// end todo
// =========

struct ModeSelectSpriteDrawProc
{
    /* 00 */ PROC_HEADER;
    /* 2C */ s32 unk_2c;
    /* 30 */ s32 unk_30;
    /* 34 */ s32 unk_34;

    /* 38 */ s32 unk_38;
    /* 3C */ u8 unk_3c;
    /* 3E */ u16 unk_3e;
    /* 40 */ s32 unk_40;
    /* 44 */ s32 unk_44;
    /* 48 */ s32 unk_48;
    /* 4C */ u8 unk_4c;
    /* 4D */ u8 unk_4d;
    /* 4E */ u8 unk_4e;
};

const u8 * const gUnk_08CE480C[] = {
    (u8 *)0x020000F4,
    (u8 *)0x020020F4,
    (u8 *)0x020040F4,
};

const u8 * const gUnk_08CE4818[] = {
    (u8 *)0x020060F4,
    (u8 *)0x0200B8F4,
    (u8 *)0x020110F4,
};

const u8 * const gUnk_08CE4824[] = {
    (u8 *)0x020168F4,
    (u8 *)0x02016994,
    (u8 *)0x02016A34,
};

const u8 * const gUnk_08CE4830[] = {
    (u8 *)0x02016AD4,
    (u8 *)0x020194D4,
    (u8 *)0x0201BED4,
};

// anims?
const int gUnk_08418DA8[] = {
    2,
    0,
    0x9c,
};

// FE7U: 0x080A7480
void InitModeSelectAnims(int count, u8 * arg_1)
{
    int i;

    for (i = 0; i < count; i++)
    {
        gUnk_0201E8D4[i].xPos = 320;
        gUnk_0201E8D4[i].yPos = 88;
        gUnk_0201E8D4[i].animId = gUnk_08418DA8[arg_1[i]];
        gUnk_0201E8D4[i].roundType = 6;
        gUnk_0201E8D4[i].genericPalId = 0;
        gUnk_0201E8D4[i].state2 = 1;
        gUnk_0201E8D4[i].oam2Tile = (i * 0x2000 + 0x2000) >> 5;
        gUnk_0201E8D4[i].oam2Pal = i + 0xd;

        gUnk_0201E8D4[i].pImgSheetBuf = (void *)gUnk_08CE480C[i];
        gUnk_0201E8D4[i].unk_24 = (void *)gUnk_08CE4818[i];
        gUnk_0201E8D4[i].unk_20 = (void *)gUnk_08CE4824[i];
        gUnk_0201E8D4[i].unk_28 = (void *)gUnk_08CE4830[i];

        gUnk_0201E8D4[i].charPalId = 0xffff;

        gUnk_0201E8D4[i].unk_30 = &gUnk_0201E97C[i];

        gUnk_0201E97C[i].magicFuncIdx = 0;
        gUnk_0201E97C[i].xOffsetBg = 0;
        gUnk_0201E97C[i].yOffsetBg = 0;
        gUnk_0201E97C[i].xOffsetObj = 0;
        gUnk_0201E97C[i].yOffsetObj = 0;
        gUnk_0201E97C[i].objChr = 0;
        gUnk_0201E97C[i].objPalId = 0;
        gUnk_0201E97C[i].bgChr = 0;
        gUnk_0201E97C[i].bgPalId = 0;
        gUnk_0201E97C[i].bg = 0;

        gUnk_0201E97C[i].bgTmBuf = NULL;
        gUnk_0201E97C[i].bgImgBuf = NULL;
        gUnk_0201E97C[i].bgTsaBuf = NULL;
        gUnk_0201E97C[i].objImgBuf = NULL;
        gUnk_0201E97C[i].resetCallback = NULL;

        NewEkrUnitMainMini(&gUnk_0201E8D4[i]);
    }

    return;
}

// FE7U: 0x080A75CC
void EndModeSelectAnims(s32 count)
{
    int i;

    for (i = 0; i < count; i++)
    {
        fe7u_func_08054EF0(&gUnk_0201E8D4[i]);
    }

    return;
}

const char StrModeSelect_MainCharacter[] = "Main character:";
const char StrModeSelect_Weapon[] = "Weapon:";

// FE7U: 0x080A75F0
void PutModeSelectLabelText(void)
{
    ClearText(&gUnk_020000A4.text[5]);
    ClearText(&gUnk_020000A4.text[6]);

    PutDrawText(&gUnk_020000A4.text[5], gBg1Tm + TILEMAP_INDEX(14, 6), TEXT_COLOR_SYSTEM_WHITE, 0, 0, StrModeSelect_MainCharacter);
    PutDrawText(&gUnk_020000A4.text[6], gBg1Tm + TILEMAP_INDEX(14, 10), TEXT_COLOR_SYSTEM_WHITE, 0, 0, StrModeSelect_Weapon);

    BG_EnableSyncByMask(BG1_SYNC_BIT);

    return;
}

const char StrModeSelect_Swd[] = "Swd";
const char StrModeSelect_Lnc[] = "Lnc";
const char StrModeSelect_Mag[] = "Mag";

const char * const StrModeSelect_Wpn[] = {
    StrModeSelect_Swd,
    StrModeSelect_Lnc,
    StrModeSelect_Mag,
};

// FE7U: 0x080A7668
void PutModeSelectCharacterText(s32 index)
{
    ClearText(&gUnk_020000A4.text[2]);
    ClearText(&gUnk_020000A4.text[3]);
    ClearText(&gUnk_020000A4.text[4]);

    PutDrawText(&gUnk_020000A4.text[2], gBg1Tm + TILEMAP_INDEX(14, 8), TEXT_COLOR_SYSTEM_BLUE, 0, 0, GetStringFromIndex(gUnk_08CE48C0[index][0]));
    PutDrawText(&gUnk_020000A4.text[4], gBg1Tm + TILEMAP_INDEX(19, 10), TEXT_COLOR_SYSTEM_BLUE, 0, 0, StrModeSelect_Wpn[index]);

    BG_EnableSyncByMask(BG1_SYNC_BIT);

    return;
}

// FE7U: 0x080A76F8
void PutModeSelectDifficultyText(struct ModeSelectProc * proc)
{
    int unk = proc->unk_43[proc->unk_41];

    ClearText(&gUnk_020000A4.text[0]);
    ClearText(&gUnk_020000A4.text[1]);

    PutDrawText(&gUnk_020000A4.text[0], gBg1Tm + TILEMAP_INDEX(15, 12), unk == 0 ? TEXT_COLOR_SYSTEM_GOLD : TEXT_COLOR_SYSTEM_GRAY, 0, 0, GetStringFromIndex(0x053E));

    BG_EnableSyncByMask(BG1_SYNC_BIT);

    switch (proc->unk_49[proc->unk_41])
    {
        case 0:
            if (!(1 & proc->unk_40))
            {
                return;
            }

            break;

        case 1:
            if (!(4 & proc->unk_40))
            {
                return;
            }

            break;

        case 2:
            if (!(0x10 & proc->unk_40))
            {
                return;
            }

            break;
    }

    PutDrawText(&gUnk_020000A4.text[1], gBg1Tm + TILEMAP_INDEX(15, 14), unk == 1 ? TEXT_COLOR_SYSTEM_GOLD : TEXT_COLOR_SYSTEM_GRAY, 0, 0, GetStringFromIndex(0x053F));

    return;
}

const int gUnk_08418DB4[3] = { 
    FID_EIRIKA, 
    FID_EPHRAIM, 
    0x46 // FID_LYON
};

// FE7U: 0x080A77C0
struct FaceProc * StartModeSelectFace(int index)
{
    struct FaceProc * pFaceProc = StartFace2(0, gUnk_08418DB4[index], 204, 72, (FACE_DISP_KIND(FACE_96x80) | FACE_DISP_HLAYER(FACE_HLAYER_0)));
    StartFaceFadeIn(pFaceProc);

    return pFaceProc;
}

extern u8 Img_08415BE8[]; // img, lyn chapter top left
extern u8 Img_08415CB0[]; // img, lyn chapter bottom left
extern u8 Img_08415DC4[]; // img, lyn chapter top right
extern u8 Img_08415E04[]; // img, lyn chapter bottom right

extern u8 Img_08415E54[]; // img, eliwood chapter top left
extern u8 Img_08415F14[]; // img, eliwood chapter bottom left
extern u8 Img_08415FF0[]; // img, eliwood chapter top right
extern u8 Img_0841601C[]; // img, eliwood chapter bottom right

extern u8 Img_08416058[]; // img, hector chapter top left
extern u8 Img_08416118[]; // img, hector chapter bottom left
extern u8 Img_084161F4[]; // img, hector chapter top right
extern u8 Img_08416220[]; // img, hector chapter bottom right

// FE7U: 0x080A77F8
void LoadModeSelectChapterGfx(s32 param_1)
{
    void * gUnk_08418DC0[3][4] = {
        {
            Img_08415BE8,
            Img_08415CB0,
            Img_08415DC4,
            Img_08415E04,
        },
        {
            Img_08415E54,
            Img_08415F14,
            Img_08415FF0,
            Img_0841601C,
        },
        {
            Img_08416058,
            Img_08416118,
            Img_084161F4,
            Img_08416220,
        },
    };

    Decompress(gUnk_08418DC0[param_1][0], (void *)0x60102C0);
    Decompress(gUnk_08418DC0[param_1][1], (void *)0x60106C0);
    Decompress(gUnk_08418DC0[param_1][2], (void *)0x6010AC0);
    Decompress(gUnk_08418DC0[param_1][3], (void *)0x6010EC0);

    return;
}

extern u16 gUnk_0201E9F4[];

void fe7u_func_080A7860(s32 palId)
{
    int i;

    u16 * palIt = gPal + (palId + 0xd) * 0x10 + 0x101;

    for (i = 0; i < 0xf; i++)
    {
        gUnk_0201E9F4[i + palId * 0xf] = *palIt++;
    }

    return;
}

void fe7u_func_080A7890(s32 palId, s32 amt)
{
    s32 i;

    u16 * r5 = (gPal + (palId + 0xd) * 0x10 + 0x101);

    if (amt > 0x40)
    {
        amt = 0x40;
    }

    amt = amt + (gUnk_ModeSelect_02000001 - 10) * 2;

    for (i = 0; i < 0xf; i++)
    {
        s32 accum = 0;
        s32 r;
        s32 g;
        s32 b;

        r = (amt * (gUnk_0201E9F4[i + palId * 0xf] & RED_MASK)) >> 6;

        if (r <= RED_MASK)
        {
            if (r < 0)
                r = 0;

            accum += r & RED_MASK;
        }
        else
            accum += RED_MASK;

        g = (amt * (gUnk_0201E9F4[i + palId * 0xf] & GREEN_MASK)) >> 6;

        if (g <= GREEN_MASK)
        {
            if (g < 0)
                g = 0;

            accum += g & GREEN_MASK;
        }
        else
            accum += GREEN_MASK;

        b = (amt * (gUnk_0201E9F4[i + palId * 0xf] & BLUE_MASK)) >> 6;

        if (b <= BLUE_MASK)
        {
            if (b < 0)
                b = 0;

            *r5 = accum + (b & BLUE_MASK);
        }
        else
        {
            *r5 = accum + BLUE_MASK;
        }

        r5++;
    }

    EnablePalSync();

    return;
}

void fe7u_func_080A793C(s32 palId, s32 b)
{
    s32 b_ = (b & 0xff);
    s32 tmp = (((b_ >= 0x81) ? b_ - 0x80 : 0x80 - b_) * 0x30 >> 7);
    fe7u_func_080A7890(palId, tmp + 0x10);

    return;
}

// FE7U: 0x080A796C
void ModeSelectSpriteDraw_Init(struct ModeSelectSpriteDrawProc * proc)
{
    proc->unk_30 = 0;
    proc->unk_3e = 0;
    proc->unk_3c = 0;
    proc->unk_34 = DISPLAY_WIDTH / 2;
    proc->unk_38 = DISPLAY_HEIGHT;
    proc->unk_40 = 0;
    proc->unk_44 = 0;
    proc->unk_3c = 0;
    proc->unk_48 = 0;
    proc->unk_4c = 0;
    proc->unk_2c = 0;
    proc->unk_4e = 0;

    return;
}

// clang-format off

// FE7U: 0x08CE483C
u16 const Sprite_ModeSelect_Mode[] =
{
    4,
    OAM0_SHAPE_32x16, OAM1_SIZE_32x16, 0,
    OAM0_SHAPE_32x16, OAM1_SIZE_32x16 + OAM1_X(32), OAM2_CHR(0x4),
    OAM0_SHAPE_32x8 + OAM0_Y(16), OAM1_SIZE_32x8, OAM2_CHR(0x40),
    OAM0_SHAPE_32x8 + OAM0_Y(16), OAM1_SIZE_32x8 + OAM1_X(32), OAM2_CHR(0x44),
};

// FE7U: 0x08CE4856
u16 const Sprite_ModeSelect_Select[] =
{
    6,
    OAM0_SHAPE_32x16, OAM1_SIZE_32x16, OAM2_CHR(0x8),
    OAM0_SHAPE_32x16, OAM1_SIZE_32x16 + OAM1_X(32), OAM2_CHR(0xC),
    OAM0_SHAPE_8x16, OAM1_SIZE_8x16 + OAM1_X(64), OAM2_CHR(0x10),
    OAM0_SHAPE_32x8 + OAM0_Y(16), OAM1_SIZE_32x8, OAM2_CHR(0x60),
    OAM0_SHAPE_32x8 + OAM0_Y(16), OAM1_SIZE_32x8 + OAM1_X(32), OAM2_CHR(0x64),
    OAM0_SHAPE_8x8 + OAM0_Y(16), OAM1_SIZE_8x8 + OAM1_X(64), OAM2_CHR(0x68),
};

// FE7U: 0x08CE487C
u16 const Sprite_ModeSelect_PressStart[] =
{
    5,
    OAM0_SHAPE_32x8, OAM1_SIZE_32x8, OAM2_CHR(0x11),
    OAM0_SHAPE_32x8 + OAM0_Y(8), OAM1_SIZE_32x8, OAM2_CHR(0x49),
    OAM0_SHAPE_32x8, OAM1_SIZE_32x8 + OAM1_X(32), OAM2_CHR(0x31),
    OAM0_SHAPE_32x8 + OAM0_Y(8), OAM1_SIZE_32x8 + OAM1_X(32), OAM2_CHR(0x4D),
    OAM0_SHAPE_8x16, OAM1_SIZE_8x16 + OAM1_X(64), OAM2_CHR(0x55),
};

// FE7U: 0x08CE489C
u16 const Sprite_ModeSelect_Change[] =
{
    1,
    OAM0_SHAPE_32x8, OAM1_SIZE_32x8, OAM2_CHR(0x51),
};

// FE7U: 0x08CE48A4
u16 const Sprite_ModeSelect_ChapterRange[] =
{
    4,
    OAM0_SHAPE_32x16 + OAM0_AFFINE_ENABLE, OAM1_SIZE_32x16, OAM2_CHR(0x16),
    OAM0_SHAPE_32x16 + OAM0_AFFINE_ENABLE, OAM1_SIZE_32x16 + OAM1_X(32), OAM2_CHR(0x1A),
    OAM0_SHAPE_16x16 + OAM0_AFFINE_ENABLE, OAM1_SIZE_16x16 + OAM1_X(64), OAM2_CHR(0x1E),
    OAM0_SHAPE_32x16 + OAM0_AFFINE_ENABLE, OAM1_SIZE_32x16 + OAM1_X(80), OAM2_CHR(0x56),
};

// clang-format on

s32 fe7u_func_080A86A0(s32, s32, s32, s32, s32);

// FE7U: 0x080A79A4
void ModeSelectSpriteDraw_Loop(struct ModeSelectSpriteDrawProc * proc)
{
    s32 i;

    if (proc->unk_3c != 0)
    {
        for (i = 0; i < proc->unk_40; i++)
        {
            s32 angle = ((proc->unk_3e >> 4) + i * proc->unk_44 + 40);
            s32 x = (proc->unk_34 << 12) + (SIN(angle) * 70);
            s32 y = (((proc->unk_38 << 12) + (COS(angle) * 28)) >> 12) - 16;

            fe7u_func_08054E10(&gUnk_0201E8D4[i], x >> 12, y);

            fe7u_func_080A793C(i, (proc->unk_3e >> 4) + i * proc->unk_44);
        }
    }

    BgAffinRotScaling(BG_2, proc->unk_3e, 0, 0, 0x160, 0x160);
    BgAffinScaling(BG_2, 0x280, 0x100);
    BgAffinAnchoring(BG_2, proc->unk_34, proc->unk_38, 76, 76);

    gUnk_ModeSelect_02000001 = fe7u_func_080A86A0(8, 8, 16, 16, proc->unk_48);

    if (proc->unk_4c == 0)
    {
        proc->unk_48 += 8;
        if (proc->unk_48 >= 0x400)
        {
            proc->unk_4c = 1;
        }
    }
    else
    {
        proc->unk_48 -= 8;

        if (proc->unk_48 <= 0)
        {
            proc->unk_4c = 0;
        }
    }

    if (proc->unk_4e & 2)
    {
        DisplayFrozenUiHandExt(108, (proc->unk_4d & 1) * 16 + 104, OAM2_CHR(0x3C0) + OAM2_LAYER(2));
    }
    else
    {
        DisplayUiHandExt(108, proc->unk_4d * 16 + 104, OAM2_CHR(0x3C0) + OAM2_LAYER(2));
    }

    PutSpriteExt(0xd, 0, 8, Sprite_ModeSelect_Mode, OAM2_PAL(11));
    PutSpriteExt(0xd, 20, 28, Sprite_ModeSelect_Select, OAM2_PAL(11));

    PutSpriteExt(0xd, 40, 64, Sprite_ModeSelect_Change, OAM2_PAL(11));

    if ((proc->unk_2c >> 2 & 1) == 0)
    {
        PutSpriteExt(0xd, 8, 130, Sprite_ModeSelect_PressStart, OAM2_PAL(11));
    }

    if (proc->unk_2c != 0)
    {
        proc->unk_2c++;
    }

    PutSpriteExt(0xd, 108, 24, Sprite_ModeSelect_ChapterRange, OAM2_PAL(10));

    fe7u_func_080A73F8(proc->unk_30);
    proc->unk_30++;

    return;
}

// clang-format off

// FE7U: 0x08CE48F0
struct ProcCmd const ProcScr_ModeSelectSpriteDraw[] =
{
    PROC_CALL(ModeSelectSpriteDraw_Init),
    PROC_YIELD,

    PROC_REPEAT(ModeSelectSpriteDraw_Loop),

    PROC_END,
};

// clang-format on

void fe7u_func_080A7B7C(void)
{
    struct ModeSelectSpriteDrawProc * proc = Proc_Find(ProcScr_ModeSelectSpriteDraw);

    if (proc != NULL)
    {
        proc->unk_2c = 1;
    }

    return;
}

void fe7u_func_080A7B98(void)
{
    struct ModeSelectSpriteDrawProc * proc = Proc_Find(ProcScr_ModeSelectSpriteDraw);

    if (proc != NULL)
    {
        proc->unk_3c = 1;
    }

    return;
}

void fe7u_func_080A7BB4(s32 arg_0)
{
    struct ModeSelectSpriteDrawProc * proc = Proc_Find(ProcScr_ModeSelectSpriteDraw);

    if (proc != NULL)
    {
        proc->unk_40 = arg_0;
        proc->unk_44 = 0x100 / arg_0;
    }

    return;
}

void fe7u_func_080A7BDC(s32 arg_0, s32 arg_1)
{
    struct ModeSelectSpriteDrawProc * proc = Proc_Find(ProcScr_ModeSelectSpriteDraw);

    if (proc != NULL)
    {
        proc->unk_34 = arg_0;
        proc->unk_38 = arg_1;
    }

    gUnk_ModeSelect_02000000 = arg_1 - 60;

    return;
}

void fe7u_func_080A7C08(u16 arg_0)
{
    struct ModeSelectSpriteDrawProc * proc = Proc_Find(ProcScr_ModeSelectSpriteDraw);

    if (proc != NULL)
    {
        proc->unk_3e = arg_0;
    }

    return;
}

void fe7u_func_080A7C24(u8 arg_0, u8 arg_1)
{
    struct ModeSelectSpriteDrawProc * proc = Proc_Find(ProcScr_ModeSelectSpriteDraw);

    if (proc != NULL)
    {
        proc->unk_4d = arg_0;
        proc->unk_4e = arg_1;
    }

    return;
}

s32 fe7u_func_080A7C4C(void)
{
    struct ModeSelectSpriteDrawProc * proc = Proc_Find(ProcScr_ModeSelectSpriteDraw);
    return proc->unk_44;
}

// struct UnkProc
// {
//     PROC_HEADER;
//     STRUCT_PAD(0x29, 0x34);
//     s16 unk_34;
//     s16 unk_36;
// };

// void fe7u_func_080A7C60(struct UnkProc * proc, s32 param_2, s32 param_3)
// {
//     if (proc != NULL)
//     {
//         proc->unk_34 = param_2;
//         proc->unk_36 = param_3;
//     }

//     return;
// }

void fe7u_func_080A4E58(void);

// FE7U: 0x080A7C6C
void ModeSelect_InitGfxMaybe(struct ModeSelectProc * proc)
{
    if (proc->unk_42 & 1)
    {
        fe7u_func_080A4E58();
    }

    return;
}

extern struct ProcCmd const ProcScr_ModeSelectSpriteDraw[];

// FE7U: 0x08CE4910
struct FaceVramEntry const FaceConfig_ModeSelect[] = {
    {
        .tileOffset = 0x1000,
        .paletteId = 0xC,
    },
    {
        .tileOffset = 0x1000,
        .paletteId = 0xC,
    },
    {
        .tileOffset = 0x1000,
        .paletteId = 0xC,
    },
    {
        .tileOffset = 0x1000,
        .paletteId = 0xC,
    },
};

extern u16 Pal_084150C0[]; // pal
extern u8 Img_08414940[];  // gfx
extern u8 Tsa_084150E0[];  // tsa

extern u16 Pal_08415AA0[]; // pal
extern u8 Img_08415594[];  // gfx
extern u8 Tsa_08415AC0[];  // tsa

extern u16 Pal_0841625C[]; // pal

int const gUnk_08418DF0[] = {
    4,
    0x10,
};

// FE7U: 0x080A7C84
void ModeSelect_Init(struct ModeSelectProc * proc) {
    int i;

    ApplySystemObjectsGraphics();
    
    SetBgOffset(BG_1, 8, -8);
    
    Proc_LockEachMarked(PROC_MARK_SAVEDRAW);
    Proc_LockEachMarked(PROC_MARK_D);
    
    gUnk_ModeSelect_02000000 = 100;
    
    SetFaceConfig((struct FaceVramEntry *)FaceConfig_ModeSelect);
    
    ApplyPalette(Pal_08415AA0, 0xF);
    
    Decompress(Img_08415594, (void *)(0x6000000 + GetBgChrOffset(1)));
    TmApplyTsa_thm(gBg0Tm, Tsa_084150E0, 0);
    fe7u_func_080AACD8(gBg1Tm, Tsa_08415AC0, 0xf000); // this loads the "claw menu" and bg of the chapters
    ApplyPalette(Pal_084150C0, 0x1B);
    
    Decompress(Img_08414940, (void *)0x6010000);
    ApplyPalette(Pal_0841625C, 0x1A);
    
    fe7u_func_08054E88();
    fe7u_func_08063FE0();
    
    proc->unk_38 = SpawnProc(ProcScr_ModeSelectSpriteDraw, proc);
    fe7u_func_080A7BDC(0, 0x70);

    proc->unk_41 = 0;
    proc->unk_4c = 0;

    proc->unk_40 = fe7u_func_0809E9FC();

    if ((proc->unk_42 & 1) == 0) {
        int gUnk_08418DF0[] = {
            4, 0x10, 
        };
        
        proc->unk_4c = 2;
        proc->unk_49[0] = 1;
        proc->unk_49[1] = 2;

        for (i = 0; i < proc->unk_4c; i++) {
            if (gPlaySt.chapterStateBits & PLAY_FLAG_HARD) {
                if (proc->unk_40 & gUnk_08418DF0[i]) {
                    proc->unk_43[i] = 1;
                } else {
                    proc->unk_43[i] = 0;
                }
            } else {
                proc->unk_43[i] = 0;
            }
        }
    } else {
        proc->unk_49[0] = 0;
        proc->unk_4c++;

        if ((proc->unk_40 & 2) != 0) {
            proc->unk_49[proc->unk_4c] = 1;
            proc->unk_4c++;
        }

        if ((proc->unk_40 & 8) != 0) {
            proc->unk_49[proc->unk_4c] = 2;
            proc->unk_4c++;
        }
    
        for (i = 0; i < proc->unk_4c; i++) {
            proc->unk_43[i] = 0;
        }
    }

    fe7u_func_080A7BB4(proc->unk_4c);
    InitModeSelectAnims(proc->unk_4c, proc->unk_49);

    for (i = 0; i < proc->unk_4c; i++) {
        fe7u_func_080A7860(i);
    }

    fe7u_func_080A7B98();
    StartUiSpinningArrows(proc);
    LoadUiSpinningArrowGfx(0, 0xd20, 9);
    LoadUiSpinningArrowGfx(0, 0xd20, 9);
    SetUiSpinningArrowPositions(30, 61, 68, 61);
    SetUiSpinningArrowConfig(3);

    InitTextFont(&gUnk_020000A4.font, (void *)0x600E000, 0x100, 0xe);
    
    InitText(&gUnk_020000A4.text[0], 5);
    InitText(&gUnk_020000A4.text[1], 9);
    InitText(&gUnk_020000A4.text[2], 5);
    InitText(&gUnk_020000A4.text[3], 8);
    InitText(&gUnk_020000A4.text[4], 4);
    InitText(&gUnk_020000A4.text[5], 10);
    InitText(&gUnk_020000A4.text[6], 5);

    proc->unk_30 = proc->unk_41 * fe7u_func_080A7C4C() * 0x10;

    proc->unk_3c = (void *)StartModeSelectFace(proc->unk_49[proc->unk_41]);
    PutModeSelectLabelText();
    PutModeSelectCharacterText(proc->unk_49[proc->unk_41]);
    PutModeSelectDifficultyText(proc);
    fe7u_func_080A7C24(proc->unk_43[proc->unk_41], proc->unk_42);
    fe7u_func_080A7C08(proc->unk_30);

    BG_EnableSyncByMask(BG0_SYNC_BIT | BG1_SYNC_BIT);

    proc->unk_2c = 0;
    proc->unk_50 = 0;

    SetWinEnable(1, 0, 0);
    SetWin0Layers(1, 1, 1, 1, 1);
    SetWin0Box(0, 0x50, 0xf0, 0x50);
    SetWOutLayers(0, 0, 0, 0, 0);

    LoadModeSelectChapterGfx(proc->unk_49[proc->unk_41]);

    // clang-format off
    SetObjAffine(
        0,
        Div(+COS(0) * 16, 0x100),
        Div(-SIN(0) * 16, 0x100),
        Div(+SIN(0) * 16, 0x100),
        Div(+COS(0) * 16, 0x100)
    );
    // clang-format on

    return;
}

// FE7U: 0x080A8054
void ModeSelect_TransitionSplitOpen(struct ModeSelectProc * proc)
{
    s32 tmp;
    s32 unk_2c;

    unk_2c = proc->unk_2c + 1;
    proc->unk_2c = unk_2c;

    SetDispEnable(1, 1, 1, 1, 1);

    tmp = 0x48 - (((0x10 - unk_2c) * 0x48) * (0x10 - unk_2c) / 256);

    SetWin0Box(0, 0x50 - tmp, 0xf0, (tmp) + 0x50);

    if (unk_2c == 0x10)
    {
        Proc_Break(proc);
    }

    return;
}

// FE7U: 0x080A80C4
void ModeSelect_TransitionSplitClose(struct ModeSelectProc * proc)
{
    s32 tmp;
    s32 unk_2c;

    unk_2c = proc->unk_2c + 1;
    proc->unk_2c = unk_2c;

    tmp = 0x48 - (((0x10 - unk_2c) * 0x48) * (0x10 - unk_2c) / 256);

    SetWin0Box(0, tmp + 8, 0xf0, -0x68 - (tmp));

    if (unk_2c == 0x10)
    {
        Proc_Break(proc);
    }

    return;
}

void fe7u_func_08054E5C(struct AnimBuffer *);

void fe7u_func_080A8120(struct ModeSelectProc * proc)
{
    s32 i;

    for (i = 0; i < proc->unk_4c; i++)
    {
        fe7u_func_08054E5C(&gUnk_0201E8D4[i]);
    }

    proc->unk_50 = 0;

    return;
}

void PutModeSelectDifficultyText(struct ModeSelectProc *);

void fe7u_func_080A8150(struct ModeSelectProc * proc, s32 param_2)
{
    proc->unk_43[proc->unk_41] = param_2;

    PutModeSelectDifficultyText(proc);
    fe7u_func_080A7C24(param_2, proc->unk_42);

    return;
}

void fe7u_func_08054C8C(struct AnimBuffer *);

// FE7U: 0x080A817C
void ModeSelect_Loop_KeyHandler(struct ModeSelectProc * proc)
{
    if (((gKeyStatusPtr->repeatedKeys & DPAD_UP) != 0) && (proc->unk_43[proc->unk_41]) == 1)
    {
        PlaySoundEffect(0x66);
        fe7u_func_080A8150(proc, 0);
        return;
    }

    if ((gKeyStatusPtr->repeatedKeys & DPAD_DOWN) != 0)
    {
        if (proc->unk_43[proc->unk_41] == 0)
        {

            if ((proc->unk_49[proc->unk_41] == 0) && ((proc->unk_40 & 1) == 0))
            {
                PlaySoundEffect(0x6c);
                return;
            }

            if ((proc->unk_49[proc->unk_41] == 1) && ((proc->unk_40 & 4) == 0))
            {
                PlaySoundEffect(0x6c);
                return;
            }

            if ((proc->unk_49[proc->unk_41] == 2) && ((proc->unk_40 & 0x10) == 0))
            {
                PlaySoundEffect(0x6c);
                return;
            }

            PlaySoundEffect(0x66);
            fe7u_func_080A8150(proc, 1);

            return;
        }
    }

    if ((gKeyStatusPtr->heldKeys & (DPAD_LEFT | L_BUTTON)) != 0)
    {
        Proc_Goto(proc, 1);

        SetUiSpinningArrowFastMaybe(0);
        PlaySoundEffect(0x67);
        fe7u_func_080A8120(proc);

        return;
    }

    if ((gKeyStatusPtr->heldKeys & (DPAD_RIGHT | R_BUTTON)) != 0)
    {
        Proc_Goto(proc, 2);

        SetUiSpinningArrowFastMaybe(1);
        PlaySoundEffect(0x67);
        fe7u_func_080A8120(proc);

        return;
    }

    if ((gKeyStatusPtr->newKeys & (START_BUTTON | A_BUTTON)) != 0)
    {
        proc->unk_2c = 0;

        PlaySoundEffect(0x6a);

        Proc_Goto(proc, 3);

        gUnk_0201E8D4[proc->unk_41].roundType = 0;
        fe7u_func_08054C8C(&gUnk_0201E8D4[proc->unk_41]);

        if ((proc->unk_42 & 1) != 0)
        {
            if (proc->unk_41 == 0)
            {
                gPlaySt.chapterModeIndex = 2;
            }

            if (proc->unk_41 == 1)
            {
                gPlaySt.chapterModeIndex = 3;
            }

            if (proc->unk_43[proc->unk_41] != 0)
            {
                gPlaySt.chapterStateBits |= PLAY_FLAG_HARD;
            }
            else
            {
                gPlaySt.chapterStateBits &= ~PLAY_FLAG_HARD;
            }
        }
        else
        {
            SaveMenu_SetDifficultyChoice(proc->unk_49[proc->unk_41], proc->unk_43[proc->unk_41]);
            fe7u_func_080A7C24(proc->unk_43[proc->unk_41], proc->unk_42 | 2);
        }

        fe7u_func_080A7B7C();
        return;
    }

    if (((gKeyStatusPtr->newKeys & B_BUTTON) != 0) && ((proc->unk_42 & 1) == 0))
    {
        proc->unk_2c = 0;

        PlaySoundEffect(0x6b);

        Proc_Goto(proc, 4);
        SaveMenu_SetDifficultyChoice(3, 0);
    }

    proc->unk_50++;

    if ((proc->unk_50 & 0x1ff) == 0x20)
    {
        gUnk_0201E8D4[proc->unk_41].roundType = 2;
        fe7u_func_08054C8C(&gUnk_0201E8D4[proc->unk_41]);
    }

    if ((proc->unk_50 & 0x1ff) != 0x80)
    {
        return;
    }

    fe7u_func_08054E5C(&gUnk_0201E8D4[proc->unk_41]);

    return;
}

// FE7U: 0x080A8424
void ModeSelect_RotateRight(struct ModeSelectProc * proc)
{
    proc->unk_34 = -1;
    proc->unk_2c = 0;

    StartFaceFadeOut(proc->unk_3c);

    if (proc->unk_41 == 0)
    {
        proc->unk_41 = proc->unk_4c - 1;
    }
    else
    {
        proc->unk_41--;
    }

    proc->unk_32 = (0x100 - fe7u_func_080A7C4C() * proc->unk_41) << 4;

    fe7u_func_080A8150(proc, proc->unk_43[proc->unk_41]);

    if (proc->unk_32 < proc->unk_30)
    {
        proc->unk_32 += 0x1000;
    }

    return;
}

// FE7U: 0x080A848C
void ModeSelect_RotateLeft(struct ModeSelectProc * proc)
{
    proc->unk_34 = 1;
    proc->unk_2c = 0;

    StartFaceFadeOut(proc->unk_3c);

    if (proc->unk_41 < (proc->unk_4c - 1))
    {
        proc->unk_41++;
    }
    else
    {
        proc->unk_41 = 0;
    }

    proc->unk_32 = (0x100 - fe7u_func_080A7C4C() * proc->unk_41) << 4;

    fe7u_func_080A8150(proc, proc->unk_43[proc->unk_41]);

    if (proc->unk_32 > proc->unk_30)
    {
        proc->unk_30 += 0x1000;
    }

    return;
}

// FE7U: 0x080A84F8
void ModeSelect_Loop_RotateCarousel(struct ModeSelectProc * proc)
{
    s32 a;
    s32 b;
    s32 c;
    u16 d;
    s32 r9;

    a = (proc->unk_32 - proc->unk_30) * proc->unk_34;
    r9 = 0x100;
    proc->unk_2c++;

    b = a >> 2;

    c = b * (0x1e - proc->unk_2c) * (0x1e - proc->unk_2c) / 900;
    d = (proc->unk_30 + proc->unk_34 * 4 * (b - c));

    if (proc->unk_2c == 13)
    {
        LoadModeSelectChapterGfx(proc->unk_49[proc->unk_41]);
    }

    if (proc->unk_2c == 14)
    {
        proc->unk_3c = StartModeSelectFace(proc->unk_49[proc->unk_41]);
    }

    if (proc->unk_2c == 20)
    {
        PutModeSelectCharacterText(proc->unk_49[proc->unk_41]);
    }

    if (proc->unk_2c == 30)
    {
        d = proc->unk_32 & 0xfff;
        proc->unk_30 = proc->unk_32 & 0xfff;
        Proc_Break(proc);
    }

    // clang-format off
    SetObjAffine(
        0,
        Div(+COS(0) * 16, r9),
        Div(-SIN(0) * 16, r9),
        Div(+SIN(0) * 16, r9),
        Div(+COS(0) * 16, r9)
    );
    // clang-format on

    fe7u_func_080A7C08(d);

    return;
}

// FE7U: 0x080A8624
void ModeSelect_End(struct ModeSelectProc * proc)
{
    EndModeSelectAnims(proc->unk_4c);
    EndEfxAnimeDrvProc();
    EndFaceById(0);

    if (!(proc->unk_42 & 1))
    {
        StartBgmVolumeChange(0x100, 0xc0, 0x10, 0);
    }
    else
    {
        SetPrimaryHBlankHandler(NULL);
    }

    return;
}

void fe7u_func_080A95B4(void);

// clang-format off

// FE7U: 0x08CE4930
struct ProcCmd const ProcScr_ModeSelect[] =
{
    PROC_CALL(DisableAllGfx),
    PROC_YIELD,

    PROC_CALL(ModeSelect_InitGfxMaybe),
    PROC_YIELD,

    PROC_CALL(ModeSelect_Init),
    PROC_YIELD,

    PROC_REPEAT(ModeSelect_TransitionSplitOpen),

PROC_LABEL(0),
    PROC_REPEAT(ModeSelect_Loop_KeyHandler),

PROC_LABEL(1),
    PROC_CALL(ModeSelect_RotateLeft),
    PROC_REPEAT(ModeSelect_Loop_RotateCarousel),

    PROC_GOTO(0),

PROC_LABEL(2),
    PROC_CALL(ModeSelect_RotateRight),
    PROC_REPEAT(ModeSelect_Loop_RotateCarousel),

    PROC_GOTO(0),

PROC_LABEL(3),
    PROC_SLEEP(60),

PROC_LABEL(4),
    PROC_REPEAT(ModeSelect_TransitionSplitClose),
    PROC_CALL(ModeSelect_End),

    PROC_END,
};

// clang-format on

// FE7U: 0x080A8664
void StartModeSelect(ProcPtr parent)
{
    struct ModeSelectProc * proc = Proc_StartBlocking(ProcScr_ModeSelect, parent);
    proc->unk_42 = 1;
    return;
}

// HOOKS

extern u16 gBgConfig_SaveMenu[];

//! Hook; overwrite function at FE8U:0x080AA1EC
LYN_REPLACE_CHECK(SaveMenu_ResetLcdFormDifficulty);
void SaveMenu_ResetLcdFormDifficulty(struct SaveMenuProc * proc)
{
    proc->scroll_cnt = 0;

    SetupBackgrounds(gBgConfig_SaveMenu);

    SetWinEnable(0, 0, 0);

    SetWin0Layers(1, 1, 1, 1, 1);
    SetWOutLayers(0, 0, 0, 0, 0);

    gLCDControlBuffer.dispcnt.mode = 0;

    gLCDControlBuffer.bg2cnt.screenSize = 0;
    gLCDControlBuffer.bg2cnt.areaOverflowMode = 0;

    gLCDControlBuffer.bg0cnt.priority = 0;
    gLCDControlBuffer.bg1cnt.priority = 1;
    gLCDControlBuffer.bg2cnt.priority = 2;
    gLCDControlBuffer.bg3cnt.priority = 3;

    return;
}