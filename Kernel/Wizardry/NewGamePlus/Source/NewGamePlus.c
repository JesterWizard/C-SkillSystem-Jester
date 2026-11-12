#include "common-chax.h"
#include "kernel-lib.h"
#include "skill-system.h"
#include "save-data.h"
#include "new-game-plus.h"

/*
 * New Game+ (Documentation/Features/NewGamePlus.md): a new game seeded from a save that has been to the Creature
 * Campaign (the postgame). At the new game, every character's skill list (the learned skills) is copied from that
 * save, and the skills each character had equipped there are kept for them. A character gets those when they first
 * join the army: loaded as a player unit, or recruited (they turn blue).
 *
 * The carried skills live in the bytes after the save tables (gpMsa_Saver, gpMsu_Saver), so the save layout and its
 * signature do not change and older saves still load. The game save holds all of it; the suspend save holds only
 * who has not joined yet, and takes the skills from the game save it belongs to.
 */

#define NGP_ARMED_MAGIC 0x4E470000

/* The offset of the bytes after gEmsSavChunks in a game save slot */
static u32 NewGamePlus_SavTailOffset(void)
{
	const struct EmsChunk *cur;
	u32 offset = 0;

	for (cur = gEmsSavChunks; cur->_identifier_ != EMS_CHUNK_INVALID_OFFSET; cur++)
		offset += cur->size;

	return offset;
}

static bool NewGamePlus_IsActive(void)
{
	return gNewGamePlus.magic == NGP_MAGIC && gNewGamePlus.active &&
	       gNewGamePlus.playthrough == gPlaySt.playthroughIdentifier;
}

static void NewGamePlus_Clear(void)
{
	CpuFill16(0, &gNewGamePlus, sizeof(gNewGamePlus));
}

bool NewGamePlus_SlotReachedPostgame(int slot)
{
	struct PlaySt playSt;

	if (slot < 0 || slot > 2 || !IsSaveValid(slot))
		return false;

	ReadGameSavePlaySt(slot, &playSt);
	return (playSt.chapterStateBits & PLAY_FLAG_POSTGAME) != 0;
}

bool NewGamePlus_IsExtraEnabled(void)
{
	int slot;

	for (slot = 0; slot < 3; slot++)
		if (NewGamePlus_SlotReachedPostgame(slot))
			return true;

	return false;
}

void NewGamePlus_Arm(int slot)
{
	gNewGamePlusArmed = NGP_ARMED_MAGIC | (slot & 0xFF);
}

void NewGamePlus_Disarm(void)
{
	gNewGamePlusArmed = 0;
}

static int NewGamePlus_ArmedSlot(void)
{
	if ((gNewGamePlusArmed & 0xFFFF0000) != NGP_ARMED_MAGIC)
		return -1;

	return gNewGamePlusArmed & 0xFF;
}

/*
 * The source save's skill lists, straight from SRAM: the new game's are empty at this point. A save from before the
 * packed list keeps the lists of the characters in its army (NewGamePlus_ReadEquips's carried bits).
 */
static void NewGamePlus_ReadSkillLists(const u8 *base)
{
	const struct EmsChunk *cur;
	u32 offset = 0;

	for (cur = gEmsSavChunks; cur->_identifier_ != EMS_CHUNK_INVALID_OFFSET; cur++) {
		if (cur->load == LoadUnitLearnedSkillLists) {
			LoadLearnedSkillsFor((u8 *)base + offset, cur->size, gNewGamePlus.carried);
			return;
		}
		offset += cur->size;
	}
}

/* The source save's player units: each character's equipped skills */
static void NewGamePlus_ReadEquips(const u8 *base)
{
	const struct EmsChunk *chunk = GetEmsChunkByIndex_Sav(EMS_CHUNK_SAVEUNITS);
	struct EmsPackedSavUnit pack;
	const u8 *src;
	u32 i;

	if (chunk == NULL)
		return;

	src = base + CalcChunkOffset_Sav(chunk);

	for (i = 0; i + SIZE_OF_SAV_UNIT_PACK <= chunk->size; i += SIZE_OF_SAV_UNIT_PACK) {
		ReadSramFast(src + i, &pack, sizeof(pack));

		if (pack.jid == 0 || pack.pid == 0 || pack.pid >= NGP_PID_MAX)
			continue;

		memcpy(gNewGamePlus.equips[pack.pid], pack.skills, sizeof(pack.skills));
		_BIT_SET(gNewGamePlus.carried, pack.pid);
		_BIT_SET(gNewGamePlus.pending, pack.pid);
	}
}

/* gNewSaveHooks, after ResetUnitLearnedSkillLists: a plain new game carries nothing */
void NewGamePlus_OnNewGame(void)
{
	int slot = NewGamePlus_ArmedSlot();
	const u8 *base;

	NewGamePlus_Disarm();
	NewGamePlus_Clear();

	if (!NewGamePlus_SlotReachedPostgame(slot))
		return;

	base = GetSaveReadAddr(slot);
	NewGamePlus_ReadEquips(base);
	NewGamePlus_ReadSkillLists(base);

	gNewGamePlus.magic = NGP_MAGIC;
	gNewGamePlus.active = true;
	gNewGamePlus.playthrough = gPlaySt.playthroughIdentifier;
}

/* A character joins: they get the skills they had equipped in the source save, once */
static void NewGamePlus_TryApply(struct Unit *unit)
{
	int i, pid;

	if (!UNIT_IS_VALID(unit) || UNIT_FACTION(unit) != FACTION_BLUE)
		return;

	pid = UNIT_CHAR_ID(unit);
	if (pid >= NGP_PID_MAX || !_BIT_CHK(gNewGamePlus.pending, pid))
		return;

	_BIT_CLR(gNewGamePlus.pending, pid);

	for (i = 0; i < UNIT_SUPPORT_MAX_COUNT; i++)
		unit->supports[i] = gNewGamePlus.equips[pid][i];

	for (i = 0; i < UNIT_RAM_SKILLS_LEN; i++) {
		int sid = GET_SKILL(unit, i);

		if (EQUIP_SKILL_VALID(sid))
			LearnSkill(unit, sid);
	}

	ResetSkillLists();
}

/* gLoadUnitHooks, after UnitAutoLoadSkills: a character loaded as a player unit */
int NewGamePlus_OnLoadUnit(struct Unit *unit, const struct CharacterData *character)
{
	if (NewGamePlus_IsActive())
		NewGamePlus_TryApply(unit);

	return 0;
}

/*
 * gClearUnitHooks: a player unit taken off the map before the character has really joined, as the opening scene does
 * with Eirika and Seth, gets the skills again at the next load. A loaded save sets who has joined after its units.
 */
int NewGamePlus_OnClearUnit(struct Unit *unit)
{
	int i, pid;

	if (!NewGamePlus_IsActive() || !UNIT_IS_VALID(unit) || UNIT_FACTION(unit) != FACTION_BLUE)
		return 0;

	pid = UNIT_CHAR_ID(unit);
	if (pid >= NGP_PID_MAX || !_BIT_CHK(gNewGamePlus.carried, pid))
		return 0;

	/* Not while another unit is the character (a Replicate copy and its original) */
	for (i = FACTION_BLUE + 1; i < FACTION_BLUE + 1 + CONFIG_UNIT_AMT_ALLY; i++) {
		struct Unit *other = GetUnit(i);

		if (other != unit && UNIT_IS_VALID(other) && UNIT_CHAR_ID(other) == pid)
			return 0;
	}

	_BIT_SET(gNewGamePlus.pending, pid);
	return 0;
}

/* Recruits turn blue through UnitChangeFaction (Talk, Persuade, events): pick them up after the action or phase */
static void NewGamePlus_SweepArmy(void)
{
	int i;

	if (!NewGamePlus_IsActive())
		return;

	for (i = FACTION_BLUE + 1; i < FACTION_BLUE + 1 + CONFIG_UNIT_AMT_ALLY; i++)
		NewGamePlus_TryApply(GetUnit(i));
}

bool NewGamePlus_PrePhase(ProcPtr proc)
{
	NewGamePlus_SweepArmy();
	return false;
}

bool NewGamePlus_PostAction(ProcPtr proc)
{
	NewGamePlus_SweepArmy();
	return false;
}

/* gpMsa_Saver: the bytes after gEmsSavChunks */
void NewGamePlus_SaveSav(u8 *dst, const u32 size)
{
	if (size < sizeof(gNewGamePlus))
		return;

	if (!NewGamePlus_IsActive())
		NewGamePlus_Clear();

	WriteAndVerifySramFast(&gNewGamePlus, dst, sizeof(gNewGamePlus));
}

/* gpMsa_Loader. A save from before New Game+ has no magic there: it carries nothing */
void NewGamePlus_LoadSav(u8 *src, const u32 size)
{
	if (size < sizeof(gNewGamePlus)) {
		NewGamePlus_Clear();
		return;
	}

	ReadSramFast(src, &gNewGamePlus, sizeof(gNewGamePlus));

	if (gNewGamePlus.magic != NGP_MAGIC)
		NewGamePlus_Clear();
}

/* gpMsu_Saver: who has not joined yet */
void NewGamePlus_SaveSus(u8 *dst, const u32 size)
{
	struct NewGamePlusSusCarry sus;

	if (size < sizeof(sus))
		return;

	CpuFill16(0, &sus, sizeof(sus));

	if (NewGamePlus_IsActive()) {
		sus.magic = NGP_MAGIC;
		sus.playthrough = gNewGamePlus.playthrough;
		sus.active = true;
		sus.pending[0] = gNewGamePlus.pending[0];
		sus.pending[1] = gNewGamePlus.pending[1];
	}

	WriteAndVerifySramFast(&sus, dst, sizeof(sus));
}

/* gpMsu_Loader, after the play state: the skills come from this game's save slot */
void NewGamePlus_LoadSus(u8 *src, const u32 size)
{
	struct NewGamePlusSusCarry sus;
	int slot = gPlaySt.gameSaveSlot;

	NewGamePlus_Clear();

	if (size < sizeof(sus))
		return;

	ReadSramFast(src, &sus, sizeof(sus));

	if (sus.magic != NGP_MAGIC || !sus.active || slot > 2)
		return;

	ReadSramFast((const u8 *)GetSaveReadAddr(slot) + NewGamePlus_SavTailOffset(), &gNewGamePlus, sizeof(gNewGamePlus));

	if (gNewGamePlus.magic != NGP_MAGIC || !gNewGamePlus.active || gNewGamePlus.playthrough != sus.playthrough) {
		NewGamePlus_Clear();
		return;
	}

	gNewGamePlus.pending[0] = sus.pending[0];
	gNewGamePlus.pending[1] = sus.pending[1];
}
