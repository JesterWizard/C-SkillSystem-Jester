#include "common-chax.h"
#include "map-anims.h"
#include "mu.h"
#include "kernel-lib.h"
#include "mokha-aoe.h"
#include "battle-system.h"
#include "constants/items.h"
#include "jester_headers/custom-functions.h"

struct ProcGamAction {
	/* 00 */ PROC_HEADER;
	/* 29 */ u8 waiting;
	/* 2A */ u8 index;
	/* 2B */ u8 count;
	/* 2C */ u8 attackIndex;
	/* 2E */ u16 expAccum;
	/* 30 */ u8 lastUid;
	/* 31 */ u8 _pad;
};

struct ProcGambitMuMotion {
	PROC_HEADER;
	u8 steps;
};

struct ProcGambitMapAnim {
	PROC_HEADER;
	struct Unit *target;
	int damage;
};

STATIC_DECLAR void Gambit_HideMapUnit(struct Unit *unit)
{
	if (!UNIT_IS_VALID(unit))
		return;

	HideUnitSprite(unit);
	unit->state |= US_HIDDEN;

	if (gBmMapUnit[unit->yPos][unit->xPos] == unit->index)
		gBmMapUnit[unit->yPos][unit->xPos] = 0;
}

STATIC_DECLAR void Gambit_ShowMapUnit(struct Unit *unit)
{
	if (!UNIT_IS_VALID(unit))
		return;

	unit->state &= ~US_HIDDEN;
	gBmMapUnit[unit->yPos][unit->xPos] = unit->index;
	ShowUnitSprite(unit);
}

STATIC_DECLAR void Gambit_ShowActiveUnit(ProcPtr proc)
{
	(void)proc;

	if (!UNIT_IS_VALID(gActiveUnit))
		return;

	Gambit_ShowMapUnit(gActiveUnit);
	RefreshUnitSprites();
}

STATIC_DECLAR void GambitMapAnim_Approach(struct ProcGambitMuMotion *proc)
{
	MapAnimMoveUnitTowardsTargetExt(
		gManimSt.actor[0].mu,
		gManimSt.actor[1].mu
	);

	if (++proc->steps >= 4)
		Proc_Break(proc);
}

STATIC_DECLAR void GambitMapAnim_Retreat(struct ProcGambitMuMotion *proc)
{
	MapAnimMoveUnitAwayFromTargetExt(
		gManimSt.actor[0].mu,
		gManimSt.actor[1].mu
	);

	if (++proc->steps >= 4)
		Proc_Break(proc);
}

STATIC_DECLAR void GambitMapAnim_EndMotion(ProcPtr proc)
{
	(void)proc;
	EndAllMus();
}

STATIC_DECLAR void GambitMapAnim_RestoreUnits(ProcPtr proc)
{
	struct Unit *target = GetUnit(gBattleTarget.unit.index);

	if (UNIT_IS_VALID(target))
		Gambit_ShowMapUnit(target);
	Gambit_ShowActiveUnit(proc);
}

STATIC_DECLAR const struct ProcCmd ProcScr_GambitMuApproach[] = {
	PROC_CALL(MapAnim_BeginSubjectFastAnim),
	PROC_REPEAT(GambitMapAnim_Approach),
	PROC_END,
};

STATIC_DECLAR const struct ProcCmd ProcScr_GambitMuRetreat[] = {
	PROC_REPEAT(GambitMapAnim_Retreat),
	PROC_CALL(GambitMapAnim_EndMotion),
	PROC_SLEEP(1),
	PROC_CALL(GambitMapAnim_RestoreUnits),
	PROC_END,
};

STATIC_DECLAR void GambitMapAnim_StartMotion(ProcPtr proc)
{
	struct ProcGambitMuMotion *motion;
	struct Unit *target;

	if (!UNIT_IS_VALID(gActiveUnit))
		return;

	Gambit_HideMapUnit(gActiveUnit);

	gManimSt.hp_changing = 0;
	gManimSt.mapAnimKind = MANIM_KIND_DAMAGE;
	gManimSt.actorCount = 2;
	gManimSt.subjectActorId = 0;
	gManimSt.targetActorId = 1;
	SetupMapBattleAnim(&gBattleActor, &gBattleTarget, gBattleHitArrayRe);
	SetBattleAnimFacing(0, 1, MA_FACING_OPPONENT);
	SetBattleAnimFacing(1, 0, MA_FACING_OPPONENT);

	/*
	 * SetupMapBattleAnim receives a BattleUnit copy.  Hide the live target
	 * after MU setup as well, including its hidden state and map occupancy,
	 * so RefreshUnitSprites cannot restore its SMS beneath the target MU.
	 */
	target = GetUnit(gBattleTarget.unit.index);
	Gambit_HideMapUnit(target);
	RefreshUnitSprites();

	motion = Proc_StartBlocking(ProcScr_GambitMuApproach, proc);
	motion->steps = 0;
}

STATIC_DECLAR void GambitMapAnim_EndMotionWait(ProcPtr proc)
{
	struct ProcGambitMuMotion *motion;

	motion = Proc_StartBlocking(ProcScr_GambitMuRetreat, proc);
	motion->steps = 0;
}

STATIC_DECLAR void GambitMapAnim_Init(ProcPtr proc)
{
	struct ProcGambitMapAnim *anim = (struct ProcGambitMapAnim *)proc;

	MapAnim_CommonInit();
	EnsureCameraOntoPosition(proc, anim->target->xPos, anim->target->yPos);
}

STATIC_DECLAR void GambitMapAnim_StartAttackAndHit(ProcPtr proc)
{
	struct ProcGambitMapAnim *anim = (struct ProcGambitMapAnim *)proc;
	struct MuProc *targetMu = gManimSt.actor[1].mu;

	StartMuActionAnim(gManimSt.actor[0].mu);

	if (targetMu)
		StartMuHitFlash(
			targetMu,
			GetSpellAssocFlashColor(gBattleActor.weaponBefore)
		);

	if (UNIT_IS_VALID(anim->target))
		AddUnitHp(anim->target, -anim->damage);
}

STATIC_DECLAR const struct ProcCmd ProcScr_GambitMapAnim[] = {
	PROC_CALL(GambitMapAnim_Init),
	PROC_YIELD,
	PROC_CALL(GambitMapAnim_StartMotion),
	PROC_YIELD,
	PROC_CALL(GambitMapAnim_StartAttackAndHit),
	PROC_SLEEP(8),
	PROC_CALL(GambitMapAnim_EndMotionWait),
	PROC_YIELD,
	PROC_CALL(MapAnim_CommonEnd),
	PROC_END,
};

STATIC_DECLAR void GambitMapAnim_Start(
	ProcPtr parent,
	struct Unit *target,
	int damage
)
{
	struct ProcGambitMapAnim *proc;

	proc = Proc_StartBlocking(ProcScr_GambitMapAnim, parent);
	proc->target = target;
	proc->damage = damage;
}

STATIC_DECLAR void Gambit_PrepareMapBattle(struct Unit *target)
{
	InitBattleUnit(&gBattleActor, gActiveUnit);
	InitBattleUnit(&gBattleTarget, target);

	/*
	 * Use an ordinary weapon association solely to select the vanilla
	 * moving map-battle animation.  The Gambit damage is applied by the
	 * local map-animation proc after the MU approach completes.
	 */
	gBattleActor.weapon = ITEM_NONE;
	gBattleActor.weaponBefore = ITEM_SWORD_IRON;
	gBattleActor.weaponAttributes = 0;
	gBattleActor.weaponType = 0;
	gBattleActor.canCounter = false;

	ClearBattleHits();
	gBattleHitArrayRe[0].attributes = 0;
	gBattleHitArrayRe[0].info = BATTLE_HIT_INFO_BEGIN;
	gBattleHitArrayRe[0].hpChange = 0;
	gBattleHitArrayRe[1].attributes = 0;
	gBattleHitArrayRe[1].info = BATTLE_HIT_INFO_END;
	gBattleHitArrayRe[1].hpChange = 0;
}

STATIC_DECLAR void Gambit_AccumulateExp(struct ProcGamAction *proc, struct Unit *target, int preHp, int damage)
{
	int postHp;
	int exp;
	u8 savedHp;

	if (damage <= 0)
		return;

	if (UNIT_FACTION(gActiveUnit) != FACTION_BLUE)
		return;

	if (gActiveUnit->exp == UNIT_EXP_DISABLED)
		return;

	if (gPlaySt.chapterStateBits & PLAY_FLAG_EXTRA_MAP)
		return;

	postHp = preHp - damage;
	if (postHp < 0)
		postHp = 0;

	/*
	 * Same helpers Override uses.  Temporarily set curHP so kill-bonus
	 * detection matches the pending hit, then restore before the anim.
	 */
	savedHp = target->curHP;
	target->curHP = postHp;
	exp = GetUnitRoundExp(gActiveUnit, target);
	if (postHp == 0)
		exp += GetUnitKillExpBonus(gActiveUnit, target);
	target->curHP = savedHp;

	if (exp < 1)
		exp = 1;

	if ((int)proc->expAccum + exp > 254)
		proc->expAccum = 254;
	else
		proc->expAccum += exp;
}

STATIC_DECLAR void Gambit_FinalizePreviousHit(struct ProcGamAction *proc)
{
	struct Unit *target;

	if (proc->lastUid == 0)
		return;

	target = GetUnit(proc->lastUid);
	proc->lastUid = 0;

	if (!UNIT_IS_VALID(target))
		return;

	if (target->curHP > 0)
		return;

	UnitKill(target);
}

STATIC_DECLAR void GambitAction_Tick(struct ProcGamAction *proc)
{
	struct Unit *target;
	int preHp;
	int damage;
	u8 uid;

	Gambit_FinalizePreviousHit(proc);

	if (proc->index >= proc->count) {
		ClearTarget_CommonFlagSaveSu();
		Proc_Break(proc);
		return;
	}

	uid = sGambitTargetSaveBuf[proc->index + 1];
	proc->index++;
	target = GetUnit(uid);

	if (!UNIT_IS_VALID(target) || (target->state & (US_DEAD | US_HIDDEN | US_BIT16)))
		return;

	preHp = GetUnitCurrentHp(target);
	damage = GetMokhaAoeDamage(target, proc->attackIndex);

	if (damage <= 0)
		return;

	Gambit_AccumulateExp(proc, target, preHp, damage);
	Gambit_PrepareMapBattle(target);
	proc->lastUid = uid;
	GambitMapAnim_Start(proc, target, damage);
}

STATIC_DECLAR void GambitAction_GrantExp(struct ProcGamAction *proc)
{
	int total = proc->expAccum;

	proc->expAccum = 0;
	sGambitExpAccum = 0;

	if (total <= 0) {
		Gambit_ShowActiveUnit(proc);
		return;
	}

	/*
	 * Match Override: start AddExp on TREE_3 (not as our child — parenting
	 * softlocks when this action ends), then keep this action alive with
	 * PROC_WHILE_EXISTS until the bar finishes transferring EXP.
	 */
	Gambit_HideMapUnit(gActiveUnit);
	AddExp_Event(total);
}

STATIC_DECLAR void GambitAction_AfterExp(struct ProcGamAction *proc)
{
	EndAllMus();
	Gambit_ShowActiveUnit(proc);
	RefreshUnitSprites();
}

STATIC_DECLAR void GambitAction_OnEnd(struct ProcGamAction *proc)
{
	(void)proc;
	ClearTarget_CommonFlagSaveSu();
	sGambitExpAccum = 0;
	GambitResetMaps();
}

const struct ProcCmd ProcScr_GambitAction[] = {
	PROC_SET_END_CB(GambitAction_OnEnd),
	PROC_YIELD,
	PROC_REPEAT(GambitAction_Tick),
	PROC_CALL(GambitAction_GrantExp),
	PROC_WHILE_EXISTS(ProcScr_AddExp),
	PROC_CALL(GambitAction_AfterExp),
	PROC_END,
};

bool GambitAction(ProcPtr parent)
{
	struct ProcGamAction *proc;

	if (!IsUnitMokhaAoeEligible(gActiveUnit)) {
		ClearTarget_CommonFlagSaveSu();
		return true;
	}

	proc = Proc_StartBlocking(ProcScr_GambitAction, parent);
	proc->waiting = 0;
	proc->index = 0;
	proc->count = sGambitTargetSaveBuf[0];
	proc->attackIndex = gActionData.unk08;
	proc->expAccum = 0;
	proc->lastUid = 0;
	sGambitExpAccum = 0;

	if (proc->count > 0x40)
		proc->count = 0x40;

	Gambit_ShowActiveUnit(proc);
	return false;
}
