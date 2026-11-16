#include "common-chax.h"
#include "bmbattle.h"
#include "ekrbattle.h"
#include "bmarena.h"
#include "gaiden-magic.h"
#include "combo-attack.h"
#include "double-badge.h"
#include "battle-system.h"

/*
 * Debugger, "Battle anim" (VeslyDebugger_Installer.event, page 3): the selected unit's battle animation, played over
 * and over against a stand-in Soldier (gArenaOpponent, off the map). Each fight is one scripted hit that does 1
 * damage: A switches the next one between the normal and the critical animation, B leaves after the fight that is
 * playing. Nothing about the unit changes: the battle is not BATTLE_CONFIG_REAL, so BattleUnwind rolls no rounds and
 * nothing applies its results.
 */

struct ProcBanimViewer {
	PROC_HEADER;

	struct Unit *unit;
	u8 crit;
	u8 quit;
	u8 anim_conf; /* the player's animation option, put back on the way out */
	u8 started;   /* the battle-animation deamon has run */
	u16 timer;
	u16 held;     /* the keys held last frame: the battle animation clears newKeys */
};

/* The weapon the animation shows: the equipped one, else the first black magic (tomes are spells here) */
/*
 * The viewer proc doesn't run while a battle animation plays (the fight locks the game logic), so the keys are
 * watched from PROC_TREE_3, where the battle animation's own procs run.
 */
struct ProcBanimViewerKeys {
	PROC_HEADER;

	struct ProcBanimViewer *viewer;
};

static void BanimViewerKeys_Loop(struct ProcBanimViewerKeys *proc)
{
	struct ProcBanimViewer *viewer = proc->viewer;
	u16 keys = gKeyStatusPtr->heldKeys & ~viewer->held;

	viewer->held = gKeyStatusPtr->heldKeys;
	if (keys & A_BUTTON) {
		viewer->crit = !viewer->crit;
		m4aSongNumStart(viewer->crit ? 0x6A : 0x6B); /* switched to the critical animation, or back */
	}
	if (keys & B_BUTTON)
		viewer->quit = true;
}

static const struct ProcCmd ProcScr_BanimViewerKeys[] = {
	PROC_NAME("DebugBanimViewerKeys"),
	PROC_REPEAT(BanimViewerKeys_Loop),
	PROC_END,
};

static bool BanimViewer_SetWeapon(struct BattleUnit *bu, struct Unit *unit)
{
	struct GaidenMagicList *list;

	if (GetUnitEquippedWeapon(unit)) {
		SetBattleUnitWeapon(bu, BU_ISLOT_AUTO);
		return bu->weapon != 0;
	}

	list = GetGaidenMagicList(unit);
	if (!list || list->bmag_cnt == 0)
		return false;

	bu->weaponSlotIndex = CHAX_BUISLOT_GAIDEN_BMAG1;
	bu->weapon = GetItemFromSlot(&bu->unit, CHAX_BUISLOT_GAIDEN_BMAG1);
	bu->weaponBefore = bu->weapon;
	bu->weaponAttributes = GetItemAttributes(bu->weapon);
	bu->weaponType = GetItemType(bu->weapon);
	bu->canCounter = false;
	return bu->weapon != 0;
}

static void BanimViewer_SetupOpponent(struct Unit *unit)
{
	struct Unit *opp = &gArenaOpponent;

	*opp = *unit;
	opp->index = FACTION_RED | 1;
	opp->pCharacterData = GetCharacterData(CHARACTER_ONEILL);
	opp->pClassData = GetClassData(CLASS_SOLDIER);
	opp->maxHP = opp->curHP = 60;
	opp->state = 0;
	CpuFill16(0, opp->items, sizeof(opp->items));

	/* next to the unit, so the fight is in melee range */
	opp->xPos = unit->xPos > 0 ? unit->xPos - 1 : unit->xPos + 1;
	opp->yPos = unit->yPos;
}

static bool BanimViewer_Generate(struct ProcBanimViewer *proc)
{
	struct Unit *unit = proc->unit;

	BanimViewer_SetupOpponent(unit);

	gBattleStats.config = 0;
	InitBattleUnit(&gBattleActor, unit);
	InitBattleUnit(&gBattleTarget, &gArenaOpponent);
	gBattleStats.range = 1;

	if (!BanimViewer_SetWeapon(&gBattleActor, unit))
		return false;

	SetBattleUnitWeapon(&gBattleTarget, BU_ISLOT_AUTO);
	BattleInitTargetCanCounter();
	SetBattleUnitTerrainBonusesAuto(&gBattleActor);
	SetBattleUnitTerrainBonusesAuto(&gBattleTarget);
	BattleGenerate(unit, &gArenaOpponent); /* not a real battle: no rounds */

	/* One hit for 1 damage, critical or not */
	ResetComboAtkList();
	ResetRoundEfxSkills();
	DoubleBadge_Set(0, 0);
	/* The kernel's hit list (BattleSys: gBattleHitArrayRe, with gExtBattleHitArray beside it), not vanilla's */
	CpuFill16(0, gExtBattleHitArray, 2 * sizeof(gExtBattleHitArray[0]));
	gBattleHitArrayRe[0].attributes = proc->crit ? BATTLE_HIT_ATTR_CRIT : 0;
	gBattleHitArrayRe[0].info = BATTLE_HIT_INFO_BEGIN;
	gBattleHitArrayRe[0].hpChange = 1;
	gBattleHitArrayRe[1].attributes = 0;
	gBattleHitArrayRe[1].info = BATTLE_HIT_INFO_END;
	gBattleHitArrayRe[1].hpChange = 0;

	gBattleTarget.unit.curHP = gBattleTarget.hpInitial - 1;
	gBattleActor.unit.curHP = gBattleActor.hpInitial;
	gBattleActor.expGain = 0;
	gBattleTarget.expGain = 0;
	gBattleActor.wexpMultiplier = 0;
	return true;
}

static void BanimViewer_Init(struct ProcBanimViewer *proc)
{
	proc->unit = gActiveUnit;
	proc->crit = false;
	proc->quit = false;
	proc->held = gKeyStatusPtr->heldKeys;
	((struct ProcBanimViewerKeys *)Proc_Start(ProcScr_BanimViewerKeys, PROC_TREE_3))->viewer = proc;
	proc->anim_conf = gPlaySt.config.animationType;
	gPlaySt.config.animationType = PLAY_ANIMCONF_ON;
}

static void BanimViewer_Begin(struct ProcBanimViewer *proc)
{
	if (!proc->unit || !BanimViewer_Generate(proc)) {
		m4aSongNumStart(0x6C); /* no weapon to show */
		Proc_Goto(proc, 1);
		return;
	}

	proc->timer = 0;
	proc->started = false;
	BeginBattleAnimations();
}

static void BanimViewer_Wait(struct ProcBanimViewer *proc)
{
	/*
	 * The battle-animation deamon runs from the start of the fight to its end. If it never starts (the fight fell
	 * back to a map animation), give up waiting after a few seconds.
	 */
	if (IsBattleDeamonActive()) {
		proc->started = true;
		return;
	}
	if (!proc->started && ++proc->timer < 240)
		return;

	if (proc->quit)
		Proc_Goto(proc, 1);
	else
		Proc_Goto(proc, 0);
}

static void BanimViewer_End(struct ProcBanimViewer *proc)
{
	Proc_EndEach(ProcScr_BanimViewerKeys);
	gPlaySt.config.animationType = proc->anim_conf;
	gBattleStats.config = 0;
	DoubleBadge_Set(0, 0);
	RefreshEntityBmMaps();
	RenderBmMap();
	RefreshUnitSprites();
}

static const struct ProcCmd ProcScr_BanimViewer[] = {
	PROC_NAME("DebugBanimViewer"),
	PROC_CALL(BanimViewer_Init),
	PROC_LABEL(0),
	PROC_CALL(BanimViewer_Begin),
	PROC_REPEAT(BanimViewer_Wait),
	PROC_LABEL(1),
	PROC_CALL(BanimViewer_End),
	PROC_END,
};

/* The debugger proc waits for it (C_Code.c, BanimViewLabel) */
void StartDebugBanimViewer(ProcPtr parent)
{
	Proc_StartBlocking(ProcScr_BanimViewer, parent);
}
