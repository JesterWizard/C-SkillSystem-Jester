#include "global.h"
#include "proc.h"
#include "hardware.h"
#include "fontgrp.h"
#include "uimenu.h"
#include "scene.h"
#include "classchg.h"
#include "soundwrapper.h"
#include "bmunit.h"
#include "bmbattle.h"
#include "bmitem.h"
#include "variables.h"
#include "constants/characters.h"
#include "constants/items.h"
#include "constants/songs.h"

/**
 * Class growths: a unit's class growths are merged into its personal growths
 * when it levels up.
 *
 *   mode 1: personal + class
 *   mode 2: (personal + class) / 2
 *
 * Standalone port of the C Skill System's class_growths
 * (Kernel/Wizardry/ClassGrowths). The kernel merges them in its growth
 * getters; vanilla FE8U has none, and every level-up routine (vanilla
 * CheckBattleUnitLevelUp or a patch replacing it) reads the growths straight
 * from bu->unit.pCharacterData. So the vanilla functions that call
 * CheckBattleUnitLevelUp are replaced here with copies that point the battle
 * unit at a copy of its character data with merged growths for the duration
 * of the call. Whatever CheckBattleUnitLevelUp is installed (vanilla,
 * levelup_modes, guaranteed_lvup, two_random_number_growths' GetStatIncrease)
 * then rolls on the merged growths.
 *
 * Class growths are the vanilla ClassData growth fields (FEBuilder class
 * editor), which vanilla also uses to autolevel generic units.
 */

enum class_growths_mode {
	CLASS_GROWTHS_OFF = 0,
	CLASS_GROWTHS_ADD = 1,     /* personal + class */
	CLASS_GROWTHS_AVERAGE = 2, /* (personal + class) / 2 */
};

/* Vanilla symbols FE-CLib does not declare */
extern struct Font *gActiveFont;

/* Installer.event: mode, promotion screen preview on/off */
extern const u8 ClassGrowthsConfig[2];

#define CLASS_GROWTHS_MODE (ClassGrowthsConfig[0])
#define CLASS_GROWTHS_PREVIEW (ClassGrowthsConfig[1])

static int GetClassGrowthBase(int personal, int job)
{
	int growth;

	switch (CLASS_GROWTHS_MODE) {
	case CLASS_GROWTHS_ADD:
		growth = personal + job;
		break;

	case CLASS_GROWTHS_AVERAGE:
		growth = (personal + job) / 2;
		break;

	default:
		return personal;
	}

	/* Character growths are bytes */
	if (growth > 0xFF)
		growth = 0xFF;

	return growth;
}

/* Vanilla class growths are unsigned in practice, read them as such */
#define CLASS_GROWTH(cls, field) ((u8)(cls)->field)

static void MergeClassGrowths(struct CharacterData *dst, const struct CharacterData *ch, const struct ClassData *cls)
{
	dst->growthHP  = GetClassGrowthBase(ch->growthHP,  CLASS_GROWTH(cls, growthHP));
	dst->growthPow = GetClassGrowthBase(ch->growthPow, CLASS_GROWTH(cls, growthPow));
	dst->growthSkl = GetClassGrowthBase(ch->growthSkl, CLASS_GROWTH(cls, growthSkl));
	dst->growthSpd = GetClassGrowthBase(ch->growthSpd, CLASS_GROWTH(cls, growthSpd));
	dst->growthDef = GetClassGrowthBase(ch->growthDef, CLASS_GROWTH(cls, growthDef));
	dst->growthRes = GetClassGrowthBase(ch->growthRes, CLASS_GROWTH(cls, growthRes));
	dst->growthLck = GetClassGrowthBase(ch->growthLck, CLASS_GROWTH(cls, growthLck));
}

/**
 * CheckBattleUnitLevelUp with the unit's class growths merged in. The copy
 * only lives for this call: nothing keeps bu->unit.pCharacterData past it.
 */
static void CheckBattleUnitLevelUp_ClassGrowths(struct BattleUnit *bu)
{
	struct CharacterData merged;
	const struct CharacterData *ch = bu->unit.pCharacterData;
	struct Unit *unit;

	if (CLASS_GROWTHS_MODE == CLASS_GROWTHS_OFF || ch == NULL || bu->unit.pClassData == NULL) {
		CheckBattleUnitLevelUp(bu);
		return;
	}

	merged = *ch;
	MergeClassGrowths(&merged, ch, bu->unit.pClassData);

	/**
	 * Some level-up rolls read the growths from the real unit instead
	 * (Standalone/levelup_modes' fixed mode), so point it at the copy too.
	 */
	unit = bu->unit.index ? GetUnit(bu->unit.index) : NULL;
	if (unit != NULL && unit->pCharacterData != ch)
		unit = NULL;

	bu->unit.pCharacterData = &merged;
	if (unit)
		unit->pCharacterData = &merged;

	CheckBattleUnitLevelUp(bu);

	bu->unit.pCharacterData = ch;
	if (unit)
		unit->pCharacterData = ch;
}

/* Vanilla callers of CheckBattleUnitLevelUp (decomp copies) */

void BattleApplyExpGains_ClassGrowths(void)
{
	if ((UNIT_FACTION(&gBattleActor.unit) != FACTION_BLUE) || (UNIT_FACTION(&gBattleTarget.unit) != FACTION_BLUE)) {
		if (!(gPlaySt.chapterStateBits & PLAY_FLAG_EXTRA_MAP)) {
			gBattleActor.expGain  = GetBattleUnitExpGain(&gBattleActor, &gBattleTarget);
			gBattleTarget.expGain = GetBattleUnitExpGain(&gBattleTarget, &gBattleActor);

			gBattleActor.unit.exp  += gBattleActor.expGain;
			gBattleTarget.unit.exp += gBattleTarget.expGain;

			CheckBattleUnitLevelUp_ClassGrowths(&gBattleActor);
			CheckBattleUnitLevelUp_ClassGrowths(&gBattleTarget);
		}
	}
}

#define ITYPE_DANCE_VANILLA 12

void BattleApplyItemExpGains_ClassGrowths(void)
{
	if (!(gPlaySt.chapterStateBits & PLAY_FLAG_EXTRA_MAP)) {
		if (gBattleActor.weaponAttributes & IA_STAFF) {
			if (UNIT_FACTION(&gBattleActor.unit) == FACTION_BLUE)
				gBattleActor.wexpMultiplier++;

			gBattleActor.expGain = GetBattleUnitStaffExp(&gBattleActor);
			gBattleActor.unit.exp += gBattleActor.expGain;

			CheckBattleUnitLevelUp_ClassGrowths(&gBattleActor);
		} else if ((gBattleActor.weaponType == ITYPE_DANCE_VANILLA) && (gBattleActor.unit.exp != UNIT_EXP_DISABLED)) {
			gBattleActor.expGain = 20;
			gBattleActor.unit.exp += 20;

			CheckBattleUnitLevelUp_ClassGrowths(&gBattleActor);
		}
	}
}

void BattleApplyMiscActionExpGains_ClassGrowths(void)
{
	if (UNIT_FACTION(&gBattleActor.unit) != FACTION_BLUE)
		return;

	if (!CanBattleUnitGainLevels(&gBattleActor))
		return;

	if (gPlaySt.chapterStateBits & PLAY_FLAG_EXTRA_MAP)
		return;

	gBattleActor.expGain = 10;
	gBattleActor.unit.exp += 10;

	CheckBattleUnitLevelUp_ClassGrowths(&gBattleActor);
}

void UnitAutolevelRealistic_ClassGrowths(struct Unit *unit)
{
	struct BattleUnit tmpBattleUnit;
	short levelsLeft;

	tmpBattleUnit.expGain = 0;

	levelsLeft = (unit->level - unit->pCharacterData->baseLevel);

	if (levelsLeft) {
		for (unit->level -= levelsLeft; levelsLeft > 0; --levelsLeft) {
			InitBattleUnit(&tmpBattleUnit, unit);

			tmpBattleUnit.unit.exp += 100;
			CheckBattleUnitLevelUp_ClassGrowths(&tmpBattleUnit);

			UpdateUnitFromBattle(unit, &tmpBattleUnit);
		}
	}
}

/* Levels up the other twin when the routes split (chapter 8) */
void AutolevelSecondaryLord_ClassGrowths(void)
{
	u8 i;
	struct BattleUnit bunit;
	struct Unit *unit = NULL;

	switch (gPlaySt.chapterModeIndex) {
	case CHAPTER_MODE_EIRIKA:
		unit = GetUnitFromCharId(CHARACTER_EPHRAIM);
		break;

	case CHAPTER_MODE_EPHRAIM:
		unit = GetUnitFromCharId(CHARACTER_EIRIKA);
		break;
	}

	/* Vanilla does not check this either; it only runs on the route split */
	if (unit == NULL)
		return;

	if (unit->level < 15) {
		u8 old_level = unit->level;
		u8 tar_level = unit->level + 6;

		if (tar_level < 10)
			tar_level = 10;
		if (tar_level > 15)
			tar_level = 15;

		while (old_level < tar_level) {
			InitBattleUnit(&bunit, unit);
			bunit.unit.exp += 100;
			CheckBattleUnitLevelUp_ClassGrowths(&bunit);
			UpdateUnitFromBattle(unit, &bunit);
			old_level++;
		}
		unit->exp = 0;
	}

	for (i = 0; i < 8; i++) {
		u8 rank = unit->ranks[i] - 1;

		if (rank <= 0x45)
			unit->ranks[i] = 0x47;
	}

	for (i = 0; i < 5; i++) {
		if (0 == unit->items[i]) {
			switch (gPlaySt.chapterModeIndex) {
			case CHAPTER_MODE_EIRIKA:
				UnitAddItem(unit, MakeNewItem(ITEM_LANCE_STEEL));
				break;

			case CHAPTER_MODE_EPHRAIM:
				UnitAddItem(unit, MakeNewItem(ITEM_SWORD_STEEL));
				break;
			}
			break;
		}
	}
}

/**
 * Class growth preview on the promotion screen.
 *
 * The class description box shows how the highlighted class would change the
 * unit's growths. R swaps the box between the growths and the vanilla class
 * description.
 */

enum class_growth_stat {
	CLASS_GROWTH_HP,
	CLASS_GROWTH_POW,
	CLASS_GROWTH_SKL,
	CLASS_GROWTH_SPD,
	CLASS_GROWTH_LCK,
	CLASS_GROWTH_DEF,
	CLASS_GROWTH_RES,

	CLASS_GROWTH_MAX
};

static const char *const ClassGrowthStatNames[CLASS_GROWTH_MAX] = {
	[CLASS_GROWTH_HP]  = "HP",
	[CLASS_GROWTH_POW] = "Str",
	[CLASS_GROWTH_SKL] = "Skl",
	[CLASS_GROWTH_SPD] = "Spd",
	[CLASS_GROWTH_LCK] = "Lck",
	[CLASS_GROWTH_DEF] = "Def",
	[CLASS_GROWTH_RES] = "Res",
};

/* Entries on the first line */
#define CLASS_GROWTH_FIRST_LINE 4

static int GetClassGrowthByStat(const struct CharacterData *ch, const struct ClassData *cls, int stat)
{
	switch (stat) {
	case CLASS_GROWTH_HP:  return GetClassGrowthBase(ch->growthHP,  CLASS_GROWTH(cls, growthHP));
	case CLASS_GROWTH_POW: return GetClassGrowthBase(ch->growthPow, CLASS_GROWTH(cls, growthPow));
	case CLASS_GROWTH_SKL: return GetClassGrowthBase(ch->growthSkl, CLASS_GROWTH(cls, growthSkl));
	case CLASS_GROWTH_SPD: return GetClassGrowthBase(ch->growthSpd, CLASS_GROWTH(cls, growthSpd));
	case CLASS_GROWTH_LCK: return GetClassGrowthBase(ch->growthLck, CLASS_GROWTH(cls, growthLck));
	case CLASS_GROWTH_DEF: return GetClassGrowthBase(ch->growthDef, CLASS_GROWTH(cls, growthDef));
	case CLASS_GROWTH_RES: return GetClassGrowthBase(ch->growthRes, CLASS_GROWTH(cls, growthRes));
	default: return 0;
	}
}

static bool ClassGrowthPreviewEnabled(void)
{
	return CLASS_GROWTHS_MODE != CLASS_GROWTHS_OFF && CLASS_GROWTHS_PREVIEW;
}

/* Kept in ProcPromoSel::_u3e, which vanilla leaves unused */
enum promo_sel_info_mode {
	PROMO_SEL_INFO_GROWTHS,
	PROMO_SEL_INFO_DESC,
};

/**
 * Talk colour groups for the growth changes. The talk palette's gold and red
 * groups go unused here, so they are reloaded with the stat screen's green
 * and a matching red, in the talk font's outline/shadow/fill order.
 */
enum promo_sel_growth_color {
	PROMO_SEL_COLOR_DEFAULT = 0,
	PROMO_SEL_COLOR_UP = 3,
	PROMO_SEL_COLOR_DOWN = 4,
};

static const u16 PromoSelColorUp[3] = {
	RGB(3, 11, 3), RGB(14, 20, 9), RGB(18, 30, 14),
};

static const u16 PromoSelColorDown[3] = {
	RGB(11, 3, 3), RGB(20, 9, 8), RGB(31, 15, 13),
};

/* RamAlloc.s, 128 bytes: the talk reads the string while it prints */
extern char ClassGrowthPreviewText[];
#define CLASS_GROWTH_PREVIEW_TEXT_SIZE 0x80

/**
 * Growth changes are drawn green when they go up and red when they go down.
 * This proc lives under the promotion screen and puts the talk palette's
 * original colours back when the screen ends.
 */
struct ProcTextColorKeeper {
	PROC_HEADER;
	u16 offset;
	u16 saved[3];
};

static void TextColorKeeper_OnEnd(struct ProcTextColorKeeper *proc)
{
	CpuCopy16(proc->saved, gPaletteBuffer + proc->offset, sizeof(proc->saved));
	EnablePaletteSync();
}

static const struct ProcCmd ProcScr_TextColorKeeper[] = {
	PROC_NAME("ClassGrowthTextColor"),
	PROC_SET_END_CB(TextColorKeeper_OnEnd),
	PROC_BLOCK,
};

static void SetClassGrowthTextColor(ProcPtr screen, int palid, int colorId, const u16 colors[3])
{
	struct ProcTextColorKeeper *proc;
	struct ProcFindIterator it;
	int offset = palid * 0x10 + colorId * 3 + 1;

	Proc_FindBegin(&it, ProcScr_TextColorKeeper);
	while ((proc = Proc_FindNext(&it)) != NULL)
		if (proc->offset == offset)
			break;

	if (!proc) {
		proc = Proc_Start(ProcScr_TextColorKeeper, screen);
		proc->offset = offset;
		CpuCopy16(gPaletteBuffer + offset, proc->saved, sizeof(proc->saved));
	}

	CpuCopy16(colors, gPaletteBuffer + offset, sizeof(proc->saved));
	EnablePaletteSync();
}

static char *AppendText(char *dst, const char *src)
{
	while (*src)
		*dst++ = *src++;

	return dst;
}

/**
 * Vanilla talk code 0x80 0x01 XX sets the print colour to XX (TalkInterpret
 * reads 0x80 0x00..0x03 followed by the colour). XX may be 0: the interpreter
 * consumes it as the colour, it does not end the text.
 */
static char *AppendColor(char *dst, int color)
{
	*dst++ = 0x80;
	*dst++ = 0x01;
	*dst++ = color;
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

/**
 * Two lines of "Str +10%" entries, gains green and losses red. At most
 * 7 * 17 bytes + [NL] + end, which fits ClassGrowthPreviewText.
 */
static void BuildClassGrowthPreviewText(struct Unit *unit, int jid)
{
	const struct ClassData *newcls = GetClassData(jid);
	char *str = ClassGrowthPreviewText;
	int i;

	for (i = 0; i < CLASS_GROWTH_MAX; i++) {
		int diff = GetClassGrowthByStat(unit->pCharacterData, newcls, i)
				 - GetClassGrowthByStat(unit->pCharacterData, unit->pClassData, i);

		if (i == CLASS_GROWTH_FIRST_LINE)
			*str++ = 0x01; /* [NL] */
		else if (i != 0)
			str = AppendText(str, "  ");

		str = AppendText(str, ClassGrowthStatNames[i]);
		*str++ = ' ';

		if (diff == 0) {
			str = AppendGrowthDiff(str, diff);
			continue;
		}

		str = AppendColor(str, diff > 0 ? PROMO_SEL_COLOR_UP : PROMO_SEL_COLOR_DOWN);
		str = AppendGrowthDiff(str, diff);
		str = AppendColor(str, PROMO_SEL_COLOR_DEFAULT);
	}

	*str = '\0';
}

static void ChangeClassGrowthPreview(struct ProcPromoSel *proc, struct Unit *unit, int jid)
{
	BuildClassGrowthPreviewText(unit, jid);

	/* Same box setup as ChangeClassDescription */
	SetInitTalkTextFont();
	SetClassGrowthTextColor(proc, gActiveFont->palid, PROMO_SEL_COLOR_UP, PromoSelColorUp);
	SetClassGrowthTextColor(proc, gActiveFont->palid, PROMO_SEL_COLOR_DOWN, PromoSelColorDown);
	ClearTalkText();
	StartTalk(2, 0xF, ClassGrowthPreviewText);
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
	struct Unit *unit = GetUnitFromCharId(proc->pid);

	if (ClassGrowthPreviewEnabled() && unit != NULL && proc->_u3e == PROMO_SEL_INFO_GROWTHS)
		ChangeClassGrowthPreview(proc, unit, proc->jid[proc->main_select]);
	else
		ChangeClassDescription(proc->msg_desc[proc->main_select]);
}

/**
 * The talk may load Pal_Text into OBJ palette 0xA a frame after it starts
 * (Standalone/text_engine does, for its floating glyphs), and the promotion
 * screen's battle anim uses that palette too. A cursor move reloads the anim
 * afterwards; R does not, so this proc puts the anim's colours back if they
 * were overwritten.
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
extern CONST_DATA struct MenuItemDef gMenuItem_PromoSel[];

/* Vanilla gMenuDef_PromoSel with an R handler */
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

/* Replaces vanilla ClassChgMenuExec */
void ClassChgMenuExec_ClassGrowths(struct ProcClassChgMenuSel *proc)
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

/* Replaces vanilla ClassChgMenuItem_OnChange */
int ClassChgMenuItem_OnChange_ClassGrowths(struct MenuProc *pmenu, struct MenuItemProc *pmitem)
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

/* Replaces vanilla sub_80CCF60 (PromoSel_StartClassDesc in the decomp) */
void PromoSel_StartClassDesc_ClassGrowths(struct ProcPromoSel *proc)
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
