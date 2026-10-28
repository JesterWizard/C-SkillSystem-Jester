/*
 * Gaiden magic spells that act on the map instead of starting a battle
 * (Freezeblink, Prominence, Tempest, Gravity) and the MP cost of spells in
 * gGaidenMagicMpCostList.
 */

#include <common-chax.h>
#include <kernel-lib.h>
#include <map-anims.h>
#include <weapon-range.h>
#include <constants/texts.h>
#include <gaiden-magic.h>
#include "bwl.h"
#include "jester_headers/custom-functions.h"

int GetGaidenMagicMpCost(struct Unit *unit, int item)
{
	if (!gpKernelDesignerConfig->mp_system)
		return 0;

	return gGaidenMagicMpCostList[ITEM_INDEX(item)];
}

bool IsGaidenMapSpell(int item)
{
	switch (ITEM_INDEX(item)) {
	case ITEM_ANIMA_FREEZEBLINK:
	case ITEM_ANIMA_PROMINENCE:
	case ITEM_ANIMA_TEMPEST:
	case ITEM_DARK_GRAVITY:
		return true;

	default:
		return false;
	}
}

void SpendGaidenMagicMp(struct Unit *unit, int item)
{
	struct NewBwl *bwl;
	int cost = GetGaidenMagicMpCost(unit, item);

	if (cost <= 0 || !CheckHasBwl(UNIT_CHAR_ID(unit)))
		return;

	bwl = GetNewBwl(UNIT_CHAR_ID(unit));

	if (cost >= bwl->currentMP)
		bwl->currentMP = 0;
	else
		bwl->currentMP -= cost;
}

/*
 * Targets for a map spell: enemies only. Freezeblink, Prominence and Tempest
 * hit every enemy on the map, Gravity only the ones in its range (see
 * MakeTargetListForGaidenMapSpell).
 */
static bool IsGaidenMapSpellTarget(struct Unit *unit, struct Unit *tunit)
{
	if (!UNIT_IS_VALID(tunit) || tunit->curHP <= 0)
		return false;

	if (tunit->state & (US_HIDDEN | US_DEAD | US_RESCUED | US_NOT_DEPLOYED | US_BIT16))
		return false;

	/* A unit is allied with itself, so this also skips the caster */
	return !AreUnitsAllied(unit->index, tunit->index);
}

void MakeTargetListForGaidenMapSpell(struct Unit *unit, int item)
{
	int uid;
	bool isGravity = ITEM_INDEX(item) == ITEM_DARK_GRAVITY;

	InitTargets(unit->xPos, unit->yPos);

	if (isGravity) {
		BmMapFill(gBmMapRange, 0);
		AddMapForItem(unit, item);
	}

	for (uid = 1; uid <= 0xBF; uid++) {
		struct Unit *tunit = GetUnit(uid);

		if (!IsGaidenMapSpellTarget(unit, tunit))
			continue;

		if (isGravity && gBmMapRange[tunit->yPos][tunit->xPos] == 0)
			continue;

		AddTarget(tunit->xPos, tunit->yPos, tunit->index, 0);
	}
}

/* Group Cast only doubles single-target B.Mag, so it cannot be used with a map spell */
bool CanUnitCastGaidenMapSpell(struct Unit *unit, int item)
{
	if (IsGroupCastActive(unit))
		return false;

	if (!CanUnitUseGaidenMagicNow(unit, item))
		return false;

	MakeTargetListForGaidenMapSpell(unit, item);
	return GetSelectTargetCount() != 0;
}

/* Range shown on the B.Mag item menu while hovering a map spell */
void DisplayGaidenMapSpellRange(struct Unit *unit, int item)
{
	BmMapFill(gBmMapRange, 0);

	if (ITEM_INDEX(item) == ITEM_DARK_GRAVITY) {
		GenerateUnitStandingReachRange(unit, GetItemReachBitsRework(item, unit));
	} else {
		int i;

		MakeTargetListForGaidenMapSpell(unit, item);

		for (i = 0; i < GetSelectTargetCount(); i++) {
			struct SelectTarget *t = GetTarget(i);

			gBmMapRange[t->y][t->x] = 1;
		}
	}

	DisplayMoveRangeGraphics(MOVLIMITV_RMAP_RED);
}

/* Gravity: the 3x3 around the cursor. Others: every target on the map. */
static u8 GaidenMapSpell_OnSwitchIn(ProcPtr proc, struct SelectTarget *target)
{
	int item = GetGaidenMagicItem(gActiveUnit, gActionData.itemSlotIndex);
	int x, y, i;

	BmMapFill(gBmMapRange, 0);

	if (ITEM_INDEX(item) == ITEM_DARK_GRAVITY) {
		for (y = target->y - 1; y <= target->y + 1; y++) {
			for (x = target->x - 1; x <= target->x + 1; x++) {
				if (x < 0 || x >= gBmMapSize.x)
					continue;

				if (y < 0 || y >= gBmMapSize.y)
					continue;

				gBmMapRange[y][x] = 1;
			}
		}
	} else {
		for (i = 0; i < GetSelectTargetCount(); i++) {
			struct SelectTarget *t = GetTarget(i);

			gBmMapRange[t->y][t->x] = 1;
		}
	}

	DisplayMoveRangeGraphics(MOVLIMITV_RMAP_RED);
	return 0;
}

static u8 GaidenMapSpell_OnSelectTarget(ProcPtr proc, struct SelectTarget *target)
{
	gActionData.targetIndex = target->uid;
	gActionData.xOther = target->x;
	gActionData.yOther = target->y;

	HideMoveRangeGraphics();

	BG_Fill(gBG2TilemapBuffer, 0);
	BG_EnableSyncByMask(BG2_SYNC_BIT);

	gActionData.unitActionType = CONFIG_UNIT_ACTION_EXPA_GaidenMapSpell;

	return TARGETSELECTION_ACTION_ENDFAST | TARGETSELECTION_ACTION_END
	     | TARGETSELECTION_ACTION_SE_6A | TARGETSELECTION_ACTION_CLEARBGS;
}

static const struct SelectInfo sSelectInfo_GaidenMapSpell = {
	.onEnd = SubtitleMapSelect_End,
	.onSwitchIn = GaidenMapSpell_OnSwitchIn,
	.onCancel = GaidenBMagAttackMapSelect_Cancel,
};

/* Called from the B.Mag item menu in place of the weapon target selection */
u8 StartGaidenMapSpellSelect(struct Unit *unit, int item)
{
	MakeTargetListForGaidenMapSpell(unit, item);
	BmMapFill(gBmMapMovement, -1);

	StartSubtitleHelp(
		NewTargetSelection_Specialized(&sSelectInfo_GaidenMapSpell, GaidenMapSpell_OnSelectTarget),
		GetStringFromIndex(ITEM_INDEX(item) == ITEM_DARK_GRAVITY ?
			MSG_GaidenMapSpell_TargetGravity : MSG_GaidenMapSpell_TargetAll));

	return MENU_ACT_SKIPCURSOR | MENU_ACT_END | MENU_ACT_SND6A | MENU_ACT_ENDFACE;
}

enum {
	GAIDEN_MAP_SPELL_DAMAGE = 10,
	GAIDEN_MAP_SPELL_TARGET_MAX = 56,
};

struct GaidenMapSpellProc {
	PROC_HEADER;
	/* 2C */ u8 uids[GAIDEN_MAP_SPELL_TARGET_MAX];
	/* 64 */ u8 count, index, exp;
};

_Static_assert(sizeof(struct GaidenMapSpellProc) <= 0x6C, "GaidenMapSpellProc too big");

/* Hurt animation for one target per tick, then kill EXP for a target that died */
static void GaidenMapSpell_DamageTick(struct GaidenMapSpellProc *proc)
{
	struct Unit *target;
	int damage;

	/* After each hurt animation, check whether that target died */
	if (proc->index > 0) {
		struct Unit *prev = GetUnit(proc->uids[proc->index - 1]);

		if (UNIT_IS_VALID(prev) && prev->curHP <= 0) {
			int exp = GetUnitRoundExp(gActiveUnit, prev) + GetUnitKillExpBonus(gActiveUnit, prev);

			proc->exp = MIN(proc->exp + exp, 100);
			UnitKill(prev);
		}
	}

	if (proc->index >= proc->count) {
		if (proc->exp > 0)
			AddExp_Event(proc->exp);
		Proc_Break(proc);
		return;
	}

	target = GetUnit(proc->uids[proc->index]);
	damage = MIN(GAIDEN_MAP_SPELL_DAMAGE, target->curHP);

	CallMapAnim_HurtExt((ProcPtr)proc, target, damage, NULL, NULL);
	proc->index++;
}

static const struct ProcCmd ProcScr_GaidenMapSpellDamage[] = {
	PROC_NAME("GaidenMapSpellDamage"),
	PROC_REPEAT(GaidenMapSpell_DamageTick),
	PROC_END,
};

static int CountGaidenMapSpellEnemies(struct Unit *unit)
{
	int uid, count = 0;

	for (uid = 1; uid <= 0xBF; uid++) {
		if (IsGaidenMapSpellTarget(unit, GetUnit(uid)))
			count++;
	}

	return count;
}

/* Freezeblink, Prominence and Tempest: each enemy on the map is rolled on its own */
static void StartGaidenMapSpellDamage(ProcPtr parent)
{
	struct Unit *caster = gActiveUnit;
	struct GaidenMapSpellProc *proc;
	int uid, hit;

	hit = 100 - CountGaidenMapSpellEnemies(caster);
	if (hit < 0)
		hit = 0;

	proc = Proc_StartBlocking(ProcScr_GaidenMapSpellDamage, parent);
	proc->count = proc->index = proc->exp = 0;

	for (uid = 1; uid <= 0xBF; uid++) {
		struct Unit *tunit = GetUnit(uid);

		if (!IsGaidenMapSpellTarget(caster, tunit))
			continue;

		if (proc->count >= GAIDEN_MAP_SPELL_TARGET_MAX)
			break;

		/* A miss is simply not queued */
		if (Roll1RN(hit))
			proc->uids[proc->count++] = (u8)tunit->index;
	}
}

/* Gravity: play the animation, then halve the MP of each enemy in the 3x3 */
static void GaidenGravity_Start(ProcPtr proc)
{
	CallMapAnim_HeavyGravity(proc, gActionData.xOther, gActionData.yOther);
}

static void GaidenGravity_Wait(ProcPtr proc)
{
	if (!MapAnimHeavyGravityExists())
		Proc_Break(proc);
}

static void GaidenGravity_Apply(ProcPtr proc)
{
	int x, y;

	for (y = gActionData.yOther - 1; y <= gActionData.yOther + 1; y++) {
		for (x = gActionData.xOther - 1; x <= gActionData.xOther + 1; x++) {
			struct Unit *tunit;
			int uid;

			if (x < 0 || x >= gBmMapSize.x)
				continue;

			if (y < 0 || y >= gBmMapSize.y)
				continue;

			uid = gBmMapUnit[y][x];
			if (uid == 0)
				continue;

			tunit = GetUnit(uid);
			if (!IsGaidenMapSpellTarget(gActiveUnit, tunit))
				continue;

			/* Units without a BWL have no MP to halve */
			if (!CheckHasBwl(UNIT_CHAR_ID(tunit)))
				continue;

			GetNewBwl(UNIT_CHAR_ID(tunit))->currentMP /= 2;
		}
	}
}

static const struct ProcCmd ProcScr_GaidenGravity[] = {
	PROC_NAME("GaidenGravity"),
	PROC_CALL(GaidenGravity_Start),
	PROC_YIELD,
	PROC_REPEAT(GaidenGravity_Wait),
	PROC_CALL(GaidenGravity_Apply),
	PROC_END,
};

/*
 * Unit action, gActiveUnit is the caster. Pays the MP cost, then either
 * Gravity halves the MP of the enemies in the 3x3 around the cursor, or
 * Freezeblink, Prominence and Tempest hit each enemy on the map for 10 damage.
 * Both run as blocking child procs, so the action ends when they finish.
 */
bool ActionGaidenMapSpell(ProcPtr proc)
{
	int item = GetGaidenMagicItem(gActiveUnit, gActionData.itemSlotIndex);

	SpendGaidenMagicMp(gActiveUnit, item);

	if (ITEM_INDEX(item) == ITEM_DARK_GRAVITY) {
		Proc_StartBlocking(ProcScr_GaidenGravity, proc);
		return false;
	}

	StartGaidenMapSpellDamage(proc);
	return false;
}
