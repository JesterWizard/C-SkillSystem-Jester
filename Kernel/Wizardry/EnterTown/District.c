#include "common-chax.h"
#include "constants/texts.h"
#include "constants/worldmap.h"
#include "jester_headers/custom-structs.h"
#include "district.h"

/*
 * A district opens when the world-map story moves on from its story chapter (the next story chapter starts). Caer
 * Pelyn opens after Chapter 12 and Jehanna Hall after Chapter 15, stub chapters for now (CustomCampaign/Chapters/Stubs).
 */
const struct DistrictNode gDistrictNodes[] = {
	{ NODE_SERAFEW,         0x3B, 0x06, 0, GLOBAL_FLAG_DISTRICT_SERAFEW },    /* Ch5 */
	{ NODE_PORT_KIRIS,      0x3C, 0x0A, 0, GLOBAL_FLAG_DISTRICT_PORT_KIRIS }, /* Ch9 Eirika */
	{ NODE_CAER_PELYN,      0x41, 0x0C, 0, GLOBAL_FLAG_DISTRICT_CAER_PELYN }, /* Ch12 (stub) */
	{ NODE_JEHANNA_HALL_13, 0x42, 0x0F, 0, GLOBAL_FLAG_DISTRICT_JEHANNA },    /* Ch15 (stub) */
	{ DISTRICT_NODE_END, 0, 0, 0, 0 },
};

const struct DistrictNode *GetDistrictAtNode(int node)
{
	const struct DistrictNode *it;

	for (it = gDistrictNodes; it->node != DISTRICT_NODE_END; it++)
		if (it->node == node)
			return it;

	return NULL;
}

bool District_IsUnlocked(const struct DistrictNode *district)
{
	return CheckFlag(district->unlockFlag) != 0;
}

void District_MarkStoryChapterCleared(int storyChapter)
{
	const struct DistrictNode *it;

	for (it = gDistrictNodes; it->node != DISTRICT_NODE_END; it++)
		if (it->storyChapter == storyChapter)
			SetFlag(it->unlockFlag);
}

/*
 * Copy of vanilla EventScr_SkirmishRetreat (FE8U 0x08592058) with its text
 * (0x08FC "Retreat?") replaced by MSG_DISTRICT_LEAVE. Yes leaves the district
 * through StartRetreatProcessing, like the vanilla retreat.
 */
static const u32 EventScr_DistrictLeave[] = {
	0x00041020, 0x00001a23, 0x000b0540, 0xffffffff,
	(MSG_DISTRICT_LEAVE << 16) | 0x1b20, 0x00001d20, 0x00070540, 0x00000001,
	0x00000c41, 0x0007000c, 0x7fff1324, 0x00041721,
	0xffff2a21, 0x00001926, 0x00010540, 0x00000001,
	0x00000c41, 0x0001000c, 0x00000d40, 0x08037d59,
	0x00000820, 0x00001b22, 0x00070228, 0x00000120,
};

/*
 * Free-movement auto trap (HookList.event): stepping on a district gate
 * shows the "Leave the district?" prompt.
 */
void DistrictGate_OnStep(ProcPtr proc)
{
	CallEvent((const u16 *)EventScr_DistrictLeave, 1);
	gEventSlots[2] = (s8)gPlaySt.chapterIndex;
}
