#pragma once

#include "common-chax.h"

/*
 * The Skill Codex (Documentation/Features/SkillCodex.md): Extras > Skill Codex, every skill the player has met.
 */

/* The Extras row: the last bit of the Extras list's byte, which vanilla never used */
#define EXTRA_MENU_SKILL_CODEX        7
#define EXTRA_MENU_OPTION_SKILL_CODEX (1 << EXTRA_MENU_SKILL_CODEX)

#define SKILL_CODEX_MAGIC 0x58444353 /* "SCDX" */
#define SKILL_CODEX_SIDS  0x400

/* One bit per skill id met; in RAM and in each save's tail (Kernel/Wizardry/SaveData/Source/SaveTail.c) */
struct SkillCodexBits {
	u32 magic;
	u8 skills[SKILL_CODEX_SIDS / 8];
};

extern struct SkillCodexBits gSkillCodexSeen;

bool SkillCodex_Listed(int sid);
bool SkillCodex_BitSet(const u8 *bits, int sid);
void SkillCodex_ReadCart(struct SkillCodexBits *bits);
void SkillCodex_MarkSeen(int sid);
void SkillCodex_OnNewGame(void);
bool SkillCodex_PrePhase(ProcPtr proc);
void SkillCodex_SaveTail(u8 *dst, const u32 size);
void SkillCodex_LoadTail(u8 *src, const u32 size);

/* SaveTail.c: the Skill Codex's share of a game save's (or the suspend save's) tail */
u32 SaveTail_SkillCodexOffset(bool suspend);

ProcPtr StartSkillCodexScreen(ProcPtr parent);
