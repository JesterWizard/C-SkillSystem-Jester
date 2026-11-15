#include "common-chax.h"
#include "skill-system.h"
#include "prep-skill.h"

#define sPrepEquipSkillListExt gGenericBuffer

void ResetPrepEquipSkillList(void)
{
	/*
	 * One byte per skill id, and ids go up to MAX_EQUIP_SKILL_NUM (0x3FF): clearing only 0x100 bytes left whatever
	 * gGenericBuffer last held in the rest, and those bytes showed as learned skills (every unit's Manage Skills list
	 * filled up).
	 */
	CpuFastFill16(0, sPrepEquipSkillListExt, 0x400);
	CpuFastFill16(0, &sPrepEquipSkillList,   sizeof(sPrepEquipSkillList));
}

STATIC_DECLAR void RegisterToPrepEquipSkillListExt(const u16 sid)
{
	sPrepEquipSkillListExt[sid] |= 1;
}

STATIC_DECLAR void SetupPrepEquipReal(void)
{
	int i;

	for (i = 1; i < MAX_EQUIP_SKILL_NUM && sPrepEquipSkillList.amt < 0xFF; i++)
		if (sPrepEquipSkillListExt[i] & 1)
			sPrepEquipSkillList.sid[sPrepEquipSkillList.amt++] = i;
}

STATIC_DECLAR void UpdatePrepEquipSkillList(struct Unit *unit)
{
	int i;

	TryAddSkillWRank(unit, false);

	ResetPrepEquipSkillList();

	for (i = 1; i < MAX_EQUIP_SKILL_NUM; i++)
		if (IsSkillLearned(unit, i))
			RegisterToPrepEquipSkillListExt(i);

	SetupPrepEquipReal();
}

struct PrepEquipSkillList *GetPrepEquipSkillList(struct Unit *unit)
{
	if (!JudgeUnitList(unit, &sPrepEquipSkillList.header)) {
		UpdatePrepEquipSkillList(unit);
		WriteUnitList(unit, &sPrepEquipSkillList.header);
	}
	return &sPrepEquipSkillList;
}
