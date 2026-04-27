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
static int GetSkillScrollSid(int item)
{
#ifdef CONFIG_ITEM_INDEX_SKILL_SCROLL_1
    if (ITEM_INDEX(item) == CONFIG_ITEM_INDEX_SKILL_SCROLL_1)
        return ITEM_USES(item);
#endif
#ifdef CONFIG_ITEM_INDEX_SKILL_SCROLL_2
    if (ITEM_INDEX(item) == CONFIG_ITEM_INDEX_SKILL_SCROLL_2)
        return ITEM_USES(item) + 0x100;
#endif
#ifdef CONFIG_ITEM_INDEX_SKILL_SCROLL_3
    if (ITEM_INDEX(item) == CONFIG_ITEM_INDEX_SKILL_SCROLL_3)
        return ITEM_USES(item) + 0x200;
#endif
#ifdef CONFIG_ITEM_INDEX_SKILL_SCROLL_4
    if (ITEM_INDEX(item) == CONFIG_ITEM_INDEX_SKILL_SCROLL_4)
        return ITEM_USES(item) + 0x300;
#endif

    return ITEM_USES(item);
}

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
    return GetSkillNameStr(GetSkillScrollSid(item));
}

int GetSkillScrollItemDescId(int item)
{
    return GetSkillDescMsg(GetSkillScrollSid(item));
}

int GetSkillScrollItemUseDescId(int item)
{
	// return GetSkillDescMsg(ITEM_USES(item));
	return MSG_ITEM_SkillScrollUseDesc;
}

int GetSkillScrollItemIconId(int item)
{
    return SKILL_ICON(GetSkillScrollSid(item));
}

/* Item use */
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

static void wait_for_skill_scroll_selection(struct Proc *proc)
{
    if (gEventSlots[EVT_SLOT_7] == 0xFFFF)
        Proc_Break(proc);
}


/* After the skill menu is called, this proc ends and what it was blocking resumes */

const struct ProcCmd ProcScr_SkillScrollUseSoftLock[] = {
    PROC_YIELD,
    PROC_CALL(call_remove_skill_menu),
    PROC_REPEAT(wait_for_skill_scroll_selection),
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
	gActionData.unk08 = -1;
	SetItemUseAction(unit);

	if (gpKernelDesignerConfig->gen_new_scroll == false) {
		/**
		 * If the unit has been filled with equipable skills,
		 * player need to select to remove a equipped skill.
		 **/
		if (GetFreeSkillSlot(unit) == -1)
			Proc_StartBlocking(ProcScr_SkillScrollUseSoftLock, Proc_Find(gProcScr_PlayerPhase));
	}
}

void ItemUseAction_SkillScroll(ProcPtr proc)
{
    struct Unit * unit = GetUnit(gActionData.subjectIndex);
    int slot = gActionData.itemSlotIndex;
    FORCE_DECLARE int item = unit->items[slot];
    int sid = GetSkillScrollSid(item);

    if (gpKernelDesignerConfig->tellius_skill_capacity_system == true)
    {
        int amt = GetUnitBattleAmt(gActiveUnit);
        int total = gpKernelDesignerConfig->tellius_skill_capacity_base;

        if (UNIT_CATTRIBUTES(unit) & CA_PROMOTED)
            total += gpKernelDesignerConfig->tellius_skill_capacity_promoted;

        int capacity = GetSkillCapacity(sid);

        if (capacity == -1)
            capacity = 0;
        else {
    #if defined(SID_CapacityHalf) && (COMMON_SKILL_VALID(SID_CapacityHalf))
            if (SkillTester(unit, SID_CapacityHalf))
                capacity = capacity / 2;
    #endif
    #if defined(SID_CapacityOne) && (COMMON_SKILL_VALID(SID_CapacityOne))
            if (SkillTester(unit, SID_CapacityOne))
                capacity = 1;
    #endif
            }

        amt += capacity;

        if (amt > total)
        {
            KernelCallEvent(EventScr_SkillCapacityReached, EV_EXEC_CUTSCENE, proc);
            gActionDataExpa.refrain_action = true;
            return;
        }
    }

    if (gEventSlots[EVT_SLOT_7] == 0xFFFF)
    {
        /* Replace skill */
        int slot_rep = gActionData.unk08;
        int sid_rep = GET_SKILL(unit, slot_rep);

        if (slot_rep >= 0 && slot_rep < UNIT_RAM_SKILLS_LEN) {
            LearnSkill(unit, sid);
            SET_SKILL(unit, slot_rep, sid);
            ResetSkillLists();
        }

        PushSkillListStack(sid);
        SetPopupItem(sid);

#if defined(SID_ScrollScribe) && (COMMON_SKILL_VALID(SID_ScrollScribe))
        if (SkillTester(unit, SID_ScrollScribe))
        {
            unit->items[slot] = ITEM_INDEX(item) | (sid_rep << 8);
        }
        else 
            UnitUpdateUsedItem(unit, slot);
#else 
        UnitUpdateUsedItem(unit, slot);
#endif

    }
    else
    {
        /* Simply add a new skill */

        AddSkill(unit, sid);
        PushSkillListStack(sid);
        SetPopupItem(sid);
        UnitUpdateUsedItem(unit, slot);
    }

    SetPopupUnit(gActiveUnit);

    NewPopup_Simple(PopupScr_LearnSkill, SONG_SE_UPDATE, 0x00, proc);
}

bool ItemUsability_SkillScroll(struct Unit *unit, int item)
{
    return !IsSkillLearned(unit, GetSkillScrollSid(item));
}

/* Prep item use */
void PrepItemUseScroll_OnDraw(struct ProcPrepItemUseJunaFruit *proc, int item, int x, int y)
{
    int skill = GetSkillScrollSid(item);
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

    gActionData.unk08 = -1;
    gEventSlots[EVT_SLOT_7] = 0;
	DrawPrepScreenItemUseStatBars(parent->unit, 0);
	DrawPrepScreenItemUseStatValues(parent->unit);

	PrepItemUseScroll_OnDraw(proc, parent->unit->items[parent->slot], 0x11, 0x0E);

    if (GetFreeSkillSlot(parent->unit) == -1) {
        Proc_StartBlocking(ProcScr_SkillScrollUseSoftLock, proc);
    }

	proc->timer = 0x78;
	PlaySoundEffect(0x5A);
}

void PrepItemUseScroll_OnEnd(struct ProcPrepItemUseJunaFruit *proc)
{
	struct ProcPrepItemUse *parent = proc->proc_parent;
    int sid = GetSkillScrollSid(parent->unit->items[parent->slot]);
    int slot = gActionData.unk08;

    if (slot >= 0 && slot < UNIT_RAM_SKILLS_LEN) {
        LearnSkill(parent->unit, sid);
        SET_SKILL(parent->unit, slot, sid);
        ResetSkillLists();
    }
    else {
        AddSkill(parent->unit, sid);
    }

    gActionData.unk08 = -1;
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
	if (gpKernelDesignerConfig->gen_new_scroll == false) {
		return true;
	}

	/**
	 * If player can equip skill by themself,
	 * then they just need to avoid from learned skill.
	 */
    return !IsSkillLearned(unit, GetSkillScrollSid(item));
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
