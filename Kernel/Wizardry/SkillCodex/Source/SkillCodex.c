#include "common-chax.h"
#include "kernel-lib.h"
#include "bmsave.h"
#include "save-data.h"
#include "skill-system.h"
#include "skill-codex.h"

/*
 * The Skill Codex's unlocks (Documentation/Features/SkillCodex.md): a bit per skill id met.
 *
 * Kept as the Bestiary's are (Kernel/Wizardry/Bestiary/Source/Bestiary.c): in RAM while playing (gSkillCodexSeen),
 * and in each save's tail, after the Bestiary's share (SaveTail.c). Loading or starting a game takes in every other
 * save's bits too, and the title's screen shows what all the saves hold together.
 *
 * At each phase start the skills of every unit on the map the player can see are marked (personal, class, equipped,
 * item skills: the unit's skill list), and every skill the army has learned.
 */

extern struct LearnedSkills gLearnedSkills;

_Static_assert(sizeof(struct SkillCodexBits) == 0x84, "gSkillCodexSeen is 0x84 bytes (config-memmap.s)");

/* A skill the codex lists: a valid id with a description */
bool SkillCodex_Listed(int sid)
{
	return sid > 0 && sid < SKILL_CODEX_SIDS - 1 && COMMON_SKILL_VALID(sid) && GetSkillDescMsg(sid) != 0;
}

bool SkillCodex_BitSet(const u8 *bits, int sid)
{
	return (unsigned int)sid < SKILL_CODEX_SIDS && (bits[sid >> 3] & (1 << (sid & 7)));
}

static void SkillCodex_Clear(struct SkillCodexBits *bits)
{
	CpuFill16(0, bits, sizeof(*bits));
	bits->magic = SKILL_CODEX_MAGIC;
}

static void SkillCodex_Or(struct SkillCodexBits *into, const struct SkillCodexBits *from)
{
	int i;

	for (i = 0; i < (int)sizeof(from->skills); i++)
		into->skills[i] |= from->skills[i];
}

/* OR into `into` a block's bits, if the block is valid and has them */
static void SkillCodex_TakeBlock(struct SkillCodexBits *into, int id)
{
	struct SaveBlockInfo info;
	struct SkillCodexBits buf;

	if (!ReadSaveBlockInfo(&info, id))
		return;

	ReadSramFast((const u8 *)GetSaveReadAddr(id) + SaveTail_SkillCodexOffset(id == SAVE_ID_SUSPEND), &buf,
		     sizeof(buf));

	if (buf.magic == SKILL_CODEX_MAGIC)
		SkillCodex_Or(into, &buf);
}

/* Every save's bits together: the three game saves and the suspend save */
void SkillCodex_ReadCart(struct SkillCodexBits *bits)
{
	int id;

	SkillCodex_Clear(bits);

	if (!IsSramWorking())
		return;

	for (id = SAVE_ID_GAME0; id <= SAVE_ID_GAME2; id++)
		SkillCodex_TakeBlock(bits, id);

	SkillCodex_TakeBlock(bits, SAVE_ID_SUSPEND);
}

/* gNewSaveHooks: a new game starts with what the other saves have met */
void SkillCodex_OnNewGame(void)
{
	SkillCodex_ReadCart(&gSkillCodexSeen);
}

/* The save tail (SaveTail.c): this game's bits */
void SkillCodex_SaveTail(u8 *dst, const u32 size)
{
	if (size < sizeof(gSkillCodexSeen))
		return;

	if (gSkillCodexSeen.magic != SKILL_CODEX_MAGIC)
		SkillCodex_Clear(&gSkillCodexSeen);

	WriteAndVerifySramFast(&gSkillCodexSeen, dst, sizeof(gSkillCodexSeen));
}

/* The save tail, on loading a game or a suspend: the save's bits and every other save's */
void SkillCodex_LoadTail(u8 *src, const u32 size)
{
	struct SkillCodexBits buf;

	SkillCodex_ReadCart(&gSkillCodexSeen);

	if (src == NULL || size < sizeof(buf))
		return;

	ReadSramFast(src, &buf, sizeof(buf));

	if (buf.magic == SKILL_CODEX_MAGIC)
		SkillCodex_Or(&gSkillCodexSeen, &buf);
}

/* A skill met, outside the phase sweep (an event, an ASMC, a scroll) */
void SkillCodex_MarkSeen(int sid)
{
	if (gSkillCodexSeen.magic != SKILL_CODEX_MAGIC)
		SkillCodex_Clear(&gSkillCodexSeen);

	if ((unsigned int)sid < SKILL_CODEX_SIDS)
		gSkillCodexSeen.skills[sid >> 3] |= 1 << (sid & 7);
}

/* A unit the player can see: on the map, alive, and not in fog (the player's own always) */
static bool SkillCodex_UnitSeen(struct Unit *unit)
{
	if (!UNIT_IS_VALID(unit) || !unit->pClassData)
		return false;

	if (unit->state & (US_HIDDEN | US_UNAVAILABLE))
		return false;

	if (gPlaySt.chapterVisionRange != 0 && UNIT_FACTION(unit) != FACTION_BLUE &&
	    gBmMapFog[unit->yPos][unit->xPos] == 0)
		return false;

	return true;
}

/* gPrePhaseFuncs: the skills of every unit the player can see, and the army's learned skills (RAM only) */
bool SkillCodex_PrePhase(ProcPtr proc)
{
	struct SkillList list;
	int i, j;

	for (i = 1; i < 0xC0; i++) {
		struct Unit *unit = GetUnit(i);

		if (!SkillCodex_UnitSeen(unit))
			continue;

		GenerateSkillListExt(unit, &list);
		for (j = 0; j < list.amt && j < (int)ARRAY_COUNT(list.sid); j++)
			SkillCodex_MarkSeen(list.sid[j]);
	}

	for (i = 0; i < gLearnedSkills.count && i < LEARNED_SKILLS_MAX; i++)
		SkillCodex_MarkSeen(gLearnedSkills.entries[i] & 0x3FF);

	return false;
}
