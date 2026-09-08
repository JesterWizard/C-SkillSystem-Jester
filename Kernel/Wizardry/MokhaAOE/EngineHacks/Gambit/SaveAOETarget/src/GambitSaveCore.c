#include "common-chax.h"
#include "mokha-aoe.h"

void SaveTarget_PostGambitTargetSelection(void)
{
	int size = GetSelectTargetCount();
	int i;

	if (size > 0x40)
		size = 0x40;

	sGambitTargetSaveBuf[0] = size;

	for (i = 0; i < size; i++)
		sGambitTargetSaveBuf[i + 1] = GetTarget(i)->uid;
}

void ClearTarget_CommonFlagSaveSu(void)
{
	int i;

	for (i = 0; i < 0x42; i++)
		sGambitTargetSaveBuf[i] = 0;

	sGambitExpAccum = 0;
}
