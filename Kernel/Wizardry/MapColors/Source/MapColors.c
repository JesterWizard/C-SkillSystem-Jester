#include "common-chax.h"
#include "map-colors.h"

/*
 * Map colours: richer colours for the map tiles, picked on the Options screen (gPlaySt.config.map_colors). The map's
 * palettes (BG palettes 6 to 15) go through MapColors_Transform when UnpackChapterMapPalette loads them
 * (EnterTown/DistrictLife.c) and when the map's palette animations write them.
 */

struct MapColorsParams {
	u8 saturation;		/* of 16 */
	u8 contrast;		/* of 16, around the middle (16) */
	const u8 *curve;	/* NULL, or a 32-entry curve applied last */
};

/* round(31 * (i / 31) ^ 1.25): darker mid-tones */
static const u8 sDeepCurve[32] = {
	0, 0, 1, 2, 2, 3, 4, 5, 6, 7, 8, 8, 9, 10, 11, 13,
	14, 15, 16, 17, 18, 19, 20, 21, 23, 24, 25, 26, 27, 29, 30, 31,
};

static const struct MapColorsParams sMapColorsParams[MAP_COLORS_MODES] = {
	[MAP_COLORS_VANILLA] = { 16, 16, NULL },
	[MAP_COLORS_VIVID]   = { 22, 17, NULL },
	[MAP_COLORS_DEEP]    = { 26, 19, sDeepCurve },
};

static int MapColors_Channel(int c, int luma, const struct MapColorsParams *p)
{
	c = luma + (((c - luma) * p->saturation) >> 4);
	c = 16 + (((c - 16) * p->contrast) >> 4);
	if (c < 0)
		c = 0;
	if (c > 31)
		c = 31;
	return p->curve ? p->curve[c] : c;
}

u16 MapColors_Transform(u16 color, int mode)
{
	const struct MapColorsParams *p;
	int r = color & 0x1F, g = (color >> 5) & 0x1F, b = (color >> 10) & 0x1F;
	int luma = (r * 77 + g * 150 + b * 29) >> 8;

	if (mode <= MAP_COLORS_VANILLA || mode >= MAP_COLORS_MODES)
		return color;

	p = &sMapColorsParams[mode];
	return MapColors_Channel(r, luma, p) | (MapColors_Channel(g, luma, p) << 5) |
	       (MapColors_Channel(b, luma, p) << 10);
}

/* Recolours count palette entries by the Options value */
void MapColors_ApplyRange(u16 *pal, int count)
{
	int i, mode = gPlaySt.config.map_colors;

	if (mode <= MAP_COLORS_VANILLA || mode >= MAP_COLORS_MODES)
		return;

	for (i = 0; i < count; i++)
		pal[i] = MapColors_Transform(pal[i], mode);
}

struct MapPalAnimEntry {
	const u16 *src;
	u8 duration;
	u8 count;
	u8 index;
	u8 pad;
};

void BMapVSync_UpdateMapPalAnimations(ProcPtr proc);

/* Vanilla's map palette animation step, with the animated colours recoloured too */
LYN_REPLACE_CHECK(BMapVSync_UpdateMapPalAnimations);
void BMapVSync_UpdateMapPalAnimations(ProcPtr proc)
{
	u8 *p = proc;
	s16 *timer = (s16 *)(p + 0x36);
	const struct MapPalAnimEntry *start = *(const struct MapPalAnimEntry **)(p + 0x38);
	const struct MapPalAnimEntry **cur = (const struct MapPalAnimEntry **)(p + 0x3C);
	const struct MapPalAnimEntry *it;

	if (!start)
		return;

	if (*timer != 0) {
		(*timer)--;
		return;
	}

	it = *cur;
	*timer = it->duration;
	CpuSet(it->src, gPaletteBuffer + 0x60 + it->index, it->count);
	MapColors_ApplyRange(gPaletteBuffer + 0x60 + it->index, it->count);
	EnablePaletteSync();

	it++;
	*cur = it->duration ? it : start;
}
