#include "common-chax.h"
#include "save-data.h"

/*
 * Save layout guard (#711). The game and suspend saves are the chunks of gEmsSavChunks and gEmsSusChunks, end to end:
 * adding or resizing one moves every chunk after it, and a save written by a build with another layout loaded garbage.
 *
 * Each save block's checksum (SaveBlockInfo.checksum32) now carries a signature of the layout: a hash of the chunk
 * sizes and ids, the block sizes and the block offsets (not of code addresses, so it only changes with the layout).
 * A block written under another layout fails VerifySaveBlockChecksum, so ReadSaveBlockInfo, IsSaveValid and
 * IsValidSuspendSave call it invalid: the save menu shows NO DATA and nothing is loaded. Saves written before the
 * guard (checksum 0x2A, the constant SramChecksum32 returns since SaveData.event patched it) fail it too.
 */

static u32 SaveLayout_Hash(u32 hash, u32 value)
{
	int i;

	for (i = 0; i < 4; i++) {
		hash ^= (value >> (8 * i)) & 0xFF;
		hash *= 16777619;
	}
	return hash;
}

static u32 SaveLayout_HashChunks(u32 hash, const struct EmsChunk *cur)
{
	for (; cur->_identifier_ != EMS_CHUNK_INVALID_OFFSET; cur++) {
		hash = SaveLayout_Hash(hash, cur->size);
		hash = SaveLayout_Hash(hash, cur->chunk_idx);
	}
	return SaveLayout_Hash(hash, 0xFFFFFFFF);
}

u32 SaveLayout_Signature(void)
{
	u32 hash = 2166136261;
	int i;

	hash = SaveLayout_HashChunks(hash, gEmsSavChunks);
	hash = SaveLayout_HashChunks(hash, gEmsSusChunks);
	hash = SaveLayout_Hash(hash, EMS_SIZE_SAV);
	hash = SaveLayout_Hash(hash, EMS_SIZE_SUS);
	for (i = 0; i < SAVE_ID_MAX; i++)
		hash = SaveLayout_Hash(hash, gEmsOffsets[i]);

	/* 0 would match a save whose checksum is the bare SramChecksum32 */
	return hash ? hash : 1;
}

LYN_REPLACE_CHECK(PopulateSaveBlockChecksum);
void PopulateSaveBlockChecksum(struct SaveBlockInfo *buf)
{
	buf->checksum32 = SramChecksum32(SramOffsetToAddr(buf->offset), buf->size) ^ SaveLayout_Signature();
}

LYN_REPLACE_CHECK(VerifySaveBlockChecksum);
bool VerifySaveBlockChecksum(struct SaveBlockInfo *buf)
{
	return buf->checksum32 == (SramChecksum32(SramOffsetToAddr(buf->offset), buf->size) ^ SaveLayout_Signature());
}
