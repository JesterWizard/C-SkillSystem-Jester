#include "global.h"
#include "eventinfo.h"
#include "event.h"
#include "bmbattle.h"
#include "bmunit.h"
#include "soundwrapper.h"
#include "constants/songs.h"

/**
 * Custom defeat quote entry (matches the C Skill System's DefeatTalkEntNew).
 *
 * pidB = 0xFFFF matches any opponent; otherwise the quote only plays when
 * pidB is the current battle actor or target.
 */
struct CustomDefeatTalkEnt {
	/* 00 */ u16 pidA;
	/* 02 */ u16 pidB;
	/* 04 */ u8 route;
	/* 05 */ u8 chapter;
	/* 06 */ u16 flag;
	/* 08 */ u16 msg;
	/* 0C */ const u16 *event;
};

extern const u8 CustomDefeatQuotes_UseVanillaFallback;
extern const struct CustomDefeatTalkEnt gCustomDefeatTalkList[];

/* What both table formats boil down to. */
struct DefeatTalkResult {
	u16 route;
	u16 flag;
	u16 msg;
	const u16 *event;
};

static bool IsBattlePid(u16 pid)
{
	const struct CharacterData *actor = gBattleActor.unit.pCharacterData;
	const struct CharacterData *target = gBattleTarget.unit.pCharacterData;

	if (actor != NULL && actor->number == pid)
		return true;

	if (target != NULL && target->number == pid)
		return true;

	return false;
}

static const struct CustomDefeatTalkEnt *GetCustomDefeatTalkEntry(u16 pidA)
{
	const struct CustomDefeatTalkEnt *it;

	for (it = gCustomDefeatTalkList; it->pidA != 0xFFFF; it++) {
		if (it->chapter != 0xff && it->chapter != gPlaySt.chapterIndex) {
			if (it->chapter != 0xfe || BattleIsTriangleAttack() != 1)
				continue;
		}

		if (GetEventTriggerState(it->flag))
			continue;

		if (pidA != it->pidA)
			continue;

		/* pidB not set (wildcard): match any opponent */
		if (it->pidB == 0xFFFF)
			return it;

		/* Otherwise, the actor or the target must be pidB */
		if (IsBattlePid(it->pidB))
			return it;
	}

	return NULL;
}

static bool FindDefeatTalk(u8 pid, struct DefeatTalkResult *out)
{
	const struct CustomDefeatTalkEnt *ent = GetCustomDefeatTalkEntry(pid);

	if (ent != NULL) {
		out->route = ent->route;
		out->flag  = ent->flag;
		out->msg   = ent->msg;
		out->event = ent->event;
		return true;
	}

	if (CustomDefeatQuotes_UseVanillaFallback) {
		const struct DefeatTalkEnt *vent = GetDefeatTalkEntry(pid);

		if (vent != NULL) {
			out->route = vent->route;
			out->flag  = vent->flag;
			out->msg   = vent->msg;
			out->event = (const u16 *)vent->event;
			return true;
		}
	}

	return false;
}

/**
 * Replaces vanilla CheckBattleDefeatTalk (FE8U 0x080835A8).
 */
s8 CheckBattleDefeatTalk_Custom(u8 pid)
{
	struct DefeatTalkResult ent;

	if (FindDefeatTalk(pid, &ent)) {
		if (ent.msg == 0 && ent.event == NULL)
			SetPidDefeatedFlag(pid, ent.flag);
		else
			return 1;
	}

	return 0;
}

/**
 * Replaces vanilla DisplayDefeatTalkForPid (FE8U 0x080835DC).
 */
void DisplayDefeatTalkForPid_Custom(u8 pid)
{
	struct DefeatTalkResult ent;

	if (!FindDefeatTalk(pid, &ent))
		return;

	if ((ent.route == 1) && (ent.flag == 0x65)) {
		StartBgm(SONG_GAME_OVER, NULL);
		gPlaySt.config.disableBgm = 1;
	} else {
		if (UNIT_FACTION(GetUnitFromCharId(pid)) == FACTION_BLUE)
			StartBgm(SONG_IN_SORROWS_SHROUD, NULL);
	}

	if (ent.msg != 0)
		CallBattleQuoteEventInBattle(ent.msg);
	else if (ent.event != NULL)
		EventEngine_CreateBattle(ent.event);

	SetPidDefeatedFlag(pid, ent.flag);
}
