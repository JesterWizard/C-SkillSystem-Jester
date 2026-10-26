#include "global.h"
#include "proc.h"
#include "hardware.h"
#include "functions.h"
#include "ctc.h"
#include "popup.h"
#include "bmunit.h"

/**
 * Item icon shine: while a popup shows an item's icon (an item got, stolen,
 * dropped...), a white glint sweeps across the icon about once a second.
 * The glint is a "\" band moving left to right, so it runs up the "/" blade
 * of a weapon icon from hilt to tip.
 *
 * Only items the player acquires shine: popups about an item being lost,
 * or about another faction's unit getting one, keep a plain icon.
 *
 * The icon is a 16x16 sprite whose tiles are loaded once from the icon
 * sheet. During a sweep they are rewritten each frame from the sheet with
 * the glint drawn over the icon's opaque pixels, and put back when it ends.
 */

enum popup_icon_shine {
	/* Frames before the first sweep, so it starts once the popup is up. */
	SHINE_DELAY = 8,
	/* One sweep every SHINE_PERIOD frames (a power of two). */
	SHINE_PERIOD = 64,
	/* The glint is SHINE_WIDTH pixels wide along x - y, and moves SHINE_SPEED a frame. */
	SHINE_WIDTH = 3,
	SHINE_SPEED = 2,
	/* x - y runs -15 to 15 over a 16x16 icon: start and finish just off it. */
	SHINE_START = -15 - SHINE_WIDTH,
	SHINE_FRAMES = (30 + SHINE_WIDTH * 2) / SHINE_SPEED + 1,
	/* An icon proc not updated for this long is new to its slot. */
	SHINE_STALE_FRAMES = 4,
};

/* ProcScr_PopupUpdateIcon's proc, using its padding for the shine. */
struct PopupIconShineProc {
	PROC_HEADER;
	/* 2C */ int x;
	/* 30 */ int y;
	/* 34 */ u32 lastClock;
	/* 38 */ u16 clock;
	/* 3A */ STRUCT_PAD(0x3A, 0x4A);
	/* 4A */ u16 oam2;
};

/* Vanilla static (FE8U 0x030005F0): the unit named by POPUP_UNIT_NAME. */
extern struct Unit *gpPopupUnit;

/*
 * Vanilla LoadIconObjectGraphics (FE8U 0x0800372C) reads the item icon sheet
 * (item_icon_tiles, 0x085926F4) from its literal pool at 0x08003788. Reading
 * the same pointer keeps the shine in step with a repointed icon sheet.
 */
#define LoadIconObjectGraphics_IconSheet (*(const u8 *const *)0x08003788)

/* Messages of popups that show an item being lost. */
static const u32 sPopupIconShineLossMsgs[] = {
	0x003, /* "broke!" (PopupScr_WpnBroken) */
	0x009, /* "was pilfered." (PopupScr_ItemWasPilfered) */
	0x00B, /* "was stolen." (PopupScr_ItemStolen) */
};

static bool PopupShowsAcquiredItem(struct PopupProc *popup)
{
	const struct PopupInstruction *inst;
	bool itemIcon = false;
	int i;

	if (popup == NULL || popup->proc_script != ProcScr_Popup || popup->iconId == 0xFFFF)
		return false;

	for (inst = popup->pDefinition; inst->opcode != POPUP_OP_END; inst++) {
		switch (inst->opcode) {
		case POPUP_OP_ITEM_ICON:
			itemIcon = true;
			break;

		/* "[Unit] got/stole/pilfered": only a player unit's gain counts. */
		case POPUP_OP_UNIT_NAME:
			if (gpPopupUnit == NULL || UNIT_FACTION(gpPopupUnit) != FACTION_BLUE)
				return false;
			break;

		case POPUP_OP_MSG:
			for (i = 0; i < (int)ARRAY_COUNT(sPopupIconShineLossMsgs); i++)
				if (inst->data == sPopupIconShineLossMsgs[i])
					return false;
			break;
		}
	}

	return itemIcon;
}

/* The glint takes the palette's brightest colour, normally its white. */
static int PopupIconShine_BrightestColor(int pal)
{
	const u16 *colors = gPaletteBuffer + (0x10 + pal) * 0x10;
	int i, best = 1, bestLuma = -1;

	for (i = 1; i < 0x10; i++) {
		int c = colors[i];
		int luma = (c & 0x1F) * 2 + ((c >> 5) & 0x1F) * 4 + ((c >> 10) & 0x1F);

		if (luma > bestLuma) {
			bestLuma = luma;
			best = i;
		}
	}

	return best;
}

/*
 * Writes the icon's four tiles from the sheet, with opaque pixels where
 * pos <= x - y < pos + SHINE_WIDTH set to colour bright. OBJ VRAM only takes
 * 16/32-bit writes, so each row is built in a register.
 */
static void PopupIconShine_PutTiles(const u8 *src, u8 *dst, int pos, int bright)
{
	int tile, row, px;

	for (tile = 0; tile < 4; tile++) {
		const u32 *in = (const u32 *)(src + tile * CHR_SIZE);
		/* 2D OBJ mapping: the bottom row of the sprite is 32 tiles on. */
		u32 *out = (u32 *)(dst + (tile & 1) * CHR_SIZE + (tile >> 1) * 0x400);
		int origin = (tile & 1) * 8 - (tile >> 1) * 8 - pos;

		for (row = 0; row < 8; row++) {
			u32 bits = in[row];

			for (px = 0; px < 8; px++) {
				int k = origin + px - row;

				if (k < 0 || k >= SHINE_WIDTH)
					continue;

				if (((bits >> (px * 4)) & 0xF) == 0)
					continue;

				bits = (bits & ~(0xFu << (px * 4))) | ((u32)bright << (px * 4));
			}

			out[row] = bits;
		}
	}
}

static void PopupIconShine_Update(struct PopupIconShineProc *proc)
{
	struct PopupProc *popup = proc->proc_parent;
	u32 now = GetGameClock();
	const u8 *src;
	u8 *dst;
	int t;

	/* Procs don't clear their memory: a stale clock means a new icon. */
	if (now - proc->lastClock > SHINE_STALE_FRAMES)
		proc->clock = 0;

	proc->lastClock = now;
	t = proc->clock++;

	if (t < SHINE_DELAY || !PopupShowsAcquiredItem(popup))
		return;

	t = (t - SHINE_DELAY) & (SHINE_PERIOD - 1);
	if (t > SHINE_FRAMES)
		return;

	/* Same source as vanilla LoadIconObjectGraphics: 0x80 bytes per icon. */
	src = LoadIconObjectGraphics_IconSheet + popup->iconId * 0x80;
	dst = OBJ_VRAM0 + CHR_SIZE * (proc->oam2 & 0x3FF);

	/* The last frame puts the plain icon back. */
	PopupIconShine_PutTiles(
		src, dst,
		t == SHINE_FRAMES ? -0x100 : SHINE_START + t * SHINE_SPEED,
		PopupIconShine_BrightestColor((proc->oam2 >> 12) & 0xF)
	);
}

/**
 * Replaces vanilla PopupIconUpdateProc_Loop (FE8U 0x08011254).
 */
void PopupIconUpdateProc_Loop_Shine(struct PopupIconUpdateProc *vproc)
{
	struct PopupIconShineProc *proc = (struct PopupIconShineProc *)vproc;

	CallARM_PushToSecondaryOAM(proc->x, proc->y, gObject_16x16, proc->oam2);
	PopupIconShine_Update(proc);
}
