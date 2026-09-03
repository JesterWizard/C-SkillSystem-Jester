#include "global.h"
#include "eventinfo.h"
#include "event.h"
#include "bmbattle.h"
#include "variables.h"

extern const u8 CustomBattleQuotes_UseVanillaFallback;
extern const struct BattleTalkExtEnt gCustomBattleTalkList[];

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
