#include "common-chax.h"
#include "kernel-lib.h"
#include "lvup.h"

/**
 * Class growth preview on the promotion screen.
 *
 * While class growths are on, the class description box shows how the
 * highlighted class would change the unit's growths. R swaps the box between
 * the growths and the vanilla class description.
 */

/* Kept in ProcPromoSel::_u3e, which vanilla leaves unused */
enum promo_sel_info_mode {
	PROMO_SEL_INFO_GROWTHS,
	PROMO_SEL_INFO_DESC,
};

extern char sClassGrowthPreviewText[];
extern CONST_DATA struct MenuItemDef gMenuItem_PromoSel[];

int ClassChgMenuItem_OnChange(struct MenuProc *pmenu, struct MenuItemProc *pmitem);
void sub_80CCF60(struct ProcPromoSel *proc);

static char *AppendText(char *dst, const char *src)
{
	while (*src)
		*dst++ = *src++;

	return dst;
}

static char *AppendGrowthDiff(char *dst, int diff)
{
	int hundreds = 0, tens = 0;

	*dst++ = diff < 0 ? '-' : '+';
	diff = ABS(diff);

	while (diff >= 100) {
		diff -= 100;
		hundreds++;
	}

	while (diff >= 10) {
		diff -= 10;
		tens++;
	}

	if (hundreds)
		*dst++ = '0' + hundreds;

	if (hundreds || tens)
		*dst++ = '0' + tens;

	*dst++ = '0' + diff;
	*dst++ = '%';
	return dst;
}

/* Two lines of "Str +10%" entries, four stats per line */
static void BuildClassGrowthPreviewText(struct Unit *unit, int jid)
{
	int i, diffs[CLASS_GROWTH_MAX];
	char *str = sClassGrowthPreviewText;

	GetClassChangeGrowthDiffs(unit, jid, diffs);

	for (i = 0; i < CLASS_GROWTH_MAX; i++) {
		if (i == CLASS_GROWTH_MAX / 2)
			*str++ = 0x01; /* [NL] */
		else if (i != 0)
			str = AppendText(str, "  ");

		str = AppendText(str, gClassGrowthStatNames[i]);
		*str++ = ' ';
		str = AppendGrowthDiff(str, diffs[i]);
	}

	*str = '\0';
}

static void ChangeClassGrowthPreview(struct Unit *unit, int jid)
{
	BuildClassGrowthPreviewText(unit, jid);

	/* Same box setup as ChangeClassDescription */
	SetInitTalkTextFont();
	ClearTalkText();
	StartTalk(2, 0xF, sClassGrowthPreviewText);
	SetTalkPrintColor(0);
	SetTalkFlag(TALK_FLAG_INSTANTSHIFT);
	SetTalkFlag(TALK_FLAG_NOBUBBLE);
	SetTalkFlag(TALK_FLAG_NOSKIP);
	SetTalkFlag(TALK_FLAG_NOFAST);
	SetTalkFlag(TALK_FLAG_SILENT);
	SetTalkPrintDelay(4);
}

static void PromoSel_DrawClassInfo(struct ProcPromoSel *proc)
{
	if (ClassGrowthPreviewEnabled() && proc->_u3e == PROMO_SEL_INFO_GROWTHS)
		ChangeClassGrowthPreview(GetUnitFromCharId(proc->pid), proc->jid[proc->main_select]);
	else
		ChangeClassDescription(proc->msg_desc[proc->main_select]);
}

/**
 * Starting a talk makes the text engine load Pal_Text into OBJ palette 0xA
 * (its float glyph palette) a frame later, and the battle anim uses that
 * palette too. A cursor move reloads the anim afterwards; R does not, so
 * this proc puts the anim's colours back once the text engine has written.
 */
#define PROMO_SEL_TEXT_OBJ_PAL 0xA

struct ProcAnimPalGuard {
	PROC_HEADER;
	int timer;
	u16 pal[0x10];
};

static u16 *AnimPalGuardBank(void)
{
	return gPaletteBuffer + 0x100 + 0x10 * PROMO_SEL_TEXT_OBJ_PAL;
}

static void AnimPalGuard_Loop(struct ProcAnimPalGuard *proc)
{
	u16 *bank = AnimPalGuardBank();
	int i;

	for (i = 0; i < 0x10; i++)
		if (bank[i] != Pal_Text[i])
			break;

	if (i == 0x10) {
		CpuCopy16(proc->pal, bank, sizeof(proc->pal));
		EnablePaletteSync();
		Proc_Break(proc);
		return;
	}

	if (++proc->timer > 30)
		Proc_Break(proc);
}

static const struct ProcCmd ProcScr_AnimPalGuard[] = {
	PROC_NAME("ClassGrowthPalGuard"),
	PROC_REPEAT(AnimPalGuard_Loop),
	PROC_END,
};

static void StartAnimPalGuard(void)
{
	struct ProcAnimPalGuard *proc;

	Proc_EndEach(ProcScr_AnimPalGuard);
	proc = Proc_Start(ProcScr_AnimPalGuard, PROC_TREE_3);
	proc->timer = 0;
	CpuCopy16(AnimPalGuardBank(), proc->pal, sizeof(proc->pal));
}

static u8 ClassChgMenuSel_OnRPress(struct MenuProc *pmenu)
{
	struct ProcClassChgMenuSel *parent = pmenu->proc_parent;
	struct ProcPromoSel *gparent = parent->proc_parent;

	if (!ClassGrowthPreviewEnabled())
		return 0;

	gparent->_u3e = (gparent->_u3e == PROMO_SEL_INFO_GROWTHS)
				  ? PROMO_SEL_INFO_DESC
				  : PROMO_SEL_INFO_GROWTHS;

	PromoSel_DrawClassInfo(gparent);

	/* Started after the talk proc so it runs after it within a frame */
	StartAnimPalGuard();

	SetTalkPrintDelay(-1);
	PlaySoundEffect(SONG_SE_SYS_CURSOR_LR1);
	return 0;
}

extern void ClassChgMenuSelOnInit(struct MenuProc *proc);
extern void ClassChgMenuSelOnEnd(struct MenuProc *proc);
extern u8 ClassChgMenuSelOnPressB(struct MenuProc *pmenu, struct MenuItemProc *pmitem);

/* gMenuDef_PromoSel with an R handler */
static const struct MenuDef MenuDef_PromoSelGrowths = {
	.rect = { 16, 2, 8, 0 },
	.menuItems = gMenuItem_PromoSel,
	.onInit = ClassChgMenuSelOnInit,
	.onEnd = ClassChgMenuSelOnEnd,
	.onBPress = ClassChgMenuSelOnPressB,
	.onRPress = ClassChgMenuSel_OnRPress,
};

static const struct MenuRect ClassChgMenuRect_ = {
	.x = 1,
	.y = 1,
	.w = 12,
	.h = 0,
};

LYN_REPLACE_CHECK(ClassChgMenuExec);
void ClassChgMenuExec(struct ProcClassChgMenuSel *proc)
{
	struct ProcPromoSel *parent = proc->proc_parent;

	parent->_u3e = PROMO_SEL_INFO_GROWTHS;

	proc->unk4C = 0;
	ResetTextFont();
	ResetText();
	SetTextFontGlyphs(0);
	InitTextFont(&gFontClassChg, (void *)BG_VRAM + 0x1400, 160, 5);
	SetTextFont(&gFontClassChg);
	proc->pmenu = StartMenuCore(
		&MenuDef_PromoSelGrowths,
		ClassChgMenuRect_,
		2,
		0,
		0,
		0,
		(struct Proc *) proc);
}

LYN_REPLACE_CHECK(ClassChgMenuItem_OnChange);
int ClassChgMenuItem_OnChange(struct MenuProc *pmenu, struct MenuItemProc *pmitem)
{
	struct ProcClassChgMenuSel *parent = pmenu->proc_parent;
	struct ProcPromoSel *gparent = parent->proc_parent;

	/* The anim reloads for the new class, don't restore the old one's colours */
	Proc_EndEach(ProcScr_AnimPalGuard);

	gparent->stat = 1;
	gparent->main_select = pmitem->itemNumber;
	PromoSel_DrawClassInfo(gparent);
	SetTalkPrintDelay(-1);
	return 0;
}

/* PromoSel_StartClassDesc in the decomp */
LYN_REPLACE_CHECK(sub_80CCF60);
void sub_80CCF60(struct ProcPromoSel *proc)
{
	u16 tmp;

	ResetTextFont();
	ResetText();
	BG_EnableSyncByMask(BG0_SYNC_BIT | BG1_SYNC_BIT | BG2_SYNC_BIT | BG3_SYNC_BIT);

	/* The menu may not have started yet, so set the mode up here as well */
	proc->_u3e = PROMO_SEL_INFO_GROWTHS;

	InitTalk(0x100, 2, 0);
	PromoSel_DrawClassInfo(proc);
	SetTalkPrintDelay(-1);

	gLCDControlBuffer.bg0cnt.priority = 0;
	gLCDControlBuffer.bg1cnt.priority = 2;
	gLCDControlBuffer.bg2cnt.priority = 1;
	gLCDControlBuffer.bg3cnt.priority = 3;

	BG_EnableSyncByMask(BG0_SYNC_BIT | BG1_SYNC_BIT | BG2_SYNC_BIT | BG3_SYNC_BIT);

	tmp = REG_BG0CNT;
	tmp &= 0xFFFC;
	REG_BG0CNT = tmp + 1;
	tmp = REG_BG1CNT;
	tmp &= 0xFFFC;
	REG_BG1CNT = tmp + 1;
	tmp = REG_BG2CNT;
	tmp &= 0xFFFC;
	REG_BG2CNT = tmp + 1;
	tmp = REG_BG3CNT;
	tmp &= 0xFFFC;
	REG_BG3CNT = tmp + 1;
}
