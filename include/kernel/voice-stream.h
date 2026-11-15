#pragma once

#include "common-chax.h"

/*
 * Kernel/Wizardry/VoiceStream: voice lines coded with VC2 (Tools/voice/vc2.py), decoded while they play into a
 * looping 8-bit wave that the m4a mixer plays. See Documentation/Features/VoiceStream.md.
 */

#define VS_FRAME    128
#define VS_RING     (18 * VS_FRAME)  /* 2304 samples, 172 ms at 13379 Hz */
#define VS_NB       14
#define VS_MAGIC    0x32435356       /* "VSC2", after a stream song's track pointer */

/* The m4a mixer's wave header (WaveData), then the ring the decoder fills */
struct VoiceStreamWave {
	u16 type;        /* 0: 8-bit PCM */
	u8 _pad;
	u8 loop;         /* 0x40: loops */
	u32 freq;        /* the rate at middle C, x 1024 */
	u32 loop_start;
	u32 size;
	s8 data[VS_RING];
};

struct VoiceStream {
	struct VoiceStreamWave wave;

	/* decoder */
	s32 ovl[VS_FRAME];
	s32 xy[VS_FRAME];  /* a frame's coefficients, then its IMDCT output (VoiceSynth_Frame) */
	const u8 *ptr;
	u32 cache;
	s32 nb;
	u32 rng;
	u16 nfr, frame;
	u8 thr[VS_NB][7];
	u8 active;
	u8 seen;          /* the mixer has started the note */

	/* playback, in samples since the note started (the ring holds sample t at data[t mod VS_RING]) */
	const void *song;
	void *player;
	u32 written;      /* decoded up to here */
	u32 played;       /* the mixer has read up to here */
	u32 end;          /* the line ends here (0: still decoding) */
	u16 wpos;         /* written mod VS_RING */
	u16 last_pos;     /* the mixer's position in the ring at the last tick */
	u16 ticks;
	u16 underruns;    /* times the mixer caught up with the decoder (for tests) */

	u8 save[0xA0];    /* the live IWRAM under the decoder's code while it runs (VoiceStream.c) */
};

extern struct VoiceStream gVoiceStream;

/* What vc2.py's stream songs put after the track pointer of their song header */
struct VoiceStreamSongExt {
	u32 magic;
	const u8 *vc2;
};

/* Shared by every stream song: the instrument that plays gVoiceStream.wave, and its one-note track */
struct VoiceStreamTone {
	u8 type, key, length, pan_sweep;
	const void *wav;
	u8 attack, decay, sustain, release;
};
extern const struct VoiceStreamTone VoiceStream_Tone;
extern const u8 VoiceStream_Track[];
