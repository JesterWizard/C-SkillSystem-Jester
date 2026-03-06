#include "common-chax.h"
#include "help-box.h"
#include "combat-art.h"
#include "wrank-bonus.h"
#include "savemenu.h"
#include "uichapterstatus.h"
#include "unitlistscreen.h"
#include "skill-system.h"
#include "constants/texts.h"

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

		Proc_Break((void *) proc);
		return;
	}

	if (boxMoved) {
#if CHAX
		sHelpBoxType = NEW_HB_DEFAULT;
#endif

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

	SetTextFontGlyphs(1);
	GetStringTextBox(GetStringFromIndex(proc->mid), &wTextBox, &hTextBox);
	SetTextFontGlyphs(0);

	sub_808A384(proc, wTextBox, hTextBox);
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

    if (gPlaySt.config.skill_capacity == 0) // gpKernelDesignerConfig->tellius_skill_capacity_system == true
    {
        int sid;
        if (TryGetSkillScrollSid(proc->item, &sid))
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
                u8 capacity = GetSkillCapacity(list->sid[i]);
                proc->pretext_lines = 1;
                Text_InsertDrawString(&gHelpBoxSt.text[0], 0, TEXT_COLOR_47CF, "Capacity:");
                Text_InsertDrawNumberOrBlank(&gHelpBoxSt.text[0], 50, TEXT_COLOR_456F, capacity);
                break;
            }
        }
    }

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
    if (gPlaySt.config.skill_capacity == 0) // gpKernelDesignerConfig->tellius_skill_capacity_system == true
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

	if (proc->info->populate)
		proc->info->populate(proc);

	SetTextFontGlyphs(1);
	GetStringTextBox(GetStringFromIndex(proc->mid), &wContent, &hContent);
	SetTextFontGlyphs(0);

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

    if (gpKernelDesignerConfig->vesly_extended_help_boxes == true)
    {
        /* Now we limit it to 5 lines (0x10 * 5) */
        if (h > 0x50) {
            h = 0x50;
        }
    }
    else
    {
        /* Vanilla behaviour to limit the help text box to three lines (0x10 * 3) */
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

    return;
}

//! FE8U = 0x0808A118
LYN_REPLACE_CHECK(ClearHelpBoxText);
void ClearHelpBoxText(void) {

    SetTextFont(&gHelpBoxSt.font);

    SpriteText_DrawBackground(&gHelpBoxSt.text[0]);
    SpriteText_DrawBackground(&gHelpBoxSt.text[1]);
    SpriteText_DrawBackground(&gHelpBoxSt.text[2]);

    /* Do not allocate additional text box space if we're using any of these procs */
    if (gpKernelDesignerConfig->vesly_extended_help_boxes == true)
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

    if (gpKernelDesignerConfig->vesly_extended_help_boxes == true)
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