#include "common-chax.h"
#include "kernel-lib.h"
#include "voice-stream.h"
#include "VoiceStreamTables.h"

/*
 * VC2 voice lines (Documentation/Features/VoiceStream.md).
 *
 * A stream song (Tools/voice/vc2.py gen-unit / gen-wm) is an ordinary one-track song whose instrument plays
 * gVoiceStream.wave, a looping 8-bit wave whose data is a ring buffer. m4aSongNumStart sees the instrument, sets the
 * decoder up and starts the song. ProcScr_VoiceStream then keeps the ring decoded ahead of the mixer, from the main
 * loop, and stops the song when the line is over.
 *
 * Decoding a 128-sample frame: the bit stream is parsed here (Thumb, from ROM); the transform runs in ARM from IWRAM
 * (VoiceSynthArm.s). IWRAM has no free room for it (VoiceStream.md, "IWRAM"), so each decode burst borrows the movement
 * flood fill's scratch pools: they only hold data while a flood fill runs, and the bursts run between proc calls in
 * the main loop, never inside one. The two live variables between the pools are saved and put back after the burst.
 */

/* 0x030049B0-0x03004E50: gMovMapFillStPool1, gWorkingTerrainMoveCosts, gStatGainSimUnit, gMovMapFillStPool2 */
#define VS_OVL_BASE     ((u8 *)0x030049B0)
#define VS_OVL_END      ((u8 *)0x03004E50)
#define VS_OVL_LIVE     ((u8 *)0x03004BB0)  /* gWorkingTerrainMoveCosts and gStatGainSimUnit, to 0x03004C50 */

#define VS_LEAD_MAX     (8 * VS_FRAME)            /* keep this many samples decoded ahead of the mixer (77 ms) */
#define VS_LEAD_MIN     (3 * VS_FRAME)            /* and at least this many: the mixer reads 224 a VBlank */
#define VS_START_GAP    (4 * VS_FRAME)            /* silence before a line, 38 ms (VoiceStream_OnLoop) */

_Static_assert(sizeof(struct VoiceStream) <= 0xE60, "gVoiceStream's _kernel_malloc_demo size (config-memmap.s)");

extern const u8 _ARM_VoiceSynth_CopyStart[], _ARM_VoiceSynth_CopyEnd[];

typedef void (*VoiceSynthFunc)(s32 *ovl, s32 *fft, s32 *xy, s8 *out);

/* A stream song's header: one track, then the VC2 extension */
struct VsSongHeader {
	u8 trackCount, blockCount, priority, reverb;
	const void *tone;
	const u8 *part[1];
	struct VoiceStreamSongExt ext;
};

/* The HQ mixer's channel fields (Contents/External/m4a_hq_mixer: CHN_WAVE_OFFSET, CHN_POSITION_ABS) */
#define VS_CHAN_WAVE           0x24
#define VS_CHAN_POS            0x28

/* ---- the shared instrument and track ---- */

/* DirectSound, middle C, gVoiceStream.wave, no envelope (attack 0xFF, sustain 0xFF) */
const struct VoiceStreamTone VoiceStream_Tone = {
	.type = 0, .key = 0x3C, .wav = &gVoiceStream.wave, .attack = 0xFF, .decay = 0, .sustain = 0xFF, .release = 0,
};

/*
 * VOL 127, KEYSH 0, TEMPO 2 bpm (one tick every 75 frames), VOICE 0, TIE C3 v64 (as the DPCM songs), W96 (two
 * minutes), FINE. ProcScr_VoiceStream stops the song when the line ends.
 */
const u8 VoiceStream_Track[] = {
	0xBE, 0x7F, 0xBC, 0x00, 0xBB, 0x01, 0xBD, 0x00, 0xCF, 0x3C, 0x40, 0xB0, 0xB1,
};

/* ---- decoder ---- */

static void VsInit(struct VoiceStream *vs, const u8 *file)
{
	int b, j;

	vs->nfr = file[4] | (file[5] << 8);
	for (b = 0; b < VS_NB; b++)
		for (j = 0; j < 7; j++)
			vs->thr[b][j] = file[12 + 7 * b + j];
	vs->ptr = file + 12 + 7 * VS_NB;
	vs->cache = 0;
	vs->nb = 0;
	vs->rng = 1;
	vs->frame = 0;
	for (j = 0; j < VS_FRAME; j++)
		vs->ovl[j] = 0;
}

/*
 * Bit reader and noise generator, in locals for the length of a frame (the struct is in EWRAM, slow to reach).
 * The cache holds the next bits MSB first; nb of them are valid.
 */
struct VsBitReader {
	const u8 *ptr;
	u32 cache;
	int nb;
	u32 rng;
};

static inline void VsFill(struct VsBitReader *br)
{
	while (br->nb <= 24) {
		br->cache |= (u32)(*br->ptr++) << (24 - br->nb);
		br->nb += 8;
	}
}

static inline u32 VsBits(struct VsBitReader *br, int n)
{
	u32 v;

	if (!n)
		return 0;
	VsFill(br);
	v = br->cache >> (32 - n);
	br->cache <<= n;
	br->nb -= n;
	return v;
}

/* Rice: a run of 1 bits (counted a byte at a time), a 0, then k bits */
static inline u32 VsRice(struct VsBitReader *br, int k)
{
	u32 q = 0;

	for (;;) {
		u32 ones;

		VsFill(br);
		ones = vd_lead1[br->cache >> 24];
		if (ones < 8) {
			q += ones;
			br->cache <<= ones + 1;
			br->nb -= ones + 1;
			break;
		}
		q += 8;
		br->cache <<= 8;
		br->nb -= 8;
	}
	return (q << k) | VsBits(br, k);
}

static inline int VsUnzz(u32 u)
{
	return (u & 1) ? -(int)((u + 1) >> 1) : (int)(u >> 1);
}

static inline int VsRnd(struct VsBitReader *br)
{
	int v;

	br->rng = br->rng * 1664525u + 1013904223u;
	v = br->rng >> 16;
	return v >= 32768 ? v - 65536 : v;
}

/* One frame's coefficients (with the IMDCT gain) into X */
static void VsParse(struct VoiceStream *vs, s32 *X)
{
	struct VsBitReader br = { vs->ptr, vs->cache, vs->nb, vs->rng };
	int sfi[VS_NB], b, i;

	sfi[0] = VsBits(&br, 6);
	for (b = 1; b < VS_NB; b++)
		sfi[b] = sfi[b - 1] + VsUnzz(VsRice(&br, 1));

	for (b = 0; b < VS_NB; b++) {
		int s = vd_edges[b], e = vd_edges[b + 1], sf = sfi[b], lj = -1, j;

		if (sf < 0)
			sf = 0;
		if (sf > 63)
			sf = 63;
		for (j = 0; j < 7; j++)
			if (sf >= vs->thr[b][j])
				lj = j;

		if (lj < 0) {
			/* noise fill */
			s32 a = vd_amp[sf] >> 4;

			for (i = s; i < e; i++)
				X[i] = ((((VsRnd(&br) >> 1) * a) >> 14) * 45) >> 6;
		} else {
			s32 st = vd_step[sf][lj];
			int k = vd_ricek[lj];

			for (i = s; i < e; i++) {
				int q = VsUnzz(VsRice(&br, k));

				X[i] = q ? (q * st) >> 4 : (((VsRnd(&br) >> 1) * (st >> 4)) >> 16);
			}
		}
	}

	vs->ptr = br.ptr;
	vs->cache = br.cache;
	vs->nb = br.nb;
	vs->rng = br.rng;
}

/* ---- the IWRAM overlay ---- */

static void VsOverlayBegin(struct VoiceStream *vs)
{
	const u32 *src = (const u32 *)_ARM_VoiceSynth_CopyStart;
	u32 *dst = (u32 *)VS_OVL_BASE;
	int i, n = (_ARM_VoiceSynth_CopyEnd - _ARM_VoiceSynth_CopyStart + 3) >> 2;

	/*
	 * Keep GCC from folding the copy into "src - dst" off one base register: the relocation for a symbol minus
	 * 0x030049B0 comes out of lyn and EA 0x08000000 short.
	 */
	__asm__("" : "+r"(src));

	for (i = 0; i < 0xA0 / 4; i++)
		((u32 *)vs->save)[i] = ((u32 *)VS_OVL_LIVE)[i];
	for (i = 0; i < n; i++)
		dst[i] = src[i];
}

static void VsOverlayEnd(struct VoiceStream *vs)
{
	int i;

	for (i = 0; i < 0xA0 / 4; i++)
		((u32 *)VS_OVL_LIVE)[i] = ((u32 *)vs->save)[i];
}

/*
 * Decode `count` frames into the ring, from the proc. The FFT's 512 bytes are on the stack (IWRAM, fast); the rest of
 * the state is in gVoiceStream. Global so that it shows in the .sym file.
 */
NOINLINE void VoiceStream_DecodeBurst(struct VoiceStream *vs, int count)
{
	s32 fft[VS_FRAME];
	VoiceSynthFunc synth = (VoiceSynthFunc)VS_OVL_BASE;
	bool overlay = false;

	while (count-- > 0) {
		s8 *out = vs->wave.data + vs->wpos;

		if (vs->frame < vs->nfr) {
			if (!overlay) {
				VsOverlayBegin(vs);
				overlay = true;
			}
			VsParse(vs, vs->xy);
			synth(vs->ovl, fft, vs->xy, out);

			/* frame 0 only primes the overlap: its output is overwritten by frame 1 */
			if (vs->frame++ == 0) {
				count++;
				continue;
			}
		} else {
			int i;

			for (i = 0; i < VS_FRAME; i += 4)
				*(u32 *)(out + i) = 0;
			if (!vs->end)
				vs->end = vs->written;
		}
		vs->written += VS_FRAME;
		vs->wpos += VS_FRAME;
		if (vs->wpos >= VS_RING)
			vs->wpos = 0;
	}
	if (overlay)
		VsOverlayEnd(vs);
}

/* ---- playback ---- */

static u8 *VsFindChannel(void)
{
	int i;

	for (i = 0; i < gSoundInfo.maxChans; i++) {
		u8 *chan = (u8 *)&gSoundInfo.chans[i];

		if (chan[0] && *(void **)(chan + VS_CHAN_WAVE) == &gVoiceStream.wave)
			return chan;
	}
	return NULL;
}

static void VsStop(struct VoiceStream *vs)
{
	if (!vs->active)
		return;
	vs->active = false;
	if (((struct MusicPlayerInfo *)vs->player)->songHeader == vs->song)
		m4aMPlayStop(vs->player);
}

static void VoiceStream_OnLoop(ProcPtr proc)
{
	struct VoiceStream *vs = &gVoiceStream;
	u8 *chan;
	int lead, count, cap, d = 0;

	if (!vs->active || ((struct MusicPlayerInfo *)vs->player)->songHeader != vs->song) {
		VsStop(vs);
		Proc_End(proc);
		return;
	}

	chan = VsFindChannel();
	if (chan) {
		int pos = *(s8 **)(chan + VS_CHAN_POS) - vs->wave.data;

		if (pos < 0 || pos > VS_RING)
			pos = vs->last_pos;
		d = pos - vs->last_pos;
		if (d < 0)
			d += VS_RING;
		vs->played += d;
		vs->last_pos = pos;
		vs->seen = true;
	} else if (vs->seen || ++vs->ticks > 60) {
		/* the note is over (or never started) */
		VsStop(vs);
		Proc_End(proc);
		return;
	}

	if (vs->end && vs->played >= vs->end) {
		VsStop(vs);
		Proc_End(proc);
		return;
	}

	/*
	 * The line starts VS_START_GAP samples ahead of the mixer (which plays the zeroed ring until then): the decoder
	 * builds up a lead first, through the lag frames a scene often has when a line starts. Behind the mixer later
	 * means the main loop stalled for longer than the lead: the mixer replayed old samples; start again the same way.
	 */
	if (vs->written < vs->played + (vs->frame ? 0 : VS_START_GAP)) {
		u32 t = ((vs->played + VS_FRAME - 1) & ~(VS_FRAME - 1)) + VS_START_GAP;

		if (vs->frame > 1)
			vs->underruns++;
		while (vs->written < t) {
			vs->written += VS_FRAME;
			vs->wpos += VS_FRAME;
			if (vs->wpos >= VS_RING)
				vs->wpos = 0;
		}
	}

	/*
	 * Top the lead up to VS_LEAD_MAX, at most what the mixer read since the last tick plus one frame, but at least
	 * to VS_LEAD_MIN ahead. A main loop that takes two VBlanks a frame gets twice the decoding, so the sound keeps
	 * up and the game slows instead; a normal frame (224 samples read) decodes 2 or 3 frames.
	 */
	lead = vs->written - vs->played;
	count = (VS_LEAD_MAX - lead + VS_FRAME - 1) / VS_FRAME;
	cap = 1 + (d + VS_FRAME - 1) / VS_FRAME;
	if (lead < VS_LEAD_MIN && cap < (VS_LEAD_MIN - lead + VS_FRAME - 1) / VS_FRAME)
		cap = (VS_LEAD_MIN - lead + VS_FRAME - 1) / VS_FRAME;
	if (cap > (lead < 2 * 224 ? 4 : 3))
		cap = lead < 2 * 224 ? 4 : 3;
	if (count > cap)
		count = cap;
	if (count > 0)
		VoiceStream_DecodeBurst(vs, count);
}

static void VoiceStream_OnEnd(ProcPtr proc)
{
	VsStop(&gVoiceStream);
}

const struct ProcCmd ProcScr_VoiceStream[] = {
	PROC_NAME("VoiceStream"),
	PROC_SET_END_CB(VoiceStream_OnEnd),
	PROC_REPEAT(VoiceStream_OnLoop),
	PROC_END,
};

/* Returns the song to start: hdr, or NULL for none */
static const void *VoiceStream_Start(const struct VsSongHeader *hdr, void *player)
{
	struct VoiceStream *vs = &gVoiceStream;
	int i;

	if (hdr->ext.magic != VS_MAGIC)
		return hdr;
	VsStop(vs);
	if (_ARM_VoiceSynth_CopyEnd - _ARM_VoiceSynth_CopyStart > VS_OVL_END - VS_OVL_BASE)
		return NULL;

	vs->wave.type = 0;
	vs->wave.loop = 0x40;
	vs->wave.freq = 13379 << 10;
	vs->wave.loop_start = 0;
	vs->wave.size = VS_RING;
	for (i = 0; i < VS_RING; i += 4)
		*(u32 *)(vs->wave.data + i) = 0;

	VsInit(vs, hdr->ext.vc2);
	vs->song = hdr;
	vs->player = player;
	vs->written = vs->played = vs->end = 0;
	vs->wpos = vs->last_pos = vs->ticks = 0;
	vs->seen = false;
	vs->active = true;

	if (!Proc_Find(ProcScr_VoiceStream))
		Proc_Start(ProcScr_VoiceStream, PROC_TREE_3);
	return hdr;
}

LYN_REPLACE_CHECK(m4aSongNumStart);
void m4aSongNumStart(u16 n)
{
	const struct Song *song = &gSongTable[n];
	const struct MusicPlayer *mplay = &gMPlayTable[song->ms];
	const void *header = song->header;

	if ((const void *)song->header->tone == &VoiceStream_Tone) {
		header = VoiceStream_Start(header, mplay->info);
		if (!header)
			return;
	}
	MPlayStart(mplay->info, (struct SongHeader *)header);
}
