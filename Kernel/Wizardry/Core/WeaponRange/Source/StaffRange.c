#include "common-chax.h"
#include "weapon-range.h"
#include "debuff.h"
#include "strmag.h"
#include "skill-system.h"
#include "constants/skills.h"
#include "jester_headers/custom-functions.h"
#include "constants/texts.h"

static bool CustomStavesEnabled(void)
{
    return gpKernelDesignerConfig->custom_staves == true;
}

LYN_REPLACE_CHECK(TryAddUnitToHealTargetList);
void TryAddUnitToHealTargetList(struct Unit* unit) {

    if (!AreUnitsAllied(gSubjectUnit->index, unit->index)) {

#if defined(SID_Saint) && (COMMON_SKILL_VALID(SID_Saint))
        if (!SkillTester(gSubjectUnit, SID_Saint))
             return;
#else 
        return;
#endif

    }

#if defined(SID_AidRefusal) && (COMMON_SKILL_VALID(SID_AidRefusal))
    if (SkillTester(unit, SID_AidRefusal))
        return;
#endif

    if (unit->state & US_RESCUED) {
        return;
    }

#if defined(SID_CursedHeal) && (COMMON_SKILL_VALID(SID_CursedHeal))
    if (!SkillTester(gSubjectUnit, SID_CursedHeal))
        if (GetUnitCurrentHp(unit) == GetUnitMaxHp(unit)) {
            return;
        }  
#else 
    if (GetUnitCurrentHp(unit) == GetUnitMaxHp(unit)) {
        return;
    }
#endif

    AddTarget(unit->xPos, unit->yPos, unit->index, 0);

    return;
}

LYN_REPLACE_CHECK(MakeTargetListForAdjacentHeal);
void MakeTargetListForAdjacentHeal(struct Unit *unit)
{
	gSubjectUnit = unit;
	InitTargets(unit->xPos, unit->yPos);

	BmMapFill(gBmMapRange, 0);
	AddMapForItem(unit, ITEM_STAFF_HEAL);
	ForEachUnit(TryAddUnitToHealTargetList, gBmMapRange, 0);

#if defined(SID_SelfHealing) && (COMMON_SKILL_VALID(SID_SelfHealing))
    if (SkillTester(unit, SID_SelfHealing))
         AddTarget(unit->xPos, unit->yPos, unit->index, 0);
#endif
}

LYN_REPLACE_CHECK(MakeTargetListForRangedHeal);
void MakeTargetListForRangedHeal(struct Unit *unit)
{
	gSubjectUnit = unit;
	InitTargets(unit->xPos, unit->yPos);

	BmMapFill(gBmMapRange, 0);
	AddMapForItem(unit, ITEM_STAFF_PHYSIC);
	ForEachUnit(TryAddUnitToHealTargetList, gBmMapRange, 0);

#if defined(SID_SelfHealing) && (COMMON_SKILL_VALID(SID_SelfHealing))
    if (SkillTester(unit, SID_SelfHealing))
         AddTarget(unit->xPos, unit->yPos, unit->index, 0);
#endif
}

LYN_REPLACE_CHECK(MakeTargetListForRestore);
void MakeTargetListForRestore(struct Unit *unit)
{
	gSubjectUnit = unit;
	InitTargets(unit->xPos, unit->yPos);

	BmMapFill(gBmMapRange, 0);
	AddMapForItem(unit, ITEM_STAFF_RESTORE);
	ForEachUnit(TryAddUnitToRestoreTargetList, gBmMapRange, 0);
}

LYN_REPLACE_CHECK(MakeTargetListForBarrier);
void MakeTargetListForBarrier(struct Unit *unit)
{
	gSubjectUnit = unit;
	InitTargets(unit->xPos, unit->yPos);

	BmMapFill(gBmMapRange, 0);
	AddMapForItem(unit, ITEM_STAFF_BARRIER);
	ForEachUnit(TryAddUnitToBarrierTargetList, gBmMapRange, 0);
}

LYN_REPLACE_CHECK(MakeTargetListForRescueStaff);
void MakeTargetListForRescueStaff(struct Unit *unit)
{
	gSubjectUnit = unit;
	InitTargets(unit->xPos, unit->yPos);

	BmMapFill(gBmMapRange, 0);
	AddMapForItem(unit, ITEM_STAFF_RESCUE);
	ForEachUnit(TryAddUnitToRescueStaffTargetList, gBmMapRange, 0);
}

LYN_REPLACE_CHECK(MakeTargetListForSilence);
void MakeTargetListForSilence(struct Unit *unit)
{
	gSubjectUnit = unit;
	InitTargets(unit->xPos, unit->yPos);

	BmMapFill(gBmMapRange, 0);
	AddMapForItem(unit, ITEM_STAFF_SILENCE);
	ForEachUnit(TryAddUnitToSilenceTargetList, gBmMapRange, 0);
}

LYN_REPLACE_CHECK(MakeTargetListForSleep);
void MakeTargetListForSleep(struct Unit *unit)
{
	gSubjectUnit = unit;
	InitTargets(unit->xPos, unit->yPos);

	BmMapFill(gBmMapRange, 0);
	AddMapForItem(unit, ITEM_STAFF_SLEEP);
	ForEachUnit(TryAddUnitToSleepTargetList, gBmMapRange, 0);
}

LYN_REPLACE_CHECK(MakeTargetListForBerserk);
void MakeTargetListForBerserk(struct Unit *unit)
{
	gSubjectUnit = unit;
	InitTargets(unit->xPos, unit->yPos);

	BmMapFill(gBmMapRange, 0);
	AddMapForItem(unit, ITEM_STAFF_BERSERK);
	ForEachUnit(TryAddUnitToBerserkTargetList, gBmMapRange, 0);
}

LYN_REPLACE_CHECK(MakeTargetListForWarp);
void MakeTargetListForWarp(struct Unit *unit)
{
	gSubjectUnit = unit;
	InitTargets(unit->xPos, unit->yPos);

	BmMapFill(gBmMapRange, 0);
	AddMapForItem(unit, ITEM_STAFF_WARP);
	ForEachUnit(TryAddUnitToWarpTargetList, gBmMapRange, 0);
}

LYN_REPLACE_CHECK(MakeTargetListForHammerne);
void MakeTargetListForHammerne(struct Unit *unit)
{
	gSubjectUnit = unit;
	InitTargets(unit->xPos, unit->yPos);

	BmMapFill(gBmMapRange, 0);
	AddMapForItem(unit, ITEM_STAFF_REPAIR);
	ForEachUnit(TryAddUnitToHammerneTargetList, gBmMapRange, 0);
}

void TryAddUnitToSlowTargetList(struct Unit* unit) 
{
    if (AreUnitsAllied(gSubjectUnit->index, unit->index)) {
        return;
    }

    if (unit->statusIndex != UNIT_STATUS_NONE) {
        return;
    }

    AddTarget(unit->xPos, unit->yPos, unit->index, 0);

    return;
}

void MakeTargetListForSlow(struct Unit *unit)
{
	int x = unit->xPos;
    int y = unit->yPos;
	gSubjectUnit = unit;
	InitTargets(x, y);

    if (!CustomStavesEnabled()) {
        return;
    }

	BmMapFill(gBmMapRange, 0);
    AddMapForItem(unit, ITEM_STAFF_SLOW);
	ForEachAdjacentUnit(x, y, TryAddUnitToSlowTargetList);
}

void TryAddUnitToForgeTargetList(struct Unit* unit) 
{	
    if (AreUnitsAllied(gSubjectUnit->index, unit->index)) {
        return;
    }

	if (gSubjectUnit->items[4] != 0) {
		return;
	}

    if (unit->res >= GetUnitMagic(gSubjectUnit)) {
        return;
    }

	for (int i = 0; i < UNIT_ITEM_COUNT; i++)
	{
		if (unit->items[i] == 0)
			return;
		
		if (GetItemAttributes(unit->items[i]) == IA_WEAPON)
			break;
	}

    AddTarget(unit->xPos, unit->yPos, unit->index, 0);

    return;
}

void MakeTargetListForForge(struct Unit *unit)
{
	int x = unit->xPos;
    int y = unit->yPos;
	gSubjectUnit = unit;
	InitTargets(x, y);

    if (!CustomStavesEnabled()) {
        return;
    }

	BmMapFill(gBmMapRange, 0);
    AddMapForItem(unit, ITEM_STAFF_FORGE);
	ForEachUnitInMagBy2Range(TryAddUnitToForgeTargetList);
}

void TryAddUnitToPoisonTargetList(struct Unit* unit) 
{
    if (AreUnitsAllied(gSubjectUnit->index, unit->index)) {
        return;
    }

    if (unit->statusIndex != UNIT_STATUS_NONE) {
        return;
    }

    AddTarget(unit->xPos, unit->yPos, unit->index, 0);

    return;
}

void MakeTargetListForPoison(struct Unit *unit)
{
	int x = unit->xPos;
    int y = unit->yPos;
	gSubjectUnit = unit;
	InitTargets(x, y);

    if (!CustomStavesEnabled()) {
        return;
    }

	BmMapFill(gBmMapRange, 0);
    AddMapForItem(unit, ITEM_STAFF_POISON);
	ForEachAdjacentUnit(x, y, TryAddUnitToPoisonTargetList);
}

void TryAddUnitToDelayTargetList(struct Unit* unit) 
{
    if (AreUnitsAllied(gSubjectUnit->index, unit->index)) {
        return;
    }

    if (unit->statusIndex != UNIT_STATUS_NONE) {
        return;
    }

    AddTarget(unit->xPos, unit->yPos, unit->index, 0);

    return;
}

void MakeTargetListForDelay(struct Unit *unit)
{
	int x = unit->xPos;
    int y = unit->yPos;
	gSubjectUnit = unit;
	InitTargets(x, y);

    if (!CustomStavesEnabled()) {
        return;
    }

	BmMapFill(gBmMapRange, 0);
    AddMapForItem(unit, ITEM_STAFF_DELAY);
	ForEachAdjacentUnit(x, y, TryAddUnitToDelayTargetList);
}

void TryAddUnitToEntrapTargetList(struct Unit* unit) 
{
    if (AreUnitsAllied(gSubjectUnit->index, unit->index)) {
        return;
    }

    AddTarget(unit->xPos, unit->yPos, unit->index, 0);
    return;
}

void MakeTargetListForEntrap(struct Unit *unit)
{
	int x = unit->xPos;
    int y = unit->yPos;
	gSubjectUnit = unit;
	InitTargets(x, y);

    if (!CustomStavesEnabled()) {
        return;
    }

	BmMapFill(gBmMapRange, 0);
    AddMapForItem(unit, ITEM_STAFF_ENTRAP);
	ForEachUnit(TryAddUnitToEntrapTargetList, gBmMapRange, 0);
}

void DoUseEntrapStaff(struct Unit* unit, void(*func)(struct Unit*))
{
    func(unit);

    BmMapFill(gBmMapMovement, -1);

    StartSubtitleHelp(
        NewTargetSelection_Specialized(&gSelectInfo_WarpUnit, StaffSelectOnSelect),
        GetStringFromIndex(MSG_ITEM_ENTRAP_STAFF_SUBTITLE)); // TODO: msgid "Select which character to bring next to you."
}

void TryAddUnitToQuickenTargetList(struct Unit* unit) 
{
    if (!AreUnitsAllied(gSubjectUnit->index, unit->index)) {
        return;
    }

    if (unit->statusIndex != UNIT_STATUS_NONE) {
        return;
    }

    AddTarget(unit->xPos, unit->yPos, unit->index, 0);
    return;
}

void MakeTargetListForQuicken(struct Unit *unit)
{
	int x = unit->xPos;
    int y = unit->yPos;
	gSubjectUnit = unit;
	InitTargets(x, y);

    if (!CustomStavesEnabled()) {
        return;
    }

	BmMapFill(gBmMapRange, 0);
    AddMapForItem(unit, ITEM_STAFF_QUICKEN);
	ForEachAdjacentUnit(x, y, TryAddUnitToQuickenTargetList);
}

void TryAddUnitToHideTargetList(struct Unit* unit) 
{
    if (!AreUnitsAllied(gSubjectUnit->index, unit->index)) {
        return;
    }

    if (unit->statusIndex != UNIT_STATUS_NONE) {
        return;
    }

    AddTarget(unit->xPos, unit->yPos, unit->index, 0);
    return;
}

void MakeTargetListForHide(struct Unit *unit)
{
	int x = unit->xPos;
    int y = unit->yPos;
	gSubjectUnit = unit;
	InitTargets(x, y);

    if (!CustomStavesEnabled()) {
        return;
    }

	BmMapFill(gBmMapRange, 0);
    AddMapForItem(unit, ITEM_STAFF_HIDE);
	ForEachAdjacentUnit(x, y, TryAddUnitToHideTargetList);
}

void TryAddUnitToProvokeTargetList(struct Unit* unit) 
{
    if (!AreUnitsAllied(gSubjectUnit->index, unit->index)) {
        return;
    }

    if (unit->statusIndex != UNIT_STATUS_NONE) {
        return;
    }

    AddTarget(unit->xPos, unit->yPos, unit->index, 0);
    return;
}

void MakeTargetListForProvoke(struct Unit *unit)
{
	int x = unit->xPos;
    int y = unit->yPos;
	gSubjectUnit = unit;
	InitTargets(x, y);

    if (!CustomStavesEnabled()) {
        return;
    }

	BmMapFill(gBmMapRange, 0);
    AddMapForItem(unit, ITEM_STAFF_PROVOKE);
	ForEachAdjacentUnit(x, y, TryAddUnitToProvokeTargetList);
}

void TryAddUnitToPetrifyTargetList(struct Unit* unit) 
{
    if (AreUnitsAllied(gSubjectUnit->index, unit->index)) {
        return;
    }

    if (unit->statusIndex != UNIT_STATUS_NONE) {
        return;
    }

    AddTarget(unit->xPos, unit->yPos, unit->index, 0);
    return;
}

void MakeTargetListForPetrify(struct Unit *unit)
{
	int x = unit->xPos;
    int y = unit->yPos;
	gSubjectUnit = unit;
	InitTargets(x, y);

    if (!CustomStavesEnabled()) {
        return;
    }

	BmMapFill(gBmMapRange, 0);
    AddMapForItem(unit, ITEM_STAFF_PETRIFY);
	ForEachAdjacentUnit(x, y, TryAddUnitToPetrifyTargetList);
}

void TryAddUnitToSoothTargetList(struct Unit* unit) 
{
    if (!AreUnitsAllied(gSubjectUnit->index, unit->index)) {
        return;
    }

    if (unit->statusIndex != UNIT_STATUS_NONE) {
        return;
    }

    AddTarget(unit->xPos, unit->yPos, unit->index, 0);
    return;
}

void MakeTargetListForSooth(struct Unit *unit)
{
	int x = unit->xPos;
    int y = unit->yPos;
	gSubjectUnit = unit;
	InitTargets(x, y);

    if (!CustomStavesEnabled()) {
        return;
    }

	BmMapFill(gBmMapRange, 0);
    AddMapForItem(unit, ITEM_STAFF_SOOTH);
	ForEachAdjacentUnit(x, y, TryAddUnitToSoothTargetList);
}

void TryAddUnitToEnfeebleTargetList(struct Unit* unit) 
{
    if (AreUnitsAllied(gSubjectUnit->index, unit->index)) {
        return;
    }

    if (unit->statusIndex != UNIT_STATUS_NONE) {
        return;
    }

    AddTarget(unit->xPos, unit->yPos, unit->index, 0);
    return;
}

void MakeTargetListForEnfeeble(struct Unit *unit)
{
	int x = unit->xPos;
    int y = unit->yPos;
	gSubjectUnit = unit;
	InitTargets(x, y);

    if (!CustomStavesEnabled()) {
        return;
    }

	BmMapFill(gBmMapRange, 0);
    AddMapForItem(unit, ITEM_STAFF_ENFEEBLE);
	ForEachAdjacentUnit(x, y, TryAddUnitToEnfeebleTargetList);
}

void TryAddUnitToInvestTargetList(struct Unit* unit) 
{
    if (!AreUnitsAllied(gSubjectUnit->index, unit->index)) {
        return;
    }

    if (unit->statusIndex != UNIT_STATUS_NONE) {
        return;
    }

    AddTarget(unit->xPos, unit->yPos, unit->index, 0);
    return;
}

void MakeTargetListForInvest(struct Unit *unit)
{
	int x = unit->xPos;
    int y = unit->yPos;
	gSubjectUnit = unit;
	InitTargets(x, y);

    if (!CustomStavesEnabled()) {
        return;
    }

	BmMapFill(gBmMapRange, 0);
    AddMapForItem(unit, ITEM_STAFF_INVEST);
	ForEachAdjacentUnit(x, y, TryAddUnitToInvestTargetList);
}