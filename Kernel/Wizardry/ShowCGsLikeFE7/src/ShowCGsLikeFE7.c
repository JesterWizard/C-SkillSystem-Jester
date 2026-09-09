#include "common-chax.h"
#include "kernel-lib.h"
#include "event.h"
#include "eventscript.h"

int GetShowCGsLikeFE7Index(int idx)
{
	struct EventEngineProc *ee;
	const u8 *scr;

	if (gpKernelDesignerConfig->show_cgs_like_fe7 == false)
		return idx;

	ee = Proc_Find(ProcScr_StdEventEngine);
	if (ee == NULL || ee->pEventCurrent == NULL)
		return idx;

	scr = (const u8 *)ee->pEventCurrent;
	if (scr[1] != EV_CMD_SHOWBG)
		return idx;

	return scr[2];
}
