#include "common-chax.h"
#include "banim-codec.h"

/*
 * Huffman-coded battle animation data (Documentation/Features/CustomBattleAnimations.md, "Compression").
 *
 * The battle animation code decompresses sheets, scripts and OAM with LZ77UnCompWram. BanimCodec.event points those
 * calls at BanimUnComp: data tagged BANIM_HUFF_TAG is decoded here, everything else goes to the BIOS as before.
 *
 * The format is deflate's (RFC 1951) symbols and bit order, with the Huffman trees stored once per animation in ROM
 * (struct BanimHuffTrees, shared by all its sheets) instead of in each stream: the decoder needs no RAM, and builds no
 * trees; lookup tables there decode most codes in one step. Tools/banim/feditor_dump.py writes it:
 *     word 0: (decompressed size << 8) | BANIM_HUFF_TAG
 *     word 1: the struct BanimHuffTrees
 *     then the bits, read from the low bit of each byte up: literals and length/distance pairs, ended by symbol 256.
 */

/*
 * The decoding loop is ARM code (BanimHuffArm.s, from Tools/banim/huffdec.c) run from a copy in IWRAM: in Thumb from
 * the cartridge it took up to 3 frames a sheet, and a sheet being decoded across VBlanks showed for a frame as garbage.
 * IWRAM has no free room for it, so each decode borrows gMovMapFillStPool2 (0x03004C50, 0x200 bytes), as VoiceStream
 * borrows the pools around it (Documentation/Features/VoiceStream.md, "The IWRAM overlay"): the movement flood fill's
 * scratch, which holds data only while a flood fill runs. BanimCodec.event checks that the code fits.
 */
#define BANIM_HUFF_IWRAM ((u32 *)0x03004C50)

extern const u8 _ARM_BanimHuff_CopyStart[], _ARM_BanimHuff_CopyEnd[];

/* Deflate's length and distance codes: base | (extra bits << 16) */
struct BanimHuffCodes {
	u32 length[29];
	u32 dist[30];
};

typedef void (*BanimHuffArmFunc)(const u8 *src, u8 *dst, const struct BanimHuffTrees *trees,
				 const struct BanimHuffCodes *codes);

static const struct BanimHuffCodes sBanimHuffCodes = {
	.length = {
		3, 4, 5, 6, 7, 8, 9, 10,
		11 | 1 << 16, 13 | 1 << 16, 15 | 1 << 16, 17 | 1 << 16,
		19 | 2 << 16, 23 | 2 << 16, 27 | 2 << 16, 31 | 2 << 16,
		35 | 3 << 16, 43 | 3 << 16, 51 | 3 << 16, 59 | 3 << 16,
		67 | 4 << 16, 83 | 4 << 16, 99 | 4 << 16, 115 | 4 << 16,
		131 | 5 << 16, 163 | 5 << 16, 195 | 5 << 16, 227 | 5 << 16,
		258,
	},
	.dist = {
		1, 2, 3, 4,
		5 | 1 << 16, 7 | 1 << 16, 9 | 2 << 16, 13 | 2 << 16,
		17 | 3 << 16, 25 | 3 << 16, 33 | 4 << 16, 49 | 4 << 16,
		65 | 5 << 16, 97 | 5 << 16, 129 | 6 << 16, 193 | 6 << 16,
		257 | 7 << 16, 385 | 7 << 16, 513 | 8 << 16, 769 | 8 << 16,
		1025 | 9 << 16, 1537 | 9 << 16, 2049 | 10 << 16, 3073 | 10 << 16,
		4097 | 11 << 16, 6145 | 11 << 16, 8193 | 12 << 16, 12289 | 12 << 16,
		16385 | 13 << 16, 24577 | 13 << 16,
	},
};

static void BanimHuffUnComp(const u32 *src, u8 *dst)
{
	const u32 *from = (const u32 *)_ARM_BanimHuff_CopyStart;
	u32 *to = BANIM_HUFF_IWRAM;
	int i, n = (_ARM_BanimHuff_CopyEnd - _ARM_BanimHuff_CopyStart + 3) >> 2;

	/* Keep GCC from folding the copy into one base register (see VoiceStream.c: the relocation comes out short) */
	__asm__("" : "+r"(from));

	for (i = 0; i < n; i++)
		to[i] = from[i];

	((BanimHuffArmFunc)BANIM_HUFF_IWRAM)((const u8 *)(src + 2), dst, (const void *)src[1], &sBanimHuffCodes);
}

/* Takes the place of LZ77UnCompWram at the battle animation code's calls (BanimCodec.event) */
void BanimUnComp(const void *src, void *dst)
{
	if (*(const u8 *)src == BANIM_HUFF_TAG)
		BanimHuffUnComp(src, dst);
	else
		LZ77UnCompWram(src, dst);
}
