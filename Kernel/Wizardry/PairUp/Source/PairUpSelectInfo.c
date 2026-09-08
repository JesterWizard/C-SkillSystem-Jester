#include "common-chax.h"
#include "bmmenu.h"
#include "uiselecttarget.h"

extern u8 PairUp_SelectionOnSwitchIn(
	ProcPtr proc,
	struct SelectTarget *target);
extern void PairUp_SelectionOnConstruction(ProcPtr proc);
extern void PairUp_SelectionOnEnd(ProcPtr proc);
extern u8 PairUp_SelectionOnSelect(
	ProcPtr proc,
	struct SelectTarget *target);
extern u8 PairUp_SelectionOnCancel(
	ProcPtr proc,
	struct SelectTarget *target);

const struct SelectInfo gSelectInfo_PairUp = {
	.onInit = PairUp_SelectionOnConstruction,
	.onEnd = PairUp_SelectionOnEnd,
	.onUnk08 = NULL,
	.onSwitchIn = PairUp_SelectionOnSwitchIn,
	.onSwitchOut = NULL,
	.onSelect = PairUp_SelectionOnSelect,
	.onCancel = PairUp_SelectionOnCancel,
	.onHelp = RescueSelection_OnHelp,
};
