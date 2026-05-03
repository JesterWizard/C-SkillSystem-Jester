#include "common-chax.h"
#include "kernel-lib.h"
#include "jester_headers/custom-arrays.h"
#include "jester_headers/custom-structs.h"

typedef int (*KillUnitFunc_t)(struct Unit *unit);
// extern const KillUnitFunc_t gKillUnitHooks[];
extern KillUnitFunc_t const *const gpKillUnitHooks;

int GetDeadUnitCount(void)
{
    int count = 0;

    for (int i = 0; i < (int)ARRAY_COUNT(gDeadUnits); i++)
    {
        if (gDeadUnits[i] == 0x00)
            break;

        count++;
    }

    return count;
}

void AddDeadUnit(u8 unitId)
{
	struct Unit *unit = GetUnit(unitId);

	if (!UNIT_IS_VALID(unit))
		return;

	if (UNIT_FACTION(unit) != FACTION_BLUE)
		return;

	for (int i = 0; i < (int)ARRAY_COUNT(gDeadUnits); i++)
	{
		if (gDeadUnits[i] == unitId)
			return;

		if (gDeadUnits[i] != 0x00)
			continue;

		gDeadUnits[i] = unitId;
		return;
	}

	// array full -> ignore overflow
}

void RemoveDeadUnit(u8 unitId)
{
	for (int i = 0; i < (int)ARRAY_COUNT(gDeadUnits); i++)
	{
		if (gDeadUnits[i] != unitId)
			continue;

		for (int j = i; j < (int)ARRAY_COUNT(gDeadUnits) - 1; j++)
			gDeadUnits[j] = gDeadUnits[j + 1];

		gDeadUnits[ARRAY_COUNT(gDeadUnits) - 1] = 0x00;
		i--;
	}
}

u8 GetLastDeadUnit(void)
{
    for (int i = ARRAY_COUNT(gDeadUnits) - 1; i >= 0; i--)
    {
		if (gDeadUnits[i] == 0x00)
			continue;

		struct Unit *unit = GetUnit(gDeadUnits[i]);

		if (!UNIT_IS_VALID(unit))
			continue;

		if (UNIT_FACTION(unit) != FACTION_BLUE)
			continue;

		if (!(unit->state & US_DEAD))
			continue;

		return gDeadUnits[i];
    }

    return 0x00; // none stored
}

void SaveDeadUnits(u8 *dst, const u32 size)
{
	if (size < sizeof(gDeadUnits))
		return;

	WriteAndVerifySramFast(gDeadUnits, dst, sizeof(gDeadUnits));
}

void LoadDeadUnits(u8 *src, const u32 size)
{
	if (size < sizeof(gDeadUnits))
		return;

	ReadSramFast(src, gDeadUnits, sizeof(gDeadUnits));
}

LYN_REPLACE_CHECK(UnitKill);
void UnitKill(struct Unit *unit)
{
#if CHAX
	const KillUnitFunc_t *it;
	for (it = gpKillUnitHooks; *it; it++)
		(*it)(unit);
#endif

	AddDeadUnit(unit->index);

	if (gpKernelDesignerConfig->prep_menu_bexp == true)
	{
		if (UNIT_FACTION(unit) == FACTION_RED)
		{
			if (UNIT_CATTRIBUTES(unit) & CA_BOSS)
				gBEXP_MapGain += gBexpGainConstants.boss;
			else
				gBEXP_MapGain += gBexpGainConstants.normal;
		} 
	}

	if (UNIT_FACTION(unit) == FACTION_BLUE) {
		if (UNIT_IS_PHANTOM(unit))
			unit->pCharacterData = NULL;
		else 
		{	
			if (gpKernelDesignerConfig->casual_mode == true)
				unit->state |= US_HIDDEN;
			else
			{
				unit->state |= US_DEAD | US_HIDDEN;
				InitUnitsupports(unit);
			}
		}
	}
	else
	{
		if (gpKernelDesignerConfig->collect_dead_units == true)
		{
			unit->state |= US_DEAD | US_HIDDEN;
			unit->curHP = unit->maxHP;
		}
		else
			unit->pCharacterData = NULL;
	}
}

//! FE8U = 0x0803286C
LYN_REPLACE_CHECK(BATTLE_HandleCombatDeaths);
void BATTLE_HandleCombatDeaths(struct CombatActionProc* proc) {
    struct Unit* unitA = GetUnit(proc->unitIdA);
    struct Unit* unitB = GetUnit(proc->unitIdB);

    DropRescueOnDeath(proc, unitA);
    DropRescueOnDeath(proc, unitB);

    KillUnitOnCombatDeath(unitA, unitB);
    KillUnitOnCombatDeath(unitB, unitA);

    return;
}

//! FE8U = 0x080327C8
LYN_REPLACE_CHECK(BATTLE_PostCombatDeathFades);
void BATTLE_PostCombatDeathFades(struct CombatActionProc* proc) {
    struct MuProc* muProc;

    proc->unk_54 = NULL;

    if (DidUnitDie(&gBattleActor.unit)) {
        muProc = Proc_Find(ProcScr_Mu);
        MU_StartDeathFade(muProc);
        proc->unk_54 = muProc;

        TryRemoveUnitFromBallista(&gBattleActor.unit);
    }

    if (DidUnitDie(&gBattleTarget.unit)) {
        struct Unit* target = GetUnit(gBattleTarget.unit.index);
        target->state |= US_HIDDEN;

        TryRemoveUnitFromBallista(target);

        RefreshUnitSprites();
        muProc = StartMu(&gBattleTarget.unit);

        gWorkingMovementScript[0] = GetFacingDirection(gBattleActor.unit.xPos, gBattleActor.unit.yPos, gBattleTarget.unit.xPos, gBattleTarget.unit.yPos);
        gWorkingMovementScript[1] = MOVE_CMD_HALT;

        SetMuMoveScript(muProc, gWorkingMovementScript);
        MU_StartDeathFade(muProc);

        proc->unk_54 = muProc;
    }

    return;
}