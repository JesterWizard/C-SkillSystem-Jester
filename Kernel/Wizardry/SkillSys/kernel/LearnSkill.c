#include "common-chax.h"
#include "bwl.h"
#include "kernel-lib.h"
#include "skill-system.h"
#include "constants/skills.h"
#include "constants/texts.h"

/*
 * Learned skills: the skills each character can equip from the prep screen's skill list.
 *
 * In RAM, one list for the whole army: entries (pid << 10 | sid), sorted, so a character's skills are together and a
 * lookup is a binary search. Skill ids are 10 bits (up to 0x3FF). The list used to be 256 bits per character, and a
 * skill from 0x100 up (the weapon-rank skills, among others) set a bit in a later character's list.
 *
 * In the save it is packed into the chunk the per-character bitmaps had (CONFIG_UNIT_AMT_ALLY * 0x20 bytes, in the game
 * and the suspend save), so the save layout does not change:
 *   u16 LEARNED_SKILLS_MAGIC, u16 the number of skills, then for each character with skills, a bit stream (low bits
 *   first) of the pid (6 bits), the number of its skills (10 bits) and the skill ids (10 bits each).
 * A chunk without the magic is the old bitmaps: list p, bit b is skill b for character p. Skills from 0x100 up were
 * stored as low skills of later characters, so only the lists of characters in the army are kept (a list only
 * changes for a player unit), and skills from 0x100 up are dropped: the weapon-rank ones are learned again at the next
 * check. The save's player units load before this chunk.
 */

#define LEARNED_SKILLS_MAGIC 0x4C53
#define LEARNED_PID(entry) ((entry) >> 10)
#define LEARNED_SID(entry) ((entry) & 0x3FF)
#define LEARNED_ENTRY(pid, sid) (((pid) << 10) | (sid))

extern struct LearnedSkills gLearnedSkills;

/* The packed list must fit the chunk with every character's group: header, 6 + 10 bits a character, 10 a skill */
_Static_assert((32 + 16 * (LEARNED_SKILLS_PID_MAX - 1) + 10 * LEARNED_SKILLS_MAX + 7) / 8 <= CONFIG_UNIT_AMT_ALLY * 0x20,
	       "the packed learned skills do not fit their save chunk");

STATIC_DECLAR int LearnedSkills_Find(u16 entry, bool *found)
{
	int lo = 0, hi = gLearnedSkills.count;

	while (lo < hi) {
		int mid = (lo + hi) >> 1;

		if (gLearnedSkills.entries[mid] < entry)
			lo = mid + 1;
		else
			hi = mid;
	}

	*found = lo < gLearnedSkills.count && gLearnedSkills.entries[lo] == entry;
	return lo;
}

STATIC_DECLAR bool LearnedSkills_Valid(int pid, int sid)
{
	return EQUIP_SKILL_VALID(sid) && pid > 0 && pid < LEARNED_SKILLS_PID_MAX;
}

STATIC_DECLAR void LearnedSkills_Add(int pid, int sid)
{
	u16 entry = LEARNED_ENTRY(pid, sid);
	bool found;
	int i, at;

	if (!LearnedSkills_Valid(pid, sid))
		return;

	at = LearnedSkills_Find(entry, &found);
	if (found || gLearnedSkills.count >= LEARNED_SKILLS_MAX)
		return;

	for (i = gLearnedSkills.count; i > at; i--)
		gLearnedSkills.entries[i] = gLearnedSkills.entries[i - 1];

	gLearnedSkills.entries[at] = entry;
	gLearnedSkills.count++;
}

/* GameInitHook */
void ResetUnitLearnedSkillLists(void)
{
	CpuFill16(0, &gLearnedSkills, sizeof(gLearnedSkills));
}

/* Bit streams to and from SRAM, a few bytes at a time */
struct LearnedSkillsStream {
	u8 *sram;
	u32 left;	/* bytes of the chunk not read or written yet */
	u32 bits;	/* bits waiting in acc */
	u32 acc;
	int pos;	/* writing: bytes in buf; reading: the next byte */
	int len;	/* reading: bytes in buf */
	u8 buf[32];
};

STATIC_DECLAR void LearnedStream_Flush(struct LearnedSkillsStream *st)
{
	if (st->pos == 0)
		return;

	WriteAndVerifySramFast(st->buf, st->sram, st->pos);
	st->sram += st->pos;
	st->pos = 0;
}

STATIC_DECLAR bool LearnedStream_Put(struct LearnedSkillsStream *st, u32 value, int nbits)
{
	st->acc |= value << st->bits;
	st->bits += nbits;

	while (st->bits >= 8) {
		if (st->left == 0)
			return false;

		st->buf[st->pos++] = st->acc & 0xFF;
		st->acc >>= 8;
		st->bits -= 8;
		st->left--;

		if (st->pos == sizeof(st->buf))
			LearnedStream_Flush(st);
	}
	return true;
}

STATIC_DECLAR bool LearnedStream_Get(struct LearnedSkillsStream *st, u32 *value, int nbits)
{
	while (st->bits < (u32)nbits) {
		if (st->pos == st->len) {
			int n = st->left < sizeof(st->buf) ? st->left : sizeof(st->buf);

			if (n == 0)
				return false;

			ReadSramFast(st->sram, st->buf, n);
			st->sram += n;
			st->left -= n;
			st->len = n;
			st->pos = 0;
		}

		st->acc |= st->buf[st->pos++] << st->bits;
		st->bits += 8;
	}

	*value = st->acc & ((1 << nbits) - 1);
	st->acc >>= nbits;
	st->bits -= nbits;
	return true;
}

/* SaveData */
void SaveUnitLearnedSkillLists(u8 *dst, const u32 size)
{
	struct LearnedSkillsStream st;
	int i, j;

	CpuFill16(0, &st, sizeof(st));
	st.sram = dst;
	st.left = size;

	LearnedStream_Put(&st, LEARNED_SKILLS_MAGIC, 16);
	LearnedStream_Put(&st, gLearnedSkills.count, 16);

	for (i = 0; i < gLearnedSkills.count; i = j) {
		int pid = LEARNED_PID(gLearnedSkills.entries[i]);

		for (j = i; j < gLearnedSkills.count && LEARNED_PID(gLearnedSkills.entries[j]) == pid; j++)
			;

		LearnedStream_Put(&st, pid, 6);
		LearnedStream_Put(&st, j - i, 10);

		for (; i < j; i++)
			if (!LearnedStream_Put(&st, LEARNED_SID(gLearnedSkills.entries[i]), 10))
				break;
	}

	/* The rest of the byte, then the rest of the chunk */
	LearnedStream_Put(&st, 0, 7);
	while (st.left != 0)
		LearnedStream_Put(&st, 0, 8);

	LearnedStream_Flush(&st);
}

STATIC_DECLAR bool LearnedSkills_InArmy(int pid, const u32 *army)
{
	int i;

	if (army)
		return pid < 64 && (army[pid >> 5] & (1 << (pid & 31)));

	for (i = FACTION_BLUE + 1; i < FACTION_BLUE + 1 + CONFIG_UNIT_AMT_ALLY; i++) {
		struct Unit *unit = GetUnit(i);

		if (UNIT_IS_VALID(unit) && UNIT_CHAR_ID(unit) == pid)
			return true;
	}
	return false;
}

/* A chunk written before the packed list: 32 bytes of bits per character */
STATIC_DECLAR void LoadOldLearnedSkillBitmaps(u8 *src, const u32 size, const u32 *army)
{
	u8 list[32];
	int pid, b;

	for (pid = 0; (pid + 1) * sizeof(list) <= size; pid++) {
		if (!LearnedSkills_InArmy(pid, army))
			continue;

		ReadSramFast(src + pid * sizeof(list), list, sizeof(list));

		for (b = 0; b < 0x100; b++)
			if (list[b >> 3] & (1 << (b & 7)))
				LearnedSkills_Add(pid, b);
	}
}

/* army: bit pid for the characters an old save's lists are kept for; NULL, the player units in RAM */
void LoadLearnedSkillsFor(u8 *src, const u32 size, const u32 *army)
{
	struct LearnedSkillsStream st;
	u32 magic, count, pid, n, sid, last = 0;
	int read = 0;

	ResetUnitLearnedSkillLists();

	CpuFill16(0, &st, sizeof(st));
	st.sram = src;
	st.left = size;

	if (!LearnedStream_Get(&st, &magic, 16) || magic != LEARNED_SKILLS_MAGIC) {
		LoadOldLearnedSkillBitmaps(src, size, army);
		return;
	}

	if (!LearnedStream_Get(&st, &count, 16))
		return;

	while (read < (int)count) {
		if (!LearnedStream_Get(&st, &pid, 6) || !LearnedStream_Get(&st, &n, 10) || n == 0)
			return;

		for (; n != 0; n--, read++) {
			u16 entry;

			if (!LearnedStream_Get(&st, &sid, 10))
				return;

			/* The list is sorted: anything else is not a list this code wrote */
			entry = LEARNED_ENTRY(pid, sid);
			if (entry <= last || !LearnedSkills_Valid(pid, sid) || gLearnedSkills.count >= LEARNED_SKILLS_MAX)
				return;

			gLearnedSkills.entries[gLearnedSkills.count++] = entry;
			last = entry;
		}
	}
}

/* LoadData */
void LoadUnitLearnedSkillLists(u8 *src, const u32 size)
{
	LoadLearnedSkillsFor(src, size, NULL);
}

bool IsSkillLearned(struct Unit *unit, const u16 sid)
{
	int pid = UNIT_CHAR_ID(unit);
	bool found;

	if (!LearnedSkills_Valid(pid, sid))
		return false;

	LearnedSkills_Find(LEARNED_ENTRY(pid, sid), &found);
	return found;
}

void LearnSkill(struct Unit *unit, const u16 sid)
{
	/* Make sure that the enemy is not effective on allies */
	if (UNIT_FACTION(unit) != FACTION_BLUE)
		return;

	LearnedSkills_Add(UNIT_CHAR_ID(unit), sid);
}

void ForgetSkill(struct Unit *unit, const u16 sid)
{
	int i, at, pid = UNIT_CHAR_ID(unit);
	bool found;

	if (!LearnedSkills_Valid(pid, sid))
		return;

	at = LearnedSkills_Find(LEARNED_ENTRY(pid, sid), &found);
	if (!found)
		return;

	gLearnedSkills.count--;
	for (i = at; i < gLearnedSkills.count; i++)
		gLearnedSkills.entries[i] = gLearnedSkills.entries[i + 1];

	gLearnedSkills.entries[gLearnedSkills.count] = 0;
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