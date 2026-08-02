#include "common-chax.h"
#include "map-anims.h"
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

struct ProcGambitGrantExp {
	PROC_HEADER;
	int remaining;
	bool waiting;
};

struct ProcGambitMuMotion {
	PROC_HEADER;
	u8 steps;
};

STATIC_DECLAR void Gambit_ShowActiveUnit(ProcPtr proc)
{
	(void)proc;

	if (!UNIT_IS_VALID(gActiveUnit))
		return;

	gActiveUnit->state &= ~US_HIDDEN;
	gBmMapUnit[gActiveUnit->yPos][gActiveUnit->xPos] = gActiveUnit->index;
	RefreshUnitSprites();
	ShowUnitSprite(gActiveUnit);
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

STATIC_DECLAR void GambitMapAnim_StartAttack(ProcPtr proc)
{
	(void)proc;
	StartMuActionAnim(gManimSt.actor[0].mu);
}

STATIC_DECLAR void GambitMapAnim_EndMotion(ProcPtr proc)
{
	struct Unit *target = GetUnit(gBattleTarget.unit.index);

	EndAllMus();
	if (UNIT_IS_VALID(target))
		ShowUnitSprite(target);
	Gambit_ShowActiveUnit(proc);
}

STATIC_DECLAR const struct ProcCmd ProcScr_GambitMuApproach[] = {
	PROC_CALL(MapAnim_BeginSubjectFastAnim),
	PROC_REPEAT(GambitMapAnim_Approach),
	PROC_END,
};

STATIC_DECLAR const struct ProcCmd ProcScr_GambitMuRetreat[] = {
	PROC_CALL(GambitMapAnim_StartAttack),
	PROC_SLEEP(8),
	PROC_REPEAT(GambitMapAnim_Retreat),
	PROC_CALL(GambitMapAnim_EndMotion),
	PROC_END,
};

STATIC_DECLAR void GambitMapAnim_StartMotion(ProcPtr proc)
{
	struct ProcGambitMuMotion *motion;

	if (!UNIT_IS_VALID(gActiveUnit))
		return;

	HideUnitSprite(gActiveUnit);
	gActiveUnit->state |= US_HIDDEN;
	gBmMapUnit[gActiveUnit->yPos][gActiveUnit->xPos] = 0;

	gManimSt.hp_changing = 0;
	gManimSt.mapAnimKind = MANIM_KIND_DAMAGE;
	gManimSt.actorCount = 2;
	gManimSt.subjectActorId = 0;
	gManimSt.targetActorId = 1;
	SetupMapBattleAnim(&gBattleActor, &gBattleTarget, gBattleHitArrayRe);
	SetBattleAnimFacing(0, 1, MA_FACING_OPPONENT);
	SetBattleAnimFacing(1, 0, MA_FACING_OPPONENT);

	motion = Proc_StartBlocking(ProcScr_GambitMuApproach, proc);
	motion->steps = 0;
}

STATIC_DECLAR void GambitMapAnim_EndMotionWait(ProcPtr proc)
{
	struct ProcGambitMuMotion *motion;

	motion = Proc_StartBlocking(ProcScr_GambitMuRetreat, proc);
	motion->steps = 0;
}

STATIC_DECLAR void GambitMapAnim_Callback1(ProcPtr proc)
{
	GambitMapAnim_StartMotion(proc);
}

STATIC_DECLAR void GambitMapAnim_Callback2(ProcPtr proc)
{
	GambitMapAnim_EndMotionWait(proc);
}

STATIC_DECLAR void Gambit_PrepareMapBattle(struct Unit *target)
{
	InitBattleUnit(&gBattleActor, gActiveUnit);
	InitBattleUnit(&gBattleTarget, target);

	/*
	 * Use an ordinary weapon association solely to select the vanilla
	 * moving map-battle animation.  The Gambit damage is applied by the
	 * established Hurt proc after the MU approach completes.
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
	struct BattleUnit actorBu;
	struct BattleUnit targetBu;
	int postHp;
	int exp;

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

	InitBattleUnit(&actorBu, gActiveUnit);
	InitBattleUnit(&targetBu, target);

	actorBu.nonZeroDamage = true;
	targetBu.hpInitial = preHp;
	targetBu.unit.curHP = postHp;

	if (!CanBattleUnitGainLevels(&actorBu) || actorBu.unit.curHP == 0)
		return;

	exp = GetBattleUnitExpGainRework(&actorBu, &targetBu);
	if (exp < 0)
		exp = 0;

	if ((int)proc->expAccum + exp > 0xFFFF)
		proc->expAccum = 0xFFFF;
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

STATIC_DECLAR void GambitGrantExp_Loop(struct ProcGambitGrantExp *proc)
{
	int chunk;

	if (proc->waiting) {
		if (Proc_Exists(ProcScr_AddExp))
			return;

		proc->waiting = false;
	}

	if (proc->remaining <= 0) {
		Proc_Break(proc);
		return;
	}

	chunk = proc->remaining > 100 ? 100 : proc->remaining;
	proc->remaining -= chunk;

	ShowUnitSprite(gActiveUnit);

	/*
	 * AddExp_Event owns its animation proc on PROC_TREE_3.  Do not parent
	 * it below the Gambit action: ending the action while the EXP bar is
	 * active otherwise leaves the game locked behind a dead child proc.
	 */
	proc->waiting = true;
	AddExp_Event(chunk);
}

STATIC_DECLAR const struct ProcCmd ProcScr_GambitGrantExp[] = {
	PROC_REPEAT(GambitGrantExp_Loop),
	PROC_END,
};

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
	CallMapAnim_HurtExt(
		proc,
		target,
		damage,
		GambitMapAnim_Callback1,
		GambitMapAnim_Callback2
	);
}

STATIC_DECLAR void GambitAction_GrantExp(struct ProcGamAction *proc)
{
	struct ProcGambitGrantExp *grant;
	int total = proc->expAccum;

	proc->expAccum = 0;
	sGambitExpAccum = 0;

	if (total <= 0)
		return;

	ShowUnitSprite(gActiveUnit);
	/*
	 * Keep the grant sequence independent from the action proc.  The
	 * standard AddExp_Event proc also lives on PROC_TREE_3; making this
	 * sequence a child of an action that is ending can strand the game
	 * lock when the EXP bar finishes.
	 */
	grant = Proc_Start(ProcScr_GambitGrantExp, PROC_TREE_3);
	grant->remaining = total;
	grant->waiting = false;
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
