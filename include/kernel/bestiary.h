#pragma once

#include "common-chax.h"

/*
 * The Bestiary (Documentation/Features/Bestiary.md): Extras > Bestiary, every character and class the player has met.
 */

/* The Extras row: vanilla's trial-map bit, which this hack never shows */
#define EXTRA_MENU_BESTIARY        6
#define EXTRA_MENU_OPTION_BESTIARY (1 << EXTRA_MENU_BESTIARY)

#define BESTIARY_MAGIC 0x59545342 /* "BSTY" */

/* One bit per character id and per class id met; in RAM and in each save's tail (SaveTail.c) */
struct BestiaryBits {
	u32 magic;
	u8 chars[0x100 / 8];
	u8 classes[0x100 / 8];
};

extern struct BestiaryBits gBestiarySeen;

/* The lists the screen shows, in order: character ids and class ids, each ended by 0 (Data/Bestiary/Bestiary.c) */
extern const u8 gBestiaryCharacters[];
extern const u8 gBestiaryClasses[];

bool Bestiary_BitSet(const u8 *bits, int id);
void Bestiary_ReadCart(struct BestiaryBits *bits);
void Bestiary_MarkSeen(int pid, int jid);
void Bestiary_OnNewGame(void);
bool Bestiary_PrePhase(ProcPtr proc);
void Bestiary_SaveTail(u8 *dst, const u32 size);
void Bestiary_LoadTail(u8 *src, const u32 size);

/* Kernel/Wizardry/SaveData/Source/SaveTail.c: the Bestiary's share of a game save's (or the suspend save's) tail */
u32 SaveTail_BestiaryOffset(bool suspend);

ProcPtr StartBestiaryScreen(ProcPtr parent);
