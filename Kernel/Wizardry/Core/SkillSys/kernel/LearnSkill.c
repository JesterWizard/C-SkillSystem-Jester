#include "common-chax.h"
#include "bwl.h"
#include "kernel-lib.h"
#include "skill-system.h"
#include "constants/skills.h"
#include "constants/texts.h"

#define LEARNED_SKILL_COUNT 25
#define LEARNED_SKILL_LIST_SIZE 32

struct LearnedSkillList {
    u8 data[LEARNED_SKILL_LIST_SIZE];
};

static inline u16 GetLearnedSkillIndex(const struct LearnedSkillList *list, int index)
{
	int bit = index * 10;
	int byte = bit >> 3;
	int shift = bit & 0x7;
	u32 value = list->data[byte];

	value |= (u32)list->data[byte + 1] << 8;

	if (byte + 2 < LEARNED_SKILL_LIST_SIZE)
		value |= (u32)list->data[byte + 2] << 16;

	return (value >> shift) & 0x3FF;
}

static inline void SetLearnedSkillIndex(struct LearnedSkillList *list, int index, u16 sid)
{
	int bit = index * 10;
	int byte = bit >> 3;
	int shift = bit & 0x7;
	u32 value = list->data[byte];
	u32 mask = ((u32)0x3FF) << shift;

	value |= (u32)list->data[byte + 1] << 8;

	if (byte + 2 < LEARNED_SKILL_LIST_SIZE)
		value |= (u32)list->data[byte + 2] << 16;

	value = (value & ~mask) | (((u32)sid & 0x3FF) << shift);

	list->data[byte] = value & 0xFF;
	list->data[byte + 1] = (value >> 8) & 0xFF;

	if (byte + 2 < LEARNED_SKILL_LIST_SIZE)
		list->data[byte + 2] = (value >> 16) & 0xFF;
}

static int FindLearnedSkillSlot(const struct LearnedSkillList *list, u16 sid)
{
	for (int i = 0; i < LEARNED_SKILL_COUNT; i++) {
		if (GetLearnedSkillIndex(list, i) == sid)
			return i;
	}

	return -1;
}

static int FindFreeLearnedSkillSlot(const struct LearnedSkillList *list)
{
	for (int i = 0; i < LEARNED_SKILL_COUNT; i++) {
		if (!GetLearnedSkillIndex(list, i))
			return i;
	}

	return -1;
}

static void CompactLearnedSkillList(struct LearnedSkillList *list)
{
	int write = 0;

	for (int read = 0; read < LEARNED_SKILL_COUNT; read++) {
		u16 sid = GetLearnedSkillIndex(list, read);

		if (!sid)
			continue;

		if (write != read)
			SetLearnedSkillIndex(list, write, sid);

		write++;
	}

	for (int i = write; i < LEARNED_SKILL_COUNT; i++)
		SetLearnedSkillIndex(list, i, 0);
}

extern struct LearnedSkillList sLearnedSkillPLists[NEW_BWL_ARRAY_NUM];

/* GameInitHook */
void ResetUnitLearnedSkillLists(void)
{
	CpuFill16(0, sLearnedSkillPLists, sizeof(sLearnedSkillPLists));
}

/* SaveData */
void SaveUnitLearnedSkillLists(u8 *dst, const u32 size)
{
	Assert(size == sizeof(sLearnedSkillPLists));

	WriteAndVerifySramFast(sLearnedSkillPLists, dst, size);
}

/* LoadData */
void LoadUnitLearnedSkillLists(u8 *src, const u32 size)
{
	Assert(size == sizeof(sLearnedSkillPLists));

	ReadSramFast(src, sLearnedSkillPLists, size);
}

bool IsSkillLearned(struct Unit *unit, const u16 sid)
{
	u8 pid = UNIT_CHAR_ID(unit);

	if (EQUIP_SKILL_VALID(sid) && pid < NEW_BWL_ARRAY_NUM)
		return FindLearnedSkillSlot(&sLearnedSkillPLists[pid], sid) >= 0;

	return false;
}

void LearnSkill(struct Unit *unit, const u16 sid)
{
	u8 pid = UNIT_CHAR_ID(unit);

	/* Make sure that the enemy is not effective on allies */
	if (UNIT_FACTION(unit) != FACTION_BLUE)
		return;

	if (EQUIP_SKILL_VALID(sid) && pid < NEW_BWL_ARRAY_NUM) {
		struct LearnedSkillList *list = &sLearnedSkillPLists[pid];

		if (FindLearnedSkillSlot(list, sid) >= 0)
			return;

		int slot = FindFreeLearnedSkillSlot(list);
		if (slot >= 0)
			SetLearnedSkillIndex(list, slot, sid);
}
}

void ForgetSkill(struct Unit *unit, const u16 sid)
{
	u8 pid = UNIT_CHAR_ID(unit);

	if (EQUIP_SKILL_VALID(sid) && pid < NEW_BWL_ARRAY_NUM) {
		struct LearnedSkillList *list = &sLearnedSkillPLists[pid];
		int slot = FindLearnedSkillSlot(list, sid);

		if (slot < 0)
			return;

		SetLearnedSkillIndex(list, slot, 0);
		CompactLearnedSkillList(list);
}
}

const struct PopupInstruction PopupScr_ObtainedSkill[] = {
    POPUP_SOUND(0x5A),
    POPUP_COLOR(TEXT_COLOR_SYSTEM_WHITE),
    POPUP_MSG(MSG_Obtained),
    POPUP_COLOR(TEXT_COLOR_SYSTEM_GOLD),
    POPUP_ITEM_STR,
    POPUP_SPACE(1),
    POPUP_ITEM_ICON,
    POPUP_COLOR(TEXT_COLOR_SYSTEM_WHITE),
    POPUP_SPACE(1),
    POPUP_END
};