#include "common-chax.h"
#include "stat-screen.h"
#include "kernel-lib.h"
#include "help-box.h"
#include "skill-system.h"
#include "savemenu.h"
#include "uichapterstatus.h"
#include "unitlistscreen.h"
#include "constants/skills.h"
#include "item-sys.h"
#include "jester_headers/maps.h"
#include "jester_headers/custom-arrays.h"

static const struct StatScreenPageChapterLock sStatScreenPageChapterLocks[] = {
    { PAGE_GAIDEN_MAGIC, CHAPTER_06 },
    { PAGE_PERSONAL_DATA, CHAPTER_08 },
    { PAGE_PROMOTIONS, CHAPTER_10 },
};

static bool IsStatScreenPageEnabledByConfig(int page)
{
    switch (page) {
    case PAGE_GAIDEN_MAGIC:
        return gpKernelDesignerConfig->stat_page_gaiden_magic;

    case PAGE_PERSONAL_DATA:
        return gpKernelDesignerConfig->stat_page_personal_info;

    case PAGE_PROMOTIONS:
        return gpKernelDesignerConfig->stat_page_promotions;

    case PAGE_SKILL_TREE:
        return gpKernelDesignerConfig->stat_page_skill_tree;

    default:
        return true;
    }
}

int GetStatScreenPageUnlockChapter(int page)
{
    for (int i = 0; i < (int)ARRAY_COUNT(sStatScreenPageChapterLocks); i++) {
        if (sStatScreenPageChapterLocks[i].page == page)
            return sStatScreenPageChapterLocks[i].chapterId;
    }

    return -1;
}

bool IsStatScreenPageAvailable(int page)
{
    int unlockChapter;

    if (!IsStatScreenPageEnabledByConfig(page))
        return false;

    unlockChapter = GetStatScreenPageUnlockChapter(page);

    if (unlockChapter >= 0 && gPlaySt.chapterIndex < unlockChapter)
        return false;

    return true;
}

LYN_REPLACE_CHECK(StartStatScreenHelp);
void StartStatScreenHelp(int pageid, struct Proc *proc)
{
	LoadHelpBoxGfx(NULL, -1); // default

	if (!gStatScreen.help) {
		switch (TranslateStatPageId(pageid)) {
		case STATSCREEN_PAGE_0:
			StartUnitScreenHelp(pageid, proc);
			break;

		case STATSCREEN_PAGE_1:
			gStatScreen.help = &gHelpInfo_Ss1Item0;
			break;

		case STATSCREEN_PAGE_2:
			StartSkillScreenHelp(pageid, proc);
			break;

		case 3:
			gStatScreen.help = RTextPageSupport;
			break;
        case PAGE_GAIDEN_MAGIC:
            gStatScreen.help = RTextPageGaidenMagic;
            break;
        case PAGE_PERSONAL_DATA:
            gStatScreen.help = RTextPagePersonalData;
            break;
        case PAGE_PROMOTIONS:
            gStatScreen.help = RTextPagePromotions;
            break;
        case PAGE_SKILL_TREE:
            StartSkillTreeScreenHelp(pageid, proc);
            break;
		} // switch (pageid)
	}
	StartMovingHelpBox(gStatScreen.help, proc);
}

int TranslateStatPageId(int pageid)
{
	int real = pageid;

	if (pageid < 0)
		return STATSCREEN_PAGE_0;

	/* Visible index → physical draw-table index (skip disabled/locked pages). */
	if (!IsStatScreenPageAvailable(PAGE_GAIDEN_MAGIC) && real >= PAGE_GAIDEN_MAGIC)
		real++;

	if (!IsStatScreenPageAvailable(PAGE_PERSONAL_DATA) && real >= PAGE_PERSONAL_DATA)
		real++;

	if (!IsStatScreenPageAvailable(PAGE_PROMOTIONS) && real >= PAGE_PROMOTIONS)
		real++;

	if (!IsStatScreenPageAvailable(PAGE_SKILL_TREE) && real >= PAGE_SKILL_TREE)
		real++;

	return real;
}

LYN_REPLACE_CHECK(DisplayPage);
void DisplayPage(int pageid)
{
    typedef void (*func_type)(void);
    extern const func_type gStatScreenDrawPages[];

    int pageAmt = GetStatPageCount();
    int realPageId;

    if (pageAmt <= 0)
        pageAmt = 1;

    if (pageid < 0)
        pageid = 0;
    else if (pageid >= pageAmt)
        pageid = pageAmt - 1;

    realPageId = TranslateStatPageId(pageid);

    CpuFastFill(0, gUiTmScratchA, sizeof(gUiTmScratchA));
    CpuFastFill(0, gUiTmScratchC, sizeof(gUiTmScratchC));

    if (realPageId < 0 || realPageId > PAGE_SKILL_TREE || gStatScreenDrawPages[realPageId] == NULL)
        return;

    gStatScreenDrawPages[realPageId]();
}

LYN_REPLACE_CHECK(LoadHelpBoxGfx);
void LoadHelpBoxGfx(void * vram, int palId)
{

// Repoint the vram used for the stat screen help box
// (0x6013000 conflicts with skill-capacity circle at 0x6013760)
    if (HelpBoxNeedsSafeVram())
    {
        if (vram == NULL) {
            if (Proc_Find(gProcScr_StatScreen) || Proc_Find(gProcScr_Shop))
                vram = (void *)0x06012000;
            else
                vram = (void *)0x06013000;
        }
    }
    else
    {
        if (vram == NULL) {
            vram = (void *)0x06013000;
        }
    }

    if (palId < 0) {
        palId = 5;
    }

    palId = (palId & 0xF) + 0x10;

    Decompress(gGfx_HelpTextBox, vram + 0x360);
    Decompress(gGfx_HelpTextBox2, vram + 0x760);
    Decompress(gGfx_HelpTextBox3, vram + 0xb60);
    Decompress(gGfx_HelpTextBox4, vram + 0xf60);
    Decompress(gGfx_HelpTextBox5, vram + 0x1360);

    InitSpriteTextFont(&gHelpBoxSt.font, vram, palId);

    InitSpriteText(&gHelpBoxSt.text[0]);
    InitSpriteText(&gHelpBoxSt.text[1]);
    InitSpriteText(&gHelpBoxSt.text[2]);

    /* Don't provide the extra text box tiles if we're using any of the procs in this list */
    if (HelpBoxModeExtended())
    {
        const struct ProcCmd * procExceptionsList[15] = 
        {
            ProcScr_SaveMenu,
            gProcScr_SaveMenuPostChapter,
            gProcScr_ChapterStatusScreen,
            gProcScr_DrawUnitInfoBgSprites,
            ProcScr_bmview,
            ProcScr_UnitListScreen_Field,
            ProcScr_UnitListScreen_PrepMenu,
            ProcScr_UnitListScreen_SoloAnim,
            ProcScr_UnitListScreen_WorldMap,
            ProcScr_PrepUnitScreen,
            ProcScr_PrepItemUseScreen,
            gProcScr_DrawPrepFundsSprite,
            gProcScr_PrepWMShopSell,
            ProcScr_SlidingWallBg,
            ProcScr_PhoenixStaff,
        };

        FORCE_DECLARE bool procFound = false;
        
        for (int i = 0; i < (int)ARRAY_COUNT(procExceptionsList); i++)
        {
            if (Proc_Find(procExceptionsList[i]))
            {
                procFound = true;
                break;
            }
        }

    #if defined(SID_SummonPlus) && (COMMON_SKILL_VALID(SID_SummonPlus))
        if (gActionData.unk08 == SID_SummonPlus && !procFound)
            procFound = true;
    #endif

        if (!procFound)
        {
            InitSpriteText(&gHelpBoxSt.text[3]);
            InitSpriteText(&gHelpBoxSt.text[4]);
        }
    }
    else if (HelpBoxModePaged())
    {
        /* Page indicator uses status-screen digit OBJ tiles (0x289) + gold OBJ pal 10. */
    }

    SetTextFont(0);

    ApplyPalette(Pal_HelpBox, palId);

    gHelpBoxSt.oam2_base = (((u32)vram << 0x11) >> 0x16) + (palId & 0xF) * 0x1000;
}

//! FE8U = 0x0808A00C
LYN_REPLACE_CHECK(HelpBoxIntroDrawTexts);
void HelpBoxIntroDrawTexts(struct ProcHelpBoxIntro * proc)
{
    struct HelpBoxScrollProc * otherProc;
    int textSpeed;
    const char *string;

    SetTextFont(&gHelpBoxSt.font);

    SetTextFontGlyphs(1);

    Text_SetColor(&gHelpBoxSt.text[0], 6);
    Text_SetColor(&gHelpBoxSt.text[1], 6);
    Text_SetColor(&gHelpBoxSt.text[2], 6);

    if (HelpBoxModeExtended())
    {
        Text_SetColor(&gHelpBoxSt.text[3], 6);
        Text_SetColor(&gHelpBoxSt.text[4], 6);
    }

    /*
     * Resolve the description string first so page counts match the same
     * buffer the scroll proc will read (avoids GetStringTextBox skew).
     */
    GetStringFromIndex(proc->msg);
    string = StringInsertSpecialPrefixByCtrl();

    if (HelpBoxModePaged() && sHelpBoxPageState.page == 0)
        sHelpBoxPageState.desc_lines = HelpBoxCountDescLines(string);

    HelpBoxFinalizePageState(proc->pretext_lines);
    HelpBoxDrawPageIndicator();

    /* Indicator uses system glyphs on text[3]; restore talk glyphs for body. */
    SetTextFont(&gHelpBoxSt.font);
    SetTextFontGlyphs(1);
    Text_SetCursor(&gHelpBoxSt.text[0], 0);
    Text_SetCursor(&gHelpBoxSt.text[1], 0);
    Text_SetCursor(&gHelpBoxSt.text[2], 0);
    Text_SetColor(&gHelpBoxSt.text[0], 6);
    Text_SetColor(&gHelpBoxSt.text[1], 6);
    Text_SetColor(&gHelpBoxSt.text[2], 6);

    Proc_EndEach(gProcScr_HelpBoxTextScroll);

    otherProc = Proc_Start(gProcScr_HelpBoxTextScroll, PROC_TREE_3);
    otherProc->font = &gHelpBoxSt.font;

    otherProc->texts[0] = &gHelpBoxSt.text[0];
    otherProc->texts[1] = &gHelpBoxSt.text[1];
    otherProc->texts[2] = &gHelpBoxSt.text[2];

    if (HelpBoxModeExtended())
    {
        otherProc->texts[3] = &gHelpBoxSt.text[3];
        otherProc->texts[4] = &gHelpBoxSt.text[4];
    }

    otherProc->pretext_lines = proc->pretext_lines;

    if (HelpBoxModePaged())
        string = HelpBoxSkipDescLines(string, HelpBoxDescLinesToSkip());

    otherProc->string = string;
    otherProc->chars_per_step = 1;
    otherProc->step = 0;

    /* Stash the first description text-slot index in unk_64 for the scroll cap. */
    otherProc->unk_64 = proc->pretext_lines;

    textSpeed = gPlaySt.config.textSpeed;
    switch (gPlaySt.config.textSpeed) {
    case 0: /* default speed */
        otherProc->speed = 2;
        break;

    case 1: /* slow */
        otherProc->speed = textSpeed;
        break;

    case 2: /* fast */
        otherProc->speed = 1;
        otherProc->chars_per_step = textSpeed;
        break;

    case 3: /* draw all at once */
        otherProc->speed = 0;
        otherProc->chars_per_step = 0x7f;
        break;
    }

    SetTextFont(0);
}

//! FE8U = 0x080898C4
LYN_REPLACE_CHECK(sub_80898C4);
void sub_80898C4(void* vram, int palId) {

// Repoint the vram used for the stat screen help box
    if (HelpBoxNeedsSafeVram())
    {
        if (vram == NULL) {
            vram = (void *)0x06012000;
        }
    }
    else
    {
        if (vram == NULL) {
            vram = (void *)0x06013000;
        }
    }

    if (palId < 0) {
        palId = 5;
    }

    palId = (palId & 0xf) + 0x10;

    Decompress(gGfx_HelpTextBox, vram + 0x360);
    Decompress(gGfx_HelpTextBox2, vram + 0x760);
    Decompress(gGfx_HelpTextBox3, vram + 0xb60);
    Decompress(gGfx_HelpTextBox4, vram + 0xf60);
    Decompress(gGfx_HelpTextBox5, vram + 0x1360);

    InitSpriteTextFont(&gHelpBoxSt.font, vram, palId);

    InitSpriteText(&gHelpBoxSt.text[0]);
    InitSpriteText(&gHelpBoxSt.text[1]);
    
    if (HelpBoxModeExtended())
    {
        InitSpriteText(&gHelpBoxSt.text[2]);
        InitSpriteText(&gHelpBoxSt.text[3]);
        InitSpriteText(&gHelpBoxSt.text[4]);
    }

    gHelpBoxSt.text[2].tile_width = 0;

    SetTextFont(0);

    ApplyPalette(Pal_HelpBox, palId);

    gHelpBoxSt.oam2_base = (((u32)vram << 0x11) >> 0x16) + (palId & 0xF) * 0x1000;

    return;
}
