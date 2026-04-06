#include "common-chax.h"
#include "item-sys.h"
#include "unit-expa.h"
#include "strmag.h"
#include "status-getter.h"
#include "skill-system.h"
#include "constants/skills.h"
#include "constants/texts.h"
#include "debuff.h"
#include "popup-reowrk.h"
#include "playerphase.h"
#include "jester_headers/custom-functions.h"

extern u8 gUnitTonicState[];
extern u16 gTonicChapterState;

static const struct PopupInstruction sTonicUsedPopup[] = {
	POPUP_SOUND(0x5A),
	POPUP_COLOR(TEXT_COLOR_SYSTEM_BLUE),
	POPUP_UNIT_NAME,
	POPUP_SPACE(1),
	POPUP_COLOR(TEXT_COLOR_SYSTEM_WHITE),
	POPUP_MSG(MSG_USED),
	POPUP_SPACE(1),
	POPUP_ITEM_ICON,
	POPUP_SPACE(1),
	POPUP_COLOR(TEXT_COLOR_SYSTEM_GOLD),
	POPUP_ITEM_STR,
	POPUP_END,
};

bool IsTonicCampaignActive(int item)
{
	struct Unit *unit = GetUnit(gActionData.subjectIndex);

	if (!UNIT_IS_VALID(unit))
		return false;

	return ITEM_INDEX(item) == ITEM_TONIC
		&& gUnitTonicState[unit->index] == ITEM_USES(item)
		&& gTonicChapterState == gPlaySt.chapterIndex;
}

bool IsTonicCampaignActiveIndex(int tonicIndex)
{
	struct Unit *unit = GetUnit(gActiveUnit ? gActiveUnit->index : 0);

	if (!UNIT_IS_VALID(unit))
		return false;

	return gUnitTonicState[unit->index] == tonicIndex;
}

int GetTonicStatBonus(struct Unit *unit, int tonicIndex)
{
    extern u8 gUnitTonicState[];
    extern u16 gTonicChapterState;
    if (gTonicChapterState != gPlaySt.chapterIndex)
        return 0;

    if (gUnitTonicState[unit->index] == 9 && tonicIndex >= 0 && tonicIndex <= 7)
        return 2;

    return gUnitTonicState[unit->index] == tonicIndex ? 2 : 0;
}

#define LOCAL_TRACE 0

STATIC_DECLAR bool IER_CheckIList(int iid, const u8 *list)
{
	int i;

	for (i = 0; list[i] != ITEM_NONE; i++)
		if (iid == list[i])
			return true;

	return false;
}

/**
 * Desc
 */
// LYN__REPLACE_CHECK(GetItemCantUseMsgid);

/**
 * Promotion list
 */
LYN_REPLACE_CHECK(CanUnitUsePromotionItem);
bool CanUnitUsePromotionItem(struct Unit *unit, int item)
{
	int iid = ITEM_INDEX(item);
	const struct IER_PromoConfig *it = *pr_gpIER_PromotionItemTable;

	for (;; it++) {
		if (it->item == ITEM_NONE || it->item == 0xFFFF)
			break;

		if (it->job_list == NULL)
			break;

		LTRACEF("item=0x%02X 0x%02X, job=0x%02X", it->item, iid, it->job_list[0]);

		if (it->item == iid) {
			int i;
			int jid = UNIT_CLASS_ID(unit);

			for (i = 0; ; i++) {
				int it_jid = it->job_list[i];

				if (it_jid == CLASS_NONE)
					return false;

				if (jid == it_jid) {
					if (it->extra_check)
						return it->extra_check(unit, item);

					return true;
				}
			}
			return false;
		}
	}
	return false;
}

// PlayerPhase_PrepareAction

/**
 * Heal
 */
LYN_REPLACE_CHECK(GetUnitItemHealAmount);
int GetUnitItemHealAmount(struct Unit *unit, int item)
{
	int result = 0;

    if (gpKernelDesignerConfig->item_effect_revamp == true)
    {
		result = GetItemMight(item) + IER_BYTE(item);

		if (result == 0) {
			switch (GetItemData(ITEM_INDEX(item))->useEffectId) {
			case IER_STAFF_HEAL:
			case IER_STAFF_PHYSIC:
			case IER_STAFF_FORTIFY:
			case IER_VULNERARY:
			case IER_VULNERARY_2:
				result = 10;
				break;

			case IER_STAFF_MEND:
				result = 20;
				break;

			case IER_STAFF_RECOVER:
			case IER_ELIXIR:
				result = 80;
				break;

			default:
				break;
			}
		}
	}
	else
	{
		switch (GetItemIndex(item)) {
		case ITEM_STAFF_HEAL:
		case ITEM_STAFF_PHYSIC:
		case ITEM_STAFF_FORTIFY:
		case ITEM_VULNERARY:
		case ITEM_VULNERARY_2:
			result = 10;
			break;

		case ITEM_STAFF_MEND:
			result = 20;
			break;

		case ITEM_STAFF_RECOVER:
		case ITEM_ELIXIR:
			result = 80;
			break;
		} // switch (GetItemIndex(item))
	}

	if (GetItemAttributes(item) & IA_STAFF)
		result += MagGetter(unit);

	if (result > 80)
		result = 80;

	return result;
}

/**
 * Key
 */
LYN_REPLACE_CHECK(AiGetChestUnlockItemSlot);
bool AiGetChestUnlockItemSlot(u8 *out)
{
	int i;

	*out = 0;

	if (GetUnitItemCount(gActiveUnit) == UNIT_ITEM_COUNT) {
		gActiveUnit->aiFlags |= AI_UNIT_FLAG_3;
		return false;
	}

	for (i = 0; i < UNIT_ITEM_COUNT; i++) {
		u16 item = gActiveUnit->items[i];

		if (item == 0)
			return false;

		*out = i;

#if CHAX
		switch (GetItemData(ITEM_INDEX(item))->useEffectId) {
		case IER_LOCKPICK:
			if (UNIT_CATTRIBUTES(gActiveUnit) & CA_STEAL)
				return true;

			break;

		case IER_CHESTKEY:
		case IER_DOORKEY:
			return true;

		default:
			break;
		}
#else
		if (GetItemIndex(item) == ITEM_CHESTKEY)
			return true;

		if (GetItemIndex(item) == ITEM_LOCKPICK) {
			if (UNIT_CATTRIBUTES(gActiveUnit) & CA_STEAL)
				return true;
		}
#endif
	}

	return false;
}

LYN_REPLACE_CHECK(AiTryHealSelf);
bool AiTryHealSelf(void)
{
	int i;

	for (i = 0; i < UNIT_ITEM_COUNT; i++) {
		u16 item = gActiveUnit->items[i];

		if (item == 0)
			return 0;

		switch (GetItemData(ITEM_INDEX(item))->useEffectId) {
		case IER_VULNERARY:
		case IER_ELIXIR:
			if (!(gAiState.flags & AI_FLAG_STAY) && !(gActiveUnit->ai_config & AI_UNIT_CONFIG_FLAG_STAY)) {
				/**
				 * If unit can move around (rather than stick on position)
				 * he may try escape to a safe place then heal itself.
				 */
				struct Vec2 position;

				if (AiFindSafestReachableLocation(gActiveUnit, &position) == true) {
					AiSetDecision(position.x, position.y, AI_ACTION_USEITEM, 0, i, 0, 0);
					return true;
				}
			} else {
				AiSetDecision(gActiveUnit->xPos, gActiveUnit->yPos, AI_ACTION_USEITEM, 0, i, 0, 0);
				return true;
			}
			break;

		default:
			break;
		}
	}
	return false;
}

STATIC_DECLAR int GetStatBoosterText(struct Unit *unit, int item)
{
	int iid = ITEM_INDEX(item);
	const struct IER_PrepStatBoosterMsg *list = *pr_IER_StatBoosterTextTable;

	for (; list->item != ITEM_NONE; list++) {
		if (list->item != iid)
			continue;

		if (list->msg_getter)
			return list->msg_getter(unit, item);
		else
			return list->msg;
	}
	return 0;
}


LYN_REPLACE_CHECK(ExecStatBoostItem);
void ExecStatBoostItem(ProcPtr proc) {
    int item;
    int messageId;
    struct Unit* unit = GetUnit(gActionData.subjectIndex);

    item = unit->items[gActionData.itemSlotIndex];

    gBattleTarget.statusOut = -1;

    messageId = ApplyStatBoostItem(unit, gActionData.itemSlotIndex);

	// This if statement is for the purposes of tonic items only
	if (GetItemIndex(item) == ITEM_TONIC) {
		gUnitTonicState[unit->index] = ITEM_USES(item);
		gTonicChapterState = gPlaySt.chapterIndex;

		SetPopupUnit(unit);
		SetPopupItem(item);
		gPopupNumber = 0;
		NewPopup_Simple(sTonicUsedPopup, 0x60, 0x00, Proc_Find(gProcScr_PlayerPhase));
	}
	else
	{
    	PlaySoundEffect(SONG_SE_UPDATE);
		NewPopup2_PlanA(proc, GetItemIconId(item), GetStringFromIndex(messageId));
	}

    return;
}

LYN_REPLACE_CHECK(ApplyStatBoostItem);
int ApplyStatBoostItem(struct Unit *unit, int slot)
{
	int item = unit->items[slot];
	FORCE_DECLARE const struct ItemData *iinfo = GetItemData(ITEM_INDEX(item));
	const struct ItemStatBonuses *statBonuses = GetItemStatBonuses(item);
	int msg = GetStatBoosterText(unit, item);
	int tonicIndex = ITEM_USES(item);

    if (gpKernelDesignerConfig->item_effect_revamp == true)
    {
		if (ITEM_INDEX(item) == ITEM_TONIC) {
			gUnitTonicState[unit->index] = tonicIndex;
			gTonicChapterState = gPlaySt.chapterIndex;
			UnitUpdateUsedItem(unit, slot);
			return msg;
		}

		if (iinfo->useEffectId == IER_METISSTOME) {
			unit->state |= US_GROWTH_BOOST;
			UnitUpdateUsedItem(unit, slot);
			return msg;
		}
	}
	else
	{
		if (GetItemIndex(item) == ITEM_TONIC) {
			gUnitTonicState[unit->index] = tonicIndex;
			gTonicChapterState = gPlaySt.chapterIndex;
			UnitUpdateUsedItem(unit, slot);
			return 0;
		}

		if (GetItemIndex(item) == ITEM_METISSTOME) 
		{
			unit->state |= US_GROWTH_BOOST;
			UnitUpdateUsedItem(unit, slot);
			return 0x1D; /* Maturity increased */
		}
	}

#if (defined(SID_ShrewdPotential) && COMMON_SKILL_VALID(SID_ShrewdPotential))
	if (SkillTester(unit, SID_ShrewdPotential)) {
		if (statBonuses->hpBonus > 0) {
			unit->maxHP += SKILL_EFF0(SID_ShrewdPotential);
			unit->curHP += SKILL_EFF0(SID_ShrewdPotential);
		}
		if (statBonuses->powBonus > 0)
			unit->pow += SKILL_EFF0(SID_ShrewdPotential);
		if (ITEM_MAG_BONUS(statBonuses) > 0)
			UNIT_MAG(unit) += SKILL_EFF0(SID_ShrewdPotential);
		if (statBonuses->sklBonus > 0)
			unit->skl += SKILL_EFF0(SID_ShrewdPotential);
		if (statBonuses->spdBonus > 0)
			unit->spd += SKILL_EFF0(SID_ShrewdPotential);
		if (statBonuses->lckBonus > 0)
			unit->lck += SKILL_EFF0(SID_ShrewdPotential);
		if (statBonuses->defBonus > 0)
			unit->def += SKILL_EFF0(SID_ShrewdPotential);
		if (statBonuses->resBonus > 0)
			unit->res += SKILL_EFF0(SID_ShrewdPotential);
		if (statBonuses->conBonus > 0)
			unit->conBonus += SKILL_EFF0(SID_ShrewdPotential);
		if (statBonuses->movBonus > 0)
			unit->movBonus += SKILL_EFF0(SID_ShrewdPotential);
	}
#endif

	unit->maxHP += statBonuses->hpBonus;
	unit->curHP += statBonuses->hpBonus;
	unit->pow += statBonuses->powBonus;
	unit->skl += statBonuses->sklBonus;
	unit->spd += statBonuses->spdBonus;
	unit->def += statBonuses->defBonus;
	unit->res += statBonuses->resBonus;
	unit->lck += statBonuses->lckBonus;
	unit->movBonus += statBonuses->movBonus;
	unit->conBonus += statBonuses->conBonus;

	UNIT_MAG(unit) += ITEM_MAG_BONUS(statBonuses);

	UnitCheckStatCaps(unit);
	UnitUpdateUsedItem(unit, slot);

    if (gpKernelDesignerConfig->item_effect_revamp == true)
    {
		return msg;
	}
	else
	{
		if (statBonuses->hpBonus > 0)
			return 0x1C;
		else if (statBonuses->powBonus > 0)
			return 0x13;
		else if (ITEM_MAG_BONUS(statBonuses) > 0)
			return 0x14;
		else if (statBonuses->sklBonus > 0)
			return 0x15;
		else if (statBonuses->spdBonus > 0)
			return 0x16;
		else if (statBonuses->lckBonus > 0)
			return 0x17;
		else if (statBonuses->defBonus > 0)
			return 0x18;
		else if (statBonuses->resBonus > 0)
			return 0x19;
		else if (statBonuses->movBonus > 0)
			return 0x1A;
		else if (statBonuses->conBonus > 0)
			return 0x1B;

		return 0;
	}
}

LYN_REPLACE_CHECK(DoUseBarrierStaff);
void DoUseBarrierStaff(struct Unit* unit)
{
    MakeTargetListForBarrier(unit);

    char * str = "NULL";

	int itemId = GetItemIndex(unit->items[gActionData.itemSlotIndex]);

    switch (itemId)
    {
	case ITEM_STAFF_FORCE:
		if (!gpKernelDesignerConfig->custom_staves)
			break;
        str = "Select which character's strength to bolster";
        break;
	case ITEM_STAFF_TEMPEST:
		if (!gpKernelDesignerConfig->custom_staves)
			break;
        str = "Select which character's magic to bolster";
        break;
	case ITEM_STAFF_ACUITY:
		if (!gpKernelDesignerConfig->custom_staves)
			break;
        str = "Select which character's skill to bolster";
        break;
	case ITEM_STAFF_SPRINT:
		if (!gpKernelDesignerConfig->custom_staves)
			break;
        str = "Select which character's speed to bolster";
        break;
	case ITEM_STAFF_FORTUNE:
		if (!gpKernelDesignerConfig->custom_staves)
			break;
        str = "Select which character's luck to bolster";
        break;
	case ITEM_STAFF_IRON:
		if (!gpKernelDesignerConfig->custom_staves)
			break;
        str = "Select which character's defense to bolster";
        break;
    case ITEM_STAFF_BARRIER:
        str = "Select which character's resistance to bolster";
        break;
	case ITEM_STAFF_OMNI:
		if (!gpKernelDesignerConfig->custom_staves)
			break;
        str = "Select which character's stats to bolster";
        break;

    default:
        break;
    }

    BmMapFill(gBmMapMovement, -1);

    StartSubtitleHelp(NewTargetSelection(&gSelectInfo_Barrier), str);
}

LYN_REPLACE_CHECK(ExecBarrierStaff);
void ExecBarrierStaff(ProcPtr proc) {

    struct Unit *unit_act = GetUnit(gActionData.subjectIndex);
    struct Unit *unit_tar = GetUnit(gActionData.targetIndex);

    BattleInitItemEffect(unit_act, gActionData.itemSlotIndex);

    BattleInitItemEffectTarget(unit_tar);

	int itemId = GetItemIndex(unit_act->items[gActionData.itemSlotIndex]);

    switch (itemId)
    {
	case ITEM_STAFF_FORCE:
		if (!gpKernelDesignerConfig->custom_staves)
			break;
        unit_tar->boostType = 0;
        break;
	case ITEM_STAFF_TEMPEST:
		if (!gpKernelDesignerConfig->custom_staves)
			break;
        unit_tar->boostType = 1;
        break;
	case ITEM_STAFF_ACUITY:
		if (!gpKernelDesignerConfig->custom_staves)
			break;
        unit_tar->boostType = 2;
        break;
	case ITEM_STAFF_SPRINT:
		if (!gpKernelDesignerConfig->custom_staves)
			break;
        unit_tar->boostType = 3;
        break;
	case ITEM_STAFF_FORTUNE:
		if (!gpKernelDesignerConfig->custom_staves)
			break;
        unit_tar->boostType = 4;
        break;
	case ITEM_STAFF_IRON:
		if (!gpKernelDesignerConfig->custom_staves)
			break;
        unit_tar->boostType = 5;
        break;
    case ITEM_STAFF_BARRIER:
        unit_tar->boostType = 6;
        break;
	case ITEM_STAFF_OMNI:
		if (!gpKernelDesignerConfig->custom_staves)
			break;
        unit_tar->boostType = 7;
        break;
    
    default:
        break;
    }

    unit_tar->barrierDuration = 7;

#if defined(SID_ExplosiveBuff) && (COMMON_SKILL_VALID(SID_ExplosiveBuff))
    if (SkillTester(unit_act, SID_ExplosiveBuff))
    {
        for (int i = 0; i < ARRAY_COUNT_RANGE1x1; i++)
        {
            int _x = gActiveUnit->xPos + gVecs_1x1[i].x;
            int _y = gActiveUnit->yPos + gVecs_1x1[i].y;
    
            struct Unit * unit_adjacent = GetUnitAtPosition(_x, _y);
            if (!UNIT_IS_VALID(unit_adjacent))
                continue;
    
            if (unit_adjacent->state & (US_HIDDEN | US_DEAD | US_RESCUED | US_BIT16))
                continue;
    
            if (AreUnitsAllied(gActiveUnit->index, unit_adjacent->index) && GetUnit(unit_adjacent->index) != unit_tar)
			{
                unit_adjacent->boostType = unit_tar->boostType;
				unit_adjacent->barrierDuration = 7;
			}
        }
    }
#endif

    BattleApplyItemEffect(proc);
    BeginBattleAnimations();

    return;
}

LYN_REPLACE_CHECK(ExecMine);
void ExecMine(ProcPtr proc) {
    BattleInitItemEffect(GetUnit(gActionData.subjectIndex), gActionData.itemSlotIndex);

	AddTrap(gActionData.xOther, gActionData.yOther, TRAP_MINE, 0);

	StartMineAnim(proc, gActionData.xOther, gActionData.yOther);

    BattleApplyItemEffect(proc);
    BeginBattleAnimations(); // I need this for the EXP bar but it's causing a softlock on the prologue, still grants exp and level ups though

    gBattleTarget.statusOut = -1;

    return;
}

LYN_REPLACE_CHECK(ExecLightRune);
void ExecLightRune(ProcPtr proc) {
    BattleInitItemEffect(GetUnit(gActionData.subjectIndex),
        gActionData.itemSlotIndex);

	AddLightRune(gActionData.xOther, gActionData.yOther, TRAP_MAPSPRITE_PAL_LIGHT_RUNE);

    BattleApplyItemEffect(proc);

	if (gpKernelDesignerConfig->custom_staves == true &&
		GetItemIndex(GetUnit(gActionData.subjectIndex)->items[gActionData.itemSlotIndex]) == ITEM_STAFF_RUNE) {
		int x = gActionData.xOther * 0x10 - gBmSt.camera.x - 0x18;
		int y = gActionData.yOther * 0x10 - gBmSt.camera.y - 0x28;

		BG_SetPosition(0, -x, -y);
		BeginBattleAnimations(); // This way we can gain EXP for using a light rune staff
	} else {
		StartLightRuneAnim(proc, gActionData.xOther, gActionData.yOther);
	}

    gBattleTarget.statusOut = -1;

    return;
}

void ExecCustomStaves(ProcPtr proc) {
    struct Unit * unit_act = GetUnit(gActionData.subjectIndex);
    struct Unit * unit_tar = GetUnit(gActionData.targetIndex);

    BattleInitItemEffect(unit_act, gActionData.itemSlotIndex);

    BattleInitItemEffectTarget(unit_tar);

    BattleApplyItemEffect(proc);

	int itemId = GetItemIndex(unit_act->items[gActionData.itemSlotIndex]);

    switch (itemId)
    {   
	case ITEM_STAFF_SLOW:
        SetUnitStatus(unit_tar, NEW_UNIT_STATUS_SLOW);
        break;
	case ITEM_STAFF_FORGE:
        // IER_Effect_Forge takes care of this. Splitting out its effects when it depends on the steal menu is... hard.
        break;
	case ITEM_STAFF_POISON:
        SetUnitStatus(unit_tar, UNIT_STATUS_POISON);
        break;
	case ITEM_STAFF_DELAY:
        SetUnitStatus(unit_tar, NEW_UNIT_STATUS_DELAY);
        break;
	case ITEM_STAFF_QUICKEN:
        SetUnitStatus(unit_tar, NEW_UNIT_STATUS_QUICKEN);
        break;
	case ITEM_STAFF_HIDE:
        SetUnitStatus(unit_tar, NEW_UNIT_STATUS_HIDE);
        break;
	case ITEM_STAFF_PROVOKE:
        SetUnitStatus(unit_tar, NEW_UNIT_STATUS_DECOY);
        break;
	case ITEM_STAFF_PETRIFY:
        SetUnitStatus(unit_tar, UNIT_STATUS_PETRIFY);
        break;
	case ITEM_STAFF_ENFEEBLE:
        SetUnitStatus(unit_tar, NEW_UNIT_STATUS_ENFEEBLE);
        break;
	case ITEM_STAFF_SOOTH:
        SetUnitStatus(unit_tar, NEW_UNIT_STATUS_RENEWAL);
        break;
	case ITEM_STAFF_INVEST:
        SetUnitStatus(unit_tar, NEW_UNIT_STATUS_INVEST);
        break;

    default:
        break;
    }

    BeginBattleAnimations();
    
    return;
}