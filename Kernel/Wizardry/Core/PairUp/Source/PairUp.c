#include "common-chax.h"
#include "pair-up.h"
#include "skill-system.h"
#include "kernel-lib.h"
#include "bmmap.h"
#include "bmphase.h"
#include "bmudisp.h"
#include "bmtarget.h"
#include "uimenu.h"
#include "uiselecttarget.h"
#include "map-anims.h"
#include "event-rework.h"
#include "action-expa.h"
#include "jester_headers/custom-functions.h"
#include "constants/texts.h"
#include "strmag.h"
#include "status-getter.h"

#pragma GCC optimize("Os", "no-jump-tables")

static bool PairUp_IsInMap(int x, int y);
extern const struct SelectInfo gPairUpAttachSelectInfo;

static bool PairUp_IsValidUnit(const struct Unit *unit)
{
	return UNIT_IS_VALID(unit) && (u8) unit->index != 0;
}

static u16 *PairUp_GetState(const struct Unit *unit)
{
	if (!PairUp_IsValidUnit(unit))
		return NULL;

	return &gPairUpState[(u8) unit->index];
}

static bool PairUp_AreAdjacent(const struct Unit *left, const struct Unit *right)
{
	int dx;
	int dy;

	if (!PairUp_IsValidUnit(left) || !PairUp_IsValidUnit(right))
		return false;

	dx = left->xPos - right->xPos;
	dy = left->yPos - right->yPos;

	if (dx < 0)
		dx = -dx;

	if (dy < 0)
		dy = -dy;

	return dx + dy == 1;
}

bool PairUp_IsEnabled(void)
{
	return gpKernelDesignerConfig && gpKernelDesignerConfig->menu_option_pair_up;
}

bool PairUp_IsPaired(const struct Unit *unit)
{
	u16 *state = PairUp_GetState(unit);

	return state && ((*state & PAIR_UP_STATE_PARTNER_MASK) != 0);
}

bool PairUp_IsLeader(const struct Unit *unit)
{
	u16 *state = PairUp_GetState(unit);

	return PairUp_IsPaired(unit) && (*state & PAIR_UP_STATE_LEADER);
}

bool PairUp_IsSupport(const struct Unit *unit)
{
	return PairUp_IsPaired(unit) && !PairUp_IsLeader(unit);
}

struct Unit *PairUp_GetPartner(const struct Unit *unit)
{
	u16 *state = PairUp_GetState(unit);
	struct Unit *partner;

	if (!state)
		return NULL;

	partner = GetUnit(*state & PAIR_UP_STATE_PARTNER_MASK);
	if (!PairUp_IsValidUnit(partner))
		return NULL;

	if (!PairUp_IsPaired(partner))
		return NULL;

	if ((*PairUp_GetState(partner) & PAIR_UP_STATE_PARTNER_MASK) != (u8) unit->index)
		return NULL;

	return partner;
}

struct Unit *PairUp_GetLeader(const struct Unit *unit)
{
	struct Unit *partner;

	if (PairUp_IsLeader(unit))
		return (struct Unit *) unit;

	if (!PairUp_IsSupport(unit))
		return NULL;

	partner = PairUp_GetPartner(unit);
	if (partner && PairUp_IsLeader(partner))
		return partner;

	return NULL;
}

struct Unit *PairUp_GetSupport(const struct Unit *unit)
{
	if (!PairUp_IsLeader(unit))
		return NULL;

	return PairUp_GetPartner(unit);
}

bool PairUp_CanPair(struct Unit *support, struct Unit *leader)
{
	if (!PairUp_IsEnabled())
		return false;

	if (!PairUp_IsValidUnit(support) || !PairUp_IsValidUnit(leader))
		return false;

	if (support == leader || !PairUp_AreAdjacent(support, leader))
		return false;

	if (!IsSameAllegiance(support->index, leader->index))
		return false;

	if (PairUp_IsPaired(support) || PairUp_IsPaired(leader))
		return false;

	if ((!UnitOnMapAvaliable(support) && support != gActiveUnit)
		|| (!UnitOnMapAvaliable(leader) && leader != gActiveUnit))
		return false;

	if ((support->state | leader->state) & (US_IN_BALLISTA | US_RESCUING | US_RESCUED))
		return false;

	if (support->statusIndex == UNIT_STATUS_BERSERK || leader->statusIndex == UNIT_STATUS_BERSERK)
		return false;

	if (UNIT_IS_PHANTOM(support) || UNIT_IS_PHANTOM(leader))
		return false;

	return true;
}

bool PairUp_Attach(struct Unit *support, struct Unit *leader)
{
	u8 supportIndex;
	u8 leaderIndex;

	if (!PairUp_CanPair(support, leader))
		return false;

	supportIndex = (u8) support->index;
	leaderIndex = (u8) leader->index;

	gPairUpState[supportIndex] = leaderIndex;
	gPairUpState[leaderIndex] = supportIndex | PAIR_UP_STATE_LEADER;

	support->state |= US_HIDDEN;
	support->xPos = leader->xPos;
	support->yPos = leader->yPos;
	leader->state &= ~US_HIDDEN;

	if (gBmMapUnit)
		gBmMapUnit[leader->yPos][leader->xPos] = leaderIndex;

	return true;
}

bool PairUp_Separate(struct Unit *leader, int x, int y)
{
	struct Unit *support;
	u8 leaderIndex;

	if (!PairUp_IsLeader(leader))
		return false;

	support = PairUp_GetSupport(leader);
	if (!PairUp_IsValidUnit(support))
		return false;

	if (!PairUp_IsInMap(x, y) || !gBmMapUnit || gBmMapUnit[y][x] != 0)
		return false;

	leaderIndex = (u8) leader->index;
	gPairUpState[leaderIndex] = 0;
	gPairUpState[(u8) support->index] = 0;

	support->state &= ~US_HIDDEN;
	support->xPos = x;
	support->yPos = y;
	gBmMapUnit[y][x] = (u8) support->index;

	return true;
}

bool PairUp_Switch(struct Unit *leader)
{
	struct Unit *support;
	u8 leaderIndex;
	u8 supportIndex;
	s8 x;
	s8 y;

	if (!PairUp_IsLeader(leader))
		return false;

	support = PairUp_GetSupport(leader);
	if (!PairUp_IsValidUnit(support))
		return false;

	leaderIndex = (u8) leader->index;
	supportIndex = (u8) support->index;
	x = leader->xPos;
	y = leader->yPos;

	gPairUpState[leaderIndex] = supportIndex;
	gPairUpState[supportIndex] = leaderIndex | PAIR_UP_STATE_LEADER;

	leader->state |= US_HIDDEN;
	support->state &= ~US_HIDDEN;
	support->xPos = x;
	support->yPos = y;

	if (gBmMapUnit)
		gBmMapUnit[y][x] = supportIndex;

	return true;
}

bool PairUp_Transfer(struct Unit *leader, struct Unit *newLeader)
{
	struct Unit *support;
	struct Unit *newSupport;
	u8 leaderIndex;
	u8 newLeaderIndex;
	u8 supportIndex;

	if (!PairUp_IsLeader(leader) || !PairUp_IsValidUnit(newLeader))
		return false;

	if (!PairUp_AreAdjacent(leader, newLeader))
		return false;

	if (!IsSameAllegiance(leader->index, newLeader->index))
		return false;

	if (PairUp_IsSupport(newLeader))
		return false;

	if (!UnitOnMapAvaliable(newLeader)
		|| (newLeader->state & (US_IN_BALLISTA | US_RESCUING | US_RESCUED))
		|| newLeader->statusIndex == UNIT_STATUS_BERSERK
		|| UNIT_IS_PHANTOM(newLeader))
		return false;

	support = PairUp_GetSupport(leader);
	if (!PairUp_IsValidUnit(support))
		return false;

	newSupport = PairUp_IsLeader(newLeader) ? PairUp_GetSupport(newLeader) : NULL;
	if (PairUp_IsLeader(newLeader) && !newSupport)
		return false;

	leaderIndex = (u8) leader->index;
	newLeaderIndex = (u8) newLeader->index;
	supportIndex = (u8) support->index;

	gPairUpState[leaderIndex] = 0;
	gPairUpState[supportIndex] = newLeaderIndex;

	support->state |= US_HIDDEN;
	support->xPos = newLeader->xPos;
	support->yPos = newLeader->yPos;

	if (newSupport) {
		gPairUpState[leaderIndex] = (u8) newSupport->index | PAIR_UP_STATE_LEADER;
		gPairUpState[newLeaderIndex] = supportIndex | PAIR_UP_STATE_LEADER;
		gPairUpState[(u8) newSupport->index] = leaderIndex;

		newSupport->state |= US_HIDDEN;
		newSupport->xPos = leader->xPos;
		newSupport->yPos = leader->yPos;
	} else {
		gPairUpState[newLeaderIndex] = supportIndex | PAIR_UP_STATE_LEADER;
	}

	leader->state &= ~US_HIDDEN;
	newLeader->state &= ~US_HIDDEN;

	return true;
}

void PairUp_ClearUnit(struct Unit *unit)
{
	struct Unit *partner = PairUp_GetPartner(unit);
	u8 index;
	int i;

	if (!PairUp_IsValidUnit(unit))
		return;

	index = (u8) unit->index;
	if (gPairUpState[index] & PAIR_UP_STATE_PARTNER_MASK)
		unit->state &= ~US_HIDDEN;
	gPairUpState[index] = 0;

	if (partner) {
		gPairUpState[(u8) partner->index] = 0;
		partner->state &= ~US_HIDDEN;
	}

	/* Also remove stale reverse links left by slot reuse or a partial load. */
	for (i = 1; i < PAIR_UP_UNIT_ID_MAX; ++i) {
		struct Unit *other;

		if (i == index || (gPairUpState[i] & PAIR_UP_STATE_PARTNER_MASK) != index)
			continue;

		gPairUpState[i] = 0;
		other = GetUnit(i);
		if (PairUp_IsValidUnit(other))
			other->state &= ~US_HIDDEN;
	}
}

void PairUp_ResetAll(void)
{
	int i;

	for (i = 0; i < PAIR_UP_UNIT_ID_MAX; ++i) {
		struct Unit *unit = GetUnit(i);

		if (gPairUpState[i] && PairUp_IsValidUnit(unit))
			unit->state &= ~US_HIDDEN;

		gPairUpState[i] = 0;
	}
}

void PairUp_Reconcile(void)
{
	int i;

	for (i = 1; i < PAIR_UP_UNIT_ID_MAX; ++i) {
		struct Unit *unit;
		struct Unit *partner;
		u8 partnerIndex;

		if (!gPairUpState[i])
			continue;

		unit = GetUnit(i);
		partnerIndex = gPairUpState[i] & PAIR_UP_STATE_PARTNER_MASK;
		partner = GetUnit(partnerIndex);

		if (!PairUp_IsValidUnit(unit)) {
			gPairUpState[i] = 0;
			continue;
		}

		if (!PairUp_IsValidUnit(partner) || partnerIndex == i
			|| !IsSameAllegiance(i, partnerIndex)
			|| (unit->state | partner->state)
				& (US_DEAD | US_IN_BALLISTA | US_RESCUING | US_RESCUED)
			|| UNIT_IS_PHANTOM(unit) || UNIT_IS_PHANTOM(partner)
			|| (gPairUpState[partnerIndex] & PAIR_UP_STATE_PARTNER_MASK) != i) {
			PairUp_ClearUnit(unit);
		}
	}

	for (i = 1; i < PAIR_UP_UNIT_ID_MAX; ++i) {
		u8 partnerIndex;

		if (!gPairUpState[i])
			continue;

		partnerIndex = gPairUpState[i] & PAIR_UP_STATE_PARTNER_MASK;
		if (i > partnerIndex)
			continue;

		if (!(gPairUpState[i] & PAIR_UP_STATE_LEADER)
			&& !(gPairUpState[partnerIndex] & PAIR_UP_STATE_LEADER)) {
			gPairUpState[i] |= PAIR_UP_STATE_LEADER;
		} else if ((gPairUpState[i] & PAIR_UP_STATE_LEADER)
			&& (gPairUpState[partnerIndex] & PAIR_UP_STATE_LEADER)) {
			gPairUpState[partnerIndex] &= ~PAIR_UP_STATE_LEADER;
		}
	}
}

int PairUp_OnClearUnit(struct Unit *unit)
{
	PairUp_ClearUnit(unit);
	return 0;
}

int PairUp_OnCopyUnit(struct Unit *from, struct Unit *to)
{
	PairUp_ClearUnit(to);

	if (PairUp_IsSupport(from))
		to->state &= ~US_HIDDEN;

	return 0;
}

int PairUp_OnUnitKill(struct Unit *unit)
{
	if (PairUp_IsLeader(unit))
		PairUp_MarkPairActed(unit);

	PairUp_ClearUnit(unit);
	return 0;
}

void PairUp_RebuildMap(void)
{
	int i;

	PairUp_Reconcile();

	for (i = 0; i < PAIR_UP_UNIT_ID_MAX; ++i) {
		struct Unit *leader = GetUnit(i);
		struct Unit *support;

		if (!PairUp_IsLeader(leader))
			continue;

		support = PairUp_GetSupport(leader);
		if (!PairUp_IsValidUnit(support))
			continue;

		leader->state &= ~US_HIDDEN;
		support->state |= US_HIDDEN;
		support->xPos = leader->xPos;
		support->yPos = leader->yPos;
	}

	if (gBmMapUnit)
		RefreshEntityBmMaps();
}

int PairUp_GetSupportLevel(struct Unit *leader, struct Unit *support)
{
	int slot;

	if (!PairUp_IsValidUnit(leader) || !PairUp_IsValidUnit(support))
		return SUPPORT_LEVEL_NONE;

	if (!UNIT_SUPPORT_DATA(leader))
		return SUPPORT_LEVEL_NONE;

	slot = GetUnitSupporterNum(leader, UNIT_CHAR_ID(support));
	if (slot < 0 || slot >= UNIT_SUPPORT_MAX_COUNT)
		return SUPPORT_LEVEL_NONE;

	return GetUnitSupportLevel(leader, slot);
}

int PairUp_GetSupportRank(struct Unit *leader, struct Unit *support)
{
	return PairUp_GetSupportLevel(leader, support) + 1;
}

static int PairUp_GetRawStat(const struct Unit *unit, int stat)
{
	switch (stat) {
	case PAIR_UP_STAT_POW:
		return PowGetter((struct Unit *) unit);
	case PAIR_UP_STAT_MAG:
		return MagGetter((struct Unit *) unit);
	case PAIR_UP_STAT_SKL:
		return SklGetter((struct Unit *) unit);
	case PAIR_UP_STAT_SPD:
		return SpdGetter((struct Unit *) unit);
	case PAIR_UP_STAT_LCK:
		return LckGetter((struct Unit *) unit);
	case PAIR_UP_STAT_DEF:
		return DefGetter((struct Unit *) unit);
	case PAIR_UP_STAT_RES:
		return ResGetter((struct Unit *) unit);
	case PAIR_UP_STAT_MOV:
		return MovGetter((struct Unit *) unit);
	default:
		return 0;
	}
}

static int PairUp_GetClassBonus(const struct Unit *support, int stat)
{
	const struct PairUpClassBonus *bonus;
	int classId;

	if (!PairUp_IsValidUnit(support) || !support->pClassData)
		return 0;

	classId = support->pClassData->number;
	if (classId < 0 || (u32) classId >= ARRAY_COUNT(gPairUpClassBonuses))
		return 0;

	bonus = &gPairUpClassBonuses[classId];

	switch (stat) {
	case PAIR_UP_STAT_POW:
		return bonus->pow;
	case PAIR_UP_STAT_MAG:
		return bonus->mag;
	case PAIR_UP_STAT_SKL:
		return bonus->skl;
	case PAIR_UP_STAT_SPD:
		return bonus->spd;
	case PAIR_UP_STAT_LCK:
		return bonus->lck;
	case PAIR_UP_STAT_DEF:
		return bonus->def;
	case PAIR_UP_STAT_RES:
		return bonus->res;
	case PAIR_UP_STAT_MOV:
		return bonus->mov;
	default:
		return 0;
	}
}

int PairUp_GetStatBonus(struct Unit *leader, struct Unit *support, int stat)
{
	int level;
	int raw;
	int classBonus;
	int statBonus;

	if (!PairUp_IsEnabled() || !PairUp_IsValidUnit(leader) || !PairUp_IsValidUnit(support))
		return 0;

	level = PairUp_GetSupportLevel(leader, support);
	raw = PairUp_GetRawStat(support, stat);
	statBonus = stat == PAIR_UP_STAT_MOV
		? 0
		: raw >= 30 ? 3 : raw >= 20 ? 2 : raw >= 10 ? 1 : 0;
	classBonus = PairUp_GetClassBonus(support, stat);

	if (stat != PAIR_UP_STAT_MOV
		&& (level == SUPPORT_LEVEL_C || level == SUPPORT_LEVEL_B))
		statBonus += classBonus ? 1 : 0;
	else if (stat != PAIR_UP_STAT_MOV && level == SUPPORT_LEVEL_A)
		statBonus += classBonus ? 2 : 0;

	return statBonus + classBonus;
}

int PairUp_GetLeadStatBonus(struct Unit *leader, int stat)
{
	if (!PairUp_IsLeader(leader))
		return 0;

	return PairUp_GetStatBonus(leader, PairUp_GetSupport(leader), stat);
}

static int PairUp_GetAdjacentSupportRank(struct Unit *leader, int x, int y)
{
	struct Unit *unit;
	int rank;

	if (!PairUp_IsInMap(x, y) || !gBmMapUnit)
		return 0;

	unit = GetUnit(gBmMapUnit[y][x]);
	if (!PairUp_IsValidUnit(unit) || unit == leader || PairUp_IsSupport(unit))
		return 0;

	if (!AreUnitsAllied(leader->index, unit->index))
		return 0;

	rank = 1;
	if (PairUp_GetSupportLevel(leader, unit) != SUPPORT_LEVEL_NONE)
		rank = PairUp_GetSupportRank(leader, unit);

	return rank;
}

int PairUp_GetDualSupportBonus(int rank, int stat)
{
	static const u8 sHitBonus[13] = {
		0, 10, 10, 10, 10, 15, 15, 15, 15, 20, 20, 20, 20,
	};
	static const u8 sAvoidBonus[13] = {
		0, 0, 10, 10, 10, 10, 15, 15, 15, 15, 20, 20, 20,
	};
	static const u8 sCritBonus[13] = {
		0, 0, 0, 0, 10, 10, 10, 10, 15, 15, 15, 15, 20,
	};
	static const u8 sDodgeBonus[13] = {
		0, 0, 0, 10, 10, 10, 10, 15, 15, 15, 15, 20, 20,
	};

	if (rank < 1)
		return 0;

	if (rank > 12)
		rank = 12;

	switch (stat) {
	case PAIR_UP_DUAL_SUPPORT_HIT:
		return sHitBonus[rank];
	case PAIR_UP_DUAL_SUPPORT_AVOID:
		return sAvoidBonus[rank];
	case PAIR_UP_DUAL_SUPPORT_CRIT:
		return sCritBonus[rank];
	case PAIR_UP_DUAL_SUPPORT_DODGE:
		return sDodgeBonus[rank];
	default:
		return 0;
	}
}

int PairUp_GetDualSupportRank(struct Unit *leader)
{
	struct Unit *support;
	int rank = 0;
	int x;
	int y;

	if (!PairUp_IsEnabled() || !PairUp_IsValidUnit(leader))
		return 0;

	if (PairUp_IsSupport(leader))
		leader = PairUp_GetLeader(leader);

	if (!leader)
		return 0;

	support = PairUp_GetSupport(leader);
	if (support)
		rank += PairUp_GetSupportRank(leader, support);

	x = leader->xPos;
	y = leader->yPos;
	rank += PairUp_GetAdjacentSupportRank(leader, x, y - 1);
	rank += PairUp_GetAdjacentSupportRank(leader, x + 1, y);
	rank += PairUp_GetAdjacentSupportRank(leader, x, y + 1);
	rank += PairUp_GetAdjacentSupportRank(leader, x - 1, y);

	if (rank > 12)
		rank = 12;

	return rank;
}

void PairUp_SaveState(u8 *dst, const u32 size)
{
	WriteAndVerifySramFast(gPairUpState, dst, size < sizeof(gPairUpState)
		? size : sizeof(gPairUpState));
}

void PairUp_LoadState(u8 *src, const u32 size)
{
	u32 copySize = size;

	if (copySize > sizeof(gPairUpState))
		copySize = sizeof(gPairUpState);

	memset(gPairUpState, 0, sizeof(gPairUpState));
	ReadSramFast(src, gPairUpState, copySize);
	PairUp_RebuildMap();
}

static void PairUp_TryAddTarget(struct Unit *unit)
{
	if (!PairUp_CanPair(gSubjectUnit, unit))
		return;

	AddTarget(unit->xPos, unit->yPos, unit->index, 0);
}

static void PairUp_MakePairTargetList(struct Unit *unit)
{
	InitTargets(0, 0);
	gSubjectUnit = unit;
	BmMapFill(gBmMapRange, 0);
	ForEachAdjacentUnit(unit->xPos, unit->yPos, PairUp_TryAddTarget);
}

static void PairUp_TryAddTransferTarget(struct Unit *unit)
{
	if (!PairUp_IsValidUnit(unit) || unit == gSubjectUnit)
		return;

	if (!IsSameAllegiance(gSubjectUnit->index, unit->index))
		return;

	if (!UnitOnMapAvaliable(unit)
		|| (unit->state & (US_IN_BALLISTA | US_RESCUING | US_RESCUED))
		|| unit->statusIndex == UNIT_STATUS_BERSERK
		|| UNIT_IS_PHANTOM(unit))
		return;

	if (PairUp_IsSupport(unit))
		return;

	if (PairUp_IsPaired(unit) && (!PairUp_IsLeader(unit) || !PairUp_GetSupport(unit)))
		return;

	AddTarget(unit->xPos, unit->yPos, unit->index, 0);
}

static void PairUp_MakeTransferTargetList(struct Unit *unit)
{
	InitTargets(0, 0);
	gSubjectUnit = unit;
	BmMapFill(gBmMapRange, 0);
	ForEachAdjacentUnit(unit->xPos, unit->yPos, PairUp_TryAddTransferTarget);
}

static bool PairUp_IsInMap(int x, int y)
{
	return gBmMapSize.x > 0 && gBmMapSize.y > 0
		&& x >= 0 && y >= 0
		&& x < gBmMapSize.x && y < gBmMapSize.y;
}

static void PairUp_MakeSeparateTargetList(struct Unit *leader)
{
	struct Unit *support = PairUp_GetSupport(leader);
	int x = leader->xPos;
	int y = leader->yPos;
	int i;
	static const s8 sDx[] = { 0, 1, 0, -1 };
	static const s8 sDy[] = { -1, 0, 1, 0 };

	InitTargets(0, 0);
	gSubjectUnit = leader;
	BmMapFill(gBmMapRange, 0);

	if (!support)
		return;

	for (i = 0; i < 4; ++i) {
		int targetX = x + sDx[i];
		int targetY = y + sDy[i];

		if (!PairUp_IsInMap(targetX, targetY))
			continue;

		if (gBmMapUnit[targetY][targetX] != 0)
			continue;

		if (!Generic_CanUnitBeOnPos(support, targetX, targetY, -1, -1))
			continue;

		AddTarget(targetX, targetY, 0, 0);
	}
}

static u8 PairUp_OnSelectAttach(ProcPtr proc, struct SelectTarget *target)
{
	gActionData.subjectIndex = gActiveUnit->index;
	gActionData.targetIndex = target->uid;
	gActionData.xOther = target->x;
	gActionData.yOther = target->y;
	gActionData.unk08 = PAIR_UP_ACTION_ATTACH;
	gActionData.unitActionType = CONFIG_UNIT_ACTION_EXPA_PairUp;

	HideMoveRangeGraphics();
	BG_Fill(gBG2TilemapBuffer, 0);
	BG_EnableSyncByMask(BG2_SYNC_BIT);

	return TARGETSELECTION_ACTION_ENDFAST
		| TARGETSELECTION_ACTION_END
		| TARGETSELECTION_ACTION_SE_6A
		| TARGETSELECTION_ACTION_CLEARBGS;
}

static u8 PairUp_OnSelectSeparate(ProcPtr proc, struct SelectTarget *target)
{
	gActionData.subjectIndex = gActiveUnit->index;
	gActionData.targetIndex = 0;
	gActionData.xOther = target->x;
	gActionData.yOther = target->y;
	gActionData.unk08 = PAIR_UP_ACTION_SEPARATE;
	gActionData.unitActionType = CONFIG_UNIT_ACTION_EXPA_PairUp;

	HideMoveRangeGraphics();
	BG_Fill(gBG2TilemapBuffer, 0);
	BG_EnableSyncByMask(BG2_SYNC_BIT);

	return TARGETSELECTION_ACTION_ENDFAST
		| TARGETSELECTION_ACTION_END
		| TARGETSELECTION_ACTION_SE_6A
		| TARGETSELECTION_ACTION_CLEARBGS;
}

static u8 PairUp_OnSelectTransfer(ProcPtr proc, struct SelectTarget *target)
{
	gActionData.subjectIndex = gActiveUnit->index;
	gActionData.targetIndex = target->uid;
	gActionData.xOther = target->x;
	gActionData.yOther = target->y;
	gActionData.unk08 = PAIR_UP_ACTION_TRANSFER;
	gActionData.unitActionType = CONFIG_UNIT_ACTION_EXPA_PairUp;

	HideMoveRangeGraphics();
	BG_Fill(gBG2TilemapBuffer, 0);
	BG_EnableSyncByMask(BG2_SYNC_BIT);

	return TARGETSELECTION_ACTION_ENDFAST
		| TARGETSELECTION_ACTION_END
		| TARGETSELECTION_ACTION_SE_6A
		| TARGETSELECTION_ACTION_CLEARBGS;
}

u8 PairUp_MenuUsability(const struct MenuItemDef *def, int number)
{
	if (!PairUp_IsEnabled() || !gActiveUnit || (gActiveUnit->state & US_HAS_MOVED))
		return MENU_NOTSHOWN;

	if (PairUp_IsPaired(gActiveUnit)
		|| (gActiveUnit->state & (US_IN_BALLISTA | US_RESCUING | US_RESCUED)))
		return MENU_NOTSHOWN;

	PairUp_MakePairTargetList(gActiveUnit);
	return GetSelectTargetCount() ? MENU_ENABLED : MENU_NOTSHOWN;
}

u8 PairUp_MenuOnSelected(struct MenuProc *menu, struct MenuItemProc *item)
{
	if (item->availability == MENU_DISABLED)
		return MENU_ACT_SND6B;

	ClearBg0Bg1();
	PairUp_MakePairTargetList(gActiveUnit);
	BmMapFill(gBmMapMovement, -1);

	StartSubtitleHelp(
		NewTargetSelection_Specialized(&gPairUpAttachSelectInfo, PairUp_OnSelectAttach),
		GetStringFromIndex(MSG_SKILL_Common_Target));

	PlaySoundEffect(0x6A);
	return MENU_ACT_SKIPCURSOR | MENU_ACT_END | MENU_ACT_SND6A;
}

u8 PairUp_SwitchUsability(const struct MenuItemDef *def, int number)
{
	if (!PairUp_IsEnabled() || !gActiveUnit || (gActiveUnit->state & US_HAS_MOVED))
		return MENU_NOTSHOWN;

	return PairUp_IsLeader(gActiveUnit) ? MENU_ENABLED : MENU_NOTSHOWN;
}

u8 PairUp_SwitchOnSelected(struct MenuProc *menu, struct MenuItemProc *item)
{
	if (item->availability == MENU_DISABLED)
		return MENU_ACT_SND6B;

	gActionData.subjectIndex = gActiveUnit->index;
	gActionData.targetIndex = PairUp_GetSupport(gActiveUnit)->index;
	gActionData.unk08 = PAIR_UP_ACTION_SWITCH;
	gActionData.unitActionType = CONFIG_UNIT_ACTION_EXPA_PairUp;

	return MENU_ACT_SKIPCURSOR | MENU_ACT_END | MENU_ACT_SND6A | MENU_ACT_CLEAR;
}

u8 PairUp_SeparateUsability(const struct MenuItemDef *def, int number)
{
	if (!PairUp_IsEnabled() || !gActiveUnit || (gActiveUnit->state & US_HAS_MOVED))
		return MENU_NOTSHOWN;

	if (!PairUp_IsLeader(gActiveUnit))
		return MENU_NOTSHOWN;

	PairUp_MakeSeparateTargetList(gActiveUnit);
	return GetSelectTargetCount() ? MENU_ENABLED : MENU_NOTSHOWN;
}

u8 PairUp_SeparateOnSelected(struct MenuProc *menu, struct MenuItemProc *item)
{
	if (item->availability == MENU_DISABLED)
		return MENU_ACT_SND6B;

	ClearBg0Bg1();
	PairUp_MakeSeparateTargetList(gActiveUnit);
	BmMapFill(gBmMapMovement, -1);

	StartSubtitleHelp(
		NewTargetSelection_Specialized(&gSelectInfo_PutTrap, PairUp_OnSelectSeparate),
		GetStringFromIndex(MSG_SKILL_Common_Target));

	PlaySoundEffect(0x6A);
	return MENU_ACT_SKIPCURSOR | MENU_ACT_END | MENU_ACT_SND6A;
}

u8 PairUp_TransferUsability(const struct MenuItemDef *def, int number)
{
	if (!PairUp_IsEnabled() || !gActiveUnit || (gActiveUnit->state & US_HAS_MOVED))
		return MENU_NOTSHOWN;

	if (!PairUp_IsLeader(gActiveUnit))
		return MENU_NOTSHOWN;

	PairUp_MakeTransferTargetList(gActiveUnit);
	return GetSelectTargetCount() ? MENU_ENABLED : MENU_NOTSHOWN;
}

u8 PairUp_TransferOnSelected(struct MenuProc *menu, struct MenuItemProc *item)
{
	if (item->availability == MENU_DISABLED)
		return MENU_ACT_SND6B;

	ClearBg0Bg1();
	PairUp_MakeTransferTargetList(gActiveUnit);
	BmMapFill(gBmMapMovement, -1);

	StartSubtitleHelp(
		NewTargetSelection_Specialized(&gSelectInfo_PutTrap, PairUp_OnSelectTransfer),
		GetStringFromIndex(MSG_SKILL_Common_Target));

	PlaySoundEffect(0x6A);
	return MENU_ACT_SKIPCURSOR | MENU_ACT_END | MENU_ACT_SND6A;
}

void PairUp_MarkPairActed(struct Unit *leader)
{
	if (!leader)
		return;

	struct Unit *support = PairUp_GetSupport(leader);

	leader->state |= US_HAS_MOVED;

	if (!PairUp_IsLeader(leader))
		return;

	if (support)
		support->state |= US_HAS_MOVED;
}

bool ActionPairUp(ProcPtr proc)
{
	struct Unit *subject = GetUnit(gActionData.subjectIndex);
	struct Unit *target;
	struct Unit *support;

	switch (gActionData.unk08) {
	case PAIR_UP_ACTION_ATTACH:
		target = GetUnit(gActionData.targetIndex);
		if (!PairUp_Attach(subject, target))
			return true;

		gActionData.xMove = target->xPos;
		gActionData.yMove = target->yPos;
		gActionData.subjectIndex = target->index;
		gActionData.targetIndex = subject->index;
		gActiveUnit = target;

		if (!(target->state & US_HAS_MOVED))
			gActionDataExpa.refrain_action = true;
		break;

	case PAIR_UP_ACTION_SWITCH:
		support = PairUp_GetSupport(subject);
		if (!PairUp_Switch(subject))
			return true;

		if (!support)
			return true;

		PairUp_MarkPairActed(support);
		gActionData.subjectIndex = support->index;
		gActionData.targetIndex = subject->index;
		gActionData.xMove = support->xPos;
		gActionData.yMove = support->yPos;
		gActiveUnit = support;
		break;

	case PAIR_UP_ACTION_SEPARATE:
		support = PairUp_GetSupport(subject);
		if (!PairUp_Separate(subject, gActionData.xOther, gActionData.yOther))
			return true;

		subject->state |= US_HAS_MOVED;
		if (support)
			support->state |= US_HAS_MOVED;
		gActionData.subjectIndex = subject->index;
		gActionData.targetIndex = support ? support->index : 0;
		gActionData.xMove = subject->xPos;
		gActionData.yMove = subject->yPos;
		gActiveUnit = subject;
		break;

	case PAIR_UP_ACTION_TRANSFER:
		target = GetUnit(gActionData.targetIndex);
		if (!PairUp_Transfer(subject, target))
			return true;

		PairUp_MarkPairActed(subject);
		if (PairUp_IsLeader(target))
			PairUp_MarkPairActed(target);
		gActionData.subjectIndex = subject->index;
		gActionData.xMove = subject->xPos;
		gActionData.yMove = subject->yPos;
		gActiveUnit = subject;
		break;

	default:
		return true;
	}

	RefreshEntityBmMaps();
	RefreshUnitSprites();
	return true;
}
