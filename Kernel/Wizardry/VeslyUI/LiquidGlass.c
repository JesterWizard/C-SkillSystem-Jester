#include "common-chax.h"
#include "kernel-lib.h"

/*
 * Liquid Glass UI style (png/LiquidGlass/generate_liquidglass.py): windows are tinted glass the map shows through.
 *
 * The GBA can blend one background layer over the layers below it. Menus and windows are drawn on BG1 (their text on
 * BG0, above it), and the game already marks BG1 as the blend's first layer, with the map layers as the second, but
 * leaves the effect off. With this style, FlushLCDControl, which copies the game's display settings
 * (gLCDControlBuffer) to the hardware every frame, turns the effect on for BG1 there: the window is mixed with what is
 * behind it. The game's own settings are not changed, so a screen that sets up another blend keeps it.
 */
int GetUI_id(void);

#define LIQUID_GLASS_UI_ID	12

/* Of 16: how much of the window shows, and how much of what is behind it */
#define LIQUID_GLASS_WINDOW	11
#define LIQUID_GLASS_BEHIND	7

#define BLD_TARGET1_MASK	0x003F
#define BLD_TARGET1_BG1		0x0002
#define BLD_EFFECT_MASK		0x00C0
#define BLD_EFFECT_ALPHA	0x0040
#define BLD_TARGET2_MASK	0x3F00

/* Vanilla's copy, field by field, in vanilla's order */
LYN_REPLACE_CHECK(FlushLCDControl);
void FlushLCDControl(void)
{
	const u8 *buf = (const u8 *)&gLCDControlBuffer;
	u8 *io = (u8 *)REG_BASE;
	u16 bldcnt = *(const u16 *)(buf + 0x3C);
	u16 bldalpha = *(const u16 *)(buf + 0x44);
	int i;

	*(vu16 *)(io + 0x00) = *(const u16 *)(buf + 0x00);	/* DISPCNT */
	*(vu16 *)(io + 0x04) = *(const u16 *)(buf + 0x04);	/* DISPSTAT */
	*(vu16 *)(io + 0x08) = *(const u16 *)(buf + 0x0C);	/* BG0CNT-BG3CNT */
	*(vu16 *)(io + 0x0A) = *(const u16 *)(buf + 0x10);
	*(vu16 *)(io + 0x0C) = *(const u16 *)(buf + 0x14);
	*(vu16 *)(io + 0x0E) = *(const u16 *)(buf + 0x18);

	for (i = 0; i < 4; i++)					/* BG0-BG3 offsets */
		*(vu32 *)(io + 0x10 + 4 * i) = *(const u32 *)(buf + 0x1C + 4 * i);

	*(vu32 *)(io + 0x40) = *(const u32 *)(buf + 0x2C);	/* WIN0H, WIN1H */
	*(vu32 *)(io + 0x44) = *(const u32 *)(buf + 0x30);	/* WIN0V, WIN1V */
	*(vu32 *)(io + 0x48) = *(const u32 *)(buf + 0x34);	/* WININ, WINOUT */
	*(vu16 *)(io + 0x4C) = *(const u16 *)(buf + 0x38);	/* MOSAIC */

	if ((bldcnt & (BLD_TARGET1_MASK | BLD_EFFECT_MASK)) == BLD_TARGET1_BG1 && (bldcnt & BLD_TARGET2_MASK) &&
	    GetUI_id() == LIQUID_GLASS_UI_ID) {
		bldcnt |= BLD_EFFECT_ALPHA;
		bldalpha = LIQUID_GLASS_WINDOW | (LIQUID_GLASS_BEHIND << 8);
	}

	*(vu16 *)(io + 0x50) = bldcnt;				/* BLDCNT */
	*(vu16 *)(io + 0x52) = bldalpha;			/* BLDALPHA */
	*(vu8 *)(io + 0x54) = buf[0x46];			/* BLDY */

	for (i = 0; i < 8; i++)					/* BG2 and BG3 affine */
		*(vu32 *)(io + 0x20 + 4 * i) = *(const u32 *)(buf + 0x48 + 4 * i);
}
