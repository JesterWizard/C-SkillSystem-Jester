// Headless mGBA runner driven by a small script.
// usage: gbarun rom.gba script.txt
// script commands (one per line, # comments):
//   wait N                 run N frames, no keys
//   press KEYS [N]         hold KEYS (A,B,L,R,START,SELECT,UP,DOWN,LEFT,RIGHT joined by +) N frames (default 2), then 6 frames released
//   hold KEYS N            hold KEYS N frames without release gap
//   shot FILE.png          screenshot
//   save FILE / load FILE  savestate
//   w8|w16|w32 ADDR VAL    write memory
//   r8|r16|r32 ADDR        print memory
//   dump ADDR LEN FILE     dump memory
//   sram FILE              load battery save file before reset (must come first)
//   trace KEYS N K         run N frames a step at a time with KEYS held (NONE for none), print the last K PCs
//   runto KEYS ADDR N      step with KEYS held until the PC is ADDR (at most N frames)
//   skipto KEYS ADDR N     step until the PC is ADDR (at most N frames), pressing KEYS every 38 frames
//   cyc LO HI [LO2 HI2] N  run N frames; cycles spent with the PC (as mGBA reports it) in [LO,HI) or [LO2,HI2)
//   regs                   print the CPU registers
//   audio FILE.wav         record the sound of the frames that follow (wait/press/hold), stereo 32768 Hz
//   audio end              stop recording and write the file
//   prof N ADDR [NAME]     run N frames one instruction at a time and report
//                          the calls to the function at ADDR: count, total
//                          and slowest call in CPU cycles (280896 per frame)
#include <mgba/core/core.h>
#include <mgba/core/timing.h>
#include <mgba/internal/arm/arm.h>
#include <mgba/core/config.h>
#include <mgba/core/serialize.h>
#include <mgba-util/vfs.h>
#include <mgba/core/log.h>
#include <mgba/core/blip_buf.h>
#include <png.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static struct mCore *core;
static uint32_t *vbuf;
static unsigned W, H;

static int keymask(const char *s)
{
	static const char *names[] = {"A", "B", "SELECT", "START", "RIGHT", "LEFT", "UP", "DOWN", "R", "L"};
	int m = 0;
	char tmp[128];
	strncpy(tmp, s, sizeof(tmp) - 1);
	tmp[sizeof(tmp) - 1] = 0;
	for (char *t = strtok(tmp, "+"); t; t = strtok(NULL, "+")) {
		int found = 0;
		for (int i = 0; i < 10; i++)
			if (!strcmp(t, names[i])) {
				m |= 1 << i;
				found = 1;
			}
		if (!found)
			fprintf(stderr, "bad key %s\n", t);
	}
	return m;
}

#define AUDIO_RATE 32768
static FILE *audio_out;
static char audio_path[256];
static uint32_t audio_samples;

static void audio_drain(void)
{
	struct blip_t *l = core->getAudioChannel(core, 0), *r = core->getAudioChannel(core, 1);
	short buf[2 * 1024];
	int n;

	while ((n = blip_samples_avail(l)) > 0) {
		if (n > 1024)
			n = 1024;
		blip_read_samples(l, buf, n, 1);
		blip_read_samples(r, buf + 1, n, 1);
		if (audio_out) {
			fwrite(buf, 4, n, audio_out);
			audio_samples += n;
		}
	}
}

static void audio_header(FILE *f, uint32_t n)
{
	uint32_t v;
	uint16_t h;

	fwrite("RIFF", 1, 4, f); v = 36 + n * 4; fwrite(&v, 4, 1, f);
	fwrite("WAVEfmt ", 1, 8, f); v = 16; fwrite(&v, 4, 1, f);
	h = 1; fwrite(&h, 2, 1, f); h = 2; fwrite(&h, 2, 1, f);
	v = AUDIO_RATE; fwrite(&v, 4, 1, f); v = AUDIO_RATE * 4; fwrite(&v, 4, 1, f);
	h = 4; fwrite(&h, 2, 1, f); h = 16; fwrite(&h, 2, 1, f);
	fwrite("data", 1, 4, f); v = n * 4; fwrite(&v, 4, 1, f);
}

static void audio_cmd(const char *a)
{
	if (!strcmp(a, "end")) {
		if (audio_out) {
			audio_drain();
			fseek(audio_out, 0, SEEK_SET);
			audio_header(audio_out, audio_samples);
			fclose(audio_out);
			audio_out = NULL;
			printf("audio %s: %u samples\n", audio_path, audio_samples);
		}
		return;
	}
	core->setAudioBufferSize(core, 4096);
	blip_set_rates(core->getAudioChannel(core, 0), core->frequency(core), AUDIO_RATE);
	blip_set_rates(core->getAudioChannel(core, 1), core->frequency(core), AUDIO_RATE);
	audio_drain();
	audio_out = fopen(a, "wb");
	strncpy(audio_path, a, sizeof(audio_path) - 1);
	audio_samples = 0;
	audio_header(audio_out, 0);
}

static void frames(int keys, int n)
{
	core->setKeys(core, keys);
	for (int i = 0; i < n; i++) {
		core->runFrame(core);
		audio_drain();
	}
}

/*
 * Times every call to the function at addr (Thumb or ARM) over n frames.
 * A call starts when the CPU executes addr and ends when it executes the
 * return address held in LR at entry with the stack popped back. With a
 * nonzero within, only calls made while the function at within runs count.
 */
static void prof(int n, uint32_t addr, const char *name, uint32_t within)
{
	struct ARMCore *cpu = core->cpu;
	uint32_t func = addr & ~1u, ret = 0, sp = 0, start = 0;
	uint32_t outer = within & ~1u, outer_ret = 0, outer_sp = 0;
	uint64_t total = 0;
	uint32_t worst = 0, calls = 0;
	int inside = 0, in_outer = !within;

	core->setKeys(core, 0);
	for (int i = 0; i < n; i++) {
		uint32_t frame = core->frameCounter(core);

		while (core->frameCounter(core) == frame) {
			uint32_t pc = cpu->gprs[ARM_PC] - (cpu->executionMode == MODE_THUMB ? 4 : 8);

			if (within && !in_outer && pc == outer) {
				in_outer = 1;
				outer_ret = cpu->gprs[ARM_LR] & ~1u;
				outer_sp = cpu->gprs[ARM_SP];
			} else if (within && in_outer && !inside && pc == outer_ret &&
				   (uint32_t)cpu->gprs[ARM_SP] >= outer_sp) {
				in_outer = 0;
			}

			if (!inside && in_outer && pc == func) {
				inside = 1;
				ret = cpu->gprs[ARM_LR] & ~1u;
				sp = cpu->gprs[ARM_SP];
				start = mTimingCurrentTime(core->timing);
			} else if (inside && pc == ret && (uint32_t)cpu->gprs[ARM_SP] >= sp) {
				uint32_t t = (uint32_t)mTimingCurrentTime(core->timing) - start;

				inside = 0;
				calls++;
				total += t;
				if (t > worst)
					worst = t;
			}
			core->step(core);
		}
	}
	printf("prof %s: calls=%u total=%llu cycles (%.1f frames) worst=%u cycles (%.2f frames)\n",
	       name, calls, (unsigned long long)total, total / 280896.0, worst, worst / 280896.0);
}

/*
 * Runs n frames one instruction at a time and prints the lowest main stack
 * pointer (system mode). The HQ sound mixer's code sits in IWRAM right below
 * the stack, up to 0x03007708: a lower SP overwrites it and the game crashes.
 */
static void minsp(int n)
{
	struct ARMCore *cpu = core->cpu;
	uint32_t sp = 0xFFFFFFFF, pc = 0, lr = 0;

	core->setKeys(core, 0);
	for (int i = 0; i < n; i++) {
		uint32_t frame = core->frameCounter(core);

		while (core->frameCounter(core) == frame) {
			if (cpu->privilegeMode == MODE_SYSTEM && (uint32_t)cpu->gprs[ARM_SP] < sp) {
				sp = cpu->gprs[ARM_SP];
				pc = cpu->gprs[ARM_PC];
				lr = cpu->gprs[ARM_LR];
			}
			core->step(core);
		}
	}
	printf("minsp %08x (pc %08x lr %08x)\n", sp, pc, lr);
}

/* Run n frames one instruction at a time with KEYS held; print the last `keep` distinct PCs (pc, sp, mode) */
static void trace(int keys, int n, int keep)
{
	struct ARMCore *cpu = core->cpu;
	static uint32_t pcs[65536][3];
	unsigned w = 0, k;

	if (keep > 65536)
		keep = 65536;
	core->setKeys(core, keys);
	for (int i = 0; i < n; i++) {
		uint32_t frame = core->frameCounter(core);

		while (core->frameCounter(core) == frame) {
			uint32_t pc = cpu->gprs[ARM_PC];

			if (pcs[(w - 1) & 65535][0] != pc) {
				pcs[w & 65535][0] = pc;
				pcs[w & 65535][1] = cpu->gprs[ARM_SP];
				pcs[w & 65535][2] = cpu->privilegeMode;
				w++;
			}
			core->step(core);
		}
	}
	for (k = w > (unsigned)keep ? w - keep : 0; k < w; k++)
		printf("%08x sp %08x m %02x\n", pcs[k & 65535][0], pcs[k & 65535][1], pcs[k & 65535][2]);
}

/* Step with KEYS held until the CPU's PC (as mGBA reports it) is ADDR, for at most n frames */
static void runto(int keys, uint32_t addr, int n)
{
	struct ARMCore *cpu = core->cpu;

	core->setKeys(core, keys);
	for (int i = 0; i < n; i++) {
		uint32_t frame = core->frameCounter(core);

		while (core->frameCounter(core) == frame) {
			if ((uint32_t)cpu->gprs[ARM_PC] == addr) {
				printf("runto %08x: reached\n", addr);
				return;
			}
			core->step(core);
		}
	}
	printf("runto %08x: not reached\n", addr);
}

/*
 * Step until the CPU's PC (as mGBA reports it) is ADDR, for at most n frames, pressing KEYS for 2 frames after every
 * 36 frames without it; returns with no key held. Skips however many scenes are left (START) without one press too
 * many: how many frames a scene takes, and so how many presses a fixed script needs, differs between mGBA versions.
 */
static void skipto(int keys, uint32_t addr, int n)
{
	struct ARMCore *cpu = core->cpu;
	int t;

	for (int i = 0; i < n; i++) {
		uint32_t frame = core->frameCounter(core);

		t = i % 38;
		core->setKeys(core, t >= 36 ? keys : 0);
		while (core->frameCounter(core) == frame) {
			if ((uint32_t)cpu->gprs[ARM_PC] == addr) {
				core->setKeys(core, 0);
				printf("skipto %08x: reached after %d frames%s\n", addr, i, t >= 36 ? ", keys held" : "");
				return;
			}
			core->step(core);
		}
	}
	core->setKeys(core, 0);
	printf("skipto %08x: not reached\n", addr);
}

/* Run n frames a step at a time; count the cycles spent with the PC in [lo, hi) and in [lo2, hi2) */
static void cyc(uint32_t lo, uint32_t hi, uint32_t lo2, uint32_t hi2, int n)
{
	struct ARMCore *cpu = core->cpu;
	uint64_t in = 0, all = 0;
	uint32_t worst = 0, cur = 0;

	core->setKeys(core, 0);
	for (int i = 0; i < n; i++) {
		uint32_t frame = core->frameCounter(core);

		cur = 0;
		while (core->frameCounter(core) == frame) {
			uint32_t pc = cpu->gprs[ARM_PC], t0 = (uint32_t)mTimingCurrentTime(core->timing), t;
			int hit = (pc >= lo && pc < hi) || (pc >= lo2 && pc < hi2);

			core->step(core);
			t = (uint32_t)mTimingCurrentTime(core->timing) - t0;
			all += t;
			if (hit) {
				in += t;
				cur += t;
			}
		}
		if (cur > worst)
			worst = cur;
	}
	printf("cyc: %llu cycles in range over %d frames (%.1f%% of %llu), worst frame %u (%.1f%%)\n", (unsigned long long)in, n,
	       100.0 * in / (all ? all : 1), (unsigned long long)all, worst, 100.0 * worst / 280896);
}

static void shot(const char *fn)
{
	FILE *f = fopen(fn, "wb");
	png_structp p = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	png_infop info = png_create_info_struct(p);
	png_init_io(p, f);
	png_set_IHDR(p, info, W, H, 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
		     PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
	png_write_info(p, info);
	unsigned char *row = malloc(W * 3);
	for (unsigned y = 0; y < H; y++) {
		for (unsigned x = 0; x < W; x++) {
			uint32_t c = vbuf[y * W + x];
			row[x * 3 + 0] = c & 0xFF;
			row[x * 3 + 1] = (c >> 8) & 0xFF;
			row[x * 3 + 2] = (c >> 16) & 0xFF;
		}
		png_write_row(p, row);
	}
	png_write_end(p, NULL);
	png_destroy_write_struct(&p, &info);
	fclose(f);
	free(row);
	printf("shot %s\n", fn);
}

static void nolog(struct mLogger *l, int cat, enum mLogLevel lvl, const char *fmt, va_list args) {}
static struct mLogger quiet = { .log = nolog };

int main(int argc, char **argv)
{
	mLogSetDefaultLogger(&quiet);
	if (argc < 3) {
		fprintf(stderr, "usage: %s rom script\n", argv[0]);
		return 1;
	}
	core = mCoreFind(argv[1]);
	if (!core || !core->init(core))
		return 1;
	mCoreInitConfig(core, NULL);
	core->desiredVideoDimensions(core, &W, &H);
	vbuf = calloc(W * H, sizeof(uint32_t));
	core->setVideoBuffer(core, (void *)vbuf, W);
	if (!mCoreLoadFile(core, argv[1]))
		return 1;

	FILE *sf = fopen(argv[2], "r");
	char line[512];
	int started = 0;
	while (fgets(line, sizeof(line), sf)) {
		char cmd[32], a[256], b[256], c[256], d[256];
		int n = sscanf(line, "%31s %255s %255s %255s", cmd, a, b, c);
		if (n <= 0 || cmd[0] == '#')
			continue;
		if (!strcmp(cmd, "sram")) {
			struct VFile *vf = VFileOpen(a, O_RDWR);
			core->loadSave(core, vf);
			continue;
		}
		if (!started) {
			core->reset(core);
			started = 1;
		}
		if (!strcmp(cmd, "wait"))
			frames(0, atoi(a));
		else if (!strcmp(cmd, "press")) {
			frames(keymask(a), n >= 3 ? atoi(b) : 2);
			frames(0, 6);
		} else if (!strcmp(cmd, "hold"))
			frames(keymask(a), atoi(b));
		else if (!strcmp(cmd, "shot"))
			shot(a);
		else if (!strcmp(cmd, "save")) {
			size_t sz = core->stateSize(core);
			void *buf = malloc(sz);
			core->saveState(core, buf);
			FILE *o = fopen(a, "wb");
			fwrite(buf, 1, sz, o);
			fclose(o);
			free(buf);
		} else if (!strcmp(cmd, "load")) {
			size_t sz = core->stateSize(core);
			void *buf = malloc(sz);
			FILE *i = fopen(a, "rb");
			if (!i || fread(buf, 1, sz, i) != sz) {
				fprintf(stderr, "bad state %s\n", a);
				return 1;
			}
			fclose(i);
			core->loadState(core, buf);
			free(buf);
		} else if (!strcmp(cmd, "w8"))
			core->busWrite8(core, strtoul(a, 0, 0), strtoul(b, 0, 0));
		else if (!strcmp(cmd, "w16"))
			core->busWrite16(core, strtoul(a, 0, 0), strtoul(b, 0, 0));
		else if (!strcmp(cmd, "w32"))
			core->busWrite32(core, strtoul(a, 0, 0), strtoul(b, 0, 0));
		else if (!strcmp(cmd, "r8"))
			printf("%s = 0x%02X\n", a, core->busRead8(core, strtoul(a, 0, 0)));
		else if (!strcmp(cmd, "r16"))
			printf("%s = 0x%04X\n", a, core->busRead16(core, strtoul(a, 0, 0)));
		else if (!strcmp(cmd, "r32"))
			printf("%s = 0x%08X\n", a, core->busRead32(core, strtoul(a, 0, 0)));
		else if (!strcmp(cmd, "dump")) {
			uint32_t ad = strtoul(a, 0, 0), len = strtoul(b, 0, 0);
			FILE *o = fopen(c, "wb");
			for (uint32_t k = 0; k < len; k++)
				fputc(core->busRead8(core, ad + k), o);
			fclose(o);
		} else if (!strcmp(cmd, "prof"))
			prof(atoi(a), strtoul(b, 0, 0), n >= 4 ? c : b,
			     sscanf(line, "%*s %*s %*s %*s %255s", d) == 1 ? strtoul(d, 0, 0) : 0);
		else if (!strcmp(cmd, "minsp"))
			minsp(atoi(a));
		else if (!strcmp(cmd, "runto"))
			runto(keymask(a), strtoul(b, 0, 0), atoi(c));
		else if (!strcmp(cmd, "skipto"))
			skipto(keymask(a), strtoul(b, 0, 0), atoi(c));
		else if (!strcmp(cmd, "cyc")) {
			char e2[64] = "0", e3[64] = "0", e4[64] = "1";
			sscanf(line, "%*s %*s %*s %63s %63s %63s", e2, e3, e4);
			if (sscanf(line, "%*s %*s %*s %*s %*s %63s", e4) != 1)
				strcpy(e4, e2), strcpy(e2, "0"), strcpy(e3, "0");
			cyc(strtoul(a, 0, 0), strtoul(b, 0, 0), strtoul(e2, 0, 0), strtoul(e3, 0, 0), atoi(e4));
		} else if (!strcmp(cmd, "trace"))
			trace(keymask(a), atoi(b), atoi(c));
		else if (!strcmp(cmd, "regs")) {
			struct ARMCore *cpu = core->cpu;
			for (int k = 0; k < 16; k++)
				printf("r%d=%08X%s", k, cpu->gprs[k], k % 4 == 3 ? "\n" : " ");
			printf("cpsr=%08X\n", cpu->cpsr.packed);
		} else if (!strcmp(cmd, "audio"))
			audio_cmd(a);
		else
			fprintf(stderr, "unknown %s\n", cmd);
	}
	audio_cmd("end");
	core->deinit(core);
	return 0;
}
