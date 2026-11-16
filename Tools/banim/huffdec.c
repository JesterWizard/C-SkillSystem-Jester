/*
 * The Huffman sheet decoder's inner loop, for IWRAM (Kernel/Wizardry/BanimCodec/Source/BanimCodec.c).
 *
 * Compiled to ARM by Tools/banim/gen_huffdec_asm.sh into Kernel/Wizardry/BanimCodec/Source/BanimHuffArm.s. It runs
 * from a copy in IWRAM, so it calls nothing and reads no data of its own: its tables come in as arguments. The format
 * is described in BanimCodec.c and include/kernel/banim-codec.h; Tools/banim/banimpack.py's huff_unpack is the same
 * decoder in Python.
 */
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;

#define LIT_BITS  9	/* BANIM_HUFF_LIT_BITS */
#define DIST_BITS 7	/* BANIM_HUFF_DIST_BITS */

struct BanimHuffTrees {
	u16 lit_count[16];
	u16 lit_symbol[288];
	u16 dist_count[16];
	u16 dist_symbol[32];
	u16 lit_fast[1 << LIT_BITS];
	u16 dist_fast[1 << DIST_BITS];
};

/* Deflate's length and distance codes: base | (extra bits << 16) */
struct BanimHuffCodes {
	u32 length[29];
	u32 dist[30];
};

#define REFILL()					\
	do {						\
		while (count <= 24) {			\
			buf |= (u32)*src++ << count;	\
			count += 8;			\
		}					\
	} while (0)

#define DECODE(out, fast, bits, cnt, sym)						\
	do {										\
		u32 e_;									\
		REFILL();								\
		e_ = (fast)[buf & ((1 << (bits)) - 1)];				\
		if (e_ != 0) {								\
			buf >>= e_ >> 9;						\
			count -= e_ >> 9;						\
			(out) = e_ & 0x1FF;						\
		} else {								\
			int s_ = 0, c_ = 0, l_ = 0;					\
			do {								\
				c_ = 2 * c_ + (buf & 1);				\
				buf >>= 1;						\
				count--;						\
				l_++;							\
				s_ += (cnt)[l_];					\
				c_ -= (cnt)[l_];					\
			} while (c_ >= 0);						\
			(out) = (sym)[s_ + c_];						\
		}									\
	} while (0)

void BanimHuff_DecodeArm(const u8 *src, u8 *dst, const struct BanimHuffTrees *t, const struct BanimHuffCodes *codes)
{
	u32 buf = 0;
	int count = 0;

	for (;;) {
		u32 sym, code, n, len, dist;
		const u8 *from;

		DECODE(sym, t->lit_fast, LIT_BITS, t->lit_count, t->lit_symbol);
		if (sym < 256) {
			*dst++ = sym;
			continue;
		}
		if (sym == 256)
			return;

		code = codes->length[sym - 257];
		n = code >> 16;
		REFILL();
		len = (code & 0xFFFF) + (buf & ((1 << n) - 1));
		buf >>= n;
		count -= n;

		DECODE(sym, t->dist_fast, DIST_BITS, t->dist_count, t->dist_symbol);
		code = codes->dist[sym];
		n = code >> 16;
		REFILL();
		dist = (code & 0xFFFF) + (buf & ((1 << n) - 1));
		buf >>= n;
		count -= n;

		from = dst - dist;
		do {
			*dst++ = *from++;
		} while (--len);
	}
}
