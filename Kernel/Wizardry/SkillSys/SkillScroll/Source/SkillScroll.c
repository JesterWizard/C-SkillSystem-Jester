#include "common-chax.h"
#include "skill-system.h"
#include "icon-rework.h"
#include "kernel-lib.h"
#include "constants/texts.h"
#include "constants/skills.h"
#include "stat-screen.h"
#include "item-sys.h"
#include "event-rework.h"
#include "action-expa.h"

/* External hooks */
bool IsSkillScrollItem(int item)
{

#ifdef CONFIG_ITEM_INDEX_SKILL_SCROLL_1
    if (ITEM_INDEX(item) == CONFIG_ITEM_INDEX_SKILL_SCROLL_1)
        return true;
#endif

#ifdef CONFIG_ITEM_INDEX_SKILL_SCROLL_2
    if (ITEM_INDEX(item) == CONFIG_ITEM_INDEX_SKILL_SCROLL_2)
        return true;
#endif

#ifdef CONFIG_ITEM_INDEX_SKILL_SCROLL_3
    if (ITEM_INDEX(item) == CONFIG_ITEM_INDEX_SKILL_SCROLL_3)
        return true;
#endif

#ifdef CONFIG_ITEM_INDEX_SKILL_SCROLL_4
    if (ITEM_INDEX(item) == CONFIG_ITEM_INDEX_SKILL_SCROLL_4)
        return true;
#endif

    return false;

}

char * GetSkillScrollItemName(int item)
{

#ifdef CONFIG_ITEM_INDEX_SKILL_SCROLL_1
    if (ITEM_INDEX(item) == CONFIG_ITEM_INDEX_SKILL_SCROLL_1)
        return GetSkillNameStr(ITEM_USES(item));
#endif
#ifdef CONFIG_ITEM_INDEX_SKILL_SCROLL_2
    if (ITEM_INDEX(item) == CONFIG_ITEM_INDEX_SKILL_SCROLL_2)
        return GetSkillNameStr(ITEM_USES(item) + 0xFF);
#endif
#ifdef CONFIG_ITEM_INDEX_SKILL_SCROLL_3
    if (ITEM_INDEX(item) == CONFIG_ITEM_INDEX_SKILL_SCROLL_3)
        return GetSkillNameStr(ITEM_USES(item) + 0x1FF);
#endif
#ifdef CONFIG_ITEM_INDEX_SKILL_SCROLL_4
    if (ITEM_INDEX(item) == CONFIG_ITEM_INDEX_SKILL_SCROLL_4)
        return GetSkillNameStr(ITEM_USES(item) + 0x2FF);
#endif
    return "";
}

int GetSkillScrollItemDescId(int item)
{
#ifdef CONFIG_ITEM_INDEX_SKILL_SCROLL_1
    if (ITEM_INDEX(item) == CONFIG_ITEM_INDEX_SKILL_SCROLL_1)
        return GetSkillDescMsg(ITEM_USES(item));
#endif
#ifdef CONFIG_ITEM_INDEX_SKILL_SCROLL_2
    if (ITEM_INDEX(item) == CONFIG_ITEM_INDEX_SKILL_SCROLL_2)
        return GetSkillDescMsg(ITEM_USES(item) + 0xFF);
#endif
#ifdef CONFIG_ITEM_INDEX_SKILL_SCROLL_3
    if (ITEM_INDEX(item) == CONFIG_ITEM_INDEX_SKILL_SCROLL_3)
        return GetSkillDescMsg(ITEM_USES(item) + 0x1FF);
#endif
#ifdef CONFIG_ITEM_INDEX_SKILL_SCROLL_4
    if (ITEM_INDEX(item) == CONFIG_ITEM_INDEX_SKILL_SCROLL_4)
        return GetSkillDescMsg(ITEM_USES(item) + 0x2FF);
#endif
    return 0;
}

int GetSkillScrollItemUseDescId(int item)
{
	// return GetSkillDescMsg(ITEM_USES(item));
	return MSG_ITEM_SkillScrollUseDesc;
}

int GetSkillScrollItemIconId(int item)
{
#ifdef CONFIG_ITEM_INDEX_SKILL_SCROLL_1
    if (ITEM_INDEX(item) == CONFIG_ITEM_INDEX_SKILL_SCROLL_1)
        return SKILL_ICON(ITEM_USES(item));
#endif
#ifdef CONFIG_ITEM_INDEX_SKILL_SCROLL_2
    if (ITEM_INDEX(item) == CONFIG_ITEM_INDEX_SKILL_SCROLL_2)
        return SKILL_ICON(ITEM_USES(item) + 0xFF);
#endif
#ifdef CONFIG_ITEM_INDEX_SKILL_SCROLL_3
    if (ITEM_INDEX(item) == CONFIG_ITEM_INDEX_SKILL_SCROLL_3)
        return SKILL_ICON(ITEM_USES(item) + 0x1FF);
#endif
#ifdef CONFIG_ITEM_INDEX_SKILL_SCROLL_4
    if (ITEM_INDEX(item) == CONFIG_ITEM_INDEX_SKILL_SCROLL_4)
        return SKILL_ICON(ITEM_USES(item) + 0x2FF);
#endif
    return 0;
}

/* Item use */
extern bool sSkillScrollReplaceMenuActive;

void SkillScroll_EndReplaceMenuWait(void)
{
	sSkillScrollReplaceMenuActive = false;
}

static bool SkillScrollReplaceMenuRunning(ProcPtr proc)
{
	(void)proc;
	return sSkillScrollReplaceMenuActive;
}

static int GetSkillIdFromScroll(int item)
{
	int sid;

	if (TryGetSkillScrollSid(item, &sid))
		return sid;

	return 0;
}

static u16 MakeSkillScrollItem(u16 sid)
{
#ifdef CONFIG_TURN_ON_ALL_SKILLS
	if (sid > 0x2FF)
		return ((sid - 0x2FF) << 8) | CONFIG_ITEM_INDEX_SKILL_SCROLL_4;
	if (sid > 0x1FF)
		return ((sid - 0x1FF) << 8) | CONFIG_ITEM_INDEX_SKILL_SCROLL_3;
	if (sid > 0x0FF)
		return ((sid - 0x0FF) << 8) | CONFIG_ITEM_INDEX_SKILL_SCROLL_2;
#endif
#ifdef CONFIG_ITEM_INDEX_SKILL_SCROLL_1
	return (sid << 8) | CONFIG_ITEM_INDEX_SKILL_SCROLL_1;
#else
	return 0;
#endif
}

static void call_remove_skill_menu(void)
{
	StartSubtitleHelp(
		StartOrphanMenu(&RemoveSkillMenuDef),
		GetStringFromIndex(MSG_RemoveSkillSubtitle)
	);
}

static void call_predation_skill_menu(void)
{
    StartSubtitleHelp(
        StartOrphanMenu(&PredationSkillMenuDef),
        GetStringFromIndex(MSG_PredationSkillChoice)
    );
}


/* After the skill menu is called, this proc ends and what it was blocking resumes */

const struct ProcCmd ProcScr_SkillScrollUseSoftLock[] = {
    PROC_YIELD,
    PROC_CALL(call_remove_skill_menu),
    PROC_WHILE(SkillScrollReplaceMenuRunning),
    PROC_END
};

const struct ProcCmd ProcScr_PredationSoftLock[] = {
    PROC_YIELD,
    PROC_SLEEP(150), /* When predation is active, sleep the thread so the learned skill can be shown in a popup */
    PROC_CALL(call_remove_skill_menu),
    PROC_END
};

const struct ProcCmd ProcScr_PredationPlusSoftLock[] = {
    PROC_YIELD,
    // PROC_SLEEP(150), /* When predation is active, sleep the thread so the learned skill can be shown in a popup */
    PROC_CALL(call_predation_skill_menu),
    PROC_END
};


/**
 * BLOCK USAGE OF SCROLL IF UNIT WOULD BE ABOVE CAPACITY LIMIT AFTER APPLYING IT
 */
static const EventScr EventScr_SkillCapacityReached[] = {
    EVBIT_MODIFY(0x4)
    TUTORIALTEXTBOXSTART
    SVAL(EVT_SLOT_B, 0xffffffff)
    TEXTSHOW(MSG_Skill_Capacity_Reached)
    TEXTEND
    REMA
    NOFADE
    ENDA
};

void ItemUseEffect_SkillScroll(struct Unit *unit)
{
	gActionData.unk08 = 0xFFFF;
	gEventSlots[EVT_SLOT_7] = 0;

	if (gpKernelDesignerConfig->gen_new_scroll && GetFreeSkillSlot(unit) == -1) {
		sSkillScrollReplaceMenuActive = true;
		Proc_StartBlocking(ProcScr_SkillScrollUseSoftLock, Proc_Find(gProcScr_PlayerPhase));
		return;
	}

	SetItemUseAction(unit);
}

void ItemUseAction_SkillScroll(ProcPtr proc)
{
	struct Unit *unit = GetUnit(gActionData.subjectIndex);
	int slot = gActionData.itemSlotIndex;
	int item = unit->items[slot];
	int learnedSid = GetSkillIdFromScroll(item);
	bool replacing = gEventSlots[EVT_SLOT_7] == 0xFFFF;
	int sid_rep = 0;

	if (!learnedSid)
		return;

	if (replacing)
		sid_rep = GET_SKILL(unit, gActionData.unk08);

	if (gpKernelDesignerConfig->tellius_skill_capacity_system == true) {
		int amt = GetUnitBattleAmt(unit);
		int total = gpKernelDesignerConfig->tellius_skill_capacity_base;
		int capacity = GetSkillCapacity(learnedSid);

		if (UNIT_CATTRIBUTES(unit) & CA_PROMOTED)
			total += gpKernelDesignerConfig->tellius_skill_capacity_promoted;

#if defined(SID_CapacityHalf) && (COMMON_SKILL_VALID(SID_CapacityHalf))
		if (SkillTester(unit, SID_CapacityHalf))
			capacity = capacity / 2;
#endif
#if defined(SID_CapacityOne) && (COMMON_SKILL_VALID(SID_CapacityOne))
		if (SkillTester(unit, SID_CapacityOne))
			capacity = 1;
#endif

		if (replacing)
			amt -= GetSkillCapacity(sid_rep);

		if (amt < 0)
			amt = 0;

		amt += capacity;

		if (amt > total) {
			KernelCallEvent(EventScr_SkillCapacityReached, EV_EXEC_CUTSCENE, proc);
			gActionDataExpa.refrain_action = true;
			return;
		}
	}

	if (replacing) {
		RemoveSkill(unit, sid_rep);
		ForgetSkill(unit, sid_rep);
	}

	AddSkill(unit, learnedSid);
	SetPopupItem(learnedSid);

	if (replacing && (gpKernelDesignerConfig->gen_new_scroll
#if defined(SID_ScrollScribe) && (COMMON_SKILL_VALID(SID_ScrollScribe))
		|| SkillTester(unit, SID_ScrollScribe)
#endif
		)) {
		unit->items[slot] = MakeSkillScrollItem(sid_rep);
	} else {
		UnitUpdateUsedItem(unit, slot);
	}

	SetPopupUnit(gActiveUnit);
	NewPopup_Simple(PopupScr_LearnSkill, SONG_SE_UPDATE, 0x00, proc);
}

bool ItemUsability_SkillScroll(struct Unit *unit, int item)
{
	int sid = GetSkillIdFromScroll(item);

	if (!sid)
		return false;

	return !IsSkillLearned(unit, sid);
}

/* Prep item use */
void PrepItemUseScroll_OnDraw(struct ProcPrepItemUseJunaFruit *proc, int item, int x, int y)
{
	int skill = GetSkillIdFromScroll(item);
	const char *str = GetStringFromIndex(MSG_SkillLearned);
	struct Text *text = &gPrepItemTexts[TEXT_PREPITEM_POPUP];
	int icon = SKILL_ICON(skill);
	int width = GetStringTextLen(str);

    DrawIcon(TILEMAP_LOCATED(gBG2TilemapBuffer, x, y), icon, TILEREF(0, STATSCREEN_BGPAL_ITEMICONS + GetSkillIconPal(skill)));

	ClearText(text);
	PutDrawText(
		text,
		TILEMAP_LOCATED(gBG2TilemapBuffer, x + 2, y),
		TEXT_COLOR_SYSTEM_WHITE,
		0, 0, str
	);

	BG_EnableSyncByMask(BG2_SYNC_BIT);

	proc->xpos = x * 8 - 4;
	proc->ypos = y * 8 - 4;
	proc->width = width / 8 + 3;
	proc->height = 2;
}

void PrepItemUseScroll_OnInit(struct ProcPrepItemUseJunaFruit *proc)
{
	struct ProcPrepItemUse *parent = proc->proc_parent;

	DrawPrepScreenItemUseStatBars(parent->unit, 0);
	DrawPrepScreenItemUseStatValues(parent->unit);

	PrepItemUseScroll_OnDraw(proc, parent->unit->items[parent->slot], 0x11, 0x0E);

	proc->timer = 0x78;
	PlaySoundEffect(0x5A);
}

void PrepItemUseScroll_OnEnd(struct ProcPrepItemUseJunaFruit *proc)
{
	struct ProcPrepItemUse *parent = proc->proc_parent;

	AddSkill(parent->unit, GetSkillIdFromScroll(parent->unit->items[parent->slot]));
	UnitUpdateUsedItem(parent->unit, parent->slot);
	PrepItemUseJuna_OnEnd(proc);
}

const struct ProcCmd ProcScr_PrepItemUseScroll[] = {
	PROC_SET_END_CB(PrepItemUseScroll_OnEnd),
	PROC_CALL(PrepItemUseScroll_OnInit),
	PROC_REPEAT(PrepItemUseJuna_IDLE),
	PROC_END
};

void PrepItemEffect_SkillScroll(struct ProcPrepItemUse *proc, u16 item)
{
	Proc_StartBlocking(ProcScr_PrepItemUseScroll, proc);
}

bool PrepItemUsability_SkillScroll(struct Unit *unit, int item)
{
	int sid = GetSkillIdFromScroll(item);

	if (!sid || IsSkillLearned(unit, sid))
		return false;

	/* Replace-and-generate-scroll needs the map menu; prep has no replace UI */
	if (gpKernelDesignerConfig->gen_new_scroll && GetFreeSkillSlot(unit) == -1)
		return false;

	return true;
}

/**
 * IER port
 */
bool IER_Usability_SkillScroll(struct Unit *unit, int item)
{
	return ItemUsability_SkillScroll(unit, item);
}

bool IER_PrepUsability_SkillScroll(struct Unit *unit, int item)
{
	return PrepItemUsability_SkillScroll(unit, item);
}

void IER_Effect_SkillScroll(struct Unit *unit, int item)
{
	ItemUseEffect_SkillScroll(unit);
}

void IER_ActionEffect_SkillScroll(ProcPtr proc, struct Unit *unit, int item)
{
	ItemUseAction_SkillScroll(proc);
}

void IER_PrepEffect_SkillScroll(struct ProcPrepItemUse *proc, u16 item)
{
	PrepItemEffect_SkillScroll(proc, item);
}
