#include "common-chax.h"
#include "kernel-lib.h"
#include "extras-ui.h"

/* Helpers the Extras screens share (include/kernel/extras-ui.h) */

#define BANNER_X 3
#define BANNER_W 24
#define BANNER_H 4
#define PAL_BANNER 11
#define OBJ_TEMP ((u8 *)0x06014000) /* OBJ VRAM these screens leave free: the banner's sheet is unpacked here */

/* A number into buf (no % here: it is broken in this build, CLAUDE.md, Pitfalls); returns the end */
char *ExtrasUi_PutNumber(char *buf, int n)
{
	if (n < 0) {
		*buf++ = '-';
		n = -n;
	}
	if (n >= 100)
		*buf++ = '0' + Div(n, 100);
	if (n >= 10)
		*buf++ = '0' + DivRem(Div(n, 10), 10);
	*buf++ = '0' + DivRem(n, 10);
	*buf = '\0';
	return buf;
}

char *ExtrasUi_Append(char *dst, const char *src)
{
	while (*src)
		*dst++ = *src++;
	*dst = '\0';
	return dst;
}

void ExtrasUi_PutCentred(struct Text *t, int px, int color, const char *str, u16 *tm)
{
	int w = GetStringTextLen(str);

	ClearText(t);
	Text_InsertDrawString(t, w < px ? (px - w) / 2 : 0, color, str);
	PutText(t, tm);
}

void ExtrasUi_PutLeft(struct Text *t, int color, const char *str, u16 *tm)
{
	ClearText(t);
	Text_InsertDrawString(t, 0, color, str);
	PutText(t, tm);
}

/* The Sound Room's song banner: its sheet into spare OBJ VRAM, the 24x4 tiles of the banner on to BG VRAM */
void ExtrasUi_LoadBanner(void)
{
	int x, y;

	Decompress(Img_PlayStatusSprites, OBJ_TEMP);

	for (y = 0; y < BANNER_H; y++)
		CpuFastCopy(OBJ_TEMP + y * 32 * 0x20, (u8 *)VRAM + (EXTRAS_UI_BANNER_TILE + y * BANNER_W) * 0x20,
			    BANNER_W * 0x20);

	ApplyPalette(Pal_PlayStatusSprites, PAL_BANNER);

	for (y = 0; y < BANNER_H; y++)
		for (x = 0; x < BANNER_W; x++)
			gBG2TilemapBuffer[TILEMAP_INDEX(BANNER_X + x, y)] =
				(PAL_BANNER << 12) | (EXTRAS_UI_BANNER_TILE + y * BANNER_W + x);
}

/*
 * A text wrapped to px pixels in n lines, each a Text, put two tile rows apart from tm: line breaks (and other control
 * bytes) become spaces, and words go on a line while it fits. The rest is left out. NULL clears the lines.
 */
void ExtrasUi_DrawWrapped(struct Text *lines, int n, int px, const char *src, u16 *tm)
{
	char text[0x200], line[0x60];
	int i, row = 0, len = 0, word;

	for (i = 0; i < n; i++)
		ClearText(&lines[i]);

	if (src != NULL) {
		for (i = 0; src[i] != '\0' && i < (int)sizeof(text) - 1; i++)
			text[i] = (u8)src[i] < 0x20 ? ' ' : src[i];
		text[i] = '\0';

		src = text;
		line[0] = '\0';

		while (*src != '\0' && row < n) {
			while (*src == ' ')
				src++;
			if (*src == '\0')
				break;

			for (word = 0; src[word] != '\0' && src[word] != ' '; word++)
				;

			if (len + 1 + word >= (int)sizeof(line))
				word = sizeof(line) - 2 - len;

			/* The line with the word: does it fit? */
			if (len != 0)
				line[len] = ' ';
			memcpy(line + len + (len != 0), src, word);
			line[len + (len != 0) + word] = '\0';

			if (len != 0 && GetStringTextLen(line) > px) {
				/* No: the line goes out as it was, the word starts the next */
				line[len] = '\0';
				Text_InsertDrawString(&lines[row++], 0, TEXT_COLOR_SYSTEM_WHITE, line);
				len = 0;
				continue;
			}

			len += (len != 0) + word;
			src += word;
		}

		if (len != 0 && row < n)
			Text_InsertDrawString(&lines[row], 0, TEXT_COLOR_SYSTEM_WHITE, line);
	}

	for (i = 0; i < n; i++)
		PutText(&lines[i], tm + i * TILEMAP_INDEX(0, 2));
}
