#pragma once

#include "common-chax.h"

/*
 * New Game+ (Documentation/Features/NewGamePlus.md): Extras > New Game+ starts a new game from a save that has been
 * to the Creature Campaign. Each character keeps the skills they had equipped there and their skill list.
 */

/* The Extras row: the unused "Battle History" slot of vanilla's Extras list (bit 3) */
#define EXTRA_MENU_NEW_GAME_PLUS        3
#define EXTRA_MENU_OPTION_NEW_GAME_PLUS (1 << EXTRA_MENU_NEW_GAME_PLUS)

/* Characters 1-0x3F keep their equipped skills */
#define NGP_PID_MAX 0x40

#define NGP_MAGIC 0x2B50474E /* "NGP+" */

struct NewGamePlusCarry {
	/* 00 */ u32 magic;
	/* 04 */ u8 playthrough;	/* gPlaySt.playthroughIdentifier of the New Game+ game */
	/* 05 */ u8 active;
	/* 06 */ u8 _pad[2];
	/* 08 */ u32 carried[2];	/* bit pid: the source save has the character */
	/* 10 */ u32 pending[2];	/* bit pid: the character has not joined yet */
	/* 18 */ u8 equips[NGP_PID_MAX][7];	/* the source save's struct Unit::supports (the equipped skills) */
};

/* The suspend save keeps only which characters have not joined; the skills are in the game save */
struct NewGamePlusSusCarry {
	u32 magic;
	u8 playthrough;
	u8 active;
	u8 _pad[2];
	u32 pending[2];
};

extern struct NewGamePlusCarry gNewGamePlus;
extern u32 gNewGamePlusArmed;

bool NewGamePlus_SlotReachedPostgame(int slot);
bool NewGamePlus_IsExtraEnabled(void);
void NewGamePlus_Arm(int slot);
void NewGamePlus_Disarm(void);

/* gNewSaveHooks */
void NewGamePlus_OnNewGame(void);

/* gLoadUnitHooks, gClearUnitHooks, gPrePhaseFuncs, gPostActionFuncs */
int NewGamePlus_OnLoadUnit(struct Unit *unit, const struct CharacterData *character);
int NewGamePlus_OnClearUnit(struct Unit *unit);
bool NewGamePlus_PrePhase(ProcPtr proc);
bool NewGamePlus_PostAction(ProcPtr proc);

/* The bytes after the save tables (gpMsa_Saver and friends), so the save layout does not change */
void NewGamePlus_SaveSav(u8 *dst, const u32 size);
void NewGamePlus_LoadSav(u8 *src, const u32 size);
void NewGamePlus_SaveSus(u8 *dst, const u32 size);
void NewGamePlus_LoadSus(u8 *src, const u32 size);
