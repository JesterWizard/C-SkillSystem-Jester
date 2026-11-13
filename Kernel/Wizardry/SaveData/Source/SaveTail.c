#include "common-chax.h"
#include "save-data.h"
#include "new-game-plus.h"
#include "bestiary.h"
#include "skill-codex.h"

/*
 * The bytes after the save tables (gpMsa_Saver, gpMsa_Loader, gpMsu_Saver, gpMsu_Loader in Preload/Reloc.event):
 * New Game+'s, then the Bestiary's, then the Skill Codex's, in the game save and in the suspend save. These bytes are
 * not a chunk, so using them changes neither the save layout nor its signature; a save written before a feature took
 * its share has no magic there, and the feature reads nothing from it.
 *
 * Each share has a fixed offset in the tail. Add a feature at the end; the size check skips it when the block is full.
 */

#define SAV_NGP_SIZE      sizeof(struct NewGamePlusCarry)
#define SUS_NGP_SIZE      sizeof(struct NewGamePlusSusCarry)
#define SAV_BESTIARY_AT   SAV_NGP_SIZE
#define SUS_BESTIARY_AT   SUS_NGP_SIZE
#define SAV_CODEX_AT      (SAV_BESTIARY_AT + sizeof(struct BestiaryBits))
#define SUS_CODEX_AT      (SUS_BESTIARY_AT + sizeof(struct BestiaryBits))

typedef void (*SaveTailFunc)(u8 *buf, const u32 size);

static u32 SaveTail_TablesSize(const struct EmsChunk *cur)
{
	u32 size = 0;

	for (; cur->_identifier_ != EMS_CHUNK_INVALID_OFFSET; cur++)
		size += cur->size;

	return size;
}

static u32 SaveTail_BlockOffset(bool suspend)
{
	return SaveTail_TablesSize(suspend ? gEmsSusChunks : gEmsSavChunks);
}

/* Where a share of a block's tail is (a game save slot or the suspend save), from the block's start */
u32 SaveTail_BestiaryOffset(bool suspend)
{
	return SaveTail_BlockOffset(suspend) + (suspend ? SUS_BESTIARY_AT : SAV_BESTIARY_AT);
}

u32 SaveTail_SkillCodexOffset(bool suspend)
{
	return SaveTail_BlockOffset(suspend) + (suspend ? SUS_CODEX_AT : SAV_CODEX_AT);
}

/* A share that starts `at` bytes into the tail: saved only if it starts in it; loaded with NULL if not */
static void SaveTail_Save(SaveTailFunc func, u8 *dst, const u32 size, u32 at)
{
	if (size >= at)
		func(dst + at, size - at);
}

static void SaveTail_Load(SaveTailFunc func, u8 *src, const u32 size, u32 at)
{
	if (size >= at)
		func(src + at, size - at);
	else
		func(NULL, 0);
}

void SaveTail_SaveSav(u8 *dst, const u32 size)
{
	NewGamePlus_SaveSav(dst, size);
	SaveTail_Save(Bestiary_SaveTail, dst, size, SAV_BESTIARY_AT);
	SaveTail_Save(SkillCodex_SaveTail, dst, size, SAV_CODEX_AT);
}

void SaveTail_LoadSav(u8 *src, const u32 size)
{
	NewGamePlus_LoadSav(src, size);
	SaveTail_Load(Bestiary_LoadTail, src, size, SAV_BESTIARY_AT);
	SaveTail_Load(SkillCodex_LoadTail, src, size, SAV_CODEX_AT);
}

void SaveTail_SaveSus(u8 *dst, const u32 size)
{
	NewGamePlus_SaveSus(dst, size);
	SaveTail_Save(Bestiary_SaveTail, dst, size, SUS_BESTIARY_AT);
	SaveTail_Save(SkillCodex_SaveTail, dst, size, SUS_CODEX_AT);
}

void SaveTail_LoadSus(u8 *src, const u32 size)
{
	NewGamePlus_LoadSus(src, size);
	SaveTail_Load(Bestiary_LoadTail, src, size, SUS_BESTIARY_AT);
	SaveTail_Load(SkillCodex_LoadTail, src, size, SUS_CODEX_AT);
}
