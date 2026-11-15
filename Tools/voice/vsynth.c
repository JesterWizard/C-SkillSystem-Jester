/*
 * VC2 synthesis: one 128-sample frame from its 128 MDCT coefficients (IMDCT via a DCT-IV on a 64-point radix-4 FFT,
 * sine window, overlap-add), written as 8-bit samples for the m4a mixer.
 *
 * This is the source of Kernel/Wizardry/VoiceStream/Source/VoiceSynthArm.s: Tools/voice/gen_synth_asm.sh compiles it
 * to ARM assembly once and the .s is committed, so every build copies the same bytes. The game copies the code into
 * borrowed IWRAM before each decode burst (VoiceStream.c), so it must stay position independent: no calls, no
 * division, nothing but its own literal pool and the tables below (which stay in ROM).
 *
 * fft: 128 words, re then im (on the caller's stack: IWRAM, fast). xy: 128 words, the coefficients on entry, then the
 * IMDCT output (the coefficients are read before it is written). ovl: 128 words of overlap kept between frames.
 */
typedef signed char s8;
typedef unsigned char u8;
typedef short s16;
typedef int s32;

#include "vsynth_tables.h"

__attribute__((target("arm"), noinline, optimize("Os")))
void VoiceSynth_Frame(s32 *ovl, s32 *fft, s32 *xy, s8 *out)
{
	s32 *re = fft, *im = fft + 64, *X = xy, *Y = xy;
	int i, len, tws;

	for (i = 0; i < 64; i++) {
		s32 xr = X[2 * i] >> 2, xi = X[127 - 2 * i] >> 2;
		s32 w = vd_pre[i], c = (s16)w, s = w >> 16;
		int r = vd_digrev4[i];

		re[r] = (xr * c - xi * s) >> VD_Q;
		im[r] = (xr * s + xi * c) >> VD_Q;
	}

	/* three radix-4 passes (span 1, 4, 16); the twiddle stride is 16 / span */
	for (len = 1, tws = 4; len < 64; len <<= 2, tws -= 2) {
		int st, k;

		for (st = 0; st < 64; st += 4 * len) {
			for (k = 0; k < len; k++) {
				int i0 = st + k, i1 = i0 + len, i2 = i1 + len, i3 = i2 + len;
				s32 x1r = re[i1], x1i = im[i1], x2r = re[i2], x2i = im[i2], x3r = re[i3], x3i = im[i3];
				s32 t0r, t0i, t1r, t1i, t2r, t2i, t3r, t3i;

				if (k) {
					int ti = k << tws;
					s32 w1 = vd_tw[ti], w2 = vd_tw[2 * ti], w3 = vd_tw[3 * ti], c, s, tr;

					c = (s16)w1; s = w1 >> 16;
					tr = (x1r * c - x1i * s) >> VD_Q; x1i = (x1r * s + x1i * c) >> VD_Q; x1r = tr;
					c = (s16)w2; s = w2 >> 16;
					tr = (x2r * c - x2i * s) >> VD_Q; x2i = (x2r * s + x2i * c) >> VD_Q; x2r = tr;
					c = (s16)w3; s = w3 >> 16;
					tr = (x3r * c - x3i * s) >> VD_Q; x3i = (x3r * s + x3i * c) >> VD_Q; x3r = tr;
				}
				t0r = re[i0] + x2r; t0i = im[i0] + x2i;
				t1r = re[i0] - x2r; t1i = im[i0] - x2i;
				t2r = x1r + x3r; t2i = x1i + x3i;
				t3r = x1i - x3i; t3i = x3r - x1r;
				re[i0] = t0r + t2r; im[i0] = t0i + t2i;
				re[i1] = t1r + t3r; im[i1] = t1i + t3i;
				re[i2] = t0r - t2r; im[i2] = t0i - t2i;
				re[i3] = t1r - t3r; im[i3] = t1i - t3i;
			}
		}
	}

	for (i = 0; i < 64; i++) {
		s32 w = vd_post[i], c = (s16)w, s = w >> 16;

		Y[2 * i] = ((re[i] * c - im[i] * s) >> VD_Q) << 2;
		Y[127 - 2 * i] = -(((re[i] * s + im[i] * c) >> VD_Q) << 2);
	}

	/* unfold, window (symmetric: vd_win128 is its first half), overlap-add, round to 8 bits */
	for (i = 0; i < 128; i++) {
		s32 v;

		if (i < 64) {
			v = ovl[i] + ((Y[64 + i] * vd_win128[i]) >> VD_Q);
			ovl[i] = -((Y[63 - i] * vd_win128[127 - i]) >> VD_Q);
		} else {
			v = ovl[i] - ((Y[191 - i] * vd_win128[i]) >> VD_Q);
			ovl[i] = -((Y[i - 64] * vd_win128[127 - i]) >> VD_Q);
		}
		v = (v + 128) >> 8;
		out[i] = v > 127 ? 127 : v < -128 ? -128 : v;
	}
}
