#include "common-chax.h"
#include "map-anims.h"
#include "kernel-lib.h"
#include "skill-system.h"
#include "battle-system.h"
#include "gaiden-magic.h"
#include "constants/skills.h"
#include "constants/texts.h"
#include "jester_headers/custom-functions.h"

/*
 * Group Cast: the B.Mag spell list opens with gActionData.unk08 set to
 * SID_GroupCast. While it is set the spell costs double (gaiden-magic.h,
 * IsGroupCastActive). After the cast, every enemy next to the target takes
 * the spell's attack minus its resistance.
 */

#if defined(SID_GroupCast) && (COMMON_SKILL_VALID(SID_GroupCast))

u8 GroupCast_Usability(const struct MenuItemDef *def, int number)
{
	if (gActiveUnit->state & US_CANTOING)
		return MENU_NOTSHOWN;

	return GaidenBMagActionCommandUsability(def, number);
}

u8 GroupCast_OnSelected(struct MenuProc *menu, struct MenuItemProc *item)
{
	if (item->availability == MENU_DISABLED) {
		MenuFrozenHelpBox(menu, MSG_SKILL_CommonFail);
		return MENU_ACT_SND6B;
	}

	gActionData.unk08 = SID_GroupCast;
	return StartGaidenBMagSpellMenu(menu, item);
}

struct GroupCastProc {
	PROC_HEADER;
	/* 2C */ struct Unit *targets[4];
	/* 3C */ u8 damage[4];
	/* 40 */ u8 count, index, exp, hidden;
};

_Static_assert(sizeof(struct GroupCastProc) <= 0x6C, "GroupCastProc too big");

static void GroupCast_Tick(struct GroupCastProc *proc)
{
	/* The caster's MU is gone during the splash, so draw its map sprite */
	if (proc->index == 0) {
		proc->hidden = gActiveUnit->state & US_HIDDEN;
		gActiveUnit->state &= ~US_HIDDEN;
		RefreshUnitSprites();
	}

	/* After each hurt animation, check whether that target died */
	if (proc->index > 0) {
		struct Unit *prev = proc->targets[proc->index - 1];

		if (UNIT_IS_VALID(prev) && prev->curHP <= 0) {
			int exp = GetUnitRoundExp(gActiveUnit, prev) + GetUnitKillExpBonus(gActiveUnit, prev);

			proc->exp = MIN(proc->exp + exp, 100);
			UnitKill(prev);
		}
	}

	if (proc->index >= proc->count) {
		/* Restore the hidden bit so the vanilla action end sees the same state */
		gActiveUnit->state |= proc->hidden;
		if (proc->exp > 0)
			AddExp_Event(proc->exp);
		Proc_Break(proc);
		return;
	}

	CallMapAnim_HurtExt((ProcPtr)proc, proc->targets[proc->index], proc->damage[proc->index], NULL, NULL);
	proc->index++;
}

static const struct ProcCmd ProcScr_GroupCastSplash[] = {
	PROC_NAME("GroupCast"),
	PROC_REPEAT(GroupCast_Tick),
	PROC_END,
};

#endif

bool PostAction_GroupCast(ProcPtr parent)
{
#if defined(SID_GroupCast) && (COMMON_SKILL_VALID(SID_GroupCast))
	static const s8 offsets[4][2] = { { 0, -1 }, { 0, 1 }, { -1, 0 }, { 1, 0 } };
	struct GroupCastProc *proc;
	struct Unit *unit = gActiveUnit;
	struct Unit *targets[4];
	u8 damage[4];
	int i, count = 0;

	if (gActionData.unk08 != SID_GroupCast)
		return false;

	/* The flag only lasts for this action, whatever the player did */
	gActionData.unk08 = 0;

	if (gActionData.unitActionType != CONFIG_UNIT_ACTION_EXPA_GaidenMagicCombat)
		return false;

	if (!UNIT_IS_VALID(unit) || !UnitAvaliable(unit) || unit->curHP <= 0)
		return false;

	/* Three Houses mode: the battle spent one use, Group Cast spends a second */
	if (IsGaidenMagicUsesSystem() && gBattleActorGlobalFlag.gaiden_magic_cast)
		ConsumeGaidenMagicUse(unit, gBattleActor.weaponBefore ? gBattleActor.weaponBefore : gBattleActor.weapon);

	for (i = 0; i < 4; i++) {
		struct Unit *tunit = GetUnitAtPosition(
			gBattleTargetPositionBackup.x + offsets[i][0],
			gBattleTargetPositionBackup.y + offsets[i][1]);
		int dmg;

		if (!UNIT_IS_VALID(tunit) || tunit->curHP <= 0)
			continue;

		if (tunit->state & (US_HIDDEN | US_DEAD | US_RESCUED | US_BIT16))
			continue;

		/* Skips the caster too: a unit is allied with itself */
		if (AreUnitsAllied(unit->index, tunit->index))
			continue;

		dmg = gBattleActor.battleAttack - GetUnitResistance(tunit);
		if (dmg <= 0)
			continue;

		if (dmg > tunit->curHP)
			dmg = tunit->curHP;

		targets[count] = tunit;
		damage[count] = dmg;
		count++;
	}

	if (count == 0)
		return false;

	proc = Proc_StartBlocking(ProcScr_GroupCastSplash, parent);
	for (i = 0; i < count; i++) {
		proc->targets[i] = targets[i];
		proc->damage[i] = damage[i];
	}
	proc->count = count;
	proc->index = 0;
	proc->exp = 0;
	proc->hidden = 0;
	return true;
#else
	return false;
#endif
}
