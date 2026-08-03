#include "common-chax.h"
#include "icon-rework.h"
#include "stat-screen.h"
#include "skill-system.h"
#include "bwl.h"
#include "jester_headers/custom-structs.h"
#include "jester_headers/custom-functions.h"

extern u16 const *const *const gpSprites_PageNameRework;
extern u16 const *const gpPageNameChrOffsetLutRe;
extern u16 const *const gpPageNamePaletteRe;

extern const u16 Pal_PresitgeStar[];
extern const u8 Gfx_PrestigeStar[];

STATIC_DECLAR void DisplayPrestigeStars(void)
{
    struct NewBwl *bwl = GetNewBwl(UNIT_CHAR_ID(gStatScreen.unit));
    int starCount;

    if (!bwl)
        return;

    if (bwl->prestigeAmt == 0)
        return;

    starCount = bwl->prestigeAmt;
    if (starCount > 3)
        starCount = 3;

    Decompress(Gfx_PrestigeStar, gGenericBuffer);
    Copy2dChr(gGenericBuffer, (void*)0x6016880, 2, 2);
    ApplyPalette(Pal_PresitgeStar, 1);

    for (int i = 0; i < starCount; i++) {
        PutSprite(4, 45 + i * 16, 98, gObject_16x16, TILEREF(0x344, 0x0));
    }
}

LYN_REPLACE_CHECK(DisplayPageNameSprite);
void DisplayPageNameSprite(int pageid)
{
	int colorid;
	int realPageId = TranslateStatPageId(pageid);

	if (realPageId < 0)
		realPageId = 0;
	else if (realPageId > PAGE_SKILL_TREE)
		realPageId = PAGE_SKILL_TREE;

    /* Display the little arrows either side of the page name */
	PutSprite(4,
		111 + gStatScreen.xDispOff, 1 + gStatScreen.yDispOff,
		sSprite_PageNameBack, TILEREF(0x293, 4) + 0xC00);

    /* Display stat screen title */
	PutSprite(4,
		114 + gStatScreen.xDispOff, 0 + gStatScreen.yDispOff,
		gpSprites_PageNameRework[realPageId],
		TILEREF(0x240 + gpPageNameChrOffsetLutRe[realPageId], 3) + 0xC00);

	colorid = (GetGameClock()/4) % 16;

	CpuCopy16(
		gpPageNamePaletteRe + colorid,
		PAL_OBJ(3) + 0xE,
		sizeof(u16));

	EnablePaletteSync();
}

struct TextInitInfo const sSSMasterTextInitInfo_NEW[] =
{
    { gStatScreen.text + STATSCREEN_TEXT_CHARANAME,  7  }, // 0
    { gStatScreen.text + STATSCREEN_TEXT_CLASSNAME,  8  }, // 1
    { gStatScreen.text + STATSCREEN_TEXT_UNUSUED,    16 }, // 2
    { gStatScreen.text + STATSCREEN_TEXT_POWLABEL,   3  }, // 3
    { gStatScreen.text + STATSCREEN_TEXT_SKLLABEL,   3  }, // 4
    { gStatScreen.text + STATSCREEN_TEXT_SPDLABEL,   3  }, // 5
    { gStatScreen.text + STATSCREEN_TEXT_LCKLABEL,   3  }, // 6
    { gStatScreen.text + STATSCREEN_TEXT_DEFLABEL,   3  }, // 7
    { gStatScreen.text + STATSCREEN_TEXT_RESLABEL,   3  }, // 8
    { gStatScreen.text + STATSCREEN_TEXT_MOVLABEL,   3  }, // 9
    { gStatScreen.text + STATSCREEN_TEXT_CONLABEL,   3  }, // 10
    { gStatScreen.text + STATSCREEN_TEXT_AIDLABEL,   3  }, // 11
    { gStatScreen.text + STATSCREEN_TEXT_RESCUENAME, 9  }, // 12
    { gStatScreen.text + STATSCREEN_TEXT_AFFINLABEL, 7  }, // 13 
    { gStatScreen.text + STATSCREEN_TEXT_STATUS,     9  }, // 14
    { gStatScreen.text + STATSCREEN_TEXT_ITEM0,      8  }, // 15 
    { gStatScreen.text + STATSCREEN_TEXT_ITEM1,      8  }, // 16
    { gStatScreen.text + STATSCREEN_TEXT_ITEM2,      8  }, // 17
    { gStatScreen.text + STATSCREEN_TEXT_ITEM3,      8  }, // 18
    { gStatScreen.text + STATSCREEN_TEXT_ITEM4,      8  }, // 19
    { gStatScreen.text + STATSCREEN_TEXT_BSRANGE,    7  }, // 20
    { gStatScreen.text + STATSCREEN_TEXT_BSATKLABEL, 3  }, // 21
    { gStatScreen.text + STATSCREEN_TEXT_BSHITLABEL, 3  }, // 22
    { gStatScreen.text + STATSCREEN_TEXT_BSCRTLABEL, 3  }, // 23
    { gStatScreen.text + STATSCREEN_TEXT_BSAVOLABEL, 4  }, // 24
    { gStatScreen.text + STATSCREEN_TEXT_WEXP0,      2  }, // 25
    { gStatScreen.text + STATSCREEN_TEXT_WEXP1,      2  }, // 26
    { gStatScreen.text + STATSCREEN_TEXT_WEXP2,      2  }, // 27
    { gStatScreen.text + STATSCREEN_TEXT_WEXP3,      2  }, // 28
    { gStatScreen.text + STATSCREEN_TEXT_SUPPORT0,   7  }, // 29
    { gStatScreen.text + STATSCREEN_TEXT_SUPPORT1,   7  }, // 30
    { gStatScreen.text + STATSCREEN_TEXT_SUPPORT2,   7  }, // 31
    { gStatScreen.text + STATSCREEN_TEXT_SUPPORT3,   7  }, // 32
    { gStatScreen.text + STATSCREEN_TEXT_SUPPORT4,   7  }, // 33
    { gStatScreen.text + STATSCREEN_TEXT_BWL,        16 }, // 34

    { }, // end
};

LYN_REPLACE_CHECK(InitTexts);
void InitTexts(void)
{
    InitTextInitInfo(sSSMasterTextInitInfo_NEW);
}

LYN_REPLACE_CHECK(StatScreen_Display);
void StatScreen_Display(struct Proc* proc)
{
    int pageAmt = GetStatPageCount();

    int fid = GetUnitPortraitId(gStatScreen.unit);

    if (gStatScreen.unit->state & US_BIT23)
        fid++;

    // Set page amount (in FE6, this was dependant on whether this is ally or enemy)
    gStatScreen.pageAmt = pageAmt;

    if (gStatScreen.pageAmt <= 0)
        gStatScreen.pageAmt = 1;

    if (gStatScreen.page >= gStatScreen.pageAmt)
        gStatScreen.page = gStatScreen.pageAmt - 1;

    // Init text and icons
    ResetText();
    ResetIconGraphics_();
    InitTexts();

    // Display portrait
    PutFace80x72(
        proc,
        gBG2TilemapBuffer + TILEMAP_INDEX(1, 1),
        fid,
        0x4E0,
        STATSCREEN_BGPAL_FACE
    );

    if (GetPortraitData(fid)->img)
        ApplyPalette(gUnknown_08A01EE4, STATSCREEN_BGPAL_2);
    else
        ApplyPalette(gUnknown_08A01F04, STATSCREEN_BGPAL_2);

    // Display Map Sprite
    EndAllMus();
    gStatScreen.mu = StartUiMu(gStatScreen.unit, 80, 138);

    // Draw left panel labels and info
    DisplayLeftPanel();

    // Draw page content
    DisplayPage(gStatScreen.page);

    TileMap_CopyRect(gUiTmScratchA, gBG0TilemapBuffer + TILEMAP_INDEX(12, 2), 18, 18);
    TileMap_CopyRect(gUiTmScratchC, gBG2TilemapBuffer + TILEMAP_INDEX(12, 2), 18, 18);

    BG_EnableSyncByMask(BG0_SYNC_BIT | BG1_SYNC_BIT | BG2_SYNC_BIT);
}

enum
{
    // Magical constants

    // Neutral left arrow position
    PAGENUM_LEFTARROW_X = 103,
    PAGENUM_LEFTARROW_Y = 3,

    // Neutral right arrow position
    PAGENUM_RIGHTARROW_X = 217,
    PAGENUM_RIGHTARROW_Y = 3,

    // initial arrow offset on select
    PAGENUM_SELECT_XOFF = 6,

    // arrow animation speeds
    PAGENUM_ANIMSPEED = 4,
    PAGENUM_SELECT_ANIMSPEED = 31,

    PAGENUM_DISPLAY_X = 215,
    PAGENUM_DISPLAY_Y = 17,

    // name animation scaling time
    PAGENAME_SCALE_TIME = 6,
};

int GetStatPageCount(void)
{
    int count = 4;

    if (IsStatScreenPageAvailable(PAGE_GAIDEN_MAGIC))
        count++;

    if (IsStatScreenPageAvailable(PAGE_PERSONAL_DATA))
        count++;

    if (IsStatScreenPageAvailable(PAGE_PROMOTIONS))
        count++;

    if (IsStatScreenPageAvailable(PAGE_SKILL_TREE))
        count++;

    return count;
}


LYN_REPLACE_CHECK(PageNumCtrl_UpdatePageNum);
void PageNumCtrl_UpdatePageNum(struct StatScreenPageNameProc* proc)
{
    int chr = 0x289;
    int pageAmt = gStatScreen.pageAmt;
    int page = gStatScreen.page;

    /* Standard page number calculations up to page 6 starting at 0x6015100 in the OBJ tile view in NoCashGBA */
    int pageAmtShift;
    int pageNumShift;

    if (pageAmt <= 0)
        pageAmt = 1;

    if (page >= pageAmt) {
        page = pageAmt - 1;
        gStatScreen.page = page;
    }

    pageAmtShift = pageAmt - 1;
    pageNumShift = page;

    /* Pages 7,8 and 9 are listed below that starting at 0x6015500 in the OBJ tile view in NoCashGBA  */
    if (pageAmt > 6)
        pageAmtShift = 0x20 - 1;
    
    if (page > 5)
        pageNumShift = (page - 7) + 0x20;

    // page amt
    PutSprite(2,
        gStatScreen.xDispOff + PAGENUM_DISPLAY_X + 13,
        gStatScreen.yDispOff + PAGENUM_DISPLAY_Y,
        gObject_8x8, TILEREF(chr, STATSCREEN_OBJPAL_4) + OAM2_LAYER(3) + pageAmtShift);

    // '/'
    PutSprite(2,
        gStatScreen.xDispOff + PAGENUM_DISPLAY_X + 7,
        gStatScreen.yDispOff + PAGENUM_DISPLAY_Y,
        gObject_8x8, TILEREF(chr, STATSCREEN_OBJPAL_4) + OAM2_LAYER(3) - 1);

    // page num
    PutSprite(2,
        gStatScreen.xDispOff + PAGENUM_DISPLAY_X,
        gStatScreen.yDispOff + PAGENUM_DISPLAY_Y,
        gObject_8x8, TILEREF(chr, STATSCREEN_OBJPAL_4) + OAM2_LAYER(3) + pageNumShift);
}

void GetPromotedUnitDescId(struct HelpBoxProc* proc)
{
    struct Unit *unit = gStatScreen.unit;
    int charId = UNIT_CHAR_ID(unit);
    const UnitPromotions *promo_data = NULL;

    for (int i = 0; unit_promotions[i].key != 0; i++)
    {
        if (unit_promotions[i].key == charId)
        {
            promo_data = &unit_promotions[i];
            break;
        }
    }

    if (promo_data != NULL)
    {
        if (proc->info->yDisplay == 0x20)
        {
            if (promo_data->promotions[0].classId)
                proc->mid = GetClassData(promo_data->promotions[0].classId)->descTextId;
        }
        else if (proc->info->yDisplay == 0x48)
        {
            if (promo_data->promotions[1].classId)
                proc->mid = GetClassData(promo_data->promotions[1].classId)->descTextId;
        }
        else if (proc->info->yDisplay == 0x70)
        {
            if (promo_data->promotions[2].classId)
                proc->mid = GetClassData(promo_data->promotions[2].classId)->descTextId;
        }
        else
            gKeyStatusPtr->newKeys = B_BUTTON; // Prevent empty text boxes
            // proc->mid = 0;
    }
}

void GetPromotedUnitSkillId(struct HelpBoxProc* proc)
{
    struct Unit *unit = gStatScreen.unit;
    int charId = UNIT_CHAR_ID(unit);
    const UnitPromotions *promo_data = NULL;

    for (int i = 0; unit_promotions[i].key != 0; i++)
    {
        if (unit_promotions[i].key == charId)
        {
            promo_data = &unit_promotions[i];
            break;
        }
    }

    if (promo_data != NULL)
    {
        if (proc->info->xDisplay == (0x12 * 0x8) && proc->info->yDisplay == (0x5 * 0x8))
            proc->mid = GetSkillDescMsg(promo_data->promotions[0].skills[0]);
        else if (proc->info->xDisplay == (0x14 * 0x8) && proc->info->yDisplay == (0x5 * 0x8))
            proc->mid = GetSkillDescMsg(promo_data->promotions[0].skills[1]);
        else if (proc->info->xDisplay == (0x16 * 0x8) && proc->info->yDisplay == (0x5 * 0x8))
            proc->mid = GetSkillDescMsg(promo_data->promotions[0].skills[2]);

        else if (proc->info->xDisplay == (0x12 * 0x8) && proc->info->yDisplay == (0xA * 0x8))
            proc->mid = GetSkillDescMsg(promo_data->promotions[1].skills[0]);
        else if (proc->info->xDisplay == (0x14 * 0x8) && proc->info->yDisplay == (0xA * 0x8))
            proc->mid = GetSkillDescMsg(promo_data->promotions[1].skills[1]);
        else if (proc->info->xDisplay == (0x16 * 0x8) && proc->info->yDisplay == (0xA * 0x8))
            proc->mid = GetSkillDescMsg(promo_data->promotions[1].skills[2]);

        else if (proc->info->xDisplay == (0x12 * 0x8) && proc->info->yDisplay == (0xF * 0x8))
            proc->mid = GetSkillDescMsg(promo_data->promotions[2].skills[0]);
        else if (proc->info->xDisplay == (0x14 * 0x8) && proc->info->yDisplay == (0xF * 0x8))
            proc->mid = GetSkillDescMsg(promo_data->promotions[2].skills[1]);
        else if (proc->info->xDisplay == (0x16 * 0x8) && proc->info->yDisplay == (0xF * 0x8))
            proc->mid = GetSkillDescMsg(promo_data->promotions[2].skills[2]);
    }

    if (proc->mid == 0)
        CloseHelpBox();
}

// Select graphic
static const void* capacityGfx[9] = {
    Gfx_Skill_Capacity_Circle_0_8,
    Gfx_Skill_Capacity_Circle_1_8,
    Gfx_Skill_Capacity_Circle_2_8,
    Gfx_Skill_Capacity_Circle_3_8,
    Gfx_Skill_Capacity_Circle_4_8,
    Gfx_Skill_Capacity_Circle_5_8,
    Gfx_Skill_Capacity_Circle_6_8,
    Gfx_Skill_Capacity_Circle_7_8,
    Gfx_Skill_Capacity_Circle_8_8,
};

LYN_REPLACE_CHECK(PageNumCtrl_DisplayMuPlatform);
void PageNumCtrl_DisplayMuPlatform(struct StatScreenPageNameProc *proc)
{
    /* Display unit platform on left side of screen */
	PutSprite(11,
		gStatScreen.xDispOff + 64,
		gStatScreen.yDispOff + 131,
		gObject_32x16, TILEREF(0x28F, STATSCREEN_OBJPAL_4) + OAM2_LAYER(3));

    // Display pretiget stars
    DisplayPrestigeStars();

    if (gpKernelDesignerConfig->tellius_skill_capacity_system == true) // (gPlaySt.config.skill_capacity == 0)
    {
        if (gStatScreen.page == 2)
        {
            int usedCapacity = GetUnitBattleAmt(gStatScreen.unit);
            int maxCapacity = gpKernelDesignerConfig->tellius_skill_capacity_base;

            if (UNIT_CATTRIBUTES(gStatScreen.unit) & CA_PROMOTED)
                maxCapacity += gpKernelDesignerConfig->tellius_skill_capacity_promoted;

            // Compute which 1/8th we're in (0–8)
            int bucket = (usedCapacity * 8) / maxCapacity;

            // Safety clamp
            if (bucket < 0)
                bucket = 0;
            else if (bucket > 8)
                bucket = 8;

            if (usedCapacity > 0 && bucket == 0)
                bucket = 1;

            Decompress(capacityGfx[bucket], gGenericBuffer);

            Copy2dChr(gGenericBuffer, (void*)0x6013760, 4, 4);
            PutSprite(4, 164, 120, gObject_32x32, TILEREF(0x1BB, 0x0));
        }
    }

    if (TranslateStatPageId(gStatScreen.page) == PAGE_PROMOTIONS)
    {
        struct Unit *unit = gStatScreen.unit;
        int charId = UNIT_CHAR_ID(unit);
        // Find the unit's promotion data
        const UnitPromotions *promo_data = NULL;
        for (int i = 0; unit_promotions[i].key != 0; i++)
        {
            if (unit_promotions[i].key == charId)
            {
                promo_data = &unit_promotions[i];
                break;
            }
        }
        
        // If we found promotion data for this character, use it
        if (promo_data != NULL)
        {
            int platform_y_positions[] = {41, 82, 122};
            int unit_sprite_y_positions[] = {35, 76, 116};
            
            // Iterate through promotions
            for (int i = 0; i < 3; i++)
            {
                // Check if this promotion exists (classId != 0)
                if (promo_data->promotions[i].classId)
                {
                    // Draw background sprite
                    PutSprite(11, 99, platform_y_positions[i], gObject_32x16, 
                            TILEREF(0x28F, STATSCREEN_OBJPAL_4) + OAM2_LAYER(3));
                    
                    // Draw unit sprite for this class
                    PutUnitSpriteForClassId(0, 108, unit_sprite_y_positions[i], 0xc800, 
                                        promo_data->promotions[i].classId);
                }
            }

            // SyncUnitSpriteSheet();
            RefreshUnitSprites();
        }
    }
}
