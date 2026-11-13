#pragma once

#include "common-chax.h"

/*
 * Helpers the Extras screens share (the Bestiary, the Skill Codex): Kernel/Wizardry/Bestiary/Source/ExtrasUi.c.
 *
 * The Sound Room's song banner goes on BG2, tiles 3-26 of rows 0-3, its tiles from BG tile EXTRAS_UI_BANNER_TILE (after
 * the text's) and BG palette 11; tiles from EXTRAS_UI_FREE_TILE up to the tilemaps (0x300) are the screen's.
 */
#define EXTRAS_UI_BANNER_TILE 0x220
#define EXTRAS_UI_FREE_TILE   (EXTRAS_UI_BANNER_TILE + 24 * 4)

char *ExtrasUi_PutNumber(char *buf, int n);
char *ExtrasUi_Append(char *dst, const char *src);
void ExtrasUi_PutCentred(struct Text *t, int px, int color, const char *str, u16 *tm);
void ExtrasUi_PutLeft(struct Text *t, int color, const char *str, u16 *tm);
void ExtrasUi_LoadBanner(void);
bool ExtrasUi_DrawWrapped(struct Text *lines, int n, int px, int margin, const char *src, u16 *tm);
