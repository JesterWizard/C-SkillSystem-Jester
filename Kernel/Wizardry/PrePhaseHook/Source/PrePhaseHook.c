#include "common-chax.h"
#include "kernel-lib.h"
#include "debug-kit.h"

extern const HookProcFunc_t gPrePhaseFuncs[];
extern bool GetFreeMovementState(void);
extern bool NewPlayerPhaseEvaluationFunc(ProcPtr proc);

bool PrePhaseHook_VanillaEnd(ProcPtr proc)
{
	Proc_StartBlocking(gProcScr_GorgonEggHatchDisplay, proc);
	return true;
}

bool PrePhaseHook(ProcPtr proc)
{
	KernelStartBlockingHookProc(gPrePhaseFuncs, proc);
	return false;
}

extern bool (*gpExternalPrePhaseHook)(ProcPtr proc);

bool CallExternalPrePhaseHook(ProcPtr proc)
{
	if (gpExternalPrePhaseHook)
	{
		return gpExternalPrePhaseHook(proc);
	}

	if (GetFreeMovementState())
	{
		return NewPlayerPhaseEvaluationFunc(proc);
	}

	return false;
}
