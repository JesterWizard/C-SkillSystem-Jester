#include "common-chax.h"
#include "save-data.h"
#include "new-game-plus.h"
#include "bestiary.h"

/*
 * The bytes after the save tables (gpMsa_Saver, gpMsa_Loader, gpMsu_Saver, gpMsu_Loader in Preload/Reloc.event):
 * New Game+'s, then the Bestiary's, in the game save and in the suspend save. These bytes are not a chunk, so using
 * them changes neither the save layout nor its signature; a save written before a feature took its share has no magic
 * there, and the feature reads nothing from it.
 *
 * Each share has a fixed offset in the tail. Add a feature at the end; the size check skips it when the block is full.
 */

#define SAV_NGP_SIZE      sizeof(struct NewGamePlusCarry)
#define SUS_NGP_SIZE      sizeof(struct NewGamePlusSusCarry)
#define SAV_BESTIARY_AT   SAV_NGP_SIZE
#define SUS_BESTIARY_AT   SUS_NGP_SIZE

static u32 SaveTail_TablesSize(const struct EmsChunk *cur)
{
	u32 size = 0;

	for (; cur->_identifier_ != EMS_CHUNK_INVALID_OFFSET; cur++)
		size += cur->size;

	return size;
}

/* Where the Bestiary's share of a block's tail is (a game save slot or the suspend save), from the block's start */
u32 SaveTail_BestiaryOffset(bool suspend)
{
	if (suspend)
		return SaveTail_TablesSize(gEmsSusChunks) + SUS_BESTIARY_AT;

	return SaveTail_TablesSize(gEmsSavChunks) + SAV_BESTIARY_AT;
}

void SaveTail_SaveSav(u8 *dst, const u32 size)
{
	NewGamePlus_SaveSav(dst, size);

	if (size >= SAV_BESTIARY_AT)
		Bestiary_SaveTail(dst + SAV_BESTIARY_AT, size - SAV_BESTIARY_AT);
}

void SaveTail_LoadSav(u8 *src, const u32 size)
{
	NewGamePlus_LoadSav(src, size);
	Bestiary_LoadTail(size >= SAV_BESTIARY_AT ? src + SAV_BESTIARY_AT : NULL,
			  size >= SAV_BESTIARY_AT ? size - SAV_BESTIARY_AT : 0);
}

void SaveTail_SaveSus(u8 *dst, const u32 size)
{
	NewGamePlus_SaveSus(dst, size);

	if (size >= SUS_BESTIARY_AT)
		Bestiary_SaveTail(dst + SUS_BESTIARY_AT, size - SUS_BESTIARY_AT);
}

void SaveTail_LoadSus(u8 *src, const u32 size)
{
	NewGamePlus_LoadSus(src, size);
	Bestiary_LoadTail(size >= SUS_BESTIARY_AT ? src + SUS_BESTIARY_AT : NULL,
			  size >= SUS_BESTIARY_AT ? size - SUS_BESTIARY_AT : 0);
}
