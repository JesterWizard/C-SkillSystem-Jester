#pragma once

#include "common-chax.h"

/*
 * Huffman-coded battle animation data (Kernel/Wizardry/BanimCodec, made by Tools/banim/feditor_dump.py):
 * word 0 is (size << 8) | BANIM_HUFF_TAG, like the BIOS's LZ77 header (whose tag is 0x10); word 1 points to the
 * animation's struct BanimHuffTrees; then the bits.
 */
#define BANIM_HUFF_TAG 0xD1

#define BANIM_HUFF_LIT_BITS  9
#define BANIM_HUFF_DIST_BITS 7

/*
 * Deflate's literal/length and distance codes: canonical Huffman trees (codes per length, symbols in code order) and,
 * for speed, lookup tables indexed by the next bits of the stream: (code length << 9) | symbol, or 0 for a code
 * longer than the table, which the trees decode.
 */
struct BanimHuffTrees {
	u16 lit_count[16];
	u16 lit_symbol[288];
	u16 dist_count[16];
	u16 dist_symbol[32];
	u16 lit_fast[1 << BANIM_HUFF_LIT_BITS];
	u16 dist_fast[1 << BANIM_HUFF_DIST_BITS];
};

void BanimUnComp(const void *src, void *dst);
