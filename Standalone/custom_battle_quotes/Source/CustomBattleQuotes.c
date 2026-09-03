#include "global.h"
#include "eventinfo.h"
#include "event.h"
#include "bmbattle.h"
#include "variables.h"
#include "constants/characters.h"
#include "constants/chapters.h"
#include "constants/event-flags.h"

extern const u8 CustomBattleQuotes_UseVanillaFallback;

/*
 * Edit this table, then run `make` in Standalone/custom_battle_quotes.
 *
 * Fields:
 *   pidA    - first character ID (0 = wildcard for pidB-only entries)
 *   pidB    - second character ID (0 = wildcard for pidA-only entries)
 *   chapter - chapter index, CHAPTER_FF for any chapter, 0xFE for triangle attacks
 *   flag    - permanent flag set after the quote plays; skipped if already set
 *   msg     - text ID (0 = use `event` instead)
 *   event   - custom battle event script (used when msg is 0)
 */
const struct BattleTalkExtEnt gCustomBattleTalkList[] = {
	/* Example: Eirika vs O'Neill on the prologue */
	/*
	{
		.pidA    = CHARACTER_EIRIKA,
		.pidB    = CHARACTER_ONEILL,
		.chapter = CHAPTER_L_PROLOGUE,
		.flag    = EVFLAG_BATTLE_QUOTES,
		.msg     = 0x0000,
	},
	*/
	{
		.pidA    = 0xFFFF,
		.chapter = 0x00,
		.msg     = 0x0000,
	},
};

static struct BattleTalkExtEnt* SearchBattleTalkList(
	const struct BattleTalkExtEnt* it, u16 pidA, u16 pidB)
{
	for (; it->pidA != 0xFFFF; it++) {
		if (it->chapter != 0xff && it->chapter != gPlaySt.chapterIndex) {
			if (it->chapter != 0xfe || BattleIsTriangleAttack() != 1)
				continue;
		}

		if (GetEventTriggerState(it->flag))
			continue;

		if (it->pidA != 0) {
			if (it->pidB == 0) {
				if (pidA == it->pidA)
					return (struct BattleTalkExtEnt *)it;
				continue;
			}
		} else {
			if (it->pidB == 0)
				continue;

			if (pidB == it->pidB)
				return (struct BattleTalkExtEnt *)it;

			continue;
		}

		if ((pidA == it->pidA) && (pidB == it->pidB))
			return (struct BattleTalkExtEnt *)it;

		if ((pidB == it->pidA) && (pidA == it->pidB))
			return (struct BattleTalkExtEnt *)it;
	}

	return NULL;
}

/**
 * Vanilla GetBattleQuoteEntry with dual-character matching against a custom table.
 */
struct BattleTalkExtEnt* GetBattleQuoteEntry_Custom(u16 pidA, u16 pidB)
{
	struct BattleTalkExtEnt* ent;

	ent = SearchBattleTalkList(gCustomBattleTalkList, pidA, pidB);
	if (ent != NULL)
		return ent;

	if (CustomBattleQuotes_UseVanillaFallback)
		return SearchBattleTalkList(gBattleTalkList, pidA, pidB);

	return NULL;
}
