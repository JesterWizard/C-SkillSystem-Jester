#include "common-chax.h"
#include "combat-art.h"
#include "skill-system.h"
#include "battle-system.h"
#include "post-action.h"

void PostActionHook(ProcPtr proc)
{
	KernelStartBlockingHookProc(gpPostActionFuncs, proc);
}

bool PostActionEnd(ProcPtr proc)
{
	/* Some other proc-free routine */
	ResetCombatArtStatus();
	ResetSkillLists();
	ResetCombatArtList();

	/* Vanilla function at the hack entry */
	HandlePostActionTraps(proc);

	/* Last entry: the hook proc yields once before it ends anyway */
	return false;
}

bool PostActionPadFunc(ProcPtr proc)
{
	return false;
}

/* CHAX hooks */
const struct ProcCmd ProcScr_PostActionHookPlayPhase[] = {
	PROC_YIELD,
	PROC_CALL(PostActionHook),
	PROC_JUMP((const struct ProcCmd *)0x0859ABE8),
	PROC_END
};

/*
 * The yield after the call: PostActionHook starts the hook list as a blocking child, but the proc engine runs a proc's
 * commands until one stops, without looking at the lock in between. The jump went straight on to vanilla's
 * RunPotentialWaitEvents (a PROC_CALL_2 that goes on when there is no event), CpPerform_Cleanup and the script's end,
 * which deleted the hook list before its first entry ran: after an enemy's action no post-action hook ran (the enemy
 * promotion's effect and popup, an enemy's post-action skills) unless a wait event stopped the script first. The
 * yield ends the frame there; the lock holds the proc until the list is done. (The player phase's script goes on the
 * same way, but does not end before the list's first frame, so its list runs.)
 */
const struct ProcCmd ProcScr_PostActionHookCpPerform[] = {
	PROC_YIELD,
	PROC_CALL(PostActionHook),
	PROC_YIELD,
	PROC_JUMP((const struct ProcCmd *)0x085A807C),
	PROC_END
};
