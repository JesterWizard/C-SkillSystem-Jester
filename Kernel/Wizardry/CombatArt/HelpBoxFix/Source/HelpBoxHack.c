#include "common-chax.h"
#include "help-box.h"
#include "combat-art.h"
#include "wrank-bonus.h"
#include "savemenu.h"
#include "uichapterstatus.h"
#include "unitlistscreen.h"
#include "skill-system.h"
#include "constants/texts.h"
#include "scene.h"
#include "statscreen.h"

void HelpBoxResetPageState(void)
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

void HelpBoxStoreDescLineCount(int descHeightPx)
{
	int lines = descHeightPx / 0x10;

	if (lines < 0)
		lines = 0;

	sHelpBoxPageState.desc_lines = lines;
}

int HelpBoxCountDescLines(const char *str)
{
	int lines = 0;

	if (!str || *str == CHFE_L_X)
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
	int lpp = 3 - pretext;

	if (lpp < 1)
		lpp = 1;

	return lpp;
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

void HelpBoxFinalizePageState(int pretext_lines)
{
	int lpp;

	if (pretext_lines < 0)
		pretext_lines = 0;
	if (pretext_lines > 2)
		pretext_lines = 2;

	lpp = HelpBoxLinesPerPageForPretext(pretext_lines);

	sHelpBoxPageState.pretext_lines = pretext_lines;
	sHelpBoxPageState.lines_per_page = lpp;

	if (!HelpBoxModePaged()) {
		sHelpBoxPageState.page_count = 1;
		return;
	}

	if (sHelpBoxPageState.page == 0) {
		sHelpBoxPageState.page0_desc_lpp = lpp;
		if (sHelpBoxPageState.page0_desc_lpp > sHelpBoxPageState.desc_lines)
			sHelpBoxPageState.page0_desc_lpp = sHelpBoxPageState.desc_lines;
		/* later_desc_lpp is set in HelpBoxSetupstringLines (excludes capacity). */
		sHelpBoxPageState.page_count = HelpBoxCalcPageCount();
	}

	if (sHelpBoxPageState.page_count < 1)
		sHelpBoxPageState.page_count = 1;
	if (sHelpBoxPageState.page >= sHelpBoxPageState.page_count)
		sHelpBoxPageState.page = 0;
}

int HelpBoxDescLinesToSkip(void)
{
	int page = sHelpBoxPageState.page;

	if (!HelpBoxModePaged() || page == 0)
		return 0;

	return sHelpBoxPageState.page0_desc_lpp +
		(page - 1) * sHelpBoxPageState.later_desc_lpp;
}

const char *HelpBoxSkipDescLines(const char *str, int linesToSkip)
{
	if (!str)
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

/* Unused on the status screen while help is open; pal 4 stays white for "3/4". */
enum { HELP_BOX_PAGE_NUM_OBJPAL = 10 };

/*
 * Same layout as item_icon_palette[1] (LoadIconPalette(1, 0x14) → white "3/4"),
 * but index 1 (the digit fill — white 0x7FDE there) is Capacity gold instead.
 */
static const u16 sHelpBoxPageNumPal[16] = {
	0x67F8,
	0x47DF, /* was 0x7FDE white → gold */
	0x6F37,
	0x4E30,
	0x212A,
	0x3BBE,
	0x1656,
	0x19B1,
	0x11FF,
	0x7DE2,
	0x7715,
	0x6238,
	0x0AE3,
	0x4BEF,
	0x594A,
	0x0000,
};

void HelpBoxDrawPageIndicator(void)
{
	if (!HelpBoxModePaged() || sHelpBoxPageState.page_count <= 1) {
		sHelpBoxPageState.indicator_px = 0;
		return;
	}

	/* Install gold remapping for the shared 0x289 digit sheet (does not touch pal 4). */
	ApplyPalette(sHelpBoxPageNumPal, HELP_BOX_PAGE_NUM_OBJPAL + 0x10);
	sHelpBoxPageState.indicator_px = 22;
}

void HelpBoxPutPageIndicatorSprites(int boxX, int boxY, int boxW)
{
	int x;
	int oam2;
	int cur;
	int tot;

	if (!HelpBoxModePaged() || sHelpBoxPageState.page_count <= 1)
		return;

	cur = sHelpBoxPageState.page; /* 0-based; tile 0x289 + 0 => "1" */
	tot = sHelpBoxPageState.page_count;

	/* Right side, same Y as the "Help" badge. */
	x = boxX + boxW - 24;
	if (x < boxX + 48)
		x = boxX + 48;

	oam2 = TILEREF(0x289, HELP_BOX_PAGE_NUM_OBJPAL);

	PutSprite(0, x, boxY - 0xB, gObject_8x8, oam2 + cur);
	PutSprite(0, x + 7, boxY - 0xB, gObject_8x8, oam2 - 1);
	PutSprite(0, x + 14, boxY - 0xB, gObject_8x8, oam2 + (tot - 1));
}

LYN_REPLACE_CHECK(HbMoveCtrl_OnIdle);
void HbMoveCtrl_OnIdle(struct HelpBoxProc *proc)
{
	u8 boxMoved = FALSE;

	DisplayUiHand(
		sHbOrigin.x*8 + proc->info->xDisplay,
		sHbOrigin.y*8 + proc->info->yDisplay);

	if (gKeyStatusPtr->repeatedKeys & DPAD_UP)
		boxMoved |= TryRelocateHbUp(proc);

	if (gKeyStatusPtr->repeatedKeys & DPAD_DOWN)
		boxMoved |= TryRelocateHbDown(proc);

	if (gKeyStatusPtr->repeatedKeys & DPAD_LEFT)
		boxMoved |= TryRelocateHbLeft(proc);

	if (gKeyStatusPtr->repeatedKeys & DPAD_RIGHT)
		boxMoved |= TryRelocateHbRight(proc);

	if (gKeyStatusPtr->newKeys & (B_BUTTON | R_BUTTON)) {
#if CHAX
		sHelpBoxType = NEW_HB_DEFAULT;
#endif
		HelpBoxResetPageState();

		Proc_Break((void *) proc);
		return;
	}

	if (HelpBoxModePaged() &&
		sHelpBoxPageState.page_count > 1 &&
		(gKeyStatusPtr->newKeys & A_BUTTON)) {
		struct HelpBoxProc *hb = Proc_Find(gProcScr_HelpBox);
		int item;
		int mid;

		/*
		 * HbMoveCtrl's own item/mid are never filled — use the live help-box
		 * proc (or info->mid as fallback). Wrong mid was leaving page 2 blank
		 * until the box was closed and reopened.
		 */
		if (!hb)
			hb = Proc_Find(ProcScr_Helpbox_bug_08A01678);

		if (hb) {
			item = hb->item;
			mid = hb->mid;
		} else if (proc->info) {
			item = 0;
			mid = proc->info->mid;
		} else {
			return;
		}

		sHelpBoxPageState.page++;
		if (sHelpBoxPageState.page >= sHelpBoxPageState.page_count)
			sHelpBoxPageState.page = 0;

		PlaySoundEffect(0x67);
		ClearHelpBoxText();
		StartHelpBoxTextInit(item, mid);
		return;
	}

	if (boxMoved) {
#if CHAX
		sHelpBoxType = NEW_HB_DEFAULT;
#endif
		HelpBoxResetPageState();

		PlaySoundEffect(0x67);
		Proc_Goto((void *) proc, 0); // TODO: label constants?
	}
}

STATIC_DECLAR void sub_808A200_vanilla(const struct HelpBoxInfo *info)
{
	int wTextBox;
	int hTextBox;

	struct HelpBoxProc *proc = Proc_Find(ProcScr_Helpbox_bug_08A01678);

	if (!proc) {
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

	SetTextFontGlyphs(1);
	GetStringTextBox(GetStringFromIndex(proc->mid), &wTextBox, &hTextBox);
	SetTextFontGlyphs(0);

	HelpBoxStoreDescLineCount(hTextBox);

	if (HelpBoxModePaged()) {
		/* Cap description height to one page; pretext added later by size helpers. */
		if (hTextBox > 0x30)
			hTextBox = 0x30;
	}

	sub_808A384(proc, wTextBox, hTextBox);

	if (HelpBoxModePaged() && proc->hBoxFinal > 0x30)
		proc->hBoxFinal = 0x30;

	sub_808A3C4(proc, info->xDisplay, info->yDisplay);

	ClearHelpBoxText();
	StartHelpBoxTextInit(proc->item, proc->mid);

	gpHelpBoxCurrentInfo = info;
}

LYN_REPLACE_CHECK(sub_808A200);
void sub_808A200(const struct HelpBoxInfo *info)
{
	sHelpBoxType = 0;
	sub_808A200_vanilla(info);
}

bool TryGetSkillScrollSid(int item, int *outSid)
{
    static const struct {
        int itemIndex;
        int sidOffset;
    } skillScrollMap[] = {
    #ifdef CONFIG_ITEM_INDEX_SKILL_SCROLL_1
        { CONFIG_ITEM_INDEX_SKILL_SCROLL_1, 0x000 },
    #endif
    #ifdef CONFIG_ITEM_INDEX_SKILL_SCROLL_2
        { CONFIG_ITEM_INDEX_SKILL_SCROLL_2, 0x0FF },
    #endif
    #ifdef CONFIG_ITEM_INDEX_SKILL_SCROLL_3
        { CONFIG_ITEM_INDEX_SKILL_SCROLL_3, 0x1FF },
    #endif
    #ifdef CONFIG_ITEM_INDEX_SKILL_SCROLL_4
        { CONFIG_ITEM_INDEX_SKILL_SCROLL_4, 0x2FF },
    #endif
    };

    for (unsigned i = 0; i < ARRAY_COUNT(skillScrollMap); i++)
    {
        if (ITEM_INDEX(item) == skillScrollMap[i].itemIndex)
        {
            *outSid = ITEM_USES(item) + skillScrollMap[i].sidOffset;
            return true;
        }
    }

    return false;
}

LYN_REPLACE_CHECK(HelpBoxSetupstringLines);
void HelpBoxSetupstringLines(struct ProcHelpBoxIntro *proc)
{
    FORCE_DECLARE u8 capacity = 0;
    FORCE_DECLARE bool skillScrollItem = false;
    FORCE_DECLARE int sid;
	int base_pretext;

	SetTextFont(&gHelpBoxSt.font);
	SetTextFontGlyphs(0);

	if (sHelpBoxType == 0) {
		/* Vanilla */
		switch (GetHelpBoxItemInfoKind(proc->item)) {
		case HB_EXTINFO_NONE:
			proc->pretext_lines = 0;
			break;

		case HB_EXTINFO_WEAPON:
			DrawHelpBoxWeaponLabels(proc->item);
			proc->pretext_lines = 2;
			break;

		case HB_EXTINFO_STAFF:
			DrawHelpBoxStaffLabels(proc->item);
			proc->pretext_lines = 1;
			break;

		case HB_EXTINFO_SAVEINFO:
			DrawHelpBoxSaveMenuLabels();
			proc->pretext_lines = 1;
			break;
		}
	} else {
		/* Hack here */
		switch (sHelpBoxType) {
		case NEW_HB_COMBAT_ART_BKSEL:
			if (!GetCombatArtInfo(proc->item)->battle_status.display_en_n) {
				DrawHelpBoxCombatArtBkselLabels();
				proc->pretext_lines = 2;
			} else {
				proc->pretext_lines = 0;
			}
			break;

		case NEW_HB_WRANK_STATSCREEN:
			DrawHelpBoxLabels_WrankBonus();
			proc->pretext_lines = 3;
    
            if (gpKernelDesignerConfig->quality_of_life_fixes == true)
                proc->pretext_lines = 2;

			break;

		default:
			break;
		}
	}

	base_pretext = proc->pretext_lines;

    if (gpKernelDesignerConfig->tellius_skill_capacity_system == true)
    {
		bool drawCapacity = !HelpBoxModePaged() || sHelpBoxPageState.page == 0;

        int sid;
        if (drawCapacity && TryGetSkillScrollSid(proc->item, &sid))
        {
            u8 capacity = GetSkillCapacity(sid);
            proc->pretext_lines = 1;
            Text_InsertDrawString(&gHelpBoxSt.text[0], 0, TEXT_COLOR_47CF, "Capacity:");
            Text_InsertDrawNumberOrBlank(&gHelpBoxSt.text[0], 50, TEXT_COLOR_456F, capacity);
        }

        struct SkillList * list = GetUnitSkillList(gStatScreen.unit);
        for (int i = 0; i < list->amt; i++)
        {
            if (GetSkillDescMsg(list->sid[i]) == proc->msg)
            {
				if (drawCapacity) {
					u8 capacity = GetSkillCapacity(list->sid[i]);
					proc->pretext_lines = 1;
					Text_InsertDrawString(&gHelpBoxSt.text[0], 0, TEXT_COLOR_47CF, "Capacity:");
					Text_InsertDrawNumberOrBlank(&gHelpBoxSt.text[0], 50, TEXT_COLOR_456F, capacity);
				}
                break;
            }
        }
    }

	/* later pages omit capacity but keep weapon/staff headers (base_pretext). */
	if (HelpBoxModePaged() && sHelpBoxPageState.page == 0)
		sHelpBoxPageState.later_desc_lpp = HelpBoxLinesPerPageForPretext(base_pretext);

	SetTextFont(0);
	Proc_Break(proc);
}

LYN_REPLACE_CHECK(HelpBoxDrawstring);
void HelpBoxDrawstring(struct ProcHelpBoxIntro *proc)
{
	int item = proc->item;

	SetTextFont(&gHelpBoxSt.font);

	if (sHelpBoxType == 0) {
		/* Vanilla */
		switch (GetHelpBoxItemInfoKind(item)) {
		case HB_EXTINFO_WEAPON:
			DrawHelpBoxWeaponStats(item);
			break;

		case HB_EXTINFO_SAVEINFO:
			DrawHelpBoxSaveMenuStats();
			break;
		}
	} else {
		/* Hack here */
		switch (sHelpBoxType) {
		case NEW_HB_COMBAT_ART_BKSEL:
			if (!GetCombatArtInfo(proc->item)->battle_status.display_en_n)
				DrawHelpBoxCombatArtBkselStats(proc);

			break;

		case NEW_HB_WRANK_STATSCREEN:
			DrawHelpBoxStats_WrankBonus(proc);
			break;

		default:
			break;
		}
	}

	SetTextFont(0);
	Proc_Break(proc);
}

LYN_REPLACE_CHECK(sub_808A454);
int sub_808A454(int item)
{
	if (sHelpBoxType == 0) {
		/* Vanilla */
		if (item == (u16)-2)
			return 3;

		if (GetItemAttributes(item) & IA_LOCK_3)
			return 0;

		if (GetItemAttributes(item) & IA_WEAPON)
			return 1;

		if (GetItemAttributes(item) & IA_STAFF)
			return 2;
	} else {
		/* Hack here */
		switch (sHelpBoxType) {
		case NEW_HB_COMBAT_ART_BKSEL:
			if (!GetCombatArtInfo(item)->battle_status.display_en_n)
				return 2;

			break;

		case NEW_HB_WRANK_STATSCREEN:
			return 2;

		default:
			break;
		}
	}

	return 0;
}

LYN_REPLACE_CHECK(ApplyHelpBoxContentSize);
void ApplyHelpBoxContentSize(struct HelpBoxProc *proc, int width, int height)
{
	#define AUTO_ADJUST_SIZE \
	do { \
		if (width < 0x90) \
			width = 0x90; \
		if (GetStringTextLen(GetStringFromIndex(proc->mid)) > 8) \
			height += 0x20; \
		else \
			height += 0x10; \
	} while (0)

	width = 0xF0 & (width + 15); // align to 16 pixel multiple

	if (sHelpBoxType == 0) {
		/* Vanilla */
		switch (GetHelpBoxItemInfoKind(proc->item)) {
		case 1: // weapon
			AUTO_ADJUST_SIZE;
			break;

		case 2: // staff
			if (width < 0x60)
				width = 0x60;

			height += 0x10;
			break;

		case 3: // save stuff
			width = 0x80;
			height += 0x10;
			break;
		}
	} else {
		/* Hack here */
		switch (sHelpBoxType) {
		case NEW_HB_COMBAT_ART_BKSEL:
			if (!GetCombatArtInfo(proc->item)->battle_status.display_en_n)
				AUTO_ADJUST_SIZE;

			break;

		case NEW_HB_WRANK_STATSCREEN:
			AUTO_ADJUST_SIZE;
			break;

		default:
			break;
		}
	}

    // Add an extra line of height to the texbox to account for the capacity text
    if (gpKernelDesignerConfig->tellius_skill_capacity_system == true)
    {
        int sid;
        if (TryGetSkillScrollSid(proc->item, &sid))
            height += 0x10;

        struct SkillList * list = GetUnitSkillList(gStatScreen.unit);

        for (int i = 0; i < list->amt; i++)
        {
            if (GetSkillDescMsg(list->sid[i]) == proc->mid)
            {
                height += 0x10;
                break;
            }
        }
    }

	if (HelpBoxModePaged() && height > 0x30)
		height = 0x30;

	proc->wBoxFinal = width;
	proc->hBoxFinal = height;

	#undef AUTO_ADJUST_SIZE
}

LYN_REPLACE_CHECK(StartHelpBoxExt);
void StartHelpBoxExt(const struct HelpBoxInfo *info, int unk)
{
	struct HelpBoxProc *proc;
	int wContent, hContent;

	proc = (void *) Proc_Find(gProcScr_HelpBox);

	if (!proc) {
		proc = (void *) Proc_Start(gProcScr_HelpBox, PROC_TREE_3);

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

	proc->timer	= 0;
	proc->timerMax = 12;

	proc->item = 0;
	proc->mid = info->mid;

#if CHAX
	sHelpBoxType = NEW_HB_DEFAULT;
#endif

	HelpBoxResetPageState();

	if (proc->info->populate)
		proc->info->populate(proc);

	SetTextFontGlyphs(1);
	GetStringTextBox(GetStringFromIndex(proc->mid), &wContent, &hContent);
	SetTextFontGlyphs(0);

	HelpBoxStoreDescLineCount(hContent);

	if (HelpBoxModePaged() && hContent > 0x30)
		hContent = 0x30;

	ApplyHelpBoxContentSize(proc, wContent, hContent);
	ApplyHelpBoxPosition(proc, info->xDisplay, info->yDisplay);

	ClearHelpBoxText();
	StartHelpBoxTextInit(proc->item, proc->mid);

	sLastHbi = info;
}

//! FE8U = 0x08089980
LYN_REPLACE_CHECK(DisplayHelpBoxObj);
void DisplayHelpBoxObj(int x, int y, int w, int h, int unk) {
    s8 flag;
    s8 flag_;
    s8 anotherFlag;

    int xCount;
    int yCount;

    int xPx;
    int yPx;
    int iy;
    int ix;

    flag = (w + 7) & 0x10;
    anotherFlag = w & 0xf;

    if (w < 0x20) {
        w = 0x20;
    }

    if (w > 0xC0) {
        w = 0xc0;
    }

    if (h < 0x10) {
        h = 0x10;
    }

    if (HelpBoxModeExtended())
    {
        /* Now we limit it to 5 lines (0x10 * 5) */
        if (h > 0x50) {
            h = 0x50;
        }
    }
    else
    {
        /* Vanilla / paginated: limit the help text box to three lines (0x10 * 3) */
        if (h > 0x30) {
            h = 0x30;
        }
    }

    xCount = (w + 0x1f) / 0x20;
    yCount = (h + 0x0f) / 0x10;

    flag_ = flag;

    for (ix = xCount - 1; ix >= 0; ix--) {
        for (iy = yCount; iy >= 0; iy--) {

            yPx = (iy + 1) * 0x10;
            if (yPx > h) {
                yPx = h;
            }
            yPx -= 0x10;

            xPx = (ix + 1) * 0x20;

            if (flag_ != 0) {
                xPx -= 0x20;
                PutSprite(0,
                x + xPx,
                y + yPx,
                gObject_16x16,
                gHelpBoxSt.oam2_base + ix * 4 + iy * 0x40);
            } else {

                if (xPx > w)
                    xPx = w;

                xPx -= 0x20;
                PutSprite(
                    0,
                    x + xPx,
                    y + yPx,
                    gObject_32x16,
                    gHelpBoxSt.oam2_base + ix * 4 + iy * 0x40);
            }
        }

        flag_ = 0;
    }

    flag_ = flag;

    for (ix = xCount - 1; ix >= 0; ix--) {
        xPx = (ix + 1) * 0x20;

        if (flag_ != 0) {
            xPx -= 0x20;

            PutSprite(0, x + xPx, y - 8, gObject_16x8, gHelpBoxSt.oam2_base + 0x1b);
            PutSprite(0, x + xPx, y + h, gObject_16x8, gHelpBoxSt.oam2_base + 0x3b);

            flag_ = 0;
        } else {
            if (xPx > w) {
                xPx = w;
            }
            xPx -= 0x20;

            PutSprite(0, x + xPx, y - 8, gObject_32x8, gHelpBoxSt.oam2_base + 0x1b);
            PutSprite(0, x + xPx, y + h, gObject_32x8, gHelpBoxSt.oam2_base + 0x3b);

        }

    }

    for (iy = yCount; iy >= 0; iy--) {
        yPx = (iy + 1) * 0x10;
        if (yPx > h) {
            yPx = h;
        }
        yPx -= 0x10;

        PutSprite(0, x - 8, y + yPx, gObject_8x16, gHelpBoxSt.oam2_base + 0x5f);
        PutSprite(0, x + w, y + yPx, gObject_8x16, gHelpBoxSt.oam2_base + 0x1f);

        if (anotherFlag != 0) {
            PutSprite(0, x + w - 8, y + yPx, gObject_8x16, gHelpBoxSt.oam2_base + 0x1a);
        }
    }

    PutSprite(0, x - 8, y - 8, gObject_8x8, gHelpBoxSt.oam2_base + 0x5b); // top left
    PutSprite(0, x + w, y - 8, gObject_8x8, gHelpBoxSt.oam2_base + 0x5c); // top right
    PutSprite(0, x - 8, y + h, gObject_8x8, gHelpBoxSt.oam2_base + 0x5d); // bottom left
    PutSprite(0, x + w, y + h, gObject_8x8, gHelpBoxSt.oam2_base + 0x5e); // bottom right

    if (anotherFlag != 0) {
        PutSprite(0, x + w - 8, y - 8, gObject_8x8, gHelpBoxSt.oam2_base + 0x1b);
        PutSprite(0, x + w - 8, y + h, gObject_8x8, gHelpBoxSt.oam2_base + 0x3b);
    }

    if (unk == 0) {
        PutSprite(0, x, y - 0xb, gObject_32x16, (0x3FF & gHelpBoxSt.oam2_base) + 0x7b);
    }

    HelpBoxPutPageIndicatorSprites(x, y, w);

    return;
}

//! FE8U = 0x08089E58
LYN_REPLACE_CHECK(HelpBoxTextScroll_OnLoop);
void HelpBoxTextScroll_OnLoop(struct HelpBoxScrollProc * proc)
{
    int i;
    int maxLine;
    int textLimit;

    proc->step--;

    if (proc->step > 0)
        return;

    proc->step = proc->speed;

    SetTextFont(proc->font);
    SetTextFontGlyphs(1); /* body R-text uses talk glyphs (black, color 6) */

    textLimit = HelpBoxModeExtended() ? 5 : 3;
    maxLine = textLimit;

    if (HelpBoxModePaged()) {
        maxLine = proc->unk_64 + sHelpBoxPageState.lines_per_page;
        if (maxLine > 3)
            maxLine = 3;
    }

    for (i = 0; i < proc->chars_per_step; i++) {
        switch (*proc->string) {
        case CHFE_L_X:
            Proc_Break(proc);
            goto helpbox_scroll_end;

        case CHFE_L_NL:
            proc->string++;
            proc->pretext_lines++;

            if (proc->pretext_lines >= maxLine) {
                Proc_Break(proc);
                goto helpbox_scroll_end;
            }
            continue;

        case CHFE_L_Pause8:
            proc->string++;
            continue;

        default:
            if (proc->pretext_lines < 0 ||
                proc->pretext_lines >= textLimit ||
                proc->texts[proc->pretext_lines] == NULL) {
                Proc_Break(proc);
                goto helpbox_scroll_end;
            }
            proc->string = Text_DrawCharacter(proc->texts[proc->pretext_lines], proc->string);
            continue;
        }
    }

helpbox_scroll_end:
    SetTextFont(0);
}

//! FE8U = 0x0808A118
LYN_REPLACE_CHECK(ClearHelpBoxText);
void ClearHelpBoxText(void) {

    SetTextFont(&gHelpBoxSt.font);

    SpriteText_DrawBackground(&gHelpBoxSt.text[0]);
    SpriteText_DrawBackground(&gHelpBoxSt.text[1]);
    SpriteText_DrawBackground(&gHelpBoxSt.text[2]);

    /* Do not allocate additional text box space if we're using any of these procs */
    if (HelpBoxModeExtended())
    {
        const struct ProcCmd * procExceptionsList[9] = 
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
            // PrepScreenProc_MapIdle,
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

        if (!procFound)
        {
            SpriteText_DrawBackground(&gHelpBoxSt.text[3]);
            SpriteText_DrawBackground(&gHelpBoxSt.text[4]);
        }
    }

    Proc_EndEach(gProcScr_HelpBoxTextScroll);
    Proc_EndEach(ProcScr_HelpBoxIntro);

    SetTextFont(0);

    return;
}

//! FE8U = 0x08089CD4
LYN_REPLACE_CHECK(DrawHelpBoxWeaponStats);
void DrawHelpBoxWeaponStats(int item)
{
    Text_InsertDrawString(&gHelpBoxSt.text[0], 32, 7, GetItemDisplayRankString(item));
    Text_InsertDrawString(&gHelpBoxSt.text[0], 67, 7, GetItemDisplayRangeString(item));
    Text_InsertDrawNumberOrBlank(&gHelpBoxSt.text[0], 129, 7, GetItemWeight(item));

    Text_InsertDrawNumberOrBlank(&gHelpBoxSt.text[1], 32, 7, GetItemMight(item));
    Text_InsertDrawNumberOrBlank(&gHelpBoxSt.text[1], 81, 7, GetItemHit(item));
    Text_InsertDrawNumberOrBlank(&gHelpBoxSt.text[1], 129, 7, GetItemCrit(item));
}

LYN_REPLACE_CHECK(GetHelpBoxItemInfoKind);
int GetHelpBoxItemInfoKind(int item)
{
    if (item == 0xFFFE)
        return HB_EXTINFO_SAVEINFO;

    if (GetItemAttributes(item) & IA_LOCK_3)
        return HB_EXTINFO_NONE;

    if (GetItemAttributes(item) & IA_WEAPON)
        return HB_EXTINFO_WEAPON;

    if (GetItemAttributes(item) & IA_STAFF)
        return HB_EXTINFO_STAFF;

    return HB_EXTINFO_NONE;
}

//! FE8U = 0x0808A5D0
LYN_REPLACE_CHECK(InitBoxDialogue);
void InitBoxDialogue(void * vram_dst, int pad_idx) {
    FORCE_DECLARE int uVar1;
    FORCE_DECLARE int iVar3;
    FORCE_DECLARE int iVar4;
    FORCE_DECLARE int iVar5;

    if (HelpBoxNeedsSafeVram())
    {
        if (vram_dst == 0) {
            vram_dst = (void *)0x06012000;
        }
    }
    else
    {
        if (vram_dst == 0) {
            vram_dst = (void *)0x06013000;
        }
    }
    
    if (pad_idx < 0) {
        pad_idx = 5;
    }

    pad_idx = (pad_idx & 0xf) + 0x10;

    if (GetDialogueBoxConfig() & 0x10) {
        vram_dst = (void *)0x06016800;
        Decompress(gGfx_YellowTextBox, vram_dst + 0x360);
        Decompress(gGfx_YellowTextBox2, vram_dst + 0x760);
        Decompress(gGfx_YellowTextBox3, vram_dst + 0xb60);
        Decompress(gGfx_YellowTextBox4, vram_dst + 0xf80);
        Decompress(gGfx_YellowTextBox5, vram_dst + 0x1380);
    } else {
        Decompress(gGfx_HelpTextBox, vram_dst + 0x360);
        Decompress(gGfx_HelpTextBox2, vram_dst + 0x760);
        Decompress(gGfx_HelpTextBox3, vram_dst + 0xb60);
        Decompress(gGfx_HelpTextBox4, vram_dst + 0xf60);
        Decompress(gGfx_HelpTextBox5, vram_dst + 0x1360);
    }

    ClearAllTalkFlags();

    if (!(GetDialogueBoxConfig() & 1)) {
        InitSpriteTextFont(&gBoxDialogueConf.font, vram_dst, pad_idx);

        InitSpriteText(&gBoxDialogueConf.texts[0]);
        InitSpriteText(&gBoxDialogueConf.texts[1]);
        InitSpriteText(&gBoxDialogueConf.texts[2]);

        if ((GetDialogueBoxConfig() & 0x10) && !(GetDialogueBoxConfig() & 0x20)) {
            InitSpriteText(&gBoxDialogueConf.texts[3]);
            InitSpriteText(&gBoxDialogueConf.texts[4]);
        }

        SetTextFont(0);

        if (GetDialogueBoxConfig() & 0x10) {
            ApplyPalette(gPal_YellowTextBox, pad_idx);
        } else {
            ApplyPalette(gPal_HelpTextBox, pad_idx);
        }

    } else {
        InitSpriteTextFont(&gBoxDialogueConf.font, vram_dst, pad_idx);

        for (iVar4 = 0; iVar4 < ((u16)GetDialogueBoxConfig() >> 8); iVar4++) {
            InitSpriteText(&gBoxDialogueConf.texts[iVar4]);
        }

        SetTextFont(0);

        ApplyPalette(Pal_Text, pad_idx);
    }

    // ORIGINAL  -> if (&vram_dst)
    if (vram_dst)
        gBoxDialogueConf.unk_40 = ((((u32)vram_dst << 0x11) >> 0x16) + (pad_idx & 0xF) * 0x1000);

    if (GetDialogueBoxConfig() & 0x10) {
        PlaySoundEffect(SONG_2E6);
    }

    return;
}