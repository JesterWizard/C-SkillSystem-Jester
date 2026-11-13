#include "common-chax.h"
#include "kernel-lib.h"
#include "bmsave.h"
#include "save-data.h"
#include "bestiary.h"

/*
 * The Bestiary's unlocks (Documentation/Features/Bestiary.md): a bit per character id and per class id met.
 *
 * Every byte of SRAM has an owner (the save blocks, then the relocated vanilla records from 0x7DEC to the end), so the
 * bits are kept in each save: in RAM while playing (gBestiarySeen), and in the bytes after the save tables of the game
 * save and the suspend save (SaveTail.c), behind a magic word. They are not a chunk: the save layout and its signature
 * do not change.
 *
 * They act as one record for the cartridge: whenever a game is loaded or started, it takes in every other save's bits
 * too, and the title's screen shows what all the saves hold together. A file erased loses only what no other save has.
 *
 * At each phase start every unit on the map, not hidden in fog, is marked: its character and its class.
 */

_Static_assert(sizeof(struct BestiaryBits) == 0x44, "gBestiarySeen is 0x44 bytes (config-memmap.s)");

bool Bestiary_BitSet(const u8 *bits, int id)
{
	return (unsigned int)id < 0x100 && (bits[id >> 3] & (1 << (id & 7)));
}

static void Bestiary_SetBit(u8 *bits, int id)
{
	if ((unsigned int)id < 0x100)
		bits[id >> 3] |= 1 << (id & 7);
}

static void Bestiary_Clear(struct BestiaryBits *bits)
{
	CpuFill16(0, bits, sizeof(*bits));
	bits->magic = BESTIARY_MAGIC;
}

/* OR into `into` a block's bits, if the block is valid and has them */
static void Bestiary_TakeBlock(struct BestiaryBits *into, int id)
{
	struct SaveBlockInfo info;
	struct BestiaryBits buf;
	int i;

	if (!ReadSaveBlockInfo(&info, id))
		return;

	ReadSramFast((const u8 *)GetSaveReadAddr(id) + SaveTail_BestiaryOffset(id == SAVE_ID_SUSPEND), &buf, sizeof(buf));
	if (buf.magic != BESTIARY_MAGIC)
		return;

	for (i = 0; i < (int)sizeof(buf.chars); i++) {
		into->chars[i] |= buf.chars[i];
		into->classes[i] |= buf.classes[i];
	}
}

/* Every save's bits together: the three game saves and the suspend save */
void Bestiary_ReadCart(struct BestiaryBits *bits)
{
	int id;

	Bestiary_Clear(bits);

	if (!IsSramWorking())
		return;

	for (id = SAVE_ID_GAME0; id <= SAVE_ID_GAME2; id++)
		Bestiary_TakeBlock(bits, id);

	Bestiary_TakeBlock(bits, SAVE_ID_SUSPEND);
}

/* gNewSaveHooks: a new game starts with what the other saves have met */
void Bestiary_OnNewGame(void)
{
	Bestiary_ReadCart(&gBestiarySeen);
}

/* The save tail (SaveTail.c): this game's bits */
void Bestiary_SaveTail(u8 *dst, const u32 size)
{
	if (size < sizeof(gBestiarySeen))
		return;

	if (gBestiarySeen.magic != BESTIARY_MAGIC)
		Bestiary_Clear(&gBestiarySeen);

	WriteAndVerifySramFast(&gBestiarySeen, dst, sizeof(gBestiarySeen));
}

/* The save tail, on loading a game or a suspend: the save's bits and every other save's */
void Bestiary_LoadTail(u8 *src, const u32 size)
{
	struct BestiaryBits buf;
	int i;

	Bestiary_ReadCart(&gBestiarySeen);

	if (src == NULL || size < sizeof(buf))
		return;

	ReadSramFast(src, &buf, sizeof(buf));
	if (buf.magic != BESTIARY_MAGIC)
		return;

	for (i = 0; i < (int)sizeof(buf.chars); i++) {
		gBestiarySeen.chars[i] |= buf.chars[i];
		gBestiarySeen.classes[i] |= buf.classes[i];
	}
}

/* A character and a class met, outside the phase sweep (an event, an ASMC) */
void Bestiary_MarkSeen(int pid, int jid)
{
	if (gBestiarySeen.magic != BESTIARY_MAGIC)
		Bestiary_Clear(&gBestiarySeen);

	Bestiary_SetBit(gBestiarySeen.chars, pid);
	Bestiary_SetBit(gBestiarySeen.classes, jid);
}

/* A unit the player can see: on the map, alive, and not in fog (the player's own always) */
static bool Bestiary_UnitSeen(struct Unit *unit)
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

/* gPrePhaseFuncs: mark every unit the player can see (RAM only; the next save keeps it) */
bool Bestiary_PrePhase(ProcPtr proc)
{
	int i;

	for (i = 1; i < 0xC0; i++) {
		struct Unit *unit = GetUnit(i);

		if (Bestiary_UnitSeen(unit))
			Bestiary_MarkSeen(unit->pCharacterData->number, unit->pClassData->number);
	}

	return false;
}
