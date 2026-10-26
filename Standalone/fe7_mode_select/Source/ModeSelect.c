/*
 * FE7-style mode select for clean FE8U.
 *
 * Replaces the New Game difficulty select of the FE8 save menu with the FE7
 * "Mode Select" screen: a carousel of lords (Eirika, Ephraim, Lyon) shown as
 * spinning battle animations, with a portrait, and a Normal / Hard choice.
 *
 * Ported from the C Skill System (Kernel/Wizardry/FE7ModeSelect), which is a
 * port of the FE7U functions noted next to each function below.
 *
 * The screen talks to the save menu exactly like vanilla's difficulty select
 * (NewNewGameDifficultySelect): it is started as a blocking child of the save
 * menu proc and reports the choice with SaveMenu_SetDifficultyChoice
 * (0 Easy, 1 Normal, 2 Difficult, 3 cancelled). The save menu then writes the
 * new game itself.
 */

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
#include "functions.h"

#include "constants/faces.h"

/* Settings from Installer.event */
extern const u8 gFe7ModeSelectConfig[];
#define CONFIG_UNLOCKS        (gFe7ModeSelectConfig[0])
#define CONFIG_NORMAL_CHOICE  (gFe7ModeSelectConfig[1])
#define CONFIG_HARD_CHOICE    (gFe7ModeSelectConfig[2])

/* Vanilla Hermite interpolation used by the FE8 difficulty menu (FE8U: 0x080AC6AC) */
int sub_80AC6AC(int a, int b, int c, int d, int e);

/* SaveMenu_SetDifficultyChoice() values */
#define DIFFICULTY_CHOICE_CANCEL 3

/*
 * Work RAM. The screen only runs on the title/save menu, where the low EWRAM
 * overlay (battle animation, ending, prep screen buffers) is free. These are
 * the same addresses FE7 uses. The save menu keeps a few things of its own in
 * the overlay (0x020007A0 .. 0x02000968: slot palette scratch, falling-sparkle
 * sprites, sub box font/text); they are saved before the screen starts and
 * restored when it ends.
 */
struct ModeSelectText
{
    struct Font font;
    struct Text text[7];
};

#define gModeSelectAnims      ((struct AnimBuffer *)        0x0201E8D4) /* [3] */
#define gModeSelectMagicFx    ((struct AnimMagicFxBuffer *) 0x0201E97C) /* [3] */
#define gModeSelectPalBackup  ((u16 *)                      0x0201E9F4) /* [3][15] */
#define gModeSelectHBlankLine (*(u8 *)                      0x0201EA4E)
#define gModeSelectHBlankBlend (*(u8 *)                     0x0201EA4F)
#define gModeSelectText       (*(struct ModeSelectText *)   0x0201EA50)
#define gModeSelectSaveBackup ((u32 *)                      0x0201EAA0)

#define SAVE_MENU_OVERLAY_START ((u32 *) 0x020007A0)
#define SAVE_MENU_OVERLAY_SIZE  (0x02000968 - 0x020007A0)

/* Battle animation buffers for the three carousel animations (FE7 layout). */
static u8 * const sAnimImgBuf[] = { (u8 *)0x020000F4, (u8 *)0x020020F4, (u8 *)0x020040F4 };
static u8 * const sAnimOamBuf[] = { (u8 *)0x020060F4, (u8 *)0x0200B8F4, (u8 *)0x020110F4 };
static u8 * const sAnimPalBuf[] = { (u8 *)0x020168F4, (u8 *)0x02016994, (u8 *)0x02016A34 };
static u8 * const sAnimScrBuf[] = { (u8 *)0x02016AD4, (u8 *)0x020194D4, (u8 *)0x0201BED4 };

struct ModeSelectProc
{
    /* 00 */ PROC_HEADER;
    /* 2C */ s32 timer;
    /* 30 */ u16 angle;
    /* 32 */ u16 targetAngle;
    /* 34 */ s32 direction;
    /* 38 */ void * spriteProc;
    /* 3C */ void * faceProc;
    /* 40 */ u8 unlocks;
    /* 41 */ u8 index;
    /* 42 */ u8 flags;        /* bit 0: new game (loads its own background) */
    /* 43 */ u8 hard[3];      /* per carousel entry: 0 Normal, 1 Hard */
    /* 46 */ STRUCT_PAD(0x46, 0x49);
    /* 49 */ u8 lord[3];      /* per carousel entry: 0 Eirika, 1 Ephraim, 2 Lyon */
    /* 4C */ u8 count;
    /* 50 */ s32 idleTimer;
};

struct ModeSelectSpriteDrawProc
{
    /* 00 */ PROC_HEADER;
    /* 2C */ s32 startBlink;
    /* 30 */ s32 clock;
    /* 34 */ s32 x;
    /* 38 */ s32 y;
    /* 3C */ u8 animsShown;
    /* 3E */ u16 angle;
    /* 40 */ s32 count;
    /* 44 */ s32 step;
    /* 48 */ s32 pulse;
    /* 4C */ u8 pulseDown;
    /* 4D */ u8 handRow;
    /* 4E */ u8 handFlags;   /* bit 1: frozen hand */
};

extern const u16 Pal_084138F0[];
extern const u16 Pal_0840F9A0[];
extern const u8 Img_08418E44[];
extern const u8 Img_0840FEB4[];
extern const u16 Tsa_0840FA00[];
extern const u16 Tsa_08411F34[];
extern const u16 Pal_084150C0[];
extern const u8 Img_08414940[];
extern const u16 Tsa_084150E0[];
extern const u16 Pal_08415AA0[];
extern const u8 Img_08415594[];
extern const u8 Tsa_08415AC0[];
extern const u16 Pal_0841625C[];

extern const u8 Img_08415BE8[];
extern const u8 Img_08415CB0[];
extern const u8 Img_08415DC4[];
extern const u8 Img_08415E04[];
extern const u8 Img_08415E54[];
extern const u8 Img_08415F14[];
extern const u8 Img_08415FF0[];
extern const u8 Img_0841601C[];
extern const u8 Img_08416058[];
extern const u8 Img_08416118[];
extern const u8 Img_084161F4[];
extern const u8 Img_08416220[];

/* Unlock bits (gFe7ModeSelectConfig[0]) */
#define UNLOCK_EIRIKA_HARD   (1 << 0)
#define UNLOCK_EPHRAIM       (1 << 1)
#define UNLOCK_EPHRAIM_HARD  (1 << 2)
#define UNLOCK_LYON          (1 << 3)
#define UNLOCK_LYON_HARD     (1 << 4)

static const u8 sHardUnlockBit[3] = { UNLOCK_EIRIKA_HARD, UNLOCK_EPHRAIM_HARD, UNLOCK_LYON_HARD };

static void ModeSelect_ApplyTsaLz(u16 * tm, const u8 * src, u16 base) /* FE7U: 0x080AACD8 */
{
    Decompress(src, gGenericBuffer);
    CallARM_FillTileRect(tm, gGenericBuffer, base);
}

/* FE7U: 0x080A32D4. Fades the top of the screen into the spell circle. */
static void ModeSelect_OnHBlank(void)
{
    u16 vcount = REG_VCOUNT + 1;

    if (vcount > DISPLAY_HEIGHT)
        vcount = 0;

    if ((vcount & 1) != 0)
        return;

    if (vcount < gModeSelectHBlankLine)
    {
        REG_BLDCNT = 0xC1;
        REG_BLDY = (gModeSelectHBlankLine != 0)
            ? (gModeSelectHBlankLine - vcount) * 0x10 / gModeSelectHBlankLine
            : 0;
    }
    else
    {
        REG_BLDCNT = 0x144;
        REG_BLDALPHA = gModeSelectHBlankBlend | 0x1000;
    }
}

static const u16 sBgConfig_ModeSelect[] = {
    0x0000, 0x6000, 0x0000,
    0xC000, 0x6800, 0x0000,
    0x8000, 0x7800, 0x0000,
    0x8000, 0x7800, 0x0000,
};

/* FE7U: 0x080A4E58. Background: spell circle (BG2, affine) and sky (BG3). */
static void ModeSelect_LoadBackground(void)
{
    SetupBackgrounds((u16 *)sBgConfig_ModeSelect);

    gLCDControlBuffer.dispcnt.mode = 1;

    gLCDControlBuffer.bg2cnt.screenSize = 1;
    gLCDControlBuffer.bg2cnt.areaOverflowMode = 0;

    gLCDControlBuffer.bg0cnt.priority = 3;
    gLCDControlBuffer.bg1cnt.priority = 0;
    gLCDControlBuffer.bg2cnt.priority = 2;
    gLCDControlBuffer.bg3cnt.priority = 2;

    EndAllMus();

    SetDispEnable(0, 0, 0, 0, 0);

    gModeSelectHBlankBlend = 10;
    gModeSelectHBlankLine = 100;

    SetPrimaryHBlankHandler(ModeSelect_OnHBlank);

    CopyToPaletteBuffer(Pal_084138F0, 0x220, 0x100);
    CopyToPaletteBuffer(Pal_0840F9A0, 0, 0x60);

    Decompress(Img_08418E44, (void *)(GetBackgroundTileDataOffset(BG_0) + VRAM));
    CallARM_FillTileRect(gBG0TilemapBuffer, Tsa_0840FA00, 0);

    Decompress(Img_0840FEB4, (void *)(GetBackgroundTileDataOffset(BG_3) + VRAM));
    sub_800154C(gBG3TilemapBuffer, Tsa_08411F34, 0, 5);

    BG_EnableSyncByMask(BG3_SYNC_BIT);
}

/* Battle animation IDs: lorf_sw1 (Eirika), lorm_sp1 (Ephraim), necm_mg1 (Lyon) */
static const int sLordBanim[3] = { 2, 0, 0x9C };

/* FE7U: 0x080A7480 */
static void InitModeSelectAnims(int count, const u8 * lords)
{
    int i;

    for (i = 0; i < count; i++)
    {
        struct AnimBuffer * anim = &gModeSelectAnims[i];
        struct AnimMagicFxBuffer * fx = &gModeSelectMagicFx[i];

        anim->xPos = 320;
        anim->yPos = 88;
        anim->animId = sLordBanim[lords[i]];
        anim->roundType = 6;
        anim->genericPalId = 0;
        anim->state2 = 1;
        anim->oam2Tile = (i * 0x2000 + 0x2000) >> 5;
        anim->oam2Pal = i + 0xD;

        anim->pImgSheetBuf = sAnimImgBuf[i];
        anim->unk_24 = sAnimOamBuf[i];
        anim->unk_20 = sAnimPalBuf[i];
        anim->unk_28 = sAnimScrBuf[i];

        anim->charPalId = -1;

        anim->unk_30 = fx;

        fx->magicFuncIdx = 0;
        fx->xOffsetBg = 0;
        fx->yOffsetBg = 0;
        fx->xOffsetObj = 0;
        fx->yOffsetObj = 0;
        fx->objChr = 0;
        fx->objPalId = 0;
        fx->bgChr = 0;
        fx->bgPalId = 0;
        fx->bg = 0;
        fx->bgTmBuf = NULL;
        fx->bgImgBuf = NULL;
        fx->bgTsaBuf = NULL;
        fx->objImgBuf = NULL;
        fx->resetCallback = NULL;

        NewEkrUnitMainMini(anim);
    }
}

/* FE7U: 0x080A75CC */
static void EndModeSelectAnims(int count)
{
    int i;

    for (i = 0; i < count; i++)
        sub_805AA28(&gModeSelectAnims[i]);
}

/* FE7U: 0x08054E5C */
static void ModeSelect_StopAnim(struct AnimBuffer * anim)
{
    anim->anim1->state3 |= 8;
    anim->anim2->state3 |= 8;
}

static const char StrModeSelect_MainCharacter[] = "Main character:";
static const char StrModeSelect_Weapon[] = "Weapon:";
static const char * const StrModeSelect_Wpn[] = { "Swd", "Lnc", "Mag" };

/* Text IDs (vanilla FE8U): "Eirika", "Ephraim", "Lyon"; "Normal", "Hard" */
static const u16 sLordNameMsg[3] = { 0x212, 0x220, 0x234 };
#define MSG_MODE_NORMAL 0x53E
#define MSG_MODE_HARD   0x53F

/* FE7U: 0x080A75F0 */
static void PutModeSelectLabelText(void)
{
    ClearText(&gModeSelectText.text[5]);
    ClearText(&gModeSelectText.text[6]);

    PutDrawText(&gModeSelectText.text[5], gBG1TilemapBuffer + TILEMAP_INDEX(14, 6), TEXT_COLOR_SYSTEM_WHITE, 0, 0, StrModeSelect_MainCharacter);
    PutDrawText(&gModeSelectText.text[6], gBG1TilemapBuffer + TILEMAP_INDEX(14, 10), TEXT_COLOR_SYSTEM_WHITE, 0, 0, StrModeSelect_Weapon);

    BG_EnableSyncByMask(BG1_SYNC_BIT);
}

/* FE7U: 0x080A7668 */
static void PutModeSelectCharacterText(int lord)
{
    ClearText(&gModeSelectText.text[2]);
    ClearText(&gModeSelectText.text[3]);
    ClearText(&gModeSelectText.text[4]);

    PutDrawText(&gModeSelectText.text[2], gBG1TilemapBuffer + TILEMAP_INDEX(14, 8), TEXT_COLOR_SYSTEM_BLUE, 0, 0, GetStringFromIndex(sLordNameMsg[lord]));
    PutDrawText(&gModeSelectText.text[4], gBG1TilemapBuffer + TILEMAP_INDEX(19, 10), TEXT_COLOR_SYSTEM_BLUE, 0, 0, StrModeSelect_Wpn[lord]);

    BG_EnableSyncByMask(BG1_SYNC_BIT);
}

static bool IsHardUnlocked(struct ModeSelectProc * proc, int lord)
{
    return (proc->unlocks & sHardUnlockBit[lord]) ? TRUE : FALSE;
}

/* FE7U: 0x080A76F8 */
static void PutModeSelectDifficultyText(struct ModeSelectProc * proc)
{
    int hard = proc->hard[proc->index];

    ClearText(&gModeSelectText.text[0]);
    ClearText(&gModeSelectText.text[1]);

    PutDrawText(&gModeSelectText.text[0], gBG1TilemapBuffer + TILEMAP_INDEX(15, 12), hard == 0 ? TEXT_COLOR_SYSTEM_GOLD : TEXT_COLOR_SYSTEM_GRAY, 0, 0, GetStringFromIndex(MSG_MODE_NORMAL));

    BG_EnableSyncByMask(BG1_SYNC_BIT);

    if (!IsHardUnlocked(proc, proc->lord[proc->index]))
        return;

    PutDrawText(&gModeSelectText.text[1], gBG1TilemapBuffer + TILEMAP_INDEX(15, 14), hard == 1 ? TEXT_COLOR_SYSTEM_GOLD : TEXT_COLOR_SYSTEM_GRAY, 0, 0, GetStringFromIndex(MSG_MODE_HARD));
}

static const int sLordFace[3] = {
    FID_EIRIKA,
    FID_EPHRAIM,
    0x46, /* Lyon */
};

/* FE7U: 0x080A77C0 */
static struct FaceProc * StartModeSelectFace(int lord)
{
    struct FaceProc * face = StartFace2(0, sLordFace[lord], 204, 72, (FACE_DISP_KIND(FACE_96x80) | FACE_DISP_HLAYER(FACE_HLAYER_0)));
    StartFaceFadeIn(face);

    return face;
}

/* FE7U: 0x080A77F8. "Chapter range" banner graphics (four pieces per lord). */
static const u8 * const sChapterGfx[3][4] = {
    { Img_08415BE8, Img_08415CB0, Img_08415DC4, Img_08415E04 },
    { Img_08415E54, Img_08415F14, Img_08415FF0, Img_0841601C },
    { Img_08416058, Img_08416118, Img_084161F4, Img_08416220 },
};

static void LoadModeSelectChapterGfx(int lord)
{
    Decompress(sChapterGfx[lord][0], (void *)0x060102C0);
    Decompress(sChapterGfx[lord][1], (void *)0x060106C0);
    Decompress(sChapterGfx[lord][2], (void *)0x06010AC0);
    Decompress(sChapterGfx[lord][3], (void *)0x06010EC0);
}

/* FE7U: 0x080A7860 */
static void ModeSelect_BackupAnimPal(int slot)
{
    int i;
    u16 * pal = gPaletteBuffer + (slot + 0xD) * 0x10 + 0x101;

    for (i = 0; i < 0xF; i++)
        gModeSelectPalBackup[i + slot * 0xF] = *pal++;
}

/* FE7U: 0x080A7890. Darkens an animation's palette by its depth in the carousel. */
static void ModeSelect_ShadeAnimPal(int slot, int amt)
{
    int i;
    u16 * pal = gPaletteBuffer + (slot + 0xD) * 0x10 + 0x101;

    if (amt > 0x40)
        amt = 0x40;

    amt = amt + (gModeSelectHBlankBlend - 10) * 2;

    for (i = 0; i < 0xF; i++)
    {
        int color = gModeSelectPalBackup[i + slot * 0xF];
        int r = (amt * (color & RED_MASK)) >> 6;
        int g = (amt * (color & GREEN_MASK)) >> 6;
        int b = (amt * (color & BLUE_MASK)) >> 6;

        r = r > RED_MASK ? RED_MASK : (r < 0 ? 0 : (r & RED_MASK));
        g = g > GREEN_MASK ? GREEN_MASK : (g < 0 ? 0 : (g & GREEN_MASK));
        b = b > BLUE_MASK ? BLUE_MASK : (b < 0 ? 0 : (b & BLUE_MASK));

        *pal++ = r + g + b;
    }

    EnablePaletteSync();
}

/* FE7U: 0x080A793C */
static void ModeSelect_ShadeAnimPalByAngle(int slot, int angle)
{
    int a = angle & 0xFF;
    int depth = (((a >= 0x81) ? a - 0x80 : 0x80 - a) * 0x30) >> 7;

    ModeSelect_ShadeAnimPal(slot, depth + 0x10);
}

/* FE7U: 0x080A796C */
static void ModeSelectSpriteDraw_Init(struct ModeSelectSpriteDrawProc * proc)
{
    proc->clock = 0;
    proc->angle = 0;
    proc->animsShown = 0;
    proc->x = DISPLAY_WIDTH / 2;
    proc->y = DISPLAY_HEIGHT;
    proc->count = 0;
    proc->step = 0;
    proc->pulse = 0;
    proc->pulseDown = 0;
    proc->startBlink = 0;
    proc->handFlags = 0;
}

// clang-format off

/* FE7U: 0x08CE483C */
static const u16 Sprite_ModeSelect_Mode[] =
{
    4,
    OAM0_SHAPE_32x16, OAM1_SIZE_32x16, 0,
    OAM0_SHAPE_32x16, OAM1_SIZE_32x16 + OAM1_X(32), OAM2_CHR(0x4),
    OAM0_SHAPE_32x8 + OAM0_Y(16), OAM1_SIZE_32x8, OAM2_CHR(0x40),
    OAM0_SHAPE_32x8 + OAM0_Y(16), OAM1_SIZE_32x8 + OAM1_X(32), OAM2_CHR(0x44),
};

/* FE7U: 0x08CE4856 */
static const u16 Sprite_ModeSelect_Select[] =
{
    6,
    OAM0_SHAPE_32x16, OAM1_SIZE_32x16, OAM2_CHR(0x8),
    OAM0_SHAPE_32x16, OAM1_SIZE_32x16 + OAM1_X(32), OAM2_CHR(0xC),
    OAM0_SHAPE_8x16, OAM1_SIZE_8x16 + OAM1_X(64), OAM2_CHR(0x10),
    OAM0_SHAPE_32x8 + OAM0_Y(16), OAM1_SIZE_32x8, OAM2_CHR(0x60),
    OAM0_SHAPE_32x8 + OAM0_Y(16), OAM1_SIZE_32x8 + OAM1_X(32), OAM2_CHR(0x64),
    OAM0_SHAPE_8x8 + OAM0_Y(16), OAM1_SIZE_8x8 + OAM1_X(64), OAM2_CHR(0x68),
};

/* FE7U: 0x08CE487C */
static const u16 Sprite_ModeSelect_PressStart[] =
{
    5,
    OAM0_SHAPE_32x8, OAM1_SIZE_32x8, OAM2_CHR(0x11),
    OAM0_SHAPE_32x8 + OAM0_Y(8), OAM1_SIZE_32x8, OAM2_CHR(0x49),
    OAM0_SHAPE_32x8, OAM1_SIZE_32x8 + OAM1_X(32), OAM2_CHR(0x31),
    OAM0_SHAPE_32x8 + OAM0_Y(8), OAM1_SIZE_32x8 + OAM1_X(32), OAM2_CHR(0x4D),
    OAM0_SHAPE_8x16, OAM1_SIZE_8x16 + OAM1_X(64), OAM2_CHR(0x55),
};

/* FE7U: 0x08CE489C */
static const u16 Sprite_ModeSelect_Change[] =
{
    1,
    OAM0_SHAPE_32x8, OAM1_SIZE_32x8, OAM2_CHR(0x51),
};

/* FE7U: 0x08CE48A4 */
static const u16 Sprite_ModeSelect_ChapterRange[] =
{
    4,
    OAM0_SHAPE_32x16 + OAM0_AFFINE_ENABLE, OAM1_SIZE_32x16, OAM2_CHR(0x16),
    OAM0_SHAPE_32x16 + OAM0_AFFINE_ENABLE, OAM1_SIZE_32x16 + OAM1_X(32), OAM2_CHR(0x1A),
    OAM0_SHAPE_16x16 + OAM0_AFFINE_ENABLE, OAM1_SIZE_16x16 + OAM1_X(64), OAM2_CHR(0x1E),
    OAM0_SHAPE_32x16 + OAM0_AFFINE_ENABLE, OAM1_SIZE_32x16 + OAM1_X(80), OAM2_CHR(0x56),
};

// clang-format on

/* FE7U: 0x080A79A4 */
static void ModeSelectSpriteDraw_Loop(struct ModeSelectSpriteDrawProc * proc)
{
    int i;

    if (proc->animsShown != 0)
    {
        for (i = 0; i < proc->count; i++)
        {
            int angle = ((proc->angle >> 4) + i * proc->step + 40);
            int x = (proc->x << 12) + (SIN(angle) * 70);
            int y = (((proc->y << 12) + (COS(angle) * 28)) >> 12) - 16;

            sub_805A940(&gModeSelectAnims[i], x >> 12, y);

            ModeSelect_ShadeAnimPalByAngle(i, (proc->angle >> 4) + i * proc->step);
        }
    }

    BgAffinRotScaling(BG_2, proc->angle, 0, 0, 0x160, 0x160);
    BgAffinScaling(BG_2, 0x280, 0x100);
    BgAffinAnchoring(BG_2, proc->x, proc->y, 76, 76);

    gModeSelectHBlankBlend = sub_80AC6AC(8, 8, 16, 16, proc->pulse);

    if (proc->pulseDown == 0)
    {
        proc->pulse += 8;

        if (proc->pulse >= 0x400)
            proc->pulseDown = 1;
    }
    else
    {
        proc->pulse -= 8;

        if (proc->pulse <= 0)
            proc->pulseDown = 0;
    }

    if (proc->handFlags & 2)
        DisplayFrozenUiHandExt(108, (proc->handRow & 1) * 16 + 104, OAM2_CHR(0x3C0) + OAM2_LAYER(2));
    else
        DisplayUiHandExt(108, proc->handRow * 16 + 104, OAM2_CHR(0x3C0) + OAM2_LAYER(2));

    PutSpriteExt(0xD, 0, 8, Sprite_ModeSelect_Mode, OAM2_PAL(11));
    PutSpriteExt(0xD, 20, 28, Sprite_ModeSelect_Select, OAM2_PAL(11));
    PutSpriteExt(0xD, 40, 64, Sprite_ModeSelect_Change, OAM2_PAL(11));

    if (((proc->startBlink >> 2) & 1) == 0)
        PutSpriteExt(0xD, 8, 130, Sprite_ModeSelect_PressStart, OAM2_PAL(11));

    if (proc->startBlink != 0)
        proc->startBlink++;

    PutSpriteExt(0xD, 108, 24, Sprite_ModeSelect_ChapterRange, OAM2_PAL(10));

    proc->clock++;
}

// clang-format off

/* FE7U: 0x08CE48F0 */
static const struct ProcCmd ProcScr_ModeSelectSpriteDraw[] =
{
    PROC_CALL(ModeSelectSpriteDraw_Init),
    PROC_YIELD,

    PROC_REPEAT(ModeSelectSpriteDraw_Loop),

    PROC_END,
};

// clang-format on

static struct ModeSelectSpriteDrawProc * FindSpriteDraw(void)
{
    return Proc_Find(ProcScr_ModeSelectSpriteDraw);
}

static void SpriteDraw_StartBlink(void) /* FE7U: 0x080A7B7C */
{
    struct ModeSelectSpriteDrawProc * proc = FindSpriteDraw();

    if (proc != NULL)
        proc->startBlink = 1;
}

static void SpriteDraw_ShowAnims(void) /* FE7U: 0x080A7B98 */
{
    struct ModeSelectSpriteDrawProc * proc = FindSpriteDraw();

    if (proc != NULL)
        proc->animsShown = 1;
}

static void SpriteDraw_SetCount(int count) /* FE7U: 0x080A7BB4 */
{
    struct ModeSelectSpriteDrawProc * proc = FindSpriteDraw();

    if (proc != NULL)
    {
        proc->count = count;
        proc->step = 0x100 / count;
    }
}

static void SpriteDraw_SetCenter(int x, int y) /* FE7U: 0x080A7BDC */
{
    struct ModeSelectSpriteDrawProc * proc = FindSpriteDraw();

    if (proc != NULL)
    {
        proc->x = x;
        proc->y = y;
    }

    gModeSelectHBlankLine = y - 60;
}

static void SpriteDraw_SetAngle(u16 angle) /* FE7U: 0x080A7C08 */
{
    struct ModeSelectSpriteDrawProc * proc = FindSpriteDraw();

    if (proc != NULL)
        proc->angle = angle;
}

static void SpriteDraw_SetHand(u8 row, u8 flags) /* FE7U: 0x080A7C24 */
{
    struct ModeSelectSpriteDrawProc * proc = FindSpriteDraw();

    if (proc != NULL)
    {
        proc->handRow = row;
        proc->handFlags = flags;
    }
}

static int SpriteDraw_GetStep(void) /* FE7U: 0x080A7C4C */
{
    struct ModeSelectSpriteDrawProc * proc = FindSpriteDraw();

    return proc != NULL ? proc->step : 0;
}

/* FE7U: 0x080A7C6C */
static void ModeSelect_InitGfx(struct ModeSelectProc * proc)
{
    /* Keep the save menu's own data that lives in the EWRAM overlay. */
    CpuFastCopy(SAVE_MENU_OVERLAY_START, gModeSelectSaveBackup, SAVE_MENU_OVERLAY_SIZE);

    if (proc->flags & 1)
        ModeSelect_LoadBackground();
}

/* FE7U: 0x08CE4910 */
static const struct FaceVramEntry FaceConfig_ModeSelect[] = {
    { .tileOffset = 0x1000, .paletteId = 0xC },
    { .tileOffset = 0x1000, .paletteId = 0xC },
    { .tileOffset = 0x1000, .paletteId = 0xC },
    { .tileOffset = 0x1000, .paletteId = 0xC },
};

static void ModeSelect_SetObjAffineIdentity(void)
{
    // clang-format off
    SetObjAffine(
        0,
        Div(+COS(0) * 16, 0x100),
        Div(-SIN(0) * 16, 0x100),
        Div(+SIN(0) * 16, 0x100),
        Div(+COS(0) * 16, 0x100)
    );
    // clang-format on
}

/* FE7U: 0x080A7C84 */
static void ModeSelect_Init(struct ModeSelectProc * proc)
{
    int i;

    LoadObjUIGfx();

    BG_SetPosition(BG_1, 8, -8);

    Proc_BlockEachMarked(PROC_MARK_SAVEDRAW);
    Proc_BlockEachMarked(PROC_MARK_D);

    gModeSelectHBlankLine = 100;

    SetupFaceGfxData((struct FaceVramEntry *)FaceConfig_ModeSelect);

    ApplyPalette(Pal_08415AA0, 0xF);

    Decompress(Img_08415594, (void *)(VRAM + GetBackgroundTileDataOffset(BG_1)));
    CallARM_FillTileRect(gBG0TilemapBuffer, Tsa_084150E0, 0);
    ModeSelect_ApplyTsaLz(gBG1TilemapBuffer, Tsa_08415AC0, 0xF000);
    ApplyPalette(Pal_084150C0, 0x1B);

    Decompress(Img_08414940, (void *)0x06010000);
    ApplyPalette(Pal_0841625C, 0x1A);

    NewEfxAnimeDrvProc();
    ResetClassReelSpell();

    proc->spriteProc = Proc_Start(ProcScr_ModeSelectSpriteDraw, proc);
    SpriteDraw_SetCenter(0, 0x70);

    proc->index = 0;
    proc->count = 0;

    proc->unlocks = CONFIG_UNLOCKS;

    /* New game (FE7 "flags & 1"): Eirika always, Ephraim and Lyon if unlocked */
    proc->lord[proc->count++] = 0;

    if (proc->unlocks & UNLOCK_EPHRAIM)
        proc->lord[proc->count++] = 1;

    if (proc->unlocks & UNLOCK_LYON)
        proc->lord[proc->count++] = 2;

    for (i = 0; i < proc->count; i++)
        proc->hard[i] = 0;

    SpriteDraw_SetCount(proc->count);
    InitModeSelectAnims(proc->count, proc->lord);

    for (i = 0; i < proc->count; i++)
        ModeSelect_BackupAnimPal(i);

    SpriteDraw_ShowAnims();
    StartUiSpinningArrows(proc);
    LoadUiSpinningArrowGfx(0, 0xD20, 9);
    SetUiSpinningArrowPositions(30, 61, 68, 61);
    SetUiSpinningArrowConfig(3);

    InitTextFont(&gModeSelectText.font, (void *)0x0600E000, 0x100, 0xE);

    InitText(&gModeSelectText.text[0], 5);
    InitText(&gModeSelectText.text[1], 9);
    InitText(&gModeSelectText.text[2], 5);
    InitText(&gModeSelectText.text[3], 8);
    InitText(&gModeSelectText.text[4], 4);
    InitText(&gModeSelectText.text[5], 10);
    InitText(&gModeSelectText.text[6], 5);

    proc->angle = proc->index * SpriteDraw_GetStep() * 0x10;

    proc->faceProc = StartModeSelectFace(proc->lord[proc->index]);
    PutModeSelectLabelText();
    PutModeSelectCharacterText(proc->lord[proc->index]);
    PutModeSelectDifficultyText(proc);
    SpriteDraw_SetHand(proc->hard[proc->index], proc->flags);
    SpriteDraw_SetAngle(proc->angle);

    BG_EnableSyncByMask(BG0_SYNC_BIT | BG1_SYNC_BIT);

    proc->timer = 0;
    proc->idleTimer = 0;

    SetWinEnable(1, 0, 0);
    SetWin0Layers(1, 1, 1, 1, 1);
    SetWin0Box(0, 0x50, 0xF0, 0x50);
    SetWOutLayers(0, 0, 0, 0, 0);

    LoadModeSelectChapterGfx(proc->lord[proc->index]);

    ModeSelect_SetObjAffineIdentity();
}

/* FE7U: 0x080A8054 */
static void ModeSelect_TransitionSplitOpen(struct ModeSelectProc * proc)
{
    int t = ++proc->timer;
    int h;

    SetDispEnable(1, 1, 1, 1, 1);

    h = 0x48 - (((0x10 - t) * 0x48) * (0x10 - t) / 256);

    SetWin0Box(0, 0x50 - h, 0xF0, h + 0x50);

    if (t == 0x10)
        Proc_Break(proc);
}

/* FE7U: 0x080A80C4 */
static void ModeSelect_TransitionSplitClose(struct ModeSelectProc * proc)
{
    int t = ++proc->timer;
    int h;

    h = 0x48 - (((0x10 - t) * 0x48) * (0x10 - t) / 256);

    SetWin0Box(0, h + 8, 0xF0, -0x68 - h);

    if (t == 0x10)
        Proc_Break(proc);
}

/* FE7U: 0x080A8120 */
static void ModeSelect_StopAllAnims(struct ModeSelectProc * proc)
{
    int i;

    for (i = 0; i < proc->count; i++)
        ModeSelect_StopAnim(&gModeSelectAnims[i]);

    proc->idleTimer = 0;
}

/* FE7U: 0x080A8150 */
static void ModeSelect_SetHard(struct ModeSelectProc * proc, int hard)
{
    proc->hard[proc->index] = hard;

    PutModeSelectDifficultyText(proc);
    SpriteDraw_SetHand(hard, proc->flags);
}

/* FE7U: 0x080A817C */
static void ModeSelect_Loop_KeyHandler(struct ModeSelectProc * proc)
{
    if ((gKeyStatusPtr->repeatedKeys & DPAD_UP) && proc->hard[proc->index] == 1)
    {
        PlaySoundEffect(0x66);
        ModeSelect_SetHard(proc, 0);
        return;
    }

    if ((gKeyStatusPtr->repeatedKeys & DPAD_DOWN) && proc->hard[proc->index] == 0)
    {
        if (!IsHardUnlocked(proc, proc->lord[proc->index]))
        {
            PlaySoundEffect(0x6C);
            return;
        }

        PlaySoundEffect(0x66);
        ModeSelect_SetHard(proc, 1);
        return;
    }

    if (gKeyStatusPtr->heldKeys & (DPAD_LEFT | L_BUTTON))
    {
        Proc_Goto(proc, 1);

        SetUiSpinningArrowFastMaybe(0);
        PlaySoundEffect(0x67);
        ModeSelect_StopAllAnims(proc);
        return;
    }

    if (gKeyStatusPtr->heldKeys & (DPAD_RIGHT | R_BUTTON))
    {
        Proc_Goto(proc, 2);

        SetUiSpinningArrowFastMaybe(1);
        PlaySoundEffect(0x67);
        ModeSelect_StopAllAnims(proc);
        return;
    }

    if (gKeyStatusPtr->newKeys & (START_BUTTON | A_BUTTON))
    {
        proc->timer = 0;

        PlaySoundEffect(0x6A);

        Proc_Goto(proc, 3);

        gModeSelectAnims[proc->index].roundType = 0;
        sub_805A7B4(&gModeSelectAnims[proc->index]);

        SaveMenu_SetDifficultyChoice(
            proc->hard[proc->index] ? CONFIG_HARD_CHOICE : CONFIG_NORMAL_CHOICE, 0);

        SpriteDraw_SetHand(proc->hard[proc->index], proc->flags | 2);
        SpriteDraw_StartBlink();
        return;
    }

    if (gKeyStatusPtr->newKeys & B_BUTTON)
    {
        proc->timer = 0;

        PlaySoundEffect(0x6B);

        Proc_Goto(proc, 4);
        SaveMenu_SetDifficultyChoice(DIFFICULTY_CHOICE_CANCEL, 0);
        return;
    }

    proc->idleTimer++;

    if ((proc->idleTimer & 0x1FF) == 0x20)
    {
        gModeSelectAnims[proc->index].roundType = 2;
        sub_805A7B4(&gModeSelectAnims[proc->index]);
    }

    if ((proc->idleTimer & 0x1FF) == 0x80)
        ModeSelect_StopAnim(&gModeSelectAnims[proc->index]);
}

/* FE7U: 0x080A8424 */
static void ModeSelect_RotateRight(struct ModeSelectProc * proc)
{
    proc->direction = -1;
    proc->timer = 0;

    StartFaceFadeOut(proc->faceProc);

    if (proc->index == 0)
        proc->index = proc->count - 1;
    else
        proc->index--;

    proc->targetAngle = (0x100 - SpriteDraw_GetStep() * proc->index) << 4;

    ModeSelect_SetHard(proc, proc->hard[proc->index]);

    if (proc->targetAngle < proc->angle)
        proc->targetAngle += 0x1000;
}

/* FE7U: 0x080A848C */
static void ModeSelect_RotateLeft(struct ModeSelectProc * proc)
{
    proc->direction = 1;
    proc->timer = 0;

    StartFaceFadeOut(proc->faceProc);

    if (proc->index < (proc->count - 1))
        proc->index++;
    else
        proc->index = 0;

    proc->targetAngle = (0x100 - SpriteDraw_GetStep() * proc->index) << 4;

    ModeSelect_SetHard(proc, proc->hard[proc->index]);

    if (proc->targetAngle > proc->angle)
        proc->angle += 0x1000;
}

/* FE7U: 0x080A84F8 */
static void ModeSelect_Loop_RotateCarousel(struct ModeSelectProc * proc)
{
    int a, b, c;
    u16 angle;

    a = (proc->targetAngle - proc->angle) * proc->direction;
    proc->timer++;

    b = a >> 2;

    c = b * (0x1E - proc->timer) * (0x1E - proc->timer) / 900;
    angle = (proc->angle + proc->direction * 4 * (b - c));

    if (proc->timer == 13)
        LoadModeSelectChapterGfx(proc->lord[proc->index]);

    if (proc->timer == 14)
        proc->faceProc = StartModeSelectFace(proc->lord[proc->index]);

    if (proc->timer == 20)
        PutModeSelectCharacterText(proc->lord[proc->index]);

    if (proc->timer == 30)
    {
        angle = proc->targetAngle & 0xFFF;
        proc->angle = proc->targetAngle & 0xFFF;
        Proc_Break(proc);
    }

    ModeSelect_SetObjAffineIdentity();

    SpriteDraw_SetAngle(angle);
}

/* FE7U: 0x080A8624 */
static void ModeSelect_End(struct ModeSelectProc * proc)
{
    EndModeSelectAnims(proc->count);
    EndEfxAnimeDrvProc();
    EndFaceById(0);
    SetupFaceGfxData(NULL);

    SetPrimaryHBlankHandler(NULL);

    CpuFastCopy(gModeSelectSaveBackup, SAVE_MENU_OVERLAY_START, SAVE_MENU_OVERLAY_SIZE);

    /* Give the save menu back its BG layout (same as SaveMenu_Init). */
    SetupBackgrounds(gBgConfig_SaveMenu);
    gLCDControlBuffer.dispcnt.mode = 0;
    gLCDControlBuffer.bg0cnt.priority = 0;
    gLCDControlBuffer.bg1cnt.priority = 1;
    gLCDControlBuffer.bg2cnt.priority = 2;
    gLCDControlBuffer.bg3cnt.priority = 3;

    /* The save menu runs SaveMenu_ReloadScreenFormDifficulty after this proc
     * ends, but that only syncs BG0/BG1: vanilla's difficulty menu never
     * touches BG2/BG3. This screen does, so reload here and sync all four. */
    SaveMenu_ReloadScreenFormDifficulty(proc->proc_parent);
    BG_EnableSyncByMask(BG0_SYNC_BIT | BG1_SYNC_BIT | BG2_SYNC_BIT | BG3_SYNC_BIT);
}

// clang-format off

/* FE7U: 0x08CE4930 */
static const struct ProcCmd ProcScr_ModeSelect[] =
{
    PROC_CALL(DisableAllGfx),
    PROC_YIELD,

    PROC_CALL(ModeSelect_InitGfx),
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
    PROC_CALL_ARG(NewFadeOut, 8),
    PROC_WHILE(FadeOutExists),
    PROC_CALL(ModeSelect_End),

    PROC_END,
};

// clang-format on

/* FE7U: 0x080A8664. Called from the save menu proc in place of
 * NewNewGameDifficultySelect. */
void StartModeSelect(ProcPtr parent)
{
    struct ModeSelectProc * proc = Proc_StartBlocking(ProcScr_ModeSelect, parent);
    proc->flags = 1;
}
